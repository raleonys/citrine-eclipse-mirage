#include "queue.hpp"
#include <stdexcept>

namespace anadius::event_queue_recovered {
Mutex& mutex_02c620(Context& context) {
    auto& guard=context.guards.mutex_0a3970;
    auto& runtime=context.initialization;
    if(runtime.thread_epoch()<guard){
        runtime.header_04b838(guard);
        if(guard==-1){
            runtime.construct_mutex_04a698(context.mutex,2);
            (void)runtime.register_atexit(Cleanup::mutex_07e030);
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
            context.storage.construct_seed_copy(initial_down_event);
            (void)runtime.register_atexit(Cleanup::queue_07e000);
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
void OwnedStorage::pop_front_locked() {
    if(data().empty())throw std::logic_error("Event storage pop requires an entry");
    data().pop_front();
    published_size_.store(data().size(),std::memory_order_relaxed);
}
} // namespace anadius::event_queue_recovered
