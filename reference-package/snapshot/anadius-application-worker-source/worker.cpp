#include "worker.hpp"
#include <exception>
#include <mutex>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace anadius::application_worker {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
}
Settings take_settings(Settings& s){
    auto configure=std::move(s.configure);s.configure={};
    // Guaranteed-prvalue aggregate: receiver owns all backing before the local
    // std::function donor retires, including non-NRVO/SBO implementations.
    return Settings{std::move(s.lifetime),std::move(s.state),std::move(s.effects),std::move(s.accept_stop),
        std::move(configure),std::exchange(s.startup_timeout_ms,0),std::exchange(s.close_retry_timeout_ms,0)};
}
struct Worker::Impl {
    struct Cells {
        std::shared_ptr<void> lifetime;Settings::StateSeed state;
    };
    struct Pending {
        Inputs inputs;std::shared_ptr<Cells> cells;
        // Capture before Graph consumes its options. Only the explicit original
        // application profile omits the additional HOST startup milestone wait.
        bool original_application=false;
        application_effects::Providers effects;
        std::optional<worker_handoff::PreparedMemory::Borrow> borrow;
        b::Resources resources;std::shared_ptr<g::Graph> graph;
        std::uint32_t tls=TLS_OUT_OF_INDEXES;
    };
    const DWORD controller=::GetCurrentThreadId();void* const controller_fiber=::GetCurrentFiber();
    const std::shared_ptr<thread_domain::DomainStorage> domain;
    const b::CallbackStateView bound;Settings settings;
    achievement_storage::Paths* shared_paths=nullptr;
    const license_recovered::AuthenticationCode* authentication_code=nullptr;
    execution_recovered::MutexState* achievement_mutex=nullptr;
    execution_recovered::Platform* achievement_platform=nullptr;
    b::OnlineComponent* effects_online=nullptr;
    std::unique_ptr<worker_handoff::PreparedMemory> memory;
    HANDLE changed=nullptr;mutable std::mutex gate;Phase phase=Phase::Prepared;
    DWORD worker=0;void* fiber=nullptr;void* ptd=nullptr;
    bool online=false,host=false,stop=false,entered=false,admitted=false,pending_present=false,online_binding=false,stop_requesting=false;
    bool close_retry_requested=false;std::uint64_t close_epoch=0,close_retries=0;
    std::unique_ptr<Pending> pending; // worker publishes BEFORE opaque configure
    Impl(std::shared_ptr<thread_domain::DomainStorage> d,b::CallbackStateView v,Settings&& s)
        :domain(std::move(d)),bound(v),settings(take_settings(s)),memory(worker_handoff::PreparedMemory::prepare(domain)){
        changed=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
        if(!changed){if(!memory->cancel())std::terminate();throw std::runtime_error("Application worker startup event creation failed");}
    }
    ~Impl()noexcept{if(phase!=Phase::Retiring||pending||changed||stop_requesting)std::terminate();}
    void control()const{
        if(::GetCurrentThreadId()!=controller||::GetCurrentFiber()!=controller_fiber)throw std::logic_error("Application worker lifecycle belongs to its original controller/fiber");
    }
    void signal_locked()noexcept{
        // Direct nonthrowing kernel event metadata only; no opaque callback.
        if(changed&&!::SetEvent(changed))phase=Phase::Poisoned;
    }
    void poison()noexcept{
        std::lock_guard lock(gate);if(phase==Phase::Closed||phase==Phase::Retiring||phase==Phase::Finishing)return;
        phase=Phase::Poisoned;signal_locked();
    }
    void check(){
        std::lock_guard lock(gate);
        if(::GetCurrentThreadId()!=worker||::GetCurrentFiber()!=fiber||::FlsGetValue(domain->native_fls_index())!=ptd){
            phase=Phase::Poisoned;throw std::logic_error("Application worker thread/fiber/Domain PTD changed");
        }
        if(phase==Phase::Poisoned||phase==Phase::Closed||phase==Phase::Finishing||(!admitted&&stop))throw std::logic_error("Application worker prefix is closed, stopped or unknown");
    }
    void startup_barrier(){
        check();
        if(pending->original_application){
            std::lock_guard lock(gate);
            if(phase==Phase::Poisoned||stop)throw std::logic_error("Original application dispatch lost Worker admission");
            admitted=true;phase=Phase::Serving;return;
        }
        const auto begin=::GetTickCount64();
        for(;;){
            {
                std::lock_guard lock(gate);
                if(phase==Phase::Poisoned||stop)throw std::logic_error("Started application worker cannot infer drain from startup cancellation");
                if(online&&host){admitted=true;phase=Phase::Serving;return;}
                phase=Phase::WaitingStartup;
                if(!::ResetEvent(changed)){phase=Phase::Poisoned;throw std::runtime_error("Cannot reset startup event");}
            }
            const auto elapsed=::GetTickCount64()-begin;
            if(elapsed>=settings.startup_timeout_ms)throw std::runtime_error("Online/host startup completion timed out; started source retained");
            const auto result=::WaitForSingleObject(changed,settings.startup_timeout_ms-static_cast<DWORD>(elapsed));
            if(result!=WAIT_OBJECT_0)throw std::runtime_error("Online/host startup wait did not complete; started source retained");
            check();
        }
    }
    void wait_close_retry(ULONGLONG begin){
        check();
        if(!settings.close_retry_timeout_ms)throw std::logic_error("Application worker close continuation is not selected");
        // Graph/server calls and their Parent Operation have fully unwound.
        // Validate the retained exact owned server, not just an exception name.
        const auto backend=pending->graph->backend().snapshot();
        const auto graph=pending->graph->snapshot().phase;
        if(backend.phase!=owned_server::Phase::Quiescing||backend.quiescence_attested||!backend.acceptor_initialized||!backend.pending_initialized||backend.active_calls||
           (graph!=g::Phase::Bound&&graph!=g::Phase::Installed&&graph!=g::Phase::Drained))
            throw std::logic_error("Close continuation requires a known retained false drain");
        {
            std::lock_guard lock(gate);
            if(phase!=Phase::Serving||!pending_present||!admitted||close_epoch==std::numeric_limits<std::uint64_t>::max())throw std::logic_error("Invalid close continuation reservation");
            if(!::ResetEvent(changed)){phase=Phase::Poisoned;throw std::runtime_error("Cannot reset close continuation event");}
            close_retry_requested=false;++close_epoch;phase=Phase::WaitingClose;
        }
        for(;;){
            check();
            const auto elapsed=::GetTickCount64()-begin;
            if(elapsed>=settings.close_retry_timeout_ms)throw std::runtime_error("Application worker close continuation timed out; owners retained");
            {
                std::lock_guard lock(gate);
                if(phase!=Phase::WaitingClose)throw std::logic_error("Close continuation wait was poisoned");
                if(close_retry_requested){close_retry_requested=false;++close_retries;phase=Phase::Serving;return;}
                // Stop/startup notifications wake us but cannot authorize retry.
                if(!::ResetEvent(changed)){phase=Phase::Poisoned;throw std::runtime_error("Cannot reset close continuation wake");}
            }
            if(::WaitForSingleObject(changed,settings.close_retry_timeout_ms-static_cast<DWORD>(elapsed))!=WAIT_OBJECT_0)
                throw std::runtime_error("Application worker close continuation wait failed; owners retained");
        }
    }
    struct Proxy final:server_recovered::ServerBackend {
        Impl& v;explicit Proxy(Impl& x):v(x){}
        template<class F>std::invoke_result_t<F> invoke(F&& f){
            v.check();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.check();}
            else{std::invoke_result_t<F> result=f();v.check();return result;}
        }
        server_recovered::ServerBackend& inner(){return *v.pending->resources.server;}
        void initialize_context_0241b0()override{invoke([&]{inner().initialize_context_0241b0();});}
        server_recovered::ParsedAddress parse_address_028690(std::uint16_t f,std::string_view s)override{return invoke([&]{return inner().parse_address_028690(f,s);});}
        void construct_acceptor_020e20(const server_recovered::Endpoint& e,bool r,int n)override{invoke([&]{inner().construct_acceptor_020e20(e,r,n);});}
        void initialize_pending_socket_0211b0()override{invoke([&]{inner().initialize_pending_socket_0211b0();});}
        server_recovered::LocalEndpoint local_endpoint()override{return invoke([&]{return inner().local_endpoint();});}
        bool set_environment_variable(std::string_view k,std::string_view s)override{return invoke([&]{return inner().set_environment_variable(k,s);});}
        void async_accept_021b00()override{invoke([&]{inner().async_accept_021b00();});}
        server_recovered::Error run_context_0298b0()override{
            // Outside Graph::Server::invoke and Parent Operation. The explicit
            // original profile admits without adding an online/host wait;
            // recovered source bodies retain their own Execution waits.
            v.startup_barrier();return invoke([&]{return inner().run_context_0298b0();});
        }
        void destroy_pending_socket_024db0()override{
            std::optional<ULONGLONG> begin;
            for(;;){try{invoke([&]{inner().destroy_pending_socket_024db0();});return;}
                catch(const owned_server::QuiescenceDeferred&){if(!begin)begin=::GetTickCount64();v.wait_close_retry(*begin);}}
        }
        void destroy_acceptor_024db0()override{invoke([&]{inner().destroy_acceptor_024db0();});}
        void destroy_context()override{invoke([&]{inner().destroy_context();});}
        [[noreturn]]void raise_error(server_recovered::Error e,std::string_view s)override{invoke([&]{inner().raise_error(e,s);});std::terminate();}
    };
    bool finish(bool unpublished){
        PreserveError error;control();Phase prior;
        {
            std::lock_guard lock(gate);prior=phase;
            if(phase==Phase::Closed)return true;
            if(online_binding||stop_requesting){phase=Phase::Poisoned;return false;}
            if(unpublished&&(entered||phase!=Phase::Prepared))return false;
            if(phase!=Phase::Prepared&&phase!=Phase::Returned)return false;
            if(pending_present)return false;
            phase=Phase::Finishing;
        }
        bool done=false;
        try{done=prior==Phase::Prepared?memory->cancel():memory->close(0);}
        catch(...){std::lock_guard lock(gate);phase=Phase::Poisoned;signal_locked();throw;}
        if(!done){const auto state=memory->snapshot().phase;std::lock_guard lock(gate);phase=state==worker_handoff::Phase::Poisoned?Phase::Poisoned:prior;return false;}
        if(!::CloseHandle(changed)){std::lock_guard lock(gate);phase=Phase::Poisoned;return false;}
        {std::lock_guard lock(gate);changed=nullptr;phase=Phase::Closed;}return true;
    }
};
Worker::Worker(std::unique_ptr<Impl> p)noexcept:impl_(std::move(p)){}
std::unique_ptr<Worker> Worker::create(std::shared_ptr<thread_domain::DomainStorage> domain,b::CallbackStateView bound,Settings settings){
    PreserveError error;
    if(!domain||!settings.lifetime||!settings.configure||!settings.state||!settings.startup_timeout_ms||settings.startup_timeout_ms==INFINITE||settings.close_retry_timeout_ms==INFINITE)
        throw std::invalid_argument("Application worker requires a Domain, owning configure provider, explicit state seeds and finite startup timeout");
    if(settings.accept_stop&&settings.accept_stop->snapshot().phase!=accept_stop::Phase::Fresh)
        throw std::invalid_argument("Controller-selected server stop must be Fresh before Worker construction");
    // Allocate facade first: no prepared provider exists if this allocation fails.
    auto result=std::unique_ptr<Worker>(new Worker(nullptr));
    result->impl_=std::make_unique<Impl>(std::move(domain),bound,take_settings(settings));return result;
}
void Worker::bind_achievement_paths(achievement_storage::Paths& paths){
    auto& v=*impl_;v.control();
    std::lock_guard lock(v.gate);
    if(v.online_binding){v.phase=Phase::Poisoned;throw std::logic_error("Worker paths cannot bind during Online identity query");}
    if(v.phase!=Phase::Prepared||v.entered||v.pending||v.shared_paths)
        throw std::logic_error("Worker achievement paths bind once before preparation/publication");
    if(!v.settings.state||(!v.settings.state->achievement_file.empty()&&
       v.settings.state->achievement_file!=paths.xml_path))
        throw std::invalid_argument("Worker achievement path seed conflicts with shared cells");
    v.shared_paths=&paths;
}
void Worker::bind_authentication_code(const license_recovered::AuthenticationCode& code){
    auto& v=*impl_;v.control();
    std::lock_guard lock(v.gate);
    if(v.online_binding){v.phase=Phase::Poisoned;throw std::logic_error("Worker authentication cannot bind during Online identity query");}
    if(v.phase!=Phase::Prepared||v.entered||v.pending||v.authentication_code)
        throw std::logic_error("Worker authentication cell binds once before preparation/publication");
    v.authentication_code=&code;
}
void Worker::bind_achievement_mutex(execution_recovered::MutexState& state,execution_recovered::Platform& platform){
    auto& v=*impl_;v.control();
    std::lock_guard lock(v.gate);
    if(v.online_binding){v.phase=Phase::Poisoned;throw std::logic_error("Worker mutex cannot bind during Online identity query");}
    if(v.phase!=Phase::Prepared||v.entered||v.pending||v.achievement_mutex)
        throw std::logic_error("Worker achievement mutex binds once before preparation/publication");
    if(state.type!=2||state.owner_48!=0xffffffffU||state.count_4c||state.srw_10||state.storage_08)
        throw std::invalid_argument("Worker achievement mutex requires initialized idle type2 state");
    v.achievement_mutex=&state;v.achievement_platform=&platform;
}
void Worker::bind_effects_online(b::OnlineComponent& online){
    auto& v=*impl_;v.control();
    {std::lock_guard lock(v.gate);
        if(v.online_binding){v.phase=Phase::Poisoned;throw std::logic_error("Worker Online binding is nonreentrant");}
        if(v.phase!=Phase::Prepared||v.entered||v.pending||v.effects_online)throw std::logic_error("Worker Online binds once before preparation/publication");
        v.online_binding=true;
    }
    // No Worker lock across an opaque identity query. Reject caught reentry.
    const bool matched=online.matches(v.bound);
    std::lock_guard lock(v.gate);
    v.online_binding=false;
    if(v.phase!=Phase::Prepared||v.entered||v.pending||v.effects_online)throw std::logic_error("Worker Online binding lost admission");
    if(!matched)throw std::invalid_argument("Worker Online belongs to a foreign Runtime");
    v.effects_online=&online;
}
Worker::~Worker()noexcept{
    PreserveError error;if(!impl_)return;auto& v=*impl_;
    {std::lock_guard lock(v.gate);
        if(::GetCurrentThreadId()!=v.controller||::GetCurrentFiber()!=v.controller_fiber||v.phase!=Phase::Closed||v.pending_present||v.online_binding||v.stop_requesting)std::terminate();
        v.phase=Phase::Retiring;
    }
    // Explicitly retire opaque capture owners before Impl/gate/referents die.
    {Settings retiring=std::move(v.settings);
        // Configure captures may borrow Composer metadata. Retire both callable
        // donors while the receiving Composer and backing state still live.
        v.settings.configure={};retiring.configure={};
        v.settings.accept_stop.reset();retiring.accept_stop.reset();
        v.settings.effects.reset();retiring.effects.reset();
        v.settings.state.reset();retiring.state.reset();
        v.settings.lifetime.reset();retiring.lifetime.reset();
    }
}
void Worker::run_server(std::uint16_t& port,std::uint8_t& ready,execution_recovered::Execution& execution)noexcept{
    PreserveError error;auto& v=*impl_;
    try{
        {
            std::lock_guard lock(v.gate);
            if(v.phase==Phase::Retiring||v.phase==Phase::Closed||v.phase==Phase::Finishing)return;
            if(v.online_binding)throw std::logic_error("Worker cannot enter during Online identity query");
            if(v.phase!=Phase::Prepared||v.entered||&port!=&v.bound.configuration.server_port||&ready!=&v.bound.runtime.server_ready||&execution!=&v.bound.execution)
                throw std::logic_error("Application worker requires its single exact bound source route");
            v.entered=true;v.worker=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();v.phase=Phase::Preparing;
        }
        v.pending=std::make_unique<Impl::Pending>();
        {std::lock_guard lock(v.gate);v.pending_present=true;}
        auto& p=*v.pending;
        v.ptd=::FlsGetValue(v.domain->native_fls_index());v.check();
        p.borrow.emplace(v.memory->acquire());v.check();
        p.cells=std::make_shared<Impl::Cells>(Impl::Cells{v.settings.lifetime,*v.settings.state});
        p.tls=::TlsAlloc();if(p.tls==TLS_OUT_OF_INDEXES)throw std::runtime_error("Application worker TLS allocation failed");
        // Outputs are already owned in Pending before arbitrary configure code.
        // Shared startup state is hidden until configure returns; configure may
        // build lowers/static settings, not read/mutate captured startup cells.
        v.settings.configure(p.inputs);v.check();
        if(v.settings.accept_stop){
            if(p.inputs.runtime.accept_stop&&p.inputs.runtime.accept_stop.get()!=v.settings.accept_stop.get())
                throw std::invalid_argument("Configured Runtime stop differs from controller-selected Worker stop");
            p.inputs.runtime.accept_stop=v.settings.accept_stop;
        }
        auto& lifecycle=p.inputs.graph.policy.lifecycle;
        if(lifecycle.mode==session_policy::LifecycleMode::SourceOwned){
            const auto& stop=p.inputs.runtime.accept_stop;
            if(!stop||stop->scope()==accept_stop::Scope::AcceptOnly)
                throw std::invalid_argument("Source-owned Worker lifecycle requires actual SessionIo Stop");
            if(lifecycle.source_stop&&lifecycle.source_stop.get()!=stop.get())
                throw std::invalid_argument("Source lifecycle proof differs from exact Runtime Stop");
            lifecycle.source_stop=stop;
        }else if(lifecycle.source_stop){
            throw std::invalid_argument("Explicit lifecycle cannot carry a source-owned Stop proof");
        }
        // Original02d540 reads native global a3c98, this exact configuration's
        // SDK string. Empty native configure version selects that actual cell
        // after startup configuration, not the bootstrap log version constant.
        if(lifecycle.mode==session_policy::LifecycleMode::OriginalApplication&&p.inputs.graph.version.empty())
            p.inputs.graph.version=v.bound.configuration.sdk;
        // Backend supplied the real RuntimeOwner startup flag, before the
        // environment/arguments pass. Read now on the actual server worker,
        // after startup has populated it. Legacy views keep configure's value.
        if(lifecycle.mode==session_policy::LifecycleMode::OriginalApplication&&v.bound.show_lsx)
            p.inputs.graph.show_lsx=*v.bound.show_lsx;
        const auto& s=p.inputs.graph.state;
        if(s.lifetime||s.configuration||s.online||s.saved_avatar_size||s.achievement_file||p.inputs.runtime.tls_index)
            throw std::invalid_argument("Application worker owns exact StateView and worker TLS; prebinding is refused");
        if(v.settings.effects){
            if(p.inputs.graph.information||p.inputs.graph.content||p.inputs.graph.licensing)
                throw std::invalid_argument("Concrete effects and legacy configure effect slots are exclusive");
            if(!v.effects_online||!v.shared_paths||!v.authentication_code||!p.inputs.graph.policy.stream)
                throw std::invalid_argument("Concrete effects require Online, shared paths, authentication and stream");
            v.check();
            v.settings.effects->compose(p.effects,{v.bound,*v.effects_online,*v.shared_paths,*p.inputs.graph.policy.stream,
                {&v,[](void* raw){static_cast<Impl*>(raw)->check();}}});
            v.check();
            if(!p.effects.information||!p.effects.content||!p.effects.licensing)
                throw std::invalid_argument("Effect composition returned incomplete owned providers");
            p.inputs.graph.information=std::move(p.effects.information);
            p.inputs.graph.content=std::move(p.effects.content);
            p.inputs.graph.licensing=std::move(p.effects.licensing);
        }
        auto* achievement_file=v.shared_paths?&v.shared_paths->xml_path:&p.cells->state.achievement_file;
        p.inputs.graph.state={p.cells,&v.bound.configuration,&v.bound.runtime.online,&p.cells->state.saved_avatar_size,achievement_file};
        p.inputs.runtime.tls_index=&p.tls;
        // Configure has returned; no Worker metadata gate across a provider.
        // Legacy custom LicenseHandlers without this optional interface keep
        // their existing route. Native Handlers borrows the Runtime-owned cell.
        if(v.authentication_code)
            if(auto* binding=dynamic_cast<license_recovered::AuthenticationBinding*>(p.inputs.graph.licensing.get()))
                binding->bind_authentication_code(*v.authentication_code);
        v.check();
        p.original_application=lifecycle.mode==session_policy::LifecycleMode::OriginalApplication;
        p.graph=g::Graph::create(std::move(p.inputs.graph));v.check();
        if(v.achievement_mutex)p.graph->bind_achievement_mutex(*v.achievement_mutex,*v.achievement_platform);
        v.check();
        p.graph->install_server_prepared(p.resources,v.domain,std::move(*p.borrow),std::move(p.inputs.server),std::move(p.inputs.runtime));v.check();
        {std::lock_guard lock(v.gate);if(v.phase==Phase::Poisoned)throw std::logic_error("Application worker setup was poisoned");v.phase=Phase::Serving;}
        Impl::Proxy proxy(v);server_recovered::server_start_029ea0(proxy,port,ready,execution);v.check();
        // All worker-affine owners die BEFORE this native procedure returns.
        p.resources.server.reset();v.check();p.graph.reset();v.check();p.inputs=Inputs{};v.check();
        if(!::TlsFree(p.tls))throw std::runtime_error("Application worker TLS retirement failed");p.tls=TLS_OUT_OF_INDEXES;v.check();
        v.pending.reset();{std::lock_guard lock(v.gate);v.pending_present=false;}v.check();
        {std::lock_guard lock(v.gate);
            if(v.phase==Phase::Poisoned)throw std::logic_error("Application worker was poisoned during provider retirement");
            v.phase=Phase::Returned;}
    }catch(...){v.poison();} // selected host retention policy, not native EH
}
void Worker::startup_complete(execution_recovered::ReadyStage stage,const execution_recovered::Execution* execution)noexcept{
    PreserveError error;auto& v=*impl_;
    {
        std::lock_guard lock(v.gate);if(v.phase==Phase::Closed||v.phase==Phase::Retiring||v.phase==Phase::Finishing)return;
        if(v.online_binding||(v.stop_requesting&&::GetCurrentThreadId()==v.controller)){v.phase=Phase::Poisoned;return;}
        if(execution!=&v.bound.execution||(stage!=execution_recovered::ReadyStage::Online&&stage!=execution_recovered::ReadyStage::Host))v.phase=Phase::Poisoned;
        else if(v.phase!=Phase::Poisoned){if(stage==execution_recovered::ReadyStage::Online)v.online=true;else v.host=true;}
        v.signal_locked();
    }
}
void Worker::startup_failed(const execution_recovered::Execution*)noexcept{
    PreserveError error;impl_->poison();
}
void Worker::request_stop(){
    PreserveError error;auto& v=*impl_;v.control();
    // Declared BEFORE selected: reservation outlives the local shared owner,
    // including opaque retirement, and is never cleared under a lower call.
    struct Request {
        Impl& value;bool armed=false;
        ~Request(){if(armed){std::lock_guard lock(value.gate);value.stop_requesting=false;}}
    } request{v};
    std::shared_ptr<accept_stop::Stop> selected;
    {
        std::lock_guard lock(v.gate);
        if(v.phase==Phase::Closed||v.phase==Phase::Retiring||v.phase==Phase::Finishing)return;
        if(v.stop_requesting){v.phase=Phase::Poisoned;throw std::logic_error("Application worker stop request reentry");}
        if(v.online_binding){v.phase=Phase::Poisoned;return;}
        v.stop=true;
        if(v.entered&&!v.admitted&&v.phase!=Phase::Returned)v.phase=Phase::Poisoned;
        v.signal_locked();
        // Started-but-not-admitted/unknown prefixes retain their original
        // policy. Do not infer a cancellable Runtime/SDK binding from them.
        if(v.phase==Phase::Poisoned||!v.settings.accept_stop)return;
        selected=v.settings.accept_stop;v.stop_requesting=true;request.armed=true;
    }
    try{
        (void)selected->request(); // wake only; CancelIoEx stays on the worker
        const auto phase=selected->snapshot().phase;
        if(phase==accept_stop::Phase::Poisoned||phase==accept_stop::Phase::Retiring)
            throw std::logic_error("Controller-selected server stop has an unknown outcome");
        // Known FALSE is retained for the existing stop/join retry. TRUE does
        // not finish Graph, release a loan, free an OV or certify completion.
        std::lock_guard lock(v.gate);
        if(v.phase==Phase::Poisoned)throw std::logic_error("Stop provider swallowed Worker lifecycle reentry");
    }catch(...){v.poison();throw;}
}
bool Worker::finish_after_join(){return impl_->finish(false);}
bool Worker::cancel_unpublished(){return impl_->finish(true);}
bool Worker::retry_close(std::uint64_t epoch){
    PreserveError error;auto& v=*impl_;v.control();std::lock_guard lock(v.gate);
    if(v.phase==Phase::Retiring||v.phase==Phase::Closed||v.phase==Phase::Finishing)return false;
    if(v.online_binding||v.stop_requesting){v.phase=Phase::Poisoned;v.signal_locked();throw std::logic_error("Close retry cannot reenter another Worker lifecycle call");}
    if(v.phase!=Phase::WaitingClose||!epoch||epoch!=v.close_epoch||!v.pending_present||!v.settings.close_retry_timeout_ms)return false;
    if(!v.close_retry_requested){v.close_retry_requested=true;v.signal_locked();if(v.phase==Phase::Poisoned)throw std::runtime_error("Cannot signal close continuation");}
    return true;
}
bool Worker::prepare_join(){
    PreserveError error;auto& v=*impl_;v.control();std::uint64_t epoch=0;
    {
        std::lock_guard lock(v.gate);
        if(v.phase==Phase::Retiring||v.phase==Phase::Finishing)return false;
        if(v.online_binding||v.stop_requesting){v.phase=Phase::Poisoned;v.signal_locked();throw std::logic_error("Worker join preparation cannot reenter lifecycle calls");}
        if(!v.settings.close_retry_timeout_ms)return true;
        if(v.phase==Phase::Prepared||v.phase==Phase::Returned||v.phase==Phase::Closed)return true;
        if(v.phase==Phase::WaitingClose)epoch=v.close_epoch;
    }
    if(epoch)(void)retry_close(epoch); // only a wake; stale transition is harmless
    return false; // retain route/owners; never block the sole signaling actor
}
Snapshot Worker::snapshot()const{
    PreserveError error;auto& v=*impl_;std::lock_guard lock(v.gate);
    return {v.phase,v.controller,v.worker,v.online,v.host,v.stop,v.pending_present,v.memory->snapshot(),v.close_epoch,v.close_retries};
}
} // namespace anadius::application_worker
