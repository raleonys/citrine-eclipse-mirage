// HF13 native shipping cutover: Asio owns accept, sessions and each socket.
// Bootstrap thread drives io_context and observes externally published stop.
#if !defined(_WIN32)
#error HF13 production ReleaseServer Asio bridge is Windows-only.
#endif
#include "asio_release_bridge.hpp"
#include "asio_service_owner.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/net/winsock_native.hpp"
#include <asio/io_context.hpp>
#include <chrono>
#include <memory>
#include <new>
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
#include "asio_acceptance_resource_fault.hpp"
#include <atomic>
#endif
#if !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF13 native ReleaseServer requires actual Windows IOCP, not reactor fallback.
#endif

namespace preloader_l { namespace runtime {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
namespace {
std::atomic<uint32_t> g_asio_fail_socket_close{0u};
std::atomic<uint32_t> g_asio_fail_listener_close{0u};
std::atomic<uint32_t> g_asio_socket_retention_events{0u};
std::atomic<uint32_t> g_asio_listener_retention_events{0u};
std::atomic<uint32_t> g_asio_pending_socket_owners{0u};
std::atomic<uint32_t> g_asio_pending_listener_owners{0u};
}
void configure_asio_acceptance_resource_fault(uint32_t selector) noexcept {
    if (selector==1u) g_asio_fail_socket_close.store(1u);
    else if (selector==2u) g_asio_fail_listener_close.store(1u);
    else if (selector==0u) {
        g_asio_fail_socket_close.store(0u);
        g_asio_fail_listener_close.store(0u);
    }
}
bool consume_asio_acceptance_socket_close_failure() noexcept {
    return g_asio_fail_socket_close.load()!=0u;
}
bool consume_asio_acceptance_listener_close_failure() noexcept {
    return g_asio_fail_listener_close.load()!=0u;
}
void note_asio_acceptance_socket_close_retained() noexcept {
    g_asio_socket_retention_events.fetch_add(1u);
    g_asio_pending_socket_owners.fetch_add(1u);
}
void note_asio_acceptance_socket_close_retired() noexcept {
    g_asio_pending_socket_owners.fetch_sub(1u);
}
void note_asio_acceptance_listener_close_retained() noexcept {
    g_asio_listener_retention_events.fetch_add(1u);
    g_asio_pending_listener_owners.fetch_add(1u);
}
void note_asio_acceptance_listener_close_retired() noexcept {
    g_asio_pending_listener_owners.fetch_sub(1u);
}
uint32_t asio_acceptance_resource_fault_metric(uint32_t selector) noexcept {
    if (selector==1u) return g_asio_socket_retention_events.load();
    if (selector==2u) return g_asio_listener_retention_events.load();
    if (selector==3u) return g_asio_pending_socket_owners.load()+g_asio_pending_listener_owners.load();
    return 0u;
}
#endif
namespace {
struct ReleaseAsioBridge final {
    asio::io_context io{};
    std::shared_ptr<AsioServiceOwner> service{};
    bool started{false};
    bool stopping{false};
    bool quiesced{false};
    bool failed{false};
    bool counted_failure{false};
    ReleaseAsioBridge() : service(std::make_shared<AsioServiceOwner>(io)) {}
    static void quiesced_callback(void* ctx) noexcept {
        static_cast<ReleaseAsioBridge*>(ctx)->quiesced = true;
    }
    static void failed_callback(void* ctx, const asio::error_code&) noexcept {
        static_cast<ReleaseAsioBridge*>(ctx)->failed = true;
    }
    static void session_callback(void*,preloader_l::protocol::ExchangeStatus) noexcept {}
    static bool accepted_callback(void*) noexcept {
        return mark_bootstrap_client_served();
    }
    bool pump() noexcept {
        try {
            if (io.stopped()) io.restart();
            io.run_for(std::chrono::milliseconds(20));
            return true;
        } catch (...) { failed=true; return false; }
    }
    bool request_stop() noexcept {
        // Quiescence is final: never queue another callback just before the
        // owning io_context is released by shutdown.
        if (quiesced) return true;
        if (!started) {
            asio::error_code ec;
            if (!service->abort_before_start(ec)) { failed=true; return false; }
            quiesced=true;
            return true;
        }
        stopping=true;
        return service->request_stop();
    }
    bool drain() noexcept {
        // A live callback or an unsuccessful close must remain owned on failure.
        // Stop may be retried by the enclosing shutdown boundary.
        for (unsigned n=0; n<500 && !quiesced; ++n) {
            if (!pump()) return false;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
            // Never wait out a deliberately retained owner and accidentally
            // permit the enclosing shutdown to report clean quiescence.
            if (asio_acceptance_resource_fault_metric(3u)!=0u) return false;
#endif
        }
        return quiesced;
    }
};
ReleaseAsioBridge* owned_bridge(ReleaseServerContext* c) noexcept {
    return c ? static_cast<ReleaseAsioBridge*>(c->asio_bridge) : nullptr;
}
}

bool start_release_asio_windows(ReleaseServerContext* context, uint16_t* bound_port) noexcept {
    if (bound_port) *bound_port=0;
    if (!context||!bound_port||context->asio_bridge||!context->model_service||
        context->first_port==0||context->last_port<context->first_port||
        context->socket_api!=preloader_l::net::native_winsock_api()) return false;
    // The pre-existing WSAStartup reference remains balanced by the enclosing
    // ReleaseServer shutdown; this does not take ownership of Asio handles.
    if (preloader_l::net::start_socket_runtime_lifecycle(&context->runtime,context->socket_api)!=
        preloader_l::net::SocketStatus::ok) return false;
    ReleaseAsioBridge* bridge=nullptr;
    try { bridge=new ReleaseAsioBridge(); }
    catch (...) {
        (void)preloader_l::net::stop_socket_runtime(&context->runtime);
        return false;
    }
    // Publish before async start: any subsequent failure leaves an exact owner
    // reachable through context, never an orphaned io_context or acceptor.
    context->asio_bridge=bridge;
    asio::error_code ec;
    uint16_t port=0;
    for (uint32_t p=context->first_port;p<=context->last_port;++p) {
        if (bridge->service->bind_loopback(static_cast<uint16_t>(p),ec)) {
            port=bridge->service->bound_port(ec);
            if (!ec&&port) break;
        }
        if (ec!=asio::error::address_in_use) break;
    }
    if (!port||ec) return false; // shutdown retries retain this owner
    if (!bridge->service->start({bridge,&ReleaseAsioBridge::quiesced_callback,
                                      &ReleaseAsioBridge::failed_callback,
                                      &ReleaseAsioBridge::session_callback,
                                      &ReleaseAsioBridge::accepted_callback}))
        return false;
    bridge->started=true;
    context->model_service->port=port;
    // The historical 0x548 service must NEVER own the Asio acceptor handle.
    // Legacy worker thread/socket fields stay unpopulated.
    if (context->listener_ready&&!context->listener_ready(port)) return false;
    *bound_port=port;
    return true;
}

bool drive_release_asio_windows(ReleaseServerContext* context) noexcept {
    auto* bridge=owned_bridge(context);
    if (!bridge||!bridge->started||bridge->stopping) return false;
    if (!bridge->pump()||bridge->failed) return false;
    return true; // bootstrap owns stop and error propagation
}

bool stop_release_asio_windows(ReleaseServerContext* context) noexcept {
    auto* bridge=owned_bridge(context);
    if (!bridge) return true;
    if (!bridge->request_stop()) return false;
    return bridge->drain();
}

bool shutdown_release_asio_windows(ReleaseServerContext* context) noexcept {
    auto* bridge=owned_bridge(context);
    if (!bridge) return true;
    if (!bridge->request_stop()||!bridge->drain()) return false;
    // No pending accept or session callback can reference io_context now.
    delete bridge;
    context->asio_bridge=nullptr;
    return true;
}
} }
