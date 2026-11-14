// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_worker_startup.hpp"
#include <bit>

namespace laureondo::socket {
void start_worker_02a000(IocpWorker& worker,IocpWorkerFunction* function,std::uint32_t stack,
    WorkerStartupApi& api,Api& errors,WorkerStartupEffects& effects){
    const auto entry_event=api.create_event(0,1,0,0);
    if(!function)throw UnsupportedInput("Native worker startup stores through function after its first CreateEventW");
    function->event_08=entry_event;
    if(!entry_event){
        const auto status=api.get_last_error();
        effects.delete_function_slot_00(*function,1);
        const auto category=errors.system_category_028620();
        if(status)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},0x1800844c8ULL);
    }
    const auto exit_event=api.create_event(0,1,0,0);
    worker.event_10=exit_event;function->event_10=exit_event;
    if(!worker.event_10){
        const auto status=api.get_last_error();
        effects.delete_function_slot_00(*function,1);
        const auto category=errors.system_category_028620();
        if(status)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},0x1800844e0ULL);
    }
    std::uint32_t thread_id=0;
    const auto thread=effects.begin_thread_05e8e0(0,stack,0x18002a4a0ULL,function,0,thread_id);
    worker.handle_08=thread;
    if(!thread){
        const auto status=api.get_last_error();
        effects.delete_function_slot_00(*function,1);
        if(entry_event)(void)api.close_handle(entry_event);
        const auto live_exit=worker.event_10;
        if(live_exit)(void)api.close_handle(live_exit);
        const auto category=errors.system_category_028620();
        if(status)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},0x1800844f4ULL);
    }
    if(entry_event){(void)api.wait_one(entry_event,0xffffffffU);(void)api.close_handle(entry_event);}
}
std::uint32_t worker_entry_02a4a0(IocpWorkerFunction& function,WorkerStartupApi& api,WorkerStartupEffects& effects){
    (void)api.set_event(function.event_08);
    effects.invoke_function_slot_08(function);
    const auto exit_event=function.event_10;
    effects.delete_function_slot_00(function,1);
    // No function access after the deleting effect; it may end object lifetime.
    (void)api.set_event(exit_event);
    (void)api.sleep_ex(0xffffffffU,1);
    return 0;
}
void WorkerStartupEffects::start_worker_02a000(IocpWorker& worker,IocpWorkerFunction* function,std::uint32_t stack){
    socket::start_worker_02a000(worker,function,stack,api_,errors_,*this);
}
std::uintptr_t WorkerStartupEffectsAdapter::begin_thread_05e8e0(std::uintptr_t security,std::uint32_t stack,
    std::uintptr_t entry,IocpWorkerFunction* argument,std::uint32_t flags,std::uint32_t& id){
    return threads::begin_thread_05e8e0(reinterpret_cast<void*>(security),stack,
        entry,reinterpret_cast<std::uintptr_t>(argument),flags,&id,threads_);
}
} // namespace laureondo::socket
