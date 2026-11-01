#if !defined(_WIN32)
#error release_server_lifecycle.cpp is Windows-only.
#endif
#include "release_server_io_internal.hpp"

namespace preloader_l { namespace runtime {
namespace detail = release_server_windows_detail;

namespace release_server_windows_detail {
namespace {
volatile int32_t g_stop_requested = 0;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
volatile int32_t g_acceptance_accepted = 0;
volatile int32_t g_acceptance_started = 0;
volatile int32_t g_acceptance_completed = 0;
volatile int32_t g_acceptance_peer_nonempty = 0;
volatile int32_t g_acceptance_reaped = 0;
volatile int32_t g_acceptance_asio_live = 0;
#endif
} // namespace

bool release_server_stop_requested() noexcept {
    return atomic_load_i32(&g_stop_requested) != 0;
}
void reset_release_server_stop() noexcept {
    (void)atomic_exchange_i32(&g_stop_requested, 0);
}
void signal_release_server_stop() noexcept {
    (void)atomic_exchange_i32(&g_stop_requested, 1);
}
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
void reset_release_acceptance_counters() noexcept {
    (void)atomic_exchange_i32(&g_acceptance_accepted, 0);
    (void)atomic_exchange_i32(&g_acceptance_started, 0);
    (void)atomic_exchange_i32(&g_acceptance_completed, 0);
    (void)atomic_exchange_i32(&g_acceptance_peer_nonempty, 0);
    (void)atomic_exchange_i32(&g_acceptance_reaped, 0);
    (void)atomic_exchange_i32(&g_acceptance_asio_live, 0);
}
void note_release_acceptance_accepted() noexcept { (void)atomic_fetch_add_i32(&g_acceptance_accepted, 1); }
void note_release_acceptance_started() noexcept { (void)atomic_fetch_add_i32(&g_acceptance_started, 1); }
void note_release_acceptance_completed() noexcept { (void)atomic_fetch_add_i32(&g_acceptance_completed, 1); }
void note_release_acceptance_peer_nonempty() noexcept { (void)atomic_fetch_add_i32(&g_acceptance_peer_nonempty, 1); }
void note_release_acceptance_reaped() noexcept { (void)atomic_fetch_add_i32(&g_acceptance_reaped, 1); }
void note_release_acceptance_asio_owner_opened() noexcept {
    (void)atomic_fetch_add_i32(&g_acceptance_asio_live, 1);
}
void note_release_acceptance_asio_owner_retired() noexcept {
    // Source of decrements is an erased, previously registered Asio session.
    (void)atomic_fetch_add_i32(&g_acceptance_asio_live, -1);
}
uint32_t release_acceptance_asio_live_owners() noexcept {
    return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_asio_live));
}
uint32_t release_acceptance_counter(uint32_t selector) noexcept {
    switch (selector) {
    case 1u: return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_accepted));
    case 2u: return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_started));
    case 3u: return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_completed));
    case 4u: return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_peer_nonempty));
    case 7u: return static_cast<uint32_t>(atomic_load_i32(&g_acceptance_reaped));
    default: return 0u;
    }
}
#endif
} // namespace release_server_windows_detail

bool initialize_release_worker_policy_windows(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return false;
    service->listener_socket = detail::INVALID_SOCKET;
    service->active = 0u;
    service->accept_thread = model_abi::ModelJthread24{};
    service->client_records = model_abi::MsvcVector24{};
    service->mutex_state = model_abi::ModelRecursiveMutex50{};
    service->mutex_state.type_state = 2u;
    service->mutex_state.owner_thread_id = 0xffffffffu;
    service->mutex_state.recursion_count = 0u;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    detail::reset_release_acceptance_counters();
#endif
    return true;
}

void reset_release_server_stop_windows() noexcept {
    detail::reset_release_server_stop();
}

void request_release_server_stop_windows() noexcept {
    detail::signal_release_server_stop();
}

