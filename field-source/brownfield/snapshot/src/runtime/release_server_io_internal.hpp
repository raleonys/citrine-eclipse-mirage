#pragma once
#if !defined(_WIN32)
#error release_server_io_internal.hpp is Windows-only.
#endif

#include "preloader_l/runtime/release_server_io.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#include "preloader_l/runtime/event_trace.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/fatal_termination.hpp"
#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace runtime { namespace release_server_windows_detail {

using SOCKET = uintptr_t;
using HANDLE = void*;
constexpr SOCKET INVALID_SOCKET = static_cast<SOCKET>(~static_cast<SOCKET>(0));
constexpr int WSAEWOULDBLOCK = 10035;
constexpr int kAfInet = 2;
constexpr uint32_t kWaitObject0 = 0u;
constexpr uint32_t kWaitTimeout = 258u;
constexpr uint32_t kInfinite = 0xffffffffu;
constexpr uint32_t kWaitSliceMs = 100u;

struct timeval { long tv_sec; long tv_usec; };
struct fd_set_native { unsigned int fd_count; SOCKET fd_array[64]; };

extern "C" {
__declspec(dllimport) int __stdcall WSAGetLastError();
__declspec(dllimport) int __stdcall select(int,fd_set_native*,fd_set_native*,fd_set_native*,const timeval*);
__declspec(dllimport) SOCKET __stdcall accept(SOCKET,void*,int*);
__declspec(dllimport) const char* __stdcall inet_ntop(int,const void*,char*,size_t);
__declspec(dllimport) int __stdcall recv(SOCKET,char*,int,int);
__declspec(dllimport) int __stdcall send(SOCKET,const char*,int,int);
__declspec(dllimport) int __stdcall ioctlsocket(SOCKET,long,uint32_t*);
__declspec(dllimport) int __stdcall closesocket(SOCKET);
__declspec(dllimport) HANDLE __stdcall CreateThread(void*, size_t, uint32_t (__stdcall *)(void*), void*, uint32_t, uint32_t*);
__declspec(dllimport) uint32_t __stdcall WaitForSingleObject(HANDLE, uint32_t);
__declspec(dllimport) int __stdcall CloseHandle(HANDLE);
__declspec(dllimport) uint32_t __stdcall GetCurrentThreadId();
__declspec(dllimport) unsigned long long __stdcall GetTickCount64();
__declspec(dllimport) void __stdcall Sleep(unsigned long);
__declspec(dllimport) void __stdcall AcquireSRWLockExclusive(void*);
__declspec(dllimport) void __stdcall ReleaseSRWLockExclusive(void*);
}

bool release_server_stop_requested() noexcept;
void reset_release_server_stop() noexcept;
void signal_release_server_stop() noexcept;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
void reset_release_acceptance_counters() noexcept;
void note_release_acceptance_accepted() noexcept;
void note_release_acceptance_started() noexcept;
void note_release_acceptance_completed() noexcept;
void note_release_acceptance_peer_nonempty() noexcept;
void note_release_acceptance_reaped() noexcept;
// HF13: test-only Asio session-registry ownership bookkeeping. The service
// remains authoritative; these counters do not drive Production admission.
void note_release_acceptance_asio_owner_opened() noexcept;
void note_release_acceptance_asio_owner_retired() noexcept;
uint32_t release_acceptance_asio_live_owners() noexcept;
uint32_t release_acceptance_counter(uint32_t selector) noexcept;
#endif

inline int32_t atomic_load_i32(volatile int32_t* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(value), 0, 0));
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}

inline int32_t atomic_exchange_i32(volatile int32_t* value, int32_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedExchange(
        reinterpret_cast<volatile long*>(value), static_cast<long>(desired)));
#else
    return __atomic_exchange_n(value, desired, __ATOMIC_ACQ_REL);
#endif
}

inline bool atomic_compare_exchange_i32(
    volatile int32_t* value, int32_t expected, int32_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(value),
        static_cast<long>(desired),
        static_cast<long>(expected))) == expected;
#else
    return __atomic_compare_exchange_n(
        value, &expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#endif
}

inline int32_t atomic_fetch_add_i32(volatile int32_t* value, int32_t delta) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<int32_t>(_InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(value), static_cast<long>(delta)));
#else
    return __atomic_fetch_add(value, delta, __ATOMIC_ACQ_REL);
