// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#error minhook_runtime.cpp is Windows-only.
#endif

#include "redfield/platform/minhook_runtime.hpp"
#include <MinHook.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif
#include <windows.h>

namespace redfield { namespace platform {
namespace {
constexpr long kGateDirect = 0;
constexpr long kGateInstalling = 1;
constexpr long kGateActive = 2;
constexpr long kGateDraining = 3;
volatile long g_runtime_initialized = 0;
volatile long g_created_hooks = 0;
volatile long g_operation_lock = 0;

constexpr size_t kBatchTransactionCapacity = 16u;
enum class BatchTransactionMode : uint32_t {
    idle = 0, staging = 1, committed = 2, faulted = 3, draining = 4
};
struct BatchTransactionState {
    BatchTransactionMode mode{BatchTransactionMode::idle};
    MinHookTicket* tickets[kBatchTransactionCapacity]{};
    MinHookCallGate* gates[kBatchTransactionCapacity]{};
    size_t count{};
};
BatchTransactionState g_batch_transaction{};

long atomic_load(volatile long* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_SEQ_CST);
#endif
}
long atomic_compare_exchange(volatile long* value, long next, long expected) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, next, expected);
#else
    long observed = expected;
    (void)__atomic_compare_exchange_n(value, &observed, next, false,
                                      __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return observed;
#endif
}
long atomic_exchange(volatile long* value, long next) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedExchange(value, next);
#else
    return __atomic_exchange_n(value, next, __ATOMIC_SEQ_CST);
#endif
}
long atomic_increment(volatile long* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedIncrement(value);
#else
    return __atomic_add_fetch(value, 1L, __ATOMIC_SEQ_CST);
#endif
}
long atomic_decrement(volatile long* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedDecrement(value);
#else
    return __atomic_sub_fetch(value, 1L, __ATOMIC_SEQ_CST);
#endif
}
void* atomic_load_pointer(void* volatile* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchangePointer(value, nullptr, nullptr);
#else
    return __atomic_load_n(value, __ATOMIC_SEQ_CST);
#endif
}
void atomic_store_pointer(void* volatile* value, void* next) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchangePointer(value, next);
#else
    __atomic_store_n(value, next, __ATOMIC_SEQ_CST);
#endif
}
void wait_yield() noexcept { (void)SwitchToThread(); }

void operation_lock_enter() noexcept {
    while (atomic_compare_exchange(&g_operation_lock, 1, 0) != 0) wait_yield();
}
void operation_lock_leave() noexcept { (void)atomic_exchange(&g_operation_lock, 0); }

class OperationLock final {
public:
    OperationLock() noexcept { operation_lock_enter(); }
    ~OperationLock() noexcept { operation_lock_leave(); }
    OperationLock(const OperationLock&) = delete;
    OperationLock& operator=(const OperationLock&) = delete;
};

MinHookInstallStatus map_create_status(MH_STATUS status) noexcept {
    switch (status) {
        case MH_OK: return MinHookInstallStatus::installed;
        case MH_ERROR_ALREADY_CREATED: return MinHookInstallStatus::already_installed;
        case MH_ERROR_NOT_EXECUTABLE: return MinHookInstallStatus::target_unavailable;
        case MH_ERROR_UNSUPPORTED_FUNCTION: return MinHookInstallStatus::unsupported_function;
        case MH_ERROR_MEMORY_ALLOC: return MinHookInstallStatus::memory_allocation_failed;
        case MH_ERROR_MEMORY_PROTECT: return MinHookInstallStatus::memory_protect_failed;
        default: return MinHookInstallStatus::create_failed;
    }
}

bool ensure_runtime_initialized() noexcept {
    if (atomic_load(&g_runtime_initialized) != 0) return true;
    const MH_STATUS status = MH_Initialize();
    if (status != MH_OK && status != MH_ERROR_ALREADY_INITIALIZED) return false;
    (void)atomic_exchange(&g_runtime_initialized, 1);
    return true;
}

