// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#error crypt_observer.cpp is Windows-only.
#endif

#include "redfield/platform/crypt_observer.hpp"
#include "redfield/platform/minhook_runtime.hpp"
#include "redfield/platform/crypt_wrapper.hpp"
#if REDFIELD_TRACE_DIAGNOSTICS
#include "crypt_observer_diagnostics_internal.hpp"
#endif
#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace {
using BOOL = int;
using DWORD = unsigned long;
using HANDLE = void*;
using CryptQueryObjectFn = BOOL (__stdcall *)(
    DWORD, const void*, DWORD, DWORD, DWORD,
    DWORD*, DWORD*, DWORD*, void**, void**, const void**);

extern "C" {
// Preserve a real CRYPT32 loader dependency in the shipping PE. MinHook detours
// the resolved export entry; no project-owned trampoline/ASM wrapper remains.
__declspec(dllimport) BOOL __stdcall CryptQueryObject(
    DWORD, const void*, DWORD, DWORD, DWORD,
    DWORD*, DWORD*, DWORD*, void**, void**, const void**);
__declspec(dllimport) DWORD __stdcall GetLastError();
__declspec(dllimport) void __stdcall SetLastError(DWORD value);
__declspec(dllimport) HANDLE __stdcall GetCurrentProcess();
__declspec(dllimport) BOOL __stdcall ReadProcessMemory(HANDLE process, const void* base,
                                                       void* buffer, size_t size, size_t* read);
}

redfield::platform::MinHookTicket g_query_hook{};
redfield::platform::MinHookCallGate g_query_gate{};


bool safe_read_bytes(const void* address, void* output, size_t bytes) noexcept {
    if (!address || !output || bytes == 0) return false;
    size_t read = 0;
    return ReadProcessMemory(GetCurrentProcess(), address, output, bytes, &read) && read == bytes;
}

bool safe_wide_path_length_and_validate(const void* object,
                                        size_t* chars_read) noexcept {
    if (chars_read) *chars_read = 0;
    if (!object || !chars_read) return false;

    const wchar_t* source = static_cast<const wchar_t*>(object);
    constexpr size_t kMaxPathUnits = 32768u;

    size_t i = 0;
    while (i < kMaxPathUnits) {
        wchar_t wc = 0;
        if (!safe_read_bytes(source + i, &wc, sizeof(wc))) return false;
        if (wc == 0) {
            *chars_read = i;
            return true;
        }

        const uint16_t unit = static_cast<uint16_t>(wc);
        if (unit >= 0xD800u && unit <= 0xDBFFu) {
            if (i + 1u >= kMaxPathUnits) return false;
            wchar_t next = 0;
            if (!safe_read_bytes(source + i + 1u, &next, sizeof(next))) return false;
            const uint16_t low = static_cast<uint16_t>(next);
            if (low < 0xDC00u || low > 0xDFFFu) return false;
            i += 2u;
            continue;
        }
        if (unit >= 0xDC00u && unit <= 0xDFFFu) return false;
        ++i;
    }
    return false;
}

const void* crypt_query_effective_object(uint32_t object_type, const void* object,
                                         void* caller) noexcept {
    const DWORD incoming_error = GetLastError();
#if REDFIELD_TRACE_DIAGNOSTICS
    redfield::platform::trace_crypt_query_diagnostics(object_type, caller);
#else
    (void)caller;
#endif

    const void* effective = object;
    bool companion = false;
    if (object_type == 1u) {
        if (!object) {
            const auto plan = redfield::platform::curio_crypt_forward_plan(1u, nullptr, 0u);
            effective = plan.effective_object;
            companion = plan.decision ==
                redfield::platform::CurioCryptObjectDecision::curio_companion_branch;
        } else {
            size_t chars = 0u;
            if (safe_wide_path_length_and_validate(object, &chars)) {
                const auto plan = redfield::platform::curio_crypt_forward_plan(
                    1u, static_cast<const wchar_t*>(object), chars);
                effective = plan.effective_object;
                companion = plan.decision ==
                    redfield::platform::CurioCryptObjectDecision::curio_companion_branch;
            }
        }
    }

#if REDFIELD_TRACE_DIAGNOSTICS
    if (companion) redfield::platform::trace_crypt_query_companion_branch();
#else
    (void)companion;
#endif
    SetLastError(incoming_error);
    return effective;
}

BOOL __stdcall crypt_query_detour(
    DWORD object_type, const void* object, DWORD expected_content, DWORD expected_format,
    DWORD flags, DWORD* msg_cert_encoding, DWORD* content_type, DWORD* format_type,
    void** cert_store, void** message, const void** context) noexcept {
    redfield::platform::MinHookCallGuard guard(&g_query_gate);
#if defined(_MSC_VER) && !defined(__clang__)
    void* caller = _ReturnAddress();
#else
    void* caller = __builtin_return_address(0);
#endif
    const void* effective = crypt_query_effective_object(object_type, object, caller);
    CryptQueryObjectFn original = reinterpret_cast<CryptQueryObjectFn>(
        redfield::platform::minhook_gate_forwarding_target(&g_query_gate));
    if (!original) return 0;
    return original(object_type, effective, expected_content, expected_format, flags,
                    msg_cert_encoding, content_type, format_type,
                    cert_store, message, context);
}

redfield::platform::CryptObserverInstallResult install_query_observer() noexcept {
    if (g_query_hook.enabled)
        return redfield::platform::CryptObserverInstallResult::already_installed;
    if (redfield::platform::minhook_ticket_has_ownership(&g_query_hook))
        return redfield::platform::CryptObserverInstallResult::inline_hook_failed;

    void* target = reinterpret_cast<void*>(&CryptQueryObject);
    const auto status = redfield::platform::create_minhook(
        target, reinterpret_cast<void*>(&crypt_query_detour), &g_query_hook);
    if (status != redfield::platform::MinHookInstallStatus::installed)
        return redfield::platform::CryptObserverInstallResult::inline_hook_failed;

    redfield::platform::MinHookTicket* batch[] = {&g_query_hook};
    redfield::platform::MinHookCallGate* gates[] = {&g_query_gate};
    if (!redfield::platform::enable_minhook_batch(batch, gates, 1u)) {
        (void)redfield::platform::remove_minhook(&g_query_hook);
        return redfield::platform::CryptObserverInstallResult::inline_hook_failed;
    }
    return redfield::platform::CryptObserverInstallResult::installed;
}

bool uninstall_query_observer() noexcept {
    if (!redfield::platform::minhook_ticket_has_ownership(&g_query_hook)) return true;
    return redfield::platform::disable_remove_minhook(&g_query_hook, &g_query_gate);
}

} // namespace

namespace redfield { namespace platform {

CryptObserverInstallResult install_crypt_query_observer_main_module() noexcept {
    return install_query_observer();
}

bool uninstall_crypt_query_observer() noexcept {
#if REDFIELD_TRACE_DIAGNOSTICS
    if (!uninstall_crypt_observer_diagnostics()) return false;
#endif
    return uninstall_query_observer();
}

} }
