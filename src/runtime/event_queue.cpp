// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/event_queue.hpp"
#include <stdexcept>

namespace laureondo::event_queue {
std::uint64_t Storage::reserve_front_locked(){throw std::logic_error("Event storage lacks delivery reservations");}
void Storage::push_persisted_locked(std::string&,const std::function<void()>&) {
    throw std::logic_error("Event storage lacks transactional publication");
}
Mutex& mutex_02c620(Context& context) {
    auto& guard=context.guards.mutex_0a3970;
    auto& runtime=context.initialization;
    if(runtime.thread_epoch()<guard){
        runtime.header_04b838(guard);
        if(guard==-1){
            try {
                runtime.construct_mutex_04a698(context.mutex,2);
                (void)runtime.register_atexit(Cleanup::mutex_07e030);
            } catch(...) {runtime.abort_initialization(guard);throw;}
            runtime.footer(guard);
        }
    }
    return context.mutex;
}
Storage& queue_02c400(Context& context) {
    auto& guard=context.guards.queue_0a39a0;
    auto& runtime=context.initialization;
    if(runtime.thread_epoch()<guard){
        runtime.header_04b838(guard);
        if(guard==-1){
            // Original constructs a deque/proxy, then COPY inserts the seed.
            // Its low-level allocator/container implementation is a boundary.
            try {
                context.storage.construct_seed_copy(initial_down_event);
                (void)runtime.register_atexit(Cleanup::queue_07e000);
            } catch(...) {runtime.abort_initialization(guard);throw;}
            runtime.footer(guard);
        }
    }
    return context.storage;
}
std::deque<std::string>& OwnedStorage::data() {
    if(!entries_)throw std::logic_error("Event storage not constructed");
    return *entries_;
}
const std::deque<std::string>& OwnedStorage::data() const {
    if(!entries_)throw std::logic_error("Event storage not constructed");
    return *entries_;
}
void OwnedStorage::construct_seed_copy(std::string_view seed) {
    if(entries_)throw std::logic_error("Event storage already constructed");
    std::deque<std::string> prepared;
    prepared.emplace_back(seed);
    entries_.emplace(std::move(prepared));
    published_size_.store(1,std::memory_order_relaxed);
}
bool OwnedStorage::nonempty_relaxed() const {
    return published_size_.load(std::memory_order_relaxed)!=0;
}
bool OwnedStorage::nonempty_locked() const {return !data().empty();}
std::string OwnedStorage::copy_front_locked() const {
    if(data().empty())throw std::logic_error("Event storage front requires an entry");
    return data().front();
}
void OwnedStorage::push_move_034660(std::string& value) {
    data().push_back(std::move(value));
    value.clear(); // Logical empty source, not native SSO/capacity reconstruction.
    published_size_.store(data().size(),std::memory_order_relaxed);
}
void OwnedStorage::push_persisted_locked(std::string& value,const std::function<void()>& persist) {
    data().push_back(std::move(value)); // Allocation completes before disk commit.
    try{persist();}catch(...){data().pop_back();throw;}
    value.clear();
    published_size_.store(data().size(),std::memory_order_relaxed);
}
void OwnedStorage::pop_front_locked() {
    if(delivery_state_.load(std::memory_order_acquire)!=0)
        throw std::logic_error("Cannot pop an event owned by a pending delivery");
    if(data().empty())throw std::logic_error("Event storage pop requires an entry");
    data().pop_front();
    published_size_.store(data().size(),std::memory_order_relaxed);
}
void OwnedStorage::settle_delivery_locked() {
    const auto state=delivery_state_.load(std::memory_order_acquire);
    if((state&3u)==2u) {
        data().pop_front();
        published_size_.store(data().size(),std::memory_order_relaxed);
        delivery_state_.store(0,std::memory_order_release);
    } else if((state&3u)==3u)delivery_state_.store(0,std::memory_order_release);
}
bool OwnedStorage::front_available_locked() {
    settle_delivery_locked();
    return delivery_state_.load(std::memory_order_acquire)==0&&!data().empty();
}
std::uint64_t OwnedStorage::reserve_front_locked() {
    if(!front_available_locked())return 0;
    next_delivery_=(next_delivery_+1)&(~std::uint64_t{0}>>2);
    if(!next_delivery_)++next_delivery_;
    const auto token=next_delivery_<<2;
    delivery_state_.store(token|1u,std::memory_order_release);
    return token;
}
bool OwnedStorage::finish_reserved(std::uint64_t token,bool success) noexcept {
    if(!token)return false;
    auto expected=token|1u;
    return delivery_state_.compare_exchange_strong(expected,token|(success?2u:3u),
        std::memory_order_release,std::memory_order_relaxed);
}
} // namespace laureondo::event_queue
