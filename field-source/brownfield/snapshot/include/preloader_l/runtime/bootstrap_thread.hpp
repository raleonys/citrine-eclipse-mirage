#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Called while DllMain owns the loader lock. It schedules the persistent
// runtime worker and takes a reference-counted module self-pin for that worker.
bool schedule_release_bootstrap(void* module) noexcept;

enum class ReleaseBootstrapReadyResult : uint32_t {
    ready = 0,
    timeout = 1,
    failed = 2,
    invalid_state = 3,
    unsupported = 4,
};

enum class ReleaseBootstrapWaitResult : uint32_t {
    stopped = 0,
    not_running = 1,
    timeout = 2,
    failed = 3,
    handle_close_failed = 4,
};

// Host lifecycle surface. Never call wait_release_bootstrap_stopped or the
// unload shutdown path from DllMain; both intentionally wait outside loader lock.
ReleaseBootstrapReadyResult wait_release_bootstrap_ready(uint32_t timeout_ms) noexcept;
void request_release_bootstrap_stop() noexcept;
ReleaseBootstrapWaitResult wait_release_bootstrap_stopped(uint32_t timeout_ms) noexcept;

enum class ReleaseRuntimeShutdownResult : uint32_t {
    shutdown = 0,
    busy = 1,
    bootstrap_timeout = 2,
    bootstrap_wait_failed = 3,
    service_active_leases = 4,
    hook_teardown_failed = 5,
    ptd_cleanup_pending = 6,
    bootstrap_handle_close_failed = 7,
    bootstrap_pin_release_failed = 8,
    environment_cleanup_pending = 9,
    service_cleanup_pending = 10,
    hook_quiescence_timeout = 11,
    platform_lock_cleanup_pending = 12,
};

// Explicit pre-unload lifecycle owner. timeout_ms is an end-to-end budget for
// bootstrap quiescence and hook active-call drain; 0xffffffff means infinite.
// A zero result means the persistent
// bootstrap thread is genuinely terminated, the worker self-pin has been
// released, the permanent service owner is gone, hooks are removed, and PTD
// cleanup is complete. The caller may then release its final module reference.
ReleaseRuntimeShutdownResult shutdown_release_runtime_for_unload(uint32_t timeout_ms) noexcept;

} }
