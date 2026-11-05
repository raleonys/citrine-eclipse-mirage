#include "jobs.hpp"
#include <atomic>
#include <exception>
#include <list>
#include <limits>
#include <mutex>
#include <optional>

namespace anadius::owned_jobs {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
std::uint32_t next_ticket(){static std::atomic<std::uint32_t> next{1};auto value=next.load();for(;;){if(value==UINT32_MAX)throw std::overflow_error("Inline source tickets exhausted");if(next.compare_exchange_weak(value,value+1))return value;}}
}
struct InlineJobs::Impl {
    enum class Stage {Empty,Reserved,Running,Completed,Retiring,Unknown};
    struct Row {Stage stage=Stage::Empty;std::uint64_t generation=0;s::FrameThreadHandle ticket;std::function<void()> runner;bool start_returned=false;};
    mutable std::mutex gate;Phase phase=Phase::Fresh;bool active=false,binding=false,closing=false,checking=false;
    std::vector<Row> rows;std::function<void()> incoming,checkpoint;std::optional<rt::Ports> ports;std::shared_ptr<void> lease;
    DWORD thread=0;void* fiber=nullptr;std::size_t pending=0;std::uint64_t started=0,completed=0,detached=0;
    explicit Impl(std::size_t capacity):rows(capacity){}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Inline jobs require the bound worker/fiber");}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed)phase=Phase::Poisoned;}
    void check(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||binding||(!active&&!closing))throw std::logic_error("Inline job admission was poisoned");}
    void probe(){
        {PreserveError error;std::lock_guard lock(gate);if(checking||phase!=Phase::Bound){if(phase!=Phase::Closed)phase=Phase::Poisoned;throw std::logic_error("Inline job admission probe reentry");}checking=true;}
        try{checkpoint();}catch(...){PreserveError error;std::lock_guard lock(gate);checking=false;if(phase!=Phase::Closed)phase=Phase::Poisoned;throw;}
        {PreserveError error;std::lock_guard lock(gate);checking=false;if(phase!=Phase::Bound)throw std::logic_error("Inline job probe was poisoned");}
    }
    void admit(bool cleanup=false){
        check();probe();check();auto& p=*ports;(void)p.view.memory.buffer_context();const auto backend=p.view.backend.snapshot().phase;
        if((backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running&&!(cleanup&&(backend==sp::o::Phase::Returned||backend==sp::o::Phase::Quiescing)))||p.view.io.snapshot().phase!=s::IoMemoryPhase::Open||p.accept.snapshot().poisoned)
            throw std::logic_error("Inline jobs require admissible source memory/IO/backend");
    }
    void begin(){PreserveError error;std::lock_guard lock(gate);if(active||binding||closing||checking){phase=Phase::Poisoned;throw std::logic_error("Inline job execution is nonreentrant");}if(phase!=Phase::Bound)throw std::logic_error("Inline jobs are unbound/closed/poisoned");affinity();active=true;}
    void finish(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||!active||closing)throw std::logic_error("Inline job completion was poisoned");active=false;}
    void failed(){poison();PreserveError error;std::lock_guard lock(gate);active=false;}
    Snapshot snapshot()const{PreserveError error;std::lock_guard lock(gate);return {phase,pending,active,started,completed,detached};}
};
InlineJobs::InlineJobs(std::size_t capacity){if(!capacity||capacity>65536)throw std::invalid_argument("Bounded inline-job capacity required");impl_=std::make_unique<Impl>(capacity);}
InlineJobs::~InlineJobs()noexcept{auto& v=*impl_;PreserveError error;std::lock_guard lock(v.gate);if(v.active||v.binding||v.closing||v.checking||v.pending||v.incoming||v.lease||(v.phase!=Phase::Fresh&&v.phase!=Phase::Closed))std::terminate();}
void InlineJobs::bind(rt::Ports ports,std::function<void()> checkpoint){
    auto& v=*impl_;{PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||v.binding){if(v.active||v.binding||v.closing)v.phase=Phase::Poisoned;throw std::logic_error("Inline jobs bind once");}v.binding=true;}
    try{
        const auto storage=ports.accept.storage_bindings();
        if(!checkpoint||!ports.server_identity||!ports.view.identity.value||storage.data()!=ports.environment.storage.current_storage().data()||storage.size()!=ports.environment.storage.current_storage().size())throw std::invalid_argument("Inline jobs require exact stable source Ports and admission");
        (void)ports.view.memory.buffer_context();const auto backend=ports.view.backend.snapshot().phase;
        if((backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running)||ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports.accept.snapshot().poisoned)throw std::logic_error("Inline jobs require an open source graph");
        v.ports.emplace(ports);v.checkpoint=std::move(checkpoint);checkpoint={};
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||!v.binding)throw std::logic_error("Inline bind target retirement was poisoned");}
        v.lease=ports.view.memory.acquire_provider_lease_from_operation();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||!v.binding)throw std::logic_error("Inline bind was poisoned");v.thread=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();v.binding=false;v.phase=Phase::Bound;}
    }catch(...){v.poison();{PreserveError error;std::lock_guard lock(v.gate);v.binding=false;}throw;}
}
s::FrameThreadHandle InlineJobs::start(sp::JobToken token,std::function<void()> runner){
    auto& v=*impl_;v.begin();Impl::Row* row=nullptr;
    try{
        v.incoming=std::move(runner);runner={};v.admit();
        {PreserveError error;std::lock_guard lock(v.gate);
            if(token.slot>=v.rows.size()||!token.generation||!v.incoming)throw std::invalid_argument("Invalid inline source job token/callable");
            row=&v.rows[token.slot];if(row->stage!=Impl::Stage::Empty||token.generation<=row->generation)throw std::invalid_argument("Inline source slot is live or generation is stale");
            const auto id=next_ticket();row->generation=token.generation;row->ticket={static_cast<std::uintptr_t>(id),id};row->stage=Impl::Stage::Reserved;row->start_returned=false;++v.pending;++v.started;
        }
        row->runner=std::move(v.incoming);v.incoming={};v.admit();
        {PreserveError error;std::lock_guard lock(v.gate);row->stage=Impl::Stage::Running;}
        row->runner();v.admit();
        s::FrameThreadHandle ticket;
        {PreserveError error;std::lock_guard lock(v.gate);row->stage=Impl::Stage::Completed;row->start_returned=true;++v.completed;ticket=row->ticket;}
        v.finish();return ticket;
    }catch(...){if(row){PreserveError error;std::lock_guard lock(v.gate);if(row->stage!=Impl::Stage::Empty)row->stage=Impl::Stage::Unknown;}v.failed();throw;}
}
std::int32_t InlineJobs::detach(const s::FrameThreadHandle& ticket){
    auto& v=*impl_;v.begin();Impl::Row* row=nullptr;
    try{
        v.admit(true);
        {PreserveError error;std::lock_guard lock(v.gate);for(auto& r:v.rows)if(r.stage!=Impl::Stage::Empty&&r.ticket.handle==ticket.handle&&r.ticket.id==ticket.id){if(row)throw std::logic_error("Ambiguous inline source ticket");row=&r;}
            if(!ticket.handle||!ticket.id||!row||row->stage!=Impl::Stage::Completed||!row->start_returned)throw std::invalid_argument("Inline detach requires a normally returned exact ticket");row->stage=Impl::Stage::Retiring;
        }
        {auto retiring=std::move(row->runner);row->runner={};}
        v.admit(true);
        {PreserveError error;std::lock_guard lock(v.gate);row->ticket={};row->start_returned=false;row->stage=Impl::Stage::Empty;--v.pending;++v.detached;}
        v.finish();return 0;
    }catch(...){if(row){PreserveError error;std::lock_guard lock(v.gate);if(row->stage!=Impl::Stage::Empty)row->stage=Impl::Stage::Unknown;}v.failed();throw;}
}
bool InlineJobs::close(bool attested){
    auto& v=*impl_;{PreserveError error;std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;if(v.active||v.binding||v.closing||v.checking){v.phase=Phase::Poisoned;throw std::logic_error("Inline job lifecycle reentry");}if(v.phase!=Phase::Bound)throw std::logic_error("Inline jobs cannot close an unknown/unbound state");v.affinity();if(!attested||v.pending||v.incoming)return false;v.closing=true;}
    try{
        v.admit(true);auto& p=*v.ports;const auto io=p.view.io.snapshot();const auto memory=p.view.memory.snapshot();const auto accept=p.accept.snapshot();const auto backend=p.view.backend.snapshot().phase;
        const bool drained=(backend==sp::o::Phase::Returned||backend==sp::o::Phase::Quiescing)&&io.phase==s::IoMemoryPhase::Open&&!io.allocations&&!io.pending&&!io.active_calls&&
            memory.phase==s::SessionMemoryPhase::Open&&!memory.allocations&&!memory.controls&&!memory.proxies&&!memory.blocks&&!memory.maps&&!memory.buffers&&!memory.reserved&&!memory.pending_frees&&!memory.partial_constructions&&!memory.buffer_borrows&&
            !accept.poisoned&&!accept.projections&&!accept.pending&&!accept.active_calls&&!p.view.scheduler.work_count_30.load()&&!p.view.scheduler.completion.head&&!p.view.scheduler.completion.tail;
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||!v.closing)throw std::logic_error("Inline job close was poisoned");if(!drained){v.closing=false;return false;}}
        {auto retiring=std::move(v.checkpoint);v.checkpoint={};}
        v.check();v.lease.reset();v.ports.reset();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||!v.closing)throw std::logic_error("Inline job provider retirement was poisoned");v.closing=false;v.phase=Phase::Closed;}return true;
    }catch(...){v.poison();{PreserveError error;std::lock_guard lock(v.gate);v.closing=false;}throw;}
}
Snapshot InlineJobs::snapshot()const{return impl_->snapshot();}

