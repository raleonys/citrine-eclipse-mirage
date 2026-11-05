#include "posted_operation.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
namespace {
OperationStorage& storage_at(std::uintptr_t identity,PostedEnvironment environment){
    for(auto& binding:environment.current_storage())
        if(binding.identity==identity && binding.storage)return *binding.storage;
    throw UnsupportedInput("Cache identity has no live borrowed storage binding");
}
std::byte read_byte(const OperationStorage& storage,std::uint64_t index){
    if(index>=storage.bytes.size())throw UnsupportedInput("Native storage read leaves bounded projection");
    return storage.bytes[static_cast<std::size_t>(index)];
}
void write_byte(OperationStorage& storage,std::uint64_t index,std::byte value){
    if(index>=storage.bytes.size())throw UnsupportedInput("Native storage write leaves bounded projection");
    storage.bytes[static_cast<std::size_t>(index)]=value;
}
}
OperationStorage& allocate_cached_021820(std::uint8_t tag,WorkerThreadState* thread,
    std::uint64_t requested,std::uint64_t alignment,PostedEnvironment environment,PostedMemoryEffects& memory){
    (void)tag;
    const auto units=(requested+std::uint64_t{3})>>2;
    if(thread){
        for(std::size_t index=0;index<2;++index){
            const auto identity=thread->cache_00[index];
            if(!identity)continue;
            auto& storage=storage_at(identity,environment);
            if(units>std::to_integer<std::uint8_t>(read_byte(storage,0)))continue;
            if(alignment==0)throw UnsupportedInput("Native cache alignment DIV has a zero divisor");
            if(identity%alignment!=0)continue;
            thread->cache_00[index]=0;
            write_byte(storage,requested,read_byte(storage,0));
            return storage;
        }
        for(std::size_t index=0;index<2;++index){
            const auto identity=thread->cache_00[index];
            if(!identity)continue;
            thread->cache_00[index]=0;
            memory.free_04b788(storage_at(identity,environment));
            break;
        }
    }
    auto& storage=memory.allocate_04b0bc(units*4+1);
    write_byte(storage,requested,static_cast<std::byte>(units>255?0:static_cast<std::uint8_t>(units)));
    return storage;
}
void consume_functor_021ac0(PostedFunctor*& slot){
    auto* const functor=slot;
    if(functor){slot=nullptr;functor->call_slot_00(1);}
}
void cleanup_posted_028fd0(PostedCleanupFrame& frame,PostedEnvironment environment,
    WorkerRunApi& api,PostedMemoryEffects& memory){
    if(auto* const operation=frame.operation_10){
        if(auto* const functor=operation->functor_38)functor->call_slot_00(0);
        frame.operation_10=nullptr;
    }
    auto* const allocation=frame.allocation_08;
    if(!allocation)return;
    auto* const context=static_cast<WorkerTlsContext*>(api.tls_get(environment.tls_index));
    if(context){
        if(auto* const thread=context->state){
            for(std::size_t index=0;index<2;++index){
                if(thread->cache_00[index])continue;
                write_byte(*allocation,0,read_byte(*allocation,0x48));
                thread->cache_00[index]=allocation->identity;
                frame.allocation_08=nullptr;
                return;
            }
        }
    }
    memory.free_04b788(*allocation);
    frame.allocation_08=nullptr;
}
void enqueue_posted_028ca0(IocpSchedulerEntry& entry,CancelOperation& operation,std::uint8_t hint,DispatcherApi& api){
    (void)hint;
    entry.work_count_30.fetch_add(1,std::memory_order_seq_cst);
    operation.queued=1;
    if(api.post(entry.completion.port,0,0,&operation.overlapped)!=0)return;
    auto& scheduler=entry.completion;
    scheduler.gate.enter();
    operation.next=nullptr;
    if(scheduler.tail)scheduler.tail->next=&operation;
    else scheduler.head=&operation;
    scheduler.tail=&operation;
    (void)scheduler.dispatch_required.exchange(1,std::memory_order_seq_cst);
    scheduler.gate.leave();
}
void dispatch_posted_022e40(PostedExecutor& executor,PostedFunctor*& source,PostedEnvironment environment,
    WorkerRunApi& api,DispatcherApi& completion,PostedMemoryEffects& memory){
    const auto executor_identity=executor.identity;
    if((executor.flags_0c&1U)==0){
        auto* const scheduler=executor.scheduler;
        auto* const key=scheduler?&scheduler->completion.port:nullptr;
        auto* context=static_cast<WorkerTlsContext*>(api.tls_get(environment.tls_index));
        for(;context;context=static_cast<WorkerTlsContext*>(context->previous)){
            if(context->port_key!=key)continue;
            if(context->state){
                auto* local=source;source=nullptr;
                consume_functor_021ac0(local);
                if(local)local->call_slot_00(0);
                return;
            }
            break;
        }
    }
    auto* const context=static_cast<WorkerTlsContext*>(api.tls_get(environment.tls_index));
    auto* const thread=context?context->state:nullptr;
    auto& storage=allocate_cached_021820(static_cast<std::uint8_t>(executor_identity),thread,0x48,1,environment,memory);
    auto& operation=memory.project_posted(storage);
    operation.operation.next=nullptr;
    operation.callback_identity_28=0x180027120ULL;
    if(storage.bytes.size()<sizeof(OVERLAPPED))throw UnsupportedInput("Posted OVERLAPPED leaves bounded allocation");
    std::fill_n(storage.bytes.begin(),sizeof(OVERLAPPED),std::byte{});
    operation.operation.overlapped={};
    operation.operation.queued=0;
    operation.functor_38=source;source=nullptr;
    auto* const live_scheduler=executor.scheduler;
    if(!live_scheduler)throw UnsupportedInput("Native posted enqueue dereferences the live scheduler");
    enqueue_posted_028ca0(*live_scheduler,operation.operation,static_cast<std::uint8_t>((executor.flags_0c>>1)&1U),completion);
    PostedCleanupFrame frame{executor_identity+8,nullptr,nullptr};
    cleanup_posted_028fd0(frame,environment,api,memory);
}
void complete_posted_027120(std::uintptr_t owner,PostedOperation& operation,PostedEnvironment environment,
    WorkerRunApi& api,PostedMemoryEffects& memory){
    auto* local=operation.functor_38;
    operation.functor_38=nullptr;
    PostedCleanupFrame frame{0,&operation.storage,&operation};
    cleanup_posted_028fd0(frame,environment,api,memory);
    if(owner)consume_functor_021ac0(local);
    if(local)local->call_slot_00(0);
    cleanup_posted_028fd0(frame,environment,api,memory);
}
} // namespace anadius::socket_recovered
