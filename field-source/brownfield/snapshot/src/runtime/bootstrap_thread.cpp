#if !defined(_WIN32)
#error bootstrap_thread.cpp is Windows-only.
#endif

#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/runtime/bootstrap_thread.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/release_server_io.hpp"
#include "preloader_l/runtime/release_service_lifetime.hpp"
#include "preloader_l/net/winsock_native.hpp"
#include "preloader_l/platform/crypt_observer.hpp"
#include "preloader_l/config/process_configuration.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include "preloader_l/platform/process_environment.hpp"
#include "preloader_l/platform/registry_hook_runtime.hpp"
#include "preloader_l/platform/connect_redirect.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include "preloader_l/platform/addon_hooks.hpp"
#include "preloader_l/platform/core_hooks.hpp"
#include "preloader_l/platform/addon_selection.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"
#include "preloader_l/runtime/ptd_fls.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#if PRELOADER_L_TRACE_DIAGNOSTICS
#include "preloader_l/platform/optional_host_compat.hpp"
#include "preloader_l/platform/identity_state.hpp"
#endif
#include "preloader_l/runtime/runtime_status.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace {
using BOOL = int;
using DWORD = unsigned long;
using HANDLE = void*;
using HMODULE = void*;
using ThreadStart = DWORD (__stdcall *)(void*);

extern "C" {
__declspec(dllimport) BOOL __stdcall DisableThreadLibraryCalls(HMODULE module);
__declspec(dllimport) HANDLE __stdcall CreateThread(void* attributes, size_t stack_size,
                                                     ThreadStart start, void* parameter,
                                                     DWORD flags, DWORD* thread_id);
__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE object);
__declspec(dllimport) DWORD __stdcall WaitForSingleObject(HANDLE object, DWORD milliseconds);
__declspec(dllimport) BOOL __stdcall GetModuleHandleExW(DWORD flags, const wchar_t* name_or_address, HMODULE* module);
__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE module);
__declspec(dllimport) void __stdcall FreeLibraryAndExitThread(HMODULE module, DWORD exit_code);
__declspec(dllimport) void __stdcall Sleep(DWORD milliseconds);
__declspec(dllimport) unsigned long long __stdcall GetTickCount64();
}

constexpr DWORD kGetModuleHandleExFlagFromAddress = 0x00000004u;
constexpr DWORD kWaitObject0 = 0x00000000u;
constexpr DWORD kWaitTimeout = 0x00000102u;
constexpr DWORD kInfinite = 0xffffffffu;

struct ReleaseBootstrapThreadState {
    HANDLE handle;
    HMODULE pinned_module;
    DWORD thread_id;
    volatile long running;
    volatile long completed;
};
ReleaseBootstrapThreadState g_bootstrap_thread{};
volatile long g_runtime_shutdown_state = 0; // 0=idle/retryable, 1=in progress, 2=complete
volatile long g_environment_restore_mask = 0;

struct ShutdownDeadline {
    bool infinite;
    uint64_t deadline_ms;
};
ShutdownDeadline make_shutdown_deadline(uint32_t timeout_ms) noexcept {
    if (timeout_ms == 0xffffffffu) return ShutdownDeadline{true, 0u};
    const uint64_t now = static_cast<uint64_t>(GetTickCount64());
    const uint64_t add = static_cast<uint64_t>(timeout_ms);
    const uint64_t max = ~uint64_t{0};
    return ShutdownDeadline{false, add > max - now ? max : now + add};
}
uint32_t remaining_shutdown_timeout(const ShutdownDeadline& deadline) noexcept {
    if (deadline.infinite) return 0xffffffffu;
    const uint64_t now = static_cast<uint64_t>(GetTickCount64());
    if (now >= deadline.deadline_ms) return 0u;
    const uint64_t remaining = deadline.deadline_ms - now;
    return remaining >= 0xffffffffu ? 0xfffffffeu : static_cast<uint32_t>(remaining);
}

