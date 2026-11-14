// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_completion_dispatcher.hpp"
#include <algorithm>
#include <bit>

namespace laureondo::socket {
namespace {
CancelOperation* pop(DispatcherQueue& queue){
    auto* const operation=queue.head;
    queue.head=operation->next;
    if(!queue.head)queue.tail=nullptr;
    operation->next=nullptr;
    return operation;
}
DispatcherOperationBinding& binding_for(DispatcherState& state,OVERLAPPED* raw){
    for(auto& binding:state.operations)
        if(binding.operation && binding.overlapped_identity==raw)return binding;
    throw UnsupportedInput("Completion pointer has no live borrowed operation binding");
}
void success(WorkerRunResult& result,Category category){result.error=Error{0,category};}
void failure(WorkerRunResult& result,std::uint32_t status,Api& errors){
    const auto category=errors.system_category_028620();
    result.error=Error{std::bit_cast<std::int32_t>(status),category};
}
}
std::int64_t dispatcher_timer_due(std::int32_t wait){
    const auto negated=std::uint32_t{0}-std::bit_cast<std::uint32_t>(wait);
    return static_cast<std::int64_t>(std::bit_cast<std::int32_t>(negated))*10;
}
std::uint64_t dispatch_completion_027c10(DispatcherState& state,std::uint32_t requested,
    WorkerThreadState& thread,WorkerRunResult& result,DispatcherEnvironment environment,
    DispatcherApi& api,DispatcherEffects& effects,WorkerRunApi& worker_api,Api& errors,WorkerRunEffects& worker){
    auto& entry=state.entry;
    auto& scheduler=entry.completion;
    const auto tls_slot=effects.capture_tls_slot();
    for(;;){
        std::int32_t expected=1;
        if(scheduler.dispatch_required.compare_exchange_strong(expected,0,std::memory_order_seq_cst)){
            scheduler.gate.enter();
            DispatcherQueue queue;
            if(scheduler.head){
                queue.head=scheduler.head;queue.tail=scheduler.tail;
                scheduler.head=nullptr;scheduler.tail=nullptr;
            }
            for(auto* service=state.services_88;service;service=service->next)
                service->collect_slot_20(queue);
            while(queue.head){
                auto* const operation=pop(queue);
                operation->queued=1;
                if(api.post(scheduler.port,0,0,&operation->overlapped)==0){
                    scheduler.gate.enter();
                    operation->next=nullptr;
                    if(scheduler.tail)scheduler.tail->next=operation;
                    else scheduler.head=operation;
                    scheduler.tail=operation;
                    if(queue.head){operation->next=queue.head;scheduler.tail=queue.tail;queue.head=nullptr;queue.tail=nullptr;}
                    (void)scheduler.dispatch_required.exchange(1,std::memory_order_seq_cst);
                    scheduler.gate.leave();
                }
            }
            if(state.timer_enabled_48){
                std::int32_t wait=300000000;
                auto* service=state.services_88;
                if(service){
                    do{wait=service->wait_slot_18(wait);service=service->next;}while(service);
                    if(wait<300000000){
                        LARGE_INTEGER due{};due.QuadPart=dispatcher_timer_due(wait);
                        (void)api.set_timer(state.timer_50,due,300000,nullptr,nullptr,0);
                    }
                }
            }
            while(queue.head){
                auto* const operation=pop(queue);
                const Error error{0,environment.generic_category};
                effects.complete(nullptr,*operation,error,0);
            }
            scheduler.gate.leave();
        }
        std::uint32_t bytes=0;
        std::uintptr_t key=0;
        OVERLAPPED* raw=nullptr;
        api.set_last_error(0);
        const auto timeout=std::min(requested,entry.timeout_40);
        const auto received=api.get(scheduler.port,bytes,key,raw,timeout);
        const auto status=api.get_last_error();
        if(raw){
            const auto epoch=effects.thread_epoch(tls_slot);
            if(environment.guard_epoch_a38e8>epoch){
                effects.guard_header_04b838(environment.guard_epoch_a38e8);
                if(environment.guard_epoch_a38e8==-1){
                    effects.register_atexit_04b770(0x18007dff0ULL);
                    effects.guard_footer_04b7cc(environment.guard_epoch_a38e8);
                }
            }
            auto& binding=binding_for(state,raw);
            auto* const operation=binding.operation;
            Error error{std::bit_cast<std::int32_t>(status),environment.system_category};
            if(key==2){error=binding.packet_error;bytes=binding.transferred_14;}
            else{binding.packet_error=error;binding.transferred_14=bytes;}
            std::uint32_t previous=0;
            {
                std::atomic_ref<std::uint32_t> queued(operation->queued);
                (void)queued.compare_exchange_strong(previous,1,std::memory_order_seq_cst);
            }
            if(previous!=1)continue;
            effects.complete(&entry,*operation,error,bytes);
            if(std::bit_cast<std::int32_t>(thread.pending_50)>0){
                thread.pending_50=0;
                std::array<std::uintptr_t,2> first,second;
                (void)effects.copy_exception_024100(first,thread.context_58);
                auto& copied=effects.copy_exception_024100(second,first);
                effects.rethrow_029880(copied);
            }
            success(result,environment.generic_category);
            const auto previous_work=entry.work_count_30.fetch_sub(1,std::memory_order_seq_cst);
            if(previous_work==1)stop_worker_02a190(entry,worker_api,errors,worker);
            return 1;
        }
        if(received==0){
            if(status!=0x102U){failure(result,status,errors);return 0;}
            if(requested==0xffffffffU)continue;
            success(result,environment.generic_category);return 0;
        }
        if(key==1)continue;
        (void)worker.exchange_wake_38(entry,0);
        if(effects.read_stop_34(entry)==0)continue;
        if(worker.exchange_wake_38(entry,1)==0){
            if(api.post(scheduler.port,0,0,nullptr)==0){
                const auto relay_status=api.get_last_error();
                failure(result,relay_status,errors);return 0;
            }
        }
        success(result,environment.generic_category);return 0;
    }
}
std::uint64_t DispatcherWorkerEffects::dispatch_027c10(IocpSchedulerEntry& entry,
    std::uint32_t timeout,WorkerThreadState& thread,WorkerRunResult& result){
    if(&entry!=&state_.entry)throw UnsupportedInput("Dispatcher bridge is bound to one live scheduler entry");
    return dispatch_completion_027c10(state_,timeout,thread,result,environment_,api_,effects_,worker_api_,errors_,*this);
}
} // namespace laureondo::socket
