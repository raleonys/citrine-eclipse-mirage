// SPDX-License-Identifier: GPL-3.0-only

#include "../../src/runtime/application_internal.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
namespace dialog {
std::wstring body,title;unsigned calls=0;
int WINAPI show(HWND,LPCWSTR text,LPCWSTR caption,UINT flags){
    if(flags!=MB_OK)std::terminate();body=text;title=caption;++calls;return IDOK;
}
}
namespace environment_path_fixture {
enum class Mode {value,empty,missing,grow_then_missing};
Mode mode=Mode::value;
std::wstring expected,name;
std::vector<DWORD> capacities;
void require(bool value,const char* message) {
    if(!value)throw std::runtime_error(message);
}
void select(std::wstring variable,Mode next,std::wstring value={}) {
    name=std::move(variable);mode=next;expected=std::move(value);capacities.clear();
}
DWORD WINAPI read(LPCWSTR variable,LPWSTR output,DWORD capacity) {
    require(variable&&std::wstring_view(variable)==name,"environment path selects the exact variable name");
    require(output!=nullptr,"environment path owns its native output buffer");
    capacities.push_back(capacity);
    require(capacities.size()<=4,"environment path makes bounded progress for supplied results");
    require(capacity==260u*capacities.size(),"environment path grows in 260 UTF16 code unit steps");
    if(mode==Mode::missing||(mode==Mode::grow_then_missing&&capacities.size()==2)) {
        SetLastError(ERROR_ENVVAR_NOT_FOUND);return 0;
    }
    if(mode==Mode::empty) {SetLastError(ERROR_SUCCESS);return 0;}
    if(mode==Mode::grow_then_missing||expected.size()>=capacity) {
        std::fill_n(output,capacity,L'?');
        // An insufficient environment buffer reports required size INCLUDING NUL.
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return mode==Mode::grow_then_missing?781u:static_cast<DWORD>(expected.size()+1);
    }
    std::fill_n(output,capacity,L'!');
    std::copy(expected.begin(),expected.end(),output);output[expected.size()]=L'\0';
    SetLastError(ERROR_ACCESS_DENIED); // Success does not require cleared last error.
    return static_cast<DWORD>(expected.size()); // Successful count EXCLUDES NUL.
}
void check(preloader_l::application::detail::Environment& env,preloader_l::application::Options& options) {
    using preloader_l::configuration::Path;
    options.program_data=u"D:\\configured\\\u00e9-data";
    select(L"ProgramData",Mode::missing);
    require(env.program_data_03c920()==options.program_data&&capacities.empty(),
        "configured ProgramData path bypasses the native environment query");
    options.program_data.clear();
    for(const auto* variable:{L"ProgramData",L"LocalAppData"}) {
        const auto path=[&]{return std::wstring_view(variable)==L"ProgramData"?env.program_data_03c920():env.local_app_data_03c6f0();};
        for(const auto length:{259u,260u,779u,780u}) {
            const std::wstring value(length,L'\u00e9');select(variable,Mode::value,value);
            require(path()==Path(value.begin(),value.end()),"environment path returns exactly the counted UTF16 value");
            const auto calls=length/260u+1;
            require(capacities.size()==calls,"capacity minus one succeeds; equal length requires another buffer");
        }
        for(const auto selected:{Mode::empty,Mode::missing,Mode::grow_then_missing}) {
            select(variable,selected);
            require(path().empty(),"zero native count yields empty under the existing missing/empty-variable policy");
            require(capacities.size()==(selected==Mode::grow_then_missing?2u:1u),"zero native count stops lookup immediately");
            select(variable,Mode::value,L"C:\\retry");
            require(path()==u"C:\\retry"&&capacities==std::vector<DWORD>{260},"retry starts with fresh storage after empty or missing result");
        }
    }
    std::cout<<"PASS: actual Environment environment-path counted growth, zero result, retry and override contracts\n";
}
}

#define GetEnvironmentVariableW environment_path_fixture::read
#define MessageBoxW dialog::show
#include "../../src/runtime/application_environment.cpp"
#undef MessageBoxW
#undef GetEnvironmentVariableW

