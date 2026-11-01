#include "move.hpp"

namespace anadius::socket_recovered {
namespace {
void distinct(SocketObject& destination,SocketObject& source){
    if(&destination==&source||&destination.state==&source.state)
        throw UnsupportedInput("Socket move requires disjoint object and state storage");
}
void release_move_control(Control* control){
    if(control&&control->strong.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->destroy_resource();
        // Reload weak after the virtual callback, matching native release.
        if(control->weak.fetch_sub(1,std::memory_order_seq_cst)==1)control->delete_control();
    }
}
}
void transfer_socket_028950(SocketService& service,SocketObject& destination,SocketObject& source){
    distinct(destination,source);
    if(service.head==&destination||destination.next||destination.previous)
        throw UnsupportedInput("Socket transfer destination must be unlinked");
    auto& dst=destination.state;auto& src=source.state;
    dst.socket=src.socket;src.socket=invalid_socket;
    dst.socket_state=src.socket_state;src.socket_state=0;
    if(source.cancellation.control)source.cancellation.control->strong.fetch_add(1,std::memory_order_seq_cst);
    auto* old=destination.cancellation.control;
    destination.cancellation.resource=source.cancellation.resource;
    destination.cancellation.control=source.cancellation.control;
    release_move_control(old);
    // Prior destruction may have changed the source pair; reload it now.
    old=source.cancellation.control;
    source.cancellation.resource=0;source.cancellation.control=nullptr;
    release_move_control(old);
    // Insert a NEW node. Native does not unlink the now-invalid source node.
    service.gate.enter();
    destination.next=service.head;destination.previous=nullptr;
    if(service.head)service.head->previous=&destination;
    service.head=&destination;
    service.gate.leave();
    dst.protocol_40=src.protocol_40;src.protocol_40=2;
    dst.byte_44=src.byte_44;src.byte_44=0;
    dst.slot_48=src.slot_48;dst.slot_50=src.slot_50;dst.slot_58=src.slot_58;dst.slot_60=src.slot_60;
    src.slot_48=2;src.slot_50=0;src.slot_58=0;src.slot_60=0;
}
SocketObject& move_socket_023d20(SocketObject& destination,SocketObject& source,ExecutorMove effect){
    distinct(destination,source);
    if(!source.service||!source.executor.table_78||!effect.move)
        throw UnsupportedInput("Socket move requires live service and selected executor slot8 effect");
    if(destination.service||destination.next||destination.previous||destination.cancellation.control||
       destination.cancellation.resource||destination.state.socket!=invalid_socket)
        throw UnsupportedInput("Socket constructor move requires fresh destination");
    destination.service=source.service;destination.state.service=source.service;
    destination.cancellation.resource=0;destination.cancellation.control=nullptr;
    auto& state=destination.state;
    state.protocol_40=2;state.byte_44=0;
    state.slot_48=0;state.slot_50=0;state.slot_58=0;state.slot_60=0;state.slot_48=2;
    destination.executor.table_78=source.executor.table_78;
    destination.executor.table_88=source.executor.table_88;
    effect.move(effect.context,destination.executor,source.executor,destination.executor.table_78);
    destination.executor.table_90=source.executor.table_90; // live reload AFTER callback
    auto* const service=destination.service;
    transfer_socket_028950(*service,destination,source);
    return destination;
}
} // namespace anadius::socket_recovered
