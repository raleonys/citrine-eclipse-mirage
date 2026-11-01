#if !defined(_WIN32)
#error release_server_worker.cpp is Windows-only.
#endif
#include "release_server_io_internal.hpp"
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
#include "asio_acceptance_delivery.hpp"
#include <atomic>
#endif

namespace preloader_l { namespace runtime {
namespace detail = release_server_windows_detail;
namespace {

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
volatile int32_t g_acceptance_send_block_enabled = 0;
volatile int32_t g_acceptance_send_block_owner_thread = 0;
volatile int32_t g_acceptance_send_block_hits = 0;
// Session identity, not an IOCP worker thread: avoids blocking unrelated
// Asio sessions driven by the same io_context thread. Native-only.
std::atomic<const void*> g_acceptance_asio_stall_session{nullptr};
#endif

#if PRELOADER_L_TRACE_DIAGNOSTICS
uint32_t next_session_id(ReleaseServerContext* context) noexcept {
    if (!context) return 0u;
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(&context->session_id), 1));
#else
    return __atomic_fetch_add(&context->session_id, 1u, __ATOMIC_ACQ_REL);
#endif
}
#endif

void initialize_empty_string(model_abi::MsvcString32* text) noexcept {
    if (!text) return;
    *text = model_abi::MsvcString32{};
    text->capacity = 0x0fu;
    text->storage[0] = 0u;
}

void release_model_string(model_abi::MsvcString32* text) noexcept {
    if (!text) return;
    if (text->capacity > 0x0fu) {
        uintptr_t pointer = 0u;
        for (size_t i = 0; i < sizeof(pointer); ++i)
            pointer |= static_cast<uintptr_t>(text->storage[i]) << (i * 8u);
        if (pointer)
            release_block(reinterpret_cast<void*>(pointer),
                          static_cast<size_t>(text->capacity + 1u),
                          windows_allocator_platform_ops());
    }
    initialize_empty_string(text);
}

struct WorkerEndpoint {
    detail::SOCKET socket;
    model_abi::ReleaseServiceObject548* service;
    model_abi::ModelStopState20* stop;
};

bool worker_stopping(const WorkerEndpoint* endpoint) noexcept {
    return !endpoint || !detail::load_active(endpoint->service) ||
           detail::stop_requested(endpoint->stop);
}

ReleaseSocketWaitStatus wait_ready(WorkerEndpoint* endpoint, bool writing) noexcept {
    if (!endpoint || endpoint->socket == detail::INVALID_SOCKET)
        return ReleaseSocketWaitStatus::error;
    if (worker_stopping(endpoint)) return ReleaseSocketWaitStatus::timeout;
    return wait_release_socket_windows(endpoint->socket, writing);
}

preloader_l::net::TransportStatus worker_receive(
    void* opaque, uint8_t* out, size_t cap, size_t* received) noexcept {
    if (received) *received = 0;
    auto* endpoint = static_cast<WorkerEndpoint*>(opaque);
    if (!endpoint || !out || !received || !cap || cap > 0x7fffffffu)
        return preloader_l::net::TransportStatus::invalid_argument;
    if (endpoint->socket == detail::INVALID_SOCKET || worker_stopping(endpoint))
        return preloader_l::net::TransportStatus::closed;
    const auto ready = wait_ready(endpoint, false);
    if (ready == ReleaseSocketWaitStatus::timeout)
        return worker_stopping(endpoint) ? preloader_l::net::TransportStatus::closed
                                         : preloader_l::net::TransportStatus::would_block;
    if (ready == ReleaseSocketWaitStatus::error) return preloader_l::net::TransportStatus::io_error;
    const int rc = detail::recv(endpoint->socket, reinterpret_cast<char*>(out),
                                static_cast<int>(cap), 0);
    if (rc > 0) { *received = static_cast<size_t>(rc); return preloader_l::net::TransportStatus::ok; }
    if (rc == 0) return preloader_l::net::TransportStatus::closed;
    return detail::WSAGetLastError() == detail::WSAEWOULDBLOCK
        ? preloader_l::net::TransportStatus::would_block
        : preloader_l::net::TransportStatus::io_error;
}

preloader_l::net::TransportStatus worker_send(
    void* opaque, const uint8_t* in, size_t len, size_t* sent) noexcept {
    if (sent) *sent = 0;
    auto* endpoint = static_cast<WorkerEndpoint*>(opaque);
    if (!endpoint || !in || !sent || !len || len > 0x7fffffffu)
        return preloader_l::net::TransportStatus::invalid_argument;
    if (endpoint->socket == detail::INVALID_SOCKET || worker_stopping(endpoint))
        return preloader_l::net::TransportStatus::closed;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // F02 test-only discriminator: the first worker thread that attempts a send
    // after the control is enabled owns the artificial backpressure. Other
    // workers continue through the real Winsock path, allowing the acceptance
    // host to prove there is no process-wide delivery-lock convoy.
    if (detail::atomic_load_i32(&g_acceptance_send_block_enabled) != 0) {
        const int32_t tid = static_cast<int32_t>(detail::GetCurrentThreadId());
        int32_t owner = detail::atomic_load_i32(&g_acceptance_send_block_owner_thread);
        if (owner == 0) {
            (void)detail::atomic_compare_exchange_i32(
                &g_acceptance_send_block_owner_thread, 0, tid);
            owner = detail::atomic_load_i32(&g_acceptance_send_block_owner_thread);
        }
        if (owner == tid) {
            (void)detail::atomic_fetch_add_i32(&g_acceptance_send_block_hits, 1);
            detail::Sleep(100u);
            return preloader_l::net::TransportStatus::would_block;
        }
    }
#endif
    const auto ready = wait_ready(endpoint, true);
    if (ready == ReleaseSocketWaitStatus::timeout)
        return worker_stopping(endpoint) ? preloader_l::net::TransportStatus::closed
                                         : preloader_l::net::TransportStatus::would_block;
    if (ready == ReleaseSocketWaitStatus::error) return preloader_l::net::TransportStatus::io_error;
    const int rc = detail::send(endpoint->socket, reinterpret_cast<const char*>(in),
                                static_cast<int>(len), 0);
    if (rc > 0) { *sent = static_cast<size_t>(rc); return preloader_l::net::TransportStatus::ok; }
    if (rc == 0) return preloader_l::net::TransportStatus::closed;
    return detail::WSAGetLastError() == detail::WSAEWOULDBLOCK
        ? preloader_l::net::TransportStatus::would_block
        : preloader_l::net::TransportStatus::io_error;
}

void worker_close(void* opaque) noexcept {
    auto* endpoint = static_cast<WorkerEndpoint*>(opaque);
    if (!endpoint || endpoint->socket == detail::INVALID_SOCKET) return;
    (void)detail::close_owned_socket_or_retain(&endpoint->socket);
}

uint64_t worker_monotonic_milliseconds(void*) noexcept {
    return static_cast<uint64_t>(detail::GetTickCount64());
}

uint32_t __stdcall worker_main(void* opaque) noexcept {
    auto* closure = static_cast<model_abi::ModelWorkerClosure38*>(opaque);
    if (!closure) return 0u;
    auto* service = reinterpret_cast<model_abi::ReleaseServiceObject548*>(closure->service);
    auto* stop = detail::as_stop(closure->stop_state);
    ReleaseServerContext* context = nullptr;
#if PRELOADER_L_TRACE_DIAGNOSTICS
    context = model_service_adapter(service);
    const uint32_t session = next_session_id(context);
    detail::server_trace(context, TraceEvent::client_accepted, session);
#endif

    WorkerEndpoint state{static_cast<detail::SOCKET>(closure->accepted_socket), service, stop};
    constexpr uint32_t kSendIdleTimeoutMs = 5000u;
    constexpr uint32_t kReceiveIdleTimeoutMs = 30000u;
    constexpr uint32_t kReceiveFrameTimeoutMs = 10000u;
    preloader_l::net::TransportEndpoint endpoint{
        &state, &worker_receive, &worker_send, &worker_close,
        &worker_monotonic_milliseconds, kSendIdleTimeoutMs,
        kReceiveIdleTimeoutMs, kReceiveFrameTimeoutMs};

    const auto protocol_allocation = allocate_block(
        sizeof(preloader_l::protocol::ExchangeState),
        AllocationRetryPolicy::outer_retry_only, windows_allocator_platform_ops());
    if (protocol_allocation.terminal != AllocationTerminal::none ||
        !protocol_allocation.pointer) {
        worker_close(&state);
        detail::server_trace(context, TraceEvent::failure,
             0x5c00u + static_cast<uint32_t>(preloader_l::protocol::ExchangeStatus::resource_error));
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        detail::note_release_acceptance_completed();
#endif
        release_model_string(&closure->remote_address);
        detail::release_stop_token(stop);
        release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
        return 0u;
    }
    auto* protocol = static_cast<preloader_l::protocol::ExchangeState*>(
        protocol_allocation.pointer);
    preloader_l::protocol::reset_exchange(protocol);
    const auto peer = model_service_string_view(closure->remote_address);
    const preloader_l::protocol::SessionPeerContext peer_context{peer.data, peer.size};
    size_t bytes = 0; uint32_t frames = 0;
    const auto status = preloader_l::protocol::serve_protocol_session_with_peer(
        endpoint, protocol, peer_context, &bytes, &frames);
    if (frames) detail::server_trace(context, TraceEvent::frame_dispatched, frames);
    if (bytes) detail::server_trace(context, TraceEvent::response_sent, static_cast<uint32_t>(bytes));
    worker_close(&state);
    preloader_l::protocol::reset_exchange(protocol);
    release_block(protocol, sizeof(*protocol), windows_allocator_platform_ops());
#if PRELOADER_L_TRACE_DIAGNOSTICS
    if (status == preloader_l::protocol::ExchangeStatus::peer_closed)
        detail::server_trace(context, TraceEvent::peer_closed, session);
    else if (!worker_stopping(&state))
        detail::server_trace(context, TraceEvent::failure, 0x5c00u + static_cast<uint32_t>(status));
    detail::server_trace(context, TraceEvent::persistent_session_closed, session);
#else
    (void)status;
#endif

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    detail::note_release_acceptance_completed();
#endif
    release_model_string(&closure->remote_address);
    detail::release_stop_token(stop);
    release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
    return 0u;
}

} // namespace

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
static std::atomic<uint32_t> g_acceptance_asio_send_idle_timeout_count{0u};
void configure_release_acceptance_send_block_windows(bool enabled) noexcept {
    if (enabled) {
        (void)detail::atomic_exchange_i32(&g_acceptance_send_block_owner_thread, 0);
        g_acceptance_asio_stall_session.store(nullptr, std::memory_order_release);
        (void)detail::atomic_exchange_i32(&g_acceptance_send_block_hits, 0);
        g_acceptance_asio_send_idle_timeout_count.store(0u,std::memory_order_release);
        (void)detail::atomic_exchange_i32(&g_acceptance_send_block_enabled, 1);
    } else {
        (void)detail::atomic_exchange_i32(&g_acceptance_send_block_enabled, 0);
        g_acceptance_asio_stall_session.store(nullptr, std::memory_order_release);
        (void)detail::atomic_exchange_i32(&g_acceptance_send_block_owner_thread, 0);
    }
}

