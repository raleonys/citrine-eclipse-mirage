// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/pending_session_cleanup.hpp"
#include <exception>
#include <memory>

namespace preloader_l {
// One ownership handoff from the worker to explicit shutdown. The worker is
// the sole producer; WorkerOwner joins it before calling retry. No destructor
// attempts OS cleanup or silently destroys a service with live completions.
template<class HookOwner>
class SessionRetirement final {
public:
    constexpr explicit SessionRetirement(HookOwner& hooks) noexcept : hooks_(hooks) {}
    SessionRetirement(const SessionRetirement&)=delete;
    SessionRetirement& operator=(const SessionRetirement&)=delete;
    bool pending() const noexcept { return pending_.load(std::memory_order_acquire)!=nullptr; }
    void retain(std::unique_ptr<LocalService>& session) noexcept {
        if (!session) return;
        LocalService* empty=nullptr;
        if (!pending_.compare_exchange_strong(empty,session.get(),std::memory_order_release,
                                              std::memory_order_relaxed)) std::terminate();
        (void)session.release();
    }
    bool finish(std::unique_ptr<LocalService>& session) noexcept {
        try {
            if (hooks_.stop(0) && (!session || session->stop_for(std::chrono::milliseconds(0)))) {
                session.reset();
                return !pending();
            }
        } catch (...) {}
        retain(session);
        return false;
    }
    bool retry(std::uint32_t timeout_ms) noexcept {
        return cleanup_pending_session(hooks_,pending_,timeout_ms);
    }
private:
    HookOwner& hooks_;
    std::atomic<LocalService*> pending_{nullptr};
};
}
