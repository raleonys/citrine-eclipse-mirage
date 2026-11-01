#if !defined(_WIN32)
#error core_hooks.cpp is Windows-only.
#endif
#include "preloader_l/platform/core_hooks.hpp"
#include "preloader_l/platform/crypt_observer.hpp"
#include "preloader_l/platform/registry_hook_runtime.hpp"
#include "preloader_l/platform/connect_redirect.hpp"
#include "preloader_l/runtime/trace_macros.hpp"
#include "preloader_l/runtime/runtime_status.hpp"

namespace preloader_l { namespace platform {
bool stage_core_hooks() noexcept {
    const auto crypt = install_crypt_query_observer_main_module();
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::crypt_observer_status,
                              static_cast<uint32_t>(crypt));
    if (crypt != CryptObserverInstallResult::installed &&
        crypt != CryptObserverInstallResult::already_installed) return false;
    if (!install_windows_registry_minhook_early()) return false;
    const auto connect_status = install_connect_redirect();
    PRELOADER_L_WINDOWS_TRACE(preloader_l::runtime::TraceEvent::connect_redirect_status,
                              static_cast<uint32_t>(connect_status));
    return connect_status == ConnectRedirectInstallResult::installed ||
           connect_status == ConnectRedirectInstallResult::already_installed;
}
} }
