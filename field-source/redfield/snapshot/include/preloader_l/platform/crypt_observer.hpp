// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>
#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 0
#endif

namespace preloader_l { namespace platform {

enum class CryptObserverInstallResult : uint32_t {
    installed = 1,
    already_installed = 2,
    import_not_found = 3,
    invalid_image = 4,
    protect_failed = 5,
    original_unavailable = 6,
    inline_hook_failed = 7,
    tracking_capacity_exhausted = 8,
};

// Loader-lock-compatible best-effort observers for the main EXE. Both are
// strictly pass-through: parameters, output buffers, BOOL result and the
// observable LastError at the real API boundary are not intentionally changed.
// CryptQueryObject uses the curio process-wide function-level
// inline-hook topology; the exported API name is retained for compatibility.
CryptObserverInstallResult install_crypt_query_observer_main_module() noexcept;
// Backward-compatible teardown entry point. Restores the process-wide
// CryptQueryObject detour and every IAT observer slot still owned by this DLL.
bool uninstall_crypt_query_observer() noexcept;
// Deferred retry. CryptQueryObject is global/function-level, so this returns
// one on installed/already-installed rather than sweeping module IATs.

#if PRELOADER_L_TRACE_DIAGNOSTICS
// Dedicated debug diagnostics only. These APIs and their IAT observer
// implementations are not compiled into the normalized release target.
CryptObserverInstallResult install_crypt_msg_get_param_observer_main_module() noexcept;
CryptObserverInstallResult install_cert_find_certificate_observer_main_module() noexcept;
uint32_t install_crypt_msg_get_param_observer_loaded_modules() noexcept;
uint32_t install_cert_find_certificate_observer_loaded_modules() noexcept;
uint32_t crypt_query_observer_call_count() noexcept;
uint32_t crypt_msg_get_param_observer_call_count() noexcept;
uint32_t cert_find_certificate_observer_call_count() noexcept;
#endif

} }
