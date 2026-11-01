#include "lifecycle.hpp"
#include <new>

namespace anadius::socket_recovered {
namespace {
void release(Control* control){
    if(control && control->strong.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->destroy_resource();
        // Read/decrement weak only after the callback; it may change weak count.
        if(control->weak.fetch_sub(1,std::memory_order_seq_cst)==1)
            control->delete_control();
    }
}
class NoopControl final:public Control {
public:void destroy_resource() override{};void delete_control() override{delete this;}
};
}
Control* OwnedNoopControlFactory::allocate_04b0bc(std::uint32_t request){
    if(request!=24)throw UnsupportedInput("Recovered control request is24 native bytes");
    // Selected host allocation only: not native04b0bc's malloc/new-handler loop.
    return new(std::nothrow) NoopControl;
}
SocketObject& initialize_0211b0(SocketObject& object,ContextIdentity context,LifecycleEffects& effects){
    object.service=&effects.resolve_service_0239c0(context);
    object.state.service=object.service;
    object.cancellation.resource=0;object.cancellation.control=nullptr;
    auto& state=object.state;
    state.protocol_40=2;state.byte_44=0;
    state.slot_48=2;state.slot_50=0;state.slot_58=0;state.slot_60=0;
    object.executor.table_88=0x180084908ULL;
    object.executor.context=context;object.executor.zero_74=0;
    object.executor.table_78=0x180084928ULL;
    object.executor.table_90=0x180084860ULL;
    object.executor.self_reference_80=true;
    auto* const service=object.service;
    state.socket=invalid_socket;state.socket_state=0;
    auto* const old=object.cancellation.control; // native reload after prior NULL store
    object.cancellation.resource=0;object.cancellation.control=nullptr;
    release(old);
    service->gate.enter();
    object.next=service->head;object.previous=nullptr;
    if(service->head)service->head->previous=&object;
    service->head=&object;
    service->gate.leave();
    return object;
}
void replace_owner_0237f0(ResourcePair& pair,std::uintptr_t resource,ControlFactory& factory){
    auto* const fresh=factory.allocate_04b0bc(24);
    if(fresh){
        fresh->strong.store(1,std::memory_order_seq_cst);
        fresh->weak.store(1,std::memory_order_seq_cst);
        fresh->resource=resource;
    }
    pair.resource=resource;
    auto* const old=pair.control; // allocation may have re-entered and changed pair
    pair.control=fresh;
    release(old);
}
void destroy_024db0(SocketObject& object,LifecycleEffects& effects){
    auto* const service=object.service;
    if(!service)throw UnsupportedInput("Destructor requires a live service projection");
    effects.close_and_cancel_026fb0(*service,object);
    service->gate.enter();
    if(service->head==&object)service->head=object.next;
    if(object.previous)object.previous->next=object.next;
    if(object.next)object.next->previous=object.previous;
    object.next=nullptr;object.previous=nullptr;
    service->gate.leave();
    effects.executor_cleanup(object,object.executor.table_78);
    release(object.cancellation.control); // loaded after cleanup; no slot clear
}
void LifecycleRuntime::initialize_0211b0(AcceptorState& state){
    if(&state!=&object_.state)throw UnsupportedInput("LifecycleRuntime is bound to one socket state");
    (void)socket_recovered::initialize_0211b0(object_,context_,effects_);
}
void LifecycleRuntime::reset_cancellation_0237f0(AcceptorState& state){
    if(&state!=&object_.state)throw UnsupportedInput("LifecycleRuntime is bound to one socket state");
    replace_owner_0237f0(object_.cancellation,0,factory_);
}
void LifecycleRuntime::destroy_socket_024db0(){destroy_024db0(object_,effects_);}
} // namespace anadius::socket_recovered
