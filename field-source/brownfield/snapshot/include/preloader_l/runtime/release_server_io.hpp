#pragma once
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/service_abi.hpp"

namespace preloader_l { namespace runtime {

// Native-Winsock implementation of the active service worker semantic ABI.
// +0x018 owns the accept jthread, +0x030 owns client jthreads, and +0x048
// serializes append/bulk-stop mutation. There is no project-specific worker
// registry. HOTFIX10 adds a project safety ceiling for simultaneously-owned
// client workers so remote connection pressure cannot grow the owner vector
// without bound or turn per-client allocation/thread failure into a process-
// terminal policy path. This is a project hardening rule, not behavioral consistency.
constexpr size_t release_client_hard_cap = 128u;
inline bool release_client_admission_allowed(size_t live_client_count) noexcept {
    return live_client_count < release_client_hard_cap;
}
bool initialize_release_worker_policy_windows(
    model_abi::ReleaseServiceObject548* service) noexcept;
void quiesce_release_worker_policy_windows(ReleaseServerContext* context) noexcept;
// Retryable physical retirement used before the service reference owner is
// released. Failed socket/handle/vector retirement preserves exact ownership
// for a later DllShutdown retry.
bool prepare_release_worker_policy_retirement_windows(
    model_abi::ReleaseServiceObject548* service) noexcept;
void release_release_worker_policy_windows(
    model_abi::ReleaseServiceObject548* service) noexcept;

enum class ReleaseSocketWaitStatus : uint8_t { ready, timeout, error };

// Clean Windows liveness policy layered outside the curio service-object
// bytes. Native listener/client sockets are explicit nonblocking endpoints;
// select() is used only as a bounded readiness wait so stop-state publication
// is observed without relying on concurrent closesocket() interruption.
bool configure_release_socket_nonblocking_windows(uintptr_t socket) noexcept;
ReleaseSocketWaitStatus wait_release_socket_windows(
    uintptr_t socket, bool writing) noexcept;

bool prepare_release_listener_windows(ReleaseServerContext* context) noexcept;
bool serve_release_workers_windows(ReleaseServerContext* context) noexcept;
void reset_release_server_stop_windows() noexcept;
void request_release_server_stop_windows() noexcept;
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
// Test-only observation surface. These counters are CLEAN_BOOKKEEPING and are
// never consulted by shipping worker policy. selector: 0 port, 1 accepted,
// 2 started, 3 completed, 4 peer-nonempty, 5 owner-count, 6 owner-capacity, 7 reaped-owner count.
uint32_t release_worker_acceptance_metric_windows(
    model_abi::ReleaseServiceObject548* service, uint32_t selector) noexcept;
// F02 native-acceptance fault injection. Test builds can force exactly one
// worker thread to observe would_block on every send while unrelated workers
// remain unaffected. Shipping builds do not compile this surface.
void configure_release_acceptance_send_block_windows(bool enabled) noexcept;
uint32_t release_acceptance_send_block_metric_windows(uint32_t selector) noexcept;
#endif

} }