uint32_t release_acceptance_send_block_metric_windows(uint32_t selector) noexcept {
    switch (selector) {
    case 0u: return static_cast<uint32_t>(
        detail::atomic_load_i32(&g_acceptance_send_block_enabled));
    case 1u: {
        // Legacy path reports an owner thread. Asio has a per-session owner
        // on a shared IOCP thread; the host only requires nonzero ownership.
        const auto legacy=detail::atomic_load_i32(&g_acceptance_send_block_owner_thread);
        if (legacy!=0) return static_cast<uint32_t>(legacy);
        return g_acceptance_asio_stall_session.load(std::memory_order_acquire) ? 1u : 0u;
    }
    case 2u: return static_cast<uint32_t>(
        detail::atomic_load_i32(&g_acceptance_send_block_hits));
    case 5u: return g_acceptance_asio_send_idle_timeout_count.load(std::memory_order_acquire);
    default: return 0u;
    }
}
// The first Asio session needing a prepared write claims the artificial
// stall. The gate never sleeps and never takes a global delivery mutex.
// Only the claimed session retries; other sessions write normally.
void release_acceptance_note_send_idle_timeout_windows() noexcept {
    (void)g_acceptance_asio_send_idle_timeout_count.fetch_add(1u,std::memory_order_acq_rel);
}
bool release_acceptance_should_hold_asio_send_windows(const void* session) noexcept {
    if (!session || detail::atomic_load_i32(&g_acceptance_send_block_enabled)==0)
        return false;
    const void* expected=nullptr;
    (void)g_acceptance_asio_stall_session.compare_exchange_strong(
        expected,session,std::memory_order_acq_rel,std::memory_order_acquire);
    if (g_acceptance_asio_stall_session.load(std::memory_order_acquire)!=session)
        return false;
    // Disable can race a timer tick: fail open for the *test injection* only.
    if (detail::atomic_load_i32(&g_acceptance_send_block_enabled)==0) return false;
    (void)detail::atomic_fetch_add_i32(&g_acceptance_send_block_hits,1);
    return true;
}
#endif