struct NativeJobs::Impl {
    enum class Stage {Empty,Preparing,Launched,Returned,Consumed,Unknown};
    struct Row {
        Impl* owner=nullptr;Stage stage=Stage::Empty;sp::JobToken token{};std::uint32_t receipt=0;
        std::optional<sp::Application::JobHandoff> handoff;
        HANDLE original=nullptr,joined=nullptr,barrier=nullptr;DWORD id=0;
        bool detached=false,work=false,notified=false,physical=false;std::exception_ptr failure;
    };
    mutable std::mutex gate;std::mutex source_gate;Phase phase=Phase::Fresh;bool active=false,binding=false,checking=false;
    std::shared_ptr<bootstrap_providers::NativeThreads> threads;NativeJobApi api;
    std::list<Row> rows;std::optional<rt::Ports> ports;sp::Application* application=nullptr;sp::Application* closed_application=nullptr; // passive retry identity only
    std::function<void()> checkpoint;std::shared_ptr<void> lease;DWORD thread=0;void* fiber=nullptr;
    std::uint64_t started=0,completed=0,detached=0;
    Impl(std::shared_ptr<bootstrap_providers::NativeThreads> t,std::size_t capacity,NativeJobApi a):threads(std::move(t)),api(a),rows(capacity){for(auto& row:rows)row.owner=this;}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Native Jobs lifecycle requires original worker/fiber");}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed)phase=Phase::Poisoned;}
    void seal(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||!active||binding||checking)throw std::logic_error("Native Jobs SDK/lifecycle seal was poisoned");}
    void begin(){PreserveError error;std::lock_guard lock(gate);affinity();if(active||binding||checking){phase=Phase::Poisoned;throw std::logic_error("Native Jobs is nonreentrant");}if(phase!=Phase::Bound)throw std::logic_error("Native Jobs is closed/unknown");active=true;}
    void end(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound||!active||checking||binding)throw std::logic_error("Native Jobs operation was poisoned");active=false;}
    void failed(Row* row=nullptr){PreserveError error;std::lock_guard lock(gate);if(row)row->stage=Stage::Unknown;phase=Phase::Poisoned;active=false;}
    void match(sp::Application& app)const{if((application?application:closed_application)!=&app)throw std::invalid_argument("Native Jobs belongs to another Application");}
    void admit(bool cleanup=false){
        PreserveError error;
        {std::lock_guard lock(gate);affinity();if(phase!=Phase::Bound||!active||checking||binding){phase=Phase::Poisoned;throw std::logic_error("Native Jobs admission/reentry denied");}checking=true;}
        try{checkpoint();(void)ports->view.memory.buffer_context();const auto backend=ports->view.backend.snapshot().phase;
            if((backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running&&!(cleanup&&(backend==sp::o::Phase::Returned||backend==sp::o::Phase::Quiescing)))||ports->view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports->accept.snapshot().poisoned)throw std::logic_error("Native Jobs source admission closed");
        }catch(...){std::lock_guard lock(gate);checking=false;phase=Phase::Poisoned;throw;}
        {std::lock_guard lock(gate);checking=false;if(phase!=Phase::Bound)throw std::logic_error("Native Jobs probe was poisoned");}
    }
    void reserve_row(sp::JobToken token,Row*& selected){
        {PreserveError error;std::lock_guard lock(gate);
            if(phase!=Phase::Bound||!active||binding||checking)throw std::logic_error("Native Jobs row reservation was poisoned");
            for(auto& row:rows)if(row.stage==Stage::Empty){selected=&row;break;}
        }
        // Live native threads retain Row* directly. Prepare a stable node
        // outside metadata gate, then publish it without allocating or moving
        // any existing row; snapshot iteration uses the same gate.
        std::list<Row> prepared;
        if(!selected){prepared.emplace_back();prepared.back().owner=this;admit();}
        {PreserveError error;std::lock_guard lock(gate);
            if(phase!=Phase::Bound||!active||binding||checking)throw std::logic_error("Native Jobs row preparation was poisoned");
            if(!selected){
                if(rows.size()==rows.max_size())throw std::length_error("Native Jobs row registry exceeds container capacity");
                selected=&prepared.front();rows.splice(rows.end(),prepared);
            }
            if(selected->owner!=this||selected->stage!=Stage::Empty)throw std::logic_error("Native Jobs row changed during preparation");
            selected->stage=Stage::Preparing;selected->token=token;selected->receipt=next_ticket();
        }
    }
    static unsigned __stdcall entry(void* raw)noexcept{
        auto& row=*static_cast<Row*>(raw);auto& v=*row.owner;unsigned result=0;
        try{
            // Detached publication is separate from the Parent gate. Do not
            // pin Parent while waiting for original start/detach to commit.
            if(v.api.wait(row.barrier,INFINITE)!=WAIT_OBJECT_0)throw std::runtime_error("Native Jobs detach barrier wait failed");
            std::unique_lock source(v.source_gate); // serializes child acquire/body/return with outside-Parent reaping
            {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||!row.detached||row.stage!=Stage::Launched)throw std::logic_error("Native Jobs detach publication is unknown");}
            row.handoff->acquire();row.handoff->process_message();row.handoff->return_memory();
            {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||row.stage!=Stage::Launched)throw std::logic_error("Native Jobs child return was poisoned");row.stage=Stage::Returned;}
        }catch(...){std::lock_guard lock(v.gate);row.failure=std::current_exception();row.stage=Stage::Unknown;v.phase=Phase::Poisoned;result=1;}
        // Raw native post only. Original-affine Runtime completion providers
        // are never called by the child. A delivered receipt is not FLS exit.
        try{if(!v.api.post(reinterpret_cast<HANDLE>(v.ports->view.scheduler.completion.port),row.receipt,s::prepared_job_completion_key,nullptr))throw std::runtime_error("Native Jobs ready notification failed");
            std::lock_guard lock(v.gate);row.notified=true;
        }catch(...){std::lock_guard lock(v.gate);row.failure=std::current_exception();row.stage=Stage::Unknown;v.phase=Phase::Poisoned;result=1;}
        return result; // actual NativeThreads wrapper performs remaining FLS exit
    }
    Snapshot snapshot()const{PreserveError error;std::lock_guard lock(gate);std::size_t pending=0;for(const auto& row:rows)pending+=row.stage!=Stage::Empty;return {phase,pending,active,started,completed,detached};}
};
NativeJobs::NativeJobs(std::shared_ptr<bootstrap_providers::NativeThreads> threads,std::size_t capacity,NativeJobApi api){
    if(!threads||!threads->memory_domain()||!capacity||!api.create_event||!api.set_event||!api.duplicate||!api.wait||!api.exit_code||!api.close||!api.post)throw std::invalid_argument("Exact Domain NativeThreads, initial native Jobs capacity and complete SDK required");
    impl_=std::make_unique<Impl>(std::move(threads),capacity,api);
}
NativeJobs::Impl& NativeJobs::attached()const{if(!impl_)throw std::logic_error("Native Jobs facade is unattached");return *impl_;}
NativeJobs::~NativeJobs()noexcept{if(!impl_)return;auto& v=*impl_;{PreserveError error;std::lock_guard lock(v.gate);if(v.active||v.binding||v.checking||v.lease||v.ports||v.application||v.checkpoint||(v.phase!=Phase::Fresh&&v.phase!=Phase::Closed))std::terminate();for(const auto& row:v.rows)if(row.stage!=Impl::Stage::Empty||row.handoff||row.original||row.joined||row.barrier||row.work)std::terminate();v.phase=Phase::Retiring;}{auto retiring=std::move(v.threads);}}
void NativeJobs::bind_prepared(sp::Application& app,rt::Ports ports,std::function<void()> checkpoint){
    auto& v=attached();{PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||v.binding){if(v.binding||v.active)v.phase=Phase::Poisoned;throw std::logic_error("Native Jobs binds once");}v.binding=true;}
    try{
        const auto storage=ports.accept.storage_bindings();
        if(!checkpoint||!ports.server_identity||!ports.view.identity.value||ports.view.memory.memory_domain_identity()!=v.threads->memory_domain().get()||storage.data()!=ports.environment.storage.current_storage().data()||storage.size()!=ports.environment.storage.current_storage().size())throw std::invalid_argument("Native Jobs requires exact same-Domain Ports/Application");
        (void)ports.view.memory.buffer_context();const auto backend=ports.view.backend.snapshot().phase;
        if((backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running)||ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports.accept.snapshot().poisoned)throw std::logic_error("Native Jobs requires open source graph");
        v.ports.emplace(ports);v.application=&app;v.checkpoint.swap(checkpoint);checkpoint={};
        v.lease=ports.view.memory.acquire_provider_lease_from_operation();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh||!v.binding)throw std::logic_error("Native Jobs bind retirement was poisoned");v.thread=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();v.binding=false;v.phase=Phase::Bound;}
    }catch(...){v.poison();PreserveError error;std::lock_guard lock(v.gate);v.binding=false;throw;}
}
s::FrameThreadHandle NativeJobs::start(sp::JobToken,std::function<void()>){throw std::logic_error("Native Jobs requires Profile PreparedJobs selection; no legacy runner fallback");}
s::FrameThreadHandle NativeJobs::start_prepared(sp::Application& app,sp::JobToken token){
    PreserveError error;auto& v=attached();v.begin();Impl::Row* row=nullptr;
    try{
        v.match(app);v.admit();
        v.reserve_row(token,row);
        row->handoff.emplace(app.prepare_job(token));v.admit();
        auto work=v.ports->view.scheduler.work_count_30.load();for(;;){if(!work||work==UINT32_MAX)throw std::overflow_error("Native Jobs requires live bounded scheduler work");if(v.ports->view.scheduler.work_count_30.compare_exchange_weak(work,work+1))break;}
        {std::lock_guard lock(v.gate);row->work=true;}
        row->barrier=v.api.create_event(nullptr,TRUE,FALSE,nullptr);if(!row->barrier)throw std::runtime_error("Native Jobs detach barrier creation failed");v.admit();
        std::uint32_t id=0;row->original=reinterpret_cast<HANDLE>(threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&Impl::entry),reinterpret_cast<std::uintptr_t>(row),0,&id,*v.threads));row->id=id;
        if(!row->original||!row->id)throw std::runtime_error("Native Jobs native launch failed");v.admit();
        HANDLE joined=nullptr;const auto duplicated=v.api.duplicate(::GetCurrentProcess(),row->original,::GetCurrentProcess(),&joined,0,FALSE,DUPLICATE_SAME_ACCESS);row->joined=joined;
        if(!duplicated||!row->joined)throw std::runtime_error("Native Jobs physical wait handle duplication failed");v.admit();
        {std::lock_guard lock(v.gate);row->stage=Impl::Stage::Launched;++v.started;}
        const s::FrameThreadHandle result{reinterpret_cast<std::uintptr_t>(row->original),row->id};v.end();return result;
    }catch(...){v.failed(row);throw;} // no speculative cancellation of launched/unknown prefix
}
std::int32_t NativeJobs::detach(const s::FrameThreadHandle& handle){
    PreserveError error;auto& v=attached();v.begin();Impl::Row* row=nullptr;
    try{
        v.admit();{std::lock_guard lock(v.gate);for(auto& r:v.rows)if(r.stage!=Impl::Stage::Empty&&reinterpret_cast<std::uintptr_t>(r.original)==handle.handle&&r.id==handle.id){if(row)throw std::logic_error("Ambiguous Native Jobs handle");row=&r;}if(!row||row->stage!=Impl::Stage::Launched||row->detached||!row->joined)throw std::invalid_argument("Native Jobs detach handle is stale or unknown");}
        if(!v.api.close(row->original))throw std::runtime_error("Native Jobs source detach handle close failed");row->original=nullptr;v.admit();
        {std::lock_guard lock(v.gate);row->detached=true;++v.detached;}
        if(!v.api.set_event(row->barrier))throw std::runtime_error("Native Jobs detach publication failed");v.admit();v.end();return 0;
    }catch(...){v.failed(row);throw;}
}
void NativeJobs::returned_prepared(sp::Application& app,std::uint32_t receipt){
    PreserveError error;auto& v=attached();v.begin();Impl::Row* row=nullptr;
    try{
        v.match(app);v.admit();{std::lock_guard lock(v.gate);for(auto& r:v.rows)if(r.stage!=Impl::Stage::Empty&&r.receipt==receipt){if(row)throw std::logic_error("Ambiguous native job receipt");row=&r;}if(!row||!row->detached||!row->work||row->stage!=Impl::Stage::Returned)throw std::logic_error("Native job receipt is foreign, duplicate or not normally returned");}
        app.complete_job(*row->handoff);v.admit();
        {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||row->stage!=Impl::Stage::Returned||!row->work)throw std::logic_error("Native job acknowledgement was poisoned");row->stage=Impl::Stage::Consumed;row->work=false;++v.completed;}
        v.end(); // dispatcher now owns exactly one scheduler work decrement
    }catch(...){v.failed(row);throw;}
}
bool NativeJobs::quiescent_prepared(sp::Application& app){
    PreserveError error;auto& v=attached();{std::lock_guard lock(v.gate);v.affinity();v.match(app);if(v.phase==Phase::Closed)return true;}v.begin();
    try{
        v.match(app);const auto p=app.ports();if(&p.view.memory!=&v.ports->view.memory||&p.view.scheduler!=&v.ports->view.scheduler)throw std::invalid_argument("Native Jobs physical drain belongs to another Parent");
        const auto backend=p.view.backend.snapshot().phase;if(backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running&&backend!=sp::o::Phase::Returned&&backend!=sp::o::Phase::Quiescing)throw std::logic_error("Native Jobs physical drain requires live/returned source graph");
        // Never wait for a child source gate while this actor owns Parent.
        // Metadata gate is not held across this probe or the source gate.
        if(p.view.memory.operation_on_this_thread())throw std::logic_error("Native Jobs physical maintenance requires outside Parent");
        if(p.view.memory.snapshot().phase!=s::SessionMemoryPhase::Open)throw std::logic_error("Native Jobs Parent is poisoned or closed");
        std::unique_lock source(v.source_gate);v.seal();
        // Waiting children cannot acquire/enter Parent during its unlocked
        // reap reservation. Other launched rows need not starve old row reuse.
        (void)p.view.memory.reap_jobs();v.seal();bool pending=false;
        for(auto& row:v.rows){
            {std::lock_guard lock(v.gate);if(row.stage==Impl::Stage::Empty)continue;if(row.stage==Impl::Stage::Unknown)throw std::logic_error("Native Jobs physical drain has unknown source work");if(row.stage!=Impl::Stage::Consumed){pending=true;continue;}if(row.work||!row.detached||row.original||!row.joined)throw std::logic_error("Native Jobs physical drain retains incomplete source work");}
            v.seal();const auto wait=v.api.wait(row.joined,0);v.seal();if(wait==WAIT_TIMEOUT){pending=true;continue;}if(wait!=WAIT_OBJECT_0)throw std::runtime_error("Native Jobs physical wait outcome unknown");
            DWORD code=STILL_ACTIVE;const auto exited=v.api.exit_code(row.joined,&code);v.seal();if(!exited||code!=0)throw std::runtime_error("Native Jobs native/FLS exit status is not normal");
            {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||row.stage!=Impl::Stage::Consumed||!row.notified||row.failure)throw std::logic_error("Native Jobs physical result was poisoned");row.physical=true;}
            if(!v.api.close(row.joined))throw std::runtime_error("Native Jobs physical handle close failed");row.joined=nullptr;v.seal();
            if(!v.api.close(row.barrier))throw std::runtime_error("Native Jobs barrier close failed");row.barrier=nullptr;v.seal();
            row.handoff.reset();{std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound)throw std::logic_error("Native Jobs retirement was poisoned");row.stage=Impl::Stage::Empty;row.token={};row.receipt=0;row.id=0;row.detached=false;row.notified=false;row.physical=false;}
        }
        v.end();return !pending;
    }catch(...){v.failed();throw;}
}
bool NativeJobs::close_prepared(sp::Application& app,bool attested){
    PreserveError error;auto& v=attached();{std::lock_guard lock(v.gate);v.affinity();v.match(app);if(v.phase==Phase::Closed)return true;}v.begin();
    try{
        v.match(app);v.admit(true);if(!attested){v.end();return false;}
        {std::lock_guard lock(v.gate);for(const auto& row:v.rows)if(row.stage!=Impl::Stage::Empty||row.handoff||row.original||row.joined||row.barrier||row.work){v.active=false;return false;}}
        const auto memory=v.ports->view.memory.snapshot();const auto io=v.ports->view.io.snapshot();const auto accept=v.ports->accept.snapshot();const auto backend=v.ports->view.backend.snapshot().phase;
        if((backend!=sp::o::Phase::Returned&&backend!=sp::o::Phase::Quiescing)||memory.allocations||memory.controls||memory.proxies||memory.blocks||memory.maps||memory.buffers||memory.reserved||memory.pending_frees||memory.partial_constructions||memory.buffer_borrows||io.allocations||io.pending||io.active_calls||accept.projections||accept.pending||accept.active_calls||v.ports->view.scheduler.work_count_30.load()||v.ports->view.scheduler.completion.head||v.ports->view.scheduler.completion.tail){v.end();return false;}
        {auto retiring=std::move(v.checkpoint);v.checkpoint={};}
        {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||!v.active)throw std::logic_error("Native Jobs admission retirement was poisoned");}
        v.lease.reset();v.ports.reset();v.closed_application=v.application;v.application=nullptr;
        {std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound||!v.active)throw std::logic_error("Native Jobs provider retirement was poisoned");v.active=false;v.phase=Phase::Closed;}return true;
    }catch(...){v.failed();throw;}
}
Snapshot NativeJobs::snapshot()const{return attached().snapshot();}
} // namespace anadius::owned_jobs
