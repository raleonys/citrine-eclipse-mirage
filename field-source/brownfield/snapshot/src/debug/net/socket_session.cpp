#include "preloader_l/net/socket_session.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace net {

void reset_socket_session_server(SocketSessionServer* server) noexcept
{
    if (!server) return;
    reset_session(&server->session);
    reset_ingress(&server->ingress);
    server->endpoint = TransportEndpoint{};
    server->dispatch_count = 0;
    server->responses_sent = 0;
}

SessionStatus bind_socket_session_server(SocketSessionServer* server,
                                         uint32_t session_id,
                                         const TransportEndpoint& endpoint) noexcept
{
    if (!server || !valid_endpoint(endpoint)) return SessionStatus::invalid_argument;
    // Preserve bind_session()'s already_connected ownership guard. Reset only
    // the per-generation ingress/counters after a new session is accepted.
    const SessionStatus status = bind_session(&server->session, session_id, endpoint);
    if (status == SessionStatus::ok) {
        reset_ingress(&server->ingress);
        server->endpoint = endpoint;
        server->dispatch_count = 0;
        server->responses_sent = 0;
    }
    return status;
}

SocketPumpStatus pump_socket_session_once(SocketSessionServer* server,
                                          char* response,
                                          size_t response_capacity,
                                          size_t* written) noexcept
{
    if (written) *written = 0;
    if (!server || !response || !written || response_capacity == 0 ||
        server->session.phase != SessionPhase::connected ||
        !valid_endpoint(server->endpoint)) {
        return SocketPumpStatus::invalid_argument;
    }

    preloader_l::xml::TextSpan frame{};
    IngressStatus ingress_status = peek_ingress_frame(&server->ingress, &frame);
    if (ingress_status == IngressStatus::need_more) {
        uint8_t input[2048];
        size_t received = 0;
        const TransportStatus receive_status = server->endpoint.receive(
            server->endpoint.context, input, sizeof(input), &received);
        if (receive_status == TransportStatus::would_block) return SocketPumpStatus::would_block;
        if (receive_status == TransportStatus::closed) {
            (void)close_session(&server->session);
            return SocketPumpStatus::peer_closed;
        }
        if (receive_status != TransportStatus::ok || received == 0) {
            fault_session(&server->session);
            return SocketPumpStatus::session_error;
        }
        if (note_session_receive(&server->session, received) != SessionStatus::ok) {
            fault_session(&server->session);
            return SocketPumpStatus::session_error;
        }
        ingress_status = feed_ingress(&server->ingress, input, received, &frame);
    }

    if (ingress_status == IngressStatus::need_more) return SocketPumpStatus::need_more;
    if (ingress_status != IngressStatus::frame_ready) {
        fault_session(&server->session);
        return SocketPumpStatus::ingress_error;
    }

    // Keep a complete request pending across recoverable output-capacity
    // failures. The size probe precedes the real render so stateful handlers
    // are not committed merely to discover that the caller buffer is small.
    const auto measured = preloader_l::core::dispatch_plaintext_request(
        frame.data, frame.size, nullptr, 0);
    if (measured.status == preloader_l::core::DispatchStatus::capacity &&
        measured.written >= response_capacity) {
        *written = measured.written;
        return SocketPumpStatus::capacity;
    }

    const auto result = preloader_l::core::dispatch_plaintext_request(
        frame.data, frame.size, response, response_capacity);
    if (result.status == preloader_l::core::DispatchStatus::capacity) {
        *written = result.written;
        return SocketPumpStatus::capacity;
    }
    ++server->dispatch_count;
    if (result.status != preloader_l::core::DispatchStatus::ok || result.written == 0) {
        (void)consume_ingress_frame(&server->ingress, frame.size);
        return SocketPumpStatus::dispatch_error;
    }

    size_t total_sent = 0;
    while (total_sent < result.written) {
        size_t sent = 0;
        const TransportStatus send_status = server->endpoint.send(
            server->endpoint.context,
            reinterpret_cast<const uint8_t*>(response) + total_sent,
            result.written - total_sent, &sent);
        if (send_status != TransportStatus::ok || sent == 0 || sent > result.written - total_sent) {
            fault_session(&server->session);
            return SocketPumpStatus::send_error;
        }
        total_sent += sent;
    }

    if (consume_ingress_frame(&server->ingress, frame.size) != IngressStatus::ok ||
        note_session_send(&server->session, total_sent) != SessionStatus::ok) {
        fault_session(&server->session);
        return SocketPumpStatus::session_error;
    }
    ++server->responses_sent;
    *written = total_sent;
    return SocketPumpStatus::ok;
}

SessionStatus close_socket_session_server(SocketSessionServer* server) noexcept
{
    if (!server) return SessionStatus::invalid_argument;
    const SessionStatus status = close_session(&server->session);
    server->endpoint = TransportEndpoint{};
    reset_ingress(&server->ingress);
    return status;
}

} }
