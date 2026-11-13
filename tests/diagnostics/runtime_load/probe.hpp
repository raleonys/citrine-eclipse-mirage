// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <exception>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <utility>
namespace citrine_eclipse_mirage::benchmark {
using Clock=std::chrono::steady_clock;
inline thread_local std::uint64_t queue_wait_ns=0;
inline std::atomic<std::uint64_t> created_message_threads{0};
// Identical instrumentation in every diagnostic copy, including the two-worker
// baseline. It adds a clock read and a function wrapper; production is untouched.
inline std::function<void()> timed_job(std::function<void()> fn) {
    return [queued=Clock::now(),fn=std::move(fn)]() mutable {
        queue_wait_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-queued).count();
        fn();
    };
}
// Diagnostic scheduling control. Each admitted message creates a fresh thread,
// but source Session FIFO and exact in-flight lifetime/drain remain intact.
// It is not an implementation or timing oracle for the original DLL.
class PerMessageJobs final {
    std::mutex mutex_;
    std::condition_variable changed_;
    unsigned active_=0;
    bool stopping_=false;
    void retired() {
        std::lock_guard lock(mutex_);
        --active_;
        changed_.notify_all(); // Last owner access occurs before unlocking.
    }
public:
    ~PerMessageJobs(){stop();}
    template<class Error>void submit(std::function<void()> fn,Error on_error) {
        {std::lock_guard lock(mutex_);if(stopping_)throw std::runtime_error("control is stopping");++active_;}
        std::thread child;
        try {
            child=std::thread([this,fn=std::move(fn),on_error]() mutable {
                try{fn();}catch(...){on_error(std::current_exception());}
                fn={}; // Release Client/function ownership before final drain.
                retired();
            });
        }catch(...){retired();throw;}
        ++created_message_threads;
        try{child.detach();}catch(...){child.join();throw;}
    }
    void stop(){std::unique_lock lock(mutex_);stopping_=true;changed_.wait(lock,[this]{return active_==0;});}
};
}
