#pragma once
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/factory.hpp"
#include "thread_error.hpp"
#include <array>
#include <limits>
#include <set>

namespace b9_test {
using namespace anadius;
using Path=configuration::Path;
namespace cs=config_storage;
inline unsigned checks=0;
inline void require(bool x,const char* why){++checks;if(!x)throw std::runtime_error(why);}
template<class E,class F>void rejects(F&& f,const char* why){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,why);}
inline unsigned total(){return checks+b8_test::checks+b7_test::count();}
struct Plan {std::vector<std::wstring> names;DWORD error=ERROR_NO_MORE_FILES;};
struct Cursor {Path pattern;Plan plan;std::size_t next=1;};
struct Sdk {
    b8_test::FsSdk base;
    std::map<Path,Plan> plans;
    std::map<std::uintptr_t,Cursor> cursors;
    std::vector<std::string> events;
    std::vector<DWORD> next_incoming;
    std::vector<Path> removed;
    std::uintptr_t next_handle=0x900;
    unsigned firsts=0,nexts=0,closes=0;
    Path opened;
};
inline Sdk* sdk=nullptr;
inline std::string label(const Path& p){return {p.begin(),p.end()};}
inline void fill(WIN32_FIND_DATAW& output,const std::wstring& name){
    if(name.size()>=std::size(output.cFileName))throw std::logic_error("synthetic filename exceeds SDK capacity");
    output={};std::copy(name.begin(),name.end(),output.cFileName);
}
inline HANDLE WINAPI first_ex(LPCWSTR pattern,FINDEX_INFO_LEVELS level,LPVOID output,FINDEX_SEARCH_OPS search,LPVOID filter,DWORD flags){
    auto& s=*sdk;
    require(level==FindExInfoBasic&&search==FindExSearchNameMatch&&!filter&&!flags,"stream preserves typed SDK enumeration operands");
    const Path p(pattern,pattern+std::wcslen(pattern));s.events.push_back("first:"+label(p));++s.firsts;
    const auto found=s.plans.find(p);if(found==s.plans.end())throw std::logic_error("unplanned synthetic enumeration");
    if(found->second.names.empty()){::SetLastError(ERROR_NO_MORE_FILES);return INVALID_HANDLE_VALUE;}
    const auto handle=s.next_handle++;s.cursors.emplace(handle,Cursor{p,found->second});
    fill(*static_cast<WIN32_FIND_DATAW*>(output),found->second.names.front());::SetLastError(0x111);
    return reinterpret_cast<HANDLE>(handle);
}
inline BOOL WINAPI next(HANDLE handle,LPWIN32_FIND_DATAW output){
    auto& s=*sdk;const auto h=reinterpret_cast<std::uintptr_t>(handle);
    auto found=s.cursors.find(h);require(found!=s.cursors.end(),"increment uses a live per-enumeration handle");
    s.next_incoming.push_back(::GetLastError());++s.nexts;s.events.push_back("next:"+label(found->second.pattern));
    auto& cursor=found->second;
    if(cursor.next==cursor.plan.names.size()){::SetLastError(cursor.plan.error);return FALSE;}
    fill(*output,cursor.plan.names[cursor.next++]);::SetLastError(0x222);return TRUE;
}
inline BOOL WINAPI find_close(HANDLE handle){
    auto& s=*sdk;const auto found=s.cursors.find(reinterpret_cast<std::uintptr_t>(handle));
    require(found!=s.cursors.end(),"close owns exactly one live enumeration");
    s.events.push_back("close:"+label(found->second.pattern));s.cursors.erase(found);++s.closes;::SetLastError(0x333);return TRUE;
}
inline HANDLE WINAPI open(LPCWSTR p,DWORD a,DWORD share,LPSECURITY_ATTRIBUTES security,DWORD disposition,DWORD flags,HANDLE prototype){
    sdk->opened=Path(p,p+std::wcslen(p));sdk->events.push_back("remove:"+label(sdk->opened));
    return b8_test::fs_open(p,a,share,security,disposition,flags,prototype);
}
inline BOOL WINAPI set(HANDLE h,FILE_INFO_BY_HANDLE_CLASS level,LPVOID value,DWORD bytes){
    const auto result=b8_test::fs_set(h,level,value,bytes);
    if(result!=FALSE&&level!=FileBasicInfo)sdk->removed.push_back(sdk->opened);
    return result;
}
inline cs::FilesystemApi api(){
    auto a=b8_test::fs_api();a.find_first_ex=first_ex;a.find_next=next;a.find_close=find_close;a.create_file=open;a.set_info=set;return a;
}
struct SdkScope {
    Sdk state;std::shared_ptr<cs::Filesystem> fs;
    SdkScope(){sdk=&state;b8_test::fs_sdk=&state.base;fs=std::make_shared<cs::Filesystem>(api());}
    ~SdkScope(){fs.reset();sdk=nullptr;b8_test::fs_sdk=nullptr;}
    void plan(const Path& dir,std::vector<std::wstring> names,DWORD error=ERROR_NO_MORE_FILES){state.plans[path_recovered::join_016b00(dir,u"*")]={std::move(names),error};}
};
enum class Failure {none,filename,narrow,remove};
class Policies:public configuration::Dependencies {
public:
    std::shared_ptr<b8_test::LowerState> state=std::make_shared<b8_test::LowerState>();
    b8_test::Lower lower{state};
    std::shared_ptr<cs::Filesystem> fs;
    Failure failure=Failure::none;
    std::vector<Path> vector_entries;
    std::vector<std::string> trace;
    unsigned vectors=0,visits=0,narrows=0,filenames=0,removes=0;
    explicit Policies(std::shared_ptr<cs::Filesystem> f={}):fs(std::move(f)){}
    Path executable_path_03c6d0()override{return lower.executable_path_03c6d0();}
    Path parent_path_019820(const Path& p)override{return lower.parent_path_019820(p);}
    Path join_016b00(const Path& p,const Path& q)override{return lower.join_016b00(p,q);}
    Path widen_0145d0(std::string_view s)override{return lower.widen_0145d0(s);}
    Path filesystem_widen_010540(std::string_view s)override{return lower.filesystem_widen_010540(s);}
    std::string narrow_path(const Path& p)override{++narrows;trace.emplace_back("narrow");if(failure==Failure::narrow)throw std::runtime_error("narrow fixture failure");return lower.narrow_path(p);}
    Path resolve_path_0192d0(std::string_view s,const Path& p,bool b)override{return lower.resolve_path_0192d0(s,p,b);}
    Path program_data_03c920()override{return lower.program_data_03c920();}
    Path local_app_data_03c6f0()override{return lower.local_app_data_03c6f0();}
    configuration::Node load_document(const Path& p)override{return lower.load_document(p);}
    void log_config_path(const Path& p)override{lower.log_config_path(p);}
    bool crt_is_space(unsigned char c)override{return lower.crt_is_space(c);}
    int crt_tolower(int c)override{return lower.crt_tolower(c);}
    void apply_license_key_02a830(std::string_view s)override{lower.apply_license_key_02a830(s);}
    void create_directories_018e00(const Path& p)override{lower.create_directories_018e00(p);}
    void write_embedded_png_03c020(const Path& p,configuration::EmbeddedPng i)override{lower.write_embedded_png_03c020(p,i);}
    std::vector<Path> directory_entries(const Path&)override{++vectors;return vector_entries;}
    void visit_directory_entries(const Path& p,configuration::DirectoryVisitor v)override{
        ++visits;if(fs)fs->visit_directory_entries(p,v);else configuration::Dependencies::visit_directory_entries(p,v);
    }
    Path filename(const Path& p)override{++filenames;trace.emplace_back("filename");if(failure==Failure::filename)throw std::runtime_error("filename fixture failure");return lower.filename(p);}
    void remove_04948c(const Path& p)override{++removes;trace.emplace_back("remove");if(failure==Failure::remove)throw std::runtime_error("remove fixture failure");if(fs)fs->remove(p);else lower.remove_04948c(p);}
    void update_achievement_storage_01eb00(configuration::State& s)override{lower.update_achievement_storage_01eb00(s);}
    bool set_environment_variable(std::string_view k,std::string_view v)override{return lower.set_environment_variable(k,v);}
};
struct Pass final:configuration::ConfigurationPass {
    unsigned calls=0;
    void load_and_apply(configuration::State&,bool,configuration::Dependencies&)override{++calls;}
};
inline std::size_t position(const std::vector<std::string>& events,const std::string& value){
    const auto found=std::find(events.begin(),events.end(),value);if(found==events.end())throw std::runtime_error("expected event absent: "+value);return static_cast<std::size_t>(found-events.begin());
}
inline configuration::State seed(){configuration::State s{};s.sdk="1.2.3";s.game_attributes["INSTALLED_VERSION"]="source-version";return s;}
inline Path user_directory(){return u"LOCAL\\anadius\\LSX emu";}
inline std::filesystem::path own_root(int argc,char** argv){
    namespace f=std::filesystem;
    const auto build=f::weakly_canonical(f::path(__FILE__).parent_path().parent_path()/"source-workspace"/"build");
    const auto expected=(build/"b9-fixtures").lexically_normal();
    const auto requested=f::weakly_canonical(argc>1?f::path(argv[1]):expected);
    if(!b8_test::same_windows_path(requested,expected))throw std::invalid_argument("B9 fixtures must resolve inside workspace source-workspace/build/b9-fixtures");
    f::create_directories(requested);
    const auto child=requested/("iteration-"+std::to_string(::GetCurrentProcessId())+"-"+std::to_string(::GetTickCount64()));
    if(!f::create_directory(child))throw std::runtime_error("B9 run directory must be fresh");
    b8_test::fixture_root=f::weakly_canonical(child);return b8_test::fixture_root;
}
}
