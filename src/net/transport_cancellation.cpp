// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_cancellation.hpp"

namespace citrine_eclipse_mirage::socket {
void wake_028850(const WakeState& wake,Api& api){
    char zero=0;
    Buffer buffer{1,&zero};
    (void)api.wsa_send(wake.socket,std::span<Buffer>(&buffer,1),0);
    (void)api.system_category_028620();
    const auto error=api.get_last_error();
    if(error==64 || error==1234)(void)api.system_category_028620();
}
void cancel_queue_026aa0(CancellationState& state,Socket socket,const Error& reason,
    CancellationRecycler& recycler,CompletionApi& completion,Api& wake_api){
    CancelOperation* first=nullptr;CancelOperation* last=nullptr;
    bool matched=false;
    for(auto& map:state.maps){
        if(map.buckets.empty())continue;
        auto& bucket=map.buckets[socket%map.buckets.size()];
        auto* node=bucket.first;
        if(node==map.sentinel)continue;
        if(!bucket.last)throw UnsupportedInput("Nonempty cancellation bucket requires initialized last node");
        auto* const end=bucket.last->next;
        while(node!=end){
            if(!node)throw UnsupportedInput("Cancellation node chain leaves initialized storage");
            if(node->socket==socket){
                if(node==map.sentinel)break;
                while(auto* operation=node->head){
                    operation->error=reason;
                    node->head=operation->next;
                    if(!node->head)node->tail=nullptr;
                    operation->next=nullptr;
                    if(last)last->next=operation;
                    else first=operation;
                    last=operation;
                }
                recycler.recycle_0283c0(map,*node);
                matched=true;
                break;
            }
            node=node->next;
        }
    }
    auto* const scheduler=state.scheduler; // native loads identity after all four maps
    while(first){
        if(!scheduler)throw UnsupportedInput("Pending cancellation requires a live completion scheduler");
        auto* const operation=first;
        first=operation->next;
        if(!first)last=nullptr;
        operation->next=nullptr;
        operation->queued=1;
        if(completion.post(scheduler->port,*operation)!=0)continue;
        scheduler->gate.enter();
        operation->next=nullptr;
        if(scheduler->tail)scheduler->tail->next=operation;
        else scheduler->head=operation;
        scheduler->tail=operation;
        if(first){operation->next=first;scheduler->tail=last;}
        (void)scheduler->dispatch_required.exchange(1,std::memory_order_seq_cst);
        scheduler->gate.leave();
        break; // failed operation plus all remaining were transferred together
    }
    if(matched)wake_028850(state.wake,wake_api);
}
void QueueCancellationEffects::cancel_026aa0(CancellationEngine& engine,Socket socket,const Error& reason){
    if(&engine!=&engine_)throw UnsupportedInput("Cancellation model is bound to one engine");
    cancel_queue_026aa0(state_,socket,reason,recycler_,completion_,wake_api_);
}
} // namespace citrine_eclipse_mirage::socket
