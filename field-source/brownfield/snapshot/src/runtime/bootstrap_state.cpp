#include "preloader_l/runtime/bootstrap_state.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace runtime {
namespace {
volatile uint32_t g_state_lock = 0;
uint32_t g_phase = static_cast<uint32_t>(BootstrapPhase::cold);
uint16_t g_bound_port = 0;
uint32_t g_schedule_count = 0;
uint32_t g_run_count = 0;
uint32_t g_client_count = 0;
uint32_t g_failure_code = 0;

void lock_bootstrap_state() noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    while (static_cast<uint32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(&g_state_lock), 1, 0)) != 0u) {}
#else
    while (__atomic_exchange_n(&g_state_lock, uint32_t{1}, __ATOMIC_ACQUIRE) != 0u) {
        while (__atomic_load_n(&g_state_lock, __ATOMIC_RELAXED) != 0u) {}
    }
#endif
}

void unlock_bootstrap_state() noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(&g_state_lock), 0);
#else
    __atomic_store_n(&g_state_lock, uint32_t{0}, __ATOMIC_RELEASE);
#endif
}

struct BootstrapStateGuard {
    BootstrapStateGuard() noexcept { lock_bootstrap_state(); }
    ~BootstrapStateGuard() noexcept { unlock_bootstrap_state(); }
    BootstrapStateGuard(const BootstrapStateGuard&) = delete;
    BootstrapStateGuard& operator=(const BootstrapStateGuard&) = delete;
};

bool transition(BootstrapPhase from, BootstrapPhase to) noexcept
{
    if (static_cast<BootstrapPhase>(g_phase) != from) return false;
    g_phase = static_cast<uint32_t>(to);
    return true;
}
}

void reset_bootstrap_for_test() noexcept
{
    BootstrapStateGuard guard;
    g_phase = static_cast<uint32_t>(BootstrapPhase::cold);
    g_bound_port = 0;
    g_schedule_count = 0;
    g_run_count = 0;
    g_client_count = 0;
    g_failure_code = 0;
}

bool mark_bootstrap_scheduled() noexcept
{
    BootstrapStateGuard guard;
    if (!transition(BootstrapPhase::cold, BootstrapPhase::scheduled)) return false;
    ++g_schedule_count;
    return true;
}

bool mark_bootstrap_running() noexcept
{
    BootstrapStateGuard guard;
    if (!transition(BootstrapPhase::scheduled, BootstrapPhase::running)) return false;
    ++g_run_count;
    return true;
}

bool mark_bootstrap_runtime_initialized() noexcept
{
    BootstrapStateGuard guard;
    return transition(BootstrapPhase::running, BootstrapPhase::runtime_initialized);
}

bool mark_bootstrap_listener_active(uint16_t port) noexcept
{
    BootstrapStateGuard guard;
    if (port == 0 || !transition(BootstrapPhase::runtime_initialized, BootstrapPhase::listener_active))
        return false;
    g_bound_port = port;
    return true;
}

bool mark_bootstrap_client_served() noexcept
{
    BootstrapStateGuard guard;
    const BootstrapPhase phase = static_cast<BootstrapPhase>(g_phase);
    if (phase != BootstrapPhase::listener_active && phase != BootstrapPhase::client_served)
        return false;
    g_phase = static_cast<uint32_t>(BootstrapPhase::client_served);
    ++g_client_count;
    return true;
}

bool mark_bootstrap_completed() noexcept
{
    BootstrapStateGuard guard;
    const BootstrapPhase phase = static_cast<BootstrapPhase>(g_phase);
    if (phase != BootstrapPhase::running &&
        phase != BootstrapPhase::runtime_initialized &&
        phase != BootstrapPhase::listener_active &&
        phase != BootstrapPhase::client_served)
        return false;
    g_phase = static_cast<uint32_t>(BootstrapPhase::completed);
    return true;
}

bool mark_bootstrap_cancelled_before_run() noexcept
{
    BootstrapStateGuard guard;
    if (!transition(BootstrapPhase::scheduled, BootstrapPhase::completed)) return false;
    return true;
}

void mark_bootstrap_failed(uint32_t failure_code) noexcept
{
    BootstrapStateGuard guard;
    g_failure_code = failure_code == 0 ? 1u : failure_code;
    g_phase = static_cast<uint32_t>(BootstrapPhase::failed);
}

bool mark_bootstrap_failed_if_unreported(uint32_t failure_code) noexcept
{
    BootstrapStateGuard guard;
    if (static_cast<BootstrapPhase>(g_phase) == BootstrapPhase::failed) return false;
    g_failure_code = failure_code == 0 ? 1u : failure_code;
    g_phase = static_cast<uint32_t>(BootstrapPhase::failed);
    return true;
}

BootstrapSnapshot snapshot_bootstrap() noexcept
{
    BootstrapStateGuard guard;
    BootstrapSnapshot out{};
    out.phase = static_cast<BootstrapPhase>(g_phase);
    out.bound_port = g_bound_port;
    out.reserved16 = 0;
    out.schedule_count = g_schedule_count;
    out.run_count = g_run_count;
    out.client_count = g_client_count;
    out.failure_code = g_failure_code;
    return out;
}

} }
