#include "factory.hpp"
#include "../anadius-host-platform-source/assembly.hpp"
#include "../anadius-socket-move-source/move.hpp"
#include "../anadius-owned-jobs-source/jobs.hpp"
#include "../anadius-bootstrap-providers-source/native_threads.hpp"
#include <stdexcept>

namespace anadius::resources_factory {
namespace {
struct Error {
    DWORD value=::GetLastError();
    ~Error(){::SetLastError(value);}
    void restore()const{::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
struct WorkPrerequisite {
    // Raw captures retire before backing, then the exact callback Domain.
    std::shared_ptr<const host_platform::SourceStopReceipt> receipt;
    std::shared_ptr<void> backing;
    std::function<bool()> raw;
    bool ready(){
        Error error;
        if(!receipt->completed())return false;
        error.restore();bool drained;
        try{drained=raw();}catch(...){error.capture();throw;}
        error.capture();
        return drained&&receipt->completed();
    }
};
template<class T>bool same_or_empty(const std::shared_ptr<T>& selected,const std::shared_ptr<T>& root){return !selected||selected==root;}
bool stream_matches(const std::shared_ptr<std::ostream>& selected,const std::shared_ptr<shared_log::Stream>& root){return !selected||selected.get()==static_cast<std::ostream*>(root.get());}
struct RemainingState {services_factory::Options options;bool used=false;};
struct WorkerState {
    std::shared_ptr<thread_domain::DomainStorage> domain;
    application_worker::Settings settings;bool used=false;
    WorkerState(std::shared_ptr<thread_domain::DomainStorage> d,application_worker::Settings& s):domain(std::move(d)),settings(application_worker::take_settings(s)){}
};
bool worker_io_empty(const application_worker::Inputs& in){
    const auto& s=in.server;const auto& r=in.runtime;
    return !s.sockets&&!s.iocp&&!s.gates&&!s.dispatcher&&!r.sockets&&!r.dispatcher&&!r.accept&&!r.factory&&!r.cancel;
}
}
void select_native_worker_io(application_worker::Inputs& in,WorkerIoImports api){
    Error error;
    if(!worker_io_empty(in))throw std::invalid_argument("Native worker IO target slots must be empty");
    const auto& s=api.sockets;const auto& i=api.iocp;const auto& g=api.gates;
    const auto& d=api.dispatcher;const auto& a=api.accept;
    if(!s.create||!s.attach||!s.last||!s.option||!s.bind||!s.listen||!s.close||!s.ioctl||
       !i.set_version_mask||!i.verify_version||!i.create_completion_port||!i.get_last_error||!i.close_handle||
       !g.initialize||!g.get_last_error||!d.set_last_error||!d.get_last_error||!d.get||!d.post||!d.set_timer||
       !a.accept||!a.addresses||!a.option||!a.error||!api.factory.open||!api.cancel)
        throw std::invalid_argument("Complete native worker IO imports required");
    auto sockets=std::make_shared<socket_recovered::NativeAcceptorApi>(api.sockets);
    auto iocp=std::make_shared<socket_recovered::NativeIocpConstructorApi>(api.iocp);
    auto gates=std::make_shared<socket_recovered::NativeServiceGateApi>(api.gates);
    auto dispatcher=std::make_shared<socket_recovered::NativeDispatcherApi>(api.dispatcher);
    auto accept=std::make_shared<socket_recovered::NativeAsyncAcceptApi>(api.accept);
    auto factory=std::make_shared<socket_recovered::NativeSocketFactoryApi>(api.factory);
    auto cancel=std::make_shared<socket_recovered::NativeRetryCancelApi>(api.cancel);
    // A failing allocation leaves all target slots uncommitted. Global
    // allocator/new-handler code must not silently replace these targets.
    if(!worker_io_empty(in))throw std::logic_error("Native worker IO targets changed during construction");
    in.server.sockets=sockets;in.runtime.sockets=std::move(sockets);
    in.server.dispatcher=dispatcher;in.runtime.dispatcher=std::move(dispatcher);
    in.server.iocp=std::move(iocp);in.server.gates=std::move(gates);
    in.runtime.accept=std::move(accept);in.runtime.factory=std::move(factory);in.runtime.cancel=std::move(cancel);
}
void select_recovered_socket_executor(application_worker::Inputs& in){
    Error error;
    const auto empty=[&]{return !in.server.executor&&!in.runtime.executor.context&&!in.runtime.executor.move;};
    if(!empty())throw std::invalid_argument("Socket executor target slots must be empty");
    auto executor=std::make_shared<socket_recovered::RecoveredSocketExecutor>();
    if(!empty())throw std::logic_error("Socket executor targets changed during construction");
    in.runtime.executor=socket_recovered::RecoveredSocketExecutor::move();
    in.server.executor=std::move(executor);
}
void select_source_worker_lifecycle(application_worker::Inputs& in){
    Error error;auto& lifecycle=in.graph.policy.lifecycle;
    if(lifecycle.mode!=session_policy::LifecycleMode::Explicit||lifecycle.select_owner||!lifecycle.external_drained)
        throw std::invalid_argument("Source lifecycle selection requires an unselected owner and explicit external drain proof");
    lifecycle.mode=session_policy::LifecycleMode::SourceOwned;
}
void select_original_worker_lifecycle(application_worker::Inputs& in){
    Error error;auto& lifecycle=in.graph.policy.lifecycle;
    if(lifecycle.mode!=session_policy::LifecycleMode::Explicit||lifecycle.select_owner||lifecycle.external_drained||lifecycle.source_stop)
        throw std::invalid_argument("Original lifecycle requires empty owner/external-proof/source-stop slots");
    lifecycle.mode=session_policy::LifecycleMode::OriginalApplication;
}
void configure_native_worker(application_worker::Inputs& in,std::shared_ptr<shared_log::Stream> stream,std::string version){
    Error error;
    const auto fresh=[&]{const auto& l=in.graph.policy.lifecycle;return worker_io_empty(in)&&
        !in.server.executor&&!in.runtime.executor.context&&!in.runtime.executor.move&&
        !in.graph.policy.stream&&in.graph.version.empty()&&l.mode==session_policy::LifecycleMode::Explicit&&
        !l.lifetime&&!l.select_owner&&!l.after&&!l.returned&&!l.external_drained&&!l.source_stop;};
    if(!stream||!fresh())throw std::invalid_argument("Fresh native Worker IO/executor/lifecycle and managed stream required");
    application_worker::Inputs selected;
    select_native_worker_io(selected);select_recovered_socket_executor(selected);select_original_worker_lifecycle(selected);
    // Native adapter constructors acquire no SDK resource. Failure above
    // leaves the actual Worker Pending inputs untouched. No opaque target is
    // copied or destroyed during the final empty-slot commit.
    if(!fresh())throw std::logic_error("Native Worker target changed during construction");
    in.server.sockets=std::move(selected.server.sockets);in.server.iocp=std::move(selected.server.iocp);
    in.server.gates=std::move(selected.server.gates);in.server.dispatcher=std::move(selected.server.dispatcher);
    in.server.executor=std::move(selected.server.executor);
    in.runtime.sockets=std::move(selected.runtime.sockets);in.runtime.dispatcher=std::move(selected.runtime.dispatcher);
    in.runtime.accept=std::move(selected.runtime.accept);in.runtime.factory=std::move(selected.runtime.factory);
    in.runtime.cancel=std::move(selected.runtime.cancel);in.runtime.executor=selected.runtime.executor;
    in.graph.policy.stream=std::move(stream);in.graph.version.swap(version);
    in.graph.policy.lifecycle.mode=session_policy::LifecycleMode::OriginalApplication;
}
void select_native_message_jobs(application_worker::Inputs& in,std::shared_ptr<bootstrap_providers::NativeThreads> threads){
    Error error;
    const auto fresh=[&]{const auto& lifecycle=in.graph.policy.lifecycle;return !in.graph.profile.jobs&&lifecycle.mode==session_policy::LifecycleMode::OriginalApplication&&session_policy::lifecycle_configuration_valid(lifecycle);};
    if(!threads||!threads->memory_domain()||threads->memory_domain()->phase()!=thread_domain::Phase::Ready||!fresh())
        throw std::invalid_argument("Original Worker, vacant jobs slot and Ready Domain NativeThreads required");
    auto jobs=std::make_shared<owned_jobs::NativeJobs>(std::move(threads),in.graph.profile.jobs_capacity);
    if(!fresh())throw std::logic_error("Native message jobs selection changed during construction");
    in.graph.profile.jobs=std::move(jobs);
}
void select_worker_stop(Inputs& in,const host_platform::Assembly& host,const accept_stop::Options& options){
    if(in.consumed_)throw std::logic_error("Resources inputs consumed or reserved");
    if(in.worker.accept_stop)throw std::invalid_argument("Worker stop slot must be empty");
    if(!in.domain||in.domain->phase()!=thread_domain::Phase::Ready||!host.callbacks||!host.stopped_sources||
       host.stopped_sources->callbacks_identity()!=host.callbacks.get()||host.callbacks->memory_domain()!=in.domain.get()||
       host.stopped_sources->memory_domain()!=in.domain.get())
        throw std::invalid_argument("Exact Ready callback/source-stop/memory Domain required");
    if(!options.lifetime||!options.other_work_drained)
        throw std::invalid_argument("Real stop backing and independent work-drain proof required");
    // Declare before all opaque copies: their retirement remains reserved too.
    struct Reservation {bool& flag;explicit Reservation(bool& f):flag(f){flag=true;}~Reservation(){flag=false;}} reserved{in.consumed_};
    auto proof=std::make_shared<WorkPrerequisite>();
    proof->receipt=host.stopped_sources;proof->backing=options.lifetime;
    proof->raw=options.other_work_drained;
    accept_stop::Options selected_options;
    selected_options.lifetime=proof;
    selected_options.imports=options.imports;selected_options.capacity=options.capacity;
    selected_options.post_drain_timeout_ms=options.post_drain_timeout_ms;selected_options.scope=options.scope;
    selected_options.other_work_drained=[proof]{return proof->ready();};
    auto selected=accept_stop::Stop::create(std::move(selected_options));
    // Only a completely constructed Fresh provider is committed. Public Inputs
    // fields must not be concurrently/directly mutated by opaque caller code.
    in.worker.accept_stop=std::move(selected);
}
void install(bootstrap_backend::Resources& r,Inputs& in){
    if(in.consumed_)throw std::logic_error("Resources inputs already consumed");
    if(r.threads||r.remaining||r.remaining_factory||r.online||r.online_factory||r.server||r.application_worker||r.library_effects)
        throw std::invalid_argument("Selected Resources target slots must be empty");
    if(!r.configuration||!dynamic_cast<achievement_storage::PathBinding*>(r.configuration.get())||!r.console||!r.host||!r.hook_cleanup||!bootstrap_backend::lifetime_configuration_valid(r))
        throw std::invalid_argument("Path-bound configuration, console, host, cleanup and selected lifetime configuration required");
    const auto& t=in.threads;const auto& o=in.online;const auto& w=in.worker;const auto& e=in.effects;const auto& l=in.library;
    if(!in.domain||in.domain->phase()!=thread_domain::Phase::Ready||!in.stream||!in.dialog||!in.waits||in.waits->active())
        throw std::invalid_argument("Ready shared Domain, managed stream/dialog and quiescent explicit wait registry required");
    if(!t.create||!t.get_module||!t.last_error||!t.close||!t.release||!t.release_and_exit||!t.exit)
        throw std::invalid_argument("Complete native thread API required");
    if(!w.lifetime||!w.configure||!w.state||w.effects||!w.startup_timeout_ms||w.startup_timeout_ms==INFINITE)
        throw std::invalid_argument("Complete uncomposed Worker settings required");
    if(w.accept_stop&&w.accept_stop->snapshot().phase!=accept_stop::Phase::Fresh)
        throw std::invalid_argument("Controller-selected Worker stop must be Fresh before Resources consumption");
    if(!o.os||!o.encoding||!o.errors||!o.stats||!o.files||!o.get_proc||!o.code_lease||!online_factory::lifetime_configuration_valid(o)||!l.errors||!e.attempts)
        throw std::invalid_argument("Complete owned Online/library providers and explicit attempt cells required");
    if(!stream_matches(o.stream,in.stream)||!stream_matches(l.stream,in.stream)||!stream_matches(e.information.stream,in.stream)||!stream_matches(e.content.stream,in.stream)||!stream_matches(e.licensing.stream,in.stream)||
       !same_or_empty(o.dialog,in.dialog)||!same_or_empty(l.dialog,in.dialog)||!same_or_empty(e.content.dialog,in.dialog)||!same_or_empty(e.licensing.dialog,in.dialog))
        throw std::invalid_argument("All selected providers must share exact managed stream/dialog");
    if(!e.information.clock||!e.information.formatter||!e.content.filesystem||!e.content.files||!e.content.clock||!e.content.formatter||!e.content.encoding||!e.content.errors||!e.content.atoi||!e.content.copy_file||
       !e.licensing.files||!e.licensing.text||!e.licensing.stats||!e.licensing.encoding||!e.licensing.errors||!e.licensing.clock||!e.licensing.formatter||!e.licensing.module||!e.licensing.procedure||!e.licensing.exit_process)
        throw std::invalid_argument("Complete owned concrete worker effect providers required");
    in.consumed_=true;
    in.library.stream=in.stream;in.library.dialog=in.dialog;
    in.online.stream=in.stream;in.online.dialog=in.dialog;
    in.effects.information.stream=in.stream;in.effects.content.stream=in.stream;in.effects.licensing.stream=in.stream;
    in.effects.content.dialog=in.dialog;in.effects.licensing.dialog=in.dialog;
    in.worker.effects=std::make_shared<application_effects::Factory>(std::move(in.effects));
    r.library_effects=std::make_unique<library_effects::BoundLibrary>(std::move(in.library));
    r.threads=in.domain->make_threads(in.threads);
    auto remaining=std::make_shared<RemainingState>();
    remaining->options.lifetime=r.state_lifetime;
    remaining->options.memory=in.domain->make_memory();
    remaining->options.logger=std::make_shared<services_completion::BootstrapLogger>(in.stream);
    remaining->options.waits=in.waits;
    r.remaining_factory=[remaining](bootstrap_runtime::CallbackRegistry& registry){
        if(remaining->used)throw std::logic_error("Remaining factory binds once");remaining->used=true;
        return services_factory::make_remaining(registry,std::move(remaining->options));
    };
    online_factory::install_online(r,in.online);
    auto worker=std::make_shared<WorkerState>(in.domain,in.worker);
    r.application_worker=[worker](bootstrap_backend::CallbackStateView state)->std::unique_ptr<bootstrap_backend::ApplicationWorker>{
        if(worker->used)throw std::logic_error("Application worker factory binds once");worker->used=true;
        return application_worker::Worker::create(worker->domain,state,application_worker::take_settings(worker->settings));
    };
}
}
