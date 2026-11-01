// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/handlers/content.hpp"
#include "src/handlers/presence.hpp"
#include "src/protocol/messages.hpp"
#include <atomic>
#include <deque>
#include <mutex>
#include <stdexcept>
namespace preloader_l {
class OwnedMutex final:public content::Mutex {
    std::recursive_mutex mutex_;std::int32_t count_=0;
public:
    int lock_04a6ac() override{mutex_.lock();++count_;return 0;}
    std::int32_t& count_4c() override{return count_;}
    void unlock_04a6b4() override{--count_;mutex_.unlock();}
    [[noreturn]] void thread_error_04a648(int code) override{throw std::runtime_error("mutex contract failure "+std::to_string(code));}
};
class Events final:public messages::EventQueue,public content::EventQueue,public presence::IgoEffects {
    OwnedMutex mutex_;std::deque<std::string> queue_;
    std::atomic<size_t> published_{0};bool initialized_=false;
    void initialize(){if(!initialized_){queue_.emplace_back("<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>");initialized_=true;published_.store(queue_.size(),std::memory_order_release);}}
public:
    content::Mutex& mutex_02c620() override{return mutex_;}
    bool initialized_locked() const override{return initialized_;}
    void initialize_locked(std::string initial) override{if(initialized_)throw std::logic_error("event queue already initialized");queue_.push_back(std::move(initial));initialized_=true;published_.store(queue_.size(),std::memory_order_release);}
    void push_locked_034660(std::string value) override{initialize();queue_.push_back(std::move(value));published_.store(queue_.size(),std::memory_order_release);}
    std::unique_ptr<content::PendingEvent> prepare_event(std::string initial,std::string payload) override {
        struct Prepared final:content::PendingEvent {
            Events& owner;std::deque<std::string> next;
            Prepared(Events& o,std::string first,std::string value):owner(o),next(o.queue_){if(!o.initialized_)next.push_back(std::move(first));next.push_back(std::move(value));}
            void commit() noexcept override{owner.queue_.swap(next);owner.initialized_=true;owner.published_.store(owner.queue_.size(),std::memory_order_release);}
        };
        return std::make_unique<Prepared>(*this,std::move(initial),std::move(payload));
    }
    bool nonempty_relaxed() const override{return published_.load(std::memory_order_relaxed)!=0;}
    int lock_04a6ac() override{return mutex_.lock_04a6ac();}
    std::int32_t& lock_count_4c() override{return mutex_.count_4c();}
    bool nonempty_locked() const override{return !queue_.empty();}
    std::string copy_front_locked() const override{if(queue_.empty())throw std::logic_error("empty event queue");return queue_.front();}
    void pop_front_locked() override{if(queue_.empty())throw std::logic_error("empty event queue");queue_.pop_front();published_.store(queue_.size(),std::memory_order_release);}
    void unlock_04a6b4() override{mutex_.unlock_04a6b4();}
    int lock_02c620_04a6ac() override{return mutex_.lock_04a6ac();}
    void acquire_event_queue_02c400() override{initialize();}
    void enqueue_034660(std::string&& value) override{push_locked_034660(std::move(value));}
    [[noreturn]] void thread_error_04a648(int n) override{mutex_.thread_error_04a648(n);}
};
}
