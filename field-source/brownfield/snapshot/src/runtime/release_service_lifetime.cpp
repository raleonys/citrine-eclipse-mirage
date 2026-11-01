#if !defined(_WIN32)
#error release_service_lifetime.cpp is Windows-only.
#endif

#include "preloader_l/runtime/release_service_lifetime.hpp"
#include "preloader_l/runtime/service_storage.hpp"
#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/service_projection.hpp"
#include "preloader_l/runtime/release_server_io.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "asio_release_bridge.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/auth_code_service.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include <new>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace {
using BOOL = int;
using DWORD = unsigned long;
using LONG = long;

struct InitOnceState { void* ptr; };

extern "C" {
__declspec(dllimport) BOOL __stdcall InitOnceBeginInitialize(
    InitOnceState* once, DWORD flags, BOOL* pending, void** context);
__declspec(dllimport) BOOL __stdcall InitOnceComplete(
    InitOnceState* once, DWORD flags, void* context);
}

InitOnceState g_once{};
constexpr DWORD kInitOnceInitFailed = 0x00000004u;
// The curio +0x10 service owns the live configuration/store policy.
// ReleaseServerContext is allocated per service and retained only as its transport adapter.
preloader_l::runtime::ModelServiceStorageView g_model_storage_view{};
preloader_l::runtime::ReferenceAliasOwner g_published_owner{};
volatile LONG g_initialized = 0;
volatile LONG g_destroy_dispatches = 0;
volatile LONG g_delete_this_dispatches = 0;
volatile LONG g_scalar_delete_dispatches = 0;

// HF08/S0235: logical service destruction and physical heap retirement are
// separate ownership transitions. A failed HeapFree keeps the exact raw
// allocation reachable in an allocation-free singleton slot until a later
// lifecycle retry confirms release. New initialization is blocked while either
// physical owner remains unresolved, so the two slots cannot be overwritten.
void* g_pending_adapter_allocation = nullptr;
void* g_pending_control_allocation = nullptr;

LONG atomic_increment(volatile LONG* value) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedIncrement(value);
#else
    return __atomic_add_fetch(value, 1, __ATOMIC_ACQ_REL);
#endif
}

LONG atomic_load(volatile LONG* value) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}

void atomic_store(volatile LONG* value, LONG desired) noexcept
{
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(value, desired);
#else
    __atomic_store_n(value, desired, __ATOMIC_RELEASE);
#endif
}

struct LiveModelServiceControl558 final
    : preloader_l::runtime::ModelServiceControl558 {
    LiveModelServiceControl558() noexcept {
        // This Phase-1 patch establishes dynamic coallocation and compiler vptr
        // identity only. Exact 0xF020 service-constructor field semantics remain
        // owned by the subsequent authoritative-inline-state phase.
        service = preloader_l::runtime::model_abi::ReleaseServiceObject548{};
    }

    void destroy() noexcept override;
    void delete_this() noexcept override;
    preloader_l::runtime::ReferenceOwnerControl* scalar_delete(uint32_t flags) noexcept override;
    void* null_query(const void*) noexcept override { return nullptr; }
};

static_assert(sizeof(LiveModelServiceControl558) == 0x558,
              "live 7000 coallocation must remain exactly 0x558 bytes");

LiveModelServiceControl558* g_model_control = nullptr;

bool retry_pending_service_physical_releases() noexcept
{
    // HF13/S0489/S0518: transient response/semantic owners participate in the
    // same shutdown-readiness boundary. A failed raw release remains intrusive
    // owner state and prevents clean unload until a retry succeeds.
    bool clean = preloader_l::runtime::retry_pending_transient_releases();
    if (g_pending_adapter_allocation) {
        if (preloader_l::runtime::release_block(
                g_pending_adapter_allocation,
                sizeof(preloader_l::runtime::ReleaseServerContext),
                preloader_l::runtime::windows_allocator_platform_ops()) ==
            preloader_l::runtime::AllocationReleaseStatus::ok) {
            g_pending_adapter_allocation = nullptr;
        } else {
            clean = false;
        }
    }
    if (g_pending_control_allocation) {
        if (preloader_l::runtime::release_block(
                g_pending_control_allocation, sizeof(LiveModelServiceControl558),
                preloader_l::runtime::windows_allocator_platform_ops()) ==
            preloader_l::runtime::AllocationReleaseStatus::ok) {
            g_pending_control_allocation = nullptr;
        } else {
            clean = false;
        }
    }
    return clean;
}

