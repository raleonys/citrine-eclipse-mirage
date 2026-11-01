// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_closing.hpp"

namespace preloader_l::socket {
void cancel_and_close_026fb0(CloseContext& context,SocketObject& object,Api& errors,
    AcceptorApi& api,CancellationEffects& cancellation){
    if(object.state.socket!=invalid_socket){
        CancellationEngine* engine=nullptr;
        (void)context.engine.compare_exchange_strong(engine,nullptr,
            std::memory_order_seq_cst,std::memory_order_seq_cst);
        if(engine){
            const auto socket=object.state.socket; // cache before engine gate
            engine->gate.enter();
            const auto category=errors.system_category_028620();
            const Error reason{995,category};
            cancellation.cancel_026aa0(*engine,socket,reason);
            engine->gate.leave();
        }
        Error ignored;
        (void)close_026e50(object.state.socket,object.state.socket_state,true,ignored,errors,api);
    }
    object.state.socket=invalid_socket;
    object.state.socket_state=0;
    object.cancellation.resource=0;
    auto* const control=object.cancellation.control;
    object.cancellation.control=nullptr;
    if(control && control->strong.fetch_sub(1,std::memory_order_seq_cst)==1){
        control->destroy_resource();
        if(control->weak.fetch_sub(1,std::memory_order_seq_cst)==1)
            control->delete_control();
    }
}
void ClosingLifecycleEffects::close_and_cancel_026fb0(SocketService& service,SocketObject& object){
    if(&service!=&service_)throw UnsupportedInput("Close context is bound to one socket service");
    cancel_and_close_026fb0(context_,object,errors_,api_,cancellation_);
}
} // namespace preloader_l::socket
