// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace redfield { namespace platform {
#if defined(_WIN32)

enum class MinHookInstallStatus : uint32_t {
    installed = 1,
    already_installed = 2,
    invalid_argument = 3,
    initialize_failed = 4,
    target_unavailable = 5,
    unsupported_function = 6,
    memory_allocation_failed = 7,
    memory_protect_failed = 8,
    create_failed = 9,
    enable_failed = 10,
    disable_failed = 11,
    remove_failed = 12,
    apply_failed = 13,
};

struct MinHookTicket {
    void* target;
    void* detour;
    void* original;
    uint8_t created;
    uint8_t enabled;
    uint8_t reserved[6];
};

// Project-level lifetime gate only. MinHook remains the sole owner of patching,
// instruction relocation, trampoline allocation and thread-IP relocation.
//
// The gate closes the one lifetime gap MinHook intentionally does not model:
// a thread may already be executing our detour body while a different thread
// disables/removes the hook. Every detour acquires a guard before loading its
// forwarding target. During removal the source is restored first, then the gate
// atomically republishes the direct target, drains calls that may still hold the
// trampoline, and only then permits MH_RemoveHook to free it.
struct MinHookCallGate {
    volatile long state;        // 0=direct, 1=installing, 2=active, 3=draining
    volatile long active_calls;
    void* direct_target;
    void* volatile forwarding_target;
};

class MinHookCallGuard {
public:
    explicit MinHookCallGuard(MinHookCallGate* gate) noexcept;
    ~MinHookCallGuard() noexcept;
    MinHookCallGuard(const MinHookCallGuard&) = delete;
    MinHookCallGuard& operator=(const MinHookCallGuard&) = delete;
private:
    MinHookCallGate* gate_;
};

MinHookInstallStatus create_minhook(void* target, void* detour,
                                    MinHookTicket* ticket) noexcept;

bool enable_minhook_batch(MinHookTicket* const* tickets,
                           MinHookCallGate* const* gates,
                           size_t count) noexcept;

enum class MinHookBatchTransactionStatus : uint32_t {
    committed = 1,
    rolled_back = 2,
    rollback_incomplete = 3,
    invalid_state = 4,
    quiescence_timeout = 5,
};

// Post-loader only. While staging, enable_minhook_batch() prepares gates and
// queues MinHook enable intent without mutating executable bytes. commit then
// applies the complete staged surface through exactly one MH_ApplyQueued call.
// The project-patched MinHook keeps upstream suspend/context/IP relocation but
// enumerates stable peer-thread handles with NtGetNextThread instead of a
// Toolhelp snapshot. A failed apply is followed by a single whole-surface
// disable apply; rollback_incomplete retains every trampoline conservatively.
bool begin_minhook_batch_transaction() noexcept;
MinHookBatchTransactionStatus commit_minhook_batch_transaction() noexcept;
MinHookBatchTransactionStatus rollback_minhook_batch_transaction() noexcept;
// Bounded variant used by explicit unload. A timeout occurs only after source
// bytes have been restored and direct forwarding has been published; MinHook
// ticket/trampoline ownership is retained in a draining transaction for retry.
MinHookBatchTransactionStatus rollback_minhook_batch_transaction_with_timeout(
    uint32_t timeout_ms) noexcept;
bool abort_minhook_batch_transaction() noexcept;
bool minhook_batch_transaction_active() noexcept;
bool disable_minhook_batch(MinHookTicket* const* tickets,
                            MinHookCallGate* const* gates,
                            size_t count) noexcept;
bool remove_minhook(MinHookTicket* ticket) noexcept;
bool disable_remove_minhook(MinHookTicket* ticket,
                             MinHookCallGate* gate) noexcept;

void minhook_gate_enter(MinHookCallGate* gate) noexcept;
void minhook_gate_leave(MinHookCallGate* gate) noexcept;
uintptr_t minhook_gate_forwarding_target(MinHookCallGate* gate) noexcept;

bool minhook_ticket_has_ownership(const MinHookTicket* ticket) noexcept;
uintptr_t minhook_target_address(const MinHookTicket* ticket) noexcept;
uintptr_t minhook_trampoline_address(const MinHookTicket* ticket) noexcept;

bool shutdown_minhook_runtime() noexcept;

#endif
} } // namespace redfield::platform
