#include "coherent_io.hpp"

namespace anadius::socket_recovered {
OperationStorage& CoherentIoMemory::allocate_04b0bc(std::uint64_t n){return accept_.allocate_04b0bc(n);}
void CoherentIoMemory::free_04b788(OperationStorage& s){accept_.free_04b788(s);}
PostedOperation& CoherentIoMemory::project_posted(OperationStorage& s){return accept_.project_posted(s);}
ReceiveHandlerTask& CoherentIoMemory::project_receive_task(OperationStorage& s,HandlerTaskContext& c){accept_.retire_for_projection(s);return base_.project_receive_task(s,c);}
SendHandlerTask& CoherentIoMemory::project_send_task(OperationStorage& s,HandlerTaskContext& c){accept_.retire_for_projection(s);return base_.project_send_task(s,c);}
OwnedResponseTask& CoherentIoMemory::project_owned_response(OperationStorage& s,OwnedResponseContext& c){accept_.retire_for_projection(s);return base_.response_memory().project_owned_response(s,c);}
AsyncReceiveOperation& CoherentIoMemory::project_async_receive(OperationStorage& s){accept_.retire_for_projection(s);return base_.project_async_receive(s);}
AsyncSendOperation& CoherentIoMemory::project_async_send(OperationStorage& s){accept_.retire_for_projection(s);return base_.project_async_send(s);}
void CoherentIoMemory::free_cached_identity(std::uintptr_t id){accept_.free_cached_identity(id);}
void CoherentIoMemory::construct_environment_023dd0(IoHandlerEnvironment& destination,std::uintptr_t allocator){
    const auto require_open=[&]{
        (void)accept_.storage_bindings(); // existing admitted parent + accept phase
        if(base_.snapshot().phase!=IoMemoryPhase::Open)throw std::logic_error("Coherent base IO is closed or poisoned");
    };
    require_open();
    IoHandlerEnvironment* source=nullptr;bool found=false;
    for(const auto& b:sources_)if(b.allocator_identity==allocator){if(found)throw UnsupportedInput("Ambiguous coherent environment source");found=true;source=b.source;}
    if(!source)throw UnsupportedInput("Coherent environment source has no live binding");
    construct_io_environment_023dd0(destination,*source,sentinels_,constructor_);
    require_open(); // swallowed poison in EITHER provider cannot publish IO
}
void CoherentIoDispatcher::complete(IocpSchedulerEntry* owner,CancelOperation& op,const Error& error,std::uint32_t bytes){
    const auto who=reinterpret_cast<std::uintptr_t>(owner);
    if(accept_.contains(&op)){accept_.complete(accept_.handle_for(&op.overlapped),who,error);return;}
    const auto handle=base_.handle_for(&op);const auto kind=base_.kind(handle);
    if(kind==IoMemoryKind::Receive){base_.complete_receive_with_memory(handle,who,error,bytes,environment_,worker_,errors_,effects_,memory_.posted());return;}
    if(kind==IoMemoryKind::Send){base_.complete_send_with_memory(handle,who,error,bytes,environment_,worker_,errors_,effects_,memory_.posted());return;}
    if(kind==IoMemoryKind::Posted){base_.complete_posted_with_memory(handle,who,environment_.storage,worker_,memory_.posted());return;}
    throw UnsupportedInput("Coherent dispatcher packet is not live accept/receive/send/posted work");
}
SessionPair& DomainFlowApi::construct(SessionPair& output,SocketObject& pending){output=memory_.construct_session(pending,executor_);return output;}
ReceiveBufferString& DomainFlowApi::greeting(SessionOwner& object,ReceiveBufferString& output){return application_.greeting(object,output);}
void DomainFlowApi::start_accept(AsyncAcceptService& service,AcceptorState& listener,AcceptPending& pending,AcceptEndpoint* output,const std::uintptr_t& handler,std::uintptr_t allocator){accept_.start(service,listener,pending,output,handler,allocator);}
[[noreturn]]void DomainFlowApi::raise_write(Error& error,std::uintptr_t context){application_.raise_write(error,context);}
} // namespace anadius::socket_recovered
