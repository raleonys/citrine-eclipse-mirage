// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config_file.hpp"
#include "preloader_l/worker_owner.hpp"
#include "preloader_l/windows_hooks.hpp"
#include <atomic>
#include <cstdint>

namespace preloader_l::testing {
// Test composition owns every counter and fault budget. This observer never
// closes/unpins resources itself: a rejected attempt leaves WorkerOwner's duty
// intact for retry. It is absent from the production include/link closure.
class WorkerObservation final {
public:
    constexpr WorkerObservation() noexcept = default;
    void configuration_result(FileConfigResult result) noexcept {
        config_.store(static_cast<unsigned>(result),std::memory_order_release);
    }
    void service_ready(bool online,std::uint16_t port) noexcept {
        online_.store(online,std::memory_order_release);
        port_.store(port,std::memory_order_release);
    }
    void transport_progress(std::uint64_t accepted,std::uint64_t active,std::uint64_t frames) noexcept {
        accepted_.store(accepted,std::memory_order_release);
        active_.store(active,std::memory_order_release);
        frames_.store(frames,std::memory_order_release);
    }
    void service_stopped() noexcept { port_.store(0,std::memory_order_release); }
    void set_faults(unsigned close_failures,unsigned unpin_failures) noexcept {
        fail_close_.store(close_failures);fail_unpin_.store(unpin_failures);
    }
    bool allow_close() noexcept { return !consume(fail_close_); }
    bool allow_unpin() noexcept { return !consume(fail_unpin_); }
    // The caller acquires WorkerOwner's phase before calling this method, just
    // as the original native ABI read the publication marker before its fields.
    std::uint64_t status(WorkerPhase phase,HookPhase hooks,std::uint64_t mapped) const noexcept {
        return static_cast<std::uint64_t>(phase) |
            (static_cast<std::uint64_t>(config_.load(std::memory_order_acquire))<<8) |
            (static_cast<std::uint64_t>(online_.load(std::memory_order_acquire))<<16) |
            (static_cast<std::uint64_t>(port_.load(std::memory_order_acquire))<<17) |
            (static_cast<std::uint64_t>(hooks)<<33) | ((mapped&0xffffu)<<40);
    }
    std::uint64_t transport() const noexcept {
        const auto frames=frames_.load(std::memory_order_acquire);
        return ((accepted_.load(std::memory_order_acquire)&0xffffu)<<48) |
               ((active_.load(std::memory_order_acquire)&0xffffu)<<32) | (frames&0xffffffffu);
    }
private:
    static bool consume(std::atomic<unsigned>& remaining) noexcept {
        auto count=remaining.load();
        while(count && !remaining.compare_exchange_weak(count,count-1)) {}
        return count!=0;
    }
    std::atomic<unsigned> config_{255},fail_close_{0},fail_unpin_{0};
    std::atomic<bool> online_{false};
    std::atomic<std::uint16_t> port_{0};
    std::atomic<std::uint64_t> frames_{0},accepted_{0},active_{0};
};
}
