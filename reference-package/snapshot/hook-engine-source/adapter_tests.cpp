#include "adapters.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>
using namespace recovered_hooks;
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
    std::vector<anadius::host_init_recovered::Callback> seen;
    std::uintptr_t rebuilt_address(anadius::host_init_recovered::Callback c)override{
        seen.push_back(c);return 0x30000+static_cast<unsigned>(c)*0x100;
    }
};
struct Remaining final : anadius::host_init_recovered::Effects, anadius::network_hook_recovered::CleanupEffects {
    std::vector<std::string> log;std::uintptr_t next_proc=0x5000;bool modern=true;
    std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t x,std::uint32_t t,std::uint8_t c)override{require(c==3,"version condition delegated");return x+t;}
    bool verify_version(const anadius::host_init_recovered::VersionRequest& v,std::uint32_t t,std::uint64_t m)override{require(v.major==6&&v.minor==2&&t==0x23&&m==0x23,"version request delegated unchanged");return modern;}
    std::uintptr_t get_module_handle(std::u16string_view s)override{require(!s.empty(),"module name forwarded");return 0x8000;}
    std::uintptr_t get_proc_address(std::uintptr_t m,std::string_view s)override{require(m==0x8000&&!s.empty(),"module and symbol forwarded");return next_proc+=0x100;}
    std::int32_t initialize_engine_046a20()override{throw std::runtime_error("unbound inner engine called");}
    std::int32_t create_hook_0466e0(std::uintptr_t,anadius::host_init_recovered::Callback,std::uintptr_t&)override{throw std::runtime_error("inner create called");}
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
        anadius::configuration::State config{};config.executable_path=u"C:\\synthetic\\game.exe";config.block_reruns=block;
        if(folders)config.documents=u"C:\\synthetic\\docs";
        anadius::host_init_recovered::Values values;std::vector<std::uintptr_t> enabled;
        std::array<std::uintptr_t,10> originals{};Remaining remaining;remaining.modern=modern;MockApi engine;Resolver resolver;
        HostInitEffects effects(remaining,engine,resolver);
        anadius::host_init_recovered::initialize_host_00e310({config,values,enabled,originals},effects);
        const std::size_t n=6+2*block+2*folders;
        require(engine.targets.size()==n&&engine.enabled==engine.targets&&enabled==engine.targets,"actual host init uses Api for each create/enable and records targets");
        require(engine.events.front()=="init"&&engine.events.size()==1+2*n,"engine order init then alternating create/enable");
        for(std::size_t i=0;i<n;++i){
            const auto slot=static_cast<std::size_t>(resolver.seen[i]);
            require(engine.detours[i]==0x30000+slot*0x100,"rebuilt callback resolver determines detour, not original preferred VA");
            require(originals[slot]==engine.targets[i]+0x10000,"engine trampoline output reaches original-pointer slot");
        }
        CleanupEffects cleanup(remaining,engine);
        anadius::network_hook_recovered::cleanup_hooks_0102b0(enabled,cleanup);
        require(engine.disabled==enabled&&engine.events.back()=="uninit","actual cleanup routes disable list then uninitialize");
        require(enabled==engine.targets,"cleanup leaves observed target list intact");
    }
}
void test_status_and_outputs(){
    for(int status:{-2,-1,0,1,2,3,7,8,9,10,11,12})for(bool write:{false,true}){
        MockApi engine;engine.create_status={status};engine.write_output=write;Resolver resolver;Remaining remaining;
        HostInitEffects effects(remaining,engine,resolver);std::uintptr_t original=0xabc;
        require(effects.create_hook_0466e0(0x555,anadius::host_init_recovered::Callback::connect_00fd00,original)==status,"create status is passed unchanged");
        require(original==(write?0x10555:0xabc),"every normal return transfers backend output, including failures");
        engine.init_status=status;engine.uninit_status=status;engine.disable_status=status;engine.enable_status={status};
        require(effects.initialize_engine_046a20()==status&&effects.enable_hook_046a10(0)==status,"initialize/enable status preserved including all-hook null target");
        CleanupEffects cleanup(remaining,engine);
        require(cleanup.disable_hook_046a00(0)==status&&cleanup.uninitialize_engine_046ac0()==status,"disable/uninitialize status preserved");
    }
    for(int init:{1,2,9}){
        MockApi engine;engine.init_status=init;Resolver resolver;Remaining remaining;HostInitEffects effects(remaining,engine,resolver);
        anadius::configuration::State config{};anadius::host_init_recovered::Values values;std::vector<std::uintptr_t> enabled;std::array<std::uintptr_t,10> originals{};
        anadius::host_init_recovered::initialize_host_00e310({config,values,enabled,originals},effects);
        require(engine.events==std::vector<std::string>{"init"}&&resolver.seen.empty(),"actual host init rejects nonzero engine result");
    }
    {MockApi engine;engine.create_status={8};Resolver resolver;Remaining remaining;HostInitEffects effects(remaining,engine,resolver);
     anadius::configuration::State config{};anadius::host_init_recovered::Values values;std::vector<std::uintptr_t> enabled;std::array<std::uintptr_t,10> originals{};
     anadius::host_init_recovered::initialize_host_00e310({config,values,enabled,originals},effects);
     require(engine.targets.size()==6&&engine.enabled.empty()&&enabled.empty(),"create errors skip enable and collection but later hooks continue");
     require(std::all_of(originals.begin(),originals.begin()+6,[](auto n){return n!=0;}),"backend writes survive failed create normal return");}
}
struct Trap{recovered_preloader::TrapSite site;};
struct PreloaderHost final : recovered_preloader::Host {
    int disables{};
    BOOL disable_thread_library_calls(HMODULE)override{++disables;return 1;}
    DWORD get_last_error()override{throw Injected{};}
    DWORD current_process_id()override{throw Injected{};}
    HMODULE main_module()override{throw Injected{};}
    int mutex_lock(void*)override{throw Injected{};}
    BOOL wait32(std::atomic<std::uint32_t>&,std::uint32_t)override{throw Injected{};}
    void wake_all(std::atomic<std::uint32_t>&)override{throw Injected{};}
    void* allocate(std::size_t)override{throw Injected{};}
    void* reallocate(void*,std::size_t)override{throw Injected{};}
    void release(void*)override{throw Injected{};}
    [[noreturn]] void invalid_instruction(recovered_preloader::TrapSite s)override{throw Trap{s};}
};
void test_preloader_composition(){
    for(int init:{-2,0,1,2,9}){
        MockApi api;api.init_status=init;api.create_status={0,8,0};api.enable_status={10};PreloaderEngine engine(api);PreloaderHost host;
        recovered_preloader::Bindings bindings{};
        bindings.process_next_target=reinterpret_cast<recovered_preloader::ProcessNext>(0x1000);
        bindings.trylock_target=reinterpret_cast<recovered_preloader::MutexRoutine>(0x2000);
        bindings.crypt_target=reinterpret_cast<recovered_preloader::CryptQuery>(0x3000);
        bindings.process_next_detour=reinterpret_cast<recovered_preloader::ProcessNext>(0x4000);
        bindings.trylock_detour=reinterpret_cast<recovered_preloader::MutexRoutine>(0x5000);
        bindings.crypt_detour=reinterpret_cast<recovered_preloader::CryptQuery>(0x6000);
        recovered_preloader::Core core(host,engine,bindings);
        core.original_trylock=reinterpret_cast<recovered_preloader::MutexRoutine>(0x777);
        try{
            require(core.dll_main(reinterpret_cast<HMODULE>(0x999),DLL_PROCESS_ATTACH,nullptr)==1,"preloader core attach preserves success with failed create/enable");
            require(init==-2||init==0||init==1,"only observed init statuses accepted");
            require(api.events==std::vector<std::string>{"init","create","create","create","enable"},"preloader once-init and three creates before enable-all");
            require(api.targets==std::vector<std::uintptr_t>{0x1000,0x2000,0x3000}&&api.detours==std::vector<std::uintptr_t>{0x4000,0x5000,0x6000},"preloader passes all selected original/rebuilt target identities");
            require(reinterpret_cast<std::uintptr_t>(core.original_process_next)==0x11000&&reinterpret_cast<std::uintptr_t>(core.original_trylock)==0x777&&reinterpret_cast<std::uintptr_t>(core.original_crypt)==0x13000,"preloader ignores failed-create output and keeps old original");
            require(api.enabled==std::vector<std::uintptr_t>{0}&&host.disables==1,"preloader enables-all using null target");
        }catch(const Trap& t){require((init==2||init==9)&&t.site==recovered_preloader::TrapSite::EngineInitialization,"bad engine init traps at observed site");require(api.targets.empty(),"initialization trap precedes hook creates");}
    }
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
int main(){try{test_native_link_only();test_host_composition();test_status_and_outputs();test_preloader_composition();std::cout<<"PASS: "<<checks<<" hook adapter/source integration checks; fake Api, no native hooks\n";}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected test exception\n";return 1;}}
