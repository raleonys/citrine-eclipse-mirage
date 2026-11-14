// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_handler_dispatch_bridge.hpp"
namespace laureondo::socket {
void HandlerDispatchEffects::schedule_send_0229d0(IoHandlerEnvironment& environment,SendHandler& handler){
    schedule_send_handler_0229d0(environment,handler,context_);
}
void HandlerDispatchEffects::schedule_receive_022c60(IoHandlerEnvironment& environment,ReceiveHandler& handler){
    schedule_receive_handler_022c60(environment,handler,context_);
}
} // namespace laureondo::socket
