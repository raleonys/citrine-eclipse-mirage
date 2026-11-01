#include "owner.hpp"
#include <new>

namespace anadius::socket_recovered {
namespace {
void release_constructor_strong(IoReference* control){
    if(control&&control->strong_08.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->dispose_slot_00();
        if(control->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_slot_08();
    }
}
}
SessionPair& construct_session_owner_023000(SessionPair& output,SocketObject& pending,ConstructionContext& context){
    auto& scratch=context.scratch;
    if(scratch.begun)throw UnsupportedInput("Session construction scratch is single-use, not a retry/rollback owner");
    if(&pending==&scratch.temporary||&pending.state==&scratch.temporary_state)
        throw UnsupportedInput("Pending socket and construction scratch must be disjoint");
    scratch.begun=true;
    auto* const control=context.allocation.allocate_control_04b0bc(0x1c8);
    if(!control)throw std::bad_alloc(); // malformed lower; never a normal null owner
    scratch.allocation=control;
    auto& owner=control->object;
    control->strong_08.store(1,std::memory_order_seq_cst);
    control->weak_0c.store(1,std::memory_order_seq_cst);
    control->table_00=0x180084750ULL;
    move_socket_023d20(scratch.temporary,pending,context.executor);
    scratch.temporary_live=true;
    owner.weak_self.object_00=0;owner.weak_self.control_08=nullptr;
    move_socket_023d20(owner.socket,scratch.temporary,context.executor);
    scratch.owner_socket_live=true;
    owner.input.storage_00.fill(0);owner.input.size_10=0;owner.input.capacity_18=15;
    owner.weak_self.input_a8=&owner.input; // sidecar binding, not another native store
    owner.output.proxy_c8=nullptr;owner.output.fields.map_d0={};
    owner.output.fields.map_size_d8=0;owner.output.fields.offset_e0=0;owner.output.fields.count_e8=0;
    auto* const proxy=context.allocation.allocate_proxy_04b0bc(16);
    if(!proxy)throw std::bad_alloc();
    proxy->zero_08=0;
    owner.output.proxy_c8=proxy;
    proxy->owner_00=&owner.output;
    scratch.proxy_live=true;
    reset_session_cipher_02bc70(owner.cipher);
    destroy_024db0(scratch.temporary,context.lifecycle);
    scratch.temporary_live=false;
    output.object=&owner;output.control=control;
    scratch.published=true;
    // Publication above must survive an old-weak delete callback throwing.
    auto* const prior_self=owner.weak_self.control_08;
    if(!prior_self||prior_self->strong_08.load(std::memory_order_seq_cst)==0){
        control->strong_08.fetch_add(1,std::memory_order_seq_cst);
        auto* const copied_control=output.control;
        IoReference* new_self=nullptr;std::uintptr_t new_object=0;
        if(copied_control){
            copied_control->weak_0c.fetch_add(1,std::memory_order_seq_cst);
            new_self=copied_control;new_object=owner.identity();
        }
        owner.weak_self.object_00=new_object;
        auto* const old_self=owner.weak_self.control_08; // live load AFTER prior work
        owner.weak_self.control_08=new_self;
        if(old_self&&old_self->weak_0c.fetch_sub(1,std::memory_order_seq_cst)==1)
            old_self->delete_slot_08();
        // Native caches the output control BEFORE old-self callback: do not
        // reload output.control here or substitute a std::shared_ptr facade.
        release_constructor_strong(copied_control);
    }
    scratch.complete=true;
    return output;
}
} // namespace anadius::socket_recovered
