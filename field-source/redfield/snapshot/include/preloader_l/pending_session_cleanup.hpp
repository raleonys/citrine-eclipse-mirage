// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/local_service.hpp"
#include "preloader_l/shutdown_budget.hpp"
#include <atomic>
#include <thread>

namespace preloader_l {
// Called only by the serialized WorkerOwner after its worker has signaled.
// A failed hook restore or transport drain retains the session and module pin.
template<class HookOwner>
bool cleanup_pending_session(HookOwner& hooks, std::atomic<LocalService*>& pending,
                             std::uint32_t timeout_ms) noexcept {
    ShutdownBudget budget(timeout_ms);
    if (!hooks.stop(budget.remaining())) return false;
    auto* session=pending.load(std::memory_order_acquire);
    if (!session) return true;
    try {
        // A drive can retire a peer and post listener bookkeeping for the next
        // drive. Keep driving within this same caller deadline; a single poll
        // is not proof of quiescence, even when given a positive duration.
        for (;;) {
            if (session->stop_for(std::chrono::milliseconds(budget.remaining()))) break;
            if (!budget.remaining()) return false;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        pending.store(nullptr,std::memory_order_release);
        delete session;
        return true;
    } catch (...) { return false; }
}
}
