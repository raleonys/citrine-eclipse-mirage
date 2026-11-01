#pragma once
#include "cancellation.hpp"

namespace anadius::socket_recovered {
// Opaque image identities only; neither field is an executable pointer.
struct ServiceKey {std::uintptr_t type=0,id=0;};
struct ServiceEntry {
    ServiceKey key;
    ServiceEntry* next=nullptr;
    virtual ~ServiceEntry()=default;
    virtual void scalar_delete(std::uint32_t flags)=0;
};
struct ServiceRegistry {
    explicit ServiceRegistry(SocketGate& gate):gate(gate){}
    SocketGate& gate;
    ServiceEntry* head=nullptr;
};
class ServiceRegistryEffects {
public:
    virtual ~ServiceRegistryEffects()=default;
    // Required native CRT RTTI comparison and socket-service construction.
    // Name addresses are opaque type-descriptor+8 identities, not dereferenced.
    virtual std::uint32_t compare_type_info_04d2d0(std::uintptr_t left_name,std::uintptr_t right_name)=0;
    virtual ServiceEntry* create_socket_service_0228e0(ContextIdentity)=0;
};
void initialize_service_key_022fa0(ServiceKey&);
// Borrowed finite initialized list; factory yields a fresh live unlinked entry.
// Normal-flow gates only: native C++ exception/unwind cleanup is not recovered.
ServiceEntry& resolve_service_0239c0(ServiceRegistry&,ContextIdentity,ServiceRegistryEffects&);

// Selected typed projection of a native service/context relationship. Entries,
// services, contexts and this binding table all remain caller-owned and stable.
struct RegistryServiceBinding {
    ServiceEntry* entry=nullptr;
    SocketService* service=nullptr;
    CloseContext* close_context=nullptr;
};
class RegistryExecutorEffects {
public:
    virtual ~RegistryExecutorEffects()=default;
    virtual void executor_cleanup(SocketObject&,std::uintptr_t table_identity)=0;
};
// Source composition, not a native ASIO registry/service memory overlay or owner.
// One context/registry and one unambiguous binding per entry/service are required.
class RegistryLifecycleEffects final:public LifecycleEffects {
public:
    RegistryLifecycleEffects(ServiceRegistry& registry,ContextIdentity context,
        ServiceRegistryEffects& registry_effects,std::span<RegistryServiceBinding> bindings,
        Api& errors,AcceptorApi& api,CancellationEffects& cancellation,RegistryExecutorEffects& executor)
        :registry_(registry),context_(context),registry_effects_(registry_effects),bindings_(bindings),
         errors_(errors),api_(api),cancellation_(cancellation),executor_(executor){}
    SocketService& resolve_service_0239c0(ContextIdentity) override;
    void close_and_cancel_026fb0(SocketService&,SocketObject&) override;
    void executor_cleanup(SocketObject&,std::uintptr_t table_identity) override;
private:
    ServiceRegistry& registry_;ContextIdentity context_;ServiceRegistryEffects& registry_effects_;
    std::span<RegistryServiceBinding> bindings_;Api& errors_;AcceptorApi& api_;
    CancellationEffects& cancellation_;RegistryExecutorEffects& executor_;
};
} // namespace anadius::socket_recovered
