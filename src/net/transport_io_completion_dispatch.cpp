// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_io_completion_dispatch.hpp"
namespace laureondo::socket {
void IoDispatcherEffects::complete_other(IocpSchedulerEntry* owner,CancelOperation& operation,const Error& error,std::uint32_t bytes){
    for(auto& binding:bindings_){
        if(binding.identity!=&operation)continue;
        if(binding.receive && binding.send)throw UnsupportedInput("Ambiguous live IO operation projection");
        if(binding.receive){
            complete_receive_027710(reinterpret_cast<std::uintptr_t>(owner),*binding.receive,error,bytes,environment_,api_,memory_,errors_,effects_);
            return;
        }
        if(binding.send){
            complete_send_027a20(reinterpret_cast<std::uintptr_t>(owner),*binding.send,error,bytes,environment_,api_,memory_,errors_,effects_);
            return;
        }
    }
    complete_unknown(owner,operation,error,bytes);
}
} // namespace laureondo::socket
