#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/runtime/local_social_host.hpp"

namespace preloader_l { namespace core {

enum class DispatchStatus : uint8_t {
    ok = 0,
    invalid_argument,
    incomplete_request,
    malformed_request,
    unknown_route,
    unimplemented_route,
    unsupported_route,
    no_response_required,
    handler_error,
    // Size query / insufficient output capacity. `written` contains the
    // logical response length excluding the convenience terminal NUL.
    capacity,
};

struct DispatchEnvironment {
    // Active 0x35990 passes the accepted peer-address MSVC string into the
    // request dispatcher. Route-specific consumers are recovered separately;
    // keeping it explicit here prevents the session boundary from dropping it.
    const char* peer_address;
    size_t peer_address_size;
    // Shipping socket transactions set this to reserve FIFO delivery outside
    // the dispatcher and commit/rollback only after network send completes.
    // Direct/debug dispatch keeps the historical immediate drain behavior.
    bool defer_event_delivery = false;
    // Only the handshake-bound socket session can supply this; direct/debug
    // dispatch has no principal and must not claim local social mutations.
    const preloader_l::runtime::LocalSocialSession* local_social_session = nullptr;
};

struct DispatchResult {
    DispatchStatus status;
    preloader_l::router::RouteId route;
    preloader_l::handlers::HandlerStatus handler_status;
    size_t written;
};

DispatchResult dispatch_plaintext_request(
    const char* input,
    size_t length,
    char* output,
    size_t capacity) noexcept;

DispatchResult dispatch_plaintext_request_with_environment(
    const char* input,
    size_t length,
    char* output,
    size_t capacity,
    const DispatchEnvironment* environment) noexcept;

} }
