#include "backend_test_fixture.hpp"
#include <process.h>
#include <thread>
#include <type_traits>
#include <cstdlib>

using namespace b3_tracking_test;
namespace {
void source_create_and_detach(){
    {Fixture f;std::uint32_t id=0;error_value=81;
        f.lower.create_hook=[&](auto* context,auto*){check(f.tracker.tracked_jobs()==1,"reservation exists before lower create, including immediate worker completion");f.lower.release(context);return t::Handle{0x1234};};
        const auto value=t::begin_thread_05e8e0(reinterpret_cast<void*>(3),4096,0x2222,0x3333,7,&id,f.tracker);
        check(value==0x1234&&id==55&&f.lower.contexts.empty(),"source begin succeeds even when worker already freed Context");
        check(f.lower.security_seen==reinterpret_cast<void*>(3)&&f.lower.stack_seen==4096&&f.lower.flags_seen==7,"all original create arguments preserved");
        check(f.lower.incoming_create_error==81&&error_value==303,"incoming and successful-create LastError survive bookkeeping");
        check(f.state.duplicated==std::vector<std::uintptr_t>{0x1234}&&f.tracker.tracked_jobs()==1,"only returned original HANDLE is duplicated");
        check(f.tracker.close_handle(value)&&f.lower.original_closes==std::vector<std::uintptr_t>{0x1234}&&f.state.closed.empty(),"bootstrap detached close forwards original; retained duplicate stays live");
        error_value=82;check(f.tracker.stop_and_join(42)&&error_value==82,"join bookkeeping preserves control caller LastError");
        check(f.state.waited==std::vector<std::uintptr_t>{0x9000}&&f.state.closed==f.state.waited&&f.state.timeouts==std::vector<DWORD>{42},"wait and close use only duplicate handles and requested timeout");
        check(f.tracker.tracked_jobs()==0&&!f.tracker.poisoned(),"successful drain removes live record without poison");
    }
    {Fixture f;f.lower.create_hook=[](auto*,auto*){return t::Handle{0};};f.lower.returned_create_error=123;
        std::uint32_t id=99;const auto value=t::begin_thread_05e8e0(nullptr,0,5,6,0,&id,f.tracker);
        check(!value&&id==99&&f.lower.mapped==123,"failed source creation maps original lower LastError and leaves outputID alone");
        check(f.tracker.tracked_jobs()==0&&f.state.duplicated.empty()&&f.lower.contexts.empty(),"failure removes pre-reservation,does not duplicate,and source cleans Context");
        check(f.lower.libraries==std::vector<std::uintptr_t>{0x44},"original source failure releases acquired module");
        f.lower.create_hook={};id=0;
        const auto retry=f.tracker.create_thread(nullptr,0,nullptr,0,&id);
        check(retry&&id==55&&f.lower.creations==2&&f.tracker.tracked_jobs()==1&&!f.tracker.poisoned(),
            "known null create leaves admission open for an explicitly retried request");
        check(f.tracker.close_handle(retry)&&f.tracker.stop_and_join(0),"successful retry detaches original and drains retained duplicate");
    }
}
void retry_and_pending(){
    {Fixture f;f.create();f.create();unsigned waits=0;f.state.wait_hook=[&](auto,DWORD){return ++waits==2?WAIT_TIMEOUT:WAIT_OBJECT_0;};
        check(!f.tracker.stop_and_join(13)&&f.tracker.tracked_jobs()==1,"timeout preserves unjoined suffix after committed first join");
        check(f.state.closed==std::vector<std::uintptr_t>{0x9000},"only signalled prefix is closed");
        f.state.wait_hook={};check(f.tracker.stop_and_join(19)&&f.state.closed==std::vector<std::uintptr_t>({0x9000,0x9001}),"retry closes remaining duplicate only");
        check(f.tracker.stop_and_join(0),"successful close is idempotent");
        rejects<std::logic_error>([&]{std::uint32_t id=0;f.tracker.create_thread(nullptr,0,nullptr,0,&id);},"new creation rejected after drain admission closes");
        check(f.lower.creations==2,"rejected admission never reaches native lower create");
    }
    {Fixture f;f.create();f.state.wait_hook=[](auto,auto){return WAIT_FAILED;};error_value=71;
        check(!f.tracker.stop_and_join(0)&&f.tracker.tracked_jobs()==1&&f.state.closed.empty()&&error_value==71,"wait failure keeps handle and preserves caller error");
        f.state.wait_hook={};bool first=true;f.state.close_hook=[&](auto){if(first){first=false;return FALSE;}return TRUE;};
        check(!f.tracker.stop_and_join(0)&&f.tracker.tracked_jobs()==1,"CloseHandle failure retains signalled handle for retry");
        check(f.tracker.stop_and_join(0)&&f.tracker.tracked_jobs()==0,"failed close retried without fabricated drain");
    }
    {Fixture f;f.create();f.state.wait_hook=[&](auto,auto){check(!f.tracker.stop_and_join(0),"reentrant drain returns false instead of deadlocking");return WAIT_OBJECT_0;};
        check(f.tracker.stop_and_join(0),"outer drain remains valid after rejected reentry");
    }
    {Fixture f;std::atomic<bool> entered=false,release=false;f.lower.create_hook=[&](auto*,auto*){entered=true;while(!release.load())std::this_thread::yield();return t::Handle{0x404};};
        std::thread creator([&]{std::uint32_t id=0;f.tracker.create_thread(nullptr,0,nullptr,0,&id);});
        while(!entered.load())std::this_thread::yield();
        check(f.tracker.tracked_jobs()==1&&!f.tracker.stop_and_join(0),"reserved not-yet-returned create prevents quiescence");
        release=true;creator.join();check(f.tracker.stop_and_join(0)&&f.tracker.tracked_jobs()==0,"pending create is duplicated and joined on later retry");
    }
}
void forwarding(){
    Fixture f;auto* context=f.tracker.calloc_context_066848();t::Handle module=0;
    check(f.tracker.get_module_handle_ex_w(4,33,&module)&&module==0x44,"module operation forwarded");
    f.tracker.required_thread_slot_064b90()=context;check(f.tracker.optional_thread_context_064d08()==context,"same lower PTD slot reference returned");
    f.tracker.write_error_slot_05d724(42);f.tracker.invalid_parameter_05c954();f.tracker.map_os_error_05d698(43);
    check(f.lower.written==42&&f.lower.mapped==43,"error operations forward unchanged");
    error_value=44;check(f.tracker.last_error()==44&&f.tracker.runtime_policy_06c0d0()==2&&f.tracker.initialize_runtime_067320(1)==27,"error/runtime returns preserved");
    f.tracker.uninitialize_runtime_06736c();check(f.tracker.invoke_entry(11,12)==63,"entry invoke operands/results delegated");
    check(f.tracker.free_library(45)&&f.lower.libraries==std::vector<std::uintptr_t>{45},"module release delegated");
    bool first=false,second=false;try{f.tracker.free_library_and_exit_thread(46,47);}catch(const Exit& e){first=e.module==46&&e.code==47;}try{f.tracker.exit_thread(48);}catch(const Exit& e){second=e.module==0&&e.code==48;}
    check(first&&second,"terminal lower operations forwarded without return substitution");
    f.tracker.free_0668c0(context);check(f.lower.contexts.empty()&&f.tracker.stop_and_join(0),"free delegates and no job is manufactured");
    auto missing=api();missing.wait=nullptr;rejects<std::invalid_argument>([&]{b::TrackingThreads invalid(f.lower,missing);},"incomplete JoinApi rejected before use");
}
DWORD WINAPI real_worker(void* opaque){return ::WaitForSingleObject(static_cast<HANDLE>(opaque),5000)==WAIT_OBJECT_0?0:1;}
struct NativeLower:Lower {
    t::Handle create_thread(void*,std::uint64_t,t::Context* context,std::uint32_t,std::uint32_t* id)override{DWORD tid=0;const auto h=::CreateThread(nullptr,0,real_worker,reinterpret_cast<void*>(context->argument),0,&tid);if(h&&id)*id=tid;return reinterpret_cast<t::Handle>(h);}
    bool close_handle(t::Handle h)override{return ::CloseHandle(reinterpret_cast<HANDLE>(h))!=FALSE;}
};
void native_owned_thread(){
    const auto gate=::CreateEventW(nullptr,TRUE,FALSE,nullptr);check(gate!=nullptr,"own native test gate created");
    NativeLower lower;b::TrackingThreads tracker(lower,b::native_join_api());t::Context context{};context.argument=number(gate);std::uint32_t id=0;
    const auto original=tracker.create_thread(nullptr,0,&context,0,&id);check(original&&id&&tracker.tracked_jobs()==1,"real own thread gets retained duplicated join handle");
    check(tracker.close_handle(original)&&!tracker.stop_and_join(0),"own original detached handle can close while duplicate observes pending thread");
    check(::SetEvent(gate)!=FALSE&&tracker.stop_and_join(5000),"real SDK wait/close drains only our released native worker");
    check(::CloseHandle(gate)!=FALSE,"own gate closed after worker drain");
}
void require_poison_sealed(Fixture& f){
    const auto before=f.lower.creations;const auto tickets=f.tracker.tracked_jobs();
    bool rejected=false;
    try{std::uint32_t id=0;f.tracker.create_thread(nullptr,0,nullptr,0,&id);}
    catch(const std::logic_error&){rejected=true;}
    catch(...){std::_Exit(84);}
    if(!rejected||f.lower.creations!=before||f.tracker.tracked_jobs()!=tickets)std::_Exit(85);
}
int join_terminal(const std::string& mode){
    std::set_terminate([]{std::_Exit(89);});Fixture f;
    const bool prefix=mode=="--wait-suffix-throws"||mode=="--close-suffix-throws";
    const bool waiting=mode=="--wait-throws"||mode=="--wait-suffix-throws";
    f.create();if(prefix)f.create();
    unsigned calls=0;
    if(waiting)f.state.wait_hook=[&](auto,auto)->DWORD{
        if(++calls==(prefix?2U:1U)){throw Failure{};}
        return WAIT_OBJECT_0;
    };
    else f.state.close_hook=[&](auto)->BOOL{
        // Model an injected boundary that throws after its opaque side effect.
        // We cannot know whether the numeric HANDLE is still a live duplicate.
        if(++calls==(prefix?2U:1U)){throw Failure{};}
        return TRUE;
    };
    error_value=876;
    bool threw=false;try{(void)f.tracker.stop_and_join(0);}catch(const Failure&){threw=true;}catch(...){std::_Exit(98);}
    if(!threw||!f.tracker.poisoned()||f.tracker.tracked_jobs()!=1||error_value!=876)std::_Exit(97);
    const auto waits=f.state.waited,closes=f.state.closed;
    const auto expected=prefix?2U:1U;
    if(waits.size()!=expected||closes.size()!=(waiting?expected-1:expected))std::_Exit(96);
    f.state.wait_hook={};f.state.close_hook={};
    if(f.tracker.stop_and_join(0)||f.state.waited!=waits||f.state.closed!=closes||f.tracker.tracked_jobs()!=1)std::_Exit(95);
    require_poison_sealed(f);
    return 90; // Unknown suffix remains retained; destructor must terminate.
}
int terminal(const std::string& mode){
    if(mode=="--wait-throws"||mode=="--close-throws"||mode=="--wait-suffix-throws"||mode=="--close-suffix-throws")return join_terminal(mode);
    std::set_terminate([]{std::_Exit(89);});Fixture f;
    if(mode=="--poison")f.state.duplicate_hook=[](auto,auto){return FALSE;};
    if(mode=="--duplicate-throws")f.state.duplicate_hook=[](auto,auto)->BOOL{throw Failure{};};
    if(mode=="--create-throws")f.lower.create_hook=[](auto*,auto*)->t::Handle{error_value=867;throw Failure{};};
    bool threw=false;
    try{std::uint32_t id=0;f.tracker.create_thread(nullptr,0,nullptr,0,&id);}catch(const Failure&){threw=true;}catch(...){std::_Exit(94);}
    if((mode=="--duplicate-throws"||mode=="--create-throws")!=threw)std::_Exit(93);
    if(mode=="--create-throws"&&error_value!=867)std::_Exit(92);
    if(mode!="--live"){
        if(!f.tracker.poisoned()||f.tracker.tracked_jobs()!=1)std::_Exit(91);
        // Exercise poison admission BEFORE stop_and_join seals accepting.
        require_poison_sealed(f);
        if(f.tracker.stop_and_join(0))std::_Exit(91);
    }
    return 90; // live/poisoned tracker destruction must terminate instead.
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal(argv[1]);
    try{source_create_and_detach();retry_and_pending();forwarding();native_owned_thread();
        for(const auto* mode:{"--live","--poison","--duplicate-throws","--create-throws","--wait-throws","--close-throws","--wait-suffix-throws","--close-suffix-throws"})check(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==89,"undrained/poisoned tracker destruction is explicit terminal contract; unknown wait/close suffix never replayed");
        std::cout<<"PASS: "<<checks<<" bootstrap-backend tracking checks; source begin/detach,pre-create tickets,LastError,pending joins/retry/poison,alllower forwarding and one own real Win32 worker; no original target executed\n";
    }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}
}
