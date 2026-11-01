#if !defined(_WIN32)
#error runtime_configuration_file.cpp is Windows-only.
#endif
#include "preloader_l/config/runtime_configuration_file.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "config2_pegtl.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include <stddef.h>
#include <stdint.h>

namespace {
using DWORD=unsigned long;
using BOOL=int;
using HANDLE=void*;
HANDLE const bad=(HANDLE)(intptr_t)-1;
constexpr DWORD read_access=0x80000000u, share_read=1u, open_existing=3u, normal=0x80u;
constexpr DWORD error_file_not_found=2u, error_path_not_found=3u;
extern "C" {
__declspec(dllimport) HANDLE __stdcall CreateFileA(const char*,DWORD,DWORD,void*,DWORD,DWORD,HANDLE);
__declspec(dllimport) HANDLE __stdcall CreateFileW(const uint16_t*,DWORD,DWORD,void*,DWORD,DWORD,HANDLE);
__declspec(dllimport) BOOL __stdcall ReadFile(HANDLE,void*,DWORD,DWORD*,void*);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE);
__declspec(dllimport) DWORD __stdcall GetLastError();
}

void release_bytes(uint8_t* p,size_t capacity) noexcept {
    if (!p) return;
    preloader_l::runtime::release_block(
        p,capacity,preloader_l::runtime::windows_allocator_platform_ops());
}

uint8_t* allocate_bytes(size_t capacity) noexcept {
    if (!capacity) return nullptr;
    const auto r=preloader_l::runtime::allocate_block(
        capacity,preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
        preloader_l::runtime::windows_allocator_platform_ops());
    return r.terminal==preloader_l::runtime::AllocationTerminal::none
        ? static_cast<uint8_t*>(r.pointer) : nullptr;
}

bool grow(uint8_t** buffer,size_t* capacity,size_t used) noexcept {
    if (!buffer||!capacity) return false;
    size_t next=*capacity ? *capacity + *capacity/2u + 1u : 4096u;
    if (next<=*capacity) return false;
    uint8_t* fresh=allocate_bytes(next);
    if(!fresh)return false;
    for(size_t i=0;i<used;++i)fresh[i]=(*buffer)[i];
    release_bytes(*buffer,*capacity);
    *buffer=fresh;*capacity=next;return true;
}
}

