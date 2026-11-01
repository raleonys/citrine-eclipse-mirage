#include "async_receive.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
namespace {
IocpSchedulerEntry& receive_scheduler(AsyncReceiveService& service){
    if(!service.scheduler_08)throw UnsupportedInput("Native receive scheduler has no live projection");
    return *service.scheduler_08;
}
DispatcherOperationBinding& receive_packet(AsyncReceiveOperation& operation){
    if(!operation.packet)throw UnsupportedInput("Async receive requires a live dispatcher packet binding");
    return *operation.packet;
}
}
AsyncReceiveResult NativeAsyncReceiveApi::receive(Socket socket,Buffer& buffer,std::uint32_t flags,OVERLAPPED& overlapped){
    if(!imports_.receive || !imports_.get_error)throw UnsupportedInput("Async receive imports require both SDK functions");
    DWORD transferred=0,private_flags=flags;
    const auto result=imports_.receive(static_cast<SOCKET>(socket),&buffer,1,&transferred,&private_flags,&overlapped,nullptr);
    const auto error=imports_.get_error();
    return {result,static_cast<std::uint32_t>(error),static_cast<std::uint32_t>(transferred)};
}
void start_async_receive_021d10(AsyncReceiveService& service,ReceiveSocketImplementation& implementation,
    ReceiveBufferSlice& buffer,std::uint32_t flags,ReceiveHandler& source,std::uintptr_t allocator_identity,
    AsyncReceiveContext& context){
    auto& storage=allocate_io_storage_0269b0(0xe0,context.environment.storage,context.worker,context.memory);
    auto& operation=context.effects.project_async_receive(storage);
    auto* const weak=implementation.weak_control_18;
    std::uintptr_t weak_object=0;
    if(weak){weak_object=implementation.weak_object_10;weak->weak_0c.fetch_add(1,std::memory_order_seq_cst);}
    operation.io.operation.next=nullptr;operation.callback_identity_28=0x180027710ULL;
    operation.io.operation.overlapped={};
    if(storage.bytes.size()<sizeof(OVERLAPPED))throw UnsupportedInput("Receive OVERLAPPED leaves bounded allocation");
    std::fill_n(storage.bytes.begin(),sizeof(OVERLAPPED),std::byte{});
    receive_packet(operation).packet_error={0,{}};receive_packet(operation).transferred_14=0;
    operation.io.operation.queued=0;operation.io.state_38=implementation.state_08;
    operation.weak_object_40=0;operation.io.weak_48=nullptr;
    if(weak){
        operation.weak_object_40=weak_object;operation.io.weak_48=weak;
        weak->weak_0c.fetch_add(1,std::memory_order_seq_cst);
    }
    operation.buffer_data_50=buffer.data;operation.io.requested_58=buffer.size;
    operation.io.handler.words_60_78=source.words_60_78;
    operation.io.handler.byte_80=source.byte_80;operation.io.handler.dword_84=source.dword_84;
    operation.io.handler.words_88_98=source.words_88_98;
    operation.io.handler.object_a0=0;operation.io.handler.control_a8=nullptr;
    operation.io.handler.object_a0=source.object_a0;operation.io.handler.control_a8=source.control_a8;
    source.object_a0=0;source.control_a8=nullptr;
    context.effects.construct_environment_023dd0(operation.io.environment_b0,allocator_identity);
    if(weak && weak->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)weak->delete_slot_08();
    // Native reloads buffer and socket state after construction and weak release.
    const auto live_buffer=buffer;
    Buffer sdk_buffer{static_cast<ULONG>(live_buffer.size),reinterpret_cast<char*>(live_buffer.data)};
    if((implementation.state_08&0x10)!=0 && live_buffer.size==0){
        receive_scheduler(service).work_count_30.fetch_add(1,std::memory_order_seq_cst);
        publish_send_status_028b50(receive_scheduler(service),operation.io.operation,receive_packet(operation),0,0,context.completion,context.errors);
    }else{
        receive_scheduler(service).work_count_30.fetch_add(1,std::memory_order_seq_cst);
        const auto socket=implementation.socket_00;
        if(socket==invalid_socket)
            publish_send_status_028b50(receive_scheduler(service),operation.io.operation,receive_packet(operation),0x2719,0,context.completion,context.errors);
        else{
            const auto result=context.receive.receive(socket,sdk_buffer,flags,operation.io.operation.overlapped);
            const auto status=result.last_error==0x40?0x2746:result.last_error==0x4d2?0x274d:result.last_error;
            if(result.result==0 || status==0x3e5)
                publish_send_started_028c00(receive_scheduler(service),operation.io.operation,context.completion);
            else
                publish_send_status_028b50(receive_scheduler(service),operation.io.operation,receive_packet(operation),status,result.transferred,context.completion,context.errors);
        }
    }
    // Native local cleanup has null operation/allocation after publication.
    IoCleanupFrame<ReceiveIoOperation> frame{};
    cleanup_receive_0293f0(frame,context.environment,context.worker,context.memory);
}
void RecoveredAsyncReceiveEffects::construct_environment_023dd0(IoHandlerEnvironment& destination,std::uintptr_t allocator_identity){
    IoHandlerEnvironment* source=nullptr;bool matched=false;
    for(const auto& binding:sources_){
        if(binding.allocator_identity!=allocator_identity)continue;
        if(matched)throw UnsupportedInput("Receive environment source identity is ambiguous");
        matched=true;source=binding.source;
    }
    if(!source)throw UnsupportedInput("Receive environment source identity has no live projection");
    construct_io_environment_023dd0(destination,*source,sentinels_,constructor_);
}
} // namespace anadius::socket_recovered
