// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <chrono>
#include <cstdint>
#include <functional>
#include <utility>
namespace citrine_eclipse_mirage::benchmark {
using Clock=std::chrono::steady_clock;
inline thread_local std::uint64_t queue_wait_ns=0;
// Identical instrumentation in every diagnostic copy, including the two-worker
// baseline. It adds a clock read and a function wrapper; production is untouched.
inline std::function<void()> timed_job(std::function<void()> fn) {
    return [queued=Clock::now(),fn=std::move(fn)]() mutable {
        queue_wait_ns=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-queued).count();
        fn();
    };
}
}