bool queue_enable_ticket(MinHookTicket* ticket) noexcept {
    if (!ticket || !ticket->created || !ticket->target) return false;
    const MH_STATUS status = MH_QueueEnableHook(ticket->target);
    return status == MH_OK || status == MH_ERROR_ENABLED;
}
bool queue_disable_ticket(MinHookTicket* ticket) noexcept {
    if (!ticket || !ticket->created || !ticket->target) return false;
    const MH_STATUS status = MH_QueueDisableHook(ticket->target);
    return status == MH_OK || status == MH_ERROR_DISABLED;
}
bool queue_all_transaction_hooks(bool enable) noexcept {
    // begin_minhook_batch_transaction() requires g_created_hooks == 0, so while
    // a project transaction is active every MinHook entry belongs to that
    // transaction. MinHook's all-hooks queue operation is therefore the only
    // failure-atomic way to converge queueEnable without leaving a per-ticket
    // mixed intent for a later MH_ApplyQueued().
    const MH_STATUS status = enable
        ? MH_QueueEnableHook(MH_ALL_HOOKS)
        : MH_QueueDisableHook(MH_ALL_HOOKS);
    return status == MH_OK;
}
void clear_ticket(MinHookTicket* ticket) noexcept {
    if (ticket) *ticket = MinHookTicket{};
}
void prepare_gate(MinHookCallGate* gate, const MinHookTicket* ticket) noexcept {
    if (!gate || !ticket) return;
    gate->direct_target = ticket->target;
    atomic_store_pointer(&gate->forwarding_target, ticket->original);
    (void)atomic_exchange(&gate->state, kGateInstalling);
}
void publish_gate_direct(MinHookCallGate* gate) noexcept {
    if (!gate) return;
    atomic_store_pointer(&gate->forwarding_target, gate->direct_target);
    (void)atomic_exchange(&gate->state, kGateDirect);
}
void publish_gate_active(MinHookCallGate* gate) noexcept {
    if (gate) (void)atomic_exchange(&gate->state, kGateActive);
}
void begin_gate_drain(MinHookCallGate* gate) noexcept {
    if (gate) (void)atomic_exchange(&gate->state, kGateDraining);
}
void wait_gate(MinHookCallGate* gate) noexcept {
    if (!gate) return;
    while (atomic_load(&gate->active_calls) != 0) wait_yield();
}

bool wait_transaction_gates(uint32_t timeout_ms) noexcept {
    const bool infinite = timeout_ms == 0xffffffffu;
    const unsigned long long start = infinite ? 0u : ::GetTickCount64();
    const unsigned long long deadline = infinite
        ? 0u
        : start + static_cast<unsigned long long>(timeout_ms);
    for (size_t i = 0; i < g_batch_transaction.count; ++i) {
        auto* gate = g_batch_transaction.gates[i];
        if (!gate) continue;
        while (atomic_load(&gate->active_calls) != 0) {
            if (!infinite && ::GetTickCount64() >= deadline) return false;
            wait_yield();
        }
    }
    return true;
}

bool batch_transaction_staging() noexcept {
    return g_batch_transaction.mode == BatchTransactionMode::staging;
}
void reset_batch_transaction() noexcept {
    g_batch_transaction = BatchTransactionState{};
}
bool transaction_contains(MinHookTicket* ticket) noexcept {
    for (size_t i = 0; i < g_batch_transaction.count; ++i)
        if (g_batch_transaction.tickets[i] == ticket) return true;
    return false;
}
bool validate_enable_batch(MinHookTicket* const* tickets,
                           MinHookCallGate* const* gates,
                           size_t count) noexcept {
    if (!tickets || !gates || count == 0u) return false;
    for (size_t i = 0; i < count; ++i) {
        if (!tickets[i] || !tickets[i]->created || !tickets[i]->target || !gates[i])
            return false;
        for (size_t j = 0; j < i; ++j)
            if (tickets[j] == tickets[i] || gates[j] == gates[i]) return false;
    }
    if (!batch_transaction_staging()) return true;
    if (g_batch_transaction.count + count > kBatchTransactionCapacity) return false;
    for (size_t i = 0; i < count; ++i)
        if (transaction_contains(tickets[i])) return false;
    return true;
}

