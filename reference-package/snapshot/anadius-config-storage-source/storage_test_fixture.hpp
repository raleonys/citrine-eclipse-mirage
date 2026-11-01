#pragma once
#include "filesystem.hpp"
#include "persistence.hpp"
#include "../anadius-bootstrap-policies-source/config_platform.hpp"
#include "../anadius-config-startup-source/startup.hpp"
#include "../anadius-path-source/paths.hpp"
#include <algorithm>
#include <atomic>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <span>
#include <stdexcept>
#include <system_error>

namespace b8_test {
using namespace anadius;
namespace cs=config_storage;
using Path=configuration::Path;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
inline Path ascii(std::string_view s){return {s.begin(),s.end()};}
inline std::string narrow_ascii(const Path& p){return {p.begin(),p.end()};}
struct LowerState {
    Path executable=u"C:\\fixture\\host.exe",program=u"PROGRAM",local=u"LOCAL";
    std::vector<std::string> events;
    std::vector<std::pair<std::string,std::string>> environment;
    unsigned pngs=0,persistence=0,creates=0,enumerations=0,removes=0;
};
class Lower final:public configuration::Dependencies {
    std::shared_ptr<LowerState> s_;
public:
    explicit Lower(std::shared_ptr<LowerState> s):s_(std::move(s)){}
    Path executable_path_03c6d0()override{return s_->executable;}
    Path parent_path_019820(const Path& p)override{return path_recovered::parent_path_019820(p);}
    Path join_016b00(const Path& a,const Path& b)override{return path_recovered::join_016b00(a,b);}
    Path widen_0145d0(std::string_view p)override{return ascii(p);}
    Path filesystem_widen_010540(std::string_view p)override{return ascii(p);}
    std::string narrow_path(const Path& p)override{return narrow_ascii(p);}
    Path resolve_path_0192d0(std::string_view p,const Path& executable,bool relative)override{
        auto value=ascii(p);return relative&&!path_recovered::is_absolute_0195b0(value)?path_recovered::join_016b00(parent_path_019820(executable),value):value;
    }
    Path program_data_03c920()override{return s_->program;}
    Path local_app_data_03c6f0()override{return s_->local;}
    configuration::Node load_document(const Path&)override{throw std::logic_error("source FilePass is required");}
    void log_config_path(const Path& p)override{s_->events.push_back(p.ends_with(u"anadius_override.cfg")?"log-override":"log-base");}
    bool crt_is_space(unsigned char c)override{return c==' '||(c>='\t'&&c<='\r');}
    int crt_tolower(int c)override{return c>='A'&&c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("no licensing policy selected");}
    void create_directories_018e00(const Path&)override{++s_->creates;s_->events.emplace_back("lower-mkdir");}
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng)override{++s_->pngs;s_->events.emplace_back("lower-png");}
    std::vector<Path> directory_entries(const Path&)override{++s_->enumerations;s_->events.emplace_back("lower-enumerate");return {};}
    Path filename(const Path& p)override{return path_recovered::filename_00e120(p);}
    void remove_04948c(const Path&)override{++s_->removes;s_->events.emplace_back("lower-remove");}
    void update_achievement_storage_01eb00(configuration::State&)override{++s_->persistence;s_->events.emplace_back("lower-persistence");}
    bool set_environment_variable(std::string_view k,std::string_view v)override{s_->environment.emplace_back(k,v);s_->events.emplace_back("environment");return false;}
};
struct MemoryFilesState {
    std::map<Path,std::string> inputs,outputs;
    std::vector<Path> reads,writes;
    std::vector<std::string> events;
    bool null_input=false,null_output=false,bad_output=false,throw_write=false,throw_close=false;
};
class MemoryInput final:public file_recovered::Input {
    std::string bytes_;bool exists_;std::uint64_t position_=0;
public:
    MemoryInput(std::string bytes,bool exists):bytes_(std::move(bytes)),exists_(exists){}
    std::uint32_t initial_state()const override{return exists_?0:2;}
    void seek_03b610(std::int64_t offset,file_recovered::Seek mode)override{require(offset==0,"source fixture seeks zero");position_=mode==file_recovered::Seek::end?bytes_.size():0;}
    std::uint64_t position_sum_03ba50()override{return position_;}
    std::int32_t peek_043790()override{throw std::logic_error("binary source does not peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> target)override{const auto count=std::min(target.size(),bytes_.size()-static_cast<std::size_t>(position_));std::copy_n(bytes_.data()+position_,count,target.data());position_+=count;return count;}
    void close_03ae70()override{}
};
class MemoryOutput final:public file_recovered::Output {
    std::shared_ptr<MemoryFilesState> s_;Path path_;
public:
    MemoryOutput(std::shared_ptr<MemoryFilesState> s,Path p):s_(std::move(s)),path_(std::move(p)){}
    std::uint32_t initial_state()const override{return s_->bad_output?2:0;}
    void write_03be80(std::string_view bytes)override{s_->events.emplace_back("write");if(s_->throw_write)throw std::runtime_error("injected write");s_->outputs[path_].assign(bytes);}
    void close_03ae70()override{s_->events.emplace_back("close");if(s_->throw_close)throw std::runtime_error("injected close");}
};
class MemoryFiles final:public file_recovered::Factory {
    std::shared_ptr<MemoryFilesState> s_;
public:
    explicit MemoryFiles(std::shared_ptr<MemoryFilesState> s):s_(std::move(s)){}
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& p,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20&&share==0x40,"source input preserves binary/default-share flags");s_->reads.push_back(p);if(s_->null_input)return {};
        const auto found=s_->inputs.find(p);return std::make_unique<MemoryInput>(found==s_->inputs.end()?std::string{}:found->second,found!=s_->inputs.end());
    }
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20&&share==0x40,"source output preserves binary/default-share flags");s_->writes.push_back(p);s_->events.emplace_back("open");if(s_->null_output)return {};
        return std::make_unique<MemoryOutput>(s_,p);
    }
};
struct FsSdk {
    std::vector<Path> creates,firsts,enumerations,opens;
    std::vector<DWORD> accesses,shares,dispositions,open_flags;
    std::vector<FILE_INFO_BY_HANDLE_CLASS> get_classes,set_classes;
    std::vector<DWORD> set_sizes;
    std::vector<std::vector<unsigned char>> set_values;
    std::deque<DWORD> create_errors,open_errors,set_errors;
    std::vector<std::wstring> names{L"z",L".",L"..",L"a"};
    std::size_t name_index=0;
    unsigned nexts=0,find_closes=0,handle_closes=0,attribute_calls=0;
    DWORD attrs=FILE_ATTRIBUTE_DIRECTORY,attribute_error=0,first_error=0,enum_error=0,next_error=ERROR_NO_MORE_FILES,get_error=0;
    bool enum_operands=true,close_ok=true,find_close_ok=true;
};
inline FsSdk* fs_sdk=nullptr;
inline BOOL WINAPI fs_create(LPCWSTR path,LPSECURITY_ATTRIBUTES security){
    auto& s=*fs_sdk;require(security==nullptr,"directory create has no invented security policy");s.creates.emplace_back(path,path+std::wcslen(path));
    const DWORD error=s.create_errors.empty()?0:s.create_errors.front();if(!s.create_errors.empty())s.create_errors.pop_front();::SetLastError(error);return error?FALSE:TRUE;
}
inline BOOL WINAPI fs_attributes(LPCWSTR,GET_FILEEX_INFO_LEVELS level,LPVOID output){
    auto& s=*fs_sdk;++s.attribute_calls;require(level==GetFileExInfoStandard&&output,"status attribute query uses exact SDK record");::SetLastError(s.attribute_error);
    if(s.attribute_error)return FALSE;auto& data=*static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(output);data={};data.dwFileAttributes=s.attrs;return TRUE;
}
inline HANDLE WINAPI fs_first(LPCWSTR path,LPWIN32_FIND_DATAW output){
    auto& s=*fs_sdk;s.firsts.emplace_back(path,path+std::wcslen(path));::SetLastError(s.first_error);
    if(s.first_error)return INVALID_HANDLE_VALUE;*output={};output->dwFileAttributes=s.attrs;return reinterpret_cast<HANDLE>(0x111);
}
inline HANDLE WINAPI fs_first_ex(LPCWSTR path,FINDEX_INFO_LEVELS level,LPVOID output,FINDEX_SEARCH_OPS search,LPVOID filter,DWORD flags){
    auto& s=*fs_sdk;s.enumerations.emplace_back(path,path+std::wcslen(path));s.enum_operands=s.enum_operands&&level==FindExInfoBasic&&search==FindExSearchNameMatch&&!filter&&!flags;
    ::SetLastError(s.enum_error);if(s.enum_error)return INVALID_HANDLE_VALUE;
    if(s.names.empty()){::SetLastError(ERROR_NO_MORE_FILES);return INVALID_HANDLE_VALUE;}
    auto& data=*static_cast<WIN32_FIND_DATAW*>(output);data={};std::copy(s.names.front().begin(),s.names.front().end(),data.cFileName);s.name_index=1;return reinterpret_cast<HANDLE>(0x111);
}
inline BOOL WINAPI fs_next(HANDLE handle,LPWIN32_FIND_DATAW output){
    auto& s=*fs_sdk;require(handle==reinterpret_cast<HANDLE>(0x111),"directory iteration retains its own find handle");++s.nexts;
    if(s.name_index==s.names.size()){::SetLastError(s.next_error);return FALSE;}
    *output={};const auto& name=s.names[s.name_index++];std::copy(name.begin(),name.end(),output->cFileName);return TRUE;
}
inline BOOL WINAPI fs_find_close(HANDLE handle){auto& s=*fs_sdk;require(handle==reinterpret_cast<HANDLE>(0x111),"only owned synthetic find handle closes");++s.find_closes;::SetLastError(0x777);return s.find_close_ok?TRUE:FALSE;}
inline HANDLE WINAPI fs_open(LPCWSTR path,DWORD access,DWORD share,LPSECURITY_ATTRIBUTES security,DWORD disposition,DWORD flags,HANDLE prototype){
    auto& s=*fs_sdk;s.opens.emplace_back(path,path+std::wcslen(path));s.accesses.push_back(access);s.shares.push_back(share);s.dispositions.push_back(disposition);s.open_flags.push_back(flags);
    require(!security&&!prototype,"filesystem open keeps null security/template operands");
    const DWORD error=s.open_errors.empty()?0:s.open_errors.front();if(!s.open_errors.empty())s.open_errors.pop_front();::SetLastError(error);return error?INVALID_HANDLE_VALUE:reinterpret_cast<HANDLE>(0x222);
}
inline BOOL WINAPI fs_get(HANDLE handle,FILE_INFO_BY_HANDLE_CLASS level,LPVOID output,DWORD bytes){
    auto& s=*fs_sdk;require(handle==reinterpret_cast<HANDLE>(0x222),"metadata uses owned synthetic file handle");s.get_classes.push_back(level);
    ::SetLastError(s.get_error);if(s.get_error)return FALSE;
    require(bytes==sizeof(FILE_BASIC_INFO),"file basic metadata has SDK size");auto& info=*static_cast<FILE_BASIC_INFO*>(output);info={};info.FileAttributes=s.attrs;return TRUE;
}
inline BOOL WINAPI fs_set(HANDLE handle,FILE_INFO_BY_HANDLE_CLASS level,LPVOID input,DWORD bytes){
    auto& s=*fs_sdk;require(handle==reinterpret_cast<HANDLE>(0x222)&&input,"disposition/attribute update uses owned live handle");s.set_classes.push_back(level);s.set_sizes.push_back(bytes);
    const auto* data=static_cast<unsigned char*>(input);s.set_values.emplace_back(data,data+bytes);
    const DWORD error=s.set_errors.empty()?0:s.set_errors.front();if(!s.set_errors.empty())s.set_errors.pop_front();::SetLastError(error);return error?FALSE:TRUE;
}
inline BOOL WINAPI fs_close(HANDLE handle){auto& s=*fs_sdk;require(handle==reinterpret_cast<HANDLE>(0x222),"only owned synthetic file handle closes");++s.handle_closes;::SetLastError(0x778);return s.close_ok?TRUE:FALSE;}
inline cs::FilesystemApi fs_api(){
    auto api=cs::native_filesystem_api();api.create_directory=fs_create;api.attributes_ex=fs_attributes;api.find_first=fs_first;api.find_first_ex=fs_first_ex;api.find_next=fs_next;api.find_close=fs_find_close;api.create_file=fs_open;api.get_info=fs_get;api.set_info=fs_set;api.close_handle=fs_close;return api;
}
struct StatsState {status_recovered::StatsResult value{ERROR_FILE_NOT_FOUND,0,{}};std::vector<Path> queried;};
class ProbeStats final:public status_recovered::Stats {
    std::shared_ptr<StatsState> s_;
public:
    explicit ProbeStats(std::shared_ptr<StatsState> s):s_(std::move(s)){}
    status_recovered::StatsResult query(const Path& p,std::uint32_t flags)override{require(flags==3,"achievement storage keeps bounded flags3 query");s_->queried.push_back(p);return s_->value;}
};
struct ParserState {std::vector<std::string> seen;std::string fail;};
class ProbeParser final:public timestamp_recovered::Parser {
    std::shared_ptr<ParserState> s_;timestamp_recovered::ClassicIso actual_;
public:
    explicit ProbeParser(std::shared_ptr<ParserState> s):s_(std::move(s)){}
    std::uint32_t extract_03d310(std::string_view text,std::string_view format,calendar_recovered::BrokenDown& output)override{
        s_->seen.emplace_back(text);if(text==s_->fail)throw std::runtime_error("injected timestamp failure");return actual_.extract_03d310(text,format,output);
    }
};
struct PersistenceFixture {
    FsSdk sdk;std::shared_ptr<cs::Filesystem> filesystem;
    std::shared_ptr<LowerState> lower=std::make_shared<LowerState>();
    std::shared_ptr<MemoryFilesState> files=std::make_shared<MemoryFilesState>();
    std::shared_ptr<StatsState> stats=std::make_shared<StatsState>();
    std::shared_ptr<ParserState> parser=std::make_shared<ParserState>();
    std::unique_ptr<cs::ConfigurationPersistence> policy;
    explicit PersistenceFixture(achievement_storage::Paths paths={}){
        fs_sdk=&sdk;filesystem=std::make_shared<cs::Filesystem>(fs_api());cs::PersistenceResources r;
        r.files=std::make_unique<MemoryFiles>(files);r.stats=std::make_unique<ProbeStats>(stats);r.parser=std::make_unique<ProbeParser>(parser);
        policy=std::make_unique<cs::ConfigurationPersistence>(filesystem,std::move(paths),std::make_unique<Lower>(lower),std::move(r));
    }
    ~PersistenceFixture(){policy.reset();filesystem.reset();fs_sdk=nullptr;}
};

