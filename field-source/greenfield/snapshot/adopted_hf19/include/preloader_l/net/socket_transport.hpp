#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace net {

enum class TransportStatus : uint8_t {
    ok = 0,
    invalid_argument,
    not_ready,
    would_block,
    closed,
    io_error,
};

struct TransportEndpoint;

using ReceiveFn = TransportStatus (*)(void* context,
                                      uint8_t* output,
                                      size_t capacity,
                                      size_t* received) noexcept;
using SendFn = TransportStatus (*)(void* context,
                                   const uint8_t* input,
                                   size_t length,
                                   size_t* sent) noexcept;
using CloseFn = void (*)(void* context) noexcept;
using MonotonicMillisecondsFn = uint64_t (*)(void* context) noexcept;

struct TransportEndpoint {
    void* context;
    ReceiveFn receive;
    SendFn send;
    CloseFn close;
    // Optional monotonic clock used by bounded send-progress handling. Existing
    // synthetic/debug endpoints may leave this null; shipping Windows workers
    // provide GetTickCount64-backed time.
    MonotonicMillisecondsFn monotonic_milliseconds = nullptr;
    // Maximum interval with no successful send progress. Zero selects the
    // protocol default; 0xffffffff keeps the historical infinite mode only for
    // deliberately non-shipping fixtures.
    uint32_t send_idle_timeout_ms = 0u;
    // Maximum interval with no receive progress while waiting for the next
    // logical frame. Zero selects the protocol default; 0xffffffff is reserved
    // for deliberate non-shipping fixtures.
    uint32_t receive_idle_timeout_ms = 0u;
    // Maximum wall-clock time from the first byte of a logical frame until the
    // frame is complete. This prevents slow trickle input from renewing the
    // idle budget indefinitely. Zero selects the protocol default.
    uint32_t receive_frame_timeout_ms = 0u;
};

using AcceptFn = TransportStatus (*)(void* context, TransportEndpoint* accepted) noexcept;

struct TransportListener {
    void* context;
    uint16_t port;
    AcceptFn accept;
    CloseFn close;
};

bool valid_endpoint(const TransportEndpoint& endpoint) noexcept;
bool valid_listener(const TransportListener& listener) noexcept;
void close_endpoint(TransportEndpoint* endpoint) noexcept;
void close_listener(TransportListener* listener) noexcept;

} } // namespace preloader_l::net
