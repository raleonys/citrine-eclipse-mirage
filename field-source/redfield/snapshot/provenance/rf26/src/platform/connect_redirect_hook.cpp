// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#error connect_redirect_hook.cpp is Windows-only.
#endif

#include "preloader_l/platform/connect_redirect.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#include <stdint.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace {
using HMODULE = void*;
using FARPROC = void*;
using NativeSocket = uintptr_t;
using LONG = long;

struct SockAddr { uint16_t family; char data[14]; };
struct SockAddrIn {
    uint16_t family;
    uint16_t port;
    uint32_t address;
    uint8_t zero[8];
};
static_assert(sizeof(SockAddrIn) == 16, "IPv4 sockaddr size mismatch");

using ConnectFn = int (__stdcall *)(NativeSocket, const SockAddr*, int);

extern "C" {
__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(const wchar_t* name);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE module, const char* name);
}

preloader_l::platform::MinHookTicket g_hook{};
preloader_l::platform::MinHookCallGate g_gate{};
volatile LONG g_calls = 0;
volatile LONG g_mapped = 0;

LONG atomic_increment(volatile LONG* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedIncrement(value);
#else
    return __atomic_add_fetch(value, 1L, __ATOMIC_RELAXED);
#endif
}
LONG atomic_load(volatile LONG* value) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_RELAXED);
#endif
}

int __stdcall redirected_connect(NativeSocket socket_handle,
                                 const SockAddr* name,
                                 int name_len) noexcept {
    preloader_l::platform::MinHookCallGuard guard(&g_gate);
    (void)atomic_increment(&g_calls);

    ConnectFn original = reinterpret_cast<ConnectFn>(
        preloader_l::platform::minhook_gate_forwarding_target(&g_gate));
    if (!original) return -1;

    // Exact consumer-observed sockaddr_in shape only. Requiring length 16 keeps
    // non-IPv4 and extended address forms byte-for-byte passthrough.
    if (!name || name_len != static_cast<int>(sizeof(SockAddrIn)))
        return original(socket_handle, name, name_len);

    const uint8_t* source = reinterpret_cast<const uint8_t*>(name);
    SockAddrIn local{};
    uint8_t* target = reinterpret_cast<uint8_t*>(&local);
    for (uint32_t i = 0; i < sizeof(SockAddrIn); ++i) target[i] = source[i];

    uint16_t mapped_port = local.port;
    const uint8_t* address_bytes = reinterpret_cast<const uint8_t*>(&local.address);
    if (!preloader_l::platform::map_ipv4_connect_destination(
            local.family, address_bytes, local.port, &mapped_port))
        return original(socket_handle, name, name_len);

    local.port = mapped_port;
    (void)atomic_increment(&g_mapped);
    PRELOADER_L_WINDOWS_TRACE(
        preloader_l::runtime::TraceEvent::connect_redirect_mapped,
        static_cast<uint32_t>(preloader_l::platform::connect_selected_port()));
    return original(socket_handle, reinterpret_cast<const SockAddr*>(&local),
                    static_cast<int>(sizeof(local)));
}

preloader_l::platform::ConnectRedirectInstallResult map_install_status(
    preloader_l::platform::MinHookInstallStatus status) noexcept {
    using preloader_l::platform::ConnectRedirectInstallResult;
    using preloader_l::platform::MinHookInstallStatus;
    switch (status) {
        case MinHookInstallStatus::installed: return ConnectRedirectInstallResult::installed;
        case MinHookInstallStatus::already_installed: return ConnectRedirectInstallResult::already_installed;
        case MinHookInstallStatus::target_unavailable: return ConnectRedirectInstallResult::export_unavailable;
        case MinHookInstallStatus::memory_allocation_failed: return ConnectRedirectInstallResult::near_allocation_failed;
        case MinHookInstallStatus::memory_protect_failed: return ConnectRedirectInstallResult::protect_failed;
        case MinHookInstallStatus::remove_failed: return ConnectRedirectInstallResult::release_failed;
        default: return ConnectRedirectInstallResult::patch_failed;
    }
}
}

namespace preloader_l { namespace platform {

ConnectRedirectInstallResult install_connect_redirect() noexcept {
    if (g_hook.enabled) return ConnectRedirectInstallResult::already_installed;
    if (minhook_ticket_has_ownership(&g_hook)) return ConnectRedirectInstallResult::patch_failed;

    HMODULE ws2 = GetModuleHandleW(L"ws2_32.dll");
    if (!ws2) return ConnectRedirectInstallResult::module_unavailable;
    FARPROC exported = GetProcAddress(ws2, "connect");
    if (!exported) return ConnectRedirectInstallResult::export_unavailable;

    const auto status = create_minhook(
        exported, reinterpret_cast<void*>(&redirected_connect), &g_hook);
    if (status != MinHookInstallStatus::installed) return map_install_status(status);
    MinHookTicket* batch[] = {&g_hook};
    MinHookCallGate* gates[] = {&g_gate};
    if (!enable_minhook_batch(batch, gates, 1u)) {
        (void)remove_minhook(&g_hook);
        return ConnectRedirectInstallResult::patch_failed;
    }
    return ConnectRedirectInstallResult::installed;
}

bool uninstall_connect_redirect() noexcept {
    if (!minhook_ticket_has_ownership(&g_hook)) return true;
    return disable_remove_minhook(&g_hook, &g_gate);
}

uint32_t connect_redirect_call_count() noexcept { return static_cast<uint32_t>(atomic_load(&g_calls)); }
uint32_t connect_redirect_mapped_count() noexcept { return static_cast<uint32_t>(atomic_load(&g_mapped)); }
uintptr_t connect_redirect_target_address() noexcept { return minhook_target_address(&g_hook); }
uintptr_t connect_redirect_trampoline_address() noexcept { return minhook_trampoline_address(&g_hook); }

} }
