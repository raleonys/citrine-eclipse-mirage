// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_service_constructor.hpp"
#include <bit>

namespace preloader_l::socket {
std::uint32_t initialize_gate_027be0(CRITICAL_SECTION& section,ServiceGateApi& api){
    if(api.initialize(section,0x80000000U)!=0)return 0;
    return api.get_last_error();
}
SocketServiceEntry* construct_service_0228e0(SocketConstructionContext& context,
    ServiceConstructionEffects& effects,ServiceGateApi& gate_api,Api& errors){
    const auto identity=context.identity; // Native RBX caches argument before allocation.
    auto* const entry=effects.allocate_04b0bc(0x80);
    if(!entry)return nullptr;
    entry->key.type=0;entry->key.id=0;
    entry->owner_context=identity;
    entry->next=nullptr;
    entry->table_identity=0x180084590ULL;
    entry->implementation_context=identity;
    entry->scheduler_identity=effects.resolve_scheduler_023b30(context.registry_identity);
    entry->word_38=0;entry->word_40=0;entry->word_48=0;
    const auto status=initialize_gate_027be0(entry->section,gate_api);
    const auto category=errors.system_category_028620();
    if(status!=0)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},"mutex");
    entry->service.head=nullptr;
    entry->table_identity=0x1800845b0ULL;
    return entry;
}
ServiceEntry* ConstructorRegistryEffects::create_socket_service_0228e0(ContextIdentity context){
    if(context.value!=context_.identity.value)throw UnsupportedInput("Service constructor projection is bound to one context identity");
    return construct_service_0228e0(context_,effects_,gate_api_,errors_);
}
} // namespace preloader_l::socket