namespace logging_fixture {
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
std::string prefix(unsigned worker,unsigned row,unsigned phase){
    return "phase-"+std::to_string(phase)+"/worker-"+std::to_string(worker)+"/row-"+std::to_string(row)+":";
}
std::string payload(){return std::string("left\0right",10);}
std::string expected(unsigned worker,unsigned row,unsigned phase){
    const auto id=prefix(worker,row,phase);
    switch(worker){
    case 0:case 1:return id+payload();
    case 2:case 3:return id+std::to_string(row);
    case 4:return "profile requested: "+id+payload();
    default:return "config loaded: C:/fixture/"+id+"file.cfg";
    }
}
void write(preloader_l::application::detail::Environment& env,unsigned worker,unsigned row,unsigned phase){
    const auto id=prefix(worker,row,phase),bytes=payload();
    switch(worker){
    case 0:env.log({id,bytes});break;
    case 1:env.debug_log({id,bytes});break;
    case 2:env.log_number(id,row);break;
    case 3:env.debug_log_number(id,row);break;
    case 4:env.log_profile_name_084f48(id+bytes);break;
    default:{const auto path="C:/fixture/"+id+"file.cfg";env.log_config_path({path.begin(),path.end()});break;}
    }
}
void concurrent_phase(preloader_l::application::detail::Environment& env,unsigned phase){
    std::mutex gate;std::exception_ptr failure;
    {
        std::vector<std::jthread> workers;
        for(unsigned worker=0;worker<6;++worker)workers.emplace_back([&,worker]{
            try{for(unsigned row=0;row<30;++row)write(env,worker,row,phase);}
            catch(...){std::lock_guard lock(gate);if(!failure)failure=std::current_exception();}
        });
    }
    if(failure)std::rethrow_exception(failure);
}
void actual_environment_logging(preloader_l::application::detail::Environment& env,preloader_l::configuration::State& state){
    namespace fs=std::filesystem;
    const auto directory=fs::temp_directory_path()/("surge-log-fixture-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    require(fs::create_directory(directory),"own fresh logging fixture directory");
    struct Cleanup{fs::path path;~Cleanup(){std::error_code error;fs::remove_all(path,error);}} cleanup{directory};
    const auto file=directory/"surge.log";
    concurrent_phase(env,0); // Real startup queue: no configured data directory.
    state.user_directory=directory.u16string();
    env.log(std::initializer_list<std::string_view>{}); // Flush startup records once.
    concurrent_phase(env,1);
    env.log(std::initializer_list<std::string_view>{}); // Must not repeat startup records.
#ifndef NDEBUG
    std::ifstream input(file,std::ios::binary);require(static_cast<bool>(input),"Debug diagnostics created the owned log");
    std::multiset<std::string> actual,wanted;std::string line;
    while(std::getline(input,line))actual.insert(line);
    require(input.eof(),"all counted log bytes can be read back");
    for(unsigned phase=0;phase<2;++phase)for(unsigned worker=0;worker<6;++worker)for(unsigned row=0;row<30;++row)
        wanted.insert(expected(worker,row,phase));
    require(actual==wanted,"six actual writers preserve all 360 complete records, embedded NULs, formatting and startup flush exactly once");
    std::cout<<"PASS: Debug Environment startup flush and six concurrent writers preserve 360 counted records\n";
#else
    require(!fs::exists(file),"Release diagnostics must never create surge.log through direct or inline entrypoints");
    std::cout<<"PASS: Release Environment direct/inline diagnostics never create surge.log\n";
#endif
}

void failed_open_preserves_startup(preloader_l::application::detail::Environment& env,
                                   preloader_l::configuration::State& state) {
    namespace fs=std::filesystem;
    const auto directory=fs::temp_directory_path()/("surge-log-open-retry-"+
        std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    require(fs::create_directory(directory),"create isolated Debug log recovery directory");
    struct Cleanup{fs::path path;~Cleanup(){std::error_code error;fs::remove_all(path,error);}} cleanup{directory};
    const auto sink=directory/"surge.log";
    state.user_directory.clear();
    env.log(std::string_view("buffered-before-open-1"));
    env.log(std::string_view("buffered-before-open-2"));
    require(fs::create_directory(sink),"block the logging file with a directory");
    state.user_directory=directory.u16string();
    env.log(std::initializer_list<std::string_view>{}); // Attempt the first flush: file open fails.
    require(fs::is_directory(sink),"the blocked sink must still be a directory");
    require(fs::remove(sink),"release the blocked sink without touching unrelated paths");
    env.log(std::string_view("record-after-open-recovery"));
    env.log(std::initializer_list<std::string_view>{}); // Successful flush must not repeat buffered records.
#ifndef NDEBUG
    std::ifstream input(sink,std::ios::binary);
    require(static_cast<bool>(input),"the recovered Debug log must exist");
    const std::string bytes((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
    require(bytes=="buffered-before-open-1\nbuffered-before-open-2\nrecord-after-open-recovery\n",
            "failed opening must not permanently discard startup diagnostic records");
    std::cout<<"PASS: Debug open failure retains buffered records through successful retry\n";
#else
    require(!fs::exists(sink),"Release must never create a log after a failed Debug-only sink");
    std::cout<<"PASS: Release logging remains disabled across a blocked sink\n";
#endif
}

void unknown_entitlement_is_nonmodal_and_redacted(preloader_l::application::detail::Environment& env,
                                                  preloader_l::configuration::State& state) {
    namespace fs=std::filesystem;
    const auto directory=fs::temp_directory_path()/("surge-unknown-field-fixture-"+
        std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    require(fs::create_directory(directory),"own isolated unknown field fixture directory");
    struct Cleanup{fs::path path;~Cleanup(){std::error_code error;fs::remove_all(path,error);}} cleanup{directory};
    state.user_directory=directory.u16string();
    const auto before=dialog::calls;
    env.unknown_entitlement_field("Unknown|sensitive-value\nforged diagnostic");
    const std::string oversized(32768,'S');
    env.unknown_entitlement_field(oversized);
    require(dialog::calls==before,"caller-controlled entitlement diagnostics must never show a modal");
#ifndef NDEBUG
    const auto file=directory/"surge.log";
    std::ifstream input(file,std::ios::binary);
    require(static_cast<bool>(input),"Debug diagnostic must create its owned log");
    const std::string output((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());
    require(output=="QueryEntitlements ignored unknown field\nQueryEntitlements ignored unknown field\n",
            "Debug diagnostics must be bounded and must not echo unknown request bytes");
    require(output.find("sensitive-value")==std::string::npos,
            "Debug diagnostic must not contain request secrets");
    std::cout<<"PASS: nonmodal bounded unknown entitlement diagnostics preserve Debug redaction\n";
#else
    const auto file=directory/"surge.log";
    require(!fs::exists(file),"Release unknown field must not create a log file");
    const std::string existing="pre-existing Debug log\n";
    {
        std::ofstream output(file,std::ios::binary);
        output<<existing;output.close();
        require(!output.fail(),"seed the existing log fixture");
    }
    env.unknown_entitlement_field("Unknown|must-not-append");
    std::ifstream input(file,std::ios::binary);
    require(static_cast<bool>(input),"read the existing log fixture");
    const std::string output((std::istreambuf_iterator<char>(input)),std::istreambuf_iterator<char>());
    require(output==existing,"Release unknown field must not append to an existing Debug log");
    require(dialog::calls==before,"Release unknown field must remain nonmodal with an existing log");
    std::cout<<"PASS: nonmodal unknown entitlement diagnostics keep Release logging disabled\n";
#endif
}
}

int main()try{
    using namespace preloader_l;
    configuration::State state{};application::Options options;
    file::NativeFactory files;xml_reader_bridge::Backend xml;achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    environment_path_fixture::check(env,options);
    env.unknown_entitlement_field("QueryEntitlements unknown attr: unexpected|value");
    if(dialog::calls!=0)
        throw std::runtime_error("unknown entitlement input must not invoke a blocking dialog");
    configuration::State logging_state{};
    application::detail::Environment logging_env{logging_state,options,files,xml,paths,clock,formatter};
    logging_fixture::actual_environment_logging(logging_env,logging_state);
    configuration::State recovery_state{};
    application::detail::Environment recovery_env{recovery_state,options,files,xml,paths,clock,formatter};
    logging_fixture::failed_open_preserves_startup(recovery_env,recovery_state);
    configuration::State unknown_state{};
    application::detail::Environment unknown_env{unknown_state,options,files,xml,paths,clock,formatter};
    logging_fixture::unknown_entitlement_is_nonmodal_and_redacted(unknown_env,unknown_state);
    std::cout<<"PASS: actual Environment unknown field diagnostics are nonmodal and redacted\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
