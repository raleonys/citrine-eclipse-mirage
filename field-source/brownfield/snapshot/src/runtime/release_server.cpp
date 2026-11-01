#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#if defined(_WIN32)
#include "preloader_l/runtime/release_server_io.hpp"
#include "asio_release_bridge.hpp"
#include "preloader_l/net/winsock_native.hpp"
#endif

namespace preloader_l { namespace runtime {
namespace {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
constexpr int kCurioListenBacklog = 0x7fffffff; // Test-only mock listener backlog
#endif

#if PRELOADER_L_TRACE_DIAGNOSTICS
void emit(ReleaseServerContext* context, TraceEvent event, uint32_t value) noexcept
{
    PRELOADER_L_TRACE_NOTE(event, value);
    if (context && context->trace) context->trace(event, value);
}
#define PRELOADER_L_SERVER_TRACE(context,event,value) emit((context),(event),(value))
#else
#define PRELOADER_L_SERVER_TRACE(context,event,value) ((void)0)
#endif

bool initialize_backend(void*) noexcept
{
    const InitResult result = try_initialize();
    return result == InitResult::ok || result == InitResult::already_initialized;
}

bool start_listener_backend(void* opaque, uint16_t* bound_port) noexcept
{
    if (bound_port) *bound_port = 0;
    auto* context = static_cast<ReleaseServerContext*>(opaque);
    if (!context || !context->socket_api || !bound_port || context->first_port == 0 ||
        context->last_port < context->first_port) return false;

#if defined(_WIN32)
#if !defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // Production/Debug fail closed for any injected legacy SocketApi.
    // The only shipping listener/session owner is the Asio IOCP bridge.
    if (context->socket_api != preloader_l::net::native_winsock_api()) return false;
    return start_release_asio_windows(context, bound_port);
#else
    // The test-only fault-injection backend retains deterministic SocketApi.
    if (context->socket_api == preloader_l::net::native_winsock_api())
        return start_release_asio_windows(context, bound_port);
#endif
#endif
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (preloader_l::net::start_socket_runtime(&context->runtime, context->socket_api) !=
        preloader_l::net::SocketStatus::ok) return false;

    for (uint32_t candidate = context->first_port; candidate <= context->last_port; ++candidate) {
        const auto status = preloader_l::net::create_loopback_listener(
            &context->runtime, static_cast<uint16_t>(candidate), kCurioListenBacklog, &context->listener);
        if (status == preloader_l::net::SocketStatus::ok) {
            *bound_port = context->listener.port;
            if (context->listener_ready && !context->listener_ready(*bound_port)) {
                if (preloader_l::net::close_socket_listener(&context->listener) !=
                    preloader_l::net::SocketStatus::ok) return false;
                (void)preloader_l::net::stop_socket_runtime(&context->runtime);
                *bound_port = 0;
                return false;
            }
            PRELOADER_L_SERVER_TRACE(context, TraceEvent::listener_active, *bound_port);
            return true;
        }
    }

    if (context->listener.socket != preloader_l::net::invalid_socket_handle &&
        preloader_l::net::close_socket_listener(&context->listener) !=
            preloader_l::net::SocketStatus::ok) return false;
    (void)preloader_l::net::stop_socket_runtime(&context->runtime);
#endif
    return false;
}

bool serve_one_backend(void* opaque) noexcept
{
    auto* context = static_cast<ReleaseServerContext*>(opaque);
    if (!context) return false;
#if defined(_WIN32)
    if (context->asio_bridge) return drive_release_asio_windows(context);
#endif
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (!context->listener.listening) return false;

    // Injectable/mock backends service exactly one accepted connection per
    // bootstrap iteration. run_persistent_bootstrap() owns the outer loop;
    // keeping a second infinite accept loop here obscures stop/failure state.
    preloader_l::net::TransportEndpoint endpoint{};
    const auto accept_status = preloader_l::net::accept_socket_endpoint(
        &context->listener, &context->client, &endpoint);
    if (accept_status != preloader_l::net::SocketStatus::ok) {
        if (bootstrap_stop_requested()) return true;
        PRELOADER_L_SERVER_TRACE(context, TraceEvent::failure, 0x5901u);
        return false;
    }

    if (!mark_bootstrap_client_served()) {
        if (context->client.open)
            preloader_l::net::close_socket_endpoint(&context->client);
        PRELOADER_L_SERVER_TRACE(context, TraceEvent::failure, 0x5902u);
        return false;
    }

#if PRELOADER_L_TRACE_DIAGNOSTICS
    const uint32_t session = context->session_id++;
    PRELOADER_L_SERVER_TRACE(context, TraceEvent::client_accepted, session);
#endif
    context->last_written = 0;
    uint32_t frames = 0;
    preloader_l::protocol::reset_exchange(&context->protocol);
    const auto exchange_status = preloader_l::protocol::serve_protocol_session(
        endpoint, &context->protocol, &context->last_written, &frames);

    if (frames) PRELOADER_L_SERVER_TRACE(context, TraceEvent::frame_dispatched, frames);
    if (context->last_written)
        PRELOADER_L_SERVER_TRACE(context, TraceEvent::response_sent,
             static_cast<uint32_t>(context->last_written));

    if (context->client.open)
        preloader_l::net::close_socket_endpoint(&context->client);
    preloader_l::protocol::reset_exchange(&context->protocol);

    if (exchange_status == preloader_l::protocol::ExchangeStatus::peer_closed) {
#if PRELOADER_L_TRACE_DIAGNOSTICS
        PRELOADER_L_SERVER_TRACE(context, TraceEvent::peer_closed, session);
        PRELOADER_L_SERVER_TRACE(context, TraceEvent::persistent_session_closed, session);
#endif
        return true;
    }

    // Client-local protocol errors close only this session. The outer bootstrap
    // loop keeps the listener alive and accepts the next client.
    PRELOADER_L_SERVER_TRACE(context, TraceEvent::failure,
         0x5A00u + static_cast<uint32_t>(exchange_status));
#if PRELOADER_L_TRACE_DIAGNOSTICS
    PRELOADER_L_SERVER_TRACE(context, TraceEvent::persistent_session_closed, session);
#endif
    return true;
#else
    // Native Asio bridge not present: never fall back to raw accept/worker.
    return false;
#endif
}

void stop_listener_backend(void* opaque) noexcept
{
    auto* context = static_cast<ReleaseServerContext*>(opaque);
    if (!context) return;
#if defined(_WIN32)
    if (context->asio_bridge) {
        (void)stop_release_asio_windows(context);
        return; // never pass an Asio-owned socket to WinSock cleanup
    }
#endif
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (context->client.socket != preloader_l::net::invalid_socket_handle &&
        preloader_l::net::close_socket_endpoint(&context->client) !=
            preloader_l::net::SocketStatus::ok) return;
    if (context->listener.socket != preloader_l::net::invalid_socket_handle &&
        preloader_l::net::close_socket_listener(&context->listener) !=
            preloader_l::net::SocketStatus::ok) return;
    if (context->runtime.started) (void)preloader_l::net::stop_socket_runtime(&context->runtime);
#endif
}

bool shutdown_backend(void* opaque) noexcept
{
    auto* context = static_cast<ReleaseServerContext*>(opaque);
#if defined(_WIN32)
    if (context && context->asio_bridge && !shutdown_release_asio_windows(context))
        return false; // retain precise owner for a retry, never unload live IOCP
#endif
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (context) {
        if (context->client.socket != preloader_l::net::invalid_socket_handle &&
            preloader_l::net::close_socket_endpoint(&context->client) !=
                preloader_l::net::SocketStatus::ok) return false;
        if (context->listener.socket != preloader_l::net::invalid_socket_handle &&
            preloader_l::net::close_socket_listener(&context->listener) !=
                preloader_l::net::SocketStatus::ok) return false;
    }
#endif
    if (context && context->runtime.started) {
        // stop_listener_backend() performs the first WSACleanup attempt. A
        // failed cleanup deliberately leaves SocketRuntime started, so retry
        // here at the enclosing shutdown boundary and fail closed if Winsock
        // still refuses cleanup. The context must not be discarded while a
        // successful WSAStartup registration remains unmatched.
        const auto status = preloader_l::net::stop_socket_runtime(&context->runtime);
        if (status != preloader_l::net::SocketStatus::ok &&
            status != preloader_l::net::SocketStatus::not_started)
            return false;
    }
    const ShutdownResult result = try_shutdown();
    return result == ShutdownResult::ok || result == ShutdownResult::already_shutdown;
}

}

void reset_release_server(ReleaseServerContext* context,
                          const preloader_l::net::SocketApi* api) noexcept
{
    if (!context) return;
    *context = ReleaseServerContext{};
    context->socket_api = api;
    context->compatibility_port = 0;
    context->first_port = 3226;
    context->last_port = 3236;
#if PRELOADER_L_TRACE_DIAGNOSTICS
    context->session_id = 1;
    context->trace = nullptr;
#endif
    context->listener_ready = nullptr;
#if defined(_WIN32)
    preloader_l::runtime::reset_release_server_stop_windows();
#endif
    preloader_l::net::reset_socket_runtime(&context->runtime);
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    preloader_l::net::reset_socket_listener(&context->listener);
    preloader_l::net::reset_socket_endpoint(&context->client);
#else
    // Keep the adapter's historical layout but initialize empty owner slots
    // without pulling legacy socket-listener/endpoint functions into shipping.
    context->listener = preloader_l::net::SocketListenerState{};
    context->listener.socket = preloader_l::net::invalid_socket_handle;
    context->client = preloader_l::net::SocketEndpointState{};
    context->client.socket = preloader_l::net::invalid_socket_handle;
#endif
    preloader_l::protocol::reset_exchange(&context->protocol);
}

BootstrapBackend make_release_server_backend(ReleaseServerContext* context) noexcept
{
    return BootstrapBackend{
        context,
        initialize_backend,
        start_listener_backend,
        serve_one_backend,
        stop_listener_backend,
        shutdown_backend,
        true,
    };
}

} }
