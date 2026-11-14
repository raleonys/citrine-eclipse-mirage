// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_close.hpp"
#include "laureondo/net/transport_lifecycle.hpp"

namespace laureondo::socket {
struct CancellationEngine {
    explicit CancellationEngine(SocketGate& gate):gate(gate){}
    SocketGate& gate;
};
struct CloseContext {std::atomic<CancellationEngine*> engine{nullptr};};
class CancellationEffects {
public:
    virtual ~CancellationEffects()=default;
    virtual void cancel_026aa0(CancellationEngine&,Socket,const Error& reason)=0;
};
// Context/engine are caller-owned typed projections, never native ASIO overlays.
// Engine capture is CAS(expected=null,desired=null), not destructive exchange.
void cancel_and_close_026fb0(CloseContext&,SocketObject&,Api& errors,
    AcceptorApi&,CancellationEffects&);
// Replaces prior LifecycleEffects close boundary with actual026fb0+026e50.
// Service resolution and executor cleanup remain abstract. The supplied context
// must belong to this service and outlive all operations/cached engine uses.
class ClosingLifecycleEffects:public LifecycleEffects {
public:
    ClosingLifecycleEffects(SocketService& service,CloseContext& context,
        Api& errors,AcceptorApi& api,CancellationEffects& cancellation)
        :service_(service),context_(context),errors_(errors),api_(api),cancellation_(cancellation){}
    void close_and_cancel_026fb0(SocketService&,SocketObject&) final;
private:
    SocketService& service_;CloseContext& context_;Api& errors_;AcceptorApi& api_;
    CancellationEffects& cancellation_;
};
} // namespace laureondo::socket