bool configure_release_socket_nonblocking_windows(uintptr_t socket_value) noexcept {
    const detail::SOCKET socket = static_cast<detail::SOCKET>(socket_value);
    if (socket == detail::INVALID_SOCKET) return false;
    uint32_t enabled = 1u;
    constexpr long kFionbio = static_cast<long>(0x8004667eu);
    return detail::ioctlsocket(socket, kFionbio, &enabled) == 0;
}

ReleaseSocketWaitStatus wait_release_socket_windows(
    uintptr_t socket_value, bool writing) noexcept {
    const detail::SOCKET socket = static_cast<detail::SOCKET>(socket_value);
    if (socket == detail::INVALID_SOCKET) return ReleaseSocketWaitStatus::error;
    detail::fd_set_native set{};
    set.fd_count = 1u;
    set.fd_array[0] = socket;
    detail::timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    const int rc = detail::select(0, writing ? nullptr : &set, writing ? &set : nullptr,
                                  nullptr, &timeout);
    if (rc > 0) return ReleaseSocketWaitStatus::ready;
    if (rc == 0) return ReleaseSocketWaitStatus::timeout;
    return ReleaseSocketWaitStatus::error;
}

namespace release_server_windows_detail {
bool create_worker_jthread(model_abi::ModelJthread24* out,
                           model_abi::ReleaseServiceObject548* service,
                           SOCKET client, const char* remote_address,
                           size_t remote_address_size) noexcept {
    if (!out || !service || client == INVALID_SOCKET) return false;
    *out = model_abi::ModelJthread24{};
    auto* stop = allocate_stop_state();
    if (!stop) return false;

    const auto closure_allocation = allocate_block(
        sizeof(model_abi::ModelWorkerClosure38),
        AllocationRetryPolicy::outer_retry_only, windows_allocator_platform_ops());
    if (closure_allocation.terminal != AllocationTerminal::none ||
        !closure_allocation.pointer) {
        release_stop_token(stop);
        release_stop_source(stop);
        return false;
    }
    auto* closure = static_cast<model_abi::ModelWorkerClosure38*>(
        closure_allocation.pointer);
    *closure = model_abi::ModelWorkerClosure38{};
    closure->stop_state = reinterpret_cast<uintptr_t>(stop);
    closure->service = reinterpret_cast<uintptr_t>(service);
    closure->accepted_socket = client;
    initialize_empty_string(&closure->remote_address);
    if (!assign_model_service_string(
            &closure->remote_address,
            preloader_l::config::ValueView{remote_address, remote_address_size, true})) {
        release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
        release_stop_token(stop);
        release_stop_source(stop);
        return false;
    }

    uint32_t thread_id = 0u;
    HANDLE handle = CreateThread(nullptr, 0, &worker_main, closure, 0, &thread_id);
    if (!handle) {
        release_model_string(&closure->remote_address);
        release_block(closure, sizeof(*closure), windows_allocator_platform_ops());
        release_stop_token(stop);
        release_stop_source(stop);
        return false;
    }
    out->handle = reinterpret_cast<uintptr_t>(handle);
    out->thread_id = thread_id;
    out->stop_state = reinterpret_cast<uintptr_t>(stop);
    return true;
}
} // namespace release_server_windows_detail

} }
