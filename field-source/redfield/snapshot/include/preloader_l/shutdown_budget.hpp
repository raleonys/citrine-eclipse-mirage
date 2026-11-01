// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <chrono>
#include <cstdint>
#include <limits>

namespace preloader_l {
// One deadline covers worker wait and subsequent cleanup. UINT32_MAX retains
// the Win32 INFINITE sentinel; zero never grants a blocking wait.
class ShutdownBudget final {
public:
    using Clock = std::chrono::steady_clock;
    explicit ShutdownBudget(std::uint32_t milliseconds,
                            Clock::time_point started=Clock::now()) noexcept
        : milliseconds_(milliseconds), started_(started) {}
    std::uint32_t remaining(Clock::time_point now=Clock::now()) const noexcept {
        if (milliseconds_==std::numeric_limits<std::uint32_t>::max()) return milliseconds_;
        const auto left=std::chrono::milliseconds(milliseconds_)-(now-started_);
        if (left<=Clock::duration::zero()) return 0;
        return static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(left).count());
    }
private:
    std::uint32_t milliseconds_;
    Clock::time_point started_;
};
}
