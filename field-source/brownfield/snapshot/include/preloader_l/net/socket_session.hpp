#pragma once

#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/frame_ingress.hpp"
#include "preloader_l/net/session_state.hpp"
#include "preloader_l/net/socket_transport.hpp"

namespace preloader_l { namespace net {

enum class SocketPumpStatus : uint8_t {
    ok = 0,
    need_more,
    would_block,
    peer_closed,
    invalid_argument,
    session_error,
    ingress_error,
    dispatch_error,
    send_error,
    // Recoverable output-capacity condition. `written` is the required
    // logical response length and the complete ingress frame remains pending.
    capacity,
};

struct SocketSessionServer {
    SessionState session;
    IngressState ingress;
    TransportEndpoint endpoint;
    uint32_t dispatch_count;
    uint32_t responses_sent;
};

void reset_socket_session_server(SocketSessionServer* server) noexcept;
SessionStatus bind_socket_session_server(SocketSessionServer* server,
                                         uint32_t session_id,
                                         const TransportEndpoint& endpoint) noexcept;
SocketPumpStatus pump_socket_session_once(SocketSessionServer* server,
                                          char* response,
                                          size_t response_capacity,
                                          size_t* written) noexcept;
SessionStatus close_socket_session_server(SocketSessionServer* server) noexcept;

} }
