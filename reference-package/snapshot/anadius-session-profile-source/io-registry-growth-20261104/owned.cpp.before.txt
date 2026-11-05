#include "owned.hpp"
#include <exception>
#include <limits>
#include <mutex>
#include <type_traits>
#include <variant>
#include <vector>

namespace anadius::socket_recovered {
struct OwnedAcceptMemory::Impl {
    struct Task final:AcceptTask {
        Impl& owner;Task(OperationStorage& s,Impl& o):AcceptTask(s),owner(o){}
        void call_slot_00(std::uint8_t flag)override;
    };
    struct Row {
        OperationStorage* storage=nullptr;std::uint64_t generation=0;bool pending=false,consumed=false,cancel=false;
        std::uint64_t submission=0;AcceptSubmission submission_state=AcceptSubmission::NotSubmitted;Socket submitted_listener=invalid_socket;
        std::shared_ptr<void> pin;
        std::variant<std::monostate,AsyncAcceptOperation,Task> object;
    };
    struct Pinned final:AsyncAcceptApi {
        Impl& owner;bool prearmed;explicit Pinned(Impl& o,bool p=false):owner(o),prearmed(p){}
        AcceptExResult accept(Socket listener,Socket accepted,std::span<std::byte> bytes,OVERLAPPED& ov)override{
            Access access(owner);const auto slot=owner.find(&ov);auto& r=owner.rows[slot];
            const auto fixed=owner.require_region(r);
            if(bytes.data()!=fixed.data()||bytes.size()!=fixed.size())throw UnsupportedInput("SDK buffer is not the authoritative owned accept address region");
            if(prearmed){
                const auto& op=std::get<AsyncAcceptOperation>(r.object);const auto h=owner.base.handle_for(*r.storage);
                const auto& b=owner.base.dispatcher_bindings()[h.slot];
                if(!r.pending||!r.pin||r.consumed||b.operation!=&op.operation||b.overlapped_identity!=&op.operation.overlapped)
                    throw std::logic_error("Retry SDK requires the same prearmed owned packet and buffer loan");
            }else owner.arm(r);
            const auto& projected=std::get<AsyncAcceptOperation>(r.object);
            if(listener!=projected.listener||accepted!=projected.accepted)throw std::logic_error("Accept SDK operands differ from the live owned operation");
            const auto sequence=r.submission;r.submission_state=AcceptSubmission::Issuing;r.submitted_listener=listener;
            try{
                const auto result=owner.sdk.accept(listener,accepted,bytes,ov);
                if(!r.pending||r.submission!=sequence||r.consumed)throw std::logic_error("Accept submission changed during SDK call");
                r.submission_state=(result.result||result.last_error==997)?AcceptSubmission::Kernel:AcceptSubmission::Software;return result;
            }catch(...){r.submission_state=AcceptSubmission::Unknown;owner.poisoned=true;throw;}
        }
        AcceptAddresses addresses(std::span<std::byte> b)override{return owner.effect([&]{owner.require_addresses(b);return owner.sdk.addresses(b);});}
        std::int32_t update_context(Socket s,Socket& listener)override{return owner.effect([&]{return owner.sdk.update_context(s,listener);});}
    };
    const std::shared_ptr<SessionMemoryDomain> memory;OwnedIoMemory& base;AcceptLower& lower;AsyncAcceptApi& sdk;
    std::shared_ptr<void> lease;AsyncAcceptContext* bound=nullptr;std::vector<Row> rows;
    mutable std::recursive_mutex gate;bool closed=false,retiring=false,poisoned=false,submitting=false;std::size_t active=0;
    Pinned pinned{*this};Pinned prearmed{*this,true};
    Impl(std::shared_ptr<SessionMemoryDomain> m,OwnedIoMemory& b,std::size_t count,AcceptLower& l,AsyncAcceptApi& s)
        :memory(std::move(m)),base(b),lower(l),sdk(s),rows(count){lease=memory->acquire_provider_lease_from_operation();}
    struct Access {
        Impl& v;std::unique_lock<std::recursive_mutex> lock;
        explicit Access(Impl& v):v(v),lock(v.gate,std::defer_lock){(void)v.memory->buffer_context();lock.lock();if(v.closed||v.retiring||v.poisoned)throw std::logic_error("Accept provider is closed or poisoned");}
    };
    struct Call {Impl& v;explicit Call(Impl& v):v(v){++v.active;}~Call(){--v.active;}};
    template<class F>auto effect(F&& fn)->std::invoke_result_t<F>{Access access(*this);Call call(*this);try{return std::forward<F>(fn)();}catch(...){poisoned=true;throw;}}
    AsyncAcceptContext& context(){if(!bound)throw std::logic_error("Accept context is not bound");return *bound;}
    std::size_t index(OperationStorage& storage){const auto h=base.handle_for(storage);if(h.slot>=rows.size())throw std::length_error("Accept row capacity does not cover base IO slot");return h.slot;}
    std::size_t find(const OVERLAPPED* ov)const{
        for(std::size_t i=0;i<rows.size();++i)if(auto* op=std::get_if<AsyncAcceptOperation>(&rows[i].object);op&&&op->operation.overlapped==ov)return i;
        throw std::invalid_argument("No live owned accept OVERLAPPED");
    }
    Row& get(AcceptHandle h){if(h.slot>=rows.size()||rows[h.slot].generation!=h.generation||!std::holds_alternative<AsyncAcceptOperation>(rows[h.slot].object))throw std::invalid_argument("Stale accept generation");return rows[h.slot];}
    std::span<std::byte> require_region(Row& r){
        (void)base.handle_for(*r.storage);const auto fixed=r.storage->bytes.subspan(0x68,288);
        const auto& op=std::get<AsyncAcceptOperation>(r.object);
        if(op.address_buffer.data()!=fixed.data()||op.address_buffer.size()!=fixed.size())throw UnsupportedInput("Accept projection does not name its own fixed address region");
        return fixed;
    }
    void require_addresses(std::span<std::byte> bytes){
        for(const auto& r:rows)if(r.storage&&std::holds_alternative<AsyncAcceptOperation>(r.object)){
            (void)base.handle_for(*r.storage);const auto fixed=r.storage->bytes.subspan(0x68,288);
            if(bytes.data()==fixed.data()&&bytes.size()==fixed.size())return;
        }
        throw UnsupportedInput("Endpoint decoder requires an authoritative live accept region");
    }
    void retire(Row& r){
        if(r.pending||r.pin)throw std::logic_error("Accept kernel borrower has not ended");
        if(auto* old=std::get_if<AsyncAcceptOperation>(&r.object))for(auto& binding:base.dispatcher_bindings())if(binding.operation==&old->operation)binding={};
        r.object.emplace<std::monostate>();r.storage=nullptr;r.consumed=false;
        r.submission_state=AcceptSubmission::NotSubmitted;r.submitted_listener=invalid_socket;
    }
    void arm(Row& r){
        if(r.pending||r.pin)throw std::logic_error("Accept already has an SDK borrower");
        if(r.submission==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Accept submission sequence exhausted");
        (void)require_region(r); // also applies to the mandatory29660 retry lower
        auto& op=std::get<AsyncAcceptOperation>(r.object);const auto h=base.handle_for(*r.storage);auto& b=base.dispatcher_bindings()[h.slot];
        if(b.operation!=&op.operation||b.overlapped_identity!=&op.operation.overlapped)throw std::logic_error("Accept packet binding was replaced before SDK submission");
        r.pin=memory->pin_buffer(r.storage->identity,r.storage->bytes.size());r.pending=true;r.consumed=false;
        ++r.submission;r.submission_state=AcceptSubmission::NotSubmitted;r.submitted_listener=invalid_socket;
    }
};
void OwnedAcceptMemory::Impl::Task::call_slot_00(std::uint8_t flag){
    owner.effect([&]{
        const auto local=handler;auto& context=owner.context();auto& allocation=storage;
        // Selected host equivalent of required22520: no body-stage claim.
        auto* tls=static_cast<WorkerTlsContext*>(context.worker.tls_get(context.environment.storage.tls_index));bool cached=false;
        if(tls&&tls->state)for(unsigned i=4;i<6;++i)if(!tls->state->cache_00[i]){
            if(allocation.bytes.size()<=0x28)throw UnsupportedInput("Owned accept task marker is out of bounds");
            allocation.bytes[0]=allocation.bytes[0x28];tls->state->cache_00[i]=allocation.identity;cached=true;break;
        }
        if(!cached)context.memory.free_04b788(allocation);
        if(flag){auto copy=local;context.effects.invoke(copy);} // no old task/allocation access
    });
}
OwnedAcceptMemory::OwnedAcceptMemory(std::shared_ptr<SessionMemoryDomain> memory,OwnedIoMemory& base,std::size_t capacity,AcceptLower& lower,AsyncAcceptApi& sdk){
    if(!memory||!capacity||capacity>65536)throw std::invalid_argument("Accept provider needs bounded row capacity and parent");
    (void)memory->buffer_context();if(capacity<base.storage_bindings().size())throw std::invalid_argument("Accept rows must cover the fixed base IO binding table");
    impl_=std::make_unique<Impl>(std::move(memory),base,capacity,lower,sdk);
}
OwnedAcceptMemory::~OwnedAcceptMemory()noexcept{if(!impl_->closed||impl_->active)std::terminate();}
void OwnedAcceptMemory::bind_context(AsyncAcceptContext& context){auto& v=*impl_;Impl::Access access(v);if(v.bound||&context.effects!=this||&context.memory!=this||&context.api!=&v.pinned)throw std::logic_error("Accept context binding must be unique and coherent");v.bound=&context;}
AsyncAcceptApi& OwnedAcceptMemory::pinned_api(){return impl_->pinned;}
AsyncAcceptApi& OwnedAcceptMemory::prearmed_api(){return impl_->prearmed;}
OperationStorage& OwnedAcceptMemory::allocate(std::uint64_t bytes){auto& v=*impl_;Impl::Access access(v);return v.base.allocate_04b0bc(bytes);}
void OwnedAcceptMemory::release(OperationStorage& storage){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.rows[v.index(storage)];if(v.submitting)throw std::logic_error("Cannot free accept storage during source initialization");
    v.retire(r);try{v.base.free_04b788(storage);}catch(...){v.poisoned=true;throw;}
}
PostedOperation& OwnedAcceptMemory::project_posted(OperationStorage& storage){auto& v=*impl_;Impl::Access access(v);auto& r=v.rows[v.index(storage)];v.retire(r);return v.base.project_posted(storage);}
void OwnedAcceptMemory::retire_for_projection(OperationStorage& storage){
    auto& v=*impl_;Impl::Access access(v);if(v.submitting)throw std::logic_error("Cannot retire accept metadata during source initialization");
    v.retire(v.rows[v.index(storage)]);
}
AsyncAcceptOperation& OwnedAcceptMemory::project(OperationStorage& storage){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage);auto& r=v.rows[i];
    if(storage.bytes.size()<0x1d9)throw UnsupportedInput("Accept allocation lacks native payload and marker");
    if(r.generation==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Accept generation exhausted");
    if(r.pending||r.pin)throw std::logic_error("Accept kernel borrower has not ended");
    // Reserve the base sidecar through its normal checked projection transition:
    // this rejects a still-borrowed receive/send before replacing its packet.
    // The inert Posted slot is only a host bookkeeping reservation; SDK and
    // dispatcher always use the distinct accept operation installed below.
    (void)v.base.project_posted(storage);v.retire(r);
    r.storage=&storage;++r.generation;auto& op=r.object.emplace<AsyncAcceptOperation>(storage);op.address_buffer=storage.bytes.subspan(0x68,288);
    auto& metadata=v.base.dispatcher_bindings()[i];metadata={&op.operation,{},0,&op.operation.overlapped};op.packet=&metadata;return op;
}
AcceptTask& OwnedAcceptMemory::project_task(OperationStorage& storage){auto& v=*impl_;Impl::Access access(v);auto& r=v.rows[v.index(storage)];v.retire(r);r.storage=&storage;return r.object.emplace<Impl::Task>(storage,v);}
void OwnedAcceptMemory::start(AsyncAcceptService& service,AcceptorState& listener,AcceptPending& pending,AcceptEndpoint* output,const std::uintptr_t& handler,std::uintptr_t allocator){
    auto& v=*impl_;Impl::Access access(v);if(v.submitting)throw std::logic_error("Reentrant accept initialization");Impl::Call call(v);v.submitting=true;
    try{start_async_accept_021b00(service,listener,pending,output,handler,allocator,v.context());v.submitting=false;}
    catch(...){v.submitting=false;v.poisoned=true;throw;}
}
AcceptHandle OwnedAcceptMemory::handle_for(const OVERLAPPED* ov)const{auto& v=*impl_;Impl::Access access(v);const auto i=v.find(ov);return {i,v.rows[i].generation};}
AcceptLoan OwnedAcceptMemory::loan(AcceptHandle h)const{
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);auto& op=std::get<AsyncAcceptOperation>(r.object);
    return {h,r.submission,r.submission_state,r.submitted_listener,&op.operation.overlapped,&op.operation,r.pending,r.consumed};
}
std::vector<AcceptLoan> OwnedAcceptMemory::pending_loans()const{
    auto& v=*impl_;Impl::Access access(v);std::vector<AcceptLoan> out;out.reserve(v.rows.size());
    for(std::size_t i=0;i<v.rows.size();++i){const auto& r=v.rows[i];if(!r.pending)continue;
        const auto& op=std::get<AsyncAcceptOperation>(r.object);
        out.push_back({{i,r.generation},r.submission,r.submission_state,r.submitted_listener,const_cast<OVERLAPPED*>(&op.operation.overlapped),const_cast<CancelOperation*>(&op.operation),r.pending,r.consumed});
    }return out;
}
bool OwnedAcceptMemory::contains(const CancelOperation* op)const{auto& v=*impl_;Impl::Access access(v);for(const auto& r:v.rows)if(auto* p=std::get_if<AsyncAcceptOperation>(&r.object);p&&&p->operation==op)return true;return false;}
void OwnedAcceptMemory::complete(AcceptHandle handle,std::uintptr_t owner,const Error& error){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(handle);if(v.submitting||r.consumed)throw std::logic_error("Premature or duplicate accept completion");
    (void)v.require_region(r); // reject cross-row/foreign spans BEFORE ending loan
    auto* op=&std::get<AsyncAcceptOperation>(r.object);const auto local=error;Impl::Call call(v);r.pin.reset();r.pending=false;r.consumed=true;
    try{complete_async_accept_0271a0(owner,*op,local,v.context());}catch(...){v.poisoned=true;throw;}
    // Native callback may have freed/cached/reprojected old row. No row use here.
}
void OwnedAcceptMemory::note_cancel_requested(AcceptHandle h){auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);if(!r.pending)throw std::logic_error("No pending accept to cancel");r.cancel=true;}
void OwnedAcceptMemory::free_cached_identity(std::uintptr_t identity){auto& v=*impl_;Impl::Access access(v);for(auto& r:v.rows)if(r.storage&&r.storage->identity==identity){release(*r.storage);return;}v.base.free_cached_identity(identity);}
std::span<OperationStorageBinding> OwnedAcceptMemory::storage_bindings(){auto& v=*impl_;Impl::Access access(v);return v.base.storage_bindings();}
std::span<DispatcherOperationBinding> OwnedAcceptMemory::dispatcher_bindings(){auto& v=*impl_;Impl::Access access(v);return v.base.dispatcher_bindings();}
AcceptMemorySnapshot OwnedAcceptMemory::snapshot()const{auto& v=*impl_;std::lock_guard lock(v.gate);AcceptMemorySnapshot result;result.active_calls=v.active;result.closed=v.closed;result.poisoned=v.poisoned;for(const auto& r:v.rows){if(!std::holds_alternative<std::monostate>(r.object))++result.projections;if(r.pending)++result.pending;}return result;}
bool OwnedAcceptMemory::close(){
    auto& v=*impl_;std::shared_ptr<void> lease;
    {std::lock_guard lock(v.gate);if(v.closed)return true;if(v.retiring||v.poisoned||v.active||v.submitting)return false;}
    // Do not hold Accept.gate while waiting on Base.gate: an existing base
    // completion may hold Base.gate and enter this adapter from its callback.
    // Nor seal admission before that callback finishes: a retryable failed
    // close must not turn its legitimate adapter call into a poisoned prefix.
    const auto base=v.base.snapshot();
    if(base.allocations||base.active_calls||base.phase!=IoMemoryPhase::Open)return false;
    {std::lock_guard lock(v.gate);if(v.closed)return true;if(v.retiring||v.poisoned||v.active||v.submitting)return false;
     for(const auto& r:v.rows)if(r.pending||r.pin||!std::holds_alternative<std::monostate>(r.object))return false;
     v.retiring=true;lease=std::move(v.lease);}
    lease.reset();{std::lock_guard lock(v.gate);v.closed=true;v.retiring=false;}return true;
}
void OwnedAcceptMemory::construct_environment(IoHandlerEnvironment& e,std::uintptr_t a){impl_->effect([&]{impl_->lower.construct_environment(e,a);});}
Socket OwnedAcceptMemory::create_socket(std::uint32_t f,std::int32_t t,std::int32_t p,Error& e){return impl_->effect([&]{return impl_->lower.create_socket(f,t,p,e);});}
void OwnedAcceptMemory::reset_owned_socket(Socket& s){impl_->effect([&]{impl_->lower.reset_owned_socket(s);});}
void OwnedAcceptMemory::reset_pending_owner(AcceptPending& p){impl_->effect([&]{impl_->lower.reset_pending_owner(p);});}
Category OwnedAcceptMemory::misc_category(){return impl_->effect([&]{return impl_->lower.misc_category();});}
Category OwnedAcceptMemory::success_category(){return impl_->effect([&]{return impl_->lower.success_category();});}
void OwnedAcceptMemory::retry(AsyncAcceptOperation& op,CancelOperation& selected){
    auto& v=*impl_;Impl::Access access(v);
    if(v.submitting)throw std::logic_error("Reentrant accept retry initialization");
    auto& r=v.rows[v.find(&op.operation.overlapped)];v.arm(r);Impl::Call call(v);v.submitting=true;
    // An inline SDK/lower callback must not consume/free/reuse the projection
    // while the retry prefix still holds its holder, buffer and OVERLAPPED.
    try{v.lower.retry(op,selected);v.submitting=false;}
    catch(...){v.submitting=false;v.poisoned=true;throw;}
}
void OwnedAcceptMemory::invoke(AcceptHandler& handler){impl_->effect([&]{impl_->lower.invoke(handler);});}
void AcceptDispatcherEffects::complete(IocpSchedulerEntry* owner,CancelOperation& op,const Error& error,std::uint32_t bytes){
    if(memory_.contains(&op)){auto handle=memory_.handle_for(&op.overlapped);memory_.complete(handle,reinterpret_cast<std::uintptr_t>(owner),error);return;}
    lower_.complete(owner,op,error,bytes);
}
} // namespace anadius::socket_recovered
