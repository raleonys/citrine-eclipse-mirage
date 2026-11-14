// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_service_constructor.hpp"

namespace laureondo::socket {
// A typed projection of the same registry used for socket services. The owner
// context corresponds to native registry+0x28; it is not the registry identity.
struct SchedulerRegistry {
    explicit SchedulerRegistry(ServiceRegistry& services):services(services){}
    ServiceRegistry& services;
    std::uintptr_t owner_context=0;
};
class SchedulerRegistryEffects {
public:
    virtual ~SchedulerRegistryEffects()=default;
    virtual std::uint32_t compare_type_info_04d2d0(std::uintptr_t left_name,std::uintptr_t right_name)=0;
    // Allocation supplies a live typed C++ projection;176 is the observed native
    // byte request, not sizeof(ServiceEntry) or a reconstructed native allocator.
    // A nonnull constructor result is an exclusively writable, fresh unlinked
    // entry and stays live through publication or its scalar-deleting callback.
    virtual ServiceEntry* allocate_04b0bc(std::uint32_t native_bytes)=0;
    virtual ServiceEntry* construct_scheduler_024990(ServiceEntry& allocation,
        std::uintptr_t owner_context,std::uint32_t concurrency_hint,std::uint8_t own_thread)=0;
};
void initialize_scheduler_key_022fd0(ServiceKey&);
// Normal returning effects and finite live list only; callbacks keep the entries
// read by the resolver alive through its final return. The native null-candidate
// key-store fault is UnsupportedInput, not a fabricated successful empty result.
ServiceEntry& resolve_scheduler_023b30(SchedulerRegistry&,SchedulerRegistryEffects&);

struct SchedulerRegistryBinding {
    std::uintptr_t registry_identity=0;
    SchedulerRegistry* registry=nullptr;
};
struct SchedulerServiceBinding {
    ServiceEntry* entry=nullptr;
    std::uintptr_t scheduler_identity=0; // Opaque projection token, never executed.
};
// Selected borrowed source composition; no registry ownership, scheduler native
// layout, port projection, IOCP/thread creation, teardown or unwind is invented.
// Both tables and their referenced objects stay live and stable through callbacks,
// with one binding per registry identity / service entry.
class SchedulerConstructionEffects:public ServiceConstructionEffects {
public:
    SchedulerConstructionEffects(std::span<SchedulerRegistryBinding> registries,
        SchedulerRegistryEffects& effects,std::span<SchedulerServiceBinding> services)
        :registries_(registries),effects_(effects),services_(services){}
    std::uintptr_t resolve_scheduler_023b30(std::uintptr_t registry_identity) final;
    // Socket allocation(0x80) and raise stay abstract in ServiceConstructionEffects.
private:
    std::span<SchedulerRegistryBinding> registries_;SchedulerRegistryEffects& effects_;
    std::span<SchedulerServiceBinding> services_;
};
} // namespace laureondo::socket
