#include "filesystem.hpp"
#include "../anadius-path-source/paths.hpp"
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <stdexcept>
#include <system_error>
#include <type_traits>

namespace anadius::config_storage {
namespace {
bool separator(char16_t c) noexcept{return c==u'\\'||c==u'/';}
bool drive_at(const Path& p,std::size_t i) noexcept{
    return i+1<p.size()&&p[i+1]==u':'&&((p[i]>=u'A'&&p[i]<=u'Z')||(p[i]>=u'a'&&p[i]<=u'z'));
}
std::wstring wide(const Path& p){static_assert(sizeof(wchar_t)==sizeof(char16_t));return {p.begin(),p.end()};}
struct Boundary {
    FilesystemApi api;DWORD value;
    Filesystem::Checkpoint checkpoint{};
    explicit Boundary(FilesystemApi a):api(a),value(a.last_error()){}
    ~Boundary() noexcept{api.set_last_error(value);}
    template<class F> auto call(F&& f){
        api.set_last_error(value);
        if(checkpoint.check)checkpoint.check(checkpoint.context);
        try{
            auto result=f();value=api.last_error();
            if(checkpoint.check)checkpoint.check(checkpoint.context);
            return result;
        }catch(...){value=api.last_error();throw;}
    }
    void visit(configuration::DirectoryVisitor visitor,const Path& path){
        // Callback effects belong to the source loop, not local bookkeeping.
        // Preserve their last SDK status for the next iterator advancement.
        api.set_last_error(value);
        try{visitor.visit(visitor.context,path);value=api.last_error();}
        catch(...){value=api.last_error();throw;}
    }
};
[[noreturn]] void fail(const char* operation,DWORD error){
    throw std::system_error(static_cast<int>(error),std::system_category(),operation);
}
HANDLE acquire(Boundary& b,auto&& action){
    // The declared SDK outcome contract excludes throwing acquisition. If an
    // injected function violates it after creating a handle, never guess that
    // no handle exists and continue with lost ownership.
    try{return b.call(action);}catch(...){std::abort();}
}
struct OwnedHandle {
    Boundary& boundary;HANDLE value;bool find;
    ~OwnedHandle() noexcept{
        if(value==INVALID_HANDLE_VALUE)return;
        try{
            const auto ok=boundary.call([&]{return find?boundary.api.find_close(value):boundary.api.close_handle(value);});
            if(ok==FALSE)std::abort();
        }catch(...){std::abort();}
    }
};
std::uint64_t timestamp(FILETIME t){return (std::uint64_t(t.dwHighDateTime)<<32)|t.dwLowDateTime;}
// Reuses the existing bounded049104 implementation rather than substituting
// an already-exists boolean based only on CreateDirectory's error.
class StatsCalls final:public status_recovered::SystemCalls {
    Boundary& b_;
public:
    explicit StatsCalls(Boundary& b):b_(b){}
    std::uint32_t attributes_ex(const Path& p,status_recovered::Metadata& out) override{
        const auto w=wide(p);WIN32_FILE_ATTRIBUTE_DATA data{};
        if(!b_.call([&]{return b_.api.attributes_ex(w.c_str(),GetFileExInfoStandard,&data);}))return b_.value;
        out={data.dwFileAttributes,timestamp(data.ftLastWriteTime),(std::uint64_t(data.nFileSizeHigh)<<32)|data.nFileSizeLow};return 0;
    }
    status_recovered::FindResult find_first(const Path& p) override{
        const auto w=wide(p);WIN32_FIND_DATAW data{};
        const auto h=acquire(b_,[&]{return b_.api.find_first(w.c_str(),&data);});
        if(h==INVALID_HANDLE_VALUE)return {status_recovered::invalid_handle,b_.value,{}};
        return {reinterpret_cast<std::intptr_t>(h),0,{data.dwFileAttributes,timestamp(data.ftLastWriteTime),(std::uint64_t(data.nFileSizeHigh)<<32)|data.nFileSizeLow}};
    }
    bool find_close(status_recovered::Handle h) override{
        // UsedStats faithfully ignores this return in retained049104. This
        // B8 ownership boundary deliberately makes failed release terminal
        // rather than discarding a known live native handle.
        if(b_.call([&]{return b_.api.find_close(reinterpret_cast<HANDLE>(h));})==FALSE)std::abort();
        return true;
    }
    status_recovered::OpenResult open_attributes(const Path& p,std::uint32_t access,std::uint32_t share,std::uint32_t disposition,std::uint32_t flags) override{
        const auto w=wide(p);
        const auto h=acquire(b_,[&]{return b_.api.create_file(w.c_str(),access,share,nullptr,disposition,flags,nullptr);});
        return {reinterpret_cast<std::intptr_t>(h),h==INVALID_HANDLE_VALUE?b_.value:0};
    }
    std::uint32_t basic_information(status_recovered::Handle h,status_recovered::Metadata& out) override{
        FILE_BASIC_INFO info{};
        if(!b_.call([&]{return b_.api.get_info(reinterpret_cast<HANDLE>(h),FileBasicInfo,&info,sizeof(info));}))return b_.value;
        out.attributes=info.FileAttributes;out.last_write=static_cast<std::uint64_t>(info.LastWriteTime.QuadPart);return 0;
    }
    bool close(status_recovered::Handle h) override{
        return b_.call([&]{return b_.api.close_handle(reinterpret_cast<HANDLE>(h));})!=FALSE;
    }
    [[noreturn]] void abort_close_failure_05f03c() override{std::abort();}
};
struct CreateResult {bool created;DWORD error;};
CreateResult create_one(const Path& p,Boundary& b){
    const auto w=wide(p);
    if(b.call([&]{return b.api.create_directory(w.c_str(),nullptr);})!=FALSE)return {true,0};
    auto error=b.value;
    if(error==ERROR_ALREADY_EXISTS){
        StatsCalls calls(b);status_recovered::UsedStats stats(calls);const auto status=stats.query(p,3);
        error=status.error;
        if(!error&&(status.attributes&FILE_ATTRIBUTE_DIRECTORY)==0)error=ERROR_ALREADY_EXISTS;
    }
    return {false,error};
}
bool ignorable_prefix(DWORD error) noexcept{
    return error==ERROR_FILE_NOT_FOUND||error==ERROR_PATH_NOT_FOUND||error==ERROR_BAD_NETPATH||error==ERROR_INVALID_NAME;
}
DWORD create_all(const Path& p,Boundary& b){
    if(p.empty())return ERROR_PATH_NOT_FOUND;
    auto cursor=path_recovered::root_name_length_00bb30(p);
    while(cursor<p.size()&&separator(p[cursor]))++cursor;
    // Retained special case for a drive component following an extended root.
    if(cursor&&p.size()-cursor>=3&&drive_at(p,cursor)&&separator(p[cursor+2]))cursor+=2;
    Path prefix=p.substr(0,cursor);auto start=cursor;DWORD last=0,remembered=0;
    while(cursor<p.size()){
        if(separator(p[cursor])){++cursor;if(cursor<p.size())continue;}
        while(cursor<p.size()&&!separator(p[cursor]))++cursor;
        prefix.append(p,start,cursor-start);
        last=create_one(prefix,b).error;
        if(last&&!ignorable_prefix(last))remembered=last;
        start=cursor;
    }
    return last&&remembered?remembered:last;
}
DWORD disposition_fallback(HANDLE h,Boundary& b){
    DWORD flags=3;
    constexpr auto extended=static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x15);
    if(b.call([&]{return b.api.set_info(h,extended,&flags,sizeof(flags));})!=FALSE)return 0;
    auto error=b.value;
    if(error!=ERROR_INVALID_FUNCTION&&error!=ERROR_NOT_SUPPORTED&&error!=ERROR_INVALID_PARAMETER)return error;
    // Retained native helper passes one byte, not compiler FILE_DISPOSITION_INFO.
    std::uint8_t deletion=1;
    if(b.call([&]{return b.api.set_info(h,FileDispositionInfo,&deletion,1);})!=FALSE)return 0;
    return b.value;
}
DWORD remove_handle(HANDLE h,bool full_access,Boundary& b){
    DWORD flags=0x13;
    constexpr auto extended=static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x15);
    if(b.call([&]{return b.api.set_info(h,extended,&flags,sizeof(flags));})!=FALSE)return 0;
    const auto first=b.value;
    if(first!=ERROR_INVALID_FUNCTION&&first!=ERROR_NOT_SUPPORTED&&first!=ERROR_INVALID_PARAMETER)return first;
    const auto fallback=disposition_fallback(h,b);
    if(fallback!=ERROR_ACCESS_DENIED||!full_access)return fallback;
    FILE_BASIC_INFO info{};
    if(b.call([&]{return b.api.get_info(h,FileBasicInfo,&info,sizeof(info));})==FALSE)return b.value;
    if(!(info.FileAttributes&FILE_ATTRIBUTE_READONLY))return ERROR_ACCESS_DENIED;
    info.FileAttributes^=FILE_ATTRIBUTE_READONLY;
    if(b.call([&]{return b.api.set_info(h,FileBasicInfo,&info,sizeof(info));})==FALSE)return b.value;
    const auto retry=disposition_fallback(h,b);
    if(retry==0)return 0;
    // This path preserves the retained side effect: restore readonly only for
    // repeated access-denied. A different retry error does not undo the change.
    if(retry!=ERROR_ACCESS_DENIED)return b.value;
    info.FileAttributes|=FILE_ATTRIBUTE_READONLY;
    if(b.call([&]{return b.api.set_info(h,FileBasicInfo,&info,sizeof(info));})==FALSE)return b.value;
    return ERROR_ACCESS_DENIED;
}
bool dot(const WIN32_FIND_DATAW& data){
    return data.cFileName[0]==L'.'&&(data.cFileName[1]==0||(data.cFileName[1]==L'.'&&data.cFileName[2]==0));
}
Path filename(const WIN32_FIND_DATAW& data){
    const auto end=std::find(std::begin(data.cFileName),std::end(data.cFileName),wchar_t{});
    if(end==std::end(data.cFileName))throw std::domain_error("SDK filename is outside its terminated WIN32_FIND_DATAW contract");
    return {std::begin(data.cFileName),end};
}
}
FilesystemApi native_filesystem_api() noexcept{
    return {::CreateDirectoryW,::GetFileAttributesExW,::FindFirstFileW,::FindFirstFileExW,
        ::FindNextFileW,::FindClose,::CreateFileW,::GetFileInformationByHandleEx,
        ::SetFileInformationByHandle,::CloseHandle,::GetLastError,::SetLastError};
}
Filesystem::Filesystem(FilesystemApi a):api_(a){
    if(!a.create_directory||!a.attributes_ex||!a.find_first||!a.find_first_ex||!a.find_next||
       !a.find_close||!a.create_file||!a.get_info||!a.set_info||!a.close_handle||!a.last_error||!a.set_last_error)
        throw std::invalid_argument("Complete configuration filesystem SDK API required");
}
void Filesystem::create_directories(const Path& p){
    create_directories(p,{});
}
void Filesystem::create_directories(const Path& p,Checkpoint checkpoint){
    if(checkpoint.check)checkpoint.check(checkpoint.context);
    Boundary boundary(api_);boundary.checkpoint=checkpoint;const auto error=create_all(p,boundary);
    if(error)fail("create_directories",error);
}
std::uint32_t Filesystem::create_directories_error(const Path& p){
    Boundary boundary(api_);return create_all(p,boundary);
}
std::vector<Path> Filesystem::directory_entries(const Path& p){
    std::vector<Path> result;
    visit_directory_entries(p,{&result,[](void* context,const Path& path){
        static_cast<std::vector<Path>*>(context)->push_back(path);
    }});
    return result;
}
void Filesystem::visit_directory_entries(const Path& p,configuration::DirectoryVisitor visitor){
    if(!visitor.visit)throw std::invalid_argument("Directory visitor callback required");
    Boundary boundary(api_);
    // 017a40 explicitly compares strlen to stored length before opening.
    if(p.empty()||p.find(u'\0')!=Path::npos)fail("directory_iterator::directory_iterator",ERROR_FILE_NOT_FOUND);
    const auto pattern=wide(path_recovered::join_016b00(p,u"*"));WIN32_FIND_DATAW data{};
    const auto handle=acquire(boundary,[&]{return api_.find_first_ex(pattern.c_str(),FindExInfoBasic,&data,FindExSearchNameMatch,nullptr,0);});
    if(handle==INVALID_HANDLE_VALUE){
        if(boundary.value==ERROR_NO_MORE_FILES)return;
        fail("directory_iterator::directory_iterator",boundary.value);
    }
    OwnedHandle owner{boundary,handle,true};bool visited=false;
    for(;;){
        if(!dot(data)){
            const auto entry=path_recovered::join_016b00(p,filename(data));
            boundary.visit(visitor,entry);visited=true;
        }
        if(boundary.call([&]{return api_.find_next(handle,&data);})!=FALSE)continue;
        if(boundary.value==ERROR_NO_MORE_FILES)return;
        fail(visited?"directory_iterator::operator++":"directory_iterator::directory_iterator",boundary.value);
    }
}
void Filesystem::remove(const Path& p){
    Boundary boundary(api_);const auto w=wide(p);bool full_access=true;
    auto handle=acquire(boundary,[&]{return api_.create_file(w.c_str(),0x10180,7,nullptr,OPEN_EXISTING,0x02200000,nullptr);});
    if(handle==INVALID_HANDLE_VALUE&&boundary.value==ERROR_ACCESS_DENIED){
        full_access=false;
        handle=acquire(boundary,[&]{return api_.create_file(w.c_str(),0x10000,7,nullptr,OPEN_EXISTING,0x02200000,nullptr);});
    }
    if(handle==INVALID_HANDLE_VALUE){
        const auto error=boundary.value;
        // Native first-open missing errors are normal false/remove-no-op.
        // Missing result after access-denied retry is not normalized.
        if(full_access&&ignorable_prefix(error))return;
        fail("remove",error);
    }
    DWORD error;
    {OwnedHandle owner{boundary,handle,false};error=remove_handle(handle,full_access,boundary);}
    if(error)fail("remove",error);
}
ConfigurationFilesystem::ConfigurationFilesystem(std::shared_ptr<Filesystem> fs,std::unique_ptr<configuration::Dependencies> lower)
    :filesystem_(std::move(fs)),lower_(std::move(lower)){
    if(!filesystem_||!lower_)throw std::invalid_argument("Filesystem and remaining configuration owner required");
}
Path ConfigurationFilesystem::executable_path_03c6d0(){return lower_->executable_path_03c6d0();}
Path ConfigurationFilesystem::parent_path_019820(const Path& p){return lower_->parent_path_019820(p);}
Path ConfigurationFilesystem::join_016b00(const Path& a,const Path& b){return lower_->join_016b00(a,b);}
Path ConfigurationFilesystem::widen_0145d0(std::string_view s){return lower_->widen_0145d0(s);}
Path ConfigurationFilesystem::filesystem_widen_010540(std::string_view s){return lower_->filesystem_widen_010540(s);}
std::string ConfigurationFilesystem::narrow_path(const Path& p){return lower_->narrow_path(p);}
Path ConfigurationFilesystem::resolve_path_0192d0(std::string_view s,const Path& p,bool r){return lower_->resolve_path_0192d0(s,p,r);}
Path ConfigurationFilesystem::program_data_03c920(){return lower_->program_data_03c920();}
Path ConfigurationFilesystem::local_app_data_03c6f0(){return lower_->local_app_data_03c6f0();}
configuration::Node ConfigurationFilesystem::load_document(const Path& p){return lower_->load_document(p);}
void ConfigurationFilesystem::log_config_path(const Path& p){lower_->log_config_path(p);}
bool ConfigurationFilesystem::crt_is_space(unsigned char c){return lower_->crt_is_space(c);}
int ConfigurationFilesystem::crt_tolower(int c){return lower_->crt_tolower(c);}
void ConfigurationFilesystem::apply_license_key_02a830(std::string_view s){lower_->apply_license_key_02a830(s);}
void ConfigurationFilesystem::create_directories_018e00(const Path& p){filesystem_->create_directories(p);}
void ConfigurationFilesystem::write_embedded_png_03c020(const Path& p,configuration::EmbeddedPng png){lower_->write_embedded_png_03c020(p,png);}
std::vector<Path> ConfigurationFilesystem::directory_entries(const Path& p){return filesystem_->directory_entries(p);}
void ConfigurationFilesystem::visit_directory_entries(const Path& p,configuration::DirectoryVisitor visitor){filesystem_->visit_directory_entries(p,visitor);}
Path ConfigurationFilesystem::filename(const Path& p){return lower_->filename(p);}
void ConfigurationFilesystem::remove_04948c(const Path& p){filesystem_->remove(p);}
void ConfigurationFilesystem::update_achievement_storage_01eb00(configuration::State& s){lower_->update_achievement_storage_01eb00(s);}
bool ConfigurationFilesystem::set_environment_variable(std::string_view n,std::string_view v){return lower_->set_environment_variable(n,v);}
static_assert(!std::is_abstract_v<Filesystem>&&!std::is_abstract_v<ConfigurationFilesystem>);
static_assert(sizeof(FILE_BASIC_INFO)==0x28);
} // namespace anadius::config_storage
