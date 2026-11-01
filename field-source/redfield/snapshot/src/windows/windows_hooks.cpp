// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/windows_hooks.hpp"
#include "preloader_l/hook_configuration.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include "preloader_l/platform/registry_hook_runtime.hpp"
#include "preloader_l/platform/crypt_observer.hpp"
#include "preloader_l/platform/connect_redirect.hpp"
#include <windows.h>
namespace preloader_l {
namespace p=preloader_l::platform;
bool WindowsHooks::start(std::shared_ptr<const ConfigSnapshot> configuration,
                         std::uint16_t port) noexcept {
    if (phase()!=HookPhase::idle || !configuration || !port) return false;
    phase_.store(HookPhase::staging);
    // Acquire the singleton configuration authority before touching global
    // tickets. A second owner must never clean up the first owner's hooks.
    configuration_bound_=bind_hook_configuration(std::move(configuration));
    if (!configuration_bound_) { phase_.store(HookPhase::stopped); return false; }
    const wchar_t* names[]={L"ws2_32.dll",L"advapi32.dll",L"crypt32.dll"};
    for (unsigned i=0;i<3;++i) {
        modules_[i]=LoadLibraryW(names[i]);
        if (!modules_[i]) { (void)stop(0); return false; }
    }
    p::set_connect_selected_port(port);
    if (!p::begin_minhook_batch_transaction()) { phase_.store(HookPhase::retained); return false; }
    const auto crypt=p::install_crypt_query_observer_main_module();
    const bool staged=(crypt==p::CryptObserverInstallResult::installed) &&
        p::install_windows_registry_minhook_early() &&
        p::install_connect_redirect()==p::ConnectRedirectInstallResult::installed;
    if (!staged) {
        (void)p::abort_minhook_batch_transaction();
        phase_.store(HookPhase::retained);
        (void)stop(0);
        return false;
    }
    const auto committed=p::commit_minhook_batch_transaction();
    if (committed!=p::MinHookBatchTransactionStatus::committed) {
        phase_.store(HookPhase::retained); (void)stop(0); return false;
    }
    // Policy is passive while all seven patches are staged. Publish it only
    // after the sole SAFE-FREEZE commit proves executable ownership.
    if (!p::install_windows_registry_iat_hooks()) {
        phase_.store(HookPhase::retained); (void)stop(0); return false;
    }
    phase_.store(HookPhase::committed);
    return true;
}
bool WindowsHooks::stop(std::uint32_t timeout_ms) noexcept {
    if (phase()==HookPhase::idle || phase()==HookPhase::stopped) return true;
    phase_.store(HookPhase::retained);
    // A live pseudo HKEY must remain closable through its detour; this
    // preflight changes no executable bytes if a logical owner remains.
    if (p::windows_registry_hook_mode()!=p::WindowsRegistryHookMode::none &&
        !p::prepare_windows_registry_teardown()) return false;
    if (p::minhook_batch_transaction_active() &&
        p::rollback_minhook_batch_transaction_with_timeout(timeout_ms)!=
            p::MinHookBatchTransactionStatus::rolled_back) return false;
    bool clean=true;
    clean=p::uninstall_connect_redirect() && clean;
    clean=p::uninstall_windows_registry_iat_hooks() && clean;
    clean=p::uninstall_crypt_query_observer() && clean;
    if (!clean || !p::shutdown_minhook_runtime()) return false;
    p::set_connect_selected_port(0);
    if (configuration_bound_) { release_hook_configuration(); configuration_bound_=false; }
    for (unsigned i=3;i>0;--i) {
        if (modules_[i-1] && !FreeLibrary(static_cast<HMODULE>(modules_[i-1]))) return false;
        modules_[i-1]=nullptr;
    }
    phase_.store(HookPhase::stopped);
    return true;
}
}