bool release_or_retain_service_allocation(
    void* allocation, size_t bytes, void** pending_owner) noexcept
{
    if (!allocation) return true;
    if (!pending_owner || *pending_owner) return false;
    if (preloader_l::runtime::release_block(
            allocation, bytes, preloader_l::runtime::windows_allocator_platform_ops()) ==
        preloader_l::runtime::AllocationReleaseStatus::ok)
        return true;
    *pending_owner = allocation;
    return false;
}

void LiveModelServiceControl558::destroy() noexcept
{
    // 2.8.6: worker/thread ownership is part of the curio service object.
    // Quiesce client owners before config/store destruction, retain the +0x018
    // accept-thread owner and +0x030 backing allocation until the later physical
    // release phase, matching the curio service lifetime split.
    auto* adapter = preloader_l::runtime::model_service_adapter(&service);
    if (adapter) preloader_l::runtime::quiesce_release_worker_policy_windows(adapter);
    preloader_l::runtime::release_model_service_derived_state(&service);
    preloader_l::runtime::release_model_service_policy(&service);
    preloader_l::runtime::release_release_worker_policy_windows(&service);
    if (adapter) {
        adapter->~ReleaseServerContext();
        (void)release_or_retain_service_allocation(
            adapter, sizeof(preloader_l::runtime::ReleaseServerContext),
            &g_pending_adapter_allocation);
    }
    (void)atomic_increment(&g_destroy_dispatches);
    atomic_store(&g_initialized, 0);
}

preloader_l::runtime::ReferenceOwnerControl*
LiveModelServiceControl558::scalar_delete(uint32_t flags) noexcept
{
    (void)atomic_increment(&g_scalar_delete_dispatches);
    auto* result = static_cast<preloader_l::runtime::ReferenceOwnerControl*>(this);
    if ((flags & 1u) != 0u) {
        auto* allocation = this;
        if (g_model_control == this) g_model_control = nullptr;
        g_model_storage_view = {};
        this->~LiveModelServiceControl558();
        (void)release_or_retain_service_allocation(
            allocation, sizeof(LiveModelServiceControl558),
            &g_pending_control_allocation);
    }
    return result;
}

void LiveModelServiceControl558::delete_this() noexcept
{
    (void)atomic_increment(&g_delete_this_dispatches);
    (void)scalar_delete(1u);
}

LiveModelServiceControl558* allocate_model_control() noexcept
{
    const auto result = preloader_l::runtime::allocate_block(
        sizeof(LiveModelServiceControl558),
        preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
        preloader_l::runtime::windows_allocator_platform_ops());
    if (result.terminal != preloader_l::runtime::AllocationTerminal::none || !result.pointer)
        return nullptr;
    return ::new (result.pointer) LiveModelServiceControl558{};
}

bool complete_pending_once_failure() noexcept
{
    atomic_store(&g_initialized, 0);
    return InitOnceComplete(&g_once, kInitOnceInitFailed, nullptr) != 0;
}

