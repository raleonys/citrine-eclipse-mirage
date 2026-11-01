#include "preloader_l/net/session_state.hpp"
#include <stdint.h>

namespace preloader_l { namespace net {
namespace {
bool add_u64(uint64_t current, size_t amount, uint64_t* out) noexcept
{
    if (!out) return false;
    const uint64_t value = static_cast<uint64_t>(amount);
    if (value > UINT64_MAX - current) return false;
    *out = current + value;
    return true;
}
}

void reset_session(SessionState* session) noexcept
{
    if (!session) return;
    *session = SessionState{};
    session->phase = SessionPhase::idle;
}

SessionStatus bind_session(SessionState* session, uint32_t session_id,
                           const TransportEndpoint& endpoint) noexcept
{
    if (!session) return SessionStatus::invalid_argument;
    if (session->phase == SessionPhase::connected || session->phase == SessionPhase::closing)
        return SessionStatus::already_connected;
    if (!valid_endpoint(endpoint)) return SessionStatus::invalid_transport;
    reset_session(session);
    session->phase = SessionPhase::connected;
    session->session_id = session_id;
    session->endpoint = endpoint;
    return SessionStatus::ok;
}

SessionStatus note_session_receive(SessionState* session, size_t bytes) noexcept
{
    if (!session) return SessionStatus::invalid_argument;
    if (session->phase != SessionPhase::connected) return SessionStatus::not_connected;
    uint64_t next = 0;
    if (!add_u64(session->received_bytes, bytes, &next) || session->receive_events == UINT32_MAX) {
        session->phase = SessionPhase::faulted;
        return SessionStatus::counter_overflow;
    }
    session->received_bytes = next;
    ++session->receive_events;
    return SessionStatus::ok;
}

SessionStatus note_session_send(SessionState* session, size_t bytes) noexcept
{
    if (!session) return SessionStatus::invalid_argument;
    if (session->phase != SessionPhase::connected) return SessionStatus::not_connected;
    uint64_t next = 0;
    if (!add_u64(session->sent_bytes, bytes, &next) || session->send_events == UINT32_MAX) {
        session->phase = SessionPhase::faulted;
        return SessionStatus::counter_overflow;
    }
    session->sent_bytes = next;
    ++session->send_events;
    return SessionStatus::ok;
}

SessionStatus close_session(SessionState* session) noexcept
{
    if (!session) return SessionStatus::invalid_argument;
    if (session->phase == SessionPhase::closed || session->phase == SessionPhase::idle) {
        session->phase = SessionPhase::closed;
        return SessionStatus::not_connected;
    }
    session->phase = SessionPhase::closing;
    close_endpoint(&session->endpoint);
    if (session->close_count != UINT32_MAX) ++session->close_count;
    session->phase = SessionPhase::closed;
    return SessionStatus::ok;
}

void fault_session(SessionState* session) noexcept
{
    if (!session) return;
    close_endpoint(&session->endpoint);
    session->phase = SessionPhase::faulted;
}

} }
