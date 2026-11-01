#include "helpers.hpp"
#include <bit>

namespace anadius::socket_recovered {
namespace {
IocpSchedulerEntry& live_scheduler(RetryEngine& engine){
    if(!engine.scheduler)throw UnsupportedInput("Retry requires a live scheduler projection");
    return *engine.scheduler;
}
void copied_failure(IocpSchedulerEntry& entry,CancelOperation& op,DispatcherOperationBinding& packet,
    Error error,DispatcherApi& api){
    op.queued=1;packet.packet_error=error;packet.transferred_14=0;
    if(api.post(entry.completion.port,0,2,&op.overlapped))return;
    // Native caches the scheduler through PostQueuedCompletionStatus and its
    // fallback, even if a lower callback changes engine.scheduler.
    auto& queue=entry.completion;queue.gate.enter();op.next=nullptr;
    if(queue.tail)queue.tail->next=&op;else queue.head=&op;
    queue.tail=&op;queue.dispatch_required.exchange(1);queue.gate.leave();
}
void require_buffer(std::span<std::byte> bytes,std::uint32_t n){
    if(static_cast<std::uint64_t>(n)*2>bytes.size())throw UnsupportedInput("Retry address ranges leave supplied storage");
}
}
Socket NativeSocketFactoryApi::open(std::int32_t family,std::int32_t type,std::int32_t protocol){
    if(!imports_.open)throw UnsupportedInput("WSASocketW import is unbound");
    return imports_.open(family,type,protocol,nullptr,0,1);
}
AcceptExResult NativeRetryAcceptApi::accept(Socket listener,Socket accepted,std::span<std::byte> bytes,
    std::uint32_t n,OVERLAPPED& op){
    require_buffer(bytes,n);
    if(!imports_.accept||!imports_.error)throw UnsupportedInput("Retry AcceptEx imports are unbound");
    DWORD transferred=0;const auto result=imports_.accept(listener,accepted,bytes.data(),0,n,n,&transferred,&op);
    const auto error=imports_.error();return {result,static_cast<std::uint32_t>(error)};
}
std::int32_t NativeRetryCancelApi::cancel(Socket listener,OVERLAPPED& op){
    if(!call_)throw UnsupportedInput("CancelIoEx import is unbound");
    return call_(reinterpret_cast<HANDLE>(listener),&op);
}
Socket create_socket_029df0(std::int32_t family,std::int32_t type,std::int32_t protocol,
    Error& error,SocketFactoryApi& factory,AcceptorApi& sockets,Api& errors){
    const auto socket=factory.open(family,type,protocol);
    if(socket!=invalid_socket){
        error.value=0; // Only the native low32 field is cleared, before option.
        if(family==23)(void)sockets.set_option(socket,41,27,0);
        return socket;
    }
    const auto category=errors.system_category_028620();
    const auto code=errors.get_last_error();error={code,category};return invalid_socket;
}
void reset_socket_holder_029590(Socket& holder,AcceptorApi& sockets,Api& errors){
    const auto socket=holder;if(socket==invalid_socket)return;
    if(sockets.close_socket(socket)!=0){
        const auto category=errors.system_category_028620();
        const auto code=errors.get_last_error();
        const auto first=errors.system_category_028620();
        if((category.name_identity==first.name_identity&&code==0x2733)||
            (category.name_identity==errors.system_category_028620().name_identity&&code==0x4d5)){
            std::uint32_t zero=0;(void)sockets.set_nonblocking(socket,0x8004667eU,zero);
            if(sockets.close_socket(socket)!=0){(void)errors.system_category_028620();(void)errors.get_last_error();}
        }
    }
    holder=invalid_socket; // Normal completion overwrites any lower mutation.
}
void retry_accept_029660(RetryEngine& engine,Socket listener,Socket& holder,
    std::int32_t family,std::int32_t type,std::int32_t protocol,std::span<std::byte> bytes,
    std::uint32_t address_length,std::atomic<std::int32_t>* cancellation,
    CancelOperation& selected,DispatcherOperationBinding& packet,AcceptRetryContext& c){
    require_buffer(bytes,address_length);
    if(packet.operation!=&selected||packet.overlapped_identity!=&selected.overlapped)
        throw UnsupportedInput("Retry selected packet has no coherent OVERLAPPED binding");
    reset_socket_holder_029590(holder,c.sockets,c.errors);
    live_scheduler(engine).work_count_30.fetch_add(1);
    if(cancellation&&cancellation->fetch_add(0)==1)
        publish_send_status_028b50(live_scheduler(engine),selected,packet,995,0,c.completion,c.errors);
    // There is deliberately no return after early cancellation publication.
    const auto socket=c.factory.open(family,type,protocol);
    if(socket==invalid_socket){
        const auto category=c.errors.system_category_028620();
        const auto code=c.errors.get_last_error();
        reset_socket_holder_029590(holder,c.sockets,c.errors);holder=invalid_socket;
        copied_failure(live_scheduler(engine),selected,packet,{code,category},c.completion);
    }else{
        if(family==23)(void)c.sockets.set_option(socket,41,27,0);
        reset_socket_holder_029590(holder,c.sockets,c.errors);holder=socket;
        const auto result=c.accept.accept(listener,socket,bytes,address_length,selected.overlapped);
        if(!result.result&&result.last_error!=997)
            publish_send_status_028b50(live_scheduler(engine),selected,packet,result.last_error,0,c.completion,c.errors);
        else{
            if(cancellation&&cancellation->fetch_add(0)==1)(void)c.cancel.cancel(listener,selected.overlapped);
            publish_send_started_028c00(live_scheduler(engine),selected,c.completion);
        }
    }
}
} // namespace anadius::socket_recovered