#endif
}

inline uint32_t atomic_load_u32(volatile uint32_t* value) noexcept {
    return static_cast<uint32_t>(atomic_load_i32(reinterpret_cast<volatile int32_t*>(value)));
}

inline uint32_t atomic_exchange_u32(volatile uint32_t* value, uint32_t desired) noexcept {
    return static_cast<uint32_t>(atomic_exchange_i32(
        reinterpret_cast<volatile int32_t*>(value), static_cast<int32_t>(desired)));
}

inline uint32_t atomic_fetch_add_u32(volatile uint32_t* value, int32_t delta) noexcept {
    return static_cast<uint32_t>(atomic_fetch_add_i32(
        reinterpret_cast<volatile int32_t*>(value), delta));
}

inline void store_active(model_abi::ReleaseServiceObject548* service, uint8_t value) noexcept {
    if (!service) return;
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange8(reinterpret_cast<volatile char*>(&service->active),
                               static_cast<char>(value));
#else
    __atomic_store_n(&service->active, value, __ATOMIC_RELEASE);
#endif
}

inline uint8_t load_active(const model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return 0u;
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint8_t>(_InterlockedCompareExchange8(
        reinterpret_cast<volatile char*>(const_cast<uint8_t*>(&service->active)), 0, 0));
#else
    return __atomic_load_n(&service->active, __ATOMIC_ACQUIRE);
#endif
}

inline bool bootstrap_stopping() noexcept {
    return release_server_stop_requested() || bootstrap_stop_requested();
}

inline model_abi::ModelStopState20* as_stop(uintptr_t value) noexcept {
    return reinterpret_cast<model_abi::ModelStopState20*>(value);
}

inline model_abi::ModelStopState20* allocate_stop_state() noexcept {
    const auto result = allocate_block(sizeof(model_abi::ModelStopState20),
                                       AllocationRetryPolicy::outer_retry_only,
                                       windows_allocator_platform_ops());
    if (result.terminal != AllocationTerminal::none || !result.pointer) return nullptr;
    auto* state = static_cast<model_abi::ModelStopState20*>(result.pointer);
    *state = model_abi::ModelStopState20{};
    state->lifetime_refs = 2;
    state->source_refs_and_stop = 2;
    return state;
}

inline void release_stop_token(model_abi::ModelStopState20* state) noexcept {
    if (!state) return;
    if (atomic_fetch_add_i32(&state->lifetime_refs, -1) == 1)
        release_block(state, sizeof(*state), windows_allocator_platform_ops());
}

inline void release_stop_source(model_abi::ModelStopState20* state) noexcept {
    if (!state) return;
    const int32_t old = atomic_fetch_add_i32(&state->source_refs_and_stop, -2);
    if ((old & ~1) == 2) release_stop_token(state);
}

inline bool stop_requested(const model_abi::ModelStopState20* state) noexcept {
    if (!state) return false;
    return (atomic_load_i32(
        const_cast<volatile int32_t*>(&state->source_refs_and_stop)) & 1) != 0;
}

inline void request_stop(model_abi::ModelStopState20* state) noexcept {
    if (!state) return;
#if defined(_MSC_VER) && !defined(__clang__)
    for (;;) {
        const long old = _InterlockedCompareExchange(
            reinterpret_cast<volatile long*>(&state->source_refs_and_stop), 0, 0);
        if ((old & 1) != 0) return;
        if (_InterlockedCompareExchange(
                reinterpret_cast<volatile long*>(&state->source_refs_and_stop),
                old | 1, old) == old) return;
    }
#else
    (void)__atomic_fetch_or(&state->source_refs_and_stop, 1, __ATOMIC_ACQ_REL);
#endif
}