enum class HookTeardownResult : uint8_t { ok = 0, failed = 1, timeout = 2 };
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
volatile long g_native_acceptance_ready = 0;
#endif

long atomic_read(volatile long* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}
void atomic_write(volatile long* value, long desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(value, desired);
#else
    __atomic_store_n(value, desired, __ATOMIC_RELEASE);
#endif
}
bool atomic_claim(volatile long* value, long expected, long desired) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, desired, expected) == expected;
#else
    return __atomic_compare_exchange_n(
        value, &expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
#endif
}

bool on_listener_ready(uint16_t port) noexcept
{
    // The destination-aware connect detour is part of the mandatory seven-hook
    // startup transaction. Listener readiness publishes the selected local port;
    // no second MinHook transaction is permitted here.
    preloader_l::platform::set_connect_selected_port(port);
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::connect_selected_port, port);

#if PRELOADER_L_TRACE_DIAGNOSTICS
    // Debug-only research/compatibility probe. The curio 44-byte signature
    // scanner is intentionally absent from the normalized production target; it
    // remains available here for explicit diagnostic builds without promoting the
    // unresolved 0x10150/0x1019D installer/callback remainder into shipping code.
    const auto compat = preloader_l::platform::scan_main_module_optional_host_compat();
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::optional_host_compat_scan_status,
        static_cast<uint32_t>(compat.status));
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::optional_host_compat_hits, compat.hit_count);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::optional_host_compat_first_rva, compat.first_rva);
#endif
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // This is the explicit acceptance-ready publication point: the native
    // listener is bound, the mandatory destination-aware redirect is installed, and the service
    // owner/adapter was already published before the listener was started.
    atomic_write(&g_native_acceptance_ready, 1);
#endif
    return true;
}

void retain_bootstrap_environment(uint32_t written_mask) noexcept
{
    atomic_write(&g_environment_restore_mask, static_cast<long>(written_mask));
}

bool restore_bootstrap_environment() noexcept
{
    const uint32_t written_mask = static_cast<uint32_t>(atomic_read(&g_environment_restore_mask));
    if (!written_mask) return true;
    uint32_t restored_mask = 0;
    const auto status = preloader_l::platform::restore_process_environment(
        written_mask, &restored_mask);
    // F321-S077-001: successful sub-restores retire only their own bits. Any
    // failed removal remains lifecycle-owned for DllShutdown retry.
    const uint32_t remaining = written_mask & ~restored_mask;
    atomic_write(&g_environment_restore_mask, static_cast<long>(remaining));
    return status == preloader_l::platform::EnvironmentStatus::ok && remaining == 0;
}

bool retry_bootstrap_pin_release() noexcept
{
    HMODULE pinned = g_bootstrap_thread.pinned_module;
    if (!pinned) return true;
    if (!FreeLibrary(pinned)) return false;
    if (g_bootstrap_thread.pinned_module == pinned)
        g_bootstrap_thread.pinned_module = nullptr;
    return true;
}

HookTeardownResult teardown_bootstrap_failure_hooks(
    uint32_t timeout_ms = 0xffffffffu) noexcept
{
    // F01: preflight the registry owner while interception is still active. If a
    // pseudo handle remains, make no executable/policy mutation so the host can
    // close it through the replacement RegCloseKey and retry shutdown. A
    // successful preflight fences future pseudo allocation before hook rollback.
    if (preloader_l::platform::windows_registry_hook_mode() !=
            preloader_l::platform::WindowsRegistryHookMode::none &&
        !preloader_l::platform::prepare_windows_registry_teardown())
        return HookTeardownResult::failed;

    // If the 7/9/9/11 transaction is still committed/faulted,
    // restore the complete executable surface through one safe-freeze apply
    // before any subsystem releases its trampoline ownership. F04 bounds the
    // post-restore active-call drain when explicit unload supplies a deadline.
    if (preloader_l::platform::minhook_batch_transaction_active()) {
        const auto transaction =
            preloader_l::platform::rollback_minhook_batch_transaction_with_timeout(
                timeout_ms);
        if (transaction ==
            preloader_l::platform::MinHookBatchTransactionStatus::quiescence_timeout)
            return HookTeardownResult::timeout;
        if (transaction !=
            preloader_l::platform::MinHookBatchTransactionStatus::rolled_back)
            return HookTeardownResult::failed;
    }

    // Keep backing service/config policy alive until every host-facing hook has
    // been removed; retained hook ownership makes cleanup retryable/fail-closed.
    bool hooks_ok = true;
    hooks_ok = preloader_l::platform::uninstall_addon_hooks() && hooks_ok;
    hooks_ok = preloader_l::platform::uninstall_crypt_query_observer() && hooks_ok;
    hooks_ok = preloader_l::platform::uninstall_connect_redirect() && hooks_ok;
    hooks_ok = preloader_l::platform::uninstall_windows_registry_iat_hooks() && hooks_ok;
    hooks_ok = preloader_l::platform::shutdown_minhook_runtime() && hooks_ok;
    if (hooks_ok) preloader_l::config::unfreeze_runtime_configuration_after_hooks();
    return hooks_ok ? HookTeardownResult::ok : HookTeardownResult::failed;
}

