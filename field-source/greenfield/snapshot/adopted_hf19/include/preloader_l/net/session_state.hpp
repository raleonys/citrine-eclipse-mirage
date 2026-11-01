#pragma once

#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/socket_transport.hpp"

namespace preloader_l { namespace net {

enum class SessionPhase : uint8_t {
    idle = 0,
    connected,
    closing,
    closed,
    faulted,
};

enum class SessionStatus : uint8_t {
    ok = 0,
    invalid_argument,
    already_connected,
    not_connected,
    invalid_transport,
    counter_overflow,
};

struct SessionState {
    SessionPhase phase;
    uint32_t session_id;
    TransportEndpoint endpoint;
    uint64_t received_bytes;
    uint64_t sent_bytes;
    uint32_t receive_events;
    uint32_t send_events;
    uint32_t close_count;
};

void reset_session(SessionState* session) noexcept;
SessionStatus bind_session(SessionState* session, uint32_t session_id,
                           const TransportEndpoint& endpoint) noexcept;
SessionStatus note_session_receive(SessionState* session, size_t bytes) noexcept;
SessionStatus note_session_send(SessionState* session, size_t bytes) noexcept;
SessionStatus close_session(SessionState* session) noexcept;
void fault_session(SessionState* session) noexcept;

} }
