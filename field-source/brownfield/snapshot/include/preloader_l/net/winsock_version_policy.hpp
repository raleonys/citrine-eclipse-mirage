#pragma once
#include <stdint.h>

namespace preloader_l { namespace net {

constexpr uint16_t winsock_requested_version = 0x0202u;
constexpr int winsock_version_not_supported = 10092;
// Internal adapter sentinel: WSAStartup succeeded, negotiated version was
// unsupported, and the immediate balancing WSACleanup failed. This is not a
// Winsock error code; it preserves cleanup ownership for SocketRuntime retry.
constexpr int winsock_version_cleanup_retry_required = -10092;

constexpr bool winsock_negotiated_version_supported(uint16_t negotiated) noexcept
{
    return negotiated == winsock_requested_version;
}

} }