bool finalize_pre_acquire_bootstrap_failure() noexcept
{
    // HF18: PTD is live before the release-service lease; hooks are not.
    // A failed acquire unwinds pre-acquire PTD/state with no installed patches.
    if (teardown_bootstrap_failure_hooks() != HookTeardownResult::ok) return false;

    // acquire_release_service_once() normally cleans a failed pending owner
    // internally. Accept not_initialized as the expected pre-acquire case,
    // but also close a residual published owner if a failure occurred after
    // publication and before the temporary lease became usable.
    const auto service =
        preloader_l::runtime::shutdown_release_service_after_bootstrap_failure();
    if (service != preloader_l::runtime::ReleaseServiceShutdownResult::shutdown &&
        service != preloader_l::runtime::ReleaseServiceShutdownResult::not_initialized)
        return false;

    const bool detached = preloader_l::runtime::detach_current_thread_ptd();
    preloader_l::runtime::shutdown_ptd_runtime();
    return detached && !preloader_l::runtime::ptd_cleanup_pending();
}

bool finalize_post_acquire_bootstrap_failure(
    preloader_l::runtime::ReleaseServiceLease* service) noexcept
{
    // The temporary lease must be released before the permanent owner can be
    // destroyed. Hooks are removed first so no replacement can retain pointers
    // into config/store/service policy after owner release.
    preloader_l::runtime::release_release_service(service);
    if (teardown_bootstrap_failure_hooks() != HookTeardownResult::ok) return false;
    if (preloader_l::runtime::shutdown_release_service_after_bootstrap_failure() !=
        preloader_l::runtime::ReleaseServiceShutdownResult::shutdown)
        return false;

    // F1544-001 / HOTFIX01: post-acquire failure is a complete runtime
    // transaction. Once hooks and service policy are quiescent, retire the
    // bootstrap worker's PTD and the process-wide FLS runtime before reporting
    // successful unwind. A failed FlsFree remains fail-closed/retryable.
    const bool detached = preloader_l::runtime::detach_current_thread_ptd();
    preloader_l::runtime::shutdown_ptd_runtime();
    return detached && !preloader_l::runtime::ptd_cleanup_pending();
}

enum class PostLoaderHookInstallResult : uint32_t {
    installed = 1,
    failed_rolled_back = 2,
    failed_ownership_retained = 3,
};