// Fixtures are restricted to the resolved workspace build directory. Each run
// creates a new child; cleanup never enumerates or recursively deletes paths.
inline std::filesystem::path fixture_root;
inline bool same_windows_path(const std::filesystem::path& a,const std::filesystem::path& b){
    const auto x=a.wstring(),y=b.wstring();
    return ::CompareStringOrdinal(x.c_str(),static_cast<int>(x.size()),y.c_str(),static_cast<int>(y.size()),TRUE)==CSTR_EQUAL;
}
inline std::filesystem::path own_root(int argc,char** argv,const char* kind){
    namespace f=std::filesystem;
    const auto build=f::weakly_canonical(f::path(__FILE__).parent_path().parent_path()/"source-workspace"/"build");
    const auto expected=(build/"b8-fixtures").lexically_normal();
    const auto requested=f::weakly_canonical(argc>1?f::path(argv[1]):expected);
    if(!same_windows_path(requested,expected))
        throw std::invalid_argument("B8 fixture root must resolve to workspace source-workspace/build/b8-fixtures");
    f::create_directories(requested);
    const auto name=std::string(kind)+"-"+std::to_string(::GetCurrentProcessId())+"-"+std::to_string(::GetTickCount64());
    const auto child=requested/name;
    if(!f::create_directory(child))throw std::runtime_error("B8 fixture child must be fresh");
    fixture_root=f::weakly_canonical(child);
    return fixture_root;
}
inline void own_path(const std::filesystem::path& path){
    if(fixture_root.empty())throw std::logic_error("Fixture owner not initialized");
    auto current=std::filesystem::weakly_canonical(path);
    while(!current.empty()){
        if(same_windows_path(current,fixture_root))return;
        const auto parent=current.parent_path();if(parent==current)break;current=parent;
    }
    throw std::invalid_argument("Fixture IO resolved outside the owned run directory");
}
inline void write_own(const std::filesystem::path& path,std::string_view value){
    own_path(path);std::ofstream output(path,std::ios::binary|std::ios::trunc);
    if(!output)throw std::runtime_error("Own fixture output open failed");
    output.write(value.data(),static_cast<std::streamsize>(value.size()));output.close();
    if(!output)throw std::runtime_error("Own fixture output failed");
}
inline std::string read_own(const std::filesystem::path& path){
    own_path(path);std::ifstream input(path,std::ios::binary);
    if(!input)throw std::runtime_error("Own fixture input open failed");
    std::string value{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
    if(input.bad())throw std::runtime_error("Own fixture input failed");
    return value;
}
}