bool append_transaction_batch(MinHookTicket* const* tickets,
                              MinHookCallGate* const* gates,
                              size_t count) noexcept {
    if (g_batch_transaction.count + count > kBatchTransactionCapacity) return false;
    for (size_t i = 0; i < count; ++i)
        if (transaction_contains(tickets[i])) return false;
    for (size_t i = 0; i < count; ++i) {
        g_batch_transaction.tickets[g_batch_transaction.count] = tickets[i];
        g_batch_transaction.gates[g_batch_transaction.count] = gates[i];
        ++g_batch_transaction.count;
    }
    return true;
}
void publish_transaction_direct() noexcept {
    for (size_t i = 0; i < g_batch_transaction.count; ++i) {
        g_batch_transaction.tickets[i]->enabled = 0u;
        publish_gate_direct(g_batch_transaction.gates[i]);
    }
}
void publish_transaction_active() noexcept {
    for (size_t i = 0; i < g_batch_transaction.count; ++i) {
        g_batch_transaction.tickets[i]->enabled = 1u;
        publish_gate_active(g_batch_transaction.gates[i]);
    }
}

// MH_ApplyQueued can fail after changing a prefix of the queued hooks. A failed
// rollback therefore retains MinHook ownership and conservatively marks every
// ticket enabled. This prevents callers from freeing a trampoline whose source
// may still be patched. A later uninstall retry can safely converge the batch.
bool restore_disabled_after_failed_enable(MinHookTicket* const* tickets,
                                          MinHookCallGate* const* gates,
                                          size_t count) noexcept {
    for (size_t i = 0; i < count; ++i) (void)queue_disable_ticket(tickets[i]);
    if (MH_ApplyQueued() != MH_OK) {
        for (size_t i = 0; i < count; ++i) {
            tickets[i]->enabled = 1u; // conservative ownership state
            publish_gate_active(gates[i]);
        }
        return false;
    }
    for (size_t i = 0; i < count; ++i) {
        tickets[i]->enabled = 0u;
        publish_gate_direct(gates[i]);
    }
    for (size_t i = 0; i < count; ++i) wait_gate(gates[i]);
    return true;
}
} // namespace

MinHookCallGuard::MinHookCallGuard(MinHookCallGate* gate) noexcept : gate_(gate) {
    minhook_gate_enter(gate_);
}
MinHookCallGuard::~MinHookCallGuard() noexcept { minhook_gate_leave(gate_); }

void minhook_gate_enter(MinHookCallGate* gate) noexcept {
    if (!gate) return;
    for (;;) {
        const long before = atomic_load(&gate->state);
        if (before == kGateInstalling) {
            wait_yield();
            continue;
        }
        (void)atomic_increment(&gate->active_calls);
        const long after = atomic_load(&gate->state);
        if (after == before && after != kGateInstalling) return;
        (void)atomic_decrement(&gate->active_calls);
    }
}
void minhook_gate_leave(MinHookCallGate* gate) noexcept {
    if (gate) (void)atomic_decrement(&gate->active_calls);
}
uintptr_t minhook_gate_forwarding_target(MinHookCallGate* gate) noexcept {
    return gate ? reinterpret_cast<uintptr_t>(atomic_load_pointer(&gate->forwarding_target)) : 0u;
}

