#include "preloader_l/net/transport_handler_dispatch.hpp"

namespace preloader_l::socket_recovered {
namespace {
OperationStorage& lookup_storage(std::uintptr_t identity,PostedEnvironment environment){
    for(auto& binding:environment.storage)
        if(binding.identity==identity && binding.storage)return *binding.storage;
    throw UnsupportedInput("Handler cache identity has no live bounded storage");
}
std::byte read_storage(const OperationStorage& storage,std::uint64_t offset){
    if(offset>=storage.bytes.size())throw UnsupportedInput("Native handler marker read leaves bounded storage");
    return storage.bytes[static_cast<std::size_t>(offset)];
}
void write_storage(OperationStorage& storage,std::uint64_t offset,std::byte value){
    if(offset>=storage.bytes.size())throw UnsupportedInput("Native handler marker write leaves bounded storage");
    storage.bytes[static_cast<std::size_t>(offset)]=value;
}
void release_control(IoReference* control){
    if(control && control->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->dispose_slot_00();
        if(control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
    }
}
HandlerSchedule& schedule_table(IoHandlerEnvironment& environment,HandlerTaskContext& context){
    for(auto& binding:context.schedules)
        if(binding.identity==environment.schedule_20 && binding.table)return *binding.table;
    throw UnsupportedInput("Native scheduler table has no live typed binding");
}
IoHandlerVtable& environment_table(IoHandlerEnvironment& environment){
    if(!environment.operations_10)throw UnsupportedInput("Native handler environment has no live vtable");
    return *environment.operations_10;
}
void release_task_storage(OperationStorage& storage,std::uint64_t marker,HandlerTaskContext& context){
    auto* const tls=static_cast<WorkerTlsContext*>(context.api.tls_get(context.environment.storage.tls_index));
    auto* const thread=tls?tls->state:nullptr;
    if(thread){
        for(std::size_t index=4;index<6;++index){
            if(thread->cache_00[index])continue;
            write_storage(storage,0,read_storage(storage,marker));
            thread->cache_00[index]=storage.identity;
            return;
        }
    }
    context.memory.free_04b788(storage);
}
}
OperationStorage& allocate_handler_cached_021900(WorkerThreadState* thread,std::uint64_t requested,
    std::uint64_t alignment,PostedEnvironment environment,PostedMemoryEffects& memory){
    const auto units=(requested+std::uint64_t{3})>>2;
    if(thread){
        for(std::size_t index=4;index<6;++index){
            const auto identity=thread->cache_00[index];
            if(!identity)continue;
            auto& storage=lookup_storage(identity,environment);
            if(units>std::to_integer<std::uint8_t>(read_storage(storage,0)))continue;
            if(alignment==0)throw UnsupportedInput("Native handler cache alignment DIV has zero divisor");
            if(identity%alignment!=0)continue;
            thread->cache_00[index]=0;
            write_storage(storage,requested,read_storage(storage,0));
            return storage;
        }
        for(std::size_t index=4;index<6;++index){
            const auto identity=thread->cache_00[index];
            if(!identity)continue;
            thread->cache_00[index]=0;
            memory.free_04b788(lookup_storage(identity,environment));
            break;
        }
    }
    auto& storage=memory.allocate_04b0bc(units*4+1);
    write_storage(storage,requested,static_cast<std::byte>(units>255?0:static_cast<std::uint8_t>(units)));
    return storage;
}
void invoke_borrowed_receive_022640(ReceiveHandler& handler,IoCompletionEffects& effects){
    auto error=handler.error;const auto bytes=handler.transferred;
    effects.receive_body_025c40(handler,error,bytes,0);
}
void invoke_borrowed_send_022780(SendHandler& handler,IoCompletionEffects& effects){
    auto error=handler.error;const auto bytes=handler.transferred;
    effects.send_body_0261d0(handler,error,bytes,0);
}
void BorrowedIoHandler::invoke(IoCompletionEffects& effects) const{
    if(receive && !send && invoke_identity==0x180022640ULL){invoke_borrowed_receive_022640(*receive,effects);return;}
    if(send && !receive && invoke_identity==0x180022780ULL){invoke_borrowed_send_022780(*send,effects);return;}
    throw UnsupportedInput("Borrowed handler descriptor has no selected native callback projection");
}
void cleanup_receive_task_0291c0(HandlerTaskCleanup<ReceiveHandlerTask>& frame,HandlerTaskContext& context){
    if(auto* const task=frame.task_10){release_control(task->handler.control_a8);frame.task_10=nullptr;}
    if(auto* const allocation=frame.allocation_08){
        release_task_storage(*allocation,0x78,context);frame.allocation_08=nullptr;
    }
}
void cleanup_send_task_0292a0(HandlerTaskCleanup<SendHandlerTask>& frame,HandlerTaskContext& context){
    if(auto* const task=frame.task_10){release_control(task->handler.control_98);frame.task_10=nullptr;}
    if(auto* const allocation=frame.allocation_08){
        release_task_storage(*allocation,0x70,context);frame.allocation_08=nullptr;
    }
}
void call_receive_task_022670(ReceiveHandlerTask& task,std::uint8_t flag,HandlerTaskContext& context){
    HandlerTaskCleanup<ReceiveHandlerTask> frame{0,&task.storage,&task};
    ReceiveHandler local;
    local.words_60_78=task.handler.words_60_78;local.byte_80=task.handler.byte_80;
    local.dword_84=task.handler.dword_84;local.words_88_98=task.handler.words_88_98;
    local.object_a0=task.handler.object_a0;local.control_a8=task.handler.control_a8;
    task.handler.object_a0=0;task.handler.control_a8=nullptr;
    local.error=task.handler.error;local.transferred=task.handler.transferred;
    cleanup_receive_task_0291c0(frame,context); // may end task/storage lifetime
    if(flag)invoke_receive_021a40(local,context.effects);
    release_control(local.control_a8);
    cleanup_receive_task_0291c0(frame,context);
}
void call_send_task_0227b0(SendHandlerTask& task,std::uint8_t flag,HandlerTaskContext& context){
    HandlerTaskCleanup<SendHandlerTask> frame{0,&task.storage,&task};
    SendHandler local;
    local.words_60_78=task.handler.words_60_78;local.dword_80=task.handler.dword_80;
    local.word_88=task.handler.word_88;local.object_90=task.handler.object_90;local.control_98=task.handler.control_98;
    task.handler.object_90=0;task.handler.control_98=nullptr;
    local.error=task.handler.error;local.transferred=task.handler.transferred;
    cleanup_send_task_0292a0(frame,context);
    if(flag)invoke_send_021a80(local,context.effects);
    release_control(local.control_98);
    cleanup_send_task_0292a0(frame,context);
}
void ReceiveHandlerTask::call_slot_00(std::uint8_t flag){auto& borrowed_context=context;call_receive_task_022670(*this,flag,borrowed_context);}
void SendHandlerTask::call_slot_00(std::uint8_t flag){auto& borrowed_context=context;call_send_task_0227b0(*this,flag,borrowed_context);}
void schedule_receive_handler_022c60(IoHandlerEnvironment& environment,ReceiveHandler& handler,HandlerTaskContext& context){
    auto& schedule=schedule_table(environment,context);
    const auto borrowed=schedule.borrowed_slot_18;
    if(borrowed){
        BorrowedIoHandler descriptor{&handler,nullptr,0x180022640ULL};
        borrowed(environment,descriptor);
        return;
    }
    const auto publish=schedule.owned_slot_10; // cached before handler move/TLS/allocator
    if(!publish)throw UnsupportedInput("Native receive owned scheduler slot10 has no projection");
    ReceiveHandler local;
    local.words_60_78=handler.words_60_78;local.byte_80=handler.byte_80;
    local.dword_84=handler.dword_84;local.words_88_98=handler.words_88_98;
    local.object_a0=handler.object_a0;local.control_a8=handler.control_a8;
    handler.object_a0=0;handler.control_a8=nullptr;
    local.error=handler.error;local.transferred=handler.transferred;
    auto* const tls=static_cast<WorkerTlsContext*>(context.api.tls_get(context.environment.storage.tls_index));
    auto& storage=allocate_handler_cached_021900(tls?tls->state:nullptr,0x78,1,context.environment.storage,context.memory);
    auto& task=context.memory.project_receive_task(storage,context);
    task.handler=local;local.object_a0=0;local.control_a8=nullptr;
    task.callback_identity_00=0x180022670ULL;
    OwnedHandlerTask* slot=&task;
    release_control(local.control_a8); // assembly local was zeroed; no fabricated release
    publish(environment,slot);
    auto* const remaining=slot;
    if(remaining)remaining->call_slot_00(0); // reload mutable slot and live callback
}
void schedule_send_handler_0229d0(IoHandlerEnvironment& environment,SendHandler& handler,HandlerTaskContext& context){
    auto* const executor=environment.executor_28;
    if(!executor)throw UnsupportedInput("Native send executor has no projection");
    const auto prepare=executor->prepare_slot_40;
    if(!prepare)throw UnsupportedInput("Native send executor factory has no projection");
    const auto query=environment_table(environment).query_slot_18(environment);
    IoPrepareScratch scratch;IoHandlerEnvironment temporary;
    prepare(temporary,query,scratch);
    IoHandlerEnvironment moved;
    move_io_environment_023cb0(moved,temporary,context.environment.sentinels);
    if(environment_table(temporary).destroy_identity!=context.environment.sentinels.guard_destroy_017a30)
        environment_table(temporary).destroy_slot_00(temporary);
    auto& schedule=schedule_table(moved,context);
    const auto borrowed=schedule.borrowed_slot_18;
    if(borrowed){
        BorrowedIoHandler descriptor{nullptr,&handler,0x180022780ULL};
        borrowed(moved,descriptor);
    }else{
        const auto publish=schedule.owned_slot_10;
        if(!publish)throw UnsupportedInput("Native send owned scheduler slot10 has no projection");
        SendHandler local;
        local.words_60_78=handler.words_60_78;local.dword_80=handler.dword_80;
        local.word_88=handler.word_88;local.object_90=handler.object_90;local.control_98=handler.control_98;
        handler.object_90=0;handler.control_98=nullptr;
        local.error=handler.error;local.transferred=handler.transferred;
        auto* const tls=static_cast<WorkerTlsContext*>(context.api.tls_get(context.environment.storage.tls_index));
        auto& storage=allocate_handler_cached_021900(tls?tls->state:nullptr,0x70,1,context.environment.storage,context.memory);
        auto& task=context.memory.project_send_task(storage,context);
        task.handler=local;local.object_90=0;local.control_98=nullptr;
        task.callback_identity_00=0x1800227b0ULL;
        OwnedHandlerTask* slot=&task;
        release_control(local.control_98);
        publish(moved,slot);
        auto* const remaining=slot;
        if(remaining)remaining->call_slot_00(0);
    }
    environment_table(moved).destroy_slot_00(moved);
}
} // namespace preloader_l::socket_recovered
