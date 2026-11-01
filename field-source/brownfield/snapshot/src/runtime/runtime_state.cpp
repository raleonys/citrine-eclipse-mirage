#include "preloader_l/runtime/runtime_state.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace runtime {
namespace {
volatile uint32_t g_phase = static_cast<uint32_t>(Phase::cold);
volatile uintptr_t g_module = 0;
volatile uint32_t g_generation = 0;
volatile uint32_t g_initialize_count = 0;
volatile uint32_t g_shutdown_count = 0;
volatile uint32_t g_detach_kind = static_cast<uint32_t>(DetachKind::none);

uint32_t atomic_load32(const volatile uint32_t* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(const_cast<volatile uint32_t*>(value)), 0, 0));
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}

void atomic_store32(volatile uint32_t* value, uint32_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(value),
                               static_cast<long>(desired));
#else
    __atomic_store_n(value, desired, __ATOMIC_RELEASE);
#endif
}

uint32_t atomic_increment32(volatile uint32_t* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedIncrement(
        reinterpret_cast<volatile long*>(value)));
#else
    return __atomic_add_fetch(value, uint32_t{1}, __ATOMIC_ACQ_REL);
#endif
}

bool atomic_compare_exchange32(volatile uint32_t* value,
                               uint32_t expected,
                               uint32_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return static_cast<uint32_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(value), static_cast<long>(desired),
        static_cast<long>(expected))) == expected;
#else
    return __atomic_compare_exchange_n(value, &expected, desired, false,
                                       __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#endif
}

uintptr_t atomic_load_ptr(const volatile uintptr_t* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_WIN64)
    return static_cast<uintptr_t>(_InterlockedCompareExchange64(
        reinterpret_cast<volatile __int64*>(const_cast<volatile uintptr_t*>(value)), 0, 0));
#else
    return static_cast<uintptr_t>(_InterlockedCompareExchange(
        reinterpret_cast<volatile long*>(const_cast<volatile uintptr_t*>(value)), 0, 0));
#endif
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}

void atomic_store_ptr(volatile uintptr_t* value, uintptr_t desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_WIN64)
    (void)_InterlockedExchange64(reinterpret_cast<volatile __int64*>(value),
                                 static_cast<__int64>(desired));
#else
    (void)_InterlockedExchange(reinterpret_cast<volatile long*>(value),
                               static_cast<long>(desired));
#endif
#else
    __atomic_store_n(value, desired, __ATOMIC_RELEASE);
#endif
}
} // namespace

void reset_state_for_test() noexcept {
    atomic_store32(&g_phase, static_cast<uint32_t>(Phase::cold));
    atomic_store_ptr(&g_module, 0);
    atomic_store32(&g_generation, 0);
    atomic_store32(&g_initialize_count, 0);
    atomic_store32(&g_shutdown_count, 0);
    atomic_store32(&g_detach_kind, static_cast<uint32_t>(DetachKind::none));
}

void on_process_attach(void* module) noexcept {
    atomic_store_ptr(&g_module, reinterpret_cast<uintptr_t>(module));
    (void)atomic_increment32(&g_generation);
    atomic_store32(&g_detach_kind, static_cast<uint32_t>(DetachKind::none));
    atomic_store32(&g_phase, static_cast<uint32_t>(Phase::loader_attached));
}

void on_process_detach(DetachKind kind) noexcept {
    atomic_store32(&g_detach_kind, static_cast<uint32_t>(kind));
    atomic_store32(&g_phase, static_cast<uint32_t>(Phase::detached));
    atomic_store_ptr(&g_module, 0);
}

InitResult try_initialize() noexcept {
    for (;;) {
        const auto phase = static_cast<Phase>(atomic_load32(&g_phase));
        if (phase == Phase::initialized) return InitResult::already_initialized;
        if (phase == Phase::shutdown) return InitResult::already_shutdown;
        if (phase == Phase::detached) return InitResult::detached;
        if (phase != Phase::loader_attached) return InitResult::not_attached;
        if (!atomic_compare_exchange32(&g_phase,
                                       static_cast<uint32_t>(Phase::loader_attached),
                                       static_cast<uint32_t>(Phase::initialized)))
            continue;
        (void)atomic_increment32(&g_initialize_count);
        return InitResult::ok;
    }
}

ShutdownResult try_shutdown() noexcept {
    for (;;) {
        const auto phase = static_cast<Phase>(atomic_load32(&g_phase));
        if (phase == Phase::shutdown) return ShutdownResult::already_shutdown;
        if (phase == Phase::detached) return ShutdownResult::detached;
        if (phase != Phase::initialized) return ShutdownResult::not_initialized;
        if (!atomic_compare_exchange32(&g_phase,
                                       static_cast<uint32_t>(Phase::initialized),
                                       static_cast<uint32_t>(Phase::shutdown)))
            continue;
        (void)atomic_increment32(&g_shutdown_count);
        return ShutdownResult::ok;
    }
}

StateSnapshot snapshot_state() noexcept {
    StateSnapshot s{};
    s.phase = static_cast<Phase>(atomic_load32(&g_phase));
    s.module_base = atomic_load_ptr(&g_module);
    s.generation = atomic_load32(&g_generation);
    s.initialize_count = atomic_load32(&g_initialize_count);
    s.shutdown_count = atomic_load32(&g_shutdown_count);
    s.detach_kind = static_cast<DetachKind>(atomic_load32(&g_detach_kind));
    return s;
}

} }