// Historical native-worker paths are retained only for the explicitly enabled
// native-acceptance instrumented build. Shipping and Debug own sessions via
// Asio IOCP and MUST NOT silently fall back to the legacy worker pool.
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
bool prepare_release_listener_windows(ReleaseServerContext* context) noexcept {
    if (!context || !context->model_service || !context->listener.listening ||
        context->listener.socket == preloader_l::net::invalid_socket_handle) return false;
    if (!configure_release_socket_nonblocking_windows(
            static_cast<uintptr_t>(context->listener.socket))) return false;
    auto* service = context->model_service;
    service->port = context->listener.port;
    service->listener_socket = static_cast<uintptr_t>(context->listener.socket);
    detail::store_active(service, 1u);
    return true;
}

bool serve_release_workers_windows(ReleaseServerContext* context) noexcept {
    if (!context || !context->model_service || !context->listener.listening ||
        context->listener.socket == preloader_l::net::invalid_socket_handle) return false;
    auto* service = context->model_service;
    if (!detail::start_accept_jthread(service)) return detail::semantic_terminal(context, 0x5d03u);

    detail::HANDLE accept_handle = reinterpret_cast<detail::HANDLE>(service->accept_thread.handle);
    while (!detail::bootstrap_stopping()) {
        if (!accept_handle) return detail::semantic_terminal(context, 0x5d04u);
        const uint32_t wait = detail::WaitForSingleObject(accept_handle, detail::kWaitSliceMs);
        if (wait == detail::kWaitObject0) {
            if (detail::bootstrap_stopping() || !detail::load_active(service)) break;
            detail::server_trace(context, TraceEvent::failure, 0x5d05u);
            return false;
        }
        if (wait != detail::kWaitTimeout) return false;
    }
    detail::request_jthread_stop(&service->accept_thread);
    return true;
}

void quiesce_release_worker_policy_windows(ReleaseServerContext* context) noexcept {
    if (!context || !context->model_service) return;
    auto* service = context->model_service;
    detail::store_active(service, 0u);
    detail::request_jthread_stop(&service->accept_thread);

    if (service->accept_thread.handle) {
        const uint32_t waited = detail::WaitForSingleObject(
            reinterpret_cast<detail::HANDLE>(service->accept_thread.handle), detail::kInfinite);
        if (waited != detail::kWaitObject0) {
            (void)detail::semantic_terminal(context, 0x5d07u);
            return;
        }
    }

    if (context->listener.socket != preloader_l::net::invalid_socket_handle) {
        const auto close_status = preloader_l::net::close_socket_listener(&context->listener);
        if (close_status != preloader_l::net::SocketStatus::ok) {
            (void)detail::semantic_terminal(context, 0x5d08u);
            return;
        }
    }
    service->listener_socket = detail::INVALID_SOCKET;

    if (!detail::lock_service_mutex(service)) {
        (void)detail::semantic_terminal(context, 0x5d06u);
        return;
    }
    // W6R1 provenance note: quiescence is wait-only. Physical HANDLE and
    // stop-state retirement remains owned by the retryable retirement preflight
    // so close/free failures are observable by the enclosing DllShutdown gate.
    const bool clients_quiesced = detail::quiesce_all_client_threads(service);
    detail::unlock_service_mutex(service);
    if (!clients_quiesced) {
        (void)detail::semantic_terminal(context, 0x5d09u);
        return;
    }
}

