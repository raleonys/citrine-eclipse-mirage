// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <span>
#include <vector>

namespace preloader_l {
enum class State { idle, starting, running, draining, cleanup_pending, stopped };
enum class StartResult { started, already_running, invalid_state, busy, start_failed, cleanup_pending };
enum class StopResult { stopped, busy, timeout, cleanup_pending };
struct Resource {
    void* context;
    bool (*start)(void*) noexcept;
    bool (*stop)(void*) noexcept;
};
struct RuntimeSnapshot { State state; std::size_t owned; std::size_t active; };

// Dependencies precede dependents. A failed start still owns its cleanup duty.
// Resources and Runtime must outlive every callback and Lease. Callbacks must
// be bounded, must not throw, and must not re-enter this Runtime.
class Runtime final {
public:
    class Lease final {
    public:
        Lease() noexcept = default;
        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;
        Lease(Lease&& other) noexcept;
        Lease& operator=(Lease&& other) noexcept;
        ~Lease();
        explicit operator bool() const noexcept { return owner_ != nullptr; }
    private:
        friend class Runtime;
        explicit Lease(Runtime* owner) noexcept : owner_(owner) {}
        Runtime* owner_ = nullptr;
        void release() noexcept;
    };
    explicit Runtime(std::span<const Resource> resources);
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    StartResult start();
    // Budget covers active-call draining. Synchronous stop callbacks have their
    // own bounded-execution contract; this is not a total wall-clock deadline.
    StopResult stop(std::chrono::milliseconds drain_budget);
    Lease acquire();
    RuntimeSnapshot snapshot() const;
private:
    bool cleanup();
    void leave() noexcept;
    std::vector<Resource> resources_;
    std::size_t owned_ = 0;
    std::size_t active_ = 0;
    State state_ = State::idle;
    mutable std::mutex state_mutex_;
    std::mutex operation_mutex_;
    std::condition_variable drained_;
};
}
