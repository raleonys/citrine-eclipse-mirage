#pragma once
#include "handler_dispatch.hpp"

namespace anadius::socket_recovered {
// Fills the previous IO-completion scheduling provider with recovered source.
// Native factory/vtable slots and the actual025c40/0261d0 algorithms remain
// mandatory. The stable context and this object must outlive outstanding tasks.
class RecoveredHandlerDispatchEffects:public IoCompletionEffects {
public:
    RecoveredHandlerDispatchEffects(IoCompletionEnvironment environment,WorkerRunApi& api,
        HandlerDispatchMemory& memory,std::span<HandlerScheduleBinding> schedules)
        :context_{environment,api,memory,*this,schedules}{}
    RecoveredHandlerDispatchEffects(const RecoveredHandlerDispatchEffects&)=delete;
    RecoveredHandlerDispatchEffects& operator=(const RecoveredHandlerDispatchEffects&)=delete;
    void schedule_send_0229d0(IoHandlerEnvironment&,SendHandler&) final;
    void schedule_receive_022c60(IoHandlerEnvironment&,ReceiveHandler&) final;
    HandlerTaskContext& handler_context(){return context_;}
private:HandlerTaskContext context_;
};
} // namespace anadius::socket_recovered
