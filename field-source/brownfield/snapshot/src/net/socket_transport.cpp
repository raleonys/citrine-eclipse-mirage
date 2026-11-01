#include "preloader_l/net/socket_transport.hpp"

namespace preloader_l { namespace net {

bool valid_endpoint(const TransportEndpoint& endpoint) noexcept
{
    return endpoint.receive != nullptr && endpoint.send != nullptr && endpoint.close != nullptr;
}

bool valid_listener(const TransportListener& listener) noexcept
{
    return listener.port != 0 && listener.accept != nullptr && listener.close != nullptr;
}

void close_endpoint(TransportEndpoint* endpoint) noexcept
{
    if (!endpoint) return;
    if (endpoint->close) endpoint->close(endpoint->context);
    *endpoint = TransportEndpoint{};
}

void close_listener(TransportListener* listener) noexcept
{
    if (!listener) return;
    if (listener->close) listener->close(listener->context);
    *listener = TransportListener{};
}

} }
