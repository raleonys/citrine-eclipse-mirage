#include "posted_operation_dispatch.hpp"

namespace anadius::socket_recovered {
void PostedDispatcherEffects::complete(IocpSchedulerEntry* owner,CancelOperation& operation,
    const Error& error,std::uint32_t bytes){
    for(auto& binding:bindings_){
        if(binding.identity!=&operation || !binding.posted)continue;
        auto* const posted=binding.posted;
        complete_posted_027120(reinterpret_cast<std::uintptr_t>(owner),*posted,environment_,api_,memory_);
        return;
    }
    complete_other(owner,operation,error,bytes);
}
} // namespace anadius::socket_recovered
