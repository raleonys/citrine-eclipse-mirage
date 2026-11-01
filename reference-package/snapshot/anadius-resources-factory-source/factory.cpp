#include "factory.hpp"
#include <stdexcept>

namespace anadius::resources_factory {
namespace {
template<class T>bool same_or_empty(const std::shared_ptr<T>& selected,const std::shared_ptr<T>& root){return !selected||selected==root;}
bool stream_matches(const std::shared_ptr<std::ostream>& selected,const std::shared_ptr<shared_log::Stream>& root){return !selected||selected.get()==static_cast<std::ostream*>(root.get());}
struct RemainingState {services_factory::Options options;bool used=false;};
struct WorkerState {
    std::shared_ptr<thread_domain::DomainStorage> domain;
    application_worker::Settings settings;bool used=false;
    WorkerState(std::shared_ptr<thread_domain::DomainStorage> d,application_worker::Settings& s):domain(std::move(d)),settings(application_worker::take_settings(s)){}
};
}
void install(bootstrap_backend::Resources& r,Inputs& in){
    if(in.consumed_)throw std::logic_error("Resources inputs already consumed");
    if(r.threads||r.remaining||r.remaining_factory||r.online||r.online_factory||r.server||r.application_worker||r.library_effects)
        throw std::invalid_argument("Selected Resources target slots must be empty");
    if(!r.configuration||!dynamic_cast<achievement_storage::PathBinding*>(r.configuration.get())||!r.console||!r.host||!r.hook_cleanup||!r.stop_sources)
        throw std::invalid_argument("Path-bound configuration, console, host, cleanup and admission stopper required");
    const auto& t=in.threads;const auto& o=in.online;const auto& w=in.worker;const auto& e=in.effects;const auto& l=in.library;
    if(!in.domain||in.domain->phase()!=thread_domain::Phase::Ready||!in.stream||!in.dialog||!in.waits||in.waits->active())
        throw std::invalid_argument("Ready shared Domain, managed stream/dialog and quiescent explicit wait registry required");
    if(!t.create||!t.get_module||!t.last_error||!t.close||!t.release||!t.release_and_exit||!t.exit)
        throw std::invalid_argument("Complete native thread API required");
    if(!w.lifetime||!w.configure||!w.state||w.effects||!w.startup_timeout_ms||w.startup_timeout_ms==INFINITE)
        throw std::invalid_argument("Complete uncomposed Worker settings required");
    if(!o.os||!o.encoding||!o.errors||!o.stats||!o.files||!o.get_proc||!o.code_lease||!o.foreign_quiescent||!l.errors||!e.attempts)
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