preloader_l::runtime::ReleaseServiceShutdownResult shutdown_published_owner_if_idle() noexcept
{
    using preloader_l::runtime::ReleaseServiceShutdownResult;
    // Retry raw owners before inspecting logical publication. A prior shutdown
    // may already have retired the shared owner while HeapFree remained pending.
    if (!retry_pending_service_physical_releases())
        return ReleaseServiceShutdownResult::resource_cleanup_pending;
    if (!g_published_owner.control)
        return ReleaseServiceShutdownResult::not_initialized;

    const preloader_l::runtime::ReferenceOwnerSnapshot owner =
        preloader_l::runtime::reference_owner_snapshot(g_published_owner.control);
    if (owner.strong_count != 1)
        return ReleaseServiceShutdownResult::active_leases;

    // HF13 Wave4: a failed Asio listener/session close is still owned by the
    // exact ReleaseServerContext. The bootstrap worker can report
    // shutdown_failed after its first unsuccessful stop, but the permanent
    // service owner MUST NOT retire its adapter (and thus strand/io_context)
    // until the same bridge has physically quiesced and been deleted.
    // This gate is used both by bootstrap-failure cleanup and by DllShutdown.
    // On failure the published service and its adapter remain reachable for
    // a later DllShutdown retry, including native acceptance fault disarm.
    auto* service = static_cast<preloader_l::runtime::model_abi::ReleaseServiceObject548*>(
        g_published_owner.alias);
    auto* adapter = preloader_l::runtime::model_service_adapter(service);
    if (adapter && adapter->asio_bridge &&
        !preloader_l::runtime::shutdown_release_asio_windows(adapter))
        return ReleaseServiceShutdownResult::resource_cleanup_pending;

    // F321-S141-001: queue nodes whose HeapFree fails remain owned by the
    // queue subsystem. Do not retire service/config authority until every
    // detached event node has actually been released.
    if (!preloader_l::runtime::clear_queued_events())
        return ReleaseServiceShutdownResult::resource_cleanup_pending;
    if (!preloader_l::config::clear_runtime_configuration_retryable())
        return ReleaseServiceShutdownResult::resource_cleanup_pending;
    if (!preloader_l::config::clear_runtime_store_retryable())
        return ReleaseServiceShutdownResult::resource_cleanup_pending;
    preloader_l::config::clear_runtime_overrides();
    preloader_l::runtime::clear_auth_code_service();

    // F06/F07: physical retirement is a preflight while the permanent owner
    // and adapter binding are still published. Any failed close/free leaves
    // exact ownership reachable for a later DllShutdown retry.
    if (!preloader_l::runtime::prepare_release_worker_policy_retirement_windows(service) ||
        !preloader_l::runtime::release_model_service_derived_state_retryable(service) ||
        !preloader_l::runtime::release_model_service_policy_retryable(service))
        return ReleaseServiceShutdownResult::resource_cleanup_pending;

    preloader_l::config::unbind_runtime_store_model_service();
    preloader_l::config::unbind_runtime_configuration_model_service();
    preloader_l::runtime::reference_owner_release_alias(&g_published_owner);
    atomic_store(&g_initialized, 0);
    // destroy()/scalar_delete() above may have completed logical destruction
    // while retaining failed adapter/control raw allocations. Do not report a
    // clean shutdown until those exact physical owners are retired.
    if (!retry_pending_service_physical_releases())
        return ReleaseServiceShutdownResult::resource_cleanup_pending;
    return ReleaseServiceShutdownResult::shutdown;
}
}

