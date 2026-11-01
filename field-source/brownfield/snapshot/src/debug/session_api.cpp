#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/debug/debug_diagnostics.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/net/frame_ingress.hpp"
#include "preloader_l/net/session_state.hpp"
#include "preloader_l/runtime/runtime_state.hpp"

namespace {
preloader_l::net::IngressState g_ingress{};
preloader_l::net::SessionState g_session{};

preloader_l::net::TransportStatus debug_receive(void*, uint8_t*, size_t, size_t* received) noexcept
{
    if (received) *received = 0;
    return preloader_l::net::TransportStatus::would_block;
}
preloader_l::net::TransportStatus debug_send(void*, const uint8_t*, size_t length, size_t* sent) noexcept
{
    if (sent) *sent = length;
    return preloader_l::net::TransportStatus::ok;
}
void debug_close(void*) noexcept {}

bool runtime_ready() noexcept
{
    return preloader_l::runtime::snapshot_state().phase == preloader_l::runtime::Phase::initialized;
}
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_reset(uint32_t session_id) noexcept
{
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    if (g_session.phase == preloader_l::net::SessionPhase::connected ||
        g_session.phase == preloader_l::net::SessionPhase::closing) {
        (void)preloader_l::net::close_session(&g_session);
    }
    preloader_l::net::reset_session(&g_session);
    preloader_l::net::reset_ingress(&g_ingress);
    const preloader_l::net::TransportEndpoint endpoint{nullptr, debug_receive, debug_send, debug_close};
    return static_cast<uint32_t>(preloader_l::net::bind_session(&g_session, session_id, endpoint));
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_feed(const char* input, size_t length,
                                                          char* response, size_t response_capacity,
                                                          size_t* written) noexcept
{
    if (written) *written = 0;
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    if (g_session.phase != preloader_l::net::SessionPhase::connected) return PRELOADER_L_DEBUG_SESSION_NOT_ACTIVE;
    if (!response || !written || (!input && length != 0)) return PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR;

    preloader_l::xml::TextSpan frame{};
    preloader_l::net::IngressStatus ingress_status;
    if (length == 0) {
        ingress_status = preloader_l::net::peek_ingress_frame(&g_ingress, &frame);
    } else {
        if (preloader_l::net::note_session_receive(&g_session, length) != preloader_l::net::SessionStatus::ok)
            return PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR;
        ingress_status = preloader_l::net::feed_ingress(
            &g_ingress, reinterpret_cast<const uint8_t*>(input), length, &frame);
    }

    if (ingress_status == preloader_l::net::IngressStatus::need_more) return PRELOADER_L_DEBUG_SESSION_NEED_MORE;
    if (ingress_status != preloader_l::net::IngressStatus::frame_ready) {
        preloader_l::net::fault_session(&g_session);
        return PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR;
    }

    // Capacity is recoverable in the debug API. Measure before the real
    // render so a too-small caller buffer cannot commit a stateful handler
    // and cannot consume the complete pending frame. A zero-input retry then
    // peeks the exact same frame and may render it into a larger buffer.
    const auto measured = preloader_l::core::dispatch_plaintext_request(
        frame.data, frame.size, nullptr, 0);
    if (measured.status == preloader_l::core::DispatchStatus::capacity &&
        measured.written >= response_capacity) {
        *written = measured.written;
        return static_cast<uint32_t>(preloader_l::core::DispatchStatus::capacity);
    }

    const auto result = preloader_l::core::dispatch_plaintext_request(
        frame.data, frame.size, response, response_capacity);
    const bool ok = result.status == preloader_l::core::DispatchStatus::ok;
    preloader_l::debug::note_dispatch(ok);
    *written = result.written;

    // A late capacity result is still retryable. `written` is required-size
    // metadata here, not delivered bytes, so preserve ingress and counters.
    if (result.status == preloader_l::core::DispatchStatus::capacity)
        return static_cast<uint32_t>(result.status);

    const size_t consumed_length = frame.size;
    if (preloader_l::net::consume_ingress_frame(&g_ingress, consumed_length) != preloader_l::net::IngressStatus::ok) {
        preloader_l::net::fault_session(&g_session);
        *written = 0;
        return PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR;
    }
    if (result.status == preloader_l::core::DispatchStatus::ok && result.written != 0 &&
        preloader_l::net::note_session_send(&g_session, result.written) != preloader_l::net::SessionStatus::ok) {
        *written = 0;
        return PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR;
    }
    return static_cast<uint32_t>(result.status);
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_session_status(preloader_l_debug_session_status* out, size_t out_size) noexcept
{
    if (!out || out_size < sizeof(preloader_l_debug_session_status)) return 1u;
    out->phase = static_cast<uint32_t>(g_session.phase);
    out->session_id = g_session.session_id;
    out->received_bytes = g_session.received_bytes;
    out->sent_bytes = g_session.sent_bytes;
    out->receive_events = g_session.receive_events;
    out->send_events = g_session.send_events;
    out->buffered_bytes = static_cast<uint32_t>(g_ingress.receive.size);
    out->feed_events = g_ingress.feed_events;
    out->frames_ready = g_ingress.frames_ready;
    out->frames_consumed = g_ingress.frames_consumed;
    out->ingress_errors = g_ingress.errors;
    out->reserved = 0;
    return 0u;
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_close() noexcept
{
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    return static_cast<uint32_t>(preloader_l::net::close_session(&g_session));
}
