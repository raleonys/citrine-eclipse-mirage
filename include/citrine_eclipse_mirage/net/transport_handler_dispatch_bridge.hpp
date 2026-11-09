// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_handler_dispatch.hpp"

namespace citrine_eclipse_mirage::socket {
// Supplies an IO-completion scheduling provider with explicit source-linked behavior.
// Native factory/vtable slots and the actual025c40/0261d0 algorithms remain
// mandatory. The stable context and this object must outlive outstanding tasks.
class HandlerDispatchEffects:public IoCompletionEffects {
public:
    HandlerDispatchEffects(IoCompletionEnvironment environment,WorkerRunApi& api,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules)
        :context_{environment,api,memory,*this,schedules}{}
    HandlerDispatchEffects(const HandlerDispatchEffects&)=delete;
    HandlerDispatchEffects& operator=(const HandlerDispatchEffects&)=delete;
    void schedule_send_0229d0(IoHandlerEnvironment&,SendHandler&) final;
    void schedule_receive_022c60(IoHandlerEnvironment&,ReceiveHandler&) final;
    HandlerTaskContext& handler_context(){return context_;}
private:HandlerTaskContext context_;
};
} // namespace citrine_eclipse_mirage::socket
