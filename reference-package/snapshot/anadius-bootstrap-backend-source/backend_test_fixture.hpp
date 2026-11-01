#pragma once
#include "tracked_threads.hpp"
#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace b3_tracking_test {
namespace b=anadius::bootstrap_backend;namespace t=anadius::threads_recovered;
inline unsigned checks=0;
inline void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& f,const char* message){bool seen=false;try{f();}catch(const E&){seen=true;}check(seen,message);}
inline HANDLE handle(std::uintptr_t value){return reinterpret_cast<HANDLE>(value);}
inline std::uintptr_t number(HANDLE value){return reinterpret_cast<std::uintptr_t>(value);}
inline thread_local DWORD error_value=0;
struct ApiState {
    std::vector<std::uintptr_t> duplicated,waited,closed;
    std::vector<DWORD> timeouts;
    std::function<BOOL(HANDLE,LPHANDLE)> duplicate_hook;
    std::function<DWORD(HANDLE,DWORD)> wait_hook;
    std::function<BOOL(HANDLE)> close_hook;
    std::uintptr_t next_join=0x9000;
};
inline ApiState* current=nullptr;
inline HANDLE WINAPI process(){error_value=700;return handle(0x777);}
inline BOOL WINAPI duplicate(HANDLE source_process,HANDLE source,HANDLE destination_process,LPHANDLE out,DWORD access,BOOL inherit,DWORD options){
    check(source_process==handle(0x777)&&destination_process==source_process&&access==0&&!inherit&&options==DUPLICATE_SAME_ACCESS,"duplicate exact process/flags contract");
    current->duplicated.push_back(number(source));error_value=701;
    if(current->duplicate_hook)return current->duplicate_hook(source,out);
    *out=handle(current->next_join++);return TRUE;
}
inline DWORD WINAPI wait(HANDLE value,DWORD timeout){current->waited.push_back(number(value));current->timeouts.push_back(timeout);error_value=702;return current->wait_hook?current->wait_hook(value,timeout):WAIT_OBJECT_0;}
inline BOOL WINAPI close(HANDLE value){current->closed.push_back(number(value));error_value=703;return current->close_hook?current->close_hook(value):TRUE;}
inline DWORD WINAPI error(){return error_value;}
inline void WINAPI set_error(DWORD value){error_value=value;}
inline b::JoinApi api(){return {process,duplicate,wait,close,error,set_error};}
struct Exit {std::uintptr_t module;std::uint32_t code;};
struct Failure {};
struct Lower:t::Effects {
    std::vector<std::string> calls;std::vector<t::Context*> contexts;
    std::vector<std::uintptr_t> original_closes,libraries;
    std::function<t::Handle(t::Context*,std::uint32_t*)> create_hook;
    std::uint32_t mapped=0,written=0;DWORD incoming_create_error=0,returned_create_error=303;
    void* security_seen=nullptr;std::uint64_t stack_seen=0;std::uint32_t flags_seen=0;
    t::Context* slot=nullptr;unsigned creations=0;
    virtual ~Lower(){for(auto* p:contexts)delete p;}
    void release(t::Context* p){const auto found=std::find(contexts.begin(),contexts.end(),p);if(found!=contexts.end())contexts.erase(found);delete p;}
    t::Context* calloc_context_066848()override{calls.push_back("calloc");auto* p=new t::Context{};contexts.push_back(p);return p;}
    void free_0668c0(t::Context* p)override{calls.push_back(p?"free":"free-null");if(p)release(p);}
    bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t address,t::Handle* result)override{calls.push_back("get-module");check(flags==4&&address!=0,"source context lookup operands");*result=0x44;return true;}
    t::Handle create_thread(void* security,std::uint64_t stack,t::Context* context,std::uint32_t flags,std::uint32_t* id)override{
        ++creations;calls.push_back("create");security_seen=security;stack_seen=stack;flags_seen=flags;incoming_create_error=error_value;
        const auto value=create_hook?create_hook(context,id):t::Handle{0x1100+creations};
        if(id&&value)*id=55;error_value=returned_create_error;return value;
    }
    std::uint32_t last_error()override{calls.push_back("last-error");return error_value;}
    void write_error_slot_05d724(std::uint32_t v)override{calls.push_back("write-error");written=v;}
    void invalid_parameter_05c954()override{calls.push_back("invalid");}
    void map_os_error_05d698(std::uint32_t v)override{calls.push_back("map-error");mapped=v;}
    bool close_handle(t::Handle v)override{calls.push_back("original-close");original_closes.push_back(v);return true;}
    bool free_library(t::Handle v)override{calls.push_back("free-library");libraries.push_back(v);return true;}
    t::Context*& required_thread_slot_064b90()override{calls.push_back("required-slot");return slot;}
    t::Context* optional_thread_context_064d08()override{calls.push_back("optional-slot");return slot;}
    int runtime_policy_06c0d0()override{calls.push_back("policy");return 2;}
    int initialize_runtime_067320(int value)override{calls.push_back("initialize");check(value==1,"runtime initialize operand");return 27;}
    void uninitialize_runtime_06736c()override{calls.push_back("uninitialize");}
    std::uint32_t invoke_entry(std::uintptr_t entry,std::uintptr_t argument)override{calls.push_back("invoke");check(entry==11&&argument==12,"entry operands forwarded");return 63;}
    [[noreturn]] void free_library_and_exit_thread(t::Handle module,std::uint32_t code)override{throw Exit{module,code};}
    [[noreturn]] void exit_thread(std::uint32_t code)override{throw Exit{0,code};}
};
struct Fixture {
    ApiState state;Lower lower;b::TrackingThreads tracker{lower,api()};
    Fixture(){current=&state;error_value=17;}
    t::Handle create(){std::uint32_t id=0;const auto value=tracker.create_thread(nullptr,0,nullptr,0,&id);check(id==55,"returned create ID preserved");return value;}
};
} // namespace b3_tracking_test