MinHookInstallStatus create_minhook(void* target, void* detour,
                                    MinHookTicket* ticket) noexcept {
    if (!target || !detour || !ticket) return MinHookInstallStatus::invalid_argument;
    OperationLock operation_lock;
    if (ticket->created) return MinHookInstallStatus::already_installed;
    if (!ensure_runtime_initialized()) return MinHookInstallStatus::initialize_failed;

    void* original = nullptr;
    const MH_STATUS status = MH_CreateHook(target, detour, &original);
    const MinHookInstallStatus mapped = map_create_status(status);
    if (status != MH_OK) return mapped;

    ticket->target = target;
    ticket->detour = detour;
    ticket->original = original;
    ticket->created = 1u;
    ticket->enabled = 0u;
    (void)atomic_increment(&g_created_hooks);
    return MinHookInstallStatus::installed;
}

bool enable_minhook_batch(MinHookTicket* const* tickets,
                           MinHookCallGate* const* gates,
                           size_t count) noexcept {
    if (!tickets || !gates || count == 0u) return false;
    OperationLock operation_lock;
    // F323-S0191-001: reject the complete parameter/ownership surface before
    // any gate transitions to installing. Operational queue/apply failures below
    // retain their explicit rollback/ownership semantics.
    if (!validate_enable_batch(tickets, gates, count)) return false;
    for (size_t i = 0; i < count; ++i) prepare_gate(gates[i], tickets[i]);
    for (size_t i = 0; i < count; ++i) {
        if (!queue_enable_ticket(tickets[i])) {
            // No ApplyQueued occurred yet, so source bytes are still original.
            // During the startup transaction this project owns every MinHook
            // entry; converge the complete queue state in one MinHook operation
            // instead of leaving a partially rewritten queueEnable vector.
            if (batch_transaction_staging())
                (void)queue_all_transaction_hooks(false);
            else
                for (size_t j = 0; j < i; ++j) (void)queue_disable_ticket(tickets[j]);
            for (size_t j = 0; j < count; ++j) publish_gate_direct(gates[j]);
            return false;
        }
    }

    if (batch_transaction_staging()) {
        // Stage the subsystem into one process-wide post-loader transaction.
        // Trampolines/originals are already available, but executable bytes are
        // not changed until commit_minhook_batch_transaction().
        if (!append_transaction_batch(tickets, gates, count)) {
            (void)queue_all_transaction_hooks(false);
            for (size_t i = 0; i < count; ++i) publish_gate_direct(gates[i]);
            return false;
        }
        return true;
    }

    // Non-transactional callers still use the safe-freeze MinHook backend.
    // A failed apply may have patched a prefix, so retain ownership unless a
    // whole-batch disable apply proves restoration.
    for (size_t i = 0; i < count; ++i) tickets[i]->enabled = 1u;
    const MH_STATUS apply_status = MH_ApplyQueued();
    if (apply_status != MH_OK) {
        (void)restore_disabled_after_failed_enable(tickets, gates, count);
        return false;
    }
    for (size_t i = 0; i < count; ++i) publish_gate_active(gates[i]);
    return true;
}

bool begin_minhook_batch_transaction() noexcept {
    OperationLock operation_lock;
    if (g_batch_transaction.mode != BatchTransactionMode::idle) return false;
    if (atomic_load(&g_created_hooks) != 0) return false;
    g_batch_transaction.mode = BatchTransactionMode::staging;
    return true;
}

MinHookBatchTransactionStatus commit_minhook_batch_transaction() noexcept {
    OperationLock operation_lock;
    if (g_batch_transaction.mode != BatchTransactionMode::staging ||
        g_batch_transaction.count == 0u)
        return MinHookBatchTransactionStatus::invalid_state;

    // Conservatively retain trampoline ownership before the first executable
    // mutation. Safe-freeze performs one peer-thread enumeration/suspend/IP
    // relocation pass for the complete 9-hook surface.
    for (size_t i = 0; i < g_batch_transaction.count; ++i)
        g_batch_transaction.tickets[i]->enabled = 1u;

    if (MH_ApplyQueued() == MH_OK) {
        publish_transaction_active();
        g_batch_transaction.mode = BatchTransactionMode::committed;
        return MinHookBatchTransactionStatus::committed;
    }

    // Initial apply may have changed a prefix. Request the original state for
    // the entire staged surface and prove rollback with one safe-freeze apply.
    const bool queue_ok = queue_all_transaction_hooks(false);
    if (queue_ok && MH_ApplyQueued() == MH_OK) {
        publish_transaction_direct();
        for (size_t i = 0; i < g_batch_transaction.count; ++i)
            wait_gate(g_batch_transaction.gates[i]);
        reset_batch_transaction();
        return MinHookBatchTransactionStatus::rolled_back;
    }

    // Executable state can no longer be proven. Keep all trampolines and active
    // forwarding authority; DllShutdown may retry the whole-surface disable.
    publish_transaction_active();
    g_batch_transaction.mode = BatchTransactionMode::faulted;
    return MinHookBatchTransactionStatus::rollback_incomplete;
}

