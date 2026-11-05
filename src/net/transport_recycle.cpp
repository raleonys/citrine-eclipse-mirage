// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_recycle.hpp"

namespace preloader_l::socket {
void recycle_0283c0(RecycleMap& state,CancelNode& node,RecycleFailureEffects& failures){
    auto& map=state.map;
    if(map.buckets.empty())throw UnsupportedInput("Recycler requires nonzero initialized bucket count");
    auto& bucket=map.buckets[node.socket%map.buckets.size()];
    auto* const first=bucket.first;
    auto* const last=bucket.last;
    if(&node==first){
        if(&node==last){bucket.last=map.sentinel;bucket.first=bucket.last;}
        else{
            if(!first)throw UnsupportedInput("Recycler bucket first is not initialized");
            bucket.first=first->next;
        }
    }else if(&node==last){
        if(!last)throw UnsupportedInput("Recycler bucket last is not initialized");
        bucket.last=last->previous;
    }
    node.socket=0;
    auto* const free_first=state.free_sentinel.next;
    auto* const after=node.next;
    // Native compares two distinct list subobject addresses; normal flow always
    // reaches this check. Counts above the limit are not changed into >= checks.
    if(state.free_size==recycle_limit)failures.length_error_047fc4(0x1800846b8ULL);
    ++state.free_size;--state.active_size;
    auto* const previous=node.previous;
    if(!previous)throw UnsupportedInput("Recycler previous node is not initialized");
    previous->next=after;
    if(!after || !after->previous)throw UnsupportedInput("Recycler successor links are not initialized");
    auto* const old_after_previous=after->previous;
    old_after_previous->next=free_first;
    if(!free_first || !free_first->previous)throw UnsupportedInput("Recycler free-list links are not initialized");
    auto* const old_free_previous=free_first->previous;
    old_free_previous->next=&node;
    free_first->previous=old_after_previous;
    after->previous=previous;
    node.previous=old_free_previous;
    --state.map_size;
}
void CancellationRecyclerAdapter::recycle_0283c0(CancelMap& map,CancelNode& node){
    for(auto* metadata:maps_){
        if(metadata && metadata->map.buckets.data()==map.buckets.data()
            && metadata->map.buckets.size()==map.buckets.size()
            && metadata->map.sentinel==map.sentinel){
            socket::recycle_0283c0(*metadata,node,failures_);return;
        }
    }
    throw UnsupportedInput("Cancellation recycler has no metadata for this map");
}
} // namespace preloader_l::socket
