// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>

namespace redfield { namespace platform {
#if defined(_WIN32)
// Captures the first host EXE paths used by the curio registry policy.
bool initialize_windows_host_bridge() noexcept;
enum class WindowsRegistryHookMode : uint32_t {
    none = 0,
    inline_global = 1,
};
// Process-wide MinHook detours are the sole shipping authority for the five
// curio registry APIs. The current runtime stages them in the post-loader bootstrap's
// single SAFE-FREEZE transaction and keeps policy passive until configuration
// is loaded.
bool install_windows_registry_minhook_early() noexcept;
bool install_windows_registry_iat_hooks() noexcept;
// Shutdown preflight: while registry replacement/policy is still live, reject
// teardown if pseudo handles remain and fence future pseudo allocation once the
// zero-handle precondition is satisfied. Idempotent after the fence is set.
bool prepare_windows_registry_teardown() noexcept;
bool uninstall_windows_registry_iat_hooks() noexcept;
WindowsRegistryHookMode windows_registry_hook_mode() noexcept;
uintptr_t resolve_windows_host_symbol(const char* module, const char* name, void* context) noexcept;
#endif
} }