inline bool lock_service_mutex(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return false;
    auto& mutex = service->mutex_state;
    const uint32_t tid = GetCurrentThreadId();

    // HOTFIX10/F1601-001: owner/recursion are part of the curio physical
    // ABI, so keep their layout unchanged but make every concurrent pre-lock
    // observation and recursive update atomic. The SRW lock remains the
    // exclusion policy for first acquisition.
    if (atomic_load_u32(&mutex.owner_thread_id) == tid) {
        const uint32_t recursion = atomic_load_u32(&mutex.recursion_count);
        if (recursion != 0u) {
            if (recursion == 0x7fffffffu) return false;
            (void)atomic_fetch_add_u32(&mutex.recursion_count, 1);
            return true;
        }
    }

    AcquireSRWLockExclusive(&mutex.srw_lock_word);
    // Publish recursion before owner so another thread can never observe this
    // TID as owner with a zero recursion count. Interlocked operations provide
    // the publication boundary without changing ModelRecursiveMutex50.
    (void)atomic_exchange_u32(&mutex.recursion_count, 1u);
    (void)atomic_exchange_u32(&mutex.owner_thread_id, tid);
    return true;
}

inline void unlock_service_mutex(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return;
    auto& mutex = service->mutex_state;
    const uint32_t tid = GetCurrentThreadId();
    if (atomic_load_u32(&mutex.owner_thread_id) != tid) return;
    const uint32_t recursion = atomic_load_u32(&mutex.recursion_count);
    if (recursion == 0u) return;
    if (recursion > 1u) {
        (void)atomic_fetch_add_u32(&mutex.recursion_count, -1);
        return;
    }

    // Retract recursive ownership before releasing the SRW lock. A competing
    // thread that observes the sentinel must still acquire the SRW lock before
    // publishing itself as owner.
    (void)atomic_exchange_u32(&mutex.owner_thread_id, 0xffffffffu);
    (void)atomic_exchange_u32(&mutex.recursion_count, 0u);
    ReleaseSRWLockExclusive(&mutex.srw_lock_word);
}

inline model_abi::ModelJthread24* client_begin(model_abi::ReleaseServiceObject548* service) noexcept {
    return service ? reinterpret_cast<model_abi::ModelJthread24*>(service->client_records.begin) : nullptr;
}
inline model_abi::ModelJthread24* client_end(model_abi::ReleaseServiceObject548* service) noexcept {
    return service ? reinterpret_cast<model_abi::ModelJthread24*>(service->client_records.end) : nullptr;
}
inline model_abi::ModelJthread24* client_capacity(model_abi::ReleaseServiceObject548* service) noexcept {
    return service ? reinterpret_cast<model_abi::ModelJthread24*>(service->client_records.capacity) : nullptr;
}
inline size_t client_size(model_abi::ReleaseServiceObject548* service) noexcept {
    auto* begin = client_begin(service); auto* end = client_end(service);
    return begin && end ? static_cast<size_t>(end - begin) : 0u;
}
inline size_t client_capacity_count(model_abi::ReleaseServiceObject548* service) noexcept {
    auto* begin = client_begin(service); auto* cap = client_capacity(service);
    return begin && cap ? static_cast<size_t>(cap - begin) : 0u;
}

inline void server_trace(ReleaseServerContext* context, TraceEvent event, uint32_t value) noexcept {
#if PRELOADER_L_TRACE_DIAGNOSTICS
    PRELOADER_L_TRACE_NOTE(event, value);
    if (context && context->trace) context->trace(event, value);
#else
    (void)context; (void)event; (void)value;
#endif
}

inline bool semantic_terminal(ReleaseServerContext* context, uint32_t code) noexcept {
    server_trace(context, TraceEvent::failure, code);
    dispatch_terminate_then_abort_windows();
    return false;
}

bool create_worker_jthread(model_abi::ModelJthread24* out,
                           model_abi::ReleaseServiceObject548* service,
                           SOCKET client, const char* remote_address,
                           size_t remote_address_size) noexcept;
bool close_owned_socket_or_retain(SOCKET* socket) noexcept;
bool retry_pending_owned_sockets() noexcept;
void request_jthread_stop(model_abi::ModelJthread24* thread) noexcept;
bool destroy_jthread(model_abi::ModelJthread24* thread) noexcept;
bool start_accept_jthread(model_abi::ReleaseServiceObject548* service) noexcept;
bool quiesce_all_client_threads(model_abi::ReleaseServiceObject548* service) noexcept;
bool stop_all_client_threads(model_abi::ReleaseServiceObject548* service) noexcept;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
void configure_release_acceptance_resource_fault_windows(uint32_t selector) noexcept;
uint32_t release_acceptance_resource_fault_metric_windows(uint32_t selector) noexcept;
#endif

} } }
