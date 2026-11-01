#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace runtime {
namespace {
uint32_t g_stop_requested = 0;

inline void stop_store(uint32_t value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(
        reinterpret_cast<volatile long*>(&g_stop_requested),
        static_cast<long>(value));
#elif defined(__clang__) || defined(__GNUC__)
    __atomic_store_n(&g_stop_requested, value, __ATOMIC_RELEASE);
#else
    g_stop_requested = value;
#endif
}

inline uint32_t stop_load() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(&g_stop_requested), 0, 0));
#elif defined(__clang__) || defined(__GNUC__)
    return __atomic_load_n(&g_stop_requested, __ATOMIC_ACQUIRE);
#else
    return g_stop_requested;
#endif
}
}

void clear_bootstrap_stop_request() noexcept { stop_store(0); }
void request_bootstrap_stop() noexcept { stop_store(1); }
bool bootstrap_stop_requested() noexcept { return stop_load() != 0; }

bool valid_bootstrap_backend(const BootstrapBackend& backend) noexcept
{
    return backend.initialize && backend.start_listener && backend.serve_one &&
           backend.stop_listener && backend.shutdown;
}

namespace {
bool stop_and_shutdown_backend(const BootstrapBackend& backend, bool stop_listener) noexcept
{
    if (stop_listener) backend.stop_listener(backend.context);
    return backend.shutdown(backend.context);
}
}

BootstrapRunResult run_one_shot_bootstrap(const BootstrapBackend& backend) noexcept
{
    if (!valid_bootstrap_backend(backend)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_argument));
        return BootstrapRunResult::invalid_argument;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::worker_started, 0);
    if (!mark_bootstrap_running()) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }
    if (!backend.initialize(backend.context)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::initialize_failed));
        return BootstrapRunResult::initialize_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::runtime_initialized, 0);
    if (!mark_bootstrap_runtime_initialized()) {
        if (!stop_and_shutdown_backend(backend, false)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }

    uint16_t port = 0;
    if (!backend.start_listener(backend.context, &port) || port == 0) {
        if (!stop_and_shutdown_backend(backend, true)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::listener_failed));
        return BootstrapRunResult::listener_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::listener_active, port);
    if (!mark_bootstrap_listener_active(port)) {
        if (!stop_and_shutdown_backend(backend, true)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }

    if (!backend.serve_one(backend.context)) {
        if (!stop_and_shutdown_backend(backend, true)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::serve_failed));
        return BootstrapRunResult::serve_failed;
    }
    if (!backend.backend_counts_clients) {
        PRELOADER_L_TRACE_NOTE(TraceEvent::client_accepted, 1);
        if (!mark_bootstrap_client_served()) {
            if (!stop_and_shutdown_backend(backend, true)) {
                mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
                return BootstrapRunResult::shutdown_failed;
            }
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
            return BootstrapRunResult::invalid_state;
        }
    }

    backend.stop_listener(backend.context);
    if (!backend.shutdown(backend.context)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
        return BootstrapRunResult::shutdown_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::shutdown_complete, 0);
    if (!mark_bootstrap_completed()) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::bootstrap_complete, 0);
    return BootstrapRunResult::ok;
}

BootstrapRunResult run_persistent_bootstrap(const BootstrapBackend& backend) noexcept
{
    if (!valid_bootstrap_backend(backend)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_argument));
        return BootstrapRunResult::invalid_argument;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::worker_started, 0);
    if (!mark_bootstrap_running()) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }
    // F321-S081-001: a stop published before the persistent worker starts is
    // authoritative. Do not acquire runtime/listener authority just to tear it
    // back down.
    if (bootstrap_stop_requested()) {
        if (!mark_bootstrap_completed()) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
            return BootstrapRunResult::invalid_state;
        }
        return BootstrapRunResult::ok;
    }
    if (!backend.initialize(backend.context)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::initialize_failed));
        return BootstrapRunResult::initialize_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::runtime_initialized, 0);
    if (!mark_bootstrap_runtime_initialized()) {
        if (!stop_and_shutdown_backend(backend, false)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }

    // A stop racing initialization suppresses listener publication. Only the
    // runtime authority actually acquired above is shut down.
    if (bootstrap_stop_requested()) {
        if (!backend.shutdown(backend.context)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        if (!mark_bootstrap_completed()) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
            return BootstrapRunResult::invalid_state;
        }
        return BootstrapRunResult::ok;
    }

    uint16_t port = 0;
    if (!backend.start_listener(backend.context, &port) || port == 0) {
        if (!stop_and_shutdown_backend(backend, true)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::listener_failed));
        return BootstrapRunResult::listener_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::listener_active, port);
    if (!mark_bootstrap_listener_active(port)) {
        if (!stop_and_shutdown_backend(backend, true)) {
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
            return BootstrapRunResult::shutdown_failed;
        }
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }

    while (!bootstrap_stop_requested()) {
        if (!backend.serve_one(backend.context)) {
            // Normal explicit/process detach sets bootstrap stop first. An
            // internal backend failure does not, and must surface as serve_failed
            // rather than becoming a hot-spin retry loop.
            if (bootstrap_stop_requested()) break;
            if (!stop_and_shutdown_backend(backend, true)) {
                mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
                return BootstrapRunResult::shutdown_failed;
            }
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::serve_failed));
            return BootstrapRunResult::serve_failed;
        }
        if (!backend.backend_counts_clients && !mark_bootstrap_client_served()) {
            if (!stop_and_shutdown_backend(backend, true)) {
                mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
                return BootstrapRunResult::shutdown_failed;
            }
            mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
            return BootstrapRunResult::invalid_state;
        }
    }

    backend.stop_listener(backend.context);
    if (!backend.shutdown(backend.context)) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::shutdown_failed));
        return BootstrapRunResult::shutdown_failed;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::shutdown_complete, 0);
    if (!mark_bootstrap_completed()) {
        mark_bootstrap_failed(static_cast<uint32_t>(BootstrapRunResult::invalid_state));
        return BootstrapRunResult::invalid_state;
    }
    PRELOADER_L_TRACE_NOTE(TraceEvent::bootstrap_complete, 0);
    return BootstrapRunResult::ok;
}

} }
