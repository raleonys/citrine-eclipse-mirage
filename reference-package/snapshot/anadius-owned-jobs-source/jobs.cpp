#include "jobs.hpp"
#include <atomic>
#include <exception>
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
        if(!checkpoint||!ports.server_identity||!ports.view.identity.value||storage.data()!=ports.environment.storage.storage.data()||storage.size()!=ports.environment.storage.storage.size())throw std::invalid_argument("Inline jobs require exact stable source Ports and admission");
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
} // namespace anadius::owned_jobs
