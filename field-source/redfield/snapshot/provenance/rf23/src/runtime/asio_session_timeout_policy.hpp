// SPDX-License-Identifier: GPL-3.0-only
#pragma once
// Private transport policy. No public ABI, Win32 API, or alternate socket owner.
// Receive-idle is reset only by delivered bytes. A started frame has a fixed
// deadline regardless of subsequent fragments. Send-idle is reset only by
// positive underlying async_write progress (never by a retry/timer tick).
#include <chrono>
#include <cstdint>

namespace redfield { namespace runtime {
class AsioSessionTimeoutPolicy final {
public:
    using Clock = std::chrono::steady_clock;
    using Time = Clock::time_point;
    enum class Expired : uint8_t { none, receive, send };
    static constexpr auto receive_idle = std::chrono::seconds(30);
    static constexpr auto receive_frame = std::chrono::seconds(10);
    static constexpr auto send_idle = std::chrono::seconds(5);

    void begin_receive(Time now, bool buffered) noexcept {
        if (receiving_) return;
        receiving_ = true;
        last_receive_ = now;
        frame_active_ = buffered;
        frame_start_ = now;
    }
    void receive_progress(Time now, bool buffered) noexcept {
        if (!receiving_) return;
        last_receive_ = now;
        if (buffered && !frame_active_) {
            frame_active_ = true;
            frame_start_ = now;
        }
    }
    void end_receive() noexcept { receiving_ = false; frame_active_ = false; }
    void begin_send(Time now) noexcept {
        sending_ = true;
        last_send_ = now;
    }
    void send_progress(Time now) noexcept {
        if (sending_) last_send_ = now;
    }
    void end_send() noexcept { sending_ = false; }
    void clear() noexcept { end_receive(); end_send(); }

    bool next(Time* out) const noexcept {
        if (!out || (!receiving_ && !sending_)) return false;
        bool found = false;
        if (receiving_) {
            *out = last_receive_ + receive_idle;
            if (frame_active_ && frame_start_ + receive_frame < *out)
                *out = frame_start_ + receive_frame;
            found = true;
        }
        if (sending_ && (!found || last_send_ + send_idle < *out))
            *out = last_send_ + send_idle;
        return true;
    }
    Expired expired(Time now) const noexcept {
        if (receiving_ && (now >= last_receive_ + receive_idle ||
            (frame_active_ && now >= frame_start_ + receive_frame)))
            return Expired::receive;
        if (sending_ && now >= last_send_ + send_idle) return Expired::send;
        return Expired::none;
    }
private:
    Time last_receive_{};
    Time frame_start_{};
    Time last_send_{};
    bool receiving_{false};
    bool frame_active_{false};
    bool sending_{false};
};
} }
