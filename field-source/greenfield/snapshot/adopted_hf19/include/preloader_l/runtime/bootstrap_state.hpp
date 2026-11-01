#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

enum class BootstrapPhase : uint32_t {
    cold = 0,
    scheduled = 1,
    running = 2,
    runtime_initialized = 3,
    listener_active = 4,
    client_served = 5,
    completed = 6,
    failed = 7,
};

struct BootstrapSnapshot {
    BootstrapPhase phase;
    uint16_t bound_port;
    uint16_t reserved16;
    uint32_t schedule_count;
    uint32_t run_count;
    uint32_t client_count;
    uint32_t failure_code;
};

void reset_bootstrap_for_test() noexcept;
bool mark_bootstrap_scheduled() noexcept;
bool mark_bootstrap_running() noexcept;
bool mark_bootstrap_runtime_initialized() noexcept;
bool mark_bootstrap_listener_active(uint16_t port) noexcept;
bool mark_bootstrap_client_served() noexcept;
bool mark_bootstrap_completed() noexcept;
// Outer worker cancellation before run_persistent_bootstrap owns no backend
// authority. Complete scheduled -> completed without fabricating a run count.
bool mark_bootstrap_cancelled_before_run() noexcept;
void mark_bootstrap_failed(uint32_t failure_code) noexcept;
// Publish a terminal outer-worker error only when no more specific failure was
// already recorded by the inner runner. Returns true when this call published.
bool mark_bootstrap_failed_if_unreported(uint32_t failure_code) noexcept;
BootstrapSnapshot snapshot_bootstrap() noexcept;

} }