PostLoaderHookInstallResult install_post_loader_minhook_surface(
    preloader_l::platform::AddonHookSelection selection) noexcept
{
    using namespace preloader_l::platform;
    if ((selection.mask & ~3u) != 0u)
        return PostLoaderHookInstallResult::failed_rolled_back;
    if (!begin_minhook_batch_transaction())
        return PostLoaderHookInstallResult::failed_ownership_retained;

    const auto abort_staging = []() noexcept {
        return abort_minhook_batch_transaction()
            ? PostLoaderHookInstallResult::failed_rolled_back
            : PostLoaderHookInstallResult::failed_ownership_retained;
    };
    if (!stage_core_hooks()) return abort_staging();
    if (!stage_addon_hooks(selection.mask)) return abort_staging();

    const auto transaction = commit_minhook_batch_transaction();
    if (transaction == MinHookBatchTransactionStatus::committed)
        return PostLoaderHookInstallResult::installed;
    if (transaction == MinHookBatchTransactionStatus::rolled_back)
        return PostLoaderHookInstallResult::failed_rolled_back;
    return PostLoaderHookInstallResult::failed_ownership_retained;
}

bool complete_outer_bootstrap_cancellation() noexcept
{
    if (!preloader_l::runtime::mark_bootstrap_cancelled_before_run()) return false;
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::bootstrap_complete, 0);
    return true;
}

DWORD cancelled_outer_bootstrap_result(bool unwind_ok) noexcept
{
    if (unwind_ok && complete_outer_bootstrap_cancellation())
        return static_cast<DWORD>(preloader_l::runtime::BootstrapRunResult::ok);
    return static_cast<DWORD>(preloader_l::runtime::BootstrapRunResult::shutdown_failed);
}

DWORD run_release_bootstrap_core() noexcept
{
    // SAFE-FREEZE contract: DllMain only schedules this worker.
    // The selected core+addon detours are created/queued and published by a
    // single post-loader MH_ApplyQueued. The pinned MinHook patch replaces
    // Toolhelp snapshot enumeration with NtGetNextThread while retaining peer
    // suspend/context/IP relocation and ordinary MinHook patch semantics.
    // F323-S0166-001: stop published after scheduling but before worker start
    // must win before any hook/PTD/service/environment authority is acquired.
    if (preloader_l::runtime::bootstrap_stop_requested())
        return cancelled_outer_bootstrap_result(true);

    // HF18 configuration-first: PTD prerequisites must precede service/config
    // acquisition, which precedes the sole MinHook transaction.
    if (!preloader_l::runtime::initialize_ptd_runtime_windows()) {
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::failure, 0x5405u);
        const bool cleaned = finalize_pre_acquire_bootstrap_failure();
        return static_cast<DWORD>(cleaned
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed);
    }
    if (preloader_l::runtime::bootstrap_stop_requested())
        return cancelled_outer_bootstrap_result(finalize_pre_acquire_bootstrap_failure());

    PRELOADER_L_WINDOWS_IDENTITY();
#if PRELOADER_L_TRACE_DIAGNOSTICS
    preloader_l::platform::initialize_reconstructed_identity_state();
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::reconstructed_identity_ready, 1u);
    PRELOADER_L_WINDOWS_RECONSTRUCTED_IDENTITY();
#endif

    // 2.9.6 intentionally does not reproduce the historical WinMM proxy
    // resolver table. This DLL does not expose the legacy WinMM proxy ABI and
    // no production consumer of those resolved pointers is present in the
    // normalized source. The original proxy behavior remains research evidence,
    // not an active shipping dependency.
#if PRELOADER_L_TRACE_DIAGNOSTICS
    const auto active = preloader_l::runtime::active_runtime_snapshot();
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::active_unresolved_mask, active.unresolved_mask);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::active_curio_mask, active.curio_mask);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::active_integrated_mask, active.integrated_mask);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::active_runtime_active_mask, active.runtime_active_mask);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::active_consistency_open_mask, active.consistency_open_mask);
#endif

    preloader_l::runtime::ReleaseServiceLease service{};
    const auto service_status = preloader_l::runtime::acquire_release_service_once(
        preloader_l::net::native_winsock_api(), &service);
    if (service_status != preloader_l::runtime::ReleaseServiceAcquireResult::acquired ||
        !service.context || !service.model_service) {
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::failure,
            0x7000u + static_cast<uint32_t>(service_status));
        const bool unwind_ok = finalize_pre_acquire_bootstrap_failure();
        const auto failure = unwind_ok
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed;
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::failure, static_cast<uint32_t>(failure));
        return static_cast<DWORD>(failure);
    }
    auto* model_service = static_cast<
        preloader_l::runtime::model_abi::ReleaseServiceObject548*>(service.model_service);
    preloader_l::config::bind_runtime_configuration_model_service(model_service);
    preloader_l::config::bind_runtime_store_model_service(model_service);

    if (preloader_l::runtime::bootstrap_stop_requested())
        return cancelled_outer_bootstrap_result(
            finalize_post_acquire_bootstrap_failure(&service));

    const auto configuration = preloader_l::config::load_process_runtime_configuration();