MinHookBatchTransactionStatus rollback_minhook_batch_transaction_with_timeout(
    uint32_t timeout_ms) noexcept {
    OperationLock operation_lock;

    // F04: a previous bounded rollback may already have restored source bytes
    // and published direct forwarding while one admitted detour was still
    // running. In that state do not queue/apply another mutation; only finish
    // the drain, then retire the transaction.
    if (g_batch_transaction.mode == BatchTransactionMode::draining) {
        if (!wait_transaction_gates(timeout_ms))
            return MinHookBatchTransactionStatus::quiescence_timeout;
        reset_batch_transaction();
        return MinHookBatchTransactionStatus::rolled_back;
    }

    if (g_batch_transaction.mode != BatchTransactionMode::committed &&
        g_batch_transaction.mode != BatchTransactionMode::faulted)
        return MinHookBatchTransactionStatus::invalid_state;

    for (size_t i = 0; i < g_batch_transaction.count; ++i)
        begin_gate_drain(g_batch_transaction.gates[i]);

    if (!queue_all_transaction_hooks(false)) {
        publish_transaction_active();
        g_batch_transaction.mode = BatchTransactionMode::faulted;
        return MinHookBatchTransactionStatus::rollback_incomplete;
    }

    if (MH_ApplyQueued() == MH_OK) {
        publish_transaction_direct();
        g_batch_transaction.mode = BatchTransactionMode::draining;
        if (!wait_transaction_gates(timeout_ms))
            return MinHookBatchTransactionStatus::quiescence_timeout;
        reset_batch_transaction();
        return MinHookBatchTransactionStatus::rolled_back;
    }

    // A failed disable can leave a restored prefix. Try to converge back to the
    // known committed state once; if that succeeds, retain all ownership for a
    // later shutdown retry. If it also fails, ownership remains fail-closed.
    const bool requeue_ok = queue_all_transaction_hooks(true);
    const bool recommit_ok = requeue_ok && MH_ApplyQueued() == MH_OK;
    publish_transaction_active();
    g_batch_transaction.mode = recommit_ok
        ? BatchTransactionMode::committed
        : BatchTransactionMode::faulted;
    return MinHookBatchTransactionStatus::rollback_incomplete;
}

MinHookBatchTransactionStatus rollback_minhook_batch_transaction() noexcept {
    return rollback_minhook_batch_transaction_with_timeout(0xffffffffu);
}

bool abort_minhook_batch_transaction() noexcept {
    OperationLock operation_lock;
    if (g_batch_transaction.mode != BatchTransactionMode::staging) return false;
    if (!queue_all_transaction_hooks(false)) {
        // Source bytes were never applied, but MinHook queue state cannot be
        // proven clean. Retain all trampoline ownership in a faulted transaction
        // so teardown can retry a whole-surface disable instead of resetting
        // project state while a stale queued-enable may still exist.
        for (size_t i = 0; i < g_batch_transaction.count; ++i)
            g_batch_transaction.tickets[i]->enabled = 1u;
        publish_transaction_active();
        g_batch_transaction.mode = BatchTransactionMode::faulted;
        return false;
    }
    for (size_t i = 0; i < g_batch_transaction.count; ++i) {
        g_batch_transaction.tickets[i]->enabled = 0u;
        publish_gate_direct(g_batch_transaction.gates[i]);
    }
    reset_batch_transaction();
    return true;
}

