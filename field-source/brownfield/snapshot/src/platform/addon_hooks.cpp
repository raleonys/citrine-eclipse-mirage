#if !defined(_WIN32)
#error addon_hooks.cpp is Windows-only.
#endif
#include "preloader_l/platform/addon_hooks.hpp"
#include "preloader_l/platform/addon_hook_policy.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <windows.h>
#include <shlobj.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

namespace {
using namespace preloader_l::platform;
using CreateAFn = BOOL (WINAPI *)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
   LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
using CreateWFn = BOOL (WINAPI *)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES,
   LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
using FolderAFn = HRESULT (WINAPI *)(HWND, int, HANDLE, DWORD, LPSTR);
using FolderWFn = HRESULT (WINAPI *)(HWND, int, HANDLE, DWORD, LPWSTR);
constexpr size_t kAddonCount = 4;
struct AddonHookState {
    MinHookTicket tickets[kAddonCount]{};
    MinHookCallGate gates[kAddonCount]{};
    HMODULE shell{};
    bool shell_owned{};
    bool recovery_pending{};
};
AddonHookState g_addon{};

bool should_block_a(const char* application) noexcept {
    if (!application || !preloader_l::config::runtime_configuration_loaded()) return false;
    preloader_l::config::RuntimeConfigurationReadLease lease{};
    if (!preloader_l::config::begin_runtime_configuration_read(&lease)) return false;
    const auto block = preloader_l::config::runtime_configuration_setting("BlockReruns");
    const auto path = preloader_l::config::runtime_configuration_setting("ClientPath");
    const bool selected = addon_hook_policy::exact_true(block.data, block.size) &&
        path.present && path.data &&
        addon_hook_policy::ascii_fold_equal(application,path.data,path.size);
    preloader_l::config::end_runtime_configuration_read(&lease);
    return selected;
}
bool should_block_w(const wchar_t* application) noexcept {
    if (!application || !preloader_l::config::runtime_configuration_loaded()) return false;
    preloader_l::config::RuntimeConfigurationReadLease lease{};
    if (!preloader_l::config::begin_runtime_configuration_read(&lease)) return false;
    const auto block = preloader_l::config::runtime_configuration_setting("BlockReruns");
    const auto path = preloader_l::config::runtime_configuration_setting("ClientPath");
    const bool selected = addon_hook_policy::exact_true(block.data,block.size) &&
        path.present && path.data &&
        addon_hook_policy::ascii_fold_equal_wide(application,path.data,path.size);
    preloader_l::config::end_runtime_configuration_read(&lease);
    return selected;
}

// Statically bounded local snapshot; never keep a pointer to a transient Config2 value
// after releasing the policy read lease. A missing/invalid setting is passthrough.
bool document_utf16(wchar_t (&dest)[260], int csidl) noexcept {
    if (csidl != 5 || !preloader_l::config::runtime_configuration_loaded()) return false;
    preloader_l::config::RuntimeConfigurationReadLease lease{};
    if (!preloader_l::config::begin_runtime_configuration_read(&lease)) return false;
    const auto documents = preloader_l::config::runtime_configuration_setting("Documents");
    bool result = false;
    if (documents.present && addon_hook_policy::selected_documents(csidl,documents.data,documents.size)
        && documents.size <= 32767u && documents.size <= static_cast<size_t>(INT_MAX)) {
        const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            documents.data, static_cast<int>(documents.size), dest, 259);
        if (count > 0 && count <= 259) {
            bool valid = true;
            for (int i = 0; i < count; ++i) if (dest[i] == 0) valid = false;
            if (valid) { dest[count] = 0; result = true; }
        }
    }
    preloader_l::config::end_runtime_configuration_read(&lease);
    return result;
}

BOOL WINAPI create_process_a(LPCSTR app, LPSTR command, LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta, BOOL inherit, DWORD flags, LPVOID env,
    LPCSTR cwd, LPSTARTUPINFOA startup, LPPROCESS_INFORMATION process) noexcept {
    MinHookCallGuard guard(&g_addon.gates[0]);
    auto original = reinterpret_cast<CreateAFn>(minhook_gate_forwarding_target(&g_addon.gates[0]));
    if (!original) { SetLastError(ERROR_PROC_NOT_FOUND); return FALSE; }
    if (should_block_a(app)) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
    return original(app,command,pa,ta,inherit,flags,env,cwd,startup,process);
}
BOOL WINAPI create_process_w(LPCWSTR app, LPWSTR command, LPSECURITY_ATTRIBUTES pa,
    LPSECURITY_ATTRIBUTES ta, BOOL inherit, DWORD flags, LPVOID env,
    LPCWSTR cwd, LPSTARTUPINFOW startup, LPPROCESS_INFORMATION process) noexcept {
    MinHookCallGuard guard(&g_addon.gates[1]);
    auto original = reinterpret_cast<CreateWFn>(minhook_gate_forwarding_target(&g_addon.gates[1]));
    if (!original) { SetLastError(ERROR_PROC_NOT_FOUND); return FALSE; }
    if (should_block_w(app)) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
    return original(app,command,pa,ta,inherit,flags,env,cwd,startup,process);
}
HRESULT WINAPI folder_path_a(HWND owner, int folder, HANDLE token, DWORD flags, LPSTR path) noexcept {
    MinHookCallGuard guard(&g_addon.gates[2]);
    auto original = reinterpret_cast<FolderAFn>(minhook_gate_forwarding_target(&g_addon.gates[2]));
    if (!original) return E_FAIL;
    wchar_t wide[260]{};
    if (path && document_utf16(wide,folder)) {
        BOOL substituted = FALSE;
        const int chars = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
            wide, -1, nullptr, 0, nullptr, &substituted);
        if (chars > 0 && chars <= 260 && !substituted) {
            char tmp[260]{};
            substituted = FALSE;
            const int wrote = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                wide, -1, tmp, 260, nullptr, &substituted);
            if (wrote == chars && !substituted) {
                for (int i = 0; i < wrote; ++i) path[i]=tmp[i];
                return S_OK;
            }
        }
    }
    return original(owner,folder,token,flags,path);
}
HRESULT WINAPI folder_path_w(HWND owner, int folder, HANDLE token, DWORD flags, LPWSTR path) noexcept {
    MinHookCallGuard guard(&g_addon.gates[3]);
    auto original = reinterpret_cast<FolderWFn>(minhook_gate_forwarding_target(&g_addon.gates[3]));
    if (!original) return E_FAIL;
    wchar_t wide[260]{};
    if (path && document_utf16(wide,folder)) {
        size_t i=0;
        for (; i<260; ++i) { path[i]=wide[i]; if (!wide[i]) break; }
        return S_OK;
    }
    return original(owner,folder,token,flags,path);
}

