#pragma once
#include "preloader_l/net/transport_service_registry.hpp"

namespace preloader_l::socket_recovered {
struct SocketConstructionContext {
    ContextIdentity identity;
    // Typed projection of the context's first native qword; read live at022929.
    std::uintptr_t registry_identity=0;
};
// Live logical object, not a 0x80-byte native allocation or ASIO memory overlay.
// The allocator supplies this dynamic type, service/gate and SDK section with
// valid lifetimes. Scalar-deleting behavior remains an explicit required effect.
struct SocketServiceEntry:ServiceEntry {
    SocketServiceEntry(SocketGate& gate,CRITICAL_SECTION& section):service(gate),section(section){}
    SocketService service;
    CRITICAL_SECTION& section;
    std::uintptr_t table_identity=0;
    ContextIdentity owner_context,implementation_context;
    std::uintptr_t scheduler_identity=0;
    std::uint64_t word_38=0,word_40=0,word_48=0;
};
class ServiceGateApi {
public:
    virtual ~ServiceGateApi()=default;
    virtual std::int32_t initialize(CRITICAL_SECTION&,std::uint32_t spin_count)=0;
    virtual std::uint32_t get_last_error()=0;
};
class ServiceConstructionEffects {
public:
    virtual ~ServiceConstructionEffects()=default;
    // Native allocation request only; the returned typed C++ object is already
    // alive, exclusively writable and unlinked. Native allocator is not rebuilt.
    virtual SocketServiceEntry* allocate_04b0bc(std::uint32_t native_bytes)=0;
    virtual std::uintptr_t resolve_scheduler_023b30(std::uintptr_t registry_identity)=0;
    [[noreturn]] virtual void raise_028240(Error,std::string_view context)=0;
};
std::uint32_t initialize_gate_027be0(CRITICAL_SECTION&,ServiceGateApi&);
// Normal-flow construction only; no invented allocation rollback, deletion or
// native exception/unwind guarantee. Projected completion_port is caller-bound,
// not a native0228e0 store; this body preserves that unrelated projection field.
SocketServiceEntry* construct_service_0228e0(SocketConstructionContext&,ServiceConstructionEffects&,ServiceGateApi&,Api& errors);
// Replaces the prior registry's required0228e0 provider with the recovered body.
// RTTI comparator stays abstract; no eager construction, ownership or teardown.
class ConstructorRegistryEffects:public ServiceRegistryEffects {
public:
    ConstructorRegistryEffects(SocketConstructionContext& context,ServiceConstructionEffects& effects,
        ServiceGateApi& gate_api,Api& errors):context_(context),effects_(effects),gate_api_(gate_api),errors_(errors){}
    ServiceEntry* create_socket_service_0228e0(ContextIdentity) final;
private:
    SocketConstructionContext& context_;ServiceConstructionEffects& effects_;
    ServiceGateApi& gate_api_;Api& errors_;
};
} // namespace preloader_l::socket_recovered
