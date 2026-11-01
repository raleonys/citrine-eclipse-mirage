#include "dispose.hpp"
#include <bit>

namespace anadius::socket_recovered {
namespace {
std::uintptr_t allocation_base(std::uintptr_t address,SessionDisposalContext& context){
    const auto header=context.memory.view(address-std::uintptr_t{8},8);
    std::uint64_t result=0;
    for(std::size_t i=0;i<8;++i)result|=std::uint64_t{header[i]}<<(i*8);
    const auto base=static_cast<std::uintptr_t>(result);
    if(address-base-std::uintptr_t{8}>31)context.allocation.invalid_parameter_05c974();
    return base;
}
void free_array(std::uintptr_t address,std::uint64_t bytes,SessionDisposalContext& context){
    if(bytes>=4096){address=allocation_base(address,context);bytes+=std::uint64_t{0x27};}
    context.allocation.free_sized_04b0f8(address,bytes);
}
void free_string_heap(ReceiveBufferString& string,SessionDisposalContext& context){
    const auto capacity=string.capacity_18;
    if(capacity>15)free_array(receive_buffer_address(string),capacity+std::uint64_t{1},context);
}
WriteFinishString* block_at(WriteFinishOwner& fields,std::uint64_t index){
    if(index>=fields.map_d0.size())throw UnsupportedInput("Session deque index leaves live bounded map");
    return fields.map_d0[static_cast<std::size_t>(index)];
}
}
void clear_session_output_026880(OutputDeque& deque,SessionDisposalContext& context){
    auto& fields=deque.fields;
    auto count=fields.count_e8;
    if(count!=0){
        do{
            const auto index=(count+fields.offset_e0-std::uint64_t{1})&(fields.map_size_d8-std::uint64_t{1});
            auto* const string=block_at(fields,index);
            if(!string)throw UnsupportedInput("Session deque logical element has no live string projection");
            free_string_heap(*string,context);
            string->size_10=0;string->capacity_18=15;string->storage_00[0]=0;
            // Reload count AFTER sizedfree; do not retain the pre-callback value.
            count=fields.count_e8-std::uint64_t{1};fields.count_e8=count;
        }while(count!=0);
        fields.offset_e0=0;
    }
    if(fields.map_d0.data()!=nullptr){
        auto remaining=fields.map_size_d8; // native RBX caches this loop bound
        while(std::bit_cast<std::int64_t>(remaining)>0){
            --remaining;
            auto* const block=block_at(fields,remaining); // live map reload
            if(block)context.allocation.free_sized_04b0f8(reinterpret_cast<std::uintptr_t>(block),32);
        }
        // Native map address and size are reloaded after all block callbacks.
        free_array(reinterpret_cast<std::uintptr_t>(fields.map_d0.data()),fields.map_size_d8<<3,context);
    }
    fields.map_d0={};fields.map_size_d8=0;
}
void dispose_session_control_026570(SessionControl& control,SessionDisposalContext& context){
    auto& owner=control.object;
    clear_session_output_026880(owner.output,context);
    auto* const proxy=owner.output.proxy_c8;
    owner.output.proxy_c8=nullptr; // observable BEFORE sizedfree16, including null
    context.allocation.free_sized_04b0f8(reinterpret_cast<std::uintptr_t>(proxy),16);
    free_string_heap(owner.input,context); // live input load after proxy callback
    owner.input.size_10=0;owner.input.capacity_18=15;owner.input.storage_00[0]=0;
    destroy_024db0(owner.socket,context.lifecycle);
    // Native reloads selfweak AFTER socket close/executor/cancellation effects.
    // Neither object nor self-control fields are cleared by026570.
    auto* const self=owner.weak_self.control_08;
    if(self&&self->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)self->delete_slot_08();
    // No owner/control access after potentially deleting self callback.
}
void delete_session_control_017420(SessionDeleteTarget* control){
    if(control)(void)control->delete_flags_slot_10(1);
}
std::uintptr_t delete_session_control_026290(SessionControl& control,std::uint32_t flags,SessionFreeEffects& effects){
    const auto identity=reinterpret_cast<std::uintptr_t>(&control);
    control.table_00=0x180084750ULL;
    if((flags&1u)!=0)effects.free_sized_04b0f8(identity,0x1c8);
    return identity;
}
void RecoveredSessionControl::dispose_slot_00(){dispose_session_control_026570(*this,context_);}
void RecoveredSessionControl::delete_slot_08(){delete_session_control_017420(this);}
std::uintptr_t RecoveredSessionControl::delete_flags_slot_10(std::uint32_t flags){
    return delete_session_control_026290(*this,flags,context_.allocation);
}
} // namespace anadius::socket_recovered