#if !PRELOADER_L_TRACE_DIAGNOSTICS
    (void)configuration;
#endif

    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::runtime_configuration_directory,
        configuration.current_directory_available ? configuration.current_directory_length : 0u);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::runtime_configuration_status,
        static_cast<uint32_t>(configuration.file_status));
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::runtime_configuration_sections,
        configuration.present_sections);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::store_parse_status,
        static_cast<uint32_t>(
            preloader_l::config::runtime_store_status()));
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::store_present,
        preloader_l::config::runtime_store_present() ? 1u : 0u);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::store_entitlement_count,
        static_cast<uint32_t>(
            preloader_l::config::runtime_store_entitlement_count()));
    // Config has been fully committed to the release model before selecting
    // which addon patches exist. A missing file retains legacy mask=0.
    if (configuration.file_status !=
            preloader_l::config::RuntimeConfigurationFileStatus::ok &&
        configuration.file_status !=
            preloader_l::config::RuntimeConfigurationFileStatus::not_found) {
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::failure, 0x54B0u);
        const bool cleaned = finalize_post_acquire_bootstrap_failure(&service);
        return static_cast<DWORD>(cleaned
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed);
    }
    // Freeze before reading the mask, so no competing Config2 publication can
    // change the selected addon surface between snapshot and hook commit.
    if (!preloader_l::config::freeze_runtime_configuration_for_hooks()) {
        const bool cleaned = finalize_post_acquire_bootstrap_failure(&service);
        return static_cast<DWORD>(cleaned
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed);
    }
    preloader_l::platform::AddonHookSelection selection{};
    if (!preloader_l::platform::read_committed_addon_selection(&selection)) {
        const bool cleaned = finalize_post_acquire_bootstrap_failure(&service);
        return static_cast<DWORD>(cleaned
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed);
    }
    if (preloader_l::runtime::bootstrap_stop_requested())
        return cancelled_outer_bootstrap_result(
            finalize_post_acquire_bootstrap_failure(&service));
    const auto hook_install = install_post_loader_minhook_surface(selection);
    if (hook_install != PostLoaderHookInstallResult::installed) {
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::failure, 0x54A0u);
        if (hook_install == PostLoaderHookInstallResult::failed_ownership_retained) {
            // A failed MinHook rollback retains executable hooks and the
            // permanent service/config owner for DllShutdown retry. Drop only
            // the worker's temporary strong lease before leaving this thread.
            preloader_l::runtime::release_release_service(&service);
            return static_cast<DWORD>(
                preloader_l::runtime::BootstrapRunResult::shutdown_failed);
        }
        const bool cleaned = finalize_post_acquire_bootstrap_failure(&service);
        return static_cast<DWORD>(cleaned
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed);
    }
    if (preloader_l::runtime::bootstrap_stop_requested())
        return cancelled_outer_bootstrap_result(
            finalize_post_acquire_bootstrap_failure(&service));

    uint32_t environment_written = 0;
    uint32_t environment_preserved = 0;
    const auto environment_status = preloader_l::platform::apply_process_environment(
        &environment_written, &environment_preserved);
    retain_bootstrap_environment(environment_written);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::environment_policy_status,
        static_cast<uint32_t>(environment_status));
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::environment_written_mask, environment_written);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::environment_preserved_mask, environment_preserved);
    if (environment_status != preloader_l::platform::EnvironmentStatus::ok) {
        const bool environment_ok = restore_bootstrap_environment();
        const bool unwind_ok = finalize_post_acquire_bootstrap_failure(&service);
        const auto failure = environment_ok && unwind_ok
            ? preloader_l::runtime::BootstrapRunResult::initialize_failed
            : preloader_l::runtime::BootstrapRunResult::shutdown_failed;
        PRELOADER_L_WINDOWS_TRACE(
            preloader_l::runtime::TraceEvent::failure, static_cast<uint32_t>(failure));
        return static_cast<DWORD>(failure);
    }
    if (preloader_l::runtime::bootstrap_stop_requested()) {
        const bool environment_ok = restore_bootstrap_environment();
        const bool unwind_ok = finalize_post_acquire_bootstrap_failure(&service);
        return cancelled_outer_bootstrap_result(environment_ok && unwind_ok);
    }

    const bool registry_hooks = preloader_l::platform::install_windows_registry_iat_hooks();
