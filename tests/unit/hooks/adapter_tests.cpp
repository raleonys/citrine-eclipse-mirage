// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_engine_adapters.hpp"
#include "laureondo/platform/hook_unload_guard.hpp"
#include <windows.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>
using namespace laureondo::platform::hook_engine;
using laureondo::host_init::callback_count;
namespace {
int checks;
void require(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
struct Injected{};
struct MockApi final : Api {
    std::vector<std::string> events;std::vector<std::uintptr_t> targets,detours,disabled;
    std::vector<int> create_status,enable_status;
    int init_status{},uninit_status{},disable_status{};bool write_output{true};
    void** last_output{};
    int initialize()override{events.push_back("init");return init_status;}
    int create(void* t,void* d,void** o)override{
        const auto index=targets.size();targets.push_back(reinterpret_cast<std::uintptr_t>(t));detours.push_back(reinterpret_cast<std::uintptr_t>(d));last_output=o;
        events.push_back("create");if(o&&write_output)*o=reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(t)+0x10000);
        return create_status.empty()?0:create_status.at(index%create_status.size());
    }
    int enable(void* t)override{events.push_back("enable");enabled.push_back(reinterpret_cast<std::uintptr_t>(t));return enable_status.empty()?0:enable_status.at((enabled.size()-1)%enable_status.size());}
    int disable(void* t)override{events.push_back("disable");disabled.push_back(reinterpret_cast<std::uintptr_t>(t));return disable_status;}
    int uninitialize()override{events.push_back("uninit");return uninit_status;}
    std::vector<std::uintptr_t> enabled;
};
struct Resolver final : CallbackResolver {
    std::vector<laureondo::host_init::Callback> seen;
    std::uintptr_t rebuilt_address(laureondo::host_init::Callback c)override{
        seen.push_back(c);return 0x30000+static_cast<unsigned>(c)*0x100;
    }
};
struct Remaining final : laureondo::host_init::Effects, laureondo::network_hook::CleanupEffects {
    std::vector<std::string> log;std::uintptr_t next_proc=0x5000;bool modern=true;
    std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t x,std::uint32_t t,std::uint8_t c)override{require(c==3,"version condition delegated");return x+t;}
    bool verify_version(const laureondo::host_init::VersionRequest& v,std::uint32_t t,std::uint64_t m)override{require(v.major==6&&v.minor==2&&t==0x23&&m==0x23,"version request delegated unchanged");return modern;}
    std::uintptr_t get_module_handle(std::u16string_view s)override{require(!s.empty(),"module name forwarded");return 0x8000;}
    std::uintptr_t get_proc_address(std::uintptr_t m,std::string_view s)override{require(m==0x8000&&!s.empty(),"module and symbol forwarded");return next_proc+=0x100;}
    std::int32_t initialize_engine_046a20()override{throw std::runtime_error("unbound inner engine called");}
    std::int32_t create_hook_0466e0(std::uintptr_t,laureondo::host_init::Callback,std::uintptr_t&)override{throw std::runtime_error("inner create called");}
    std::int32_t enable_hook_046a10(std::uintptr_t)override{throw std::runtime_error("inner enable called");}
    void log_text_006880(std::string_view s)override{log.emplace_back(s);}
    void log_integer_00af90(std::int32_t n)override{log.push_back(std::to_string(n));}
    void finish_log_0088e0()override{log.push_back("flush");}
    std::int32_t disable_hook_046a00(std::uintptr_t)override{throw std::runtime_error("inner disable called");}
    std::int32_t uninitialize_engine_046ac0()override{throw std::runtime_error("inner uninitialize called");}
    void log_literal_006880(std::string_view s)override{log.emplace_back(s);}
    void log_finish_0088e0()override{log.push_back("flush");}
};
void test_host_composition(){
    for(bool modern:{false,true})for(bool block:{false,true})for(bool folders:{false,true}){
        laureondo::configuration::State config{};config.executable_path=u"C:\\synthetic\\game.exe";config.block_reruns=block;
        if(folders)config.documents=u"C:\\synthetic\\docs";
        laureondo::host_init::Values values;std::vector<std::uintptr_t> enabled;
        std::array<std::uintptr_t,callback_count> originals{};Remaining remaining;remaining.modern=modern;MockApi engine;Resolver resolver;
        HostInitEffects effects(remaining,engine,resolver);
        laureondo::host_init::initialize_host_00e310({config,values,enabled,originals},effects);
        const std::size_t n=7+2*block+2*folders;
        require(engine.targets.size()==n&&engine.enabled==engine.targets&&enabled==engine.targets,"actual host init uses Api for each create/enable and records targets");
        require(engine.events.front()=="init"&&engine.events.size()==1+2*n,"engine order init then alternating create/enable");
        for(std::size_t i=0;i<n;++i){
            const auto slot=static_cast<std::size_t>(resolver.seen[i]);
            require(engine.detours[i]==0x30000+slot*0x100,"rebuilt callback resolver determines detour, not original preferred VA");
            require(originals[slot]==engine.targets[i]+0x10000,"engine trampoline output reaches original-pointer slot");
        }
        CleanupEffects cleanup(remaining,engine);
        laureondo::network_hook::cleanup_hooks_0102b0(enabled,cleanup);
        require(engine.disabled==enabled&&engine.events.back()=="uninit","actual cleanup routes disable list then uninitialize");
        require(enabled==engine.targets,"cleanup leaves observed target list intact");
    }
}
void test_status_and_outputs(){
    for(int status:{-2,-1,0,1,2,3,7,8,9,10,11,12})for(bool write:{false,true}){
        MockApi engine;engine.create_status={status};engine.write_output=write;Resolver resolver;Remaining remaining;
        HostInitEffects effects(remaining,engine,resolver);std::uintptr_t original=0xabc;
        require(effects.create_hook_0466e0(0x555,laureondo::host_init::Callback::connect_00fd00,original)==status,"create status is passed unchanged");
        require(original==(write?0x10555:0xabc),"every normal return transfers backend output, including failures");
        engine.init_status=status;engine.uninit_status=status;engine.disable_status=status;engine.enable_status={status};
        require(effects.initialize_engine_046a20()==status&&effects.enable_hook_046a10(0)==status,"initialize/enable status preserved including all-hook null target");
        CleanupEffects cleanup(remaining,engine);
        require(cleanup.disable_hook_046a00(0)==status&&cleanup.uninitialize_engine_046ac0()==status,"disable/uninitialize status preserved");
    }
    for(int init:{1,2,9}){
        MockApi engine;engine.init_status=init;Resolver resolver;Remaining remaining;HostInitEffects effects(remaining,engine,resolver);
        laureondo::configuration::State config{};laureondo::host_init::Values values;std::vector<std::uintptr_t> enabled;std::array<std::uintptr_t,callback_count> originals{};
        laureondo::host_init::initialize_host_00e310({config,values,enabled,originals},effects);
        require(engine.events==std::vector<std::string>{"init"}&&resolver.seen.empty(),"actual host init rejects nonzero engine result");
    }
    {MockApi engine;engine.create_status={8};Resolver resolver;Remaining remaining;HostInitEffects effects(remaining,engine,resolver);
     laureondo::configuration::State config{};laureondo::host_init::Values values;std::vector<std::uintptr_t> enabled;std::array<std::uintptr_t,callback_count> originals{};
     laureondo::host_init::initialize_host_00e310({config,values,enabled,originals},effects);
     require(engine.targets.size()==7&&engine.enabled.empty()&&enabled.empty(),"create errors skip enable and collection but later hooks continue");
     require(std::all_of(originals.begin(),originals.begin()+6,[](auto n){return n!=0;})&&originals[10]!=0,"backend writes survive failed create normal return including crypt slot");}
}
using ActualFunction=int(WINAPI*)(int);
volatile int native_a_offset=11,native_b_offset=13;
ActualFunction original_a=nullptr,original_b=nullptr;
__declspec(noinline) int WINAPI native_a(int n){return n+native_a_offset;}
__declspec(noinline) int WINAPI native_b(int n){return n+native_b_offset;}
int WINAPI detour_a(int n){return original_a(n)+100;}
int WINAPI detour_b(int n){return original_b(n)+200;}
void test_native_queued_activation(){
    NativeApi api;require(api.initialize()==status_code::ok,"real engine initialized for queue behavior");
    void* a=nullptr;void* b=nullptr;
    require(api.create(reinterpret_cast<void*>(&native_a),reinterpret_cast<void*>(&detour_a),&a)==status_code::ok,"first actual fixture hook created");
    require(api.create(reinterpret_cast<void*>(&native_b),reinterpret_cast<void*>(&detour_b),&b)==status_code::ok,"second actual fixture hook created");
    original_a=reinterpret_cast<ActualFunction>(a);original_b=reinterpret_cast<ActualFunction>(b);
    ActualFunction volatile call_a=&native_a,call_b=&native_b;
    require(api.queue_enable(reinterpret_cast<void*>(&native_a))==status_code::ok&&api.queue_enable(reinterpret_cast<void*>(&native_b))==status_code::ok,"both actual hooks queued");
    require(call_a(3)==14&&call_b(3)==16,"queued hooks remain inactive before one batch apply");
    require(api.apply_queued()==status_code::ok&&call_a(3)==114&&call_b(3)==216,"one batch apply activates both hooks and usable originals");
    require(api.disable(nullptr)==status_code::ok&&call_a(3)==14&&call_b(3)==16,"batch disable restores both original functions");
    require(api.uninitialize()==status_code::ok,"actual queued test releases engine");
}
DWORD WINAPI guard_thread(void* stop){WaitForSingleObject(static_cast<HANDLE>(stop),INFINITE);return 0;}
void test_external_thread_guard(){
    const auto stop=CreateEventW(nullptr,TRUE,FALSE,nullptr);require(stop!=nullptr,"guard fixture stop event");
    const auto thread=CreateThread(nullptr,0,guard_thread,stop,0,nullptr);require(thread!=nullptr,"guard fixture thread");
    require(!laureondo::external_threads_outside_module(),"external thread in module code or its return stack defers unload");
    SetEvent(stop);require(WaitForSingleObject(thread,5000)==WAIT_OBJECT_0,"guard fixture thread stopped");
    CloseHandle(thread);CloseHandle(stop);
    bool clear=false;for(int i=0;i<100&&!clear;++i){clear=laureondo::external_threads_outside_module();if(!clear)Sleep(10);}
    require(clear,"module becomes quiescent after external thread exits");
}
void test_native_link_only(){
    // Forces linkage of NativeApi's real methods and the full production engine
    // while exercising only C++ construction/RTTI/destruction, never hook APIs.
    NativeApi native;
    Api* volatile selected=&native;
    Api* observed=selected;
    require(typeid(*observed)==typeid(NativeApi),"production NativeApi and all engine entry points link; methods not called");
}
}
int main(){try{test_native_link_only();test_host_composition();test_status_and_outputs();test_native_queued_activation();test_external_thread_guard();std::cout<<"PASS: "<<checks<<" hook adapter/source checks; controlled backend plus real queued activation and external thread guard\n";}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected test exception\n";return 1;}}
