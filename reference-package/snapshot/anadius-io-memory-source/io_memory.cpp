#include "io_memory.hpp"
#include <algorithm>
#include <exception>
#include <limits>
#include <mutex>
#include <variant>
#include <vector>

namespace anadius::socket_recovered {
namespace {
struct PreserveError {
    DWORD value=::GetLastError();
    ~PreserveError(){::SetLastError(value);}
    void restore()const{::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
template<class Error=std::logic_error>[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* message){
    lock.unlock();throw Error(message);
}
void retain(IoReference& control){
    auto count=control.strong_08.load();
    do{if(count==0||count==std::numeric_limits<std::uint32_t>::max())throw std::logic_error("SDK loan requires a live nonsaturated owner strong");}
    while(!control.strong_08.compare_exchange_weak(count,count+1));
}
}
struct OwnedIoMemory::Impl {
    using Projection=std::variant<std::monostate,PostedOperation,AsyncReceiveOperation,AsyncSendOperation,ReceiveHandlerTask,SendHandlerTask,OwnedResponseTask>;
    struct Record {
        OperationStorage storage{};OperationStorageBinding storage_binding{};DispatcherOperationBinding packet{};
        std::uintptr_t raw=0;std::uint64_t extent=0,generation=0;
        IoMemoryKind kind=IoMemoryKind::None;Projection projection;
        std::shared_ptr<void> buffer_pin;IoReference* owner_pin=nullptr;
        bool ready=false,busy=false,pending=false,cancel=false,unknown=false,completed=false,failed=false;
        IoSubmission submission_state=IoSubmission::NotSubmitted;std::uint64_t submission=0;
        Socket submitted_socket=invalid_socket;IoReference* submitted_owner=nullptr;
        std::uintptr_t submitted_buffer=0;std::uint64_t submitted_bytes=0;
        DispatcherOperationBinding** external_packet=nullptr;
    };
    struct Access;
    struct Effect;
    struct Call;
    struct Resolver final:OperationStorageResolver,DispatcherOperationResolver {
        Impl& v;explicit Resolver(Impl& s):v(s){}
        MemoryLookup lookup_storage(std::uintptr_t,OperationStorage*&)override;
        MemoryLookup lookup_operation(OVERLAPPED*,DispatcherOperationBinding*&)override;
        MemoryLookup lookup_operation(CancelOperation*,DispatcherOperationBinding*&)override;
    };
    struct Response final:OwnedResponseMemory {
        Impl& state;explicit Response(Impl& s):state(s){}
        OperationStorage& allocate_04b0bc(std::uint64_t n)override{return state.owner.allocate_04b0bc(n);}
        void free_04b788(OperationStorage& s)override{state.owner.free_04b788(s);}
        PostedOperation& project_posted(OperationStorage& s)override{return state.owner.project_posted(s);}
        OwnedResponseTask& project_owned_response(OperationStorage&,OwnedResponseContext&)override;
    };
    OwnedIoMemory& owner;const std::shared_ptr<SessionMemoryDomain> parent;
    const bool concurrent;std::shared_ptr<void> parent_lease;
    mutable std::mutex gate;IoMemoryPhase phase=IoMemoryPhase::Open;
    std::size_t active_calls=0,effect_count=0;Effect* effects=nullptr;Call* calls=nullptr;
    std::vector<std::shared_ptr<Record>> records;std::vector<std::uint64_t> generations;
    // Serialized compatibility views remain authoritative, including public
    // tamper checks. Concurrent mode resolves the embedded stable Record rows.
    std::vector<OperationStorageBinding> storage;std::vector<DispatcherOperationBinding> packets;
    std::span<OperationStorageBinding> live_storage;std::span<DispatcherOperationBinding> live_packets;
    Resolver resolver{*this};Response response{*this};
    Impl(OwnedIoMemory& o,std::shared_ptr<SessionMemoryDomain> p,std::size_t capacity)
        :owner(o),parent(std::move(p)),concurrent(parent->operation_policy()==OperationPolicy::OriginalConcurrentActors),
         records(capacity),generations(capacity),storage(concurrent?0:capacity),packets(concurrent?0:capacity),live_storage(storage),live_packets(packets){
        parent_lease=parent->acquire_provider_lease();
    }
    void poison(){PreserveError error;std::unique_lock lock(gate);phase=IoMemoryPhase::Poisoned;}
    struct Access {
        Impl& v;const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        std::unique_lock<std::mutex> lock;
        explicit Access(Impl& s):v(s),lock(s.gate,std::defer_lock){relock();}
        ~Access(){PreserveError error;if(lock.owns_lock())lock.unlock();}
        void unlock(){PreserveError error;if(lock.owns_lock())lock.unlock();}
        void relock(Effect* allowed=nullptr){
            PreserveError error;
            // Parent entry, not TLS state or IO metadata, supplies authority.
            (void)v.parent->buffer_context();
            lock.lock();v.check_locked(*this,allowed);
        }
    };
    void check_locked(Access& access,Effect* allowed=nullptr){
        if(phase!=IoMemoryPhase::Open)reject(access.lock,"IO memory is closed or poisoned");
        for(auto* effect=effects;effect;effect=effect->next)
            if(effect!=allowed&&effect->thread==access.thread&&effect->fiber==access.fiber){
                phase=IoMemoryPhase::Poisoned;reject(access.lock,"IO memory reentered during an allocation or lifecycle effect");
            }
    }
    struct Effect {
        Access& access;Impl& v;const DWORD thread;void* const fiber;Effect* next=nullptr;
        std::shared_ptr<Record> record;
        explicit Effect(Access& a,std::shared_ptr<Record> r={}):access(a),v(a.v),thread(a.thread),fiber(a.fiber),record(std::move(r)){
            v.check_locked(access);
            if(v.active_calls==std::numeric_limits<std::size_t>::max()-v.effect_count)reject<std::overflow_error>(access.lock,"IO active source count exhausted");
            if(record){v.check_row_locked(*record,access);if(record->busy)reject(access.lock,"IO row already has an active lifecycle effect");record->busy=true;}
            next=v.effects;v.effects=this;++v.effect_count;
        }
        ~Effect(){
            PreserveError error;
            if(!access.lock.owns_lock())access.lock.lock();
            if(record)record->busy=false;
            auto** at=&v.effects;while(*at&&*at!=this)at=&(*at)->next;
            if(!*at||!v.effect_count)std::terminate();*at=next;--v.effect_count;
            access.lock.unlock();record.reset();
        }
        void relock(){access.relock(this);}
    };
    struct Call {
        Access& access;Impl& v;const DWORD thread;void* const fiber;const std::uint64_t generation;
        Call* next=nullptr;std::shared_ptr<Record> record;
        Call(Access& a,std::shared_ptr<Record> r):access(a),v(a.v),thread(a.thread),fiber(a.fiber),generation(r->generation),record(std::move(r)){
            v.check_row_locked(*record,access);
            if(v.active_calls==std::numeric_limits<std::size_t>::max()-v.effect_count)reject<std::overflow_error>(access.lock,"IO active source count exhausted");
            next=v.calls;v.calls=this;++v.active_calls;
        }
        ~Call(){
            PreserveError error;
            if(!access.lock.owns_lock())access.lock.lock();
            auto** at=&v.calls;while(*at&&*at!=this)at=&(*at)->next;
            if(!*at||!v.active_calls)std::terminate();*at=next;--v.active_calls;
            access.lock.unlock();record.reset();
            // No old projection/slot/generation access after the native call.
        }
    };
    void check_row_locked(const Record& r,Access& access)const{
        if(r.busy)reject(access.lock,"IO row already has an active lifecycle effect");
        for(auto* call=calls;call;call=call->next)
            if(call->record.get()==&r&&call->generation==r.generation&&(call->thread!=access.thread||call->fiber!=access.fiber))
                reject(access.lock,"IO projection is still owned by its actual callback actor");
    }
    std::shared_ptr<Record> at(IoMemoryHandle h,Access& access){
        if(h.slot>=records.size()||!records[h.slot]||records[h.slot]->generation!=h.generation||!records[h.slot]->ready||records[h.slot]->failed)
            reject<std::invalid_argument>(access.lock,"Stale or failed IO projection handle");
        auto r=records[h.slot];check_row_locked(*r,access);return r;
    }
    std::size_t index(const OperationStorage& s,Access& access)const{
        for(std::size_t i=0;i<records.size();++i)if(records[i]&&records[i]->ready&&&records[i]->storage==&s){
            const auto& r=*records[i];check_row_locked(r,access);
            if(s.identity!=r.raw||s.bytes.data()!=reinterpret_cast<std::byte*>(r.raw)||s.bytes.size()!=r.extent)
                reject<std::invalid_argument>(access.lock,"Owned operation descriptor was altered");
            return i;
        }
        reject<std::invalid_argument>(access.lock,"Operation storage descriptor is not owned/live");
    }
    DispatcherOperationBinding& packet(std::size_t i){return concurrent?records[i]->packet:packets[i];}
    void clear_binding(std::size_t i)noexcept{
        if(auto*& alias=records[i]->external_packet;alias){*alias=nullptr;alias=nullptr;}
        packet(i)={};
    }
    void validate_aliases(Access& access)const{
        for(std::size_t i=0;i<records.size();++i)if(records[i]&&records[i]->ready&&!records[i]->busy){
            const auto& r=*records[i];const auto* expected=concurrent?&r.packet:&packets[i];
            if(r.external_packet&&*r.external_packet!=expected)reject(access.lock,"External IO packet field was altered");
            if(r.kind==IoMemoryKind::Receive&&std::get<AsyncReceiveOperation>(r.projection).packet!=expected)reject(access.lock,"Receive packet field was altered");
            if(r.kind==IoMemoryKind::Send&&std::get<AsyncSendOperation>(r.projection).packet!=expected)reject(access.lock,"Send packet field was altered");
        }
    }
    void reserve_record(Effect& effect,std::shared_ptr<Record> prepared,std::size_t& slot){
        auto& access=effect.access;PreserveError error;
        for(;;){
            auto empty=[&]{std::size_t i=0;while(i<records.size()&&records[i])++i;return i;};
            slot=empty();
            if(slot<records.size()){
                if(generations[slot]==std::numeric_limits<std::uint64_t>::max())reject<std::overflow_error>(access.lock,"IO generation exhausted");
                prepared->generation=++generations[slot];prepared->busy=true;
                records[slot]=prepared;effect.record=std::move(prepared);return;
            }
            const auto count=records.size();
            const auto limit=std::min(records.max_size(),generations.max_size());
            if(count>=limit)reject<std::length_error>(access.lock,"IO registry exceeds container capacity");
            const auto grown=count>limit-count?limit:count*2;
            access.unlock();
            std::vector<std::shared_ptr<Record>> next_records(grown);
            std::vector<std::uint64_t> next_generations(grown);
            std::vector<OperationStorageBinding> next_storage(concurrent?0:grown);
            std::vector<DispatcherOperationBinding> next_packets(concurrent?0:grown);
            effect.relock();
            // A sibling may have grown/claimed a slot during preparation. Use
            // its free slot or prepare a larger table without entering effects.
            if(empty()<records.size()||records.size()>=grown){
                access.unlock();decltype(next_records){}.swap(next_records);decltype(next_generations){}.swap(next_generations);
                decltype(next_storage){}.swap(next_storage);decltype(next_packets){}.swap(next_packets);effect.relock();continue;
            }
            validate_aliases(access);const auto current=records.size();
            std::copy(generations.begin(),generations.end(),next_generations.begin());
            if(!concurrent){std::copy(storage.begin(),storage.end(),next_storage.begin());std::copy(packets.begin(),packets.end(),next_packets.begin());}
            for(std::size_t i=0;i<current;++i)next_records[i]=std::move(records[i]);
            records.swap(next_records);generations.swap(next_generations);
            if(!concurrent){
                storage.swap(next_storage);packets.swap(next_packets);
                for(std::size_t i=0;i<current;++i)if(records[i]&&records[i]->ready){
                    auto& r=*records[i];
                    if(r.kind==IoMemoryKind::Receive)std::get<AsyncReceiveOperation>(r.projection).packet=&packets[i];
                    if(r.kind==IoMemoryKind::Send)std::get<AsyncSendOperation>(r.projection).packet=&packets[i];
                    if(r.external_packet)*r.external_packet=&packets[i];
                }
                live_storage=storage;live_packets=packets;
            }
            // Replaced vectors contain no live owners. Their allocation/free
            // stays outside the gate even while old Record addresses survive.
            access.unlock();decltype(next_records){}.swap(next_records);decltype(next_generations){}.swap(next_generations);
            decltype(next_storage){}.swap(next_storage);decltype(next_packets){}.swap(next_packets);effect.relock();
        }
    }
    template<class Make>auto& project(OperationStorage& s,std::uint64_t minimum,IoMemoryKind kind,Make&& make){
        PreserveError error;
        Access access(*this);const auto i=index(s,access);auto r=records[i];Effect effect(access,r);
        if(r->failed||r->pending||r->owner_pin||r->buffer_pin||r->extent<minimum)reject(access.lock,"IO projection still borrowed, failed or too small");
        if(generations[i]==std::numeric_limits<std::uint64_t>::max())reject<std::overflow_error>(access.lock,"IO projection generation exhausted");
        clear_binding(i);access.unlock();
        try{
            auto& result=make(*r);effect.relock();
            r->generation=++generations[i];r->kind=kind;r->completed=false;r->cancel=false;r->unknown=false;
            r->submission_state=IoSubmission::NotSubmitted;r->submitted_socket=invalid_socket;
            r->submitted_owner=nullptr;r->submitted_buffer=0;r->submitted_bytes=0;
            if(kind==IoMemoryKind::Posted){auto& p=std::get<PostedOperation>(r->projection);packet(i)={&p.operation,{},0,&p.operation.overlapped};}
            if(kind==IoMemoryKind::Receive){auto& p=std::get<AsyncReceiveOperation>(r->projection);packet(i)={&p.io.operation,{},0,&p.io.operation.overlapped};p.packet=&packet(i);}
            if(kind==IoMemoryKind::Send){auto& p=std::get<AsyncSendOperation>(r->projection);packet(i)={&p.io.operation,{},0,&p.io.operation.overlapped};p.packet=&packet(i);}
            return result;
        }catch(...){
            if(!access.lock.owns_lock())access.lock.lock();r->failed=true;phase=IoMemoryPhase::Poisoned;throw;
        }
    }
    IoLoan describe(std::size_t i,Access& access,bool strict=true){
        auto& r=*records[i];check_row_locked(r,access);CancelOperation* actual=nullptr;
        if(r.kind==IoMemoryKind::Receive)actual=&std::get<AsyncReceiveOperation>(r.projection).io.operation;
        else if(r.kind==IoMemoryKind::Send)actual=&std::get<AsyncSendOperation>(r.projection).io.operation;
        else if(r.kind==IoMemoryKind::Posted)actual=&std::get<PostedOperation>(r.projection).operation;
        else reject(access.lock,"IO loan has no private packet projection");
        const auto& binding=packet(i);
        const bool valid=(binding.operation==actual&&binding.overlapped_identity==&actual->overlapped)||(r.completed&&!binding.operation&&!binding.overlapped_identity);
        if(strict&&!valid){phase=IoMemoryPhase::Poisoned;reject(access.lock,"Public IO binding differs from the authoritative private projection");}
        return {{i,r.generation},r.kind,r.submission_state,r.submission,r.submitted_socket,
            &actual->overlapped,actual,r.submitted_owner,r.submitted_buffer,r.submitted_bytes,r.pending,r.completed,valid,binding.operation,binding.overlapped_identity};
    }
    IoMemoryHandle arm(Access& access,OVERLAPPED& overlapped,const Buffer& buffer,IoMemoryKind expected,Socket socket,bool typed){
        PreserveError error;
        std::size_t i=records.size();
        for(std::size_t n=0;n<records.size();++n)if(records[n]&&records[n]->ready&&packet(n).overlapped_identity==&overlapped&&packet(n).operation){i=n;break;}
        if(i==records.size())reject<std::invalid_argument>(access.lock,"SDK OVERLAPPED has no current owned projection");
        auto r=records[i];Effect effect(access,r);
        if(r->kind!=expected||r->pending||r->completed||r->failed)reject(access.lock,"IO submission has no fresh matching projection");
        if(r->submission==std::numeric_limits<std::uint64_t>::max())reject<std::overflow_error>(access.lock,"IO submission sequence exhausted");
        auto& native=expected==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r->projection).io.operation:std::get<AsyncSendOperation>(r->projection).io.operation;
        if(&native.overlapped!=&overlapped||packet(i).operation!=&native)reject<std::invalid_argument>(access.lock,"SDK binding must name the actual typed OVERLAPPED");
        auto* control=expected==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r->projection).io.handler.control_a8:std::get<AsyncSendOperation>(r->projection).io.handler.control_98;
        const auto generation=r->generation;access.unlock();std::shared_ptr<void> pin;bool retained=false;
        try{
            if(!parent->owns_control(control))throw std::invalid_argument("SDK operation owner must be a live control from the same session domain");
            pin=parent->pin_buffer(reinterpret_cast<std::uintptr_t>(buffer.buf),buffer.len);retain(*control);retained=true;
            effect.relock();
            r->owner_pin=control;r->buffer_pin=std::move(pin);r->pending=true;
            ++r->submission;r->submission_state=typed?IoSubmission::Issuing:IoSubmission::Unclassified;
            r->submitted_socket=socket;r->submitted_owner=control;r->submitted_buffer=reinterpret_cast<std::uintptr_t>(buffer.buf);r->submitted_bytes=buffer.len;
            return {i,generation};
        }catch(...){
            access.unlock();
            const bool healthy=parent->snapshot().phase==SessionMemoryPhase::Open;
            if(!access.lock.owns_lock())access.lock.lock();
            // Retain any completed compatibility ownership prefix. Never
            // silently drop an acquired strong after an unknown postcheck.
            if(retained){r->owner_pin=control;r->buffer_pin=std::move(pin);r->pending=true;r->unknown=true;r->submission_state=IoSubmission::Unknown;r->failed=true;phase=IoMemoryPhase::Poisoned;}
            else if(!healthy){r->failed=true;phase=IoMemoryPhase::Poisoned;}
            access.unlock();throw;
        }
    }
    void observed(std::size_t i,Access& access){
        PreserveError error;
        auto r=records[i];if(r->completed||r->unknown)reject(access.lock,"Completion already consumed or submission outcome unknown");
        IoReference* control=nullptr;
        if(r->pending){
            auto* native=r->kind==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r->projection).io.handler.control_a8:std::get<AsyncSendOperation>(r->projection).io.handler.control_98;
            if(!r->owner_pin||native!=r->owner_pin){phase=IoMemoryPhase::Poisoned;reject(access.lock,"Pending source handler owner changed");}
            control=r->owner_pin;
        }
        access.unlock();
        if(control){
            auto count=control->strong_08.load();
            do{if(count<2){poison();throw std::logic_error("Pending source owner strong was consumed before terminal completion");}}
            while(!control->strong_08.compare_exchange_weak(count,count-1));
        }
        // The exact Call reservation excludes foreign same-generation reuse.
        // There is no callback/checkpoint between the non-last decrement and
        // this commit. A failed count guard above leaves the original pending
        // row, owner pointer and range pin wholly retained.
        std::shared_ptr<void> pin;access.lock.lock();
        // A different row can poison the owner while this actor was unlocked.
        // Revalidate metadata before forgetting a loan or entering its native
        // callback. Any already consumed CAS remains a retained, nonreplayable
        // prefix; do not invoke a Parent/admission callback in this interval.
        check_locked(access);
        if(control){r->owner_pin=nullptr;pin=std::move(r->buffer_pin);r->pending=false;}
        r->completed=true;r->cancel=false;clear_binding(i);access.unlock();
        pin.reset(); // Parent lease destructor never runs under IO gate.
    }
    template<class Select>MemoryLookup lookup_packet(Access& access,Select&& select,DispatcherOperationBinding*& out){
        DispatcherOperationBinding* found=nullptr;
        for(std::size_t i=0;i<records.size();++i)if(records[i]&&records[i]->ready&&!records[i]->failed&&!records[i]->busy){
            auto& r=*records[i];auto& b=packet(i);if(!b.operation||!select(b))continue;
            check_row_locked(r,access);
            if(found)return MemoryLookup::Ambiguous;found=&b;
        }
        if(!found)return MemoryLookup::Miss;out=found;return MemoryLookup::Found;
    }
};
OwnedIoMemory::OwnedIoMemory(std::shared_ptr<SessionMemoryDomain> parent,std::size_t capacity){
    if(!parent||!capacity||capacity>65536)throw std::invalid_argument("Owned IO memory requires parent and bounded positive capacity");
    impl_=std::make_unique<Impl>(*this,std::move(parent),capacity);
}
OwnedIoMemory::~OwnedIoMemory()noexcept{if(impl_->phase!=IoMemoryPhase::Closed||impl_->active_calls||impl_->effect_count)std::terminate();}
OperationStorage& OwnedIoMemory::allocate_04b0bc(std::uint64_t bytes){
    PreserveError result_error;
    auto& v=*impl_;Impl::Access access(v);Impl::Effect effect(access);
    if(!bytes)reject<std::invalid_argument>(access.lock,"IO storage needs its marker byte");
    std::shared_ptr<Impl::Record> r;std::size_t i=0;
    {PreserveError error;access.unlock();r=std::make_shared<Impl::Record>();r->extent=bytes;effect.relock();v.reserve_record(effect,r,i);}
    access.unlock();std::uintptr_t raw=0;
    try{
        auto& context=v.parent->buffer_context();result_error.restore();raw=context.api.allocate_04b0bc(bytes);result_error.capture();
        // Reserved private fields are not read by live resolvers/snapshots.
        r->raw=raw;r->storage.identity=raw;r->storage.bytes={reinterpret_cast<std::byte*>(raw),static_cast<std::size_t>(bytes)};r->storage_binding={raw,&r->storage};
        effect.relock();r->ready=true;if(!v.concurrent)v.storage[i]=r->storage_binding;return r->storage;
    }catch(...){
        result_error.capture();
        access.unlock();
        const bool healthy=!raw&&v.parent->snapshot().phase==SessionMemoryPhase::Open;
        if(!access.lock.owns_lock())access.lock.lock();
        if(healthy&&v.phase==IoMemoryPhase::Open)v.records[i].reset();
        else{r->failed=true;v.phase=IoMemoryPhase::Poisoned;}
        access.unlock();throw;
    }
}
void OwnedIoMemory::free_04b788(OperationStorage& storage){
    PreserveError result_error;
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage,access);auto r=v.records[i];Impl::Effect effect(access,r);
    if(r->pending||r->owner_pin||r->buffer_pin)reject(access.lock,"SDK borrower has not produced terminal completion");
    const auto raw=r->raw,extent=r->extent;v.clear_binding(i);if(!v.concurrent)v.storage[i]={};r->storage_binding={};r->ready=false;
    access.unlock();
    try{
        r->projection.emplace<std::monostate>();r->kind=IoMemoryKind::None;
        result_error.restore();v.parent->free_sized_04b0f8(raw,extent);result_error.capture();effect.relock();v.records[i].reset();
    }catch(...){
        result_error.capture();
        if(!access.lock.owns_lock())access.lock.lock();r->failed=true;v.phase=IoMemoryPhase::Poisoned;throw;
    }
}
void OwnedIoMemory::free_cached_identity(std::uintptr_t identity){
    PreserveError result_error;
    auto& v=*impl_;Impl::Access access(v);std::shared_ptr<Impl::Record> r;
    for(const auto& candidate:v.records)if(candidate&&candidate->ready&&candidate->raw==identity){r=candidate;break;}
    if(!r)reject<std::invalid_argument>(access.lock,"Worker cache identity is not owned/live");
    access.unlock();
    try{result_error.restore();free_04b788(r->storage);result_error.capture();}
    catch(...){result_error.capture();throw;}
}
PostedOperation& OwnedIoMemory::project_posted(OperationStorage& s){
    return impl_->project(s,0x49,IoMemoryKind::Posted,[](auto& r)->PostedOperation&{return r.projection.template emplace<PostedOperation>(PostedOperation{r.storage,{},0,nullptr});});
}
AsyncReceiveOperation& OwnedIoMemory::project_async_receive(OperationStorage& s){
    return impl_->project(s,0xe1,IoMemoryKind::Receive,[](auto& r)->AsyncReceiveOperation&{return r.projection.template emplace<AsyncReceiveOperation>(r.storage);});
}
AsyncSendOperation& OwnedIoMemory::project_async_send(OperationStorage& s){
    return impl_->project(s,0xd1,IoMemoryKind::Send,[](auto& r)->AsyncSendOperation&{return r.projection.template emplace<AsyncSendOperation>(r.storage);});
}
ReceiveHandlerTask& OwnedIoMemory::project_receive_task(OperationStorage& s,HandlerTaskContext& context){
    return impl_->project(s,0x79,IoMemoryKind::ReceiveTask,[&](auto& r)->ReceiveHandlerTask&{return r.projection.template emplace<ReceiveHandlerTask>(r.storage,context);});
}
SendHandlerTask& OwnedIoMemory::project_send_task(OperationStorage& s,HandlerTaskContext& context){
    return impl_->project(s,0x71,IoMemoryKind::SendTask,[&](auto& r)->SendHandlerTask&{return r.projection.template emplace<SendHandlerTask>(r.storage,context);});
}
OwnedResponseTask& OwnedIoMemory::Impl::Response::project_owned_response(OperationStorage& s,OwnedResponseContext& context){
    return state.project(s,0x49,IoMemoryKind::Response,[&](auto& r)->OwnedResponseTask&{return r.projection.template emplace<OwnedResponseTask>(r.storage,context);});
}
OwnedResponseMemory& OwnedIoMemory::response_memory(){return impl_->response;}
std::span<OperationStorageBinding> OwnedIoMemory::storage_bindings(){
    if(impl_->concurrent)throw std::logic_error("Concurrent IO requires exact storage resolver");return impl_->storage;
}
std::span<DispatcherOperationBinding> OwnedIoMemory::dispatcher_bindings(){
    if(impl_->concurrent)throw std::logic_error("Concurrent IO requires exact dispatcher resolver");return impl_->packets;
}
const std::span<OperationStorageBinding>* OwnedIoMemory::storage_view()const{
    if(impl_->concurrent)throw std::logic_error("Concurrent IO requires exact storage resolver");return &impl_->live_storage;
}
const std::span<DispatcherOperationBinding>* OwnedIoMemory::dispatcher_view()const{
    if(impl_->concurrent)throw std::logic_error("Concurrent IO requires exact dispatcher resolver");return &impl_->live_packets;
}
OperationStorageResolver& OwnedIoMemory::storage_resolver()const noexcept{return impl_->resolver;}
DispatcherOperationResolver& OwnedIoMemory::dispatcher_resolver()const noexcept{return impl_->resolver;}
std::size_t OwnedIoMemory::registry_capacity()const{PreserveError error;std::unique_lock lock(impl_->gate);return impl_->records.size();}
DispatcherOperationBinding& OwnedIoMemory::packet_binding(OperationStorage& storage){
    auto& v=*impl_;Impl::Access access(v);return v.packet(v.index(storage,access));
}
MemoryLookup OwnedIoMemory::Impl::Resolver::lookup_storage(std::uintptr_t identity,OperationStorage*& out){
    PreserveError error;Access access(v);OperationStorage* found=nullptr;
    for(const auto& r:v.records)if(r&&r->ready&&!r->failed&&!r->busy&&r->raw==identity){
        v.check_row_locked(*r,access);
        if(r->storage.identity!=r->raw||r->storage.bytes.data()!=reinterpret_cast<std::byte*>(r->raw)||r->storage.bytes.size()!=r->extent)
            reject<std::invalid_argument>(access.lock,"Owned operation descriptor was altered");
        if(found)return MemoryLookup::Ambiguous;found=&r->storage;
    }
    if(!found)return MemoryLookup::Miss;out=found;return MemoryLookup::Found;
}
MemoryLookup OwnedIoMemory::Impl::Resolver::lookup_operation(OVERLAPPED* raw,DispatcherOperationBinding*& out){
    PreserveError error;Access access(v);return v.lookup_packet(access,[&](const auto& b){return b.overlapped_identity==raw;},out);
}
MemoryLookup OwnedIoMemory::Impl::Resolver::lookup_operation(CancelOperation* raw,DispatcherOperationBinding*& out){
    PreserveError error;Access access(v);return v.lookup_packet(access,[&](const auto& b){return b.operation==raw;},out);
}
void OwnedIoMemory::bind_external_packet(OperationStorage& storage,DispatcherOperationBinding*& packet){
    PreserveError error;auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage,access);auto& r=*v.records[i];
    if(packet!=&v.packet(i)||(r.external_packet&&r.external_packet!=&packet))reject<std::invalid_argument>(access.lock,"External packet field is not the current owned row");
    for(const auto& other:v.records)if(other&&other.get()!=&r&&other->external_packet==&packet)reject<std::invalid_argument>(access.lock,"External packet field belongs to another owned row");
    r.external_packet=&packet;
}
void OwnedIoMemory::unbind_external_packet(DispatcherOperationBinding*& packet){
    PreserveError error;auto& v=*impl_;Impl::Access access(v);
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->external_packet==&packet){
        v.check_row_locked(*v.records[i],access);
        if(packet&&packet!=&v.packet(i))reject<std::invalid_argument>(access.lock,"External packet unbind differs from its registered row");
        packet=nullptr;v.records[i]->external_packet=nullptr;return;
    }
    if(packet)reject<std::invalid_argument>(access.lock,"Unregistered external packet field is not cleared");
}
IoMemoryHandle OwnedIoMemory::handle_for(const OperationStorage& storage)const{
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage,access);return {i,v.records[i]->generation};
}
IoMemoryHandle OwnedIoMemory::handle_for(const OVERLAPPED* raw)const{
    auto& v=*impl_;Impl::Access access(v);
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->ready&&v.packet(i).operation&&v.packet(i).overlapped_identity==raw){v.check_row_locked(*v.records[i],access);return {i,v.records[i]->generation};}
    reject<std::invalid_argument>(access.lock,"OVERLAPPED binding is inactive/stale");
}
IoMemoryHandle OwnedIoMemory::handle_for(const CancelOperation* op)const{
    auto& v=*impl_;Impl::Access access(v);
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->ready&&v.packet(i).operation==op&&op){v.check_row_locked(*v.records[i],access);return {i,v.records[i]->generation};}
    reject<std::invalid_argument>(access.lock,"Operation binding is inactive/stale");
}
IoMemoryKind OwnedIoMemory::kind(IoMemoryHandle h)const{auto& v=*impl_;Impl::Access access(v);return v.at(h,access)->kind;}
AsyncReceiveOperation& OwnedIoMemory::receive_operation(IoMemoryHandle h){auto& v=*impl_;Impl::Access access(v);return std::get<AsyncReceiveOperation>(v.at(h,access)->projection);}
AsyncSendOperation& OwnedIoMemory::send_operation(IoMemoryHandle h){auto& v=*impl_;Impl::Access access(v);return std::get<AsyncSendOperation>(v.at(h,access)->projection);}
IoMemoryHandle OwnedIoMemory::arm_receive(OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(access,ov,buffer,IoMemoryKind::Receive,invalid_socket,false);}
IoMemoryHandle OwnedIoMemory::arm_send(OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(access,ov,buffer,IoMemoryKind::Send,invalid_socket,false);}
IoMemoryHandle OwnedIoMemory::arm_receive(Socket socket,OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(access,ov,buffer,IoMemoryKind::Receive,socket,true);}
IoMemoryHandle OwnedIoMemory::arm_send(Socket socket,OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(access,ov,buffer,IoMemoryKind::Send,socket,true);}
void OwnedIoMemory::note_submission(IoMemoryHandle h,std::int32_t result,std::uint32_t error){
    PreserveError saved;auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(!r->pending||r->completed||r->submission_state!=IoSubmission::Issuing||(r->kind!=IoMemoryKind::Receive&&r->kind!=IoMemoryKind::Send))
        reject(access.lock,"IO SDK result no longer matches its issuing loan");
    auto* const control=r->kind==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r->projection).io.handler.control_a8:std::get<AsyncSendOperation>(r->projection).io.handler.control_98;
    if(control!=r->submitted_owner||r->owner_pin!=r->submitted_owner)reject(access.lock,"IO SDK changed the captured source owner");
    (void)v.describe(h.slot,access);r->submission_state=(result==0||error==997)?IoSubmission::Kernel:IoSubmission::Software;
}
IoLoan OwnedIoMemory::loan(IoMemoryHandle h)const{PreserveError error;auto& v=*impl_;Impl::Access access(v);(void)v.at(h,access);return v.describe(h.slot,access);}
std::vector<IoLoan> OwnedIoMemory::pending_loans()const{
    PreserveError error;auto& v=*impl_;Impl::Access access(v);Impl::Effect effect(access);std::vector<IoLoan> out;
    for(;;){const auto count=v.records.size();access.unlock();out.reserve(count);effect.relock();if(out.capacity()>=v.records.size())break;}
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->ready&&!v.records[i]->busy&&v.records[i]->pending)out.push_back(v.describe(i,access));
    access.unlock();return out;
}
std::vector<IoLoan> OwnedIoMemory::active_loans()const{
    PreserveError error;auto& v=*impl_;Impl::Access access(v);Impl::Effect effect(access);std::vector<IoLoan> out;
    for(;;){const auto count=v.records.size();access.unlock();out.reserve(count);effect.relock();if(out.capacity()>=v.records.size())break;}
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->ready&&!v.records[i]->busy){
        const auto& r=*v.records[i];
        if(!r.completed&&(r.kind==IoMemoryKind::Receive||r.kind==IoMemoryKind::Send||r.kind==IoMemoryKind::Posted))out.push_back(v.describe(i,access,r.kind!=IoMemoryKind::Posted));
    }
    access.unlock();return out;
}
void OwnedIoMemory::note_cancel_requested(IoMemoryHandle h){
    PreserveError error;auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(!r->pending)reject(access.lock,"Cancellation requires a pending submission");r->cancel=true;
}
void OwnedIoMemory::note_unknown_submission(IoMemoryHandle h)noexcept{
    PreserveError error;auto& v=*impl_;std::unique_lock lock(v.gate);v.phase=IoMemoryPhase::Poisoned;
    if(h.slot<v.records.size()&&v.records[h.slot]&&v.records[h.slot]->generation==h.generation){v.records[h.slot]->unknown=true;v.records[h.slot]->submission_state=IoSubmission::Unknown;}
}
void OwnedIoMemory::complete_receive(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects){
    complete_receive_with_memory(h,owner,error,bytes,env,api,errors,effects,*this);
}
void OwnedIoMemory::complete_receive_with_memory(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(r->completed||r->unknown)reject(access.lock,"Completion already consumed or submission outcome unknown");
    Impl::Call call(access,std::move(r));
    auto* operation=&std::get<AsyncReceiveOperation>(call.record->projection).io;const auto local_error=error;
    try{v.observed(h.slot,access);complete_receive_027710(owner,*operation,local_error,bytes,env,api,memory,errors,effects);}
    catch(...){access.unlock();v.poison();throw;}
}
void OwnedIoMemory::complete_send(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects){
    complete_send_with_memory(h,owner,error,bytes,env,api,errors,effects,*this);
}
void OwnedIoMemory::complete_send_with_memory(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(r->completed||r->unknown)reject(access.lock,"Completion already consumed or submission outcome unknown");
    Impl::Call call(access,std::move(r));
    auto* operation=&std::get<AsyncSendOperation>(call.record->projection).io;const auto local_error=error;
    try{v.observed(h.slot,access);complete_send_027a20(owner,*operation,local_error,bytes,env,api,memory,errors,effects);}
    catch(...){access.unlock();v.poison();throw;}
}
void OwnedIoMemory::complete_posted(IoMemoryHandle h,std::uintptr_t owner,PostedEnvironment env,WorkerRunApi& api){
    complete_posted_with_memory(h,owner,env,api,*this);
}
void OwnedIoMemory::complete_posted_with_memory(IoMemoryHandle h,std::uintptr_t owner,PostedEnvironment env,WorkerRunApi& api,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(r->completed||r->unknown)reject(access.lock,"Completion already consumed or submission outcome unknown");
    Impl::Call call(access,std::move(r));
    auto* operation=&std::get<PostedOperation>(call.record->projection);
    try{v.observed(h.slot,access);complete_posted_027120(owner,*operation,env,api,memory);}
    catch(...){access.unlock();v.poison();throw;}
}
void OwnedIoMemory::invoke_task(IoMemoryHandle h,std::uint8_t flag){
    auto& v=*impl_;Impl::Access access(v);auto r=v.at(h,access);
    if(r->kind!=IoMemoryKind::ReceiveTask&&r->kind!=IoMemoryKind::SendTask&&r->kind!=IoMemoryKind::Response)reject<std::invalid_argument>(access.lock,"IO handle is not an owned task");
    if(r->completed)reject(access.lock,"Owned task already consumed");
    Impl::Call call(access,std::move(r));PostedFunctor* task=nullptr;
    if(call.record->kind==IoMemoryKind::ReceiveTask)task=&std::get<ReceiveHandlerTask>(call.record->projection);
    else if(call.record->kind==IoMemoryKind::SendTask)task=&std::get<SendHandlerTask>(call.record->projection);
    else task=&std::get<OwnedResponseTask>(call.record->projection);
    call.record->completed=true;access.unlock();
    try{task->call_slot_00(flag);}catch(...){v.poison();throw;}
}
IoMemorySnapshot OwnedIoMemory::snapshot()const{
    PreserveError error;auto& v=*impl_;std::unique_lock lock(v.gate);IoMemorySnapshot out;out.phase=v.phase;out.active_calls=v.active_calls+v.effect_count;
    for(const auto& r:v.records)if(r){++out.allocations;if(r->pending)++out.pending;if(r->cancel)++out.cancel_requested;}return out;
}
bool OwnedIoMemory::close(){
    PreserveError error;auto& v=*impl_;std::shared_ptr<void> retiring;
    {std::unique_lock lock(v.gate);if(v.phase==IoMemoryPhase::Closed)return true;
     if(v.phase!=IoMemoryPhase::Open||v.effects||v.active_calls||std::any_of(v.records.begin(),v.records.end(),[](const auto& p){return bool(p);}))return false;
     v.phase=IoMemoryPhase::Retiring;retiring=std::move(v.parent_lease);}
    retiring.reset();{std::unique_lock lock(v.gate);v.phase=IoMemoryPhase::Closed;}return true;
}
AsyncReceiveResult PinnedAsyncReceiveApi::receive(Socket socket,Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov){
    auto handle=memory_.arm_receive(socket,ov,buffer);const auto original=buffer;
    try{const auto result=lower_.receive(socket,buffer,flags,ov);if(buffer.buf!=original.buf||buffer.len!=original.len)throw std::logic_error("Receive SDK changed captured WSABUF operands");memory_.note_submission(handle,result.result,result.last_error);return result;}catch(...){memory_.note_unknown_submission(handle);throw;}
}
AsyncSendResult PinnedAsyncSendApi::send(Socket socket,Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov){
    auto handle=memory_.arm_send(socket,ov,buffer);const auto original=buffer;
    try{const auto result=lower_.send(socket,buffer,flags,ov);if(buffer.buf!=original.buf||buffer.len!=original.len)throw std::logic_error("Send SDK changed captured WSABUF operands");memory_.note_submission(handle,result.result,result.last_error);return result;}catch(...){memory_.note_unknown_submission(handle);throw;}
}
void OwnedIoDispatcherEffects::complete(IocpSchedulerEntry* owner,CancelOperation& operation,const Error& error,std::uint32_t bytes){
    const auto handle=memory_.handle_for(&operation);const auto who=reinterpret_cast<std::uintptr_t>(owner);
    const auto kind=memory_.kind(handle);
    if(kind==IoMemoryKind::Receive){memory_.complete_receive(handle,who,error,bytes,environment_,api_,errors_,effects_);return;}
    if(kind==IoMemoryKind::Send){memory_.complete_send(handle,who,error,bytes,environment_,api_,errors_,effects_);return;}
    if(kind==IoMemoryKind::Posted){memory_.complete_posted(handle,who,environment_.storage,api_);return;}
    throw std::invalid_argument("Dispatcher operation is not owned posted/IO work");
}
} // namespace anadius::socket_recovered
