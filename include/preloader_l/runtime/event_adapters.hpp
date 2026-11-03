// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/protocol/messages.hpp"
#include "preloader_l/handlers/misc_handlers.hpp"

namespace preloader_l::event_queue {
// New composition adapters over one Context, not separate original functions.
// They add no locking; caller bodies determine lock ordering.
class ContentQueue final:public content_handlers::EventQueue {
public:
    explicit ContentQueue(Context& context):context_(context){}
    Mutex& mutex_02c620() override;
    // Executes native lazy queue getter under the caller's existing event lock.
    bool initialized_locked() const override;
    void initialize_locked(std::string initial) override;
    void push_locked_034660(std::string value) override;
    void push_persisted_locked(std::string,const std::function<void()>&) override;
private:Context& context_;
};
class IgoQueue final:public misc_handlers::IgoEffects {
public:
    explicit IgoQueue(Context& context):context_(context){}
    int lock_02c620_04a6ac() override;
    std::int32_t& lock_count_4c() override;
    void acquire_event_queue_02c400() override;
    void enqueue_034660(std::string&&) override;
    void unlock_04a6b4() override;
    [[noreturn]] void thread_error_04a648(int) override;
private:Context& context_;
};
class MessageQueue final:public messages::EventQueue {
public:
    explicit MessageQueue(Context& context):context_(context){}
    bool nonempty_relaxed() const override;
    int lock_04a6ac() override;
    std::int32_t& lock_count_4c() override;
    bool nonempty_locked() const override;
    std::string copy_front_locked() const override;
    void pop_front_locked() override;
    void unlock_04a6b4() override;
    std::uint64_t reserve_front_locked() override;
    bool finish_reserved(std::uint64_t,bool) noexcept override;
private:Context& context_;
};
} // namespace preloader_l::event_queue
