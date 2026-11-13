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
#include <sstream>
#include <functional>
#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <cstdio>
namespace config_path_conversion {
struct Observation {
    const citrine_eclipse_mirage::configuration::Path& path;
    std::size_t utf8_bytes;
    unsigned fail_at=0,calls=0;
    DWORD error=ERROR_NOT_ENOUGH_MEMORY;
};
thread_local Observation* selected=nullptr;
struct Arm {
    explicit Arm(Observation& value){
        if(selected)throw std::logic_error("conversion observations must not overlap");
        selected=&value;
    }
    ~Arm(){selected=nullptr;}
    Arm(const Arm&)=delete;
    Arm& operator=(const Arm&)=delete;
};
int WINAPI convert(UINT page,DWORD flags,LPCWCH input,int count,LPSTR output,
                   int capacity,LPCCH default_character,LPBOOL used_default){
    if(selected){
        auto& value=*selected;++value.calls; // Observe every call, not just faults.
        if(page!=CP_UTF8||flags!=WC_ERR_INVALID_CHARS||!input||
           count!=static_cast<int>(value.path.size())||default_character||used_default||
           !std::equal(input,input+count,value.path.begin()))
            throw std::runtime_error("config path conversion changed its counted UTF16 operands");
        if(value.calls==1){
            if(output||capacity!=0)throw std::runtime_error("first path conversion must size output");
        }else if(value.calls==2){
            if(!output||capacity!=static_cast<int>(value.utf8_bytes))
                throw std::runtime_error("second path conversion must fill the exact UTF8 buffer");
        }else throw std::runtime_error("unexpected extra path conversion");
        if(value.calls==value.fail_at){SetLastError(value.error);return 0;}
    }
    return ::WideCharToMultiByte(page,flags,input,count,output,capacity,default_character,used_default);
}
int WINAPI reverse_convert(UINT page,DWORD flags,LPCCH input,int count,LPWSTR output,int capacity){
    if(selected){
        ++selected->calls;
        throw std::runtime_error("config path logging must not enter reverse conversion");
    }
    return ::MultiByteToWideChar(page,flags,input,count,output,capacity);
}
}
// The test controls the Win32 call only. Both native conversion phases and
// the actual Environment logging bodies remain production implementations.
#define WideCharToMultiByte ::config_path_conversion::convert
#define MultiByteToWideChar ::config_path_conversion::reverse_convert
#include "../../src/platform/encoding_windows.cpp"
#undef MultiByteToWideChar
#undef WideCharToMultiByte
namespace dialog {
std::wstring body,title;unsigned calls=0;UINT expected_flags=MB_OK;std::function<void()> before_show;
int WINAPI show(HWND,LPCWSTR text,LPCWSTR caption,UINT flags){
    if(flags!=expected_flags)std::terminate();if(before_show)before_show();body=text;title=caption;++calls;return IDOK;
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
void check(citrine_eclipse_mirage::application::detail::Environment& env,citrine_eclipse_mirage::application::Options& options) {
    using citrine_eclipse_mirage::configuration::Path;
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
std::string log_bytes(const std::filesystem::path& path){
    std::ifstream input(path,std::ios::binary);
    if(!input){require(!std::filesystem::exists(path),"existing log must be readable");return {};}
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
void config_path_failure_routes(){
    using namespace citrine_eclipse_mirage;namespace fs=std::filesystem;
    const configuration::Path path=u"C:/fixture/caf\u00e9 config.cfg";
    const std::string utf8="C:/fixture/caf\xc3\xa9 config.cfg";
    for(bool queued:{true,false})for(unsigned fail_at:{1U,2U}){
        configuration::State state{};application::Options options;file::NativeFactory files;
        xml_reader_bridge::Backend xml;achievement_storage::Paths paths;
        achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
        application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
        const auto directory=fs::temp_directory_path()/("surge-config-path-log-"+
            std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
        require(fs::create_directory(directory),"fresh owned config path log directory");
        struct Cleanup{fs::path path;~Cleanup(){std::error_code error;fs::remove_all(path,error);}} cleanup{directory};
        const auto file=directory/"surge.log";
        const std::string before=queued?"":"sentinel\n";
        if(!queued){
            state.user_directory=directory.u16string();
            {std::ofstream seed(file,std::ios::binary);seed<<before;}
            require(log_bytes(file)==before,"existing log sentinel prepared");
        }
        config_path_conversion::Observation failure{path,utf8.size(),fail_at,0,
            static_cast<DWORD>(fail_at==1?ERROR_NO_UNICODE_TRANSLATION:ERROR_NOT_ENOUGH_MEMORY)};
        bool caught=false;
        {
            config_path_conversion::Arm arm(failure);
            try{env.log_config_path(path);}
            catch(const std::system_error& error){
                caught=true;require(error.code().category()==std::system_category()&&
                    error.code().value()==static_cast<int>(failure.error),"conversion error category/code preserved");
            }
        }
#ifndef NDEBUG
        require(caught&&failure.calls==fail_at,"Debug path log propagates each selected conversion failure");
#else
        require(!caught&&failure.calls==0,"Release path log performs no conversion calls, including sizing");
#endif
        if(queued){
            state.user_directory=directory.u16string();
            env.log(std::initializer_list<std::string_view>{});
        }
        require(log_bytes(file)==before,"failed path conversion publishes no partial queued or file record");
        config_path_conversion::Observation recovery{path,utf8.size()};
        {config_path_conversion::Arm arm(recovery);env.log_config_path(path);}
#ifndef NDEBUG
        require(recovery.calls==2,"same Environment recovers through actual sizing and fill");
        require(log_bytes(file)==before+"config loaded: "+utf8+"\n","recovery appends exactly one complete UTF8 record");
#else
        require(recovery.calls==0&&log_bytes(file)==before,"Release valid path also skips conversion and log output");
        if(queued)require(!fs::exists(file),"Release path logging and flush cannot create a file");
#endif
    }
    std::cout<<"PASS: config path sizing/fill failures preserve queued/file logs and same-owner recovery; Release observes zero conversion calls\n";
}
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
void write(citrine_eclipse_mirage::application::detail::Environment& env,unsigned worker,unsigned row,unsigned phase){
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
void concurrent_phase(citrine_eclipse_mirage::application::detail::Environment& env,unsigned phase){
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
void actual_environment_logging(citrine_eclipse_mirage::application::detail::Environment& env,citrine_eclipse_mirage::configuration::State& state){
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

void failed_open_preserves_startup(citrine_eclipse_mirage::application::detail::Environment& env,
                                   citrine_eclipse_mirage::configuration::State& state) {
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

void unknown_entitlement_is_nonmodal_and_redacted(citrine_eclipse_mirage::application::detail::Environment& env,
                                                  citrine_eclipse_mirage::configuration::State& state) {
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

// Fail this test process, rather than leave an unexpected real modal pending.
// Production dialog behavior and the existing CTest budget are unchanged.
class UnknownMessageModalWatchdog final {
    std::mutex mutex_;
    std::condition_variable changed_;
    bool finished_=false;
    std::thread worker_;
public:
    explicit UnknownMessageModalWatchdog(std::chrono::milliseconds budget)
        :worker_([this,budget]{
            std::unique_lock lock(mutex_);
            if(!changed_.wait_for(lock,budget,[this]{return finished_;})){
                std::fputs("FAIL: UnknownMessage test did not return from intercepted dialog\n",stderr);
                std::_Exit(3);
            }
        }){}
    ~UnknownMessageModalWatchdog(){
        {std::lock_guard lock(mutex_);finished_=true;}
        changed_.notify_one();worker_.join();
    }
    UnknownMessageModalWatchdog(const UnknownMessageModalWatchdog&)=delete;
    UnknownMessageModalWatchdog& operator=(const UnknownMessageModalWatchdog&)=delete;
};

void actual_unknown_message_diagnostic(citrine_eclipse_mirage::application::detail::Environment& env){
    UnknownMessageModalWatchdog watchdog(std::chrono::seconds(10));
    using namespace citrine_eclipse_mirage;
    struct Input final:messages::UnknownMessageInput{
        std::string bytes=std::string("request-\xc3\xa9\0tail",15);
        std::string_view view()const override{return bytes;}
    } input;
    std::ostringstream output;auto* previous=std::cout.rdbuf(output.rdbuf());
    struct Restore{std::streambuf* previous;~Restore(){std::cout.rdbuf(previous);dialog::expected_flags=MB_OK;dialog::before_show={};}} restore{previous};
    dialog::expected_flags=0x10;const auto before=dialog::calls;const auto original=input.bytes;
    dialog::before_show=[&]{
        if(output.str()!=std::string("[UNKNOWN MESSAGE]\n")+original+"\n")throw std::runtime_error("UnknownMessage counted console diagnostic must precede UI in Debug and Release");
        input.bytes.assign(5000,'x'); // The SDK receives the already owned UTF16 copy.
    };
    env.unknown_message_07bd70(input);
    if(dialog::calls!=before+1||dialog::title!=L"UNKNOWN MESSAGE"||dialog::body!=L"request-\u00e9")
        throw std::runtime_error("UnknownMessage must preserve exact title, error flags, UTF8 conversion and SDK first-NUL display");
    struct DialogFailure{};dialog::before_show=[]{throw DialogFailure{};};bool caught=false;
    try{env.unknown_message_07bd70(input);}catch(const DialogFailure&){caught=true;}
    if(!caught||dialog::calls!=before+1)throw std::runtime_error("UnknownMessage dialog failure must propagate without completion");
}

int main()try{
    using namespace citrine_eclipse_mirage;
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
    logging_fixture::config_path_failure_routes();
    configuration::State recovery_state{};
    application::detail::Environment recovery_env{recovery_state,options,files,xml,paths,clock,formatter};
    logging_fixture::failed_open_preserves_startup(recovery_env,recovery_state);
    configuration::State unknown_state{};
    application::detail::Environment unknown_env{unknown_state,options,files,xml,paths,clock,formatter};
    logging_fixture::unknown_entitlement_is_nonmodal_and_redacted(unknown_env,unknown_state);
    std::cout<<"PASS: actual Environment unknown field diagnostics are nonmodal and redacted\n";
    actual_unknown_message_diagnostic(env);
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
