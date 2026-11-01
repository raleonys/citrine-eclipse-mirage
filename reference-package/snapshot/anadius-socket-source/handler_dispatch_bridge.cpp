#include "handler_dispatch_bridge.hpp"
namespace anadius::socket_recovered {
void RecoveredHandlerDispatchEffects::schedule_send_0229d0(IoHandlerEnvironment& environment,SendHandler& handler){
    schedule_send_handler_0229d0(environment,handler,context_);
}
void RecoveredHandlerDispatchEffects::schedule_receive_022c60(IoHandlerEnvironment& environment,ReceiveHandler& handler){
    schedule_receive_handler_022c60(environment,handler,context_);
}
} // namespace anadius::socket_recovered
