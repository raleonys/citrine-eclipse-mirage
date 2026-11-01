#include "preloader_l/net/socket_runtime.hpp"

namespace preloader_l { namespace net {
void reset_socket_listener(SocketListenerState* listener) noexcept
{
    if (!listener) return;
    listener->runtime = nullptr;
    listener->socket = invalid_socket_handle;
    listener->port = 0;
    listener->listening = false;
}

SocketStatus close_socket_listener(SocketListenerState* listener) noexcept
{
    if (!listener) return SocketStatus::invalid_argument;
    if (!listener->runtime || !listener->runtime->api ||
        listener->socket == invalid_socket_handle) {
        reset_socket_listener(listener);
        return SocketStatus::ok;
    }
    const SocketApi* api = listener->runtime->api;
    if (!api->close || api->close(api->context, listener->socket) != 0) {
        listener->runtime->last_error = api->last_error ? api->last_error(api->context) : 0;
        // F321-S093-001: retain the socket owner for a later retry. Never
        // publish an empty listener while closesocket may still own a live FD.
        return SocketStatus::api_error;
    }
    reset_socket_listener(listener);
    return SocketStatus::ok;
}

SocketStatus create_loopback_listener(SocketRuntime* runtime, uint16_t requested_port,
                                      int backlog, SocketListenerState* listener) noexcept
{
    if (!runtime || !listener || backlog <= 0) return SocketStatus::invalid_argument;
    if (!runtime->started || !runtime->api) return SocketStatus::not_started;
    // A prior failed close is retry ownership, not reusable empty state.
    if (listener->socket != invalid_socket_handle || listener->runtime)
        return SocketStatus::api_error;
    reset_socket_listener(listener);
    const SocketApi* api = runtime->api;
    const SocketHandle socket = api->open_tcp(api->context);
    if (socket == invalid_socket_handle) {
        runtime->last_error = api->last_error(api->context);
        return SocketStatus::api_error;
    }
    // Publish ownership immediately so bind/listen rollback cannot lose a
    // socket if the close itself fails.
    listener->runtime = runtime;
    listener->socket = socket;
    listener->port = 0;
    listener->listening = false;

    uint16_t bound_port = 0;
    if (api->bind_loopback(api->context, socket, requested_port, &bound_port) != 0 ||
        bound_port == 0) {
        runtime->last_error = api->last_error(api->context);
        (void)close_socket_listener(listener);
        return SocketStatus::api_error;
    }
    if (api->listen(api->context, socket, backlog) != 0) {
        runtime->last_error = api->last_error(api->context);
        (void)close_socket_listener(listener);
        return SocketStatus::api_error;
    }
    listener->port = bound_port;
    listener->listening = true;
    runtime->last_error = 0;
    return SocketStatus::ok;
}

} }
