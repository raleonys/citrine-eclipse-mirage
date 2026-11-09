// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_io_completion.hpp"

namespace citrine_eclipse_mirage::socket {
namespace {
IoHandlerVtable& table(IoHandlerEnvironment& environment){
    if(!environment.operations_10)throw UnsupportedInput("Native handler vtable has no live projection");
    return *environment.operations_10;
}
void release_weak(IoReference* control){
    if(control && control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
}
void release_strong(IoReference* control){
    if(control && control->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->dispose_slot_00();
        release_weak(control);
    }
}
Error map_error(const Error& input,IoReference* weak,Api& api){
    // The branch decision (including weak+8) precedes the category callback.
    if(input.value==0x40){
        const auto value=weak && weak->strong_08.load(std::memory_order_relaxed)!=0?0x2746:0x3e3;
        return {value,api.system_category_028620()};
    }
    if(input.value==0x4d2)return {0x274d,api.system_category_028620()};
    return input;
}
}
IoHandlerEnvironment& move_io_environment_023cb0(IoHandlerEnvironment& destination,
    IoHandlerEnvironment& source,IoHandlerSentinels sentinels){
    destination.operations_10=source.operations_10;
    destination.schedule_20=source.schedule_20;
    source.operations_10=sentinels.empty_operations_846c8;
    source.schedule_20=sentinels.empty_schedule_846e8;
    table(destination).move_slot_10(destination,source);
    source.state_18=0;
    destination.executor_28=source.executor_28; // live reload AFTER callback
    source.executor_28=sentinels.empty_executor_84770;
    return destination;
}
void release_io_storage_026a00(OperationStorage& storage,std::uint64_t size,
    PostedEnvironment environment,WorkerRunApi& api,PostedMemoryEffects& memory){
    // Native caches the allocation and size across the TLS getter.
    auto* const context=static_cast<WorkerTlsContext*>(api.tls_get(environment.tls_index));
    auto* const thread=context?context->state:nullptr;
    if(size<=0x3fc && thread){
        for(std::size_t index=0;index<2;++index){
            if(thread->cache_00[index])continue;
            if(size>=storage.bytes.size() || storage.bytes.empty())throw UnsupportedInput("Native IO cache marker leaves bounded allocation");
            storage.bytes[0]=storage.bytes[static_cast<std::size_t>(size)];
            thread->cache_00[index]=storage.identity;
            return;
        }
    }
    memory.free_04b788(storage);
}
void cleanup_receive_0293f0(IoCleanupFrame<ReceiveIoOperation>& frame,IoCompletionEnvironment environment,
    WorkerRunApi& api,PostedMemoryEffects& memory){
    if(auto* const operation=frame.operation_10){
        table(operation->environment_b0).destroy_slot_00(operation->environment_b0);
        release_strong(operation->handler.control_a8); // live AFTER destroy
        release_weak(operation->weak_48); // live AFTER strong dispose/delete
        frame.operation_10=nullptr;
    }
    if(auto* const allocation=frame.allocation_08){
        release_io_storage_026a00(*allocation,0xe0,environment.storage,api,memory);
        frame.allocation_08=nullptr;
    }
}
void cleanup_send_0294c0(IoCleanupFrame<SendIoOperation>& frame,IoCompletionEnvironment environment,
    WorkerRunApi& api,PostedMemoryEffects& memory){
    if(auto* const operation=frame.operation_10){
        table(operation->environment_a0).destroy_slot_00(operation->environment_a0);
        release_strong(operation->handler.control_98);
        release_weak(operation->weak_40);
        frame.operation_10=nullptr;
    }
    if(auto* const allocation=frame.allocation_08){
        release_io_storage_026a00(*allocation,0xd0,environment.storage,api,memory);
        frame.allocation_08=nullptr;
    }
}
void invoke_receive_021a40(ReceiveHandler& handler,IoCompletionEffects& effects){
    auto error=handler.error;const auto bytes=handler.transferred;
    effects.receive_body_025c40(handler,error,bytes,0);
}
void invoke_send_021a80(SendHandler& handler,IoCompletionEffects& effects){
    auto error=handler.error;const auto bytes=handler.transferred;
    effects.send_body_0261d0(handler,error,bytes,0);
}
void complete_send_027a20(std::uintptr_t owner,SendIoOperation& operation,const Error& packet_error,std::uint64_t bytes,
    IoCompletionEnvironment environment,WorkerRunApi& api,PostedMemoryEffects& memory,Api& errors,IoCompletionEffects& effects){
    const auto incoming=packet_error; // cache before move callback can mutate input
    IoCleanupFrame<SendIoOperation> frame{0,&operation.storage,&operation};
    IoHandlerEnvironment local_environment;
    move_io_environment_023cb0(local_environment,operation.environment_a0,environment.sentinels);
    auto error=map_error(incoming,operation.weak_40,errors);
    auto handler=operation.handler;
    operation.handler.object_90=0;operation.handler.control_98=nullptr;
    handler.error=error;handler.transferred=bytes;
    cleanup_send_0294c0(frame,environment,api,memory); // may end operation lifetime
    if(owner){
        if(local_environment.state_18==0)invoke_send_021a80(handler,effects);
        else effects.schedule_send_0229d0(local_environment,handler);
    }
    release_strong(handler.control_98); // live local reload after handler
    table(local_environment).destroy_slot_00(local_environment);
    cleanup_send_0294c0(frame,environment,api,memory);
}
void complete_receive_027710(std::uintptr_t owner,ReceiveIoOperation& operation,const Error& packet_error,std::uint64_t bytes,
    IoCompletionEnvironment environment,WorkerRunApi& api,PostedMemoryEffects& memory,Api& errors,IoCompletionEffects& effects){
    const auto incoming=packet_error;
    IoCleanupFrame<ReceiveIoOperation> frame{0,&operation.storage,&operation};
    IoHandlerEnvironment local_environment;
    move_io_environment_023cb0(local_environment,operation.environment_b0,environment.sentinels);
    auto error=map_error(incoming,operation.weak_48,errors);
    if(incoming.value==0x2738 || incoming.value==0xea)error.value=0;
    else if(incoming.value==0 && bytes==0 && (operation.state_38&0x10)!=0 && operation.requested_58!=0)
        error={2,effects.misc_category_0285b0()};
    auto handler=operation.handler;
    operation.handler.object_a0=0;operation.handler.control_a8=nullptr;
    handler.error=error;handler.transferred=bytes;
    cleanup_receive_0293f0(frame,environment,api,memory);
    if(owner){
        if(local_environment.state_18==0)invoke_receive_021a40(handler,effects);
        else{
            // Inline native nested executor preparation, environment move,
            // conditional temporary destroy, dispatch and unconditional destroy.
            auto* const executor=local_environment.executor_28;
            if(!executor)throw UnsupportedInput("Native receive executor has no live projection");
            const auto prepare=executor->prepare_slot_40; // cached function, not executor-this
            if(!prepare)throw UnsupportedInput("Native receive executor slot40 has no live projection");
            const auto query=table(local_environment).query_slot_18(local_environment);
            IoPrepareScratch scratch;IoHandlerEnvironment temporary;
            prepare(temporary,query,scratch);
            IoHandlerEnvironment nested;
            move_io_environment_023cb0(nested,temporary,environment.sentinels);
            if(table(temporary).destroy_identity!=environment.sentinels.guard_destroy_017a30)
                table(temporary).destroy_slot_00(temporary);
            effects.schedule_receive_022c60(nested,handler);
            table(nested).destroy_slot_00(nested);
        }
    }
    release_strong(handler.control_a8);
    table(local_environment).destroy_slot_00(local_environment);
    cleanup_receive_0293f0(frame,environment,api,memory);
}
} // namespace citrine_eclipse_mirage::socket