#if !PRELOADER_L_TRACE_DIAGNOSTICS
    (void)registry_hooks;
#endif
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::registry_iat_hook_status, registry_hooks ? 1u : 0u);
    // CryptQueryObject is already covered process-wide by the mandatory startup
    // detour; no legacy loaded-module observer sweep remains in the current runtime.
    // Active-parity policy: do not auto-sweep loaded modules for diagnostic
    // CryptMsgGetParam / CertFindCertificateInStore IAT observers.
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::worker_started, 0);

#if PRELOADER_L_TRACE_DIAGNOSTICS
    service.context->trace = &preloader_l::runtime::windows_debug_trace;
#endif
    service.context->listener_ready = &on_listener_ready;
    auto result = preloader_l::runtime::run_persistent_bootstrap(
        preloader_l::runtime::make_release_server_backend(service.context));

    const bool environment_ok = restore_bootstrap_environment();
    if (result == preloader_l::runtime::BootstrapRunResult::ok && environment_ok) {
        // Normal persistent-service stop is owned by DllShutdown: drop only the
        // bootstrap thread's temporary lease and leave permanent policy alive
        // for the outside-loader-lock teardown sequence.
        preloader_l::runtime::release_release_service(&service);
        PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::bootstrap_complete, 0);
        return static_cast<DWORD>(result);
    }

    // Any terminal bootstrap failure is transactional: the temporary lease is
    // released, host-facing hooks are removed while backing policy is still
    // valid, then the permanent service owner/config/store policy is closed.
    // A failed unwind is reported as shutdown_failed and deliberately retains
    // backing policy for a later DllShutdown retry.
    const bool unwind_ok = finalize_post_acquire_bootstrap_failure(&service);
    if (!environment_ok || !unwind_ok)
        result = preloader_l::runtime::BootstrapRunResult::shutdown_failed;

    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::failure, static_cast<uint32_t>(result));
    return static_cast<DWORD>(result);
}

DWORD __stdcall release_bootstrap_thread(void* opaque) noexcept
{
    auto* pinned_module = static_cast<HMODULE>(opaque);
    atomic_write(&g_bootstrap_thread.running, 1);
    const DWORD result = run_release_bootstrap_core();
    if (result != static_cast<DWORD>(preloader_l::runtime::BootstrapRunResult::ok)) {
        // F323-S0160-001: inner runner paths may already have a more specific
        // failure. Publish only previously-unreported outer startup failures.
        (void)preloader_l::runtime::mark_bootstrap_failed_if_unreported(result);
    }
    atomic_write(&g_bootstrap_thread.running, 0);
    atomic_write(&g_bootstrap_thread.completed, 1);

    // The thread HANDLE remains owned by the outside-loader-lock lifecycle
    // owner. It must stay open until WaitForSingleObject proves that no
    // instruction from this worker can still execute.
    g_bootstrap_thread.pinned_module = nullptr;

    // Reference-counted lifetime rule: the worker releases its own module pin
    // atomically with thread exit, so there is no return into unmapped DLL code.
    FreeLibraryAndExitThread(pinned_module, result);
    return result; // unreachable, keeps the ThreadStart signature explicit.
}
}