namespace preloader_l { namespace config {
namespace {
RuntimeConfigurationFileStatus classify_open_error(DWORD error) noexcept {
    return error==error_file_not_found||error==error_path_not_found
        ? RuntimeConfigurationFileStatus::not_found
        : RuntimeConfigurationFileStatus::io_error;
}

RuntimeConfigurationFileStatus load_runtime_configuration_handle(HANDLE h) noexcept {
    if(h==bad)return RuntimeConfigurationFileStatus::io_error;

    uint8_t* bytes=nullptr;
    size_t capacity=0,total=0;
    RuntimeConfigurationFileStatus file_status=RuntimeConfigurationFileStatus::ok;

    for(;;){
        if(total==capacity && !grow(&bytes,&capacity,total)){
            file_status=RuntimeConfigurationFileStatus::resource_error;
            break;
        }
        const size_t available=capacity-total;
        const DWORD want=available>0x10000u?0x10000u:static_cast<DWORD>(available);
        DWORD got=0;
        if(!ReadFile(h,bytes+total,want,&got,nullptr)){
            file_status=RuntimeConfigurationFileStatus::io_error;
            break;
        }
        total+=static_cast<size_t>(got);
        if(got<want)break;
    }
    CloseHandle(h);

    if(file_status!=RuntimeConfigurationFileStatus::ok){
        release_bytes(bytes,capacity);
        return file_status;
    }

    const auto s=set_runtime_configuration_cfg(reinterpret_cast<const char*>(bytes),total);
    release_bytes(bytes,capacity);
    if(s==RuntimeConfigurationStatus::ok)return RuntimeConfigurationFileStatus::ok;
    if(s==RuntimeConfigurationStatus::invalid_cfg)return RuntimeConfigurationFileStatus::invalid_cfg;
    if(s==RuntimeConfigurationStatus::resource_error)return RuntimeConfigurationFileStatus::resource_error;
    return RuntimeConfigurationFileStatus::invalid_argument;
}
}

// Two-phase, no-partial-publication Config2 base+override path. Each file is
// independently owned; a single combined document reaches the authoritative
// transactional model/service writer exactly once.
namespace {
struct FileBytes { uint8_t* data=nullptr; size_t size=0; size_t capacity=0; };
void clear_file(FileBytes& b) noexcept {
    release_bytes(b.data,b.capacity); b=FileBytes{};
}
RuntimeConfigurationFileStatus read_wide_owned(const uint16_t* path,FileBytes& out) noexcept {
    if(!path||!path[0])return RuntimeConfigurationFileStatus::invalid_argument;
    const HANDLE h=CreateFileW(path,read_access,share_read,nullptr,open_existing,normal,nullptr);
    if(h==bad)return classify_open_error(GetLastError());
    RuntimeConfigurationFileStatus result=RuntimeConfigurationFileStatus::ok;
    for(;;){
        if(out.size==out.capacity && !grow(&out.data,&out.capacity,out.size)){
            result=RuntimeConfigurationFileStatus::resource_error;break;
        }
        const size_t available=out.capacity-out.size;
        const DWORD want=available>0x10000u?0x10000u:static_cast<DWORD>(available);
        DWORD got=0;
        if(!ReadFile(h,out.data+out.size,want,&got,nullptr)){
            result=RuntimeConfigurationFileStatus::io_error;break;
        }
        if(got>want){result=RuntimeConfigurationFileStatus::io_error;break;}
        out.size+=static_cast<size_t>(got);
        if(got<want)break;
    }
    if(!CloseHandle(h) && result==RuntimeConfigurationFileStatus::ok)
        result=RuntimeConfigurationFileStatus::io_error;
    return result;
}
RuntimeConfigurationFileStatus publish_buffer(const char* data,size_t length) noexcept {
    const auto status=set_runtime_configuration_cfg(data,length);
    if(status==RuntimeConfigurationStatus::ok)return RuntimeConfigurationFileStatus::ok;
    if(status==RuntimeConfigurationStatus::invalid_cfg)return RuntimeConfigurationFileStatus::invalid_cfg;
    if(status==RuntimeConfigurationStatus::resource_error)return RuntimeConfigurationFileStatus::resource_error;
    return RuntimeConfigurationFileStatus::invalid_argument;
}
// A grammar-valid Config2 document has one outer object. Lexically locate its
// opening/closing braces, excluding quoted text and both comment forms.
// The actual PEGTL parser is authoritative; this scan only extracts its body.
bool valid_root_body(const FileBytes& b,size_t* begin,size_t* end) noexcept {
    if(!begin||!end||!b.data||!b.size)return false;
    const preloader_l::config2_pegtl::Visitor empty{nullptr,nullptr,nullptr};
    if(preloader_l::config2_pegtl::parse(reinterpret_cast<const char*>(b.data),b.size,empty)
       !=preloader_l::config2_pegtl::Result::ok)return false;
    bool quoted=false,escaped=false,line_comment=false,block_comment=false;
    size_t depth=0;bool found=false;
    *begin=*end=0;
    for(size_t i=0;i<b.size;++i){
        const char c=static_cast<char>(b.data[i]);
        const char next=i+1u<b.size?static_cast<char>(b.data[i+1u]):0;
        if(line_comment){if(c=='\n'||c=='\r')line_comment=false;continue;}
        if(block_comment){if(c=='*'&&next=='/'){block_comment=false;++i;}continue;}
        if(quoted){
            if(escaped)escaped=false;
            else if(c=='\\')escaped=true;
            else if(c=='"')quoted=false;
            continue;
        }
        if(c=='"'){quoted=true;continue;}
        if(c=='/'&&next=='/'){line_comment=true;++i;continue;}
        if(c=='/'&&next=='*'){block_comment=true;++i;continue;}
        if(c=='{'){
            if(!depth){if(found)return false;*begin=i+1u;found=true;}
            ++depth;
        }else if(c=='}'){
            if(!depth)return false;
            --depth;
            if(!depth)*end=i;
        }
    }
    return found&&!depth&&*begin<=*end&&*end<=b.size;
}
} // namespace

RuntimeConfigurationFileStatus load_runtime_configuration_pair_wide(
    const uint16_t* base_path,const uint16_t* override_path) noexcept {
    if(!base_path||!base_path[0]||!override_path||!override_path[0])
        return RuntimeConfigurationFileStatus::invalid_argument;
    FileBytes base{},override_file{};
    const auto base_status=read_wide_owned(base_path,base);
    if(base_status!=RuntimeConfigurationFileStatus::ok&&
       base_status!=RuntimeConfigurationFileStatus::not_found){
        clear_file(base);return base_status;
    }
    const auto override_status=read_wide_owned(override_path,override_file);
    if(override_status!=RuntimeConfigurationFileStatus::ok&&
       override_status!=RuntimeConfigurationFileStatus::not_found){
        clear_file(base);clear_file(override_file);return override_status;
    }
    const bool have_base=base_status==RuntimeConfigurationFileStatus::ok;
    const bool have_override=override_status==RuntimeConfigurationFileStatus::ok;
    if(!have_base&&!have_override){clear_file(base);clear_file(override_file);
        return RuntimeConfigurationFileStatus::not_found;}
    if(have_base&&!have_override){
        const auto s=publish_buffer(reinterpret_cast<const char*>(base.data),base.size);
        clear_file(base);clear_file(override_file);return s;
    }
    if(!have_base&&have_override){
        const auto s=publish_buffer(reinterpret_cast<const char*>(override_file.data),override_file.size);
        clear_file(base);clear_file(override_file);return s;
    }
    size_t bbegin=0,bend=0,obegin=0,oend=0;
    if(!valid_root_body(base,&bbegin,&bend)||
       !valid_root_body(override_file,&obegin,&oend)){
        clear_file(base);clear_file(override_file);
        return RuntimeConfigurationFileStatus::invalid_cfg;
    }
    constexpr char prefix[]="\"Config2\" {\n";
    constexpr char middle[]="\n";
    constexpr char suffix[]="\n}";
    const size_t nb=bend-bbegin,no=oend-obegin;
    const size_t extra=sizeof(prefix)-1u+sizeof(middle)-1u+sizeof(suffix)-1u;
    if(nb>static_cast<size_t>(-1)-extra ||
       no>static_cast<size_t>(-1)-extra-nb){
        clear_file(base);clear_file(override_file);
        return RuntimeConfigurationFileStatus::resource_error;
    }
    const size_t total=extra+nb+no;
    uint8_t* merged=allocate_bytes(total);
    if(!merged){clear_file(base);clear_file(override_file);
        return RuntimeConfigurationFileStatus::resource_error;}
    size_t pos=0;
    auto append=[&](const uint8_t* src,size_t n) noexcept {
        for(size_t i=0;i<n;++i)merged[pos++]=src[i];
    };
    append(reinterpret_cast<const uint8_t*>(prefix),sizeof(prefix)-1u);
    append(base.data+bbegin,nb);
    append(reinterpret_cast<const uint8_t*>(middle),sizeof(middle)-1u);
    append(override_file.data+obegin,no);
    append(reinterpret_cast<const uint8_t*>(suffix),sizeof(suffix)-1u);
    const auto result=publish_buffer(reinterpret_cast<const char*>(merged),total);
    release_bytes(merged,total);
    clear_file(base);clear_file(override_file);
    return result;
}

RuntimeConfigurationFileStatus load_runtime_configuration_file(const char* path) noexcept {
    const char* p=(path&&path[0])?path:"surge.cfg";
    HANDLE h=CreateFileA(p,read_access,share_read,nullptr,open_existing,normal,nullptr);
    if(h==bad){const DWORD error=GetLastError();return classify_open_error(error);}
    return load_runtime_configuration_handle(h);
}

RuntimeConfigurationFileStatus load_runtime_configuration_file_wide(const uint16_t* path) noexcept {
    if(!path||!path[0])return RuntimeConfigurationFileStatus::invalid_argument;
    HANDLE h=CreateFileW(path,read_access,share_read,nullptr,open_existing,normal,nullptr);
    if(h==bad){const DWORD error=GetLastError();return classify_open_error(error);}
    return load_runtime_configuration_handle(h);
}
} }
