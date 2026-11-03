// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/event_adapters.hpp"
#include <stdexcept>

namespace preloader_l::event_queue {
Mutex& ContentQueue::mutex_02c620(){return event_queue::mutex_02c620(context_);}
bool ContentQueue::initialized_locked() const {
    (void)queue_02c400(context_);
    return true; // Getter has completed or its provider/initializer threw.
}
void ContentQueue::initialize_locked(std::string initial) {
    if(initial!=initial_down_event)throw std::invalid_argument("Native event seed must be DOWN");
    (void)queue_02c400(context_);
}
void ContentQueue::push_locked_034660(std::string value){context_.storage.push_move_034660(value);}
void ContentQueue::push_persisted_locked(std::string value,const std::function<void()>& persist) {
    context_.storage.push_persisted_locked(value,persist);
}
int IgoQueue::lock_02c620_04a6ac(){return mutex_02c620(context_).lock_04a6ac();}
std::int32_t& IgoQueue::lock_count_4c(){return context_.mutex.count_4c();}
void IgoQueue::acquire_event_queue_02c400(){(void)queue_02c400(context_);}
void IgoQueue::enqueue_034660(std::string&& value){context_.storage.push_move_034660(value);}
void IgoQueue::unlock_04a6b4(){context_.mutex.unlock_04a6b4();}
[[noreturn]]void IgoQueue::thread_error_04a648(int code){context_.mutex.thread_error_04a648(code);}
bool MessageQueue::nonempty_relaxed() const {return queue_02c400(context_).nonempty_relaxed();}
int MessageQueue::lock_04a6ac(){return mutex_02c620(context_).lock_04a6ac();}
std::int32_t& MessageQueue::lock_count_4c(){return context_.mutex.count_4c();}
bool MessageQueue::nonempty_locked() const{return context_.storage.front_available_locked();}
std::string MessageQueue::copy_front_locked() const{return context_.storage.copy_front_locked();}
void MessageQueue::pop_front_locked(){context_.storage.pop_front_locked();}
void MessageQueue::unlock_04a6b4(){context_.mutex.unlock_04a6b4();}
std::uint64_t MessageQueue::reserve_front_locked(){return context_.storage.reserve_front_locked();}
bool MessageQueue::finish_reserved(std::uint64_t token,bool success) noexcept {
    return context_.storage.finish_reserved(token,success);
}
} // namespace preloader_l::event_queue
