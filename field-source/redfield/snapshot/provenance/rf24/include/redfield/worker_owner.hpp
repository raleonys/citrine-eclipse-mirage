// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <atomic>
#include <cstdint>

namespace redfield {
enum class WorkerPhase : std::uint32_t { idle, scheduled, ready, failed, cancelled, finished, stopped };
enum class WaitResult { signaled, timeout, failed };
// Preserve the existing public shutdown result values for corresponding duties.
enum class ShutdownResult : std::uint32_t {
    stopped=0, busy=1, timeout=2, wait_failed=3,
    handle_close_pending=7, pin_release_pending=8, service_cleanup_pending=10
};
class WorkerOwner;
struct WorkerOperations {
    void* user;
    bool (*pin)(void*, void* module, void** pinned) noexcept;
    void* (*spawn)(void*, WorkerOwner*) noexcept;
    WaitResult (*wait)(void*, void* thread, std::uint32_t milliseconds) noexcept;
    bool (*close)(void*, void* thread) noexcept;
    bool (*unpin)(void*, void* module) noexcept;
    bool (*cleanup)(void*, std::uint32_t remaining_ms) noexcept = nullptr;
    // Runs even when the worker reports its session clean; process resources
    // may still retain physical release duties. Must be retryable/idempotent.
    bool (*finalize)(void*, std::uint32_t remaining_ms) noexcept = nullptr;
};
// Static, allocation-free scheduling state. schedule runs once during attach.
// Operations/context must outlive this owner. Shutdown requires an independent
// host-held DLL reference until it returns; never call it from DllMain.
class WorkerOwner final {
public:
    constexpr WorkerOwner() noexcept = default;
    bool schedule(const WorkerOperations& operations, void* module) noexcept;
    ShutdownResult shutdown(std::uint32_t timeout_ms) noexcept;
    bool stop_requested() const noexcept { return stop_.load(std::memory_order_acquire); }
    WorkerPhase phase() const noexcept { return phase_.load(std::memory_order_acquire); }
    void publish_phase(WorkerPhase phase) noexcept { phase_.store(phase,std::memory_order_release); }
    void mark_worker_clean() noexcept { clean_.store(true,std::memory_order_release); }
private:
    std::atomic_flag operation_ = ATOMIC_FLAG_INIT;
    std::atomic<bool> stop_{false};
    std::atomic<bool> clean_{false};
    std::atomic<WorkerPhase> phase_{WorkerPhase::idle};
    const WorkerOperations* operations_ = nullptr;
    void* thread_ = nullptr;
    void* pin_ = nullptr;
    bool shutdown_complete_ = false;
};
static_assert(std::atomic<bool>::is_always_lock_free);
static_assert(std::atomic<WorkerPhase>::is_always_lock_free);
}