bool prepare_release_worker_policy_retirement_windows(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    detail::store_active(service, 0u);
    detail::request_jthread_stop(&service->accept_thread);
    bool clients_ok = true;
    if (detail::client_size(service) != 0u) {
        if (!detail::lock_service_mutex(service)) return false;
        clients_ok = detail::stop_all_client_threads(service);
        detail::unlock_service_mutex(service);
    }
    if (!clients_ok) return false;
    if (!detail::retry_pending_owned_sockets()) return false;
    if (!detail::destroy_jthread(&service->accept_thread)) return false;

    auto* begin = detail::client_begin(service);
    const size_t capacity = detail::client_capacity_count(service);
    if (begin) {
        const auto status = release_block(
            begin, capacity * sizeof(model_abi::ModelJthread24),
            windows_allocator_platform_ops());
        if (status != AllocationReleaseStatus::ok) return false;
    }
    service->client_records = model_abi::MsvcVector24{};
    service->mutex_state.owner_thread_id = 0xffffffffu;
    service->mutex_state.recursion_count = 0u;
    service->mutex_state.srw_lock_word = 0u;
    return true;
}

#else
bool prepare_release_listener_windows(ReleaseServerContext*) noexcept {
    return false; // no raw native listener adoption by the release backend
}
bool serve_release_workers_windows(ReleaseServerContext*) noexcept {
    return false; // a failed Asio start must not start a legacy worker
}
namespace {
bool legacy_worker_slots_unowned(const model_abi::ReleaseServiceObject548* s) noexcept {
    if (!s) return true;
    return s->listener_socket == detail::INVALID_SOCKET &&
           s->accept_thread.handle == 0u && s->accept_thread.stop_state == 0u &&
           s->client_records.begin == 0u && s->client_records.end == 0u &&
           s->client_records.capacity == 0u;
}
}
void quiesce_release_worker_policy_windows(ReleaseServerContext* context) noexcept {
    // Empty physical ABI fields are mirrors only; never close an Asio handle.
    // Unexpected nonempty slots stay intact; the pre-retirement gate rejects
    // them rather than discarding possible ownership.
    if (!context || !context->model_service) return;
    if (legacy_worker_slots_unowned(context->model_service))
        detail::store_active(context->model_service, 0u);
}
bool prepare_release_worker_policy_retirement_windows(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!legacy_worker_slots_unowned(service)) return false;
    if (!service) return true;
    detail::store_active(service, 0u);
    service->mutex_state.owner_thread_id = 0xffffffffu;
    service->mutex_state.recursion_count = 0u;
    service->mutex_state.srw_lock_word = 0u;
    return true;
}
#endif

void release_release_worker_policy_windows(
    model_abi::ReleaseServiceObject548* service) noexcept {
    (void)prepare_release_worker_policy_retirement_windows(service);
}

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
uint32_t release_worker_acceptance_metric_windows(
    model_abi::ReleaseServiceObject548* service, uint32_t selector) noexcept {
    switch (selector) {
    case 0u: return service ? static_cast<uint32_t>(service->port) : 0u;
    case 1u: return detail::release_acceptance_counter(1u);
    case 2u: return detail::release_acceptance_counter(2u);
    case 3u: return detail::release_acceptance_counter(3u);
    case 4u: return detail::release_acceptance_counter(4u);
    case 5u: {
        // The WinSock jthread vector is deliberately empty on the Asio path.
        // Report real Asio registry ownership instead of a legacy zero.
        auto* adapter = model_service_adapter(service);
        if (adapter && adapter->asio_bridge)
            return detail::release_acceptance_asio_live_owners();
        if (!service || !detail::lock_service_mutex(service)) return 0xffffffffu;
        const uint32_t value = static_cast<uint32_t>(detail::client_size(service));
        detail::unlock_service_mutex(service);
        return value;
    }
    case 6u: {
        auto* adapter = model_service_adapter(service);
        if (adapter && adapter->asio_bridge)
            return static_cast<uint32_t>(release_client_hard_cap);
        if (!service || !detail::lock_service_mutex(service)) return 0xffffffffu;
        const uint32_t value = static_cast<uint32_t>(detail::client_capacity_count(service));
        detail::unlock_service_mutex(service);
        return value;
    }
    case 7u: return detail::release_acceptance_counter(7u);
    default: return 0u;
    }
}
#endif

} }
