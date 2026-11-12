// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_message_dispatch.hpp"

namespace citrine_eclipse_mirage::socket {
namespace {
void reset(ReceiveBufferString& value){value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;}
void destroy(ReceiveBufferString& value,MessageDispatchContext& context){
    const auto capacity=value.capacity_18;if(capacity<=15)return;
    auto data=receive_buffer_address(value);auto bytes=capacity+std::uint64_t{1};
    if(bytes>=4096){
        bytes+=39;std::uintptr_t base=0;bool found=false;
        for(const auto& binding:context.frame.allocation_headers){
            if(binding.address!=data-std::uintptr_t{8})continue;
            if(found)throw UnsupportedInput("Message allocation header is ambiguous");
            found=true;base=binding.allocation_base;
        }
        if(!found)throw UnsupportedInput("Message allocation header is unbound");
        if(data-base-std::uintptr_t{8}>31)context.frame.api.invalid_parameter_05c974();
        data=base;
    }
    context.frame.api.free_sized_04b0f8(data,bytes);
}
MessageDispatchOwner& owner_at(std::uintptr_t identity,MessageDispatchContext& context){
    MessageDispatchOwner* found=nullptr;bool matched=false;
    for(const auto& binding:context.owners){
        if(binding.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Message owner identity is ambiguous");
        matched=true;found=binding.owner;
    }
    if(!found || !found->lifetime)throw UnsupportedInput("Message owner has no live shared projection");
    return *found;
}
MessageEnvironmentCopy copy_at(IoHandlerVtable* table,MessageDispatchContext& context){
    MessageEnvironmentCopy found=nullptr;bool matched=false;
    for(const auto& binding:context.copies){
        if(binding.table!=table)continue;
        if(matched)throw UnsupportedInput("Message copy slot is ambiguous");
        matched=true;found=binding.copy_slot_08;
    }
    if(!table || !found)throw UnsupportedInput("Message copy slot has no live projection");
    return found;
}
IoHandlerVtable& table_of(IoHandlerEnvironment& env){
    if(!env.operations_10)throw UnsupportedInput("Message environment destroy table is unbound");
    return *env.operations_10;
}
}
void dispatch_message_028d30(std::uintptr_t identity,ReceiveBufferString& request,MessageDispatchContext& context){
    if(!request.size_10){context.api.destroy_empty_message_00aa50(request);return;}
    ReceiveBufferString response;
    context.api.process_message_033d70(identity+std::uintptr_t{0xf0},response,request);
    auto& owner=owner_at(identity,context);
    auto* const acquisition=owner.lifetime->control_08;
    if(!acquisition)context.frame.terminal.bad_weak_ptr_026860();
    auto expected=acquisition->strong_08.load(std::memory_order_relaxed);
    for(;;){
        if(!expected)context.frame.terminal.bad_weak_ptr_026860();
        if(acquisition->strong_08.compare_exchange_strong(expected,expected+std::uint32_t{1},
            std::memory_order_seq_cst,std::memory_order_seq_cst))break;
    }
    MessageWriteTask task;
    task.function_00=0x180028270ULL;task.response_08=response;reset(response);
    task.object_28=owner.lifetime->object_00;task.control_30=owner.lifetime->control_08;
    auto* source=owner.environment_78;
    if(!source)throw UnsupportedInput("Message owner environment is unbound");
    IoHandlerEnvironment first;
    first.operations_10=source->operations_10;first.schedule_20=source->schedule_20;
    const auto first_copy=copy_at(first.operations_10,context);
    first_copy(first,*source);
    first.executor_28=source->executor_28; // live after first copy
    IoHandlerEnvironment second;
    second.operations_10=first.operations_10;second.schedule_20=first.schedule_20;
    const auto second_copy=copy_at(second.operations_10,context);
    second_copy(second,first);
    second.executor_28=first.executor_28; // live after second copy
    context.api.dispatch_response_021420(second,task,0,0);
    table_of(second).destroy_slot_00(second);
    table_of(first).destroy_slot_00(first);
    auto* const residual=task.control_30;
    if(residual && residual->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        residual->dispose_slot_00();
        if(residual->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1){
            // Native028ecf reloads the delete TARGET after disposal, while the
            // weak counter above still belongs to the cached residual control.
            auto* const target=task.control_30;
            if(!target)throw UnsupportedInput("Message live delete target is null");
            target->delete_slot_08();
        }
    }
    destroy(task.response_08,context);
    destroy(response,context);reset(response);
    destroy(request,context);reset(request);
}
void invoke_frame_message(std::uintptr_t function,std::uintptr_t owner,ReceiveBufferString& request,MessageDispatchContext& context){
    if(function!=0x180028d30ULL)throw UnsupportedInput("Unsupported frame message function token");
    dispatch_message_028d30(owner,request,context);
}
std::uint32_t message_thread_entry_021770(FrameMessageClosure& closure,MessageDispatchContext& context,FrameMessageInvoker invoke){
    const auto owner=closure.owner_20,function=closure.function_28;
    auto request=closure.message_00;reset(closure.message_00);
    if(!invoke)throw UnsupportedInput("Message thread callee is unbound");
    invoke(function,owner,request,context);
    context.api.cleanup_exit_waits_04a47c();
    destroy(closure.message_00,context);reset(closure.message_00);
    context.frame.api.free_sized_04b0f8(reinterpret_cast<std::uintptr_t>(&closure),0x30);
    return 0; // closure may have been deleted; never read again
}
std::uint32_t route_message_thread_entry(std::uintptr_t entry,std::uintptr_t argument,MessageDispatchContext& context){
    if(entry!=0x180021770ULL)throw UnsupportedInput("Unsupported message thread entry token");
    FrameMessageClosure* found=nullptr;bool matched=false;
    for(const auto& binding:context.closures){
        if(binding.identity!=argument)continue;
        if(matched)throw UnsupportedInput("Message closure identity is ambiguous");
        matched=true;found=binding.closure;
    }
    if(!argument || !found)throw UnsupportedInput("Message thread argument has no live closure");
    return message_thread_entry_021770(*found,context);
}
std::uintptr_t MessageThreadApi::begin_frame_thread_05e8e0(std::uintptr_t security,std::uint32_t stack,
    std::uintptr_t entry,FrameMessageClosure* closure,std::uint32_t flags,std::uint32_t& id){
    return threads::begin_thread_05e8e0(reinterpret_cast<void*>(security),stack,entry,
        reinterpret_cast<std::uintptr_t>(closure),flags,&id,threads_);
}
} // namespace citrine_eclipse_mirage::socket
