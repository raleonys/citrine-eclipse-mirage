#if !defined(_WIN32)
#error release_server_acceptor.cpp is Windows-only.
#endif
#include "release_server_io_internal.hpp"

namespace preloader_l { namespace runtime {
namespace detail = release_server_windows_detail;
namespace {

struct SockaddrIn16 {
    uint16_t family;
    uint16_t port;
    uint32_t address;
    uint8_t zero[8];
};
static_assert(sizeof(SockaddrIn16) == 0x10, "active IPv4 sockaddr geometry drift");

constexpr size_t kPendingSocketCapacity = release_client_hard_cap + 1u;
detail::SOCKET g_pending_sockets[kPendingSocketCapacity]{};
size_t g_pending_socket_count = 0u;
volatile int32_t g_pending_socket_lock = 0;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
volatile int32_t g_fail_next_socket_close = 0;
volatile int32_t g_fail_next_handle_close = 0;
volatile int32_t g_retained_socket_events = 0;
volatile int32_t g_retained_handle_events = 0;
#endif

void pending_socket_lock() noexcept {
    while (!detail::atomic_compare_exchange_i32(&g_pending_socket_lock, 0, 1)) {}
}
void pending_socket_unlock() noexcept {
    (void)detail::atomic_exchange_i32(&g_pending_socket_lock, 0);
}
int close_socket_owned(detail::SOCKET socket) noexcept {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (detail::atomic_exchange_i32(&g_fail_next_socket_close, 0) != 0) return -1;
#endif
    return detail::closesocket(socket);
}
int close_handle_owned(detail::HANDLE handle) noexcept {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    if (detail::atomic_exchange_i32(&g_fail_next_handle_close, 0) != 0) return 0;
#endif
    return detail::CloseHandle(handle);
}
bool retain_pending_socket(detail::SOCKET socket) noexcept {
    if (socket == detail::INVALID_SOCKET) return true;
    pending_socket_lock();
    for (size_t i = 0; i < g_pending_socket_count; ++i) {
        if (g_pending_sockets[i] == socket) { pending_socket_unlock(); return true; }
    }
    if (g_pending_socket_count >= kPendingSocketCapacity) {
        pending_socket_unlock();
        return false;
    }
    g_pending_sockets[g_pending_socket_count++] = socket;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    (void)detail::atomic_fetch_add_i32(&g_retained_socket_events, 1);
#endif
    pending_socket_unlock();
    return true;
}

size_t reclaim_completed_client_threads_locked(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return 0u;
    auto* begin = detail::client_begin(service);
    size_t size = detail::client_size(service);
    if (!begin || size == 0u) return 0u;

    size_t reclaimed = 0u;
    size_t index = 0u;
    while (index < size) {
        auto* thread = begin + index;
        detail::HANDLE handle = reinterpret_cast<detail::HANDLE>(thread->handle);
        if (!handle || detail::WaitForSingleObject(handle, 0u) != detail::kWaitObject0) {
            ++index;
            continue;
        }
        if (!detail::destroy_jthread(thread)) {
            ++index;
            continue;
        }
        --size;
        for (size_t move = index; move < size; ++move) {
            begin[move] = begin[move + 1u];
            begin[move + 1u] = model_abi::ModelJthread24{};
        }
        service->client_records.end = reinterpret_cast<uintptr_t>(begin + size);
        ++reclaimed;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        detail::note_release_acceptance_reaped();
#endif
    }
    return reclaimed;
}

enum class AppendClientResult : uint8_t { started, capacity_rejected, resource_failure };

AppendClientResult append_client_thread(model_abi::ReleaseServiceObject548* service,
                                        detail::SOCKET client, const char* remote_address,
                                        size_t remote_address_size) noexcept {
    const size_t size = detail::client_size(service);
    if (!release_client_admission_allowed(size)) return AppendClientResult::capacity_rejected;

    const size_t capacity = detail::client_capacity_count(service);
    auto* begin = detail::client_begin(service);
    if (size < capacity) {
        auto* slot = begin + size;
        if (!detail::create_worker_jthread(slot, service, client, remote_address, remote_address_size))
            return AppendClientResult::resource_failure;
        service->client_records.end = reinterpret_cast<uintptr_t>(slot + 1);
        return AppendClientResult::started;
    }

    const size_t required = size + 1u;
    size_t grown = capacity + capacity / 2u;
    if (grown < required) grown = required;
    // Never reserve ownership beyond the project safety ceiling. This keeps
    // vector capacity bounded as well as the number of live worker owners.
    if (grown > release_client_hard_cap) grown = release_client_hard_cap;
    if (grown < required || grown > static_cast<size_t>(-1) / sizeof(model_abi::ModelJthread24))
        return AppendClientResult::resource_failure;
    const size_t bytes = grown * sizeof(model_abi::ModelJthread24);
    const auto allocation = allocate_block(bytes, AllocationRetryPolicy::outer_retry_only,
                                           windows_allocator_platform_ops());
    if (allocation.terminal != AllocationTerminal::none || !allocation.pointer)
        return AppendClientResult::resource_failure;
    auto* fresh = static_cast<model_abi::ModelJthread24*>(allocation.pointer);
    for (size_t i = 0; i < grown; ++i) fresh[i] = model_abi::ModelJthread24{};

    if (!detail::create_worker_jthread(fresh + size, service, client, remote_address, remote_address_size)) {
        release_block(fresh, bytes, windows_allocator_platform_ops());
        return AppendClientResult::resource_failure;
    }
    for (size_t i = 0; i < size; ++i) {
        fresh[i] = begin[i];
        begin[i] = model_abi::ModelJthread24{};
    }
    if (begin) {
        const size_t old_bytes = capacity * sizeof(model_abi::ModelJthread24);
        release_block(begin, old_bytes, windows_allocator_platform_ops());
    }
    service->client_records.begin = reinterpret_cast<uintptr_t>(fresh);
    service->client_records.end = reinterpret_cast<uintptr_t>(fresh + required);
    service->client_records.capacity = reinterpret_cast<uintptr_t>(fresh + grown);
    return AppendClientResult::started;
}

struct AcceptClosure10 {
    uintptr_t stop_state;
    uintptr_t service;
};
static_assert(sizeof(AcceptClosure10) == 0x10, "model accept closure geometry drift");

uint32_t __stdcall accept_main(void* opaque) noexcept {
    auto* closure = static_cast<AcceptClosure10*>(opaque);
    if (!closure) return 0u;
    auto* stop = detail::as_stop(closure->stop_state);
    auto* service = reinterpret_cast<model_abi::ReleaseServiceObject548*>(closure->service);
    auto* context = model_service_adapter(service);

    while (!detail::stop_requested(stop) && detail::load_active(service) != 0u && !detail::bootstrap_stopping()) {
        if (!detail::lock_service_mutex(service)) {
            (void)detail::semantic_terminal(context, 0x5d0au);
            break;
        }
        (void)reclaim_completed_client_threads_locked(service);
        detail::unlock_service_mutex(service);

        const detail::SOCKET listener = static_cast<detail::SOCKET>(service->listener_socket);
        if (listener == detail::INVALID_SOCKET) break;
        const auto ready = wait_release_socket_windows(listener, false);
        if (ready == ReleaseSocketWaitStatus::timeout) continue;
        if (ready == ReleaseSocketWaitStatus::error) {
            if (detail::stop_requested(stop) || !detail::load_active(service) || detail::bootstrap_stopping()) break;
            (void)detail::semantic_terminal(context, 0x5d08u);
            break;
        }

        SockaddrIn16 peer{};
        int peer_size = static_cast<int>(sizeof(peer));
        detail::SOCKET client = detail::accept(listener, &peer, &peer_size);
        if (client == detail::INVALID_SOCKET) {
            if (detail::stop_requested(stop) || !detail::load_active(service) || detail::bootstrap_stopping()) break;
            continue;
        }
        if (!configure_release_socket_nonblocking_windows(client)) {
            if (!detail::close_owned_socket_or_retain(&client))
                detail::signal_release_server_stop();
            (void)detail::semantic_terminal(context, 0x5d09u);
            break;
        }

        char remote_address[0x16]{};
        (void)detail::inet_ntop(detail::kAfInet, &peer.address, remote_address, sizeof(remote_address));
        size_t remote_address_size = 0u;
        while (remote_address[remote_address_size] != 0) ++remote_address_size;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        detail::note_release_acceptance_accepted();
        if (remote_address_size != 0u)
            detail::note_release_acceptance_peer_nonempty();
#endif
        if (!detail::lock_service_mutex(service)) {
            if (!detail::close_owned_socket_or_retain(&client))
                detail::signal_release_server_stop();
            (void)detail::semantic_terminal(context, 0x5d01u);
            break;
        }
        const AppendClientResult appended = append_client_thread(
            service, client, remote_address, remote_address_size);
        detail::unlock_service_mutex(service);

        if (appended != AppendClientResult::started) {
            if (!detail::close_owned_socket_or_retain(&client)) {
                detail::signal_release_server_stop();
            }
            if (appended == AppendClientResult::capacity_rejected) {
                detail::server_trace(context, TraceEvent::client_capacity_rejected,
                                     static_cast<uint32_t>(release_client_hard_cap));
            } else {
                // HOTFIX10/F1598-001: per-client owner allocation/thread
                // pressure is contained to this connection. Do not escalate a
                // remotely-triggerable admission failure to process-terminal.
                detail::server_trace(context, TraceEvent::failure, 0x5d02u);
            }
            continue;
        }
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        detail::note_release_acceptance_started();
#endif
        if (!mark_bootstrap_client_served()) {
            detail::signal_release_server_stop();
            break;
        }
    }

    detail::release_stop_token(stop);
    release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
    return 0u;
}

} // namespace

namespace release_server_windows_detail {
bool close_owned_socket_or_retain(SOCKET* socket) noexcept {
    if (!socket || *socket == INVALID_SOCKET) return true;
    const SOCKET owned = *socket;
    if (close_socket_owned(owned) == 0) {
        *socket = INVALID_SOCKET;
        return true;
    }
    // F06: transfer exact ownership to a bounded registry. Once any close
    // fails, stop admission so the registry can never outgrow live-worker cap
    // plus the single accept-thread rejection owner.
    if (!retain_pending_socket(owned)) return false;
    *socket = INVALID_SOCKET;
    signal_release_server_stop();
    return false;
}

bool retry_pending_owned_sockets() noexcept {
    bool all = true;
    pending_socket_lock();
    size_t out = 0u;
    for (size_t i = 0u; i < g_pending_socket_count; ++i) {
        const SOCKET socket = g_pending_sockets[i];
        if (close_socket_owned(socket) == 0) continue;
        g_pending_sockets[out++] = socket;
        all = false;
    }
    for (size_t i = out; i < g_pending_socket_count; ++i)
        g_pending_sockets[i] = INVALID_SOCKET;
    g_pending_socket_count = out;
    pending_socket_unlock();
    return all;
}

void request_jthread_stop(model_abi::ModelJthread24* thread) noexcept {
    if (!thread) return;
    request_stop(as_stop(thread->stop_state));
}

bool destroy_jthread(model_abi::ModelJthread24* thread) noexcept {
    if (!thread) return true;
    auto* stop = as_stop(thread->stop_state);
    request_stop(stop);
    if (thread->thread_id != 0u) {
        if (thread->thread_id == GetCurrentThreadId()) dispatch_terminate_then_abort_windows();
        HANDLE handle = reinterpret_cast<HANDLE>(thread->handle);
        if (handle) {
            const uint32_t waited = WaitForSingleObject(handle, kInfinite);
            if (waited != kWaitObject0) dispatch_terminate_then_abort_windows();
            if (!close_handle_owned(handle)) {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
                (void)atomic_fetch_add_i32(&g_retained_handle_events, 1);
#endif
                return false;
            }
        }
    }
    release_stop_source(stop);
    *thread = model_abi::ModelJthread24{};
    return true;
}

bool start_accept_jthread(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return false;
    if (service->accept_thread.thread_id != 0u) return true;
    auto* stop = allocate_stop_state();
    if (!stop) return false;
    const auto allocation = allocate_block(sizeof(AcceptClosure10),
                                           AllocationRetryPolicy::outer_retry_only,
                                           windows_allocator_platform_ops());
    if (allocation.terminal != AllocationTerminal::none || !allocation.pointer) {
        release_stop_token(stop); release_stop_source(stop); return false;
    }
    auto* closure = static_cast<AcceptClosure10*>(allocation.pointer);
    closure->stop_state = reinterpret_cast<uintptr_t>(stop);
    closure->service = reinterpret_cast<uintptr_t>(service);
    uint32_t thread_id = 0u;
    HANDLE handle = CreateThread(nullptr, 0, &accept_main, closure, 0, &thread_id);
    if (!handle) {
        release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
        release_stop_token(stop); release_stop_source(stop); return false;
    }
    service->accept_thread.handle = reinterpret_cast<uintptr_t>(handle);
    service->accept_thread.thread_id = thread_id;
    service->accept_thread.stop_state = reinterpret_cast<uintptr_t>(stop);
    return true;
}

bool quiesce_all_client_threads(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    auto* begin = client_begin(service); auto* end = client_end(service);
    for (auto* it = begin; it && it != end; ++it) request_jthread_stop(it);
    for (auto* it = begin; it && it != end; ++it) {
        if (it->thread_id == 0u) continue;
        if (it->thread_id == GetCurrentThreadId()) dispatch_terminate_then_abort_windows();
        HANDLE handle = reinterpret_cast<HANDLE>(it->handle);
        if (handle && WaitForSingleObject(handle, kInfinite) != kWaitObject0) return false;
    }
    return true;
}

bool stop_all_client_threads(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    auto* begin = client_begin(service); auto* end = client_end(service);
    for (auto* it = begin; it && it != end; ++it) request_jthread_stop(it);
    auto* retained = begin;
    for (auto* it = begin; it && it != end; ++it) {
        if (destroy_jthread(it)) continue;
        if (retained != it) {
            *retained = *it;
            *it = model_abi::ModelJthread24{};
        }
        ++retained;
    }
    service->client_records.end = reinterpret_cast<uintptr_t>(retained);
    return retained == begin;
}

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
void configure_release_acceptance_resource_fault_windows(uint32_t selector) noexcept {
    if (selector == 1u) (void)atomic_exchange_i32(&g_fail_next_socket_close, 1);
    else if (selector == 2u) (void)atomic_exchange_i32(&g_fail_next_handle_close, 1);
    else if (selector == 0u) {
        (void)atomic_exchange_i32(&g_fail_next_socket_close, 0);
        (void)atomic_exchange_i32(&g_fail_next_handle_close, 0);
    }
}
uint32_t release_acceptance_resource_fault_metric_windows(uint32_t selector) noexcept {
    if (selector == 1u) return static_cast<uint32_t>(atomic_load_i32(&g_retained_socket_events));
    if (selector == 2u) return static_cast<uint32_t>(atomic_load_i32(&g_retained_handle_events));
    if (selector == 3u) return static_cast<uint32_t>(g_pending_socket_count);
    return 0u;
}
#endif
} // namespace release_server_windows_detail

} }
