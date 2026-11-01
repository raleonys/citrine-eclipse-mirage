#include "response_dispatch.hpp"

namespace anadius::socket_recovered {
namespace {
void reset(ReceiveBufferString& value){value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;}
void destroy(ReceiveBufferString& value,FrameReadContext& context){
    const auto cap=value.capacity_18;if(cap<=15)return;
    auto bytes=cap+std::uint64_t{1};auto data=receive_buffer_address(value);
    if(bytes>=4096){
        bytes+=39;std::uintptr_t base=0;bool found=false;
        for(const auto& item:context.allocation_headers){
            if(item.address!=data-std::uintptr_t{8})continue;
            if(found)throw UnsupportedInput("Response allocation header is ambiguous");
            found=true;base=item.allocation_base;
        }
        if(!found)throw UnsupportedInput("Response allocation header is unbound");
        if(data-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        data=base;
    }
    context.api.free_sized_04b0f8(data,bytes);
}
MessageWriteTask move_task(MessageWriteTask& source){
    MessageWriteTask local;local.function_00=source.function_00;local.response_08=source.response_08;reset(source.response_08);
    local.object_28=source.object_28;local.control_30=source.control_30;source.object_28=0;source.control_30=nullptr;return local;
}
IoHandlerVtable& table(IoHandlerEnvironment& value){
    if(!value.operations_10)throw UnsupportedInput("Response environment table is unbound");
    return *value.operations_10;
}
ResponseExecutorBinding& executor(IoHandlerExecutor* identity,ResponseDispatchContext& context){
    ResponseExecutorBinding* found=nullptr;bool matched=false;
    for(auto& item:context.executors){
        if(item.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Response executor identity is ambiguous");
        matched=true;found=&item;
    }
    if(!identity || !found)throw UnsupportedInput("Response executor factory is unbound");
    return *found;
}
ResponseSchedule& schedule(std::uintptr_t identity,ResponseDispatchContext& context){
    ResponseSchedule* found=nullptr;bool matched=false;
    for(const auto& item:context.schedules){
        if(item.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Response scheduler identity is ambiguous");
        matched=true;found=item.table;
    }
    if(!found)throw UnsupportedInput("Response scheduler table is unbound");
    return *found;
}
WriteFinishOwner& queue_at(std::uintptr_t identity,ResponseQueueContext& context){
    WriteFinishOwner* found=nullptr;bool matched=false;
    for(const auto& item:context.owners){
        if(item.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Response queue identity is ambiguous");
        matched=true;found=item.owner;
    }
    if(!found)throw UnsupportedInput("Response queue owner is unbound");
    return *found;
}
WriteFinishString*& slot_at(WriteFinishOwner& queue,std::uint64_t index){
    if(index>=queue.map_d0.size())throw UnsupportedInput("Response queue slot leaves bounded map");
    return queue.map_d0[static_cast<std::size_t>(index)];
}
}
void dispatch_response_021420(IoHandlerEnvironment& source,MessageWriteTask& incoming,ResponseDispatchContext& context){
    auto local=move_task(incoming);
    const auto construct20=executor(source.executor_28,context).factory_20;
    if(!construct20)throw UnsupportedInput("Response factory20 is unbound");
    const auto query=table(source).query_slot_18(source);
    IoHandlerEnvironment prepared1;construct20(prepared1,query,0x180084425ULL);
    IoHandlerEnvironment first;move_io_environment_023cb0(first,prepared1,context.sentinels);
    if(table(prepared1).destroy_identity!=context.sentinels.guard_destroy_017a30)table(prepared1).destroy_slot_00(prepared1);
    // Native021532/539 uses first.executor LIVE after conditional destruction,
    // then caches its slot88 before first.query may change/delete that executor.
    const auto construct88=executor(first.executor_28,context).factory_88;
    if(!construct88)throw UnsupportedInput("Response factory88 is unbound");
    const auto second_query=table(first).query_slot_18(first);
    IoHandlerEnvironment prepared2;ResponseExecutorScratch scratch;construct88(prepared2,second_query,scratch);
    IoHandlerEnvironment intermediate;move_io_environment_023cb0(intermediate,prepared2,context.sentinels);
    if(table(prepared2).destroy_identity!=context.sentinels.guard_destroy_017a30)table(prepared2).destroy_slot_00(prepared2);
    IoHandlerEnvironment final;move_io_environment_023cb0(final,intermediate,context.sentinels);
    table(intermediate).destroy_slot_00(intermediate); // unconditional, including guard
    auto& selected=schedule(final.schedule_20,context);
    const auto borrowed=selected.borrowed_slot_18;
    if(borrowed){
        BorrowedResponse descriptor{&local,0x180022240ULL};borrowed(final,descriptor);
    }else{
        const auto publish=selected.owned_slot_10;
        if(!publish)throw UnsupportedInput("Response owned slot10 is unbound");
        auto transferred=move_task(local);PostedFunctor* slot=nullptr;
        context.api.make_owned_response_020cb0(slot,transferred);
        publish(final,slot);
        auto* const residual=slot;if(residual)residual->call_slot_00(0);
    }
    table(final).destroy_slot_00(final);
    table(first).destroy_slot_00(first);
    auto* const residual=local.control_30;
    if(residual && residual->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        residual->dispose_slot_00();
        if(residual->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)residual->delete_slot_08();
    }
    destroy(local.response_08,context.strings);
}
void enqueue_response_028270(std::uintptr_t identity,ReceiveBufferString& response,ResponseQueueContext& context,ResponseQueueStart call_02a500){
    auto& queue=queue_at(identity,context);
    if(queue.map_size_d8<=queue.count_e8+std::uint64_t{1})context.api.grow_deque_026670(queue,1);
    const auto normalized=(queue.map_size_d8-std::uint64_t{1})&queue.offset_e0;queue.offset_e0=normalized;
    const auto absolute=queue.count_e8+normalized;
    const auto initial_index=(queue.map_size_d8-std::uint64_t{1})&absolute;
    if(!slot_at(queue,initial_index)){
        auto* allocated=context.api.allocate_string_04b0bc(0x20);
        slot_at(queue,initial_index)=allocated; // live map, cached original slot index
    }
    auto* destination=slot_at(queue,(queue.map_size_d8-std::uint64_t{1})&absolute);
    if(!destination)throw UnsupportedInput("Response destination string is null");
    destination->storage_00.fill(0);destination->size_10=0;destination->capacity_18=0;
    *destination=response;reset(response);
    ++queue.count_e8;
    if(queue.count_e8==1){
        if(!context.queued || !call_02a500)throw UnsupportedInput("Response queued-write context is unbound");
        call_02a500(identity,*context.queued,&start_async_send_021fd0);
    }
    // Queued write can end owner/queue lifetime. Only the original response
    // argument is read after the call, as at native028354.
    destroy(response,context.strings);reset(response);
}
void invoke_response_write(std::uintptr_t function,std::uintptr_t owner,ReceiveBufferString& response,ResponseDispatchContext& context){
    if(function!=0x180028270ULL || !context.queue)throw UnsupportedInput("Response write target/context is unbound");
    enqueue_response_028270(owner,response,*context.queue);
}
void invoke_borrowed_response_022240(MessageWriteTask& task,ResponseDispatchContext& context,ResponseWriteCall call){
    const auto function=task.function_00,owner=task.object_28;
    ReceiveBufferString temporary;
    auto& copied=context.api.copy_response_009eb0(temporary,task.response_08);
    if(!call)throw UnsupportedInput("Response write callback is unbound");
    call(function,owner,copied,context);
}
void BorrowedResponse::invoke(ResponseDispatchContext& context) const{
    if(!task || invoke_identity!=0x180022240ULL)throw UnsupportedInput("Borrowed response descriptor is unbound");
    invoke_borrowed_response_022240(*task,context);
}
void RecoveredResponseDispatchApi::dispatch_response_021420(IoHandlerEnvironment& env,MessageWriteTask& task,std::uint64_t,std::uint64_t){
    socket_recovered::dispatch_response_021420(env,task,context_);
}
} // namespace anadius::socket_recovered
