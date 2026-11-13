// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_service_registry.hpp"

namespace citrine_eclipse_mirage::socket {
namespace {
bool matches(ServiceEntry& entry,const ServiceKey& key,ServiceRegistryEffects& effects){
    // The fixed022fa0 key has id0; keep the native short-circuit predicate without
    // adding an input that would make its ID-match branch reachable in0239c0.
    if(entry.key.id && key.id && entry.key.id==key.id)return true;
    return entry.key.type && key.type
        && effects.compare_type_info_04d2d0(entry.key.type+8,key.type+8)==0;
}
ServiceEntry* find(ServiceRegistry& registry,const ServiceKey& key,ServiceRegistryEffects& effects){
    for(auto* entry=registry.head;entry;entry=entry->next)
        if(matches(*entry,key,effects))return entry;
    return nullptr;
}
}
void initialize_service_key_022fa0(ServiceKey& key){
    key.type=0x1800a2620ULL;key.id=0;
}
ServiceEntry& resolve_service_0239c0(ServiceRegistry& registry,ContextIdentity context,ServiceRegistryEffects& effects){
    ServiceKey key;initialize_service_key_022fa0(key);
    registry.gate.enter();
    if(auto* existing=find(registry,key,effects)){
        registry.gate.leave();return *existing;
    }
    registry.gate.leave();
    auto* const fresh=effects.create_socket_service_0228e0(context);
    if(!fresh)throw UnsupportedInput("Service constructor must return a live entry; native null result faults at key store");
    fresh->key=key;
    registry.gate.enter();
    if(auto* existing=find(registry,key,effects)){
        // Native deleting destructor runs while the second gate is held. It may
        // mutate the registry; the selected existing entry remains cached.
        fresh->scalar_delete(1);
        registry.gate.leave();return *existing;
    }
    fresh->next=registry.head;
    registry.head=fresh;
    registry.gate.leave();return *fresh;
}
SocketService& RegistryLifecycleEffects::resolve_service_0239c0(ContextIdentity context){
    if(context.value!=context_.value)throw UnsupportedInput("Registry projection is bound to one context identity");
    auto& entry=socket::resolve_service_0239c0(registry_,context,registry_effects_);
    for(const auto& binding:bindings_)
        if(binding.entry==&entry && binding.service && binding.close_context)return *binding.service;
    throw UnsupportedInput("Resolved registry entry has no live service/context projection");
}
void RegistryLifecycleEffects::close_and_cancel_026fb0(SocketService& service,SocketObject& object){
    for(const auto& binding:bindings_){
        if(binding.entry && binding.service==&service && binding.close_context){
            cancel_and_close_026fb0(*binding.close_context,object,errors_,api_,cancellation_);return;
        }
    }
    throw UnsupportedInput("Socket service has no close-context projection");
}
void RegistryLifecycleEffects::executor_cleanup(SocketObject& object,std::uintptr_t table){
    executor_.executor_cleanup(object,table);
}
} // namespace citrine_eclipse_mirage::socket
