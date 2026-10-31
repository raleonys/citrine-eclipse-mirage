#include "../../src/runtime/application_internal.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
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
#define MessageBoxW dialog::show
#include "../../src/runtime/application_environment.cpp"
#undef MessageBoxW

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
}

int main()try{
    using namespace preloader_l;
    configuration::State state{};application::Options options;
    file_recovered::NativeFactory files;xml_reader_bridge::Backend xml;achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    env.unknown_entitlement_field("QueryEntitlements unknown attr: \xc3\xa9|value");
    if(dialog::calls!=1||dialog::title!=L"SURGE"||dialog::body!=L"QueryEntitlements unknown attr: \u00e9|value")
        throw std::runtime_error("actual dialog lost the supplied field/value diagnostic");
    configuration::State logging_state{};
    application::detail::Environment logging_env{logging_state,options,files,xml,paths,clock,formatter};
    logging_fixture::actual_environment_logging(logging_env,logging_state);
    std::cout<<"PASS: actual Environment dialog forwards counted UTF8 diagnostic and product title\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
