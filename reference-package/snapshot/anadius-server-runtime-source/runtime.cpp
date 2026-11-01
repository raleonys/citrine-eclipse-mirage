#include "runtime.hpp"
#include <exception>
#include <mutex>
#include <optional>
#include <type_traits>

namespace anadius::server_runtime {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
struct Identity {
    const void* backend;std::uintptr_t identity;const void* service;const void* acceptor;
    const void* pending;const void* scheduler;const void* memory;const void* io;
    explicit Identity(const o::ServerView& v):backend(&v.backend),identity(v.identity.value),service(&v.service),
        acceptor(&v.acceptor),pending(&v.pending),scheduler(&v.scheduler),memory(&v.memory),io(&v.io){}
    bool operator==(const Identity&)const=default;
};
}
struct Runtime::Impl {
    struct Graph;
    struct Completion final:s::DispatcherApi {
        Impl& v;o::AdmittedDispatcherApi* admitted=nullptr;explicit Completion(Impl& v):v(v){}
        s::DispatcherApi& target(){return admitted?static_cast<s::DispatcherApi&>(*admitted):*v.deps.dispatcher;}
        void set_last_error(std::uint32_t n)override{v.policy([&]{target().set_last_error(n);});}
        std::uint32_t get_last_error()override{return v.policy([&]{return target().get_last_error();});}
        std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t)override{
            throw std::logic_error("Only the actual admitted dispatcher may perform worker waits");
        }
        std::int32_t post(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* o)override{return v.policy([&]{return target().post(p,n,k,o);});}
        std::int32_t set_timer(std::uintptr_t t,LARGE_INTEGER& d,std::int32_t p,PTIMERAPCROUTINE f,void* a,std::int32_t r)override{return v.policy([&]{return target().set_timer(t,d,p,f,a,r);});}
    } completion{*this};
    struct WorkerApi final:s::WorkerRunApi {
        Impl& v;explicit WorkerApi(Impl& v):v(v){}
        void* tls_get(std::uint32_t n)override{return v.policy([&]{return v.deps.worker->tls_get(n);});}
        std::int32_t tls_set(std::uint32_t n,void* p)override{return v.policy([&]{return v.deps.worker->tls_set(n,p);});}
        std::int32_t post_stop(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* o)override{return v.completion.post(p,n,k,o);}
        std::uint32_t get_last_error()override{return v.completion.get_last_error();}
    } worker{*this};
    struct Lower final:s::AcceptLower {
        Impl& v;explicit Lower(Impl& v):v(v){}
        void construct_environment(s::IoHandlerEnvironment& e,std::uintptr_t a)override;
        s::Socket create_socket(std::uint32_t,std::int32_t,std::int32_t,s::Error&)override{throw std::logic_error("Recovered factory binding required");}
        void reset_owned_socket(s::Socket&)override{throw std::logic_error("Recovered holder binding required");}
        void reset_pending_owner(s::AcceptPending&)override;
        s::Category misc_category()override{return v.deps.misc_category;}
        s::Category success_category()override{return v.deps.generic_category;}
        void retry(s::AsyncAcceptOperation&,s::CancelOperation&)override{throw std::logic_error("Recovered retry binding required");}
        void invoke(s::AcceptHandler&)override;
    } lower{*this};
    struct FlowApplication final:s::FlowApplication {
        Impl& v;explicit FlowApplication(Impl& v):v(v){}
        s::ReceiveBufferString& greeting(s::SessionOwner& owner,s::ReceiveBufferString& out)override;
        [[noreturn]]void raise_write(s::Error& e,std::uintptr_t p)override;
    } flow_application{*this};
    struct Graph {
        o::ServerView view;const std::uintptr_t server_identity;
        s::AsyncAcceptService service;s::AcceptPending pending;s::RetryEngine retry_engine;
        std::array<s::AcceptRetryBinding,1> retry_bindings{};
        std::unique_ptr<Environment> environment;
        std::unique_ptr<s::RecoveredAcceptLower> recovered;
        std::unique_ptr<s::OwnedAcceptMemory> accept;
        std::unique_ptr<s::BoundedAcceptRetryApi> retry;
        std::unique_ptr<s::CoherentIoMemory> memory;
        std::unique_ptr<Application> application;
        std::optional<Bindings> bindings;
        std::unique_ptr<s::AsyncAcceptContext> accept_context;
        std::unique_ptr<s::DomainFlowApi> flow_api;
        std::unique_ptr<s::FlowServer> flow_server;
        std::array<s::FlowServerBinding,1> servers{};
        std::unique_ptr<s::FlowContext> flow;
        Graph(o::ServerView v,std::uintptr_t id):view(v),server_identity(id),
            service{&v.scheduler,reinterpret_cast<std::uintptr_t>(&retry_engine)},
            pending{v.pending,&v.scheduler},retry_engine{&v.scheduler}{}
    };
    Dependencies deps;mutable std::mutex gate;Phase phase=Phase::Fresh;
    std::size_t active=0;std::uint64_t handled=0;DWORD thread=0;void* fiber=nullptr;
    std::uint32_t index=0;std::optional<Identity> identity;std::unique_ptr<Graph> graph;
    explicit Impl(Dependencies d):deps(std::move(d)){}
    void poison(){PreserveError error;std::lock_guard lock(gate);phase=Phase::Poisoned;}
    void backend_admissible(bool idle){
        PreserveError error;const auto state=graph->view.backend.snapshot().phase;
        if(idle?(state!=o::Phase::Returned&&state!=o::Phase::Quiescing):(state!=o::Phase::Ready&&state!=o::Phase::Running))
            throw std::logic_error("The bound server backend is not admissible or was poisoned");
    }
    void expect(Phase expected){
        PreserveError error;{std::lock_guard lock(gate);if(phase!=expected)throw std::logic_error("Runtime lifecycle was changed or poisoned by a lower callback");}
        if(expected==Phase::Binding)backend_admissible(false);
        if(expected==Phase::Quiescing)backend_admissible(true);
    }
    struct Call {
        Impl& v;
        Call(Impl& v,std::initializer_list<Phase> allowed):v(v){
            PreserveError error;std::lock_guard lock(v.gate);
            if(v.thread&&(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()))throw std::logic_error("Runtime lifecycle requires its bound thread/fiber");
            if(v.active){v.phase=Phase::Poisoned;throw std::logic_error("Runtime lifecycle reentry is unsupported");}
            bool valid=false;for(auto p:allowed)valid|=v.phase==p;
            if(!valid)throw std::logic_error("Runtime lifecycle phase precondition failed");
            ++v.active;
        }
        ~Call(){PreserveError error;std::lock_guard lock(v.gate);--v.active;}
    };
    void matching(const o::ServerView& view){if(!identity||*identity!=Identity(view))throw std::invalid_argument("Foreign server view does not match the complete bound graph");}
    void checkpoint(){
        PreserveError error;
        {std::lock_guard lock(gate);if(phase!=Phase::Ready&&phase!=Phase::Running)throw std::logic_error("Runtime graph is not published or was poisoned");}
        if(*deps.tls_index!=index)throw std::logic_error("Runtime TLS binding was changed after publication");
        backend_admissible(false);
        auto& g=*graph;(void)g.view.memory.buffer_context();
        if(g.view.io.snapshot().phase!=s::IoMemoryPhase::Open||g.accept->snapshot().poisoned)
            throw std::logic_error("Runtime observed a poisoned storage provider");
    }
    template<class F>std::invoke_result_t<F> policy(F&& fn){
        checkpoint();
        if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(fn)();checkpoint();}
        else{decltype(auto) result=std::forward<F>(fn)();checkpoint();return result;}
    }
    void verify_bindings(){
        auto& g=*graph;auto& b=*g.bindings;auto& buffer=g.view.memory.buffer_context();
        if(&b.frame.buffer!=&buffer||&b.frame.completion!=&b.completion||!b.frame.read)
            throw std::invalid_argument("Frame must bind this parent buffer and selected completion effects");
        auto& read=*b.frame.read;auto& receive=read.receive;
        if(&read.buffer!=&buffer||&read.completion!=&b.completion||&receive.worker!=&worker||
           &receive.memory!=&g.memory->posted()||&receive.effects!=g.memory.get()||
           &receive.completion!=&completion||&receive.errors!=deps.errors.get()||
           &receive.environment.storage.tls_index!=deps.tls_index||
           receive.environment.storage.storage.data()!=g.accept->storage_bindings().data()||
           receive.environment.storage.storage.size()!=g.accept->storage_bindings().size())
            throw std::invalid_argument("Read/receive must use coherent memory and the exact runtime APIs/storage");
        if(auto* write=b.write;write&&(&write->worker!=&worker||&write->memory!=&g.memory->posted()||
            &write->completion!=&completion||&write->errors!=deps.errors.get()||&write->environment.storage.tls_index!=deps.tls_index))
            throw std::invalid_argument("Write must use coherent memory and the exact runtime APIs");
    }
    bool drained(std::size_t leases){
        auto& g=*graph;auto operation=g.view.memory.enter();const auto io=g.view.io.snapshot();
        const auto memory=g.view.memory.snapshot();const auto accept=g.accept->snapshot();
        return io.phase==s::IoMemoryPhase::Open&&!io.allocations&&!io.pending&&!io.active_calls&&
            memory.phase==s::SessionMemoryPhase::Open&&!memory.allocations&&!memory.controls&&!memory.proxies&&!memory.blocks&&
            !memory.maps&&!memory.buffers&&!memory.reserved&&!memory.pending_frees&&!memory.partial_constructions&&
            !memory.buffer_borrows&&memory.provider_leases==leases&&!accept.poisoned&&!accept.projections&&!accept.pending&&!accept.active_calls&&
            !g.view.scheduler.work_count_30.load()&&!g.view.scheduler.completion.head&&!g.view.scheduler.completion.tail;
    }
    void bind(o::ServerView& view){
        if(view.io.storage_bindings().size()!=deps.capacity||!view.identity.value)
            throw std::invalid_argument("Runtime capacity and nonzero context identity must match the owned base");
        // Publish the empty record before any opaque factory or mandatory-close
        // provider exists. Later failures retain this graph; no constructor unwind.
        {std::lock_guard lock(gate);thread=::GetCurrentThreadId();fiber=::GetCurrentFiber();index=*deps.tls_index;
            identity.emplace(view);graph=std::make_unique<Graph>(view,reinterpret_cast<std::uintptr_t>(this));phase=Phase::Binding;}
        auto& g=*graph;
        g.environment=std::make_unique<Environment>(deps.profile->environment(view,g.server_identity));expect(Phase::Binding);
        if(!g.environment->sentinels.empty_operations_846c8)throw std::invalid_argument("Live environment sentinels required");
        g.recovered=std::make_unique<s::RecoveredAcceptLower>(lower,*deps.factory,*deps.sockets,*deps.errors,completion,g.retry_bindings);
        g.accept=std::make_unique<s::OwnedAcceptMemory>(view.memory.shared_from_this(),view.io,deps.capacity,*g.recovered,*deps.accept);
        g.retry=std::make_unique<s::BoundedAcceptRetryApi>(g.accept->prearmed_api());
        g.retry_bindings[0]={g.service.retry_engine_identity,&g.retry_engine,g.retry.get(),deps.cancel.get()};
        g.memory=std::make_unique<s::CoherentIoMemory>(*g.accept,view.io,g.environment->sentinels,g.environment->constructor,g.environment->sources);
        s::IoCompletionEnvironment environment{{*deps.tls_index,g.accept->storage_bindings()},g.environment->sentinels};
        g.application=deps.profile->create(Ports{view,*g.accept,*g.memory,worker,completion,environment,g.server_identity});expect(Phase::Binding);
        if(!g.application)throw std::invalid_argument("Application factory returned no owned contexts");
        g.bindings.emplace(g.application->bindings());expect(Phase::Binding);verify_bindings();
        g.accept_context=std::make_unique<s::AsyncAcceptContext>(s::AsyncAcceptContext{environment,worker,*g.accept,
            g.accept->pinned_api(),completion,*deps.errors,*deps.sockets,*g.accept,g.bindings->accept_schedules});
        g.accept->bind_context(*g.accept_context);
        g.flow_api=std::make_unique<s::DomainFlowApi>(view.memory,*g.accept,deps.executor,flow_application);
        g.flow_server=std::make_unique<s::FlowServer>(s::FlowServer{g.server_identity,g.service,view.acceptor.state,g.pending});
        g.servers[0]={g.server_identity,g.flow_server.get()};
        g.flow=std::make_unique<s::FlowContext>(s::FlowContext{*g.flow_api,view.memory.buffer_context(),g.bindings->frame,
            *deps.errors,g.bindings->logging,g.servers,g.bindings->categories,deps.generic_category});
        if(deps.accept_stop)deps.accept_stop->bind(view,*g.accept);
        expect(Phase::Binding);{std::lock_guard lock(gate);phase=Phase::Ready;}
    }
};
void Runtime::Impl::Lower::construct_environment(s::IoHandlerEnvironment& e,std::uintptr_t a){v.checkpoint();v.graph->application->construct_accept_environment(e,a);v.checkpoint();}
void Runtime::Impl::Lower::reset_pending_owner(s::AcceptPending& p){v.checkpoint();v.graph->application->reset_pending_owner(p);v.checkpoint();}
void Runtime::Impl::Lower::invoke(s::AcceptHandler& h){v.checkpoint();v.verify_bindings();s::invoke_accept_handler_021a10(h,*v.graph->flow);v.checkpoint();}
s::ReceiveBufferString& Runtime::Impl::FlowApplication::greeting(s::SessionOwner& owner,s::ReceiveBufferString& out){
    v.checkpoint();auto& result=v.graph->application->greeting(owner,out);v.checkpoint();v.verify_bindings();return result;
}
void Runtime::Impl::FlowApplication::raise_write(s::Error& e,std::uintptr_t p){v.checkpoint();v.graph->application->raise_write(e,p);std::terminate();}
Runtime::Runtime(Dependencies d){
    if(!d.errors||!d.sockets||!d.dispatcher||!d.worker||!d.accept||!d.factory||!d.cancel||!d.profile||
       !d.tls_index||*d.tls_index==TLS_OUT_OF_INDEXES||!d.guard_epoch||!d.capacity||d.capacity>65536||!d.executor.move)
        throw std::invalid_argument("Complete runtime providers, stable TLS/epoch/executor and bounded capacity required");
    impl_=std::make_unique<Impl>(std::move(d));
}
Runtime::~Runtime()noexcept{
    auto& v=*impl_;
    {PreserveError error;std::lock_guard lock(v.gate);if(v.graph||v.active)std::terminate();v.phase=Phase::Retiring;}
    // Provider destructors may call metadata or attempt lifecycle reentry.
    // Retire them outside the mutex while Impl/proxies/mutex are still alive;
    // Retiring denies every lifecycle call before any moved provider is read.
    {Dependencies retiring=std::move(v.deps);}
}
void Runtime::submit_accept(o::ServerView& view){
    auto& v=*impl_;Impl::Call call(v,{Phase::Fresh,Phase::Ready});
    if(v.identity)v.matching(view);
    try{
        (void)view.memory.buffer_context();if(!v.graph)v.bind(view);v.checkpoint();
        if(v.graph->accept->snapshot().pending)throw std::logic_error("An initial accept is already pending");
        s::rearm_accept_0270e0(*v.graph->flow_server,*v.graph->flow);v.checkpoint();
    }catch(...){v.poison();throw;}
}
server_recovered::Error Runtime::run(o::ServerView& view,o::AdmittedDispatcherApi& admitted){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready});v.matching(view);
    {std::lock_guard lock(v.gate);v.phase=Phase::Running;}
    struct Unbind {Impl::Completion& proxy;~Unbind(){proxy.admitted=nullptr;}} unbind{v.completion};v.completion.admitted=&admitted;
    try{
        v.checkpoint();
        auto& g=*v.graph;auto& b=*g.bindings;
        struct Admitted final:s::DispatcherApi {
            Impl& v;o::AdmittedDispatcherApi& lower;
            bool saved=false;std::uint32_t status=0;
            Admitted(Impl& v,o::AdmittedDispatcherApi& l):v(v),lower(l){}
            void set_last_error(std::uint32_t n)override{v.policy([&]{lower.set_last_error(n);});}
            std::uint32_t get_last_error()override{return v.policy([&]{if(saved){saved=false;return status;}return lower.get_last_error();});}
            std::int32_t get(std::uintptr_t p,std::uint32_t& n,std::uintptr_t& k,OVERLAPPED*& o,std::uint32_t t)override{
                if(!v.deps.accept_stop)return v.policy([&]{return lower.get(p,n,k,o,t);});
                return v.policy([&]{
                    if(saved)throw std::logic_error("Previous physical packet status was not consumed");
                    v.deps.accept_stop->pump();
                    for(;;){
                        lower.set_last_error(0);const auto received=lower.get(p,n,k,o,t);
                        const auto immediate=lower.get_last_error();
                        if(v.deps.accept_stop->physical(received,n,k,o,immediate)){v.deps.accept_stop->pump();continue;}
                        status=immediate;saved=true;return received;
                    }
                });
            }
            std::int32_t post(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* o)override{return v.policy([&]{return lower.post(p,n,k,o);});}
            std::int32_t set_timer(std::uintptr_t t,LARGE_INTEGER& d,std::int32_t p,PTIMERAPCROUTINE f,void* a,std::int32_t r)override{return v.policy([&]{return lower.set_timer(t,d,p,f,a,r);});}
        } admitted_api{v,admitted};
        s::CoherentIoDispatcher coherent(*g.accept,view.io,*g.memory,g.accept_context->environment,v.worker,*v.deps.errors,b.completion,b.guard);
        struct Router final:s::DispatcherEffects {
            Impl& v;s::CoherentIoDispatcher& coherent;
            Router(Impl& v,s::CoherentIoDispatcher& c):v(v),coherent(c){}
            s::DispatcherEffects& guard(){return v.graph->bindings->guard;}
            std::uintptr_t capture_tls_slot()override{return v.policy([&]{return guard().capture_tls_slot();});}
            std::int32_t thread_epoch(std::uintptr_t p)override{return v.policy([&]{return guard().thread_epoch(p);});}
            void guard_header_04b838(std::int32_t& n)override{v.policy([&]{guard().guard_header_04b838(n);});}
            void register_atexit_04b770(std::uintptr_t p)override{v.policy([&]{guard().register_atexit_04b770(p);});}
            void guard_footer_04b7cc(std::int32_t& n)override{v.policy([&]{guard().guard_footer_04b7cc(n);});}
            std::uint32_t read_stop_34(s::IocpSchedulerEntry& e)override{return v.policy([&]{return guard().read_stop_34(e);});}
            std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& a,const std::array<std::uintptr_t,2>& b)override{return v.policy([&]() -> std::array<std::uintptr_t,2>&{return guard().copy_exception_024100(a,b);});}
            [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>& p)override{v.checkpoint();guard().rethrow_029880(p);std::terminate();}
            void complete(s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes)override{
                v.checkpoint();auto* const operation=&op;auto* const overlapped=&op.overlapped;
                const auto before=v.graph->view.io.handle_for(overlapped);
                anadius::accept_stop::Receipt receipt;
                if(v.deps.accept_stop)receipt=v.deps.accept_stop->before(owner,op,error,bytes);
                auto* selected=receipt.drop_owner?nullptr:v.graph->application->completion_owner(owner,op,error,bytes);v.checkpoint();
                const auto after=v.graph->view.io.handle_for(overlapped);
                const auto& packet=v.graph->accept->dispatcher_bindings()[before.slot];
                if(before.slot!=after.slot||before.generation!=after.generation||packet.operation!=operation||packet.overlapped_identity!=overlapped)
                    throw std::logic_error("Completion policy retired or replaced the observed operation");
                v.verify_bindings();
                if(selected&&selected!=owner)throw std::invalid_argument("Completion policy selected a foreign owner");
                coherent.complete(selected,op,error,bytes); // op may end here
                v.checkpoint();v.graph->application->after_completion();v.checkpoint();
                if(v.deps.accept_stop)v.deps.accept_stop->after(receipt);
            }
        } router{v,coherent};
        s::DispatcherState state{view.scheduler,nullptr,0,0,g.accept->dispatcher_bindings()};
        struct Worker final:s::RecoveredDispatcherWorkerEffects {
            Impl& v;
            Worker(Impl& v,s::DispatcherState& state,s::DispatcherEnvironment env,s::DispatcherApi& api,s::DispatcherEffects& effects)
                :RecoveredDispatcherWorkerEffects(state,env,api,effects,v.worker,*v.deps.errors),v(v){}
            WorkerControl& lower(){return v.graph->bindings->worker;}
            std::uint32_t exchange_stop_34(s::IocpSchedulerEntry& e,std::uint32_t n)override{return v.policy([&]{return lower().exchange_stop(e,n);});}
            std::uint32_t exchange_wake_38(s::IocpSchedulerEntry& e,std::uint32_t n)override{return v.policy([&]{return lower().exchange_wake(e,n);});}
            void initialize_04a240(std::array<std::uintptr_t,2>& p)override{v.policy([&]{lower().initialize(p);});}
            void finish_04a2cc(std::array<std::uintptr_t,2>& p)override{v.policy([&]{lower().finish(p);});}
            void free_cache_04b788(std::uintptr_t p)override{v.checkpoint();v.graph->memory->free_cached_identity(p);}
            void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override{throw std::logic_error("Embedded runtime worker has no allocated worker-function object");}
            [[noreturn]]void raise_028240(s::Error e,std::uintptr_t p)override{v.checkpoint();lower().raise(e,p);std::terminate();}
        } worker{v,state,{*v.deps.guard_epoch,v.deps.generic_category,v.deps.system_category},admitted_api,router};
        if(v.deps.accept_stop)v.deps.accept_stop->running();
        s::WorkerRunResult result;const auto count=s::run_worker_0298b0(view.scheduler,result,{*v.deps.tls_index,v.deps.generic_category},v.worker,*v.deps.errors,worker);
        v.checkpoint();g.application->worker_returned(count,result);v.checkpoint();
        if(admitted.poisoned())throw std::logic_error("Runtime worker admission was poisoned");
        {std::lock_guard lock(v.gate);v.handled=count;v.phase=Phase::Returned;}
        if(!result.error)throw std::logic_error("Native worker returned without a defined error result");
        return {result.error->value,result.error->category.logical};
    }catch(...){v.poison();throw;}
}
bool Runtime::quiescent(o::ServerView& view){
    auto& v=*impl_;Impl::Call call(v,{Phase::Returned,Phase::Quiescing,Phase::Closed});v.matching(view);
    {std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;v.phase=Phase::Quiescing;}
    try{
        const auto attested=v.deps.profile->external_quiescent(view,*v.graph->application);
        v.expect(Phase::Quiescing);
        if(view.memory.snapshot().phase!=s::SessionMemoryPhase::Open||view.io.snapshot().phase!=s::IoMemoryPhase::Open||
           *v.deps.tls_index!=v.index)throw std::logic_error("Quiescence provider changed or poisoned source admission");
        if(!attested)return false;
        if(!v.drained(2))return false; // Base lease + this owning accept provider
        if(v.deps.accept_stop&&!v.deps.accept_stop->close_worker())return false;
        if(!v.graph->accept->close())return false;
        v.expect(Phase::Quiescing);
        // Retire the application before the memory it borrows. Explicitly
        // destroy the graph to break Core->Runtime->Parent->Core ownership.
        v.graph->flow.reset();v.graph->flow_api.reset();v.graph->application.reset();v.expect(Phase::Quiescing);
        if(!v.drained(1))throw std::logic_error("Application retirement changed the attested source graph");
        {std::lock_guard lock(v.gate);v.graph.reset();v.phase=Phase::Closed;}
        return true;
    }catch(...){v.poison();throw;}
}
Snapshot Runtime::snapshot()const{auto& v=*impl_;PreserveError error;std::lock_guard lock(v.gate);return {v.phase,v.active,bool(v.graph),v.handled};}
} // namespace anadius::server_runtime