bool drain_created() noexcept {
    bool okay=true;
    for (size_t i=kAddonCount; i>0; --i) {
        auto& t=g_addon.tickets[i-1];
        if (!minhook_ticket_has_ownership(&t)) continue;
        okay = remove_minhook(&t) && okay;
    }
    // An owned module must not be forgotten if FreeLibrary reports failure.
    // The same cleanup path is used by both shutdown and installation rollback.
    if (okay && g_addon.shell_owned) {
        if (!FreeLibrary(g_addon.shell)) {
            okay=false; // retry from uninstall_addon_hooks after the failed unwind
        } else {
            g_addon.shell=nullptr; g_addon.shell_owned=false;
        }
    }
    g_addon.recovery_pending=!okay;
    return okay;
}
} // namespace

namespace preloader_l { namespace platform {
// Config-selected addon staging in the shared MinHook transaction.
// Caller supplies the committed and frozen Config2 mask. No second transaction here.
bool stage_addon_hooks(unsigned mask) noexcept {
    if ((mask & ~3u) != 0u || g_addon.recovery_pending) return false;
    for (size_t i=0; i<kAddonCount; ++i)
        if (minhook_ticket_has_ownership(&g_addon.tickets[i])) return false;
    if (!mask) return true; // no addon API/module lookup and no staged tickets
    HMODULE kernel=nullptr;
    HMODULE shell=nullptr;
    bool owned=false;
    if (mask & 1u) {
        kernel=GetModuleHandleW(L"kernel32.dll");
        if (!kernel) return false;
    }
    if (mask & 2u) {
        shell=GetModuleHandleW(L"shell32.dll");
        if (!shell) { shell=LoadLibraryW(L"shell32.dll"); owned=shell!=nullptr; }
        if (!shell) return false;
    }
    g_addon.shell=shell; g_addon.shell_owned=owned;
    const char* names[kAddonCount]={"CreateProcessA","CreateProcessW","SHGetFolderPathA","SHGetFolderPathW"};
    void* detours[kAddonCount]={
        reinterpret_cast<void*>(&create_process_a),reinterpret_cast<void*>(&create_process_w),
        reinterpret_cast<void*>(&folder_path_a),reinterpret_cast<void*>(&folder_path_w)};
    MinHookTicket* batch[kAddonCount]{};
    MinHookCallGate* gates[kAddonCount]{};
    size_t selected_count=0;
    for (size_t i=0; i<kAddonCount; ++i) {
        const bool chosen = (i<2) ? ((mask&1u)!=0u) : ((mask&2u)!=0u);
        if (!chosen) continue;
        FARPROC target=GetProcAddress(i<2?kernel:shell,names[i]);
        if (!target || create_minhook(reinterpret_cast<void*>(target),detours[i],
                  &g_addon.tickets[i])!=MinHookInstallStatus::installed) {
            (void)drain_created(); return false;
        }
        batch[selected_count]=&g_addon.tickets[i];
        gates[selected_count]=&g_addon.gates[i];
        ++selected_count;
    }
    if (!enable_minhook_batch(batch,gates,selected_count)) {
        (void)drain_created(); return false;
    }
    return true;
}
bool uninstall_addon_hooks() noexcept {
    MinHookTicket* batch[kAddonCount]{};
    MinHookCallGate* gates[kAddonCount]{};
    size_t count=0;
    for (size_t i=0; i<kAddonCount; ++i) {
        if (!minhook_ticket_has_ownership(&g_addon.tickets[i])) continue;
        batch[count]=&g_addon.tickets[i]; gates[count]=&g_addon.gates[i]; ++count;
    }
    if (count && !disable_minhook_batch(batch,gates,count)) {
        g_addon.recovery_pending=true;
        return false;
    }
    return drain_created();
}
} }
