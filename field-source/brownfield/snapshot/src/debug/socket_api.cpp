#include "preloader_l/debug/debug_api.hpp"
#include "preloader_l/net/socket_runtime.hpp"
#include "preloader_l/net/socket_session.hpp"
#include "preloader_l/net/winsock_native.hpp"
#include "preloader_l/runtime/runtime_state.hpp"

namespace {
preloader_l::net::SocketRuntime g_socket_runtime{};
preloader_l::net::SocketListenerState g_listener{};
preloader_l::net::SocketEndpointState g_client{};
preloader_l::net::SocketSessionServer g_server{};

bool runtime_ready() noexcept
{
    return preloader_l::runtime::snapshot_state().phase == preloader_l::runtime::Phase::initialized;
}
void reset_all() noexcept
{
    preloader_l::net::reset_socket_runtime(&g_socket_runtime);
    preloader_l::net::reset_socket_listener(&g_listener);
    preloader_l::net::reset_socket_endpoint(&g_client);
    preloader_l::net::reset_socket_session_server(&g_server);
}
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_start(uint16_t requested_port,
                                                          uint16_t* bound_port) noexcept
{
    if (bound_port) *bound_port = 0;
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    if (!bound_port) return static_cast<uint32_t>(preloader_l::net::SocketStatus::invalid_argument);
    if (g_socket_runtime.started || g_listener.listening)
        return static_cast<uint32_t>(preloader_l::net::SocketStatus::already_started);
    reset_all();
    auto status = preloader_l::net::start_socket_runtime(&g_socket_runtime, preloader_l::net::native_winsock_api());
    if (status != preloader_l::net::SocketStatus::ok) return static_cast<uint32_t>(status);
    status = preloader_l::net::create_loopback_listener(&g_socket_runtime, requested_port, 1, &g_listener);
    if (status != preloader_l::net::SocketStatus::ok) {
        (void)preloader_l::net::stop_socket_runtime(&g_socket_runtime);
        return static_cast<uint32_t>(status);
    }
    *bound_port = g_listener.port;
    return 0u;
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_accept(uint32_t session_id) noexcept
{
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    if (!g_listener.listening) return static_cast<uint32_t>(preloader_l::net::SocketStatus::not_started);
    if (g_client.open || g_server.session.phase == preloader_l::net::SessionPhase::connected)
        return static_cast<uint32_t>(preloader_l::net::SessionStatus::already_connected) | 0x80030000u;
    preloader_l::net::TransportEndpoint endpoint{};
    const auto status = preloader_l::net::accept_socket_endpoint(&g_listener, &g_client, &endpoint);
    if (status != preloader_l::net::SocketStatus::ok) return static_cast<uint32_t>(status);
    const auto session_status = preloader_l::net::bind_socket_session_server(&g_server, session_id, endpoint);
    if (session_status != preloader_l::net::SessionStatus::ok) {
        preloader_l::net::close_socket_endpoint(&g_client);
        return static_cast<uint32_t>(session_status) | 0x80030000u;
    }
    return 0u;
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_pump(char* response,
                                                         size_t response_capacity,
                                                         size_t* written) noexcept
{
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    return static_cast<uint32_t>(
        preloader_l::net::pump_socket_session_once(&g_server, response, response_capacity, written));
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_socket_status(preloader_l_debug_socket_status* out,
                                                              size_t out_size) noexcept
{
    if (!out || out_size < sizeof(preloader_l_debug_socket_status)) return 1u;
    out->runtime_started = g_socket_runtime.started ? 1u : 0u;
    out->listener_active = g_listener.listening ? 1u : 0u;
    out->client_active = g_client.open ? 1u : 0u;
    out->session_phase = static_cast<uint32_t>(g_server.session.phase);
    out->session_id = g_server.session.session_id;
    out->bound_port = g_listener.port;
    out->reserved16 = 0;
    out->last_socket_error = g_socket_runtime.last_error;
    out->dispatch_count = g_server.dispatch_count;
    out->responses_sent = g_server.responses_sent;
    return 0u;
}

extern "C" uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_stop() noexcept
{
    if (!runtime_ready()) return PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED;
    if (g_server.session.phase == preloader_l::net::SessionPhase::connected ||
        g_server.session.phase == preloader_l::net::SessionPhase::faulted ||
        g_server.session.phase == preloader_l::net::SessionPhase::closing) {
        (void)preloader_l::net::close_socket_session_server(&g_server);
    } else if (g_client.open) {
        preloader_l::net::close_socket_endpoint(&g_client);
    }
    preloader_l::net::close_socket_listener(&g_listener);
    if (g_socket_runtime.started) (void)preloader_l::net::stop_socket_runtime(&g_socket_runtime);
    reset_all();
    return 0u;
}
