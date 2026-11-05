#include "memory.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include <algorithm>
#include <cstring>
#include <exception>
#include <list>
#include <limits>
#include <new>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace anadius::socket_recovered {
namespace {
struct PreserveError {
    DWORD value=::GetLastError();
    ~PreserveError(){::SetLastError(value);}
    void restore()const{::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
std::uintptr_t identity(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
constexpr std::size_t maximum=static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max());
template<class Error=std::logic_error>[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* message){
    // Exception strings can allocate and invoke a handler. Never construct one
    // while the nonrecursive metadata gate is owned.
    lock.unlock();throw Error(message);
}
}
struct SessionMemoryDomain::ActorState {
    struct ViewBorrow {void* ticket=nullptr;};
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    DWORD slot=FLS_OUT_OF_INDEXES;void* ptd=nullptr;
    bool active=false,mutating=false,constructing=false,reserving_view=false;
    std::unique_ptr<worker_handoff::PreparedMemory::JobScope> job_scope;
    std::list<ViewBorrow> views;
};
struct SessionMemoryDomain::Impl {
    enum class Kind {Control,Proxy,Block,Map,Bytes};
    enum class State {Reserved,Live,Retired};
    struct Ticket {
        Kind kind;State state=State::Reserved;
        void* raw=nullptr;void* object=nullptr;std::uintptr_t key=0;
        std::size_t extent=0,elements=0,borrows=0,views=0;std::uint64_t native_free=0;
        bool typed_live=false,has_region=false,busy=false;ReceiveBufferRegion region{};
        explicit Ticket(Kind k):kind(k){}
    };
    using Iterator=std::list<Ticket>::iterator;
    struct Lease {
        std::shared_ptr<SessionMemoryDomain> root;Ticket* ticket=nullptr;bool armed=false;
        explicit Lease(std::shared_ptr<SessionMemoryDomain> r):root(std::move(r)){}
        ~Lease()noexcept{
            if(!armed)return;
            PreserveError error;auto& v=*root->impl_;std::unique_lock lock(v.gate);
            if(ticket){if(!ticket->borrows||!v.buffer_borrows)std::terminate();--ticket->borrows;--v.buffer_borrows;}
            else{if(!v.provider_leases)std::terminate();--v.provider_leases;}
        }
    };
    struct Buffer final:ReceiveBufferApi {
        Impl& state;explicit Buffer(Impl& v):state(v){}
        std::uintptr_t allocate_04b0bc(std::uint64_t)override;
        void copy_bytes(std::uintptr_t,std::uintptr_t,std::uint64_t)override;
        void fill_bytes(std::uintptr_t,std::uint8_t,std::uint64_t)override;
        void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{state.owner.free_sized_04b0f8(p,n);}
        [[noreturn]]void length_error_00ce80()override;
        [[noreturn]]void allocation_overflow_00cd30()override;
        [[noreturn]]void invalid_parameter_05c974()override{state.owner.invalid_parameter_05c974();}
    };
    struct Resolver final:ReceiveBufferResolver,AllocationHeaderResolver,DequeMapResolver {
        Impl& state;explicit Resolver(Impl& s):state(s){}
        MemoryLookup lookup_bytes(std::uintptr_t,std::uint64_t,std::span<std::uint8_t>&)override;
        MemoryLookup lookup_header(std::uintptr_t,std::uintptr_t&)override;
        MemoryLookup lookup_map(std::uintptr_t,std::uint64_t,std::span<WriteFinishString*>&)override;
        MemoryLookup lookup_address(std::span<WriteFinishString*>,std::uintptr_t&)override;
    };
    struct Construction {ConstructionScratch scratch;SessionPair pair;};
    struct Admission {DWORD thread;void* fiber;Admission* next=nullptr;};
    SessionMemoryDomain& owner;const OperationPolicy policy;
    std::shared_ptr<thread_domain::ApplicationMemory> owned_backing;
    std::optional<worker_handoff::PreparedMemory::Borrow> borrowed_backing;
    thread_domain::ApplicationMemory* backing;
    std::shared_ptr<LifecycleEffects> lifecycle;
    const thread_domain::DomainStorage* source_domain;
    // Only Serialized owns this gate for a source body. The shared metadata
    // gate is never held across a backing/lifecycle/allocator/OS effect.
    std::recursive_mutex serial_gate;mutable std::mutex gate;
    SessionMemoryPhase phase=SessionMemoryPhase::Open;
    std::size_t admissions=0,view_borrows=0;
    Admission* pending_admissions=nullptr;
    bool preparing_lease=false,closing=false,reaping=false;
    DWORD reaping_thread=0;
    std::list<std::shared_ptr<ActorState>> actors;
    std::list<Ticket> tickets;std::vector<ReceiveBufferRegion> regions;
    std::vector<DequeMapBinding> maps;std::list<Construction> constructions;
    Resolver resolver{*this};ReceiveBufferMemory memory{};Buffer buffer{*this};
    ReceiveBufferContext receive{memory,buffer};SessionDisposalContext disposal;
    std::size_t allocations=0,frees=0,controls_destroyed=0,provider_leases=0,buffer_borrows=0;
    Impl(SessionMemoryDomain& o,std::shared_ptr<thread_domain::ApplicationMemory> b,std::shared_ptr<LifecycleEffects> l,OperationPolicy p)
        :owner(o),policy(p),owned_backing(std::move(b)),backing(owned_backing.get()),lifecycle(std::move(l)),source_domain(backing->domain_identity()),disposal{memory,o,*lifecycle}{
        if(policy==OperationPolicy::OriginalConcurrentActors)memory.resolver=&resolver;
    }
    Impl(SessionMemoryDomain& o,worker_handoff::PreparedMemory::Borrow&& b,std::shared_ptr<LifecycleEffects> l,OperationPolicy p)
        :owner(o),policy(p),borrowed_backing(std::move(b)),backing(&borrowed_backing->memory()),lifecycle(std::move(l)),source_domain(backing->domain_identity()),disposal{memory,o,*lifecycle}{
        if(policy==OperationPolicy::OriginalConcurrentActors)memory.resolver=&resolver;
    }
    void poison(){PreserveError error;std::unique_lock lock(gate);phase=SessionMemoryPhase::Poisoned;}
    void finish_admission_locked(Admission& admission)noexcept{
        auto** at=&pending_admissions;
        while(*at&&*at!=&admission)at=&(*at)->next;
        if(!*at||!admissions)std::terminate();*at=admission.next;--admissions;
    }
    std::shared_ptr<ActorState> current_locked(DWORD thread,void* fiber,std::unique_lock<std::mutex>& lock)const{
        for(const auto& actor:actors)if(actor->active&&actor->thread==thread&&actor->fiber==fiber)return actor;
        reject(lock,"Session memory requires an explicitly admitted thread/fiber Operation");
    }
    void check_locked(const ActorState& actor,std::unique_lock<std::mutex>& lock)const{
        if(phase!=SessionMemoryPhase::Open||!actor.active)reject(lock,"Session memory actor is closed or has an unknown outcome");
    }
    bool ptd_matches(const ActorState& actor)const noexcept{
        PreserveError error;
        return backing&&source_domain->native_fls_index()==actor.slot&&::FlsGetValue(actor.slot)==actor.ptd;
    }
    void require_actor(const std::shared_ptr<ActorState>& actor){
        PreserveError error;
        {std::unique_lock lock(gate);check_locked(*actor,lock);}
        try{
            if(actor->job_scope){
                if(&actor->job_scope->memory()!=backing){poison();throw std::logic_error("Job permission backing changed");}
            }else if(borrowed_backing)(void)borrowed_backing->memory();
            if(!ptd_matches(*actor)){poison();throw std::logic_error("Backing-domain PTD changed within Session memory Operation");}
        }catch(...){poison();throw;}
        {std::unique_lock lock(gate);check_locked(*actor,lock);}
    }
    struct Access {
        Impl& v;std::shared_ptr<ActorState> actor;bool mutation;
        Access(Impl& value,bool mutate,bool pure_lookup=false):v(value),mutation(false){
            PreserveError error;
            const auto thread=::GetCurrentThreadId();auto* const fiber=::GetCurrentFiber();
            bool busy;
            {
                std::unique_lock lock(v.gate);actor=v.current_locked(thread,fiber,lock);v.check_locked(*actor,lock);
                busy=actor->reserving_view||(actor->mutating&&!pure_lookup);
                if(mutate&&!busy){actor->mutating=true;mutation=true;}
            }
            // Observe PTD corruption even when a lower catches this reentry
            // rejection and restores its slot before the outer effect returns.
            try{v.require_actor(actor);if(busy)throw std::logic_error("Backing allocation/free effect reentry is unsupported");}
            catch(...){if(mutation){std::unique_lock lock(v.gate);actor->mutating=false;}throw;}
        }
        ~Access(){PreserveError error;if(mutation){std::unique_lock lock(v.gate);actor->mutating=false;}}
        void check(){v.require_actor(actor);}
    };
    // Serialized legacy consumers still borrow these vectors. Concurrent mode
    // never publishes a mutable union catalog; its exact resolver is mandatory.
    void reserve_legacy(){
        if(policy!=OperationPolicy::Serialized)return;
        PreserveError error;std::size_t count;
        {std::unique_lock lock(gate);count=tickets.size()+1;}
        regions.reserve(count);memory.regions=regions;maps.reserve(count);
    }
    void refresh_locked()noexcept{
        if(policy!=OperationPolicy::Serialized)return;
        regions.clear();maps.clear();
        for(const auto& t:tickets)if(t.state==State::Live&&!t.busy){
            if(t.has_region)regions.push_back(t.region);
            if(t.kind==Kind::Map&&t.typed_live)maps.push_back({identity(t.object),{static_cast<WriteFinishString**>(t.object),t.elements}});
        }
        memory.regions=regions;
    }
    Iterator allocate(Access& access,Kind kind,std::size_t extent,std::uint64_t native_free){
        PreserveError error;
        if(backing->tracking_poisoned()){poison();throw std::logic_error("Backing allocation tracking is unknown");}
        std::list<Ticket> prepared;prepared.emplace_back(kind);
        prepared.front().extent=extent;prepared.front().native_free=native_free;
        reserve_legacy();access.check();auto ticket=prepared.begin();
        {std::unique_lock lock(gate);check_locked(*access.actor,lock);tickets.splice(tickets.end(),prepared);}
        void* returned=nullptr;error.restore();
        try{
            returned=backing->allocate(extent);error.capture();
            // A Reserved row is private to this attempt. Publish the returned
            // prefix before any post-effect check can throw; observers inspect
            // only its Reserved state until the later typed commit.
            ticket->raw=returned;ticket->key=identity(returned);
            {std::unique_lock lock(gate);++allocations;}
            access.check();return ticket;
        }catch(...){
            error.capture();bool clean=false;
            try{clean=!returned&&ptd_matches(*access.actor)&&!backing->tracking_poisoned();}catch(...){}
            std::list<Ticket> retired;
            {std::unique_lock lock(gate);
                if(clean&&phase==SessionMemoryPhase::Open)retired.splice(retired.end(),tickets,ticket);
                else phase=SessionMemoryPhase::Poisoned;
            }
            // No global allocation-count comparison: another healthy actor may
            // have allocated or freed while this particular attempt failed.
            throw;
        }
    }
    void publish(Access& access,Iterator ticket){
        PreserveError error;
        access.check();std::unique_lock lock(gate);check_locked(*access.actor,lock);
        ticket->state=State::Live;refresh_locked();
    }
    bool retained_live(void* raw)noexcept{
        try{return backing->allocation_state(raw)==thread_domain::ApplicationMemory::AllocationState::Live;}catch(...){return false;}
    }
    void drop_own_view_locked(ActorState& actor,Ticket& ticket,std::list<ActorState::ViewBorrow>& retired){
        for(auto i=actor.views.begin();i!=actor.views.end();++i)if(i->ticket==&ticket){
            if(!ticket.views||!view_borrows)std::terminate();
            --ticket.views;--view_borrows;retired.splice(retired.end(),actor.views,i);return;
        }
    }
    void reserve_retire_locked(ActorState& actor,Ticket& ticket,std::list<ActorState::ViewBorrow>& retired,std::unique_lock<std::mutex>& lock){
        if(ticket.busy)reject(lock,"Session allocation already has an active effect");
        const auto own=std::any_of(actor.views.begin(),actor.views.end(),[&](const auto& b){return b.ticket==&ticket;})?1u:0u;
        if(ticket.borrows||ticket.views>own)reject(lock,"Another live actor or pending SDK borrower still owns this allocation range");
        drop_own_view_locked(actor,ticket,retired);ticket.busy=true;
    }
    void destroy(Iterator ticket)noexcept{
        bool typed;Kind kind;void* object;std::size_t elements;
        {std::unique_lock lock(gate);typed=ticket->typed_live;kind=ticket->kind;object=ticket->object;elements=ticket->elements;}
        if(!typed)return;
        switch(kind){
            case Kind::Control:std::destroy_at(static_cast<RecoveredSessionControl*>(object));break;
            case Kind::Proxy:std::destroy_at(static_cast<DequeProxy*>(object));break;
            case Kind::Block:std::destroy_at(static_cast<WriteFinishString*>(object));break;
            case Kind::Map:std::destroy_n(static_cast<WriteFinishString**>(object),elements);break;
            case Kind::Bytes:break;
        }
        {std::unique_lock lock(gate);ticket->typed_live=false;if(kind==Kind::Control)++controls_destroyed;}
    }
    void retire(Access& access,Iterator ticket){
        PreserveError error;
        destroy(ticket);
        {std::unique_lock lock(gate);ticket->has_region=false;ticket->state=State::Retired;refresh_locked();}
        error.restore();
        try{access.check();backing->deallocate(ticket->raw,ticket->extent);error.capture();access.check();}
        catch(...){
            error.capture();const bool live=retained_live(ticket->raw);
            std::unique_lock lock(gate);
            if(!live)phase=SessionMemoryPhase::Poisoned;
            else ticket->busy=false;
            throw;
        }
        std::list<Ticket> retired;
        {std::unique_lock lock(gate);++frees;retired.splice(retired.end(),tickets,ticket);refresh_locked();}
    }
    void cleanup_constructor(Access& access,Iterator ticket)noexcept{
        try{
            std::list<ActorState::ViewBorrow> retired;
            {std::unique_lock lock(gate);reserve_retire_locked(*access.actor,*ticket,retired,lock);}
            retire(access,ticket);
        }catch(...){}
    }
    // Lookup selects and pins ONE stable ticket atomically. Preparation of the
    // extra C++ list node runs outside the metadata gate with a temporary pin.
    template<class Find>MemoryLookup pin_lookup(const std::shared_ptr<ActorState>& actor,Find&& find){
        PreserveError error;Ticket* ticket=nullptr;
        {
            std::unique_lock lock(gate);check_locked(*actor,lock);
            if(actor->reserving_view)reject(lock,"Session view metadata allocation reentry");
            const auto result=find(ticket);if(result!=MemoryLookup::Found)return result;
            for(const auto& view:actor->views)if(view.ticket==ticket)return MemoryLookup::Found;
            if(ticket->views==std::numeric_limits<std::size_t>::max()||view_borrows==std::numeric_limits<std::size_t>::max())reject<std::overflow_error>(lock,"Session view borrow counter exhausted");
            ++ticket->views;++view_borrows;actor->reserving_view=true;
        }
        std::list<ActorState::ViewBorrow> prepared;
        try{
            prepared.push_back({ticket});require_actor(actor);
            std::unique_lock lock(gate);check_locked(*actor,lock);
            if(!actor->reserving_view||ticket->state!=State::Live||ticket->busy)reject(lock,"Session view reservation changed");
            actor->views.splice(actor->views.end(),prepared);actor->reserving_view=false;
            return MemoryLookup::Found;
        }catch(...){
            std::unique_lock lock(gate);--ticket->views;--view_borrows;actor->reserving_view=false;throw;
        }
    }
};
static_assert(alignof(RecoveredSessionControl)<=MEMORY_ALLOCATION_ALIGNMENT);
static_assert(alignof(DequeProxy)<=MEMORY_ALLOCATION_ALIGNMENT&&sizeof(DequeProxy)==16);
static_assert(alignof(WriteFinishString)<=MEMORY_ALLOCATION_ALIGNMENT&&sizeof(WriteFinishString)==32);
static_assert(std::is_nothrow_copy_constructible_v<ReceiveBufferRegion>);
static_assert(std::is_nothrow_copy_constructible_v<DequeMapBinding>);
static_assert(std::is_nothrow_destructible_v<RecoveredSessionControl>);

SessionMemoryDomain::SessionMemoryDomain(std::shared_ptr<thread_domain::ApplicationMemory>&& backing,std::shared_ptr<LifecycleEffects> lifecycle,OperationPolicy policy){
    if(policy!=OperationPolicy::Serialized&&policy!=OperationPolicy::OriginalConcurrentActors)throw std::invalid_argument("Unknown session memory operation policy");
    if(!backing||backing.use_count()!=1||!lifecycle)throw std::invalid_argument("A sole-owned dedicated ApplicationMemory and owning lifecycle provider are required");
    if(backing->tracking_poisoned()||backing->outstanding_allocations()!=0)throw std::invalid_argument("Backing memory must be clean and empty");
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(lifecycle),policy);
}
SessionMemoryDomain::SessionMemoryDomain(worker_handoff::PreparedMemory::Borrow&& backing,std::shared_ptr<LifecycleEffects> lifecycle,OperationPolicy policy){
    if(policy!=OperationPolicy::Serialized&&policy!=OperationPolicy::OriginalConcurrentActors)throw std::invalid_argument("Unknown session memory operation policy");
    auto& provider=backing.memory();
    if(!lifecycle||provider.tracking_poisoned()||provider.outstanding_allocations())throw std::invalid_argument("A clean dedicated worker borrow and lifecycle provider are required");
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(lifecycle),policy);
}
SessionMemoryDomain::~SessionMemoryDomain()noexcept{
    const auto& v=*impl_;
    if(!v.actors.empty()||v.admissions||v.pending_admissions||v.preparing_lease||v.closing||v.reaping||v.view_borrows||v.provider_leases||v.buffer_borrows||!v.tickets.empty()||!v.constructions.empty()||v.phase!=SessionMemoryPhase::Closed)std::terminate();
}
SessionMemoryDomain::Operation::Operation(Impl& v):Operation(v,nullptr){}
SessionMemoryDomain::Operation::Operation(Impl& v,worker_handoff::PreparedMemory::JobMemory* job):state_(v),serial_lock_(v.serial_gate,std::defer_lock){
    PreserveError error;if(v.policy==OperationPolicy::Serialized)serial_lock_.lock();
    const auto thread=::GetCurrentThreadId();Impl::Admission admission{thread,::GetCurrentFiber()};
    {
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||v.preparing_lease||v.closing||(v.policy==OperationPolicy::Serialized&&(v.reaping||!v.actors.empty())))reject(lock,"Session memory Operation is closed, poisoned or reentrant");
        if(v.reaping&&v.reaping_thread==thread)reject(lock,"Session memory cannot enter during this actor's job reaping");
        for(const auto& actor:v.actors)if(actor->active&&actor->thread==thread)reject(lock,"Session memory cannot nest another Operation on its actor thread");
        for(auto* pending=v.pending_admissions;pending;pending=pending->next)if(pending->thread==thread)reject(lock,"Session memory actor admission cannot reenter during metadata preparation");
        if(v.admissions==std::numeric_limits<std::size_t>::max())reject<std::overflow_error>(lock,"Session actor admission counter exhausted");
        admission.next=v.pending_admissions;v.pending_admissions=&admission;++v.admissions;
    }
    try{
        std::list<std::shared_ptr<ActorState>> prepared;
        prepared.push_back(std::make_shared<ActorState>());auto actor=prepared.front();
        if(job){
            if(!v.borrowed_backing)throw std::invalid_argument("Job entry requires this Parent's prepared backing borrow");
            actor->job_scope=v.borrowed_backing->pin_job(*job);
            if(&actor->job_scope->memory()!=v.backing)throw std::invalid_argument("Job entry requires exact Parent backing");
        }else if(v.borrowed_backing)(void)v.borrowed_backing->memory();
        actor->slot=v.source_domain->native_fls_index();actor->ptd=::FlsGetValue(actor->slot);
        if(actor->slot==FLS_OUT_OF_INDEXES||!actor->ptd||actor->ptd==crt_ptd_recovered::construction_sentinel())throw std::logic_error("Session memory requires a ready PTD in its backing domain on this fiber");
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||v.preparing_lease||v.closing||(v.policy==OperationPolicy::Serialized&&(v.reaping||!v.actors.empty())))reject(lock,"Session actor admission changed during metadata allocation");
        for(const auto& current:v.actors)if(current->active&&current->thread==actor->thread)reject(lock,"Session actor thread was admitted during preparation");
        actor->active=true;v.actors.splice(v.actors.end(),prepared);actor_=std::move(actor);v.finish_admission_locked(admission);
    }catch(...){std::unique_lock lock(v.gate);v.finish_admission_locked(admission);throw;}
}
SessionMemoryDomain::Operation::~Operation()noexcept{
    PreserveError error;auto& v=state_;
    if(!actor_||::GetCurrentThreadId()!=actor_->thread||::GetCurrentFiber()!=actor_->fiber)std::terminate();
    const bool valid=v.ptd_matches(*actor_)&&(!actor_->job_scope||actor_->job_scope->valid());
    std::list<std::shared_ptr<ActorState>> retired;std::list<ActorState::ViewBorrow> retired_views;
    std::unique_ptr<worker_handoff::PreparedMemory::JobScope> scope;
    {
        std::unique_lock lock(v.gate);
        if(!actor_->active||actor_->mutating||actor_->constructing||actor_->reserving_view)std::terminate();
        if(!valid)v.phase=SessionMemoryPhase::Poisoned;
        for(const auto& view:actor_->views){
            auto* ticket=static_cast<Impl::Ticket*>(view.ticket);
            if(!ticket->views||!v.view_borrows)std::terminate();--ticket->views;--v.view_borrows;
        }
        retired_views.splice(retired_views.end(),actor_->views);actor_->active=false;
        const auto found=std::find(v.actors.begin(),v.actors.end(),actor_);if(found==v.actors.end())std::terminate();
        retired.splice(retired.end(),v.actors,found);scope=std::move(actor_->job_scope);
    }
    // Scope return and all metadata destruction occur after releasing Parent.
    scope.reset();
}
SessionMemoryDomain::JobPermit::JobPermit(std::shared_ptr<SessionMemoryDomain> parent,std::shared_ptr<ActorState> seal)noexcept:parent_(std::move(parent)),seal_(std::move(seal)){}
SessionMemoryDomain::JobPermit SessionMemoryDomain::Operation::job_permit(){
    PreserveError error;
    if(actor_->thread!=::GetCurrentThreadId()||actor_->fiber!=::GetCurrentFiber())throw std::logic_error("Job permit requires its Operation's actual thread/fiber");
    state_.require_actor(actor_);
    {std::unique_lock lock(state_.gate);
        if(!actor_->job_scope||actor_->mutating||actor_->constructing||actor_->reserving_view)reject(lock,"A live explicit job Parent Operation is required");
    }
    return JobPermit(state_.owner.shared_from_this(),actor_);
}
void SessionMemoryDomain::JobPermit::check(const SessionMemoryDomain& parent)const{
    PreserveError error;
    if(!parent_||!seal_||parent_.get()!=&parent)throw std::invalid_argument("Job permit requires exact Parent identity");
    if(::GetCurrentThreadId()!=seal_->thread||::GetCurrentFiber()!=seal_->fiber)throw std::logic_error("Job permit belongs to its original child thread/fiber");
    auto& v=*parent_->impl_;
    {std::unique_lock lock(v.gate);v.check_locked(*seal_,lock);
        if(!seal_->job_scope||seal_->mutating||seal_->constructing||seal_->reserving_view)reject(lock,"Job permit is not a healthy exact Parent source scope");
    }
    v.require_actor(seal_);
}
SessionMemoryDomain::Operation SessionMemoryDomain::enter(){return Operation(*impl_);}
SessionMemoryDomain::Operation SessionMemoryDomain::enter(worker_handoff::PreparedMemory::JobMemory& job){return Operation(*impl_,&job);}
worker_handoff::PreparedMemory::JobMemory SessionMemoryDomain::prepare_job(){
    PreserveError error;auto& v=*impl_;Impl::Access access(v,true);
    if(access.actor->job_scope||!v.borrowed_backing)throw std::logic_error("Job offers require the original prepared worker borrow");
    auto result=v.borrowed_backing->prepare_job();access.check();return result;
}
std::size_t SessionMemoryDomain::reap_jobs(){
    PreserveError error;auto& v=*impl_;const auto thread=::GetCurrentThreadId();
    {
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||v.reaping||v.closing||v.preparing_lease||!v.borrowed_backing||
           (v.policy==OperationPolicy::Serialized&&!v.actors.empty()))reject(lock,"Job reaping requires available original-worker Parent");
        for(const auto& actor:v.actors)if(actor->thread==thread)reject(lock,"Job reaping must be outside this worker's Operation");
        v.reaping=true;v.reaping_thread=thread;
    }
    struct Active {Impl& v;~Active(){std::unique_lock lock(v.gate);v.reaping=false;v.reaping_thread=0;}} active{v};
    try{
        (void)v.borrowed_backing->memory();const auto result=v.borrowed_backing->reap_jobs();
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||!v.reaping)reject(lock,"Parent job reaping was poisoned");
        return result;
    }catch(...){
        // Prepared reaping may fail while preparing passive snapshot metadata,
        // before any OS outcome is entered. Native wait/close unknowns poison
        // that lower; validate its exact actual-worker capability rather than
        // guessing from exception type or a global allocation count.
        bool healthy=false;
        try{healthy=&v.borrowed_backing->memory()==v.backing&&!v.backing->tracking_poisoned();}catch(...){}
        if(!healthy)v.poison(); // Never clears a poison already observed here.
        throw;
    }
}
bool SessionMemoryDomain::operation_on_this_thread()const{
    PreserveError error;auto& v=*impl_;const auto thread=::GetCurrentThreadId();std::unique_lock lock(v.gate);
    for(const auto& actor:v.actors)if(actor->active&&actor->thread==thread)return true;
    return false;
}
SessionControl* SessionMemoryDomain::allocate_control_04b0bc(std::uint64_t request){
    auto& v=*impl_;Impl::Access access(v,true);
    if(request!=0x1c8)throw std::invalid_argument("Session control native request must be1c8");
    auto ticket=v.allocate(access,Impl::Kind::Control,sizeof(RecoveredSessionControl),0x1c8);
    RecoveredSessionControl* control=nullptr;
    try{control=::new(ticket->raw) RecoveredSessionControl(v.disposal);}catch(...){v.cleanup_constructor(access,ticket);throw;}
    ticket->object=control;ticket->key=identity(static_cast<SessionControl*>(control));ticket->typed_live=true;
    ticket->region={identity(control->object.input.storage_00.data()),control->object.input.storage_00};ticket->has_region=true;
    v.publish(access,ticket);return control;
}
DequeProxy* SessionMemoryDomain::allocate_proxy_04b0bc(std::uint64_t request){
    auto& v=*impl_;Impl::Access access(v,true);
    if(request!=16)throw std::invalid_argument("Deque proxy native request must be16");
    auto ticket=v.allocate(access,Impl::Kind::Proxy,sizeof(DequeProxy),16);
    auto* proxy=::new(ticket->raw) DequeProxy{};ticket->object=proxy;ticket->typed_live=true;
    v.publish(access,ticket);return proxy;
}
WriteFinishString* SessionMemoryDomain::allocate_output_block(){
    auto& v=*impl_;Impl::Access access(v,true);
    auto ticket=v.allocate(access,Impl::Kind::Block,sizeof(WriteFinishString),32);
    auto* block=::new(ticket->raw) WriteFinishString{};ticket->object=block;ticket->typed_live=true;
    ticket->region={identity(block->storage_00.data()),block->storage_00};ticket->has_region=true;
    v.publish(access,ticket);return block;
}
std::span<WriteFinishString*> SessionMemoryDomain::allocate_output_map(std::uint64_t slots){
    auto& v=*impl_;Impl::Access access(v,true);
    if(!slots||(slots&(slots-1))!=0||slots>(maximum-39)/sizeof(WriteFinishString*))throw std::length_error("Output map requires a bounded positive power-of-two slot count");
    const auto count=static_cast<std::size_t>(slots),bytes=count*sizeof(WriteFinishString*);
    const auto extent=bytes+(bytes>=4096?39:0);
    auto ticket=v.allocate(access,Impl::Kind::Map,extent,extent);auto data=identity(ticket->raw);
    if(bytes>=4096){data=(data+39)&~std::uintptr_t{31};const auto raw=identity(ticket->raw);std::memcpy(reinterpret_cast<void*>(data-8),&raw,8);}
    auto* map=::new(reinterpret_cast<void*>(data)) WriteFinishString*[count]{};
    ticket->object=map;ticket->elements=count;ticket->typed_live=true;
    ticket->region={identity(ticket->raw),{static_cast<std::uint8_t*>(ticket->raw),extent}};ticket->has_region=true;
    v.publish(access,ticket);
    if(v.policy==OperationPolicy::OriginalConcurrentActors){
        PreserveError error;std::span<WriteFinishString*> borrowed;
        // The returned span itself is a live borrow of THIS ticket. Additional
        // metadata failure retains the already published allocation prefix.
        if(v.resolver.lookup_map(data,bytes,borrowed)!=MemoryLookup::Found||borrowed.data()!=map||borrowed.size()!=count)
            throw std::logic_error("Allocated output-map span lost its exact owned projection");
    }
    return {map,count};
}
std::uintptr_t SessionMemoryDomain::Impl::Buffer::allocate_04b0bc(std::uint64_t bytes){
    Access access(state,true);if(bytes>maximum)throw std::bad_array_new_length();
    auto ticket=state.allocate(access,Kind::Bytes,static_cast<std::size_t>(bytes),bytes);
    auto* raw=::new(ticket->raw) std::uint8_t[static_cast<std::size_t>(bytes)];
    ticket->object=raw;ticket->region={identity(raw),{raw,static_cast<std::size_t>(bytes)}};ticket->has_region=true;
    state.publish(access,ticket);return identity(raw);
}
void SessionMemoryDomain::Impl::Buffer::copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t bytes){
    Access access(state,true);if(!bytes)return;
    auto destination=state.memory.view(to,bytes),source=state.memory.view(from,bytes);
    std::memmove(destination.data(),source.data(),destination.size());access.check();
}
void SessionMemoryDomain::Impl::Buffer::fill_bytes(std::uintptr_t address,std::uint8_t value,std::uint64_t bytes){
    Access access(state,true);if(!bytes)return;auto destination=state.memory.view(address,bytes);
    std::fill(destination.begin(),destination.end(),value);access.check();
}
[[noreturn]]void SessionMemoryDomain::Impl::Buffer::length_error_00ce80(){Access access(state,false);throw std::length_error("Recovered receive string maximum exceeded");}
[[noreturn]]void SessionMemoryDomain::Impl::Buffer::allocation_overflow_00cd30(){Access access(state,false);throw std::bad_array_new_length();}
MemoryLookup SessionMemoryDomain::Impl::Resolver::lookup_bytes(std::uintptr_t address,std::uint64_t count,std::span<std::uint8_t>& out){
    Access access(state,false,true);std::span<std::uint8_t> found;
    const auto result=state.pin_lookup(access.actor,[&](Ticket*& ticket){
        bool matched=false;
        for(auto& t:state.tickets){
            if(t.state!=State::Live||t.busy||!t.has_region||address<t.region.address)continue;
            const auto offset=address-t.region.address;if(offset>t.region.bytes.size()||count>t.region.bytes.size()-offset)continue;
            if(matched)return MemoryLookup::Ambiguous;matched=true;ticket=&t;
            found=t.region.bytes.subspan(static_cast<std::size_t>(offset),static_cast<std::size_t>(count));
        }
        return matched?MemoryLookup::Found:MemoryLookup::Miss;
    });
    if(result==MemoryLookup::Found)out=found;return result;
}
MemoryLookup SessionMemoryDomain::Impl::Resolver::lookup_header(std::uintptr_t header,std::uintptr_t& out){
    std::span<std::uint8_t> bytes;const auto result=lookup_bytes(header,8,bytes);
    if(result==MemoryLookup::Found){std::uintptr_t base=0;for(unsigned i=0;i<8;++i)base|=std::uintptr_t{bytes[i]}<<(i*8);out=base;}
    return result;
}
MemoryLookup SessionMemoryDomain::Impl::Resolver::lookup_map(std::uintptr_t address,std::uint64_t bytes,std::span<WriteFinishString*>& out){
    Access access(state,false,true);if(bytes%sizeof(WriteFinishString*))return MemoryLookup::Miss;
    std::span<WriteFinishString*> found;
    const auto result=state.pin_lookup(access.actor,[&](Ticket*& ticket){
        bool matched=false;
        for(auto& t:state.tickets){
            if(t.state!=State::Live||t.busy||t.kind!=Kind::Map||!t.typed_live)continue;
            const auto start=identity(t.object);if(address<start)continue;const auto distance=address-start;
            if(distance%sizeof(WriteFinishString*))continue;
            const auto offset=distance/sizeof(WriteFinishString*),count=bytes/sizeof(WriteFinishString*);
            if(offset>t.elements||count>t.elements-offset)continue;
            if(matched)return MemoryLookup::Ambiguous;matched=true;ticket=&t;
            found={static_cast<WriteFinishString**>(t.object)+offset,static_cast<std::size_t>(count)};
        }
        return matched?MemoryLookup::Found:MemoryLookup::Miss;
    });
    if(result==MemoryLookup::Found)out=found;return result;
}
MemoryLookup SessionMemoryDomain::Impl::Resolver::lookup_address(std::span<WriteFinishString*> slots,std::uintptr_t& out){
    Access access(state,false,true);std::uintptr_t found=0;
    const auto result=state.pin_lookup(access.actor,[&](Ticket*& ticket){
        bool matched=false;
        for(auto& t:state.tickets){
            if(t.state!=State::Live||t.busy||t.kind!=Kind::Map||!t.typed_live||t.object!=slots.data()||slots.size()>t.elements)continue;
            if(matched)return MemoryLookup::Ambiguous;matched=true;ticket=&t;found=identity(t.object);
        }
        return matched?MemoryLookup::Found:MemoryLookup::Miss;
    });
    if(result==MemoryLookup::Found)out=found;return result;
}
ReceiveBufferContext& SessionMemoryDomain::buffer_context(){auto& v=*impl_;Impl::Access access(v,false);return v.receive;}
AllocationHeaderResolver& SessionMemoryDomain::allocation_header_resolver(){auto& v=*impl_;Impl::Access access(v,false);return v.resolver;}
DequeMapResolver& SessionMemoryDomain::output_map_resolver(){auto& v=*impl_;Impl::Access access(v,false);return v.resolver;}
OperationPolicy SessionMemoryDomain::operation_policy()const noexcept{return impl_->policy;}
std::shared_ptr<void> SessionMemoryDomain::acquire_provider_lease(){
    PreserveError error;auto self=shared_from_this();auto& v=*impl_;
    {std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||!v.actors.empty()||v.admissions||v.preparing_lease||v.closing||v.reaping)reject(lock,"Provider lease requires an idle open session domain");
        v.preparing_lease=true;
    }
    try{
        if(v.borrowed_backing)(void)v.borrowed_backing->memory();
        auto lease=std::make_shared<Impl::Lease>(std::move(self));
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||!v.preparing_lease||!v.actors.empty())reject(lock,"Provider lease admission changed");
        if(v.provider_leases==std::numeric_limits<std::size_t>::max())reject<std::overflow_error>(lock,"Provider lease counter exhausted");
        ++v.provider_leases;lease->armed=true;v.preparing_lease=false;return lease;
    }catch(...){std::unique_lock lock(v.gate);v.preparing_lease=false;throw;}
}
std::shared_ptr<void> SessionMemoryDomain::acquire_provider_lease_from_operation(){
    PreserveError error;auto& v=*impl_;Impl::Access access(v,true);
    auto lease=std::make_shared<Impl::Lease>(shared_from_this());access.check();
    std::unique_lock lock(v.gate);v.check_locked(*access.actor,lock);
    if(v.provider_leases==std::numeric_limits<std::size_t>::max())reject<std::overflow_error>(lock,"Provider lease counter exhausted");
    ++v.provider_leases;lease->armed=true;return lease;
}
std::shared_ptr<void> SessionMemoryDomain::pin_buffer(std::uintptr_t address,std::uint64_t bytes){
    PreserveError error;auto& v=*impl_;Impl::Access access(v,true);if(!bytes)return {};
    auto self=shared_from_this();Impl::Ticket* ticket=nullptr;
    {
        std::unique_lock lock(v.gate);
        for(auto& t:v.tickets){
            if(t.state!=Impl::State::Live||t.busy||!t.has_region||address<t.region.address)continue;
            const auto offset=address-t.region.address;if(offset>t.region.bytes.size()||bytes>t.region.bytes.size()-offset)continue;
            if(ticket)reject<std::invalid_argument>(lock,"Borrowed SDK buffer mapping is ambiguous");ticket=&t;
        }
        if(!ticket)reject<std::invalid_argument>(lock,"Borrowed SDK buffer must belong to this session domain");
        if(ticket->borrows==std::numeric_limits<std::size_t>::max()||v.buffer_borrows==std::numeric_limits<std::size_t>::max())reject<std::overflow_error>(lock,"Buffer borrow counter exhausted");
        ++ticket->borrows;++v.buffer_borrows;
    }
    try{
        auto lease=std::make_shared<Impl::Lease>(std::move(self));access.check();
        lease->ticket=ticket;lease->armed=true;return lease;
    }catch(...){std::unique_lock lock(v.gate);--ticket->borrows;--v.buffer_borrows;throw;}
}
bool SessionMemoryDomain::owns_control(const IoReference* control)const{
    PreserveError error;auto& v=*impl_;Impl::Access access(v,false);std::unique_lock lock(v.gate);
    for(const auto& t:v.tickets)if(t.kind==Impl::Kind::Control&&t.state==Impl::State::Live&&!t.busy&&t.typed_live&&
        static_cast<const IoReference*>(static_cast<const RecoveredSessionControl*>(t.object))==control)return true;
    return false;
}
bool SessionMemoryDomain::owns_output_queue(const WriteFinishOwner* queue)const{
    PreserveError error;auto& v=*impl_;Impl::Access access(v,false);std::unique_lock lock(v.gate);
    for(const auto& t:v.tickets)if(t.kind==Impl::Kind::Control&&t.state==Impl::State::Live&&!t.busy&&t.typed_live){
        const auto* control=static_cast<const RecoveredSessionControl*>(t.object);
        if(&control->object.output.fields==queue&&control->strong_08.load())return true;
    }
    return false;
}
std::span<const DequeMapBinding> SessionMemoryDomain::output_map_bindings(){
    PreserveError error;auto& v=*impl_;Impl::Access access(v,false);
    if(v.policy!=OperationPolicy::Serialized)throw std::logic_error("Concurrent actors require exact output-map resolver, not a borrowed union catalog");
    return v.maps;
}
SessionPair SessionMemoryDomain::construct_session(SocketObject& pending,ExecutorMove executor){
    auto& v=*impl_;Impl::Access access(v,false);
    {PreserveError error;std::unique_lock lock(v.gate);if(access.actor->constructing)reject(lock,"Session construction reentry is unsupported");access.actor->constructing=true;}
    std::list<Impl::Construction> prepared,retired;Impl::Construction* record=nullptr;
    try{
        {PreserveError error;prepared.emplace_back();access.check();record=&prepared.front();
            std::unique_lock lock(v.gate);v.constructions.splice(v.constructions.end(),prepared);
        }
        ConstructionContext context{*this,executor,*v.lifecycle,record->scratch};
        construct_session_owner_023000(record->pair,pending,context);access.check();
        const auto result=record->pair;
        {PreserveError error;
            {std::unique_lock lock(v.gate);
                const auto i=std::find_if(v.constructions.begin(),v.constructions.end(),[&](const auto& c){return &c==record;});
                retired.splice(retired.end(),v.constructions,i);access.actor->constructing=false;
            }
            retired.clear();
        }
        return result;
    }catch(...){
        PreserveError error;
        {std::unique_lock lock(v.gate);access.actor->constructing=false;
            if(record&&!record->scratch.allocation){
                const auto i=std::find_if(v.constructions.begin(),v.constructions.end(),[&](const auto& c){return &c==record;});
                if(i!=v.constructions.end())retired.splice(retired.end(),v.constructions,i);
            }
        }
        prepared.clear();retired.clear();
        throw;
    }
}
void SessionMemoryDomain::free_sized_04b0f8(std::uintptr_t address,std::uint64_t bytes){
    auto& v=*impl_;Impl::Access access(v,true);if(!address)return;
    Impl::Iterator ticket;std::list<ActorState::ViewBorrow> retired_views;
    {
        PreserveError error;
        std::unique_lock lock(v.gate);
        ticket=std::find_if(v.tickets.begin(),v.tickets.end(),[address](const auto& t){return t.state==Impl::State::Live&&!t.busy&&t.key==address;});
        if(ticket==v.tickets.end()||ticket->state!=Impl::State::Live||ticket->native_free!=bytes)reject<std::invalid_argument>(lock,"Foreign, retired or size-mismatched session allocation");
        if(ticket->kind==Impl::Kind::Control){
            auto* control=static_cast<RecoveredSessionControl*>(ticket->object);
            if(control->strong_08.load()!=0||control->weak_0c.load()!=0)reject(lock,"Session control free requires final strong/weak release");
        }
        v.reserve_retire_locked(*access.actor,*ticket,retired_views,lock);
    }
    {PreserveError error;retired_views.clear();}
    v.retire(access,ticket);
}
[[noreturn]]void SessionMemoryDomain::invalid_parameter_05c974(){auto& v=*impl_;Impl::Access access(v,false);throw std::invalid_argument("Recovered allocation alignment/header check failed");}
bool SessionMemoryDomain::retry_pending_frees(){
    auto& v=*impl_;Impl::Access access(v,true);
    for(;;){
        Impl::Iterator ticket;std::list<ActorState::ViewBorrow> retired;
        {std::unique_lock lock(v.gate);
            ticket=std::find_if(v.tickets.begin(),v.tickets.end(),[](const auto& t){return t.state==Impl::State::Retired&&!t.busy;});
            if(ticket==v.tickets.end())return true;
            v.reserve_retire_locked(*access.actor,*ticket,retired,lock);
        }
        if(!v.retained_live(ticket->raw)){v.poison();return false;}
        try{v.retire(access,ticket);}catch(...){return false;}
    }
}
bool SessionMemoryDomain::close(){
    PreserveError error;auto& v=*impl_;
    {
        std::unique_lock lock(v.gate);
        if(v.phase==SessionMemoryPhase::Closed)return true;
        if(v.phase!=SessionMemoryPhase::Open||!v.actors.empty()||v.admissions||v.preparing_lease||v.closing||v.reaping||v.provider_leases||v.buffer_borrows||v.view_borrows||!v.tickets.empty()||!v.constructions.empty())return false;
        v.closing=true;
    }
    struct Closing {Impl& v;~Closing(){std::unique_lock lock(v.gate);v.closing=false;}} closing{v};
    if(v.borrowed_backing){
        (void)v.borrowed_backing->memory();if(!v.borrowed_backing->jobs_returned())return false;
    }
    if(v.backing->tracking_poisoned()||v.backing->outstanding_allocations()!=0){v.poison();return false;}
    std::shared_ptr<LifecycleEffects> retiring_lifecycle;
    std::shared_ptr<thread_domain::ApplicationMemory> retiring_backing;
    std::optional<worker_handoff::PreparedMemory::Borrow> retiring_borrow;
    {
        std::unique_lock lock(v.gate);
        if(v.phase!=SessionMemoryPhase::Open||!v.actors.empty()||v.admissions)reject(lock,"Session close admission changed");
        v.phase=SessionMemoryPhase::Retiring;v.memory.regions={};v.regions.clear();
        retiring_lifecycle=std::move(v.lifecycle);retiring_backing=std::move(v.owned_backing);
        if(v.borrowed_backing){retiring_borrow.emplace(std::move(*v.borrowed_backing));v.borrowed_backing.reset();}
        v.backing=nullptr;
    }
    try{
        retiring_lifecycle.reset();retiring_backing.reset();
        if(retiring_borrow&&!retiring_borrow->retire()){v.poison();return false;}
    }catch(...){v.poison();throw;}
    {std::unique_lock lock(v.gate);if(v.phase!=SessionMemoryPhase::Retiring){v.phase=SessionMemoryPhase::Poisoned;return false;}v.phase=SessionMemoryPhase::Closed;}
    return true;
}
SessionMemorySnapshot SessionMemoryDomain::snapshot()const{
    PreserveError error;const auto& v=*impl_;std::unique_lock lock(v.gate);SessionMemorySnapshot out;
    out.phase=v.phase;out.allocations=v.tickets.size();out.partial_constructions=v.constructions.size();
    out.successful_allocations=v.allocations;out.successful_frees=v.frees;out.controls_destroyed=v.controls_destroyed;
    out.provider_leases=v.provider_leases;out.buffer_borrows=v.buffer_borrows;
    for(const auto& t:v.tickets){
        if(t.state==Impl::State::Reserved){++out.reserved;continue;}
        if(t.state==Impl::State::Retired){++out.pending_frees;continue;}
        switch(t.kind){case Impl::Kind::Control:++out.controls;break;case Impl::Kind::Proxy:++out.proxies;break;case Impl::Kind::Block:++out.blocks;break;case Impl::Kind::Map:++out.maps;break;case Impl::Kind::Bytes:++out.buffers;break;}
    }
    return out;
}
const thread_domain::DomainStorage* SessionMemoryDomain::memory_domain_identity()const noexcept{return impl_->source_domain;}
} // namespace anadius::socket_recovered
