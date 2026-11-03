// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_worker_run.hpp"
#include <bit>

namespace preloader_l::socket {
void stop_worker_02a190(IocpSchedulerEntry& scheduler,WorkerRunApi& api,Api& errors,WorkerRunEffects& effects){
    if(effects.exchange_stop_34(scheduler,1)!=0)return;
    if(effects.exchange_wake_38(scheduler,1)!=0)return;
    if(api.post_stop(scheduler.completion.port,0,0,nullptr)!=0)return;
    const auto status=api.get_last_error();
    const auto category=errors.system_category_028620();
    if(status)effects.raise_028240({std::bit_cast<std::int32_t>(status),category},0x180084530ULL);
}
std::uint64_t run_worker_0298b0(IocpSchedulerEntry& scheduler,WorkerRunResult& result,
    WorkerRunEnvironment environment,WorkerRunApi& api,Api& errors,WorkerRunEffects& effects){
    const auto work=scheduler.work_count_30.fetch_add(0,std::memory_order_seq_cst);
    if(work==0){
        stop_worker_02a190(scheduler,api,errors,effects);
        result.error=Error{0,environment.generic_category};
        return 0;
    }
    WorkerThreadState state;
    state.pending_50=0;
    state.context_58.fill(0);
    effects.initialize_04a240(state.context_58);
    state.cache_00.fill(0);
    WorkerTlsContext context{&scheduler.completion.port,&state,nullptr};
    context.previous=api.tls_get(environment.tls_index);
    (void)api.tls_set(environment.tls_index,&context);
    std::uint64_t count=0;
    auto handled=effects.dispatch_027c10(scheduler,0xffffffffU,state,result);
    while(handled!=0){
        count=increment_worker_count(count);
        handled=effects.dispatch_027c10(scheduler,0xffffffffU,state,result);
    }
    (void)api.tls_set(environment.tls_index,context.previous);
    for(std::size_t index=0;index<state.cache_00.size();++index){
        const auto pointer=state.cache_00[index];
        if(pointer)effects.free_cache_04b788(pointer);
    }
    effects.finish_04a2cc(state.context_58);
    return count;
}
void run_worker_function_029890(IocpWorkerFunction& function,WorkerRunEnvironment environment,
    WorkerRunApi& api,Api& errors,WorkerRunEffects& effects){
    auto* const scheduler=function.scheduler_18;
    if(!scheduler)throw UnsupportedInput("Native worker run dereferences scheduler18 at0298cc");
    WorkerRunResult ignored;
    (void)run_worker_0298b0(*scheduler,ignored,environment,api,errors,effects);
}
std::uintptr_t delete_worker_function_026320(IocpWorkerFunction& function,std::uint32_t flags,WorkerRunEffects& effects){
    const auto identity=reinterpret_cast<std::uintptr_t>(&function);
    function.table_identity=0x1800844b8ULL;
    if((flags&1U)!=0)effects.delete_function_04b0f8(function,32);
    return identity;
}
void WorkerFunctionEffects::delete_function_slot_00(IocpWorkerFunction& function,std::uint32_t flags){
    (void)delete_worker_function_026320(function,flags,effects_);
}
void WorkerFunctionEffects::invoke_function_slot_08(IocpWorkerFunction& function){
    run_worker_function_029890(function,environment_,api_,errors_,effects_);
}
} // namespace preloader_l::socket
