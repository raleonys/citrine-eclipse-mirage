// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_scheduler_registry.hpp"

namespace laureondo::socket {
namespace {
bool matches(ServiceEntry& entry,const ServiceKey& key,SchedulerRegistryEffects& effects){
    if(entry.key.id && key.id && entry.key.id==key.id)return true;
    return entry.key.type && key.type
        && effects.compare_type_info_04d2d0(entry.key.type+8,key.type+8)==0;
}
ServiceEntry* find(ServiceRegistry& registry,const ServiceKey& key,SchedulerRegistryEffects& effects){
    for(auto* entry=registry.head;entry;entry=entry->next)
        if(matches(*entry,key,effects))return entry;
    return nullptr;
}
}
void initialize_scheduler_key_022fd0(ServiceKey& key){key.type=0x1800a2540ULL;key.id=0;}
ServiceEntry& resolve_scheduler_023b30(SchedulerRegistry& state,SchedulerRegistryEffects& effects){
    ServiceKey key;initialize_scheduler_key_022fd0(key);
    const auto owner_context=state.owner_context; // Native023b5a, before first gate.
    auto& registry=state.services;
    registry.gate.enter();
    if(auto* existing=find(registry,key,effects)){
        registry.gate.leave();return *existing;
    }
    registry.gate.leave();
    auto* const allocation=effects.allocate_04b0bc(0xb0);
    auto* const fresh=allocation
        ?effects.construct_scheduler_024990(*allocation,owner_context,0xffffffffU,1)
        :nullptr;
    if(!fresh)throw UnsupportedInput("Scheduler construction requires a live entry; native null key store faults");
    fresh->key=key;
    registry.gate.enter();
    if(auto* existing=find(registry,key,effects)){
        fresh->scalar_delete(1);
        registry.gate.leave();return *existing;
    }
    fresh->next=registry.head;
    registry.head=fresh;
    registry.gate.leave();return *fresh;
}
std::uintptr_t SchedulerConstructionEffects::resolve_scheduler_023b30(std::uintptr_t identity){
    for(const auto& binding:registries_){
        if(binding.registry && binding.registry_identity==identity){
            auto& entry=socket::resolve_scheduler_023b30(*binding.registry,effects_);
            for(const auto& service:services_)
                if(service.entry==&entry)return service.scheduler_identity;
            throw UnsupportedInput("Resolved scheduler entry has no identity projection");
        }
    }
    throw UnsupportedInput("Scheduler resolver has no registry identity binding");
}
} // namespace laureondo::socket