bool minhook_batch_transaction_active() noexcept {
    OperationLock operation_lock;
    return g_batch_transaction.mode != BatchTransactionMode::idle;
}

bool disable_minhook_batch(MinHookTicket* const* tickets,
                            MinHookCallGate* const* gates,
                            size_t count) noexcept {
    if (!tickets || !gates || count == 0u) return false;
    OperationLock operation_lock;
    for (size_t i = 0; i < count; ++i) {
        if (!tickets[i] || !tickets[i]->created || !gates[i]) return false;
        begin_gate_drain(gates[i]);
    }
    for (size_t i = 0; i < count; ++i) {
        if (!queue_disable_ticket(tickets[i])) {
            // Nothing has been applied yet. Restore the global queued intent
            // for entries already touched by this batch before releasing the
            // project transaction lock.
            for (size_t j = 0; j < i; ++j) (void)queue_enable_ticket(tickets[j]);
            for (size_t j = 0; j < count; ++j) publish_gate_active(gates[j]);
            return false;
        }
    }
    const MH_STATUS apply_status = MH_ApplyQueued();
    if (apply_status != MH_OK) {
        // A prefix may have been restored. Keeping the trampoline published is
        // safe for both states; conservatively retain ownership for a retry.
        for (size_t i = 0; i < count; ++i) {
            tickets[i]->enabled = 1u;
            publish_gate_active(gates[i]);
        }
        return false;
    }

    // Source bytes are restored while MinHook still owns every trampoline.
    // Atomically switch forwarding to the direct target before marking DIRECT.
    // Stale detour entrants that have not acquired their guard yet can no longer
    // acquire a trampoline pointer, while calls that already did are drained.
    for (size_t i = 0; i < count; ++i) {
        tickets[i]->enabled = 0u;
        publish_gate_direct(gates[i]);
    }
    for (size_t i = 0; i < count; ++i) wait_gate(gates[i]);
    return true;
}

bool remove_minhook(MinHookTicket* ticket) noexcept {
    if (!ticket || !ticket->created) return true;
    OperationLock operation_lock;
    if (ticket->enabled) return false;
    const MH_STATUS status = MH_RemoveHook(ticket->target);
    if (status != MH_OK && status != MH_ERROR_NOT_CREATED) return false;
    clear_ticket(ticket);
    (void)atomic_decrement(&g_created_hooks);
    return true;
}

bool disable_remove_minhook(MinHookTicket* ticket,
                             MinHookCallGate* gate) noexcept {
    if (!ticket || !ticket->created) return true;
    MinHookTicket* tickets[] = {ticket};
    MinHookCallGate* gates[] = {gate};
    if (ticket->enabled && !disable_minhook_batch(tickets, gates, 1u)) return false;
    return remove_minhook(ticket);
}

bool minhook_ticket_has_ownership(const MinHookTicket* ticket) noexcept {
    return ticket && ticket->created != 0u;
}
uintptr_t minhook_target_address(const MinHookTicket* ticket) noexcept {
    return ticket ? reinterpret_cast<uintptr_t>(ticket->target) : 0u;
}
uintptr_t minhook_trampoline_address(const MinHookTicket* ticket) noexcept {
    return ticket ? reinterpret_cast<uintptr_t>(ticket->original) : 0u;
}

bool shutdown_minhook_runtime() noexcept {
    OperationLock operation_lock;
    if (atomic_load(&g_created_hooks) != 0) return false;
    if (atomic_load(&g_runtime_initialized) == 0) return true;
    const MH_STATUS status = MH_Uninitialize();
    if (status != MH_OK && status != MH_ERROR_NOT_INITIALIZED) return false;
    (void)atomic_exchange(&g_runtime_initialized, 0);
    return true;
}

} } // namespace redfield::platform
