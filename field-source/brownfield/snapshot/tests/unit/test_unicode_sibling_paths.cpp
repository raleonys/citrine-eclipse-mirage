#if defined(_WIN32)
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/process_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "preloader_l/runtime/avatar_path.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace {
constexpr uint16_t kModulePath[]={
    'C',':','\\','G','a','m','e','s','\\','E','A',' ','&',' ','S','p','o','r',0x011f,
    '\\','p','r','e','l','o','a','d','e','r','_','l','.','d','l','l',0u
};
uint16_t g_opened_config[128]{};
uint16_t g_opened_override[128]{};
unsigned int g_last_code_page=0;
unsigned long g_last_flags=0;

size_t wide_len(const uint16_t* s) {
    size_t n=0; while(s&&s[n]) ++n; return n;
}

int utf8_encode(const uint16_t* src,int count,char* out,int capacity) {
    int written=0;
    for(int i=0;i<count;++i){
        uint32_t cp=src[i];
        if(cp>=0xd800u&&cp<=0xdbffu){
            if(i+1>=count)return 0;
            const uint32_t low=src[++i];
            if(low<0xdc00u||low>0xdfffu)return 0;
            cp=0x10000u+((cp-0xd800u)<<10)+(low-0xdc00u);
        } else if(cp>=0xdc00u&&cp<=0xdfffu) return 0;
        char bytes[4]{}; int n=0;
        if(cp<=0x7fu){bytes[0]=static_cast<char>(cp);n=1;}
        else if(cp<=0x7ffu){bytes[0]=static_cast<char>(0xc0u|(cp>>6));bytes[1]=static_cast<char>(0x80u|(cp&0x3fu));n=2;}
        else if(cp<=0xffffu){bytes[0]=static_cast<char>(0xe0u|(cp>>12));bytes[1]=static_cast<char>(0x80u|((cp>>6)&0x3fu));bytes[2]=static_cast<char>(0x80u|(cp&0x3fu));n=3;}
        else {bytes[0]=static_cast<char>(0xf0u|(cp>>18));bytes[1]=static_cast<char>(0x80u|((cp>>12)&0x3fu));bytes[2]=static_cast<char>(0x80u|((cp>>6)&0x3fu));bytes[3]=static_cast<char>(0x80u|(cp&0x3fu));n=4;}
        if(out){if(written+n>capacity)return 0;for(int j=0;j<n;++j)out[written+j]=bytes[j];}
        written+=n;
    }
    return written;
}
}

extern "C" unsigned long GetModuleFileNameW(void*,uint16_t* buffer,unsigned long size) {
    const size_t n=wide_len(kModulePath);
    if(!buffer||size<=n)return 0;
    for(size_t i=0;i<=n;++i)buffer[i]=kModulePath[i];
    return static_cast<unsigned long>(n);
}
extern "C" unsigned long GetCurrentDirectoryA(unsigned long size,char* buffer) {
    constexpr char cwd[]="C:\\cwd"; const size_t n=sizeof(cwd)-1u;
    if(!buffer||size==0)return static_cast<unsigned long>(n+1u);
    if(size<=n)return static_cast<unsigned long>(n+1u);
    std::memcpy(buffer,cwd,sizeof(cwd)); return static_cast<unsigned long>(n);
}
extern "C" unsigned long GetCurrentDirectoryW(unsigned long size,uint16_t* buffer) {
    constexpr uint16_t cwd[]={'C',':','\\','c','w','d',0u}; const size_t n=6u;
    if(!buffer||size==0)return static_cast<unsigned long>(n+1u);
    if(size<=n)return static_cast<unsigned long>(n+1u);
    for(size_t i=0;i<=n;++i)buffer[i]=cwd[i];
    return static_cast<unsigned long>(n);
}
extern "C" char* GetCommandLineA() { static char cmd[]="game.exe"; return cmd; }
extern "C" int WideCharToMultiByte(unsigned int cp,unsigned long flags,const uint16_t* src,int count,char* out,int capacity,const char*,int*) {
    g_last_code_page=cp; g_last_flags=flags;
    if(!src||count<0)return 0;
    return utf8_encode(src,count,out,capacity);
}
extern "C" void* GetProcessHeap(){return reinterpret_cast<void*>(1);}
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){return std::malloc(n);}
extern "C" int HeapFree(void*,unsigned long,void* p){std::free(p);return 1;}

