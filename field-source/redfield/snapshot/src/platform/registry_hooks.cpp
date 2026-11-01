// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#error registry_hooks.cpp is Windows-only.
#endif

#include "registry_hook_runtime_internal.hpp"

namespace preloader_l { namespace platform { namespace host_bridge_windows_detail {
namespace {
constexpr const char* registry_hook_module = "advapi32.dll";
constexpr const char* registry_hook_names[registry_hook_count] = {
    "RegOpenKeyExA",
    "RegOpenKeyExW",
    "RegQueryValueExA",
    "RegQueryValueExW",
    "RegCloseKey",
};

const char* registry_hook_name(size_t index) noexcept {
    return index < registry_hook_count ? registry_hook_names[index] : nullptr;
}

struct RegistryHookState {
    MinHookTicket hooks[registry_hook_count];
    MinHookCallGate gates[registry_hook_count];
    WindowsRegistryHookMode mode;
    bool recovery_pending;
};
RegistryHookState g_registry_hooks{};

bool remove_created_registry_hooks(RegistryHookState& state, size_t count) noexcept {
    bool ok = true;
    while (count) {
        --count;
        if (!minhook_ticket_has_ownership(&state.hooks[count])) continue;
        ok = remove_minhook(&state.hooks[count]) && ok;
    }
    state.recovery_pending = !ok;
    return ok;
}
}

WindowsRegistryHookMode registry_hook_mode() noexcept { return g_registry_hooks.mode; }
bool registry_hook_install_blocked() noexcept { return g_registry_hooks.recovery_pending; }
MinHookCallGate* registry_quiescence_gate(size_t index) noexcept {
    return index<registry_hook_count?&g_registry_hooks.gates[index]:nullptr;
}


RegistryInlineInstallResult install_registry_inline_global() noexcept {
    auto& state=g_registry_hooks;
    if (state.recovery_pending) return RegistryInlineInstallResult::rollback_incomplete;
    if (state.mode==WindowsRegistryHookMode::inline_global) return RegistryInlineInstallResult::installed;

    const uintptr_t* replacements=registry_replacement_addresses();
    MinHookTicket* batch[registry_hook_count]{};
    MinHookCallGate* gates[registry_hook_count]{};
    size_t created=0;
    for(;created<registry_hook_count;++created) {
        const uintptr_t target=preloader_l::platform::resolve_windows_host_symbol(
            registry_hook_module,registry_hook_name(created),nullptr);
        if(!target||!replacements[created])break;
        const auto status=create_minhook(
            reinterpret_cast<void*>(target),reinterpret_cast<void*>(replacements[created]),
            &state.hooks[created]);
        if(status!=MinHookInstallStatus::installed)break;
        batch[created]=&state.hooks[created];
        gates[created]=&state.gates[created];
    }
    if(created!=registry_hook_count) {
        const bool cleanup_ok = remove_created_registry_hooks(state, created);
        return cleanup_ok ? RegistryInlineInstallResult::clean_failure
                          : RegistryInlineInstallResult::rollback_incomplete;
    }

    if(!enable_minhook_batch(batch, gates, registry_hook_count)) {
        bool cleanup_ok=true;
        for(size_t i=0;i<registry_hook_count;++i)
            cleanup_ok=remove_minhook(&state.hooks[i])&&cleanup_ok;
        state.recovery_pending=!cleanup_ok;
        return cleanup_ok ? RegistryInlineInstallResult::clean_failure
                          : RegistryInlineInstallResult::rollback_incomplete;
    }

    state.mode=WindowsRegistryHookMode::inline_global;
    state.recovery_pending=false;
    return RegistryInlineInstallResult::installed;
}

bool uninstall_registry_hooks() noexcept {
    auto& state=g_registry_hooks;
    if(state.mode==WindowsRegistryHookMode::none&&!state.recovery_pending) {
        return true;
    }

    MinHookTicket* tickets[registry_hook_count]{};
    MinHookCallGate* gates[registry_hook_count]{};
    size_t active=0;
    for(size_t i=0;i<registry_hook_count;++i) {
        if(!minhook_ticket_has_ownership(&state.hooks[i]))continue;
        tickets[active]=&state.hooks[i];
        gates[active]=&state.gates[i];
        ++active;
    }

    if(active&& !disable_minhook_batch(tickets,gates,active)) {
        state.recovery_pending=true;
        return false;
    }

    bool ok=true;
    for(size_t i=0;i<active;++i)ok=remove_minhook(tickets[i])&&ok;
    if(!ok) {
        state.recovery_pending=true;
        return false;
    }

    state.mode=WindowsRegistryHookMode::none;
    state.recovery_pending=false;
    return true;
}

} } }
