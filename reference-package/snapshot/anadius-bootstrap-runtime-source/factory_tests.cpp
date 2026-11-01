#include "runtime_test_fixture.hpp"
#include <system_error>

namespace {
using namespace b4_test;
void composed_shutdown_retries(){
    Fixture f;f.start();
    require(f.host.registrations==1&&f.host.callbacks.size()==1&&f.host.handler!=previous_terminate,"factory installs concrete exit and terminate registrations");
    require(!f.remaining->exit_callback&&!f.remaining->terminate_callback,"registration bypasses remaining Services policies");
    require(f.runtime->configuration().username=="override-user"&&f.runtime->configuration().sdk=="1,2,3","actual FilePass and config sources run through Runtime");
    require(f.joins.size()==3&&f.threads->original_closes.size()==3&&f.library_os->paths==std::vector<std::wstring>{L"main.dll"},"three pending source workers and synchronous main-load phase constructed");
    const auto defaults=f.runtime->selected_defaults();
    require(!defaults.files&&!defaults.input&&!defaults.library_os&&!defaults.execution&&!defaults.thread_join,"explicit selected providers are retained without replacement");
    require(f.library_os->unloaded.empty()&&f.cleanup->targets.empty(),"cleanup is not performed before quiescence");
    f.stop_allowed=false;
    require(!f.runtime->close()&&f.runtime->phase()==o::Phase::Stopping&&n::is_bound(),"failed external stop retains runtime and native binding");
    require(f.api_state.waited.empty()&&f.library_os->unloaded.empty()&&f.host.handler!=previous_terminate,"failed external stop neither joins nor cleans nor restores handler");
    f.stop_allowed=true;f.timeout_once=true;
    require(!f.runtime->close()&&f.runtime->phase()==o::Phase::Stopping&&f.joins.size()==3,"timeout retains all pending workers for retry");
    require(f.library_os->unloaded.empty()&&f.cleanup->targets.empty()&&n::is_bound(),"timeout excludes cleanup while retaining source binding");
    // Paused CRT dispatch cannot run callbacks before remaining worker joins.
    f.host.callbacks.back()();
    require(f.library_os->unloaded.empty()&&f.cleanup->targets.empty(),"paused permanent exit dispatcher does not run pending cleanup early");
    require(f.runtime->close()&&f.runtime->phase()==o::Phase::Closed&&!n::is_bound(),"retry joins, drains application cleanup and closes native binding");
    require(f.joins.empty()&&f.api_state.closed.size()==3&&f.server->runs==1&&f.online->loads==1,"all three source jobs complete before provider release");
    require(f.runtime->configuration().server_port==0x1234&&f.runtime->configuration().username=="network-user","source workers update single owned configuration");
    require(f.library_os->paths==std::vector<std::wstring>{L"main.dll",L"worker.dll"},"worker paths refresh after shared source waits");
    require(f.library_os->unloaded==std::vector<std::uintptr_t>{0x777,0x501,0x502},"post-join exit source unloads retained module vector exactly once");
    require(f.cleanup->targets.size()==6&&f.cleanup->uninitializations==1,"source host target list reaches source cleanup");
    const auto last_join=std::find(f.track.events.rbegin(),f.track.events.rend(),"join-handle-close");
    require(last_join!=f.track.events.rend()&&static_cast<std::size_t>(std::distance(f.track.events.begin(),last_join.base())-1)<pos(f.track.events,"library-free"),"all join handles are closed before first application cleanup");
    require(pos(f.track.events,"library-free")<pos(f.track.events,"online-destroy")&&pos(f.track.events,"online-free")<pos(f.track.events,"hook-disable"),"existing library then online then hook cleanup order retained");
    require(absent(f.track.events,"cleanup-first")&&absent(f.track.events,"cleanup-second")&&absent(f.track.events,"cleanup-network"),"remaining Services cleanup substitutes never execute");
    require(f.host.handler==previous_terminate,"controlled close restores prior terminate handler");
    const auto events=f.track.events.size();const auto stops=f.stops;auto late_exit=f.host.callbacks.back();
    require(f.runtime->close()&&f.stops==stops&&f.track.events.size()==events,"closed Runtime is idempotent");
    rejects<std::logic_error>([&]{f.runtime->publish();},"closed Runtime cannot publish again");
    rejects<std::logic_error>([&]{(void)f.runtime->application_entry();},"closed Runtime does not return application entry");
    late_exit();require(f.track.events.size()==events,"late registered exit thunk is inert after finish");
    f.runtime.reset();require(f.destroyed_config=="network-user","backing state dies while source configuration remains alive");
    const auto after_destroy=f.track.events.size();late_exit();require(f.track.events.size()==after_destroy,"late thunk does not dereference destroyed Runtime");
}
void external_throw_reentry_and_foreign_handler(){
    Fixture f;f.start();
    f.stop_hook=[]{throw std::runtime_error("external-stop");};
    rejects<std::runtime_error>([&]{f.runtime->close();},"external stop exception is retryable");
    require(f.runtime->phase()==o::Phase::Stopping&&n::is_bound()&&f.api_state.waited.empty(),"exception retains pending graph and binding before joins");
    f.stop_hook=[&]{rejects<std::logic_error>([&]{f.runtime->close();},"Runtime rejects stop callback control reentry");};
    f.host.handler=foreign_terminate;
    require(f.runtime->close()&&f.host.handler==foreign_terminate,"retry closes normally and preserves sequential foreign terminate replacement");
    require(f.cleanup->uninitializations==1&&f.library_os->unloaded.size()==3,"retry after stop exception performs exactly one application cleanup");
}
void failed_registration_and_constructed_close(){
    {
        Fixture f;f.host.exit_result=23;f.start();
        require(f.host.registrations==1&&f.host.callbacks.empty(),"nonzero registrar result is observed without inventing registered callback");
        require(f.runtime->close()&&f.joins.empty()&&!n::is_bound(),"ignored source registrar failure still permits worker drain and controlled close");
        require(f.library_os->unloaded.empty()&&f.cleanup->targets.empty()&&f.cleanup->uninitializations==0,"failed exit registration does not invent successful source exit cleanup");
        require(f.host.handler==previous_terminate,"failed exit registration still restores installed terminate handler");
    }
    {
        Fixture f;f.construct(f.resources());
        require(f.runtime->phase()==o::Phase::Constructed&&!n::is_bound()&&f.host.registrations==0&&f.host.sets==0,"factory construction neither publishes nor registers callbacks");
        require(f.runtime->configuration().username=="seed-user"&&f.runtime->console().show_lsx==0,"factory preserves caller initial values before attach");
        rejects<std::logic_error>([&]{(void)f.runtime->application_entry();},"unpublished Runtime does not return application entry");
        require(f.runtime->close()&&f.stops==0&&f.api_state.waited.empty(),"constructed close avoids application and stop policy execution");
        require(f.host.registrations==0&&f.host.sets==0&&f.library_os->unloaded.empty(),"constructed close remains free of callback registration and cleanup");
    }
    {
        Fixture f;f.construct(f.resources());f.runtime->publish();
        require(n::is_bound()&&f.host.registrations==0,"publication alone does not initialize application callbacks");
        require(f.runtime->close()&&!n::is_bound()&&f.stops==1,"published without attach closes empty provider graph safely");
        require(f.host.registrations==0&&f.host.sets==0&&f.library_os->unloaded.empty(),"unattached close creates no invented exit callback");
    }
}
void defaults_and_required_provider_graph(){
    {
        Fixture f;auto resources=f.resources();resources.files.reset();resources.input.reset();resources.library_os.reset();resources.execution.reset();resources.join_api={};
        f.construct(std::move(resources));const auto selected=f.runtime->selected_defaults();
        require(selected.files&&selected.input&&selected.library_os&&selected.execution&&selected.thread_join,"all five documented concrete native defaults selected");
        require(f.runtime->close()&&f.host.registrations==0&&f.api_state.waited.empty(),"native default adapter construction executes no original application or user file operations");
    }
    for(unsigned missing=0;missing!=10;++missing){
        Fixture f;auto resources=f.resources();
        switch(missing){case 0:resources.configuration.reset();break;case 1:resources.console.reset();break;case 2:resources.threads.reset();break;case 3:resources.remaining.reset();break;case 4:resources.server.reset();break;case 5:resources.online.reset();break;case 6:resources.library_effects.reset();break;case 7:resources.host.reset();break;case 8:resources.hook_cleanup.reset();break;case 9:resources.stop_sources={};break;}
        rejects<std::invalid_argument>([&]{f.construct(std::move(resources));},"missing mandatory source provider rejected before publication");
        require(!n::is_bound()&&f.host.registrations==0&&f.host.sets==0,"failed factory constructor leaves no global binding or callback registration");
    }
    {
        Fixture f;auto resources=f.resources();resources.join_api.wait=nullptr;
        rejects<std::invalid_argument>([&]{f.construct(std::move(resources));},"partially supplied JoinApi is rejected rather than silently defaulted");
        require(!n::is_bound()&&f.host.registrations==0,"partial SDK table failure publishes nothing");
    }
}
void explicit_owned_teardown_and_borrowed_state(){
    for(bool owned:{false,true}){
        Fixture f;auto initial=b2_test::seed();auto& config=initial.configuration;
        config.executable_path=u"owned-teardown-fixture";config.content_id="retained-id";
        auto resources=f.resources();auto backing=f.lifetime;backing->state=&config;
        auto source=std::make_unique<b::SourceBackend>(f.runtime_seed(),std::move(resources));
        source->bind_state(config,{initial.console.show_lsx,initial.console.show_console,initial.console.no_console});
        f.lifetime.reset();backing.reset();
        if(owned){
            require(source->stop_and_join(),"Explicit owner establishes quiescence before aggregate storage release");
            source->retire_owned_state_037f70();
            require(f.destroyed_config=="seed-user","All backing/provider destructors see configuration before ordered storage cleanup");
            require(config.username.empty()&&config.content_id.empty()&&config.executable_path.empty()&&config.game_attributes.empty(),"Explicit owning route consumes actual bound configuration storage");
            require(source->state().libraries.loaded.empty()&&source->state().libraries.loaded.capacity()==0&&source->state().libraries.from_main.empty()&&source->state().libraries.from_worker.empty(),"Native loaded storage and separate host-only path copies retire without FreeLibrary");
            const auto events=f.track.events.size();source.reset();require(f.track.events.size()==events,"Later Backend destructor does not repeat opaque provider retirement");
        }else{
            source.reset();
            require(config.username=="seed-user"&&config.content_id=="retained-id"&&config.executable_path==u"owned-teardown-fixture","Ordinary borrowed-state Backend destruction preserves caller configuration");
            require(f.destroyed_config=="seed-user","Borrowed backing destructor observes original caller configuration");
        }
        require(absent(f.track.events,"library-free")&&absent(f.track.events,"online-destroy")&&absent(f.track.events,"hook-disable"),"Storage-only retirement adds no unload, online destroy or hook cleanup");
    }
}
struct ExecutionState {PSRWLOCK acquired=nullptr,released=nullptr;DWORD slept=0;bool frequency_ok=true,counter_ok=true;};
ExecutionState* execution_state=nullptr;
DWORD WINAPI thread_id(){return 0x456;}
void WINAPI acquire(PSRWLOCK value){execution_state->acquired=value;}
void WINAPI release(PSRWLOCK value){execution_state->released=value;}
BOOL WINAPI frequency(LARGE_INTEGER* value){value->QuadPart=9876543210123LL;::SetLastError(41);return execution_state->frequency_ok;}
BOOL WINAPI counter(LARGE_INTEGER* value){value->QuadPart=-654321012345LL;::SetLastError(42);return execution_state->counter_ok;}
void WINAPI sleep(DWORD value){execution_state->slept=value;}
r::ExecutionApi execution_api(){return {thread_id,acquire,release,frequency,counter,sleep};}
void sdk_execution_adapter(){
    b2_test::Track track;b2_test::Backend errors(track);ExecutionState state;execution_state=&state;
    r::NativeExecution native(errors,execution_api());std::uintptr_t storage=0;
    require(native.current_thread_id()==0x456,"NativeExecution preserves exact SDK thread identifier");
    native.acquire_srw_exclusive(storage);native.release_srw_exclusive(storage);
    require(state.acquired==reinterpret_cast<PSRWLOCK>(&storage)&&state.released==state.acquired,"SRW SDK boundary receives exact caller storage address");
    require(storage==0,"adapter does not reinitialize caller SRW storage");
    require(native.query_performance_frequency()==9876543210123LL&&native.query_performance_counter()==-654321012345LL,"QPF and QPC preserve full signed 64-bit values");
    native.sleep_milliseconds(0xffffffffu);require(state.slept==0xffffffffu,"sleep receives unchanged 32-bit operand");
    rejects<std::runtime_error>([&]{native.raise_thread_error_04a648(6);},"native thread error remains explicit lower Services policy");
    state.frequency_ok=false;bool frequency_error=false;
    try{(void)native.query_performance_frequency();}catch(const std::system_error& e){frequency_error=e.code().value()==41;}
    require(frequency_error,"QPF failure preserves Win32 error as explicit adapter exception");
    state.counter_ok=false;bool counter_error=false;
    try{(void)native.query_performance_counter();}catch(const std::system_error& e){counter_error=e.code().value()==42;}
    require(counter_error,"QPC failure preserves Win32 error as explicit adapter exception");
    for(unsigned missing=0;missing!=6;++missing){
        auto api=execution_api();switch(missing){case 0:api.thread_id=nullptr;break;case 1:api.acquire=nullptr;break;case 2:api.release=nullptr;break;case 3:api.frequency=nullptr;break;case 4:api.counter=nullptr;break;case 5:api.sleep=nullptr;break;}
        rejects<std::invalid_argument>([&]{r::NativeExecution absent(errors,api);},"incomplete execution SDK table is rejected");
    }
    const auto api=r::native_execution_api();
    require(api.thread_id==::GetCurrentThreadId&&api.acquire==::AcquireSRWLockExclusive&&api.release==::ReleaseSRWLockExclusive&&api.frequency==::QueryPerformanceFrequency&&api.counter==::QueryPerformanceCounter&&api.sleep==::Sleep,"native execution table binds actual SDK symbols without invoking user target");
    execution_state=nullptr;
}
}
int main(){try{
    composed_shutdown_retries();external_throw_reentry_and_foreign_handler();failed_registration_and_constructed_close();defaults_and_required_provider_graph();explicit_owned_teardown_and_borrowed_state();sdk_execution_adapter();
    std::cout<<"PASS: "<<b2_test::checks+b3_tracking_test::checks<<" bootstrap-runtime factory integration checks; actual B2/B3 source graph, post-join callback cleanup, stop/timeout retry, concrete adapter selection and SDK execution boundaries; lower foreign providers and PE entry remain explicit/open\n";
}catch(const std::exception& e){std::cerr<<"FAIL after "<<b2_test::checks+b3_tracking_test::checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<b2_test::checks+b3_tracking_test::checks<<" checks\n";return 1;}}
