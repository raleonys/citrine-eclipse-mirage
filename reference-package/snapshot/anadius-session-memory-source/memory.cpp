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
}
struct SessionMemoryDomain::Impl {
    enum class Kind {Control,Proxy,Block,Map,Bytes};
    enum class State {Reserved,Live,Retired};
    struct Ticket {
        Kind kind;State state=State::Reserved;
        void* raw=nullptr;void* object=nullptr;std::uintptr_t key=0;
        std::size_t extent=0,elements=0,borrows=0;std::uint64_t native_free=0;
        bool typed_live=false,has_region=false;ReceiveBufferRegion region{};
        explicit Ticket(Kind k):kind(k){}
    };
    using Iterator=std::list<Ticket>::iterator;
    struct Lease {
        std::shared_ptr<SessionMemoryDomain> root;Ticket* ticket=nullptr;bool armed=false;
        explicit Lease(std::shared_ptr<SessionMemoryDomain> r):root(std::move(r)){}
        ~Lease()noexcept{
            if(!armed)return;
            PreserveError error;auto& v=*root->impl_;std::lock_guard lock(v.gate);
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
    struct Construction {ConstructionScratch scratch;SessionPair pair;};
    SessionMemoryDomain& owner;
    std::shared_ptr<thread_domain::ApplicationMemory> owned_backing;
    std::optional<worker_handoff::PreparedMemory::Borrow> borrowed_backing;
    thread_domain::ApplicationMemory* backing;
    std::shared_ptr<LifecycleEffects> lifecycle;
    const thread_domain::DomainStorage* source_domain;
    mutable std::recursive_mutex gate;
    SessionMemoryPhase phase=SessionMemoryPhase::Open;
    bool active=false,mutating=false,constructing=false;DWORD thread=0,slot=FLS_OUT_OF_INDEXES;void* fiber=nullptr;void* ptd=nullptr;
    std::list<Ticket> tickets;std::vector<ReceiveBufferRegion> regions;
    std::vector<DequeMapBinding> maps;
    std::list<Construction> constructions;
    ReceiveBufferMemory memory{};Buffer buffer{*this};
    ReceiveBufferContext receive{memory,buffer};
    SessionDisposalContext disposal;
    std::size_t allocations=0,frees=0,controls_destroyed=0,provider_leases=0,buffer_borrows=0;
    Impl(SessionMemoryDomain& o,std::shared_ptr<thread_domain::ApplicationMemory> b,std::shared_ptr<LifecycleEffects> l)
        :owner(o),owned_backing(std::move(b)),backing(owned_backing.get()),lifecycle(std::move(l)),source_domain(backing->domain_identity()),disposal{memory,o,*lifecycle}{}
    Impl(SessionMemoryDomain& o,worker_handoff::PreparedMemory::Borrow&& b,std::shared_ptr<LifecycleEffects> l)
        :owner(o),borrowed_backing(std::move(b)),backing(&borrowed_backing->memory()),lifecycle(std::move(l)),source_domain(backing->domain_identity()),disposal{memory,o,*lifecycle}{}
    struct Access {
        Impl& v;std::unique_lock<std::recursive_mutex> lock;bool mutation;
        Access(Impl& value,bool mutate):v(value),lock(v.gate),mutation(mutate){
            if(!v.active||v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber())throw std::logic_error("Session memory requires its whole-call thread/fiber Operation");
            if(v.phase!=SessionMemoryPhase::Open)throw std::logic_error("Session memory is closed or has an unknown outcome");
            v.require_ptd();
            if(v.mutating)throw std::logic_error("Backing allocation/free effect reentry is unsupported");
            if(mutation)v.mutating=true;
        }
        ~Access(){if(mutation)v.mutating=false;}
    };
    bool ptd_matches()const noexcept{
        PreserveError error;
        return backing&&source_domain->native_fls_index()==slot&&::FlsGetValue(slot)==ptd;
    }
    void require_ptd(){
        if(phase!=SessionMemoryPhase::Open)throw std::logic_error("Session memory admission was poisoned or closed during a backing effect");
        if(borrowed_backing)(void)borrowed_backing->memory();
        if(!ptd_matches()){phase=SessionMemoryPhase::Poisoned;throw std::logic_error("Backing-domain PTD changed within Session memory Operation");}
    }
    void refresh()noexcept{
        regions.clear();maps.clear();
        for(const auto& t:tickets)if(t.state==State::Live){
            if(t.has_region)regions.push_back(t.region);
            if(t.kind==Kind::Map&&t.typed_live)maps.push_back({identity(t.object),{static_cast<WriteFinishString**>(t.object),t.elements}});
        }
        memory.regions=regions;
    }
    Iterator allocate(Kind kind,std::size_t extent,std::uint64_t native_free){
        PreserveError error;
        if(backing->tracking_poisoned()){phase=SessionMemoryPhase::Poisoned;throw std::logic_error("Backing allocation tracking is unknown");}
        const auto before=backing->outstanding_allocations();
        auto ticket=tickets.emplace(tickets.end(),kind);
        try{regions.reserve(tickets.size());memory.regions=regions;maps.reserve(tickets.size());}catch(...){tickets.erase(ticket);throw;}
        ticket->extent=extent;ticket->native_free=native_free;error.restore();
        try{
            auto* raw=backing->allocate(extent);error.capture();
            ticket->raw=raw;ticket->key=identity(raw);++allocations;
            require_ptd();
            return ticket;
        }catch(...){
            error.capture();bool clean=false;
            try{clean=ptd_matches()&&!backing->tracking_poisoned()&&backing->outstanding_allocations()==before;}catch(...){}
            if(clean)tickets.erase(ticket);else phase=SessionMemoryPhase::Poisoned;
            throw;
        }
    }
    bool retained_live(void* raw)noexcept{
        try{return backing->allocation_state(raw)==thread_domain::ApplicationMemory::AllocationState::Live;}catch(...){return false;}
    }
    void destroy(Ticket& t)noexcept{
        if(!t.typed_live)return;
        switch(t.kind){
            case Kind::Control:std::destroy_at(static_cast<RecoveredSessionControl*>(t.object));++controls_destroyed;break;
            case Kind::Proxy:std::destroy_at(static_cast<DequeProxy*>(t.object));break;
            case Kind::Block:std::destroy_at(static_cast<WriteFinishString*>(t.object));break;
            case Kind::Map:std::destroy_n(static_cast<WriteFinishString**>(t.object),t.elements);break;
            case Kind::Bytes:break;
        }
        t.typed_live=false;
    }
    void retire(Iterator ticket){
        PreserveError error;
        destroy(*ticket);ticket->has_region=false;ticket->state=State::Retired;refresh();error.restore();
        try{require_ptd();backing->deallocate(ticket->raw,ticket->extent);error.capture();require_ptd();}
        catch(...){error.capture();if(!retained_live(ticket->raw))phase=SessionMemoryPhase::Poisoned;throw;}
        ++frees;tickets.erase(ticket);refresh();
    }
    void cleanup_constructor(Iterator ticket)noexcept{
        // Placement constructor already unwound its constructed C++ members.
        // Keep failed raw frees in the ledger; never hide an unknown outcome.
        try{retire(ticket);}catch(...){}
    }
};
static_assert(alignof(RecoveredSessionControl)<=MEMORY_ALLOCATION_ALIGNMENT);
static_assert(alignof(DequeProxy)<=MEMORY_ALLOCATION_ALIGNMENT&&sizeof(DequeProxy)==16);
static_assert(alignof(WriteFinishString)<=MEMORY_ALLOCATION_ALIGNMENT&&sizeof(WriteFinishString)==32);
static_assert(std::is_nothrow_copy_constructible_v<ReceiveBufferRegion>);
static_assert(std::is_nothrow_copy_constructible_v<DequeMapBinding>);
static_assert(std::is_nothrow_destructible_v<RecoveredSessionControl>);

SessionMemoryDomain::SessionMemoryDomain(std::shared_ptr<thread_domain::ApplicationMemory>&& backing,std::shared_ptr<LifecycleEffects> lifecycle){
    if(!backing||backing.use_count()!=1||!lifecycle)throw std::invalid_argument("A sole-owned dedicated ApplicationMemory and owning lifecycle provider are required");
    if(backing->tracking_poisoned()||backing->outstanding_allocations()!=0)throw std::invalid_argument("Backing memory must be clean and empty");
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(lifecycle));
}
SessionMemoryDomain::SessionMemoryDomain(worker_handoff::PreparedMemory::Borrow&& backing,std::shared_ptr<LifecycleEffects> lifecycle){
    auto& provider=backing.memory();
    if(!lifecycle||provider.tracking_poisoned()||provider.outstanding_allocations())throw std::invalid_argument("A clean dedicated worker borrow and lifecycle provider are required");
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(lifecycle));
}
SessionMemoryDomain::~SessionMemoryDomain()noexcept{
    const auto& v=*impl_;
    if(v.active||v.mutating||v.constructing||v.provider_leases||v.buffer_borrows||!v.tickets.empty()||!v.constructions.empty()||v.phase!=SessionMemoryPhase::Closed)std::terminate();
}
SessionMemoryDomain::Operation::Operation(Impl& v):state_(v),thread_(::GetCurrentThreadId()),fiber_(::GetCurrentFiber()),lock_(v.gate){
    PreserveError error;
    if(v.phase!=SessionMemoryPhase::Open||v.active||v.mutating)throw std::logic_error("Session memory Operation is closed, poisoned or reentrant");
    if(v.borrowed_backing)(void)v.borrowed_backing->memory();
    const auto index=v.source_domain->native_fls_index();
    auto* const ptd=::FlsGetValue(index);
    if(index==FLS_OUT_OF_INDEXES||!ptd||ptd==crt_ptd_recovered::construction_sentinel())throw std::logic_error("Session memory requires a ready PTD in its backing domain on this fiber");
    v.active=true;v.thread=thread_;v.fiber=fiber_;v.slot=index;v.ptd=ptd;
}
SessionMemoryDomain::Operation::~Operation()noexcept{
    if(::GetCurrentThreadId()!=thread_||::GetCurrentFiber()!=fiber_||!state_.active||state_.mutating||state_.constructing)std::terminate();
    if(!state_.ptd_matches())state_.phase=SessionMemoryPhase::Poisoned;
    state_.active=false;state_.thread=0;state_.fiber=nullptr;state_.ptd=nullptr;state_.slot=FLS_OUT_OF_INDEXES;
}
SessionMemoryDomain::Operation SessionMemoryDomain::enter(){return Operation(*impl_);}
SessionControl* SessionMemoryDomain::allocate_control_04b0bc(std::uint64_t request){
    auto& v=*impl_;Impl::Access access(v,true);
    if(request!=0x1c8)throw std::invalid_argument("Session control native request must be1c8");
    auto ticket=v.allocate(Impl::Kind::Control,sizeof(RecoveredSessionControl),0x1c8);
    RecoveredSessionControl* control=nullptr;
    try{control=::new(ticket->raw) RecoveredSessionControl(v.disposal);}catch(...){v.cleanup_constructor(ticket);throw;}
    ticket->object=control;ticket->key=identity(static_cast<SessionControl*>(control));ticket->typed_live=true;
    ticket->region={identity(control->object.input.storage_00.data()),control->object.input.storage_00};
    ticket->has_region=true;ticket->state=Impl::State::Live;v.refresh();return control;
}
DequeProxy* SessionMemoryDomain::allocate_proxy_04b0bc(std::uint64_t request){
    auto& v=*impl_;Impl::Access access(v,true);
    if(request!=16)throw std::invalid_argument("Deque proxy native request must be16");
    auto ticket=v.allocate(Impl::Kind::Proxy,sizeof(DequeProxy),16);
    auto* proxy=::new(ticket->raw) DequeProxy{};
    ticket->object=proxy;ticket->typed_live=true;ticket->state=Impl::State::Live;v.refresh();return proxy;
}
WriteFinishString* SessionMemoryDomain::allocate_output_block(){
    auto& v=*impl_;Impl::Access access(v,true);auto ticket=v.allocate(Impl::Kind::Block,sizeof(WriteFinishString),32);
    auto* block=::new(ticket->raw) WriteFinishString{};ticket->object=block;ticket->typed_live=true;
    ticket->region={identity(block->storage_00.data()),block->storage_00};ticket->has_region=true;ticket->state=Impl::State::Live;v.refresh();return block;
}
std::span<WriteFinishString*> SessionMemoryDomain::allocate_output_map(std::uint64_t slots){
    auto& v=*impl_;Impl::Access access(v,true);
    if(!slots||(slots&(slots-1))!=0||slots>(maximum-39)/sizeof(WriteFinishString*))throw std::length_error("Output map requires a bounded positive power-of-two slot count");
    const auto count=static_cast<std::size_t>(slots),bytes=count*sizeof(WriteFinishString*);
    const auto extent=bytes+(bytes>=4096?39:0);
    auto ticket=v.allocate(Impl::Kind::Map,extent,extent);auto data=identity(ticket->raw);
    if(bytes>=4096){
        data=(data+39)&~std::uintptr_t{31};const auto raw=identity(ticket->raw);
        std::memcpy(reinterpret_cast<void*>(data-8),&raw,8);
    }
    auto* map=::new(reinterpret_cast<void*>(data)) WriteFinishString*[count]{};
    ticket->object=map;ticket->elements=count;ticket->typed_live=true;
    ticket->region={identity(ticket->raw),{static_cast<std::uint8_t*>(ticket->raw),extent}};
    ticket->has_region=true;ticket->state=Impl::State::Live;v.refresh();return {map,count};
}
std::uintptr_t SessionMemoryDomain::Impl::Buffer::allocate_04b0bc(std::uint64_t bytes){
    Access access(state,true);if(bytes>maximum)throw std::bad_array_new_length();
    auto ticket=state.allocate(Kind::Bytes,static_cast<std::size_t>(bytes),bytes);
    auto* raw=::new(ticket->raw) std::uint8_t[static_cast<std::size_t>(bytes)];
    ticket->object=raw;ticket->region={identity(raw),{raw,static_cast<std::size_t>(bytes)}};
    ticket->has_region=true;ticket->state=State::Live;state.refresh();return identity(raw);
}
void SessionMemoryDomain::Impl::Buffer::copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t bytes){
    Access access(state,true);if(!bytes)return;
    auto destination=state.memory.view(to,bytes),source=state.memory.view(from,bytes);
    std::memmove(destination.data(),source.data(),destination.size());
}
void SessionMemoryDomain::Impl::Buffer::fill_bytes(std::uintptr_t address,std::uint8_t value,std::uint64_t bytes){
    Access access(state,true);if(!bytes)return;auto destination=state.memory.view(address,bytes);std::fill(destination.begin(),destination.end(),value);
}
[[noreturn]]void SessionMemoryDomain::Impl::Buffer::length_error_00ce80(){Access access(state,false);throw std::length_error("Recovered receive string maximum exceeded");}
[[noreturn]]void SessionMemoryDomain::Impl::Buffer::allocation_overflow_00cd30(){Access access(state,false);throw std::bad_array_new_length();}
ReceiveBufferContext& SessionMemoryDomain::buffer_context(){auto& v=*impl_;Impl::Access access(v,false);return v.receive;}
std::shared_ptr<void> SessionMemoryDomain::acquire_provider_lease(){
    PreserveError error;auto self=shared_from_this();auto& v=*impl_;std::lock_guard lock(v.gate);
    if(v.phase!=SessionMemoryPhase::Open||v.active||v.mutating)throw std::logic_error("Provider lease requires an idle open session domain");
    if(v.borrowed_backing)(void)v.borrowed_backing->memory();
    if(v.provider_leases==std::numeric_limits<std::size_t>::max())throw std::overflow_error("Provider lease counter exhausted");
    v.mutating=true;
    try{auto lease=std::make_shared<Impl::Lease>(std::move(self));++v.provider_leases;lease->armed=true;v.mutating=false;return lease;}
    catch(...){v.mutating=false;throw;}
}
std::shared_ptr<void> SessionMemoryDomain::acquire_provider_lease_from_operation(){
    PreserveError error;auto& v=*impl_;Impl::Access access(v,true);
    if(v.provider_leases==std::numeric_limits<std::size_t>::max())throw std::overflow_error("Provider lease counter exhausted");
    auto lease=std::make_shared<Impl::Lease>(shared_from_this());++v.provider_leases;lease->armed=true;return lease;
}
std::shared_ptr<void> SessionMemoryDomain::pin_buffer(std::uintptr_t address,std::uint64_t bytes){
    auto& v=*impl_;Impl::Access access(v,true);if(!bytes)return {};
    Impl::Ticket* found=nullptr;
    for(auto& t:v.tickets){
        if(t.state!=Impl::State::Live||!t.has_region||address<t.region.address)continue;
        const auto offset=address-t.region.address;
        if(offset>t.region.bytes.size()||bytes>t.region.bytes.size()-offset)continue;
        if(found)throw std::invalid_argument("Borrowed SDK buffer mapping is ambiguous");found=&t;
    }
    if(!found)throw std::invalid_argument("Borrowed SDK buffer must belong to this session domain");
    if(found->borrows==std::numeric_limits<std::size_t>::max()||v.buffer_borrows==std::numeric_limits<std::size_t>::max())throw std::overflow_error("Buffer borrow counter exhausted");
    auto lease=std::make_shared<Impl::Lease>(shared_from_this());lease->ticket=found;
    ++found->borrows;++v.buffer_borrows;lease->armed=true;return lease;
}
bool SessionMemoryDomain::owns_control(const IoReference* control)const{
    auto& v=*impl_;Impl::Access access(v,false);
    for(const auto& t:v.tickets)if(t.kind==Impl::Kind::Control&&t.state==Impl::State::Live&&t.typed_live&&
        static_cast<const IoReference*>(static_cast<const RecoveredSessionControl*>(t.object))==control)return true;
    return false;
}
bool SessionMemoryDomain::owns_output_queue(const WriteFinishOwner* queue)const{
    auto& v=*impl_;Impl::Access access(v,false);
    for(const auto& t:v.tickets)if(t.kind==Impl::Kind::Control&&t.state==Impl::State::Live&&t.typed_live){
        const auto* control=static_cast<const RecoveredSessionControl*>(t.object);
        if(&control->object.output.fields==queue&&control->strong_08.load())return true;
    }
    return false;
}
std::span<const DequeMapBinding> SessionMemoryDomain::output_map_bindings(){
    auto& v=*impl_;Impl::Access access(v,false);return v.maps;
}
SessionPair SessionMemoryDomain::construct_session(SocketObject& pending,ExecutorMove executor){
    auto& v=*impl_;Impl::Access access(v,false);
    if(v.constructing)throw std::logic_error("Session construction reentry is unsupported");
    auto record=v.constructions.emplace(v.constructions.end());v.constructing=true;
    try{
        ConstructionContext context{*this,executor,*v.lifecycle,record->scratch};
        construct_session_owner_023000(record->pair,pending,context);
        const auto result=record->pair;v.constructions.erase(record);v.constructing=false;return result;
    }catch(...){
        v.constructing=false;
        // No source move/provider is called before a control is returned.
        // Otherwise keep the actual scratch sidecar alive, not a stack address.
        if(!record->scratch.allocation)v.constructions.erase(record);
        throw;
    }
}
void SessionMemoryDomain::free_sized_04b0f8(std::uintptr_t address,std::uint64_t bytes){
    auto& v=*impl_;Impl::Access access(v,true);if(!address)return;
    const auto ticket=std::find_if(v.tickets.begin(),v.tickets.end(),[address](const auto& t){return t.key==address;});
    if(ticket==v.tickets.end()||ticket->state!=Impl::State::Live||ticket->native_free!=bytes)throw std::invalid_argument("Foreign, retired or size-mismatched session allocation");
    if(ticket->borrows)throw std::logic_error("A pending SDK borrower still owns this allocation range");
    if(ticket->kind==Impl::Kind::Control){
        auto* control=static_cast<RecoveredSessionControl*>(ticket->object);
        if(control->strong_08.load()!=0||control->weak_0c.load()!=0)throw std::logic_error("Session control free requires final strong/weak release");
    }
    v.retire(ticket);
}
[[noreturn]]void SessionMemoryDomain::invalid_parameter_05c974(){auto& v=*impl_;Impl::Access access(v,false);throw std::invalid_argument("Recovered allocation alignment/header check failed");}
bool SessionMemoryDomain::retry_pending_frees(){
    auto& v=*impl_;Impl::Access access(v,true);
    for(auto current=v.tickets.begin();current!=v.tickets.end();){
        auto ticket=current++;
        if(ticket->state!=Impl::State::Retired)continue;
        if(!v.retained_live(ticket->raw)){v.phase=SessionMemoryPhase::Poisoned;return false;}
        try{v.retire(ticket);}catch(...){return false;}
    }
    return true;
}
bool SessionMemoryDomain::close(){
    PreserveError error;auto& v=*impl_;
    std::shared_ptr<LifecycleEffects> retiring_lifecycle;
    std::shared_ptr<thread_domain::ApplicationMemory> retiring_backing;
    std::optional<worker_handoff::PreparedMemory::Borrow> retiring_borrow;
    {
        std::lock_guard lock(v.gate);
        if(v.phase==SessionMemoryPhase::Closed)return true;
        if(v.phase!=SessionMemoryPhase::Open||v.active||v.mutating||v.constructing||v.provider_leases||v.buffer_borrows||!v.tickets.empty()||!v.constructions.empty())return false;
        if(v.borrowed_backing)(void)v.borrowed_backing->memory();
        if(v.backing->tracking_poisoned()||v.backing->outstanding_allocations()!=0){v.phase=SessionMemoryPhase::Poisoned;return false;}
        // Close admission BEFORE invoking owned provider destructors/deleters.
        // Reentrant enter is rejected; nested close reports still in progress.
        v.phase=SessionMemoryPhase::Retiring;v.memory.regions={};v.regions.clear();
        retiring_lifecycle=std::move(v.lifecycle);retiring_backing=std::move(v.owned_backing);
        if(v.borrowed_backing){retiring_borrow.emplace(std::move(*v.borrowed_backing));v.borrowed_backing.reset();}
        v.backing=nullptr;
    }
    retiring_lifecycle.reset();retiring_backing.reset();
    if(retiring_borrow&&!retiring_borrow->retire()){
        std::lock_guard lock(v.gate);v.phase=SessionMemoryPhase::Poisoned;return false;
    }
    {std::lock_guard lock(v.gate);v.phase=SessionMemoryPhase::Closed;}
    return true;
}
SessionMemorySnapshot SessionMemoryDomain::snapshot()const{
    PreserveError error;const auto& v=*impl_;std::lock_guard lock(v.gate);SessionMemorySnapshot out;
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