namespace preloader_l { namespace runtime {
StateSnapshot snapshot_state() noexcept {
    StateSnapshot s{}; s.phase=Phase::initialized; s.module_base=0x1234u; return s;
}
AllocationResult allocate_block(size_t size,AllocationRetryPolicy,const AllocatorPlatformOps&) {
    return {std::malloc(size),AllocationTerminal::none,size,size,0u,false};
}
AllocationReleaseStatus release_block(void* p,size_t,const AllocatorPlatformOps&) noexcept { std::free(p); return AllocationReleaseStatus::ok; }
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
    static const AllocatorPlatformOps ops{nullptr,nullptr,nullptr,nullptr}; return ops;
}
} }

namespace preloader_l { namespace config {
RuntimeConfigurationFileStatus load_runtime_configuration_pair_wide(const uint16_t* base, const uint16_t* over) noexcept {
    if(!base||!over)return RuntimeConfigurationFileStatus::invalid_argument;
    size_t i=0; for(;base[i]&&i+1u<sizeof(g_opened_config)/sizeof(g_opened_config[0]);++i)g_opened_config[i]=base[i];
    g_opened_config[i]=0u;
    i=0; for(;over[i]&&i+1u<sizeof(g_opened_override)/sizeof(g_opened_override[0]);++i)g_opened_override[i]=over[i];
    g_opened_override[i]=0u;
    return RuntimeConfigurationFileStatus::not_found;
}
uint32_t runtime_configuration_present_sections() noexcept { return 0u; }
RuntimeConfigurationStatus set_runtime_current_directory(const char*,size_t) noexcept { return RuntimeConfigurationStatus::ok; }
bool set_runtime_current_directory_wide_policy(const uint16_t*,size_t) noexcept { return true; }
void set_runtime_language_override(ValueView) noexcept {}
} }

int main(){
    const auto r=preloader_l::config::load_process_runtime_configuration();
    if(r.file_status!=preloader_l::config::RuntimeConfigurationFileStatus::not_found)return 1;
    constexpr uint16_t expected_cfg[]={
        'C',':','\\','G','a','m','e','s','\\','E','A',' ','&',' ','S','p','o','r',0x011f,
        '\\','s','u','r','g','e','.','c','f','g',0u
    };
    for(size_t i=0;;++i){if(g_opened_config[i]!=expected_cfg[i])return 2;if(!expected_cfg[i])break;}

    constexpr uint16_t expected_override[]={
        'C',':','\\','G','a','m','e','s','\\','E','A',' ','&',' ','S','p','o','r',0x011f,
        '\\','s','u','r','g','e','_','o','v','e','r','r','i','d','e','.','c','f','g',0u
    };
    for(size_t i=0;;++i){if(g_opened_override[i]!=expected_override[i])return 8;if(!expected_override[i])break;}

    size_t needed=0;
    if(preloader_l::runtime::normalized_avatar_path(nullptr,0,&needed))return 3;
    constexpr char expected_avatar[]="C:\\Games\\EA & Spor\xc4\x9f\\surge.png";
    if(needed!=sizeof(expected_avatar)-1u)return 4;
    char out[128]{}; size_t actual=0;
    if(!preloader_l::runtime::normalized_avatar_path(out,sizeof(out),&actual))return 5;
    if(actual!=sizeof(expected_avatar)-1u||std::memcmp(out,expected_avatar,sizeof(expected_avatar))!=0)return 6;
    if(g_last_code_page!=65001u||g_last_flags!=0x80u)return 7;
    return 0;
}
#else
int main(){return 0;}
#endif
