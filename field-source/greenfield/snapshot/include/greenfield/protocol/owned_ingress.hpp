#pragma once
#include "preloader_l/net/frame_ingress.hpp"
#include <cstddef>
#include <cstdint>

namespace greenfield::protocol {
// GF02 isolated, single-threaded and bounded frame-buffer owner. No WinSock,
// listener, worker or exported DLL lifecycle ownership is claimed.
// Never returns a borrowed TextSpan from its internal buffer to the caller.
class OwnedIngress final {
public:
    OwnedIngress() noexcept;
    OwnedIngress(const OwnedIngress&) = delete;
    OwnedIngress& operator=(const OwnedIngress&) = delete;
    preloader_l::net::IngressStatus feed(const std::uint8_t* data, std::size_t length) noexcept;
    // Copies a full frame into caller-owned storage; on insufficient capacity
    // the complete pending frame is retained for an exact retry.
    preloader_l::net::IngressStatus pop(char* data, std::size_t capacity, std::size_t* written) noexcept;
    void shutdown() noexcept;
    bool active() const noexcept { return active_; }
    std::size_t pending_bytes() const noexcept { return state_.receive.size; }
private:
    preloader_l::net::IngressState state_{};
    bool active_{true};
};
}
