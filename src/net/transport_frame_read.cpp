// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_frame_read.hpp"
#include <algorithm>

namespace laureondo::socket {
namespace {
FrameReadOwner& owner_at(std::uintptr_t identity,FrameReadContext& context){
    FrameReadOwner* found=nullptr;bool matched=false;
    for(const auto& item:context.owners){
        if(item.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Frame owner identity is ambiguous");
        matched=true;found=item.owner;
    }
    if(!found)throw UnsupportedInput("Frame owner has no live projection");
    return *found;
}
ReceiveBufferString& input_at(std::uintptr_t identity,FrameReadContext& context){
    auto* input=owner_at(identity,context).input_a8;
    if(!input)throw UnsupportedInput("Frame input has no live projection");
    return *input;
}
WriteFinishCategory& category_at(const Category& key,FrameReadContext& context){
    WriteFinishCategory* found=nullptr;bool matched=false;
    for(const auto& item:context.categories){
        if(item.key.name_identity!=key.name_identity || item.key.logical!=key.logical)continue;
        if(matched)throw UnsupportedInput("Frame category identity is ambiguous");
        matched=true;found=item.category;
    }
    if(!found)throw UnsupportedInput("Frame category has no live projection");
    return *found;
}
void destroy_message(ReceiveBufferString& message,FrameReadContext& context){
    const auto capacity=message.capacity_18;
    if(capacity<=15)return;
    auto address=receive_buffer_address(message);
    auto bytes=capacity+std::uint64_t{1};
    if(bytes>=4096){
        bytes+=39;std::uintptr_t base=0;bool matched=false;
        for(const auto& item:context.allocation_headers){
            if(item.address!=address-std::uintptr_t{8})continue;
            if(matched)throw UnsupportedInput("Frame allocation header is ambiguous");
            matched=true;base=item.allocation_base;
        }
        if(!matched)throw UnsupportedInput("Frame allocation header leaves projection");
        if(address-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        address=base;
    }
    context.api.free_sized_04b0f8(address,bytes);
}
}
void start_frame_read_0280d0(std::uintptr_t identity,FrameReadContext& context,FrameReadComposedCall call_025c40){
    auto& owner=owner_at(identity,context);
    auto* const acquisition=owner.control_08;
    if(!acquisition)context.terminal.bad_weak_ptr_026860();
    auto expected=acquisition->strong_08.load(std::memory_order_relaxed);
    for(;;){
        if(!expected)context.terminal.bad_weak_ptr_026860();
        if(acquisition->strong_08.compare_exchange_strong(expected,expected+std::uint32_t{1},
            std::memory_order_seq_cst,std::memory_order_seq_cst))break;
    }
    ReceiveHandler handler;
    handler.words_60_78={identity+std::uintptr_t{0x10},identity+std::uintptr_t{0xa8},UINT64_MAX,UINT64_MAX};
    handler.byte_80=0;handler.dword_84=0;handler.words_88_98={0,0,identity};
    handler.object_a0=owner.object_00;handler.control_a8=owner.control_08;
    Error error{0,context.success_category_a10f8};
    if(!context.read || !call_025c40)throw UnsupportedInput("Frame read context/callee is unbound");
    call_025c40(handler,error,0,1,*context.read,&start_async_receive_021d10);
    // No owner use after publication. Dispose may mutate handler/control;
    // weak release still targets this SAME cached residual control.
    auto* const residual=handler.control_a8;
    if(residual && residual->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        residual->dispose_slot_00();
        if(residual->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)residual->delete_slot_08();
    }
}
void finish_frame_read_025480(ReceiveHandler& handler,Error& error,std::uint64_t matched,
    FrameReadContext& context,FrameReadRestart call_0280d0){
    const auto code=error.value;
    if(code!=0){
        const auto misc=context.completion.misc_category_0285b0();
        auto& category=category_at(error.category,context);
        if(code==2 && category.identity_08==misc.name_identity)return;
        auto& initial=context.api.begin_log_006880(0x1800a4130ULL,0x1800845c8ULL);
        ReceiveBufferString message;
        category.message_slot_10(message,code);
        auto& stream=context.api.append_007990(initial,receive_buffer_address(message),message.size_10);
        const auto adjusted=stream.identity+static_cast<std::uintptr_t>(static_cast<std::intptr_t>(stream.virtual_base_offset_04));
        const auto character=context.api.widen_01e510(adjusted,10);
        context.api.put_00fe90(stream,static_cast<std::uint8_t>(character));
        context.api.flush_00e1e0(stream);
        destroy_message(message,context);return;
    }
    auto& input=input_at(static_cast<std::uintptr_t>(handler.words_88_98[2]),context);
    const auto count=std::min(input.size_10,matched-std::uint64_t{1});
    ReceiveBufferString message;message.capacity_18=0; // native local begins as all zero
    context.api.construct_message_006b30(message,receive_buffer_address(input),count);
    auto* closure=context.api.allocate_frame_04b0bc(0x30);
    if(closure){
        closure->message_00=message;
        message.size_10=0;message.capacity_18=15;message.storage_00[0]=0;
        closure->owner_20=static_cast<std::uintptr_t>(handler.words_88_98[2]);
        closure->function_28=0x180028d30ULL;
    }
    FrameThreadHandle thread;
    thread.handle=context.api.begin_frame_thread_05e8e0(0,0,0x180021770ULL,closure,0,thread.id);
    if(!thread.handle){thread.id=0;context.api.thread_error_04a648(6);}
    if(!thread.id || context.api.detach_04a458(thread)!=0)context.api.thread_error_04a648(1);
    destroy_message(message,context);
    auto& saved_input=input_at(static_cast<std::uintptr_t>(handler.words_88_98[2]),context);
    const auto size=saved_input.size_10;
    const auto consumed=std::min(size,matched),remaining=size-consumed;
    const auto data=receive_buffer_address(saved_input);
    context.buffer.api.copy_bytes(data,data+static_cast<std::uintptr_t>(consumed),remaining+std::uint64_t{1});
    saved_input.size_10=remaining; // saved target/size even if byte provider mutates owner
    if(!call_0280d0)throw UnsupportedInput("Frame restart callee is unbound");
    call_0280d0(static_cast<std::uintptr_t>(handler.words_88_98[2]),context,&composed_receive_025c40);
    // Restart may end handler/owner lifetime. No accesses after this point.
}
void FrameReadApiAdapter::finish_frame_025480(ReceiveHandler& h,Error& e,std::uint64_t n){
    finish_frame_read_025480(h,e,n,context_);
}
} // namespace laureondo::socket
