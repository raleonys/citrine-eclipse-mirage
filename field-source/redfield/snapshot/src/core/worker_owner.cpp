// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/worker_owner.hpp"
#include "preloader_l/shutdown_budget.hpp"
namespace preloader_l {
namespace {
struct ReleaseFlag {
    std::atomic_flag& flag;
    ~ReleaseFlag() { flag.clear(std::memory_order_release); }
};
}
bool WorkerOwner::schedule(const WorkerOperations& operations, void* module) noexcept {
    if (operation_.test_and_set(std::memory_order_acquire)) return false;
    ReleaseFlag release{operation_};
    if (phase()!=WorkerPhase::idle || !module || !operations.pin || !operations.spawn ||
        !operations.wait || !operations.close || !operations.unpin) return false;
    operations_=&operations;
    phase_.store(WorkerPhase::scheduled,std::memory_order_release);
    if (!operations.pin(operations.user,module,&pin_) || !pin_) {
        phase_.store(WorkerPhase::failed,std::memory_order_release);
        return false;
    }
    thread_=operations.spawn(operations.user,this);
    if (!thread_) {
        // Loader-lock path: retain the counted pin; never FreeLibrary here.
        phase_.store(WorkerPhase::failed,std::memory_order_release);
        return false;
    }
    return true;
}
ShutdownResult WorkerOwner::shutdown(std::uint32_t timeout_ms) noexcept {
    if (operation_.test_and_set(std::memory_order_acquire)) return ShutdownResult::busy;
    ReleaseFlag release{operation_};
    if (shutdown_complete_) return ShutdownResult::stopped;
    ShutdownBudget budget(timeout_ms);
    stop_.store(true,std::memory_order_release); // never reset after scheduling
    if (thread_) {
        const auto waited=operations_->wait(operations_->user,thread_,budget.remaining());
        if (waited==WaitResult::timeout) return ShutdownResult::timeout;
        if (waited!=WaitResult::signaled) return ShutdownResult::wait_failed;
        if (!clean_.load(std::memory_order_acquire)) {
            if (!operations_->cleanup || !operations_->cleanup(operations_->user,budget.remaining()))
                return ShutdownResult::service_cleanup_pending;
            clean_.store(true,std::memory_order_release);
        }
    }
    if (operations_ && operations_->finalize &&
        !operations_->finalize(operations_->user,budget.remaining()))
        return ShutdownResult::service_cleanup_pending;
    if (thread_) {
        if (!operations_->close(operations_->user,thread_)) return ShutdownResult::handle_close_pending;
        thread_=nullptr;
    }
    if (pin_) {
        if (!operations_->unpin(operations_->user,pin_)) return ShutdownResult::pin_release_pending;
        pin_=nullptr;
    }
    shutdown_complete_=true;
    phase_.store(WorkerPhase::stopped,std::memory_order_release);
    return ShutdownResult::stopped;
}
}
