#include "owner_test_fixture.hpp"
using namespace b2_test;
namespace {
void construction_and_publication(){
    {Track t;rejects<std::invalid_argument>([&]{o::RuntimeOwner value(seed(),nullptr,permanent_errors);},"null mandatory provider owner rejected");require(!n::is_bound(),"null construction never publishes native binding");}
    {Track t;auto backend=std::make_unique<Backend>(t);backend->bind_failure=true;rejects<std::runtime_error>([&]{o::RuntimeOwner value(seed(),std::move(backend),permanent_errors);},"state binding exception propagates");require(t.provider_queries==1&&t.binds==1&&t.destroyed==1&&t.destroyed_state=="seed-user","failed construction frees providers while owned state is live");require(!n::is_bound(),"partial construction never becomes native current owner");}
    {Owned f;require(f.track.provider_queries==1&&f.track.binds==1&&f.backend->state==&f.owner->configuration(),"one provider query and one stable owner state binding");require(f.owner->configuration().username=="seed-user"&&f.owner->console().show_lsx==0,"caller-owned initial values retained without guessed globals");require(f.owner->phase()==o::Phase::Constructed&&!n::is_bound(),"constructed owner is unpublished");rejects<std::logic_error>([&]{f.owner->application_entry();},"entry unavailable before publication");require(f.owner->close()&&f.owner->close()&&f.track.stops==0,"unpublished close and repeated close do not request worker drain");rejects<std::logic_error>([&]{f.owner->publish();},"closed owner cannot republish");f.owner.reset();require(f.track.destroyed==1&&f.track.destroyed_state=="seed-user","normal unpublished destruction retains shared state until provider destructor");}
    {Owned first;first.owner->publish();require(n::is_bound()&&first.owner->phase()==o::Phase::Bound,"fully constructed owner publishes exact native binding");require(first.owner->application_entry()==n::Resolver{}.application(),"owner exposes rebuilt typed application entry");rejects<std::logic_error>([&]{first.owner->publish();},"duplicate owner publish rejected");Owned second;bool occupied=false;try{second.owner->publish();}catch(const Fault& f){occupied=f.reason==n::LifecycleFault::AlreadyBound;}require(occupied&&second.owner->phase()==o::Phase::Constructed&&n::is_bound(),"second publication fails without losing first binding or corrupting second phase");require(first.owner->close()&&!n::is_bound(),"first close releases global binding");second.owner->publish();require(second.owner->close()&&!n::is_bound(),"second unpublished owner can retry after occupancy ends");}
}
void close_prefixes_and_reentrancy(){
    {Owned f;f.owner->publish();f.backend->stop_success=false;require(!f.owner->close()&&f.owner->phase()==o::Phase::Stopping&&n::is_bound(),"failed quiescence preserves native binding and stopping owner");require(f.track.stops==1&&n::active_calls()==0,"worker drain is required even with zero admitted native calls");rejects<std::logic_error>([&]{f.owner->application_entry();},"stopping owner no longer hands out entry pointers");f.backend->stop_success=true;f.backend->stop_failure=true;rejects<std::runtime_error>([&]{f.owner->close();},"drain exception propagates without silently closing");require(f.owner->phase()==o::Phase::Stopping&&n::is_bound()&&!f.track.destroyed,"exception retains owner providers and binding");f.backend->stop_failure=false;require(f.owner->close()&&f.owner->phase()==o::Phase::Closed&&!n::is_bound(),"retry after stop failure closes cleanly");const auto calls=f.track.stops;require(f.owner->close()&&f.track.stops==calls,"closed owner close is idempotent");}
    {Owned f;f.owner->publish();f.backend->stop_hook=[&]{rejects<std::logic_error>([&]{f.owner->close();},"reentrant close rejected before a second drain");rejects<std::logic_error>([&]{f.owner->publish();},"reentrant publish rejected during close");};require(f.owner->close()&&f.track.stops==1&&!n::is_bound(),"outer close retains exclusive control and completes");}
    {Owned f;f.owner->publish();f.backend->stop_hook=[&]{f.owner->close();};rejects<std::logic_error>([&]{f.owner->close();},"uncaught nested close unwinds outer control scope");require(f.owner->phase()==o::Phase::Stopping&&f.track.stops==1&&n::is_bound(),"nested failure leaves committed stopping state and live binding");f.backend->stop_hook={};require(f.owner->close()&&!n::is_bound(),"control guard resets on exception so close can retry");}
}
void facade_sources_and_failures(){
    Track tr;Backend backend(tr);auto initial=seed();o::BootstrapFacade facade(initial.configuration,initial.console,backend.providers());
    require(static_cast<recovered::BootstrapDependencies*>(&facade)!=nullptr,"single BootstrapDependencies base is unambiguous");
    require(facade.register_exit(recovered::RegisteredCallback::exit_038950)==-1&&backend.exit_callback==n::Resolver{}.exit_callback(),"register_exit routes actual rebuilt callback and preserves returned status");
    facade.install_terminate(recovered::RegisteredCallback::terminate_038890);require(backend.terminate_callback==n::Resolver{}.terminate_callback(),"terminate callback routes typed rebuilt entry");
    rejects<std::invalid_argument>([&]{facade.register_exit(recovered::RegisteredCallback::terminate_038890);},"wrong exit identity rejected before policy call");
    rejects<std::invalid_argument>([&]{facade.install_terminate(recovered::RegisteredCallback::exit_038950);},"wrong terminate identity rejected before policy call");
    recovered::NoArgumentPayload payload{&Backend::first};std::uint32_t id=0;
    const auto handle=facade.begin_thread_05e8e0(recovered::ThreadEntry::no_argument_0388a0,&payload,&id);
    require(handle==0x301&&id==11&&backend.threads.null_frees==1&&backend.threads.pending.size()==1,"facade reuses actual begin_thread source including free(nullptr) and id publication");
    const auto* context=backend.threads.pending[0];require(context->entry==reinterpret_cast<std::uintptr_t>(n::Resolver{}.thread_entry(recovered::ThreadEntry::no_argument_0388a0))&&context->argument==reinterpret_cast<std::uintptr_t>(&payload),"source context contains rebuilt callback code and unchanged payload identity");
    require(facade.close_handle(handle)&&backend.threads.closed==std::vector<t::Handle>{handle},"handle close shares existing thread Effects rather than a second policy");
    backend.threads.pending.clear();backend.service_failure="bootstrap-log-finish";recovered::Bootstrap bootstrap(facade);
    rejects<std::runtime_error>([&]{bootstrap.initialize();},"source bootstrap propagates supplied logging failure");require(tr.events.back()=="bootstrap-log-finish"&&backend.allocated.empty()&&backend.files.opened.empty(),"failure prefix stops config and workers without fabricated rollback");
    backend.service_failure.clear();bootstrap.on_exit();require(position(tr.events,"cleanup-first")<position(tr.events,"cleanup-second")&&position(tr.events,"cleanup-second")<position(tr.events,"cleanup-network"),"source exit reuses ordered three mandatory cleanup policies");
}
int terminal_mode(const std::string& mode){
    std::set_terminate([]{std::_Exit(89);});
    {Track track;auto backend=std::make_unique<Backend>(track);auto* raw=backend.get();o::RuntimeOwner owner(seed(),std::move(backend),permanent_errors);owner.publish();
        if(mode=="--stopping-owner"){raw->stop_success=false;owner.close();}
        else if(mode=="--post-close"){owner.close();}
    }
    if(mode=="--post-close")n::Resolver{}.application()(nullptr,0,nullptr);
    return 90;
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_mode(argv[1]);
    try{construction_and_publication();close_prefixes_and_reentrancy();facade_sources_and_failures();
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--live-owner",static_cast<const char*>(nullptr))==89,"bound owner destruction is explicit terminal contract");
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--stopping-owner",static_cast<const char*>(nullptr))==89,"stopping owner destruction is explicit terminal contract");
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--post-close",static_cast<const char*>(nullptr))==86,"process-lifetime error policy survives closed and destroyed owner");
        std::cout<<"PASS: "<<checks<<" bootstrap-owner core checks; single-base source facade,publication/quiescence/reentrant failure prefixes and permanent-error lifetime; native backend/CRT entry remain open\n";
    }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}
}