namespace preloader_l { namespace runtime {

bool schedule_release_bootstrap(void* module) noexcept
{
    if (!module) {
        mark_bootstrap_failed(0x5401u);
        return false;
    }
    if (!mark_bootstrap_scheduled()) return false;
    PRELOADER_L_TRACE_NOTE(TraceEvent::bootstrap_scheduled, 0);

    // One bootstrap generation is scheduled per DLL load. Reset its
    // stop/readiness state before dispatch; the worker never clears a stop
    // request published after this point.
    clear_bootstrap_stop_request();
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    atomic_write(&g_native_acceptance_ready, 0);
#endif

    (void)DisableThreadLibraryCalls(static_cast<HMODULE>(module));

    HMODULE pinned = nullptr;
    const auto address = reinterpret_cast<const wchar_t*>(
        reinterpret_cast<uintptr_t>(&release_bootstrap_thread));
    if (!GetModuleHandleExW(kGetModuleHandleExFlagFromAddress, address, &pinned) || !pinned) {
        mark_bootstrap_failed(0x5402u);
        return false;
    }

    g_bootstrap_thread.handle = nullptr;
    g_bootstrap_thread.pinned_module = pinned;
    g_bootstrap_thread.thread_id = 0;
    atomic_write(&g_bootstrap_thread.running, 0);
    atomic_write(&g_bootstrap_thread.completed, 0);

    DWORD thread_id = 0;
    HANDLE thread = CreateThread(nullptr, 0, &release_bootstrap_thread, pinned,
                                 0u, &thread_id);
    if (!thread) {
        // Audit3 S0152: PROCESS_ATTACH owns the loader lock here. Keep the
        // counted self-pin published; DllShutdown retries its release only
        // after leaving the loader-lock path.
        mark_bootstrap_failed(0x5403u);
        return false;
    }

    // During PROCESS_ATTACH Windows serializes DLL initialization, so the new
    // thread cannot execute DLL code until DllMain returns. State is published
    // before/at CreateThread and no loader-lock wait or ResumeThread is needed.
    g_bootstrap_thread.handle = thread;
    g_bootstrap_thread.thread_id = thread_id;
    return true;
}

ReleaseBootstrapReadyResult wait_release_bootstrap_ready(uint32_t timeout_ms) noexcept
{
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    uint32_t waited = 0;
    for (;;) {
        if (atomic_read(&g_native_acceptance_ready) != 0) {
            const auto service = release_service_lifetime_snapshot();
            if (atomic_read(&g_bootstrap_thread.running) != 0 &&
                service.initialized != 0 && service.context != 0 &&
                service.model_service != 0 && service.owner_control != 0)
                return ReleaseBootstrapReadyResult::ready;
            return ReleaseBootstrapReadyResult::invalid_state;
        }
        // Audit3 S0158: a scheduler failure can be terminal before a worker
        // exists, so readiness must observe the published bootstrap phase
        // instead of waiting only for a worker-owned completed flag.
        if (snapshot_bootstrap().phase == BootstrapPhase::failed)
            return ReleaseBootstrapReadyResult::failed;
        if (atomic_read(&g_bootstrap_thread.completed) != 0)
            return ReleaseBootstrapReadyResult::failed;
        if (timeout_ms != 0xffffffffu && waited >= timeout_ms)
            return ReleaseBootstrapReadyResult::timeout;
        Sleep(1u);
        if (timeout_ms != 0xffffffffu) ++waited;
    }
#else
    (void)timeout_ms;
    return ReleaseBootstrapReadyResult::unsupported;
#endif
}

void request_release_bootstrap_stop() noexcept
{
    request_bootstrap_stop();
    request_release_server_stop_windows();
}

ReleaseBootstrapWaitResult wait_release_bootstrap_stopped(uint32_t timeout_ms) noexcept
{
    // A completed flag is not a quiescence proof: the worker publishes it just
    // before FreeLibraryAndExitThread. Keep the kernel thread object open and
    // wait for the signaled object so the self-pin has actually been released.
    HANDLE thread = g_bootstrap_thread.handle;
    if (!thread) {
        return atomic_read(&g_bootstrap_thread.completed) != 0
            ? ReleaseBootstrapWaitResult::stopped
            : ReleaseBootstrapWaitResult::not_running;
    }
    const DWORD wait_ms = timeout_ms == 0xffffffffu ? kInfinite : static_cast<DWORD>(timeout_ms);
    const DWORD rc = WaitForSingleObject(thread, wait_ms);
    if (rc == kWaitTimeout) return ReleaseBootstrapWaitResult::timeout;
    if (rc != kWaitObject0) return ReleaseBootstrapWaitResult::failed;
    if (g_bootstrap_thread.handle == thread) {
        // F321-S082-001: a signaled thread HANDLE remains owned until the
        // kernel confirms CloseHandle. Later DllShutdown retries the same
        // signaled object without ambiguity.
        if (!CloseHandle(thread)) return ReleaseBootstrapWaitResult::handle_close_failed;
        g_bootstrap_thread.handle = nullptr;
        g_bootstrap_thread.thread_id = 0;
    }
    return ReleaseBootstrapWaitResult::stopped;
}

ReleaseRuntimeShutdownResult shutdown_release_runtime_for_unload(uint32_t timeout_ms) noexcept
{
    if (atomic_read(&g_runtime_shutdown_state) == 2)
        return ReleaseRuntimeShutdownResult::shutdown;
    if (!atomic_claim(&g_runtime_shutdown_state, 0, 1))
        return ReleaseRuntimeShutdownResult::busy;

    const ShutdownDeadline shutdown_deadline = make_shutdown_deadline(timeout_ms);
    request_release_bootstrap_stop();
    const auto wait = wait_release_bootstrap_stopped(
        remaining_shutdown_timeout(shutdown_deadline));
    if (wait == ReleaseBootstrapWaitResult::timeout) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::bootstrap_timeout;
    }
    if (wait == ReleaseBootstrapWaitResult::failed) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::bootstrap_wait_failed;
    }
    if (wait == ReleaseBootstrapWaitResult::handle_close_failed) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::bootstrap_handle_close_failed;
    }
    if (!retry_bootstrap_pin_release()) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::bootstrap_pin_release_failed;
    }
    if (!restore_bootstrap_environment()) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::environment_cleanup_pending;
    }

    // Keep shared service/config policy alive until every host-facing hook
    // has been removed. If any uninstall retains live ownership, fail closed
    // with the backing policy intact so a later retry remains meaningful.
    const auto hooks = teardown_bootstrap_failure_hooks(
        remaining_shutdown_timeout(shutdown_deadline));
    if (hooks == HookTeardownResult::timeout) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::hook_quiescence_timeout;
    }
    if (hooks != HookTeardownResult::ok) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::hook_teardown_failed;
    }

    const auto service = shutdown_release_service_after_quiescence();
    if (service == ReleaseServiceShutdownResult::active_leases) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::service_active_leases;
    }
    if (service == ReleaseServiceShutdownResult::resource_cleanup_pending) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::service_cleanup_pending;
    }

    const bool ptd_detached = detach_current_thread_ptd();
    shutdown_ptd_runtime();
    if (!ptd_detached || ptd_cleanup_pending()) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::ptd_cleanup_pending;
    }

    // F08: DllShutdown is the final-unload boundary for this module generation.
    // Same-module restart after successful DllShutdown is intentionally
    // unsupported; a fresh LoadLibrary receives fresh INIT_ONCE/lock globals.
    if (!shutdown_ptd_platform_locks_windows() ||
        !shutdown_allocator_platform_lock_windows()) {
        atomic_write(&g_runtime_shutdown_state, 0);
        return ReleaseRuntimeShutdownResult::platform_lock_cleanup_pending;
    }

    atomic_write(&g_runtime_shutdown_state, 2);
    return ReleaseRuntimeShutdownResult::shutdown;
}


} }
