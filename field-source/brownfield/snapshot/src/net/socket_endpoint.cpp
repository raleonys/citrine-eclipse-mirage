#include "preloader_l/net/socket_runtime.hpp"

namespace preloader_l { namespace net {
namespace {
TransportStatus endpoint_receive(void* context, uint8_t* output, size_t capacity,
                                 size_t* received) noexcept
{
    auto* state = static_cast<SocketEndpointState*>(context);
    if (received) *received = 0;
    if (!state || !state->open || !state->runtime || !state->runtime->started ||
        !state->runtime->api || !state->runtime->api->receive) {
        return TransportStatus::not_ready;
    }
    return state->runtime->api->receive(state->runtime->api->context, state->socket,
                                        output, capacity, received);
}
TransportStatus endpoint_send(void* context, const uint8_t* input, size_t length,
                              size_t* sent) noexcept
{
    auto* state = static_cast<SocketEndpointState*>(context);
    if (sent) *sent = 0;
    if (!state || !state->open || !state->runtime || !state->runtime->started ||
        !state->runtime->api || !state->runtime->api->send) {
        return TransportStatus::not_ready;
    }
    return state->runtime->api->send(state->runtime->api->context, state->socket,
                                     input, length, sent);
}
void endpoint_close(void* context) noexcept
{
    (void)close_socket_endpoint(static_cast<SocketEndpointState*>(context));
}
}

void reset_socket_endpoint(SocketEndpointState* endpoint) noexcept
{
    if (!endpoint) return;
    endpoint->runtime = nullptr;
    endpoint->socket = invalid_socket_handle;
    endpoint->open = false;
}

SocketStatus accept_socket_endpoint(SocketListenerState* listener,
                                    SocketEndpointState* endpoint,
                                    TransportEndpoint* transport) noexcept
{
    if (!listener || !endpoint || !transport) return SocketStatus::invalid_argument;
    *transport = TransportEndpoint{};
    if (!listener->listening || !listener->runtime || !listener->runtime->started ||
        !listener->runtime->api) return SocketStatus::not_started;
    // Audit3 S0445: a failed close intentionally preserves endpoint ownership
    // for retry. Never erase that owner merely because the persistent server
    // reached another accept iteration.
    if (endpoint->open) return SocketStatus::already_started;
    reset_socket_endpoint(endpoint);
    const SocketApi* api = listener->runtime->api;
    const SocketHandle accepted = api->accept(api->context, listener->socket);
    if (accepted == invalid_socket_handle) {
        listener->runtime->last_error = api->last_error(api->context);
        return SocketStatus::api_error;
    }
    endpoint->runtime = listener->runtime;
    endpoint->socket = accepted;
    endpoint->open = true;
    *transport = TransportEndpoint{endpoint, endpoint_receive, endpoint_send, endpoint_close};
    listener->runtime->last_error = 0;
    return SocketStatus::ok;
}

SocketStatus close_socket_endpoint(SocketEndpointState* endpoint) noexcept
{
    if (!endpoint) return SocketStatus::invalid_argument;
    if (!endpoint->runtime || !endpoint->runtime->api ||
        endpoint->socket == invalid_socket_handle) {
        reset_socket_endpoint(endpoint);
        return SocketStatus::ok;
    }
    const SocketApi* api = endpoint->runtime->api;
    if (!api->close || api->close(api->context, endpoint->socket) != 0) {
        endpoint->runtime->last_error = api->last_error ? api->last_error(api->context) : 0;
        return SocketStatus::api_error;
    }
    reset_socket_endpoint(endpoint);
    return SocketStatus::ok;
}


} }
