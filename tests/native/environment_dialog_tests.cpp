// SPDX-License-Identifier: GPL-3.0-only

#include "../../src/runtime/application_internal.hpp"
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
    require(!fs::exists(directory/"surge.log"),"Release unknown field must not create a log file");
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
    env.unknown_entitlement_field("QueryEntitlements unknown attr: unexpected|value");
    if(dialog::calls!=0)
        throw std::runtime_error("unknown entitlement input must not invoke a blocking dialog");
    configuration::State logging_state{};
    application::detail::Environment logging_env{logging_state,options,files,xml,paths,clock,formatter};
    logging_fixture::actual_environment_logging(logging_env,logging_state);
    configuration::State unknown_state{};
    application::detail::Environment unknown_env{unknown_state,options,files,xml,paths,clock,formatter};
    logging_fixture::unknown_entitlement_is_nonmodal_and_redacted(unknown_env,unknown_state);
    std::cout<<"PASS: actual Environment unknown field remains nonmodal under shared-handler invocation\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
