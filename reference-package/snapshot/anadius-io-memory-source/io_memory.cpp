#include "io_memory.hpp"
#include <algorithm>
#include <exception>
#include <limits>
#include <variant>
#include <vector>

namespace anadius::socket_recovered {
namespace {
void retain(IoReference& control){
    auto count=control.strong_08.load();
    do{if(count==0||count==std::numeric_limits<std::uint32_t>::max())throw std::logic_error("SDK loan requires a live nonsaturated owner strong");}
    while(!control.strong_08.compare_exchange_weak(count,count+1));
}
}
struct OwnedIoMemory::Impl {
    using Projection=std::variant<std::monostate,PostedOperation,AsyncReceiveOperation,AsyncSendOperation,ReceiveHandlerTask,SendHandlerTask,OwnedResponseTask>;
    struct Record {
        OperationStorage storage{};std::uintptr_t raw=0;std::uint64_t extent=0,generation=0;
        IoMemoryKind kind=IoMemoryKind::None;Projection projection;
        std::shared_ptr<void> buffer_pin;IoReference* owner_pin=nullptr;
        bool pending=false,cancel=false,unknown=false,completed=false,failed=false;
        IoSubmission submission_state=IoSubmission::NotSubmitted;std::uint64_t submission=0;
        Socket submitted_socket=invalid_socket;IoReference* submitted_owner=nullptr;
        std::uintptr_t submitted_buffer=0;std::uint64_t submitted_bytes=0;
    };
    struct Response final:OwnedResponseMemory {
        Impl& state;explicit Response(Impl& s):state(s){}
        OperationStorage& allocate_04b0bc(std::uint64_t n)override{return state.owner.allocate_04b0bc(n);}
        void free_04b788(OperationStorage& s)override{state.owner.free_04b788(s);}
        PostedOperation& project_posted(OperationStorage& s)override{return state.owner.project_posted(s);}
        OwnedResponseTask& project_owned_response(OperationStorage&,OwnedResponseContext&)override;
    };
    OwnedIoMemory& owner;const std::shared_ptr<SessionMemoryDomain> parent;
    std::shared_ptr<void> parent_lease;
    mutable std::recursive_mutex gate;IoMemoryPhase phase=IoMemoryPhase::Open;
    std::size_t active_calls=0;bool mutating=false;
    std::vector<std::unique_ptr<Record>> records;
    std::vector<std::uint64_t> generations;
    std::vector<OperationStorageBinding> storage;
    std::vector<DispatcherOperationBinding> packets;
    Response response{*this};
    Impl(OwnedIoMemory& o,std::shared_ptr<SessionMemoryDomain> p,std::size_t capacity)
        :owner(o),parent(std::move(p)),records(capacity),generations(capacity),storage(capacity),packets(capacity){parent_lease=parent->acquire_provider_lease();}
    struct Access {
        Impl& v;std::unique_lock<std::recursive_mutex> lock;
        explicit Access(Impl& s):v(s),lock(s.gate,std::defer_lock){
            (void)v.parent->buffer_context(); // parent Operation already held; never opens a nested one
            lock.lock();if(v.phase!=IoMemoryPhase::Open||v.mutating)throw std::logic_error("IO memory is closed, poisoned or reentered during an allocation effect");
        }
    };
    struct Mutation {Impl& v;explicit Mutation(Impl& s):v(s){v.mutating=true;}~Mutation(){v.mutating=false;}};
    struct Call {Impl& v;explicit Call(Impl& s):v(s){++v.active_calls;}~Call(){--v.active_calls;}};
    std::size_t index(const OperationStorage& s)const{
        for(std::size_t i=0;i<records.size();++i)if(records[i]&&&records[i]->storage==&s){
            const auto& r=*records[i];
            if(s.identity!=r.raw||s.bytes.data()!=reinterpret_cast<std::byte*>(r.raw)||s.bytes.size()!=r.extent)throw std::invalid_argument("Owned operation descriptor was altered");
            return i;
        }
        throw std::invalid_argument("Operation storage descriptor is not owned/live");
    }
    Record& get(IoMemoryHandle h){
        if(h.slot>=records.size()||!records[h.slot]||records[h.slot]->generation!=h.generation||records[h.slot]->failed)throw std::invalid_argument("Stale or failed IO projection handle");
        return *records[h.slot];
    }
    void clear_binding(std::size_t i)noexcept{packets[i]={};}
    void next_projection(std::size_t i,std::uint64_t minimum,IoMemoryKind kind){
        auto& r=*records[i];
        if(r.failed||r.pending||r.owner_pin||r.buffer_pin||r.extent<minimum)throw std::logic_error("IO projection still borrowed, failed or too small");
        if(generations[i]==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("IO projection generation exhausted");
        clear_binding(i);r.projection.emplace<std::monostate>();
        r.generation=++generations[i];r.kind=kind;r.completed=false;r.cancel=false;r.unknown=false;
        r.submission_state=IoSubmission::NotSubmitted;r.submitted_socket=invalid_socket;
        r.submitted_owner=nullptr;r.submitted_buffer=0;r.submitted_bytes=0;
    }
    void packet(std::size_t i,CancelOperation& operation){packets[i]={&operation,{},0,&operation.overlapped};}
    IoMemoryHandle arm(OVERLAPPED& overlapped,const Buffer& buffer,IoMemoryKind expected,Socket socket,bool typed){
        std::size_t slot=records.size();
        for(std::size_t i=0;i<packets.size();++i)if(packets[i].overlapped_identity==&overlapped&&packets[i].operation){slot=i;break;}
        if(slot==records.size())throw std::invalid_argument("SDK OVERLAPPED has no current owned projection");
        auto& r=*records[slot];
        if(r.kind!=expected||r.pending||r.completed||r.failed)throw std::logic_error("IO submission has no fresh matching projection");
        if(r.submission==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("IO submission sequence exhausted");
        auto& native=expected==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r.projection).io.operation:std::get<AsyncSendOperation>(r.projection).io.operation;
        if(&native.overlapped!=&overlapped||packets[slot].operation!=&native)throw std::invalid_argument("SDK binding must name the actual typed OVERLAPPED");
        auto* control=expected==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r.projection).io.handler.control_a8:std::get<AsyncSendOperation>(r.projection).io.handler.control_98;
        if(!parent->owns_control(control))throw std::invalid_argument("SDK operation owner must be a live control from the same session domain");
        auto pin=parent->pin_buffer(reinterpret_cast<std::uintptr_t>(buffer.buf),buffer.len);
        retain(*control);r.owner_pin=control;r.buffer_pin=std::move(pin);r.pending=true;
        ++r.submission;r.submission_state=typed?IoSubmission::Issuing:IoSubmission::Unclassified;
        r.submitted_socket=socket;r.submitted_owner=control;r.submitted_buffer=reinterpret_cast<std::uintptr_t>(buffer.buf);r.submitted_bytes=buffer.len;
        return {slot,r.generation};
    }
    IoLoan describe(std::size_t i,bool strict=true){
        auto& r=*records[i];CancelOperation* actual=nullptr;
        if(r.kind==IoMemoryKind::Receive)actual=&std::get<AsyncReceiveOperation>(r.projection).io.operation;
        else if(r.kind==IoMemoryKind::Send)actual=&std::get<AsyncSendOperation>(r.projection).io.operation;
        else if(r.kind==IoMemoryKind::Posted)actual=&std::get<PostedOperation>(r.projection).operation;
        else throw std::logic_error("IO loan has no private packet projection");
        const auto& binding=packets[i];
        const bool valid=(binding.operation==actual&&binding.overlapped_identity==&actual->overlapped)||(r.completed&&!binding.operation&&!binding.overlapped_identity);
        if(strict&&!valid){phase=IoMemoryPhase::Poisoned;throw std::logic_error("Public IO binding differs from the authoritative private projection");}
        return {{i,r.generation},r.kind,r.submission_state,r.submission,r.submitted_socket,
            &actual->overlapped,actual,r.submitted_owner,r.submitted_buffer,r.submitted_bytes,r.pending,r.completed,valid,binding.operation,binding.overlapped_identity};
    }
    void observed(std::size_t i){
        auto& r=*records[i];if(r.completed||r.unknown)throw std::logic_error("Completion already consumed or submission outcome unknown");
        if(r.pending){
            auto* native=r.kind==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r.projection).io.handler.control_a8:std::get<AsyncSendOperation>(r.projection).io.handler.control_98;
            if(!r.owner_pin||native!=r.owner_pin){phase=IoMemoryPhase::Poisoned;throw std::logic_error("Pending source handler owner changed");}
            auto count=r.owner_pin->strong_08.load();
            do{if(count<2){phase=IoMemoryPhase::Poisoned;throw std::logic_error("Pending source owner strong was consumed before terminal completion");}}
            while(!r.owner_pin->strong_08.compare_exchange_weak(count,count-1));
            r.owner_pin=nullptr;r.buffer_pin.reset();r.pending=false;
        }
        r.completed=true;r.cancel=false;clear_binding(i);
    }
};
OwnedIoMemory::OwnedIoMemory(std::shared_ptr<SessionMemoryDomain> parent,std::size_t capacity){
    if(!parent||!capacity||capacity>65536)throw std::invalid_argument("Owned IO memory requires parent and bounded positive capacity");
    impl_=std::make_unique<Impl>(*this,std::move(parent),capacity);
}
OwnedIoMemory::~OwnedIoMemory()noexcept{if(impl_->phase!=IoMemoryPhase::Closed||impl_->active_calls)std::terminate();}
OperationStorage& OwnedIoMemory::allocate_04b0bc(std::uint64_t bytes){
    auto& v=*impl_;Impl::Access access(v);Impl::Mutation mutation(v);
    if(!bytes)throw std::invalid_argument("IO storage needs its marker byte");
    const auto it=std::find_if(v.records.begin(),v.records.end(),[](const auto& r){return !r;});
    if(it==v.records.end())throw std::bad_alloc();const auto i=static_cast<std::size_t>(it-v.records.begin());
    if(v.generations[i]==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("IO generation exhausted");
    auto record=std::make_unique<Impl::Record>();record->extent=bytes;record->generation=++v.generations[i];*it=std::move(record);
    try{
        auto& context=v.parent->buffer_context();const auto raw=context.api.allocate_04b0bc(bytes);
        auto& r=**it;r.raw=raw;r.storage.identity=raw;r.storage.bytes={reinterpret_cast<std::byte*>(raw),static_cast<std::size_t>(bytes)};
        v.storage[i]={raw,&r.storage};return r.storage;
    }catch(...){(*it)->failed=true;v.phase=IoMemoryPhase::Poisoned;throw;}
}
void OwnedIoMemory::free_04b788(OperationStorage& storage){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage);auto& r=*v.records[i];
    if(r.pending||r.owner_pin||r.buffer_pin)throw std::logic_error("SDK borrower has not produced terminal completion");
    Impl::Mutation mutation(v);const auto raw=r.raw,extent=r.extent;
    v.clear_binding(i);v.storage[i]={};r.projection.emplace<std::monostate>();r.kind=IoMemoryKind::None;
    try{v.parent->free_sized_04b0f8(raw,extent);v.records[i].reset();}
    catch(...){r.failed=true;v.phase=IoMemoryPhase::Poisoned;throw;}
}
void OwnedIoMemory::free_cached_identity(std::uintptr_t identity){
    auto& v=*impl_;Impl::Access access(v);
    for(auto& r:v.records)if(r&&r->raw==identity){free_04b788(r->storage);return;}
    throw std::invalid_argument("Worker cache identity is not owned/live");
}
PostedOperation& OwnedIoMemory::project_posted(OperationStorage& s){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(s);v.next_projection(i,0x49,IoMemoryKind::Posted);
    auto& r=*v.records[i];auto& p=r.projection.emplace<PostedOperation>(PostedOperation{s,{},0,nullptr});v.packet(i,p.operation);return p;
}
AsyncReceiveOperation& OwnedIoMemory::project_async_receive(OperationStorage& s){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(s);v.next_projection(i,0xe1,IoMemoryKind::Receive);
    auto& p=v.records[i]->projection.emplace<AsyncReceiveOperation>(s);v.packet(i,p.io.operation);p.packet=&v.packets[i];return p;
}
AsyncSendOperation& OwnedIoMemory::project_async_send(OperationStorage& s){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(s);v.next_projection(i,0xd1,IoMemoryKind::Send);
    auto& p=v.records[i]->projection.emplace<AsyncSendOperation>(s);v.packet(i,p.io.operation);p.packet=&v.packets[i];return p;
}
ReceiveHandlerTask& OwnedIoMemory::project_receive_task(OperationStorage& s,HandlerTaskContext& ctx){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(s);v.next_projection(i,0x79,IoMemoryKind::ReceiveTask);return v.records[i]->projection.emplace<ReceiveHandlerTask>(s,ctx);
}
SendHandlerTask& OwnedIoMemory::project_send_task(OperationStorage& s,HandlerTaskContext& ctx){
    auto& v=*impl_;Impl::Access access(v);const auto i=v.index(s);v.next_projection(i,0x71,IoMemoryKind::SendTask);return v.records[i]->projection.emplace<SendHandlerTask>(s,ctx);
}
OwnedResponseTask& OwnedIoMemory::Impl::Response::project_owned_response(OperationStorage& s,OwnedResponseContext& ctx){
    Access access(state);const auto i=state.index(s);state.next_projection(i,0x49,IoMemoryKind::Response);return state.records[i]->projection.emplace<OwnedResponseTask>(s,ctx);
}
OwnedResponseMemory& OwnedIoMemory::response_memory(){return impl_->response;}
std::span<OperationStorageBinding> OwnedIoMemory::storage_bindings(){return impl_->storage;}
std::span<DispatcherOperationBinding> OwnedIoMemory::dispatcher_bindings(){return impl_->packets;}
IoMemoryHandle OwnedIoMemory::handle_for(const OperationStorage& storage)const{auto& v=*impl_;Impl::Access access(v);const auto i=v.index(storage);return {i,v.records[i]->generation};}
IoMemoryHandle OwnedIoMemory::handle_for(const OVERLAPPED* raw)const{
    auto& v=*impl_;Impl::Access access(v);for(std::size_t i=0;i<v.packets.size();++i)if(v.packets[i].operation&&v.packets[i].overlapped_identity==raw)return {i,v.records[i]->generation};
    throw std::invalid_argument("OVERLAPPED binding is inactive/stale");
}
IoMemoryHandle OwnedIoMemory::handle_for(const CancelOperation* op)const{
    auto& v=*impl_;Impl::Access access(v);for(std::size_t i=0;i<v.packets.size();++i)if(v.packets[i].operation==op&&op)return {i,v.records[i]->generation};
    throw std::invalid_argument("Operation binding is inactive/stale");
}
IoMemoryKind OwnedIoMemory::kind(IoMemoryHandle h)const{auto& v=*impl_;Impl::Access access(v);return v.get(h).kind;}
AsyncReceiveOperation& OwnedIoMemory::receive_operation(IoMemoryHandle h){auto& v=*impl_;Impl::Access access(v);return std::get<AsyncReceiveOperation>(v.get(h).projection);}
AsyncSendOperation& OwnedIoMemory::send_operation(IoMemoryHandle h){auto& v=*impl_;Impl::Access access(v);return std::get<AsyncSendOperation>(v.get(h).projection);}
IoMemoryHandle OwnedIoMemory::arm_receive(OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(ov,buffer,IoMemoryKind::Receive,invalid_socket,false);}
IoMemoryHandle OwnedIoMemory::arm_send(OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(ov,buffer,IoMemoryKind::Send,invalid_socket,false);}
IoMemoryHandle OwnedIoMemory::arm_receive(Socket socket,OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(ov,buffer,IoMemoryKind::Receive,socket,true);}
IoMemoryHandle OwnedIoMemory::arm_send(Socket socket,OVERLAPPED& ov,const Buffer& buffer){auto& v=*impl_;Impl::Access access(v);return v.arm(ov,buffer,IoMemoryKind::Send,socket,true);}
void OwnedIoMemory::note_submission(IoMemoryHandle h,std::int32_t result,std::uint32_t error){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);
    if(!r.pending||r.completed||r.submission_state!=IoSubmission::Issuing||(r.kind!=IoMemoryKind::Receive&&r.kind!=IoMemoryKind::Send))
        throw std::logic_error("IO SDK result no longer matches its issuing loan");
    auto* const control=r.kind==IoMemoryKind::Receive?std::get<AsyncReceiveOperation>(r.projection).io.handler.control_a8:std::get<AsyncSendOperation>(r.projection).io.handler.control_98;
    if(control!=r.submitted_owner||r.owner_pin!=r.submitted_owner)throw std::logic_error("IO SDK changed the captured source owner");
    (void)v.describe(h.slot); // public binding cannot replace the captured OV
    r.submission_state=(result==0||error==997)?IoSubmission::Kernel:IoSubmission::Software;
}
IoLoan OwnedIoMemory::loan(IoMemoryHandle h)const{auto& v=*impl_;Impl::Access access(v);(void)v.get(h);return v.describe(h.slot);}
std::vector<IoLoan> OwnedIoMemory::pending_loans()const{
    auto& v=*impl_;Impl::Access access(v);std::vector<IoLoan> out;out.reserve(v.records.size());
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]&&v.records[i]->pending)out.push_back(v.describe(i));return out;
}
std::vector<IoLoan> OwnedIoMemory::active_loans()const{
    auto& v=*impl_;Impl::Access access(v);std::vector<IoLoan> out;out.reserve(v.records.size());
    for(std::size_t i=0;i<v.records.size();++i)if(v.records[i]){
        const auto& r=*v.records[i];
        if(!r.completed&&(r.kind==IoMemoryKind::Receive||r.kind==IoMemoryKind::Send||r.kind==IoMemoryKind::Posted))out.push_back(v.describe(i,r.kind!=IoMemoryKind::Posted));
    }
    return out;
}
void OwnedIoMemory::note_cancel_requested(IoMemoryHandle h){auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);if(!r.pending)throw std::logic_error("Cancellation requires a pending submission");r.cancel=true;}
void OwnedIoMemory::note_unknown_submission(IoMemoryHandle h)noexcept{
    auto& v=*impl_;std::lock_guard lock(v.gate);v.phase=IoMemoryPhase::Poisoned;
    if(h.slot<v.records.size()&&v.records[h.slot]&&v.records[h.slot]->generation==h.generation){v.records[h.slot]->unknown=true;v.records[h.slot]->submission_state=IoSubmission::Unknown;}
}
void OwnedIoMemory::complete_receive(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects){
    complete_receive_with_memory(h,owner,error,bytes,env,api,errors,effects,*this);
}
void OwnedIoMemory::complete_receive_with_memory(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);auto* operation=&std::get<AsyncReceiveOperation>(r.projection).io;
    const auto local_error=error;Impl::Call call(v);v.observed(h.slot);
    try{complete_receive_027710(owner,*operation,local_error,bytes,env,api,memory,errors,effects);}catch(...){v.phase=IoMemoryPhase::Poisoned;throw;}
    // No old record/projection access: callback may have freed/reused this slot.
}
void OwnedIoMemory::complete_send(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects){
    complete_send_with_memory(h,owner,error,bytes,env,api,errors,effects,*this);
}
void OwnedIoMemory::complete_send_with_memory(IoMemoryHandle h,std::uintptr_t owner,const Error& error,std::uint64_t bytes,IoCompletionEnvironment env,WorkerRunApi& api,Api& errors,IoCompletionEffects& effects,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);auto* operation=&std::get<AsyncSendOperation>(r.projection).io;
    const auto local_error=error;Impl::Call call(v);v.observed(h.slot);
    try{complete_send_027a20(owner,*operation,local_error,bytes,env,api,memory,errors,effects);}catch(...){v.phase=IoMemoryPhase::Poisoned;throw;}
}
void OwnedIoMemory::complete_posted(IoMemoryHandle h,std::uintptr_t owner,PostedEnvironment env,WorkerRunApi& api){
    complete_posted_with_memory(h,owner,env,api,*this);
}
void OwnedIoMemory::complete_posted_with_memory(IoMemoryHandle h,std::uintptr_t owner,PostedEnvironment env,WorkerRunApi& api,PostedMemoryEffects& memory){
    auto& v=*impl_;Impl::Access access(v);auto* operation=&std::get<PostedOperation>(v.get(h).projection);Impl::Call call(v);v.observed(h.slot);
    try{complete_posted_027120(owner,*operation,env,api,memory);}catch(...){v.phase=IoMemoryPhase::Poisoned;throw;}
}
void OwnedIoMemory::invoke_task(IoMemoryHandle h,std::uint8_t flag){
    auto& v=*impl_;Impl::Access access(v);auto& r=v.get(h);Impl::Call call(v);
    if(r.kind!=IoMemoryKind::ReceiveTask&&r.kind!=IoMemoryKind::SendTask&&r.kind!=IoMemoryKind::Response)throw std::invalid_argument("IO handle is not an owned task");
    if(r.completed)throw std::logic_error("Owned task already consumed");r.completed=true;
    try{
        if(r.kind==IoMemoryKind::ReceiveTask)std::get<ReceiveHandlerTask>(r.projection).call_slot_00(flag);
        else if(r.kind==IoMemoryKind::SendTask)std::get<SendHandlerTask>(r.projection).call_slot_00(flag);
        else if(r.kind==IoMemoryKind::Response)std::get<OwnedResponseTask>(r.projection).call_slot_00(flag);
        else throw std::invalid_argument("IO handle is not an owned task");
    }catch(...){v.phase=IoMemoryPhase::Poisoned;throw;}
}
IoMemorySnapshot OwnedIoMemory::snapshot()const{
    auto& v=*impl_;std::lock_guard lock(v.gate);IoMemorySnapshot out;out.phase=v.phase;out.active_calls=v.active_calls;
    for(const auto& r:v.records)if(r){++out.allocations;if(r->pending)++out.pending;if(r->cancel)++out.cancel_requested;}return out;
}
bool OwnedIoMemory::close(){
    auto& v=*impl_;std::shared_ptr<void> retiring;
    {std::lock_guard lock(v.gate);if(v.phase==IoMemoryPhase::Closed)return true;
     if(v.phase!=IoMemoryPhase::Open||v.mutating||v.active_calls||std::any_of(v.records.begin(),v.records.end(),[](const auto& p){return bool(p);}))return false;
     v.phase=IoMemoryPhase::Retiring;retiring=std::move(v.parent_lease);}
    retiring.reset();{std::lock_guard lock(v.gate);v.phase=IoMemoryPhase::Closed;}return true;
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
