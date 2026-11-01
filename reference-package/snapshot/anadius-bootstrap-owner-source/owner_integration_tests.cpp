#include "owner_test_fixture.hpp"
using namespace b2_test;
namespace {
void full_source_attach_and_drain(){
    Owned f;f.owner->publish();auto entry=f.owner->application_entry();require(entry(nullptr,1,nullptr)==TRUE,"source native entry reaches concrete facade and actual bootstrap process attach");
    require(f.owner->configuration().username=="override-user"&&f.owner->configuration().sdk=="1,2,3","actual file/parser/config startup shares owner state with override and SDK normalization");
    require(f.owner->configuration().language_from_arguments==1&&f.owner->console().show_lsx==1&&f.owner->console().show_console==1,"actual CLI source updates owned configuration and console flag bytes");
    require(f.backend->files.opened==std::vector<configuration::Path>{u"C:/fixture/anadius.cfg",u"C:/fixture/anadius_override.cfg"},"base and override are synthetic in-memory file providers in exact path order");
    require(f.backend->exit_callback==n::Resolver{}.exit_callback()&&f.backend->terminate_callback==n::Resolver{}.terminate_callback(),"bootstrap registration receives rebuilt native callbacks before worker creation");
    require(f.backend->allocated==std::vector<std::size_t>{8,8,16}&&f.backend->threads.pending.size()==3,"source bootstrap allocates exact three payloads and existing begin source retains pending contexts");
    require(f.backend->threads.closed==std::vector<t::Handle>{0x301,0x302,0x303}&&f.backend->threads.null_frees==3,"detached handle close is distinct from pending callback completion");
    auto& events=f.track.events;
    require(position(events,"register-exit")<position(events,"arguments")&&position(events,"arguments-publish")<position(events,"console-attach")&&position(events,"bootstrap-log-finish")<position(events,"executable"),"actual bootstrap/config/console orchestration order remains shared");
    require(position(events,"env:EAGameLocale")<position(events,"thread-1")&&position(events,"thread-1")<position(events,"between-workers")&&position(events,"between-workers")<position(events,"thread-2")&&position(events,"thread-2")<position(events,"sync-boolean")&&position(events,"sync-boolean")<position(events,"thread-3"),"post-config effects precede workers and synchronous true boolean precedes third thread");
    require(n::active_calls()==0&&f.backend->threads.pending.size()==3,"native entry lease ends while pending source callbacks still exist");
    f.backend->stop_success=false;require(!f.owner->close()&&n::is_bound()&&f.backend->threads.pending.size()==3,"quiescence false retains pending callback lifetime despite zero active calls");
    f.backend->stop_success=true;require(f.owner->close()&&!n::is_bound()&&f.backend->threads.pending.empty(),"backend drains queued rebuilt entries before owner unpublishes binding");
    require(f.backend->payloads.empty()&&f.backend->deleted==std::vector<std::size_t>{8,8,16},"actual noarg/boolean source callbacks delete exact owned payloads");
    require(position(events,"worker-first")<position(events,"worker-second")&&position(events,"worker-second")<position(events,"worker-boolean"),"controlled drain invokes both noarg workers and false boolean on actual callback sources");
    require(std::count(events.begin(),events.end(),"cleanup-thread")==3,"actual bootstrap callback bodies retain per-worker cleanup before deallocation");
    f.owner.reset();require(f.track.destroyed==1&&f.track.destroyed_state=="override-user","backend destructor observes live owned configured state after successful close");
}
void admitted_close_retained_and_retry(){
    Owned f;bool saw_retained=false;f.backend->drain=false;
    f.backend->guard_hook=[&]{require(n::active_calls()==1,"application callback admission is live during nested close");saw_retained=!f.owner->close();
        require(saw_retained&&f.track.stops==0&&f.owner->phase()==o::Phase::Stopping&&n::is_bound(),"admitted startup retains owner and binding before any backend stop/join");};
    f.owner->publish();auto entry=f.owner->application_entry();require(entry(nullptr,1,nullptr)==TRUE&&saw_retained&&n::active_calls()==0,"retained close lets admitted startup finish and release its native lease normally");
    f.backend->guard_hook={};f.backend->drain=true;require(f.owner->close()&&!n::is_bound()&&f.backend->payloads.empty(),"retry after callback exit drains pending work and closes retained binding");
}
void source_failure_prefix_before_publish(){
    Track track;Backend backend(track);auto initial=seed();o::BootstrapFacade facade(initial.configuration,initial.console,backend.providers());backend.state=&initial.configuration;
    recovered::Bootstrap bootstrap(facade);backend.threads.fail_create=true;
    rejects<std::runtime_error>([&]{bootstrap.initialize();},"source thread creation failure reaches mandatory throwing thread policy");
    require(initial.configuration.username=="override-user"&&backend.allocated==std::vector<std::size_t>{8}&&backend.threads.creations==1,"create failure occurs after complete shared config and first payload allocation");
    require(track.events.back()=="free-context"&&std::find(track.events.begin(),track.events.end(),"between-workers")==track.events.end(),"failed begin frees its context and stops later bootstrap operations");
    require(!n::is_bound()&&backend.payloads.size()==1,"direct source failure adds no implicit binding or invented payload rollback");
}
}
int main(){try{full_source_attach_and_drain();admitted_close_retained_and_retry();source_failure_prefix_before_publish();std::cout<<"PASS: "<<checks<<" bootstrap-owner integration checks; actual config/parser/bootstrap/native callbacks and source begin-thread share owned state; pending drain/admitted-startup retry/error prefixes; OS workers/backend/CRT entry remain controlled/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