namespace preloader_l { namespace runtime {

ReleaseServiceAcquireResult acquire_release_service_once(
    const preloader_l::net::SocketApi* api,
    ReleaseServiceLease* lease) noexcept
{
    if (!api || !lease) return ReleaseServiceAcquireResult::invalid_argument;
    // InitOnce may permit a retry after a failed initialization. Never create a
    // new singleton generation while the previous generation still owns raw
    // heap blocks whose physical release failed.
    if (!retry_pending_service_physical_releases())
        return ReleaseServiceAcquireResult::owner_not_published;
    lease->context = nullptr;
    lease->model_service = nullptr;
    lease->owner = nullptr;

    BOOL pending = 0;
    if (!InitOnceBeginInitialize(&g_once, 0, &pending, nullptr))
        return ReleaseServiceAcquireResult::once_begin_failed;

    if (pending) {
        g_model_control = allocate_model_control();
        if (!g_model_control)
            return complete_pending_once_failure()
                ? ReleaseServiceAcquireResult::owner_not_published
                : ReleaseServiceAcquireResult::once_complete_failed;

        const ModelServiceStorageStatus storage_status =
            model_service_storage_view(
                g_model_control, &g_model_storage_view);
        if (storage_status != ModelServiceStorageStatus::ok) {
            (void)g_model_control->scalar_delete(1u);
            return complete_pending_once_failure()
                ? ReleaseServiceAcquireResult::owner_not_published
                : ReleaseServiceAcquireResult::once_complete_failed;
        }

        const auto adapter_allocation = allocate_block(
            sizeof(ReleaseServerContext), AllocationRetryPolicy::outer_retry_only,
            windows_allocator_platform_ops());
        if (adapter_allocation.terminal != AllocationTerminal::none ||
            !adapter_allocation.pointer) {
            (void)g_model_control->scalar_delete(1u);
            return complete_pending_once_failure()
                ? ReleaseServiceAcquireResult::owner_not_published
                : ReleaseServiceAcquireResult::once_complete_failed;
        }

        auto* adapter = ::new (adapter_allocation.pointer) ReleaseServerContext{};
        if (!initialize_model_service_policy(
                g_model_storage_view.service, adapter) ||
            !initialize_release_worker_policy_windows(
                g_model_storage_view.service)) {
            adapter->~ReleaseServerContext();
            (void)release_or_retain_service_allocation(
                adapter, sizeof(ReleaseServerContext), &g_pending_adapter_allocation);
            (void)g_model_control->scalar_delete(1u);
            return complete_pending_once_failure()
                ? ReleaseServiceAcquireResult::owner_not_published
                : ReleaseServiceAcquireResult::once_complete_failed;
        }
        reset_release_server(adapter, api);
        adapter->model_service = g_model_storage_view.service;

        ReferenceAliasOwner local_owner{
            g_model_storage_view.service,
            g_model_storage_view.owner,
        };
        reference_owner_move_publish(&g_published_owner, &local_owner);

        atomic_store(&g_initialized, 1);
        if (!InitOnceComplete(&g_once, 0, nullptr)) {
            atomic_store(&g_initialized, 0);
            reference_owner_release_alias(&g_published_owner);
            // If success-completion itself is fault-injected or otherwise
            // rejected, make a best-effort failed completion while this thread
            // still owns the pending attempt so later callers cannot inherit a
            // permanently pending once-state.
            (void)InitOnceComplete(&g_once, kInitOnceInitFailed, nullptr);
            return ReleaseServiceAcquireResult::once_complete_failed;
        }
    }

    if (!atomic_load(&g_initialized))
        return ReleaseServiceAcquireResult::once_complete_failed;

    ReferenceAliasOwner temporary{};
    if (!reference_owner_copy_acquire(&g_published_owner, &temporary))
        return ReleaseServiceAcquireResult::owner_not_published;

    lease->model_service = temporary.alias;
    lease->context = model_service_adapter(
        static_cast<model_abi::ReleaseServiceObject548*>(temporary.alias));
    lease->owner = temporary.control;
    temporary.alias = nullptr;
    temporary.control = nullptr;
    return ReleaseServiceAcquireResult::acquired;
}

void release_release_service(ReleaseServiceLease* lease) noexcept
{
    if (!lease || !lease->owner) return;
    ReferenceAliasOwner temporary{
        lease->model_service,
        static_cast<ReferenceOwnerControl*>(lease->owner),
    };
    lease->context = nullptr;
    lease->model_service = nullptr;
    lease->owner = nullptr;
    reference_owner_release_alias(&temporary);
}

ReleaseServiceShutdownResult shutdown_release_service_after_quiescence() noexcept
{
    // External unload path: caller has already proved bootstrap-thread exit.
    return shutdown_published_owner_if_idle();
}

ReleaseServiceShutdownResult shutdown_release_service_after_bootstrap_failure() noexcept
{
    // Failure path: run_persistent_bootstrap has returned on this owner thread
    // and its temporary lease must already be released by the caller.
    return shutdown_published_owner_if_idle();
}

ReleaseServiceLifetimeSnapshot release_service_lifetime_snapshot() noexcept
{
    const ReferenceOwnerSnapshot owner = reference_owner_snapshot(
        g_published_owner.control);
    const int32_t baseline = g_published_owner.control ? 1 : 0;
    const int32_t active = owner.strong_count > baseline
        ? owner.strong_count - baseline
        : 0;
    return ReleaseServiceLifetimeSnapshot{
        static_cast<uint32_t>(atomic_load(&g_initialized) != 0),
        static_cast<uint32_t>(active),
        reinterpret_cast<uintptr_t>(model_service_adapter(
            static_cast<model_abi::ReleaseServiceObject548*>(g_published_owner.alias))),
        reinterpret_cast<uintptr_t>(g_published_owner.alias),
        reinterpret_cast<uintptr_t>(g_published_owner.control),
        owner.strong_count,
        owner.weak_count,
        static_cast<uint32_t>(atomic_load(&g_destroy_dispatches)),
        static_cast<uint32_t>(atomic_load(&g_delete_this_dispatches)),
        static_cast<uint32_t>(atomic_load(&g_scalar_delete_dispatches)),
    };
}

} }
