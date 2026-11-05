#include "write_finish.hpp"

namespace anadius::socket_recovered {
MemoryLookup lookup_allocation_header(std::uintptr_t header,
    std::span<const WriteFinishAllocationHeader> headers,AllocationHeaderResolver* resolver,std::uintptr_t& output){
    if(resolver){
        std::uintptr_t found=0;
        const auto match=resolver->lookup_header(header,found);
        if(match==MemoryLookup::Found)output=found;
        return match;
    }
    std::uintptr_t found=0;bool matched=false;
    for(const auto& binding:headers){
        if(binding.address!=header)continue;
        if(matched)return MemoryLookup::Ambiguous;
        matched=true;found=binding.allocation_base;
    }
    if(!matched)return MemoryLookup::Miss;
    output=found;return MemoryLookup::Found;
}
namespace {
std::uintptr_t heap_address(const WriteFinishString& string){
    std::uint64_t address=0;
    for(std::size_t i=0;i<8;++i)address|=std::uint64_t{string.storage_00[i]}<<(8*i);
    return static_cast<std::uintptr_t>(address);
}
std::uintptr_t string_address(const WriteFinishString& string){
    return string.capacity_18>15?heap_address(string):reinterpret_cast<std::uintptr_t>(string.storage_00.data());
}
WriteFinishOwner& owner(std::uintptr_t identity,WriteFinishContext& context){
    WriteFinishOwner* found=nullptr;bool matched=false;
    for(const auto& binding:context.owners){
        if(binding.identity!=identity)continue;
        if(matched)throw UnsupportedInput("Write completion owner identity is ambiguous");
        matched=true;found=binding.owner;
    }
    if(!found)throw UnsupportedInput("Write completion owner has no live projection");
    return *found;
}
WriteFinishCategory& category(const Category& value,WriteFinishContext& context){
    WriteFinishCategory* found=nullptr;bool matched=false;
    for(const auto& binding:context.categories){
        if(binding.key.name_identity!=value.name_identity || binding.key.logical!=value.logical)continue;
        if(matched)throw UnsupportedInput("Write completion category identity is ambiguous");
        matched=true;found=binding.category;
    }
    if(!found)throw UnsupportedInput("Write completion category has no live object projection");
    return *found;
}
std::uintptr_t allocation_base(std::uintptr_t header,WriteFinishContext& context){
    std::uintptr_t found=0;
    const auto match=lookup_allocation_header(header,context.allocation_headers,context.allocation_header_resolver,found);
    if(match==MemoryLookup::Ambiguous)throw UnsupportedInput("Write completion allocation header is ambiguous");
    if(match!=MemoryLookup::Found)throw UnsupportedInput("Write completion allocation header leaves bounded projection");
    return found;
}
void destroy_heap(WriteFinishString& string,WriteFinishContext& context){
    const auto capacity=string.capacity_18;
    if(capacity<=15)return;
    auto bytes=capacity+std::uint64_t{1};
    auto address=heap_address(string);
    if(bytes>=4096){
        bytes+=0x27;
        const auto base=allocation_base(address-std::uintptr_t{8},context);
        if(address-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        address=base;
    }
    context.api.free_sized_04b0f8(address,bytes);
}
}
void finish_write_025a70(SendHandler& handler,Error& error,WriteFinishContext& context){
    const auto code=error.value; // native caches this before any provider call
    if(code==0){
        auto& saved_owner=owner(static_cast<std::uintptr_t>(handler.word_88),context);
        const auto index=(saved_owner.map_size_d8-std::uint64_t{1})&saved_owner.offset_e0;
        if(index>=saved_owner.map_d0.size() || !saved_owner.map_d0[static_cast<std::size_t>(index)])
            throw UnsupportedInput("Write completion deque head leaves bounded map");
        auto& string=*saved_owner.map_d0[static_cast<std::size_t>(index)];
        destroy_heap(string,context);
        string.size_10=0;
        string.capacity_18=15;
        string.storage_00[0]=0; // other prefix bytes are retained, not zero-filled
        --saved_owner.count_e8;
        if(saved_owner.count_e8==0)saved_owner.offset_e0=0;
        else ++saved_owner.offset_e0;
        // Native reloads the owner token AFTER free and queue updates. The
        // next callback may end handler/owner lifetime; nothing is read later.
        context.api.next_write_02a500(static_cast<std::uintptr_t>(handler.word_88));
        return;
    }
    const auto misc=context.completion.misc_category_0285b0();
    auto& saved_category=category(error.category,context); // live after misc
    if(saved_category.identity_08==misc.name_identity && code==2)return;
    auto& initial=context.api.begin_log_006880(0x1800a4130ULL,0x1800845d8ULL);
    WriteFinishString message;
    saved_category.message_slot_10(message,code); // live virtual call after begin
    const auto data=string_address(message);
    auto& stream=context.api.append_007990(initial,data,message.size_10);
    const auto adjusted=stream.identity+static_cast<std::uintptr_t>(static_cast<std::intptr_t>(stream.virtual_base_offset_04));
    const auto character=context.api.widen_01e510(adjusted,10);
    context.api.put_00fe90(stream,static_cast<std::uint8_t>(character));
    context.api.flush_00e1e0(stream);
    destroy_heap(message,context); // capacity/address reloaded after log callbacks
}
void RecoveredFinishedWriteEffects::finish_write_025a70(SendHandler& handler,Error& error){
    socket_recovered::finish_write_025a70(handler,error,finish_context_);
}
} // namespace anadius::socket_recovered
