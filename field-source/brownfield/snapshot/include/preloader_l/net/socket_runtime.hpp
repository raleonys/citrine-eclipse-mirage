#pragma once

#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/socket_transport.hpp"

namespace preloader_l { namespace net {

using SocketHandle = uintptr_t;
constexpr SocketHandle invalid_socket_handle = static_cast<SocketHandle>(~static_cast<SocketHandle>(0));

enum class SocketStatus : uint8_t {
    ok = 0,
    invalid_argument,
    not_started,
    already_started,
    api_error,
    would_block,
    closed,
    capacity,
};

using SocketStartupFn = int (*)(void* context, uint16_t version) noexcept;
using SocketCleanupFn = int (*)(void* context) noexcept;
using SocketOpenTcpFn = SocketHandle (*)(void* context) noexcept;
using SocketBindLoopbackFn = int (*)(void* context, SocketHandle socket,
                                     uint16_t requested_port, uint16_t* bound_port) noexcept;
using SocketListenFn = int (*)(void* context, SocketHandle socket, int backlog) noexcept;
using SocketAcceptFn = SocketHandle (*)(void* context, SocketHandle listener) noexcept;
using SocketReceiveFn = TransportStatus (*)(void* context, SocketHandle socket,
                                            uint8_t* output, size_t capacity,
                                            size_t* received) noexcept;
using SocketSendFn = TransportStatus (*)(void* context, SocketHandle socket,
                                         const uint8_t* input, size_t length,
                                         size_t* sent) noexcept;
using SocketCloseFn = int (*)(void* context, SocketHandle socket) noexcept;
using SocketLastErrorFn = int (*)(void* context) noexcept;

struct SocketApi {
    void* context;
    SocketStartupFn startup;
    SocketCleanupFn cleanup;
    SocketOpenTcpFn open_tcp;
    SocketBindLoopbackFn bind_loopback;
    SocketListenFn listen;
    SocketAcceptFn accept;
    SocketReceiveFn receive;
    SocketSendFn send;
    SocketCloseFn close;
    SocketLastErrorFn last_error;
};

struct SocketRuntime {
    const SocketApi* api;
    bool started;
    uint32_t startup_count;
    uint32_t cleanup_count;
    int last_error;
};

struct SocketListenerState {
    SocketRuntime* runtime;
    SocketHandle socket;
    uint16_t port;
    bool listening;
};

struct SocketEndpointState {
    SocketRuntime* runtime;
    SocketHandle socket;
    bool open;
};

bool valid_socket_api(const SocketApi& api) noexcept;
// Shipping Asio uses only the WSA lifecycle callbacks; legacy socket slots
// need not be populated. ABI/layout of SocketApi and SocketRuntime is unchanged.
bool valid_socket_lifecycle_api(const SocketApi& api) noexcept;
SocketStatus start_socket_runtime_lifecycle(SocketRuntime* runtime, const SocketApi* api) noexcept;
void reset_socket_runtime(SocketRuntime* runtime) noexcept;
SocketStatus start_socket_runtime(SocketRuntime* runtime, const SocketApi* api) noexcept;
SocketStatus stop_socket_runtime(SocketRuntime* runtime) noexcept;

void reset_socket_listener(SocketListenerState* listener) noexcept;
SocketStatus create_loopback_listener(SocketRuntime* runtime, uint16_t requested_port,
                                      int backlog, SocketListenerState* listener) noexcept;
SocketStatus close_socket_listener(SocketListenerState* listener) noexcept;

void reset_socket_endpoint(SocketEndpointState* endpoint) noexcept;
SocketStatus accept_socket_endpoint(SocketListenerState* listener,
                                    SocketEndpointState* endpoint,
                                    TransportEndpoint* transport) noexcept;
SocketStatus close_socket_endpoint(SocketEndpointState* endpoint) noexcept;

} }
