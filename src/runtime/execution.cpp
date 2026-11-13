// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/execution.hpp"
#include <bit>
#include <limits>
#include <atomic>

namespace citrine_eclipse_mirage::execution {
namespace {
std::int64_t wrap_add(std::int64_t a, std::int64_t b) {
    return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}
std::int64_t wrap_multiply(std::int64_t a, std::int64_t b) {
    return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
}
std::int64_t wrap_subtract(std::int64_t a, std::int64_t b) {
    return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b));
}
}

void initialize_mutex_04a698(MutexState& mutex, std::uint32_t observed_type) {
    mutex.owner_48 = 0xffffffff;
    mutex.storage_08 = 0;
    mutex.srw_10 = 0;
    mutex.count_4c = 0;
    mutex.type = observed_type;
}

int lock_04a6ac(MutexState& mutex, Platform& platform) {
    // 04a6ac is XOR EDX,EDX; JMP04a6d4. Timed/try branches of04a6d4
    // are intentionally outside this untimed caller's contract.
    const auto thread = platform.current_thread_id();
    // Owner is inspected BEFORE the native lock is acquired. Raw C++ accesses
    // race with another thread's publication/release even though the original
    // x64 loads/stores are naturally aligned. Preserve the specified layout.
    std::atomic_ref<std::uint32_t> owner(mutex.owner_48);
    if ((mutex.type & ~std::uint32_t{0x100}) == 1) {
        if (owner.load(std::memory_order_relaxed) != thread) {
            platform.acquire_srw_exclusive(mutex.srw_10);
            owner.store(thread,std::memory_order_relaxed);
        }
        ++mutex.count_4c;
        return 0;
    }
    if (owner.load(std::memory_order_relaxed) != thread) platform.acquire_srw_exclusive(mutex.srw_10);
    const auto previous = mutex.count_4c;
    const auto updated = previous + 1;
    mutex.count_4c = updated;
    if (std::bit_cast<std::int32_t>(updated) <= 1) {
        owner.store(thread,std::memory_order_relaxed);
    } else if ((mutex.type & 0x100) == 0) {
        mutex.count_4c = previous;
        return 3;
    }
    return 0;
}

int unlock_04a6b4(MutexState& mutex, Platform& platform) {
    --mutex.count_4c;
    if (mutex.count_4c == 0) {
        std::atomic_ref<std::uint32_t>(mutex.owner_48).store(0xffffffff,std::memory_order_relaxed);
        platform.release_srw_exclusive(mutex.srw_10);
    }
    return 0;
}

std::int64_t counter_to_nanoseconds(std::int64_t counter, std::int64_t frequency) {
    if (frequency == 10'000'000) return wrap_multiply(counter, 100);
    // The24MHz magic-multiply assembly is signed division by24'000'000,
    // followed by quotient/remainder conversion. Keep the arithmetic explicit.
    const auto divisor = frequency;
    const auto quotient = counter / divisor;
    const auto remainder = counter % divisor;
    return wrap_add(wrap_multiply(remainder, 1'000'000'000) / divisor,
                    wrap_multiply(quotient, 1'000'000'000));
}

std::int64_t twenty_ms_deadline(std::int64_t now) {
    // CMP RAX,0x7ffffffffeced2ff; JL normal, otherwise INT64_MAX.
    if (now >= 0x7ffffffffeced2ffLL) return std::numeric_limits<std::int64_t>::max();
    return wrap_add(now, 20'000'000);
}

std::uint32_t observed_sleep_milliseconds(std::int64_t remaining) {
    if (remaining >= 86'400'000'000'000LL) return 86'400'000;
    auto milliseconds = remaining / 1'000'000;
    if (milliseconds * 1'000'000 < remaining) ++milliseconds;
    return static_cast<std::uint32_t>(milliseconds);
}

void Execution::acquire_checked() {
    if (lock_04a6ac(mutex_, platform_) != 0) platform_.raise_thread_error_04a648(5);
    if (mutex_.count_4c == 0x7fffffff) {
        mutex_.count_4c = 0x7ffffffe;
        platform_.raise_thread_error_04a648(6);
    }
}

void Execution::finish_flag_036820(std::uint8_t& supplied_flag) {
    acquire_checked();
    supplied_flag = 1;
    unlock_04a6b4(mutex_, platform_);
}

bool Execution::steps_finished_initializing_under_lock(std::int32_t count) const {
    const auto limit = count == -1 ? 3 : count;
    std::int32_t index = 1;
    bool finished = true;
    if (limit >= 1) {
        // Assembly loads third, first, second once under the mutex before scan.
        const auto third = flags_.third_0a3c3e;
        const auto first = flags_.first_0a3c3d;
        const auto second = flags_.second_0a3c3c;
        do {
            if (!finished) break;
            if (index == 1) finished = first != 0;
            else if (index == 2) finished = second != 0;
            else if (index == 3) finished = third != 0;
            // Preserve INC EAX wrapping; count INT_MAX can wrap and continue.
            index = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(index) + 1);
        } while (index <= limit);
    }
    return finished;
}

std::int64_t Execution::now_nanoseconds_036740() {
    // 04a824 returns RAX even though Ghidra inferred a void declaration.
    // More than one waiter may observe the caller-owned cold cache. Publish
    // the frequency atomically without changing its storage or mutex layout.
    std::atomic_ref<std::int64_t> cache(frequency_cache_);
    auto frequency = cache.load(std::memory_order_relaxed);
    if (frequency == 0) {
        frequency = platform_.query_performance_frequency();
        std::int64_t empty = 0;
        if (!cache.compare_exchange_strong(empty,frequency,std::memory_order_relaxed))
            frequency = empty;
    }
    const auto counter = platform_.query_performance_counter();
    return counter_to_nanoseconds(counter, frequency);
}

void Execution::sleep_until_observed_deadline(std::int64_t deadline) {
    for (;;) {
        const auto now = now_nanoseconds_036740();
        if (now >= deadline) return;
        const auto remaining = wrap_subtract(deadline, now);
        platform_.sleep_milliseconds(observed_sleep_milliseconds(remaining));
    }
}

void Execution::wait_steps_036880(std::int32_t count) {
    for (;;) {
        acquire_checked();
        const auto finished = steps_finished_initializing_under_lock(count);
        unlock_04a6b4(mutex_, platform_);
        if (finished) return;
        const auto deadline = twenty_ms_deadline(now_nanoseconds_036740());
        sleep_until_observed_deadline(deadline);
    }
}
}
