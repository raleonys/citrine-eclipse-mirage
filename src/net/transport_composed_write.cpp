// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_composed_write.hpp"
#include <algorithm>

namespace citrine_eclipse_mirage::socket {
namespace {
IocpSchedulerEntry& scheduler(AsyncSendService& service){
    if(!service.scheduler_08)throw UnsupportedInput("Native send scheduler has no live projection");
    return *service.scheduler_08;
}
void release_temporary_weak(IoReference* control){
    if(control && control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
}
void append_failed(CompletionScheduler& queue,CancelOperation& operation){
    queue.gate.enter();
    operation.next=nullptr;
    if(queue.tail)queue.tail->next=&operation;else queue.head=&operation;
    queue.tail=&operation;
    (void)queue.dispatch_required.exchange(1,std::memory_order_seq_cst);
    queue.gate.leave();
}
DispatcherOperationBinding& packet(AsyncSendOperation& operation){
    if(!operation.packet)throw UnsupportedInput("Async send requires a live dispatcher packet binding");
    return *operation.packet;
}
SendStreamBinding& stream_at(std::uintptr_t identity,ComposedWriteContext& context){
    for(auto& binding:context.streams)if(binding.identity==identity)return binding;
    throw UnsupportedInput("Composed write stream has no typed member binding");
}
}
OperationStorage& allocate_io_storage_0269b0(std::uint64_t requested,PostedEnvironment environment,
    WorkerRunApi& api,PostedMemoryEffects& memory){
    auto* const tls=static_cast<WorkerTlsContext*>(api.tls_get(environment.tls_index));
    // Native's uninitialized RCX tag is unused by021820. Host0 is an ignored
    // argument, not a claim about the original scratch byte.
    return allocate_cached_021820(0,tls?tls->state:nullptr,requested,1,environment,memory);
}
void publish_send_status_028b50(IocpSchedulerEntry& entry,CancelOperation& operation,
    DispatcherOperationBinding& metadata,std::uint32_t status,std::uint32_t bytes,DispatcherApi& api,Api& errors){
    operation.queued=1;
    const auto category=errors.system_category_028620();
    metadata.packet_error={static_cast<std::int32_t>(status),category};metadata.transferred_14=bytes;
    if(api.post(entry.completion.port,0,2,&operation.overlapped)!=0)return;
    append_failed(entry.completion,operation);
}
void publish_send_started_028c00(IocpSchedulerEntry& entry,CancelOperation& operation,DispatcherApi& api){
    std::uint32_t previous=0;
    {
        std::atomic_ref<std::uint32_t> state(operation.queued);
        (void)state.compare_exchange_strong(previous,1,std::memory_order_seq_cst,std::memory_order_seq_cst);
    }
    if(previous!=1)return;
    if(api.post(entry.completion.port,0,2,&operation.overlapped)!=0)return;
    append_failed(entry.completion,operation);
}
void start_async_send_021fd0(AsyncSendService& service,SendSocketImplementation& implementation,
    SendBufferSlice& buffer,std::uint32_t flags,SendHandler& source,std::uintptr_t allocator_identity,ComposedWriteContext& context){
    auto& storage=allocate_io_storage_0269b0(0xd0,context.environment.storage,context.worker,context.memory);
    auto& operation=context.effects.project_async_send(storage);
    auto* const weak=implementation.weak_control_18;
    std::uintptr_t weak_object=0;
    if(weak){weak_object=implementation.weak_object_10;weak->weak_0c.fetch_add(1,std::memory_order_seq_cst);}
    operation.io.operation.next=nullptr;operation.callback_identity_28=0x180027a20ULL;
    operation.io.operation.overlapped={};
    if(storage.bytes.size()<sizeof(OVERLAPPED))throw UnsupportedInput("Send OVERLAPPED leaves bounded allocation");
    std::fill_n(storage.bytes.begin(),sizeof(OVERLAPPED),std::byte{});
    packet(operation).packet_error={0,{}};packet(operation).transferred_14=0;
    operation.io.operation.queued=0;operation.weak_object_38=0;operation.io.weak_40=nullptr;
    if(weak){
        operation.weak_object_38=weak_object;operation.io.weak_40=weak;
        weak->weak_0c.fetch_add(1,std::memory_order_seq_cst);
    }
    operation.buffer_48=buffer;
    operation.io.handler.words_60_78=source.words_60_78;
    operation.io.handler.dword_80=source.dword_80;operation.io.handler.word_88=source.word_88;
    operation.io.handler.object_90=0;operation.io.handler.control_98=nullptr;
    operation.io.handler.object_90=source.object_90;operation.io.handler.control_98=source.control_98;
    source.object_90=0;source.control_98=nullptr;
    context.effects.construct_environment_023dd0(operation.io.environment_a0,allocator_identity);
    release_temporary_weak(weak);
    // Native reloads the full buffer AFTER environment construction/ref release.
    const auto live_buffer=buffer;
    Buffer sdk_buffer{static_cast<ULONG>(live_buffer.size),reinterpret_cast<char*>(live_buffer.data)};
    if((implementation.state_08&0x10)!=0 && live_buffer.size==0){
        scheduler(service).work_count_30.fetch_add(1,std::memory_order_seq_cst);
        publish_send_status_028b50(scheduler(service),operation.io.operation,packet(operation),0,0,context.completion,context.errors);
    }else{
        scheduler(service).work_count_30.fetch_add(1,std::memory_order_seq_cst);
        const auto socket=implementation.socket_00;
        if(socket==invalid_socket)
            publish_send_status_028b50(scheduler(service),operation.io.operation,packet(operation),0x2719,0,context.completion,context.errors);
        else{
            const auto result=context.send.send(socket,sdk_buffer,flags,operation.io.operation.overlapped);
            const auto status=result.last_error==0x4d2?0x274d:result.last_error;
            if(result.result==0 || status==0x3e5)
                publish_send_started_028c00(scheduler(service),operation.io.operation,context.completion);
            else
                publish_send_status_028b50(scheduler(service),operation.io.operation,packet(operation),status,result.transferred,context.completion,context.errors);
        }
    }
    IoCleanupFrame<SendIoOperation> frame{};
    cleanup_send_0294c0(frame,context.environment,context.worker,context.memory); // native null tail cleanup
}
void composed_write_0261d0(SendHandler& handler,const Error& error,std::uint64_t bytes,std::uint32_t flag,
    ComposedWriteContext& context){
    handler.dword_80=flag;
    const auto code=error.value;
    std::uint64_t maximum=0x10000;
    if(flag!=1){
        handler.words_60_78[3]+=bytes; // unsigned64 wrap before completion decision
        if((code==0 && bytes==0) || handler.words_60_78[3]>=handler.words_60_78[2] || code!=0){
            auto copy=error;
            context.effects.finish_write_025a70(handler,copy);
            return;
        }
    }else if(code!=0)maximum=0;
    const auto length=handler.words_60_78[2];
    const auto offset=std::min(handler.words_60_78[3],length);
    SendBufferSlice buffer{handler.words_60_78[1]+offset,std::min(maximum,length-offset)};
    auto& stream=stream_at(handler.words_60_78[0],context);
    if(!stream.service || !stream.implementation)throw UnsupportedInput("Native composed-write stream members have no live projection");
    start_async_send_021fd0(*stream.service,*stream.implementation,buffer,0,handler,stream.allocator_identity_68,context);
}
void ComposedWriteEffectsAdapter::send_body_0261d0(SendHandler& handler,Error& error,std::uint64_t bytes,std::uint32_t flag){
    composed_write_0261d0(handler,error,bytes,flag,context_);
}
} // namespace citrine_eclipse_mirage::socket
