// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_acceptor.hpp"
#include <atomic>

namespace preloader_l::socket {
struct Control {
    std::atomic<std::int32_t> strong{0},weak{0};
    std::uintptr_t resource=0;
    virtual void destroy_resource()=0;
    virtual void delete_control()=0;
protected:~Control()=default;
};
struct ResourcePair {std::uintptr_t resource=0;Control* control=nullptr;};
class ControlFactory {
public:
    virtual ~ControlFactory()=default;
    // Required original allocation effect. Returned object must have a live
    // dynamic Control type and exclusively owned, writable atomic fields.
    virtual Control* allocate_04b0bc(std::uint32_t native_request)=0;
};
// Explicit selected host backend; does not reconstruct04b0bc/CRT allocator.
// Resource destruction is a no-op, as for the observed noop_deleter owner.
class OwnedNoopControlFactory final:public ControlFactory {
public:Control* allocate_04b0bc(std::uint32_t native_request) override;
};
class SocketGate {
public:virtual ~SocketGate()=default;virtual void enter()=0;virtual void leave()=0;
};
struct SocketObject;
struct SocketService:AcceptorService {
    explicit SocketService(SocketGate& gate):gate(gate){}
    SocketGate& gate;
    SocketObject* head=nullptr;
};
struct ContextIdentity {std::uintptr_t value=0;};
struct ExecutorFields {
    ContextIdentity context;
    std::uint32_t zero_74=0;
    // Opaque identity tokens, not executable pointers or native vtables.
    std::uintptr_t table_78=0,table_88=0,table_90=0;
    bool self_reference_80=false;
};
struct SocketObject {
    explicit SocketObject(AcceptorState& state):state(state){}
    AcceptorState& state;
    SocketService* service=nullptr;
    ResourcePair cancellation;
    SocketObject* next=nullptr;
    SocketObject* previous=nullptr;
    ExecutorFields executor;
};
class LifecycleEffects {
public:
    virtual ~LifecycleEffects()=default;
    virtual SocketService& resolve_service_0239c0(ContextIdentity)=0;
    // Required low-level cancellation/socket-close path.024db0 itself does not
    // substitute025310 for026fb0 or invent its native reset/error behavior.
    virtual void close_and_cancel_026fb0(SocketService&,SocketObject&)=0;
    virtual void executor_cleanup(SocketObject&,std::uintptr_t table_identity)=0;
};
SocketObject& initialize_0211b0(SocketObject&,ContextIdentity,LifecycleEffects&);
void replace_owner_0237f0(ResourcePair&,std::uintptr_t resource,ControlFactory&);
void destroy_024db0(SocketObject&,LifecycleEffects&);
// Partial replacement for previous AcceptorRuntime's two required providers.
// Category and exception methods remain abstract. No RAII destructor implicitly
// calls024db0; caller supplies matching service/object lifetimes and scheduling.
class LifecycleRuntime:public AcceptorRuntime {
public:
    LifecycleRuntime(SocketObject& object,ContextIdentity context,LifecycleEffects& effects,ControlFactory& factory)
        :object_(object),context_(context),effects_(effects),factory_(factory){}
    void initialize_0211b0(AcceptorState&) final;
    void reset_cancellation_0237f0(AcceptorState&) final;
    void destroy_socket_024db0();
private:
    SocketObject& object_;ContextIdentity context_;LifecycleEffects& effects_;ControlFactory& factory_;
};
} // namespace preloader_l::socket
