#if !defined(_WIN32)
#error registry_hook_runtime.cpp is Windows-only.
#endif
#include "registry_hook_runtime_internal.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#endif

namespace preloader_l { namespace platform { namespace host_bridge_windows_detail {
using DWORD=unsigned long;using HMODULE=void*;
extern "C" {
__declspec(dllimport) DWORD __stdcall GetModuleFileNameA(HMODULE module,char* buffer,DWORD size);
__declspec(dllimport) DWORD __stdcall GetModuleFileNameW(HMODULE module,uint16_t* buffer,DWORD size);
__declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char* module);
__declspec(dllimport) void* __stdcall GetProcAddress(HMODULE module,const char* name);
}

namespace {
struct HostBridgeRuntimeState {
    char path_a[path_capacity];
    uint16_t path_w[path_capacity];
    HostBridgeState bridge;
};
HostBridgeRuntimeState g_host_runtime{};
volatile long g_registry_policy_active = 0;
}

bool registry_policy_active() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    return _InterlockedCompareExchange(&g_registry_policy_active, 0, 0) != 0;
#else
    return __atomic_load_n(&g_registry_policy_active, __ATOMIC_ACQUIRE) != 0;
#endif
}
void set_registry_policy_active(bool active) noexcept {
#if defined(_MSC_VER) && !defined(__clang__)
    (void)_InterlockedExchange(&g_registry_policy_active, active ? 1L : 0L);
#else
    __atomic_store_n(&g_registry_policy_active, active ? 1L : 0L, __ATOMIC_RELEASE);
#endif
}

bool initialize_host_bridge_runtime() noexcept {
    clear_registry_policy_state_for_test();
    reset_host_bridge(&g_host_runtime.bridge);
    const DWORD a=GetModuleFileNameA(nullptr,g_host_runtime.path_a,static_cast<DWORD>(path_capacity));
    const DWORD w=GetModuleFileNameW(nullptr,g_host_runtime.path_w,static_cast<DWORD>(path_capacity));
    if(!a||a>=path_capacity||!w||w>=path_capacity)return false;
    g_host_runtime.bridge.executable_path=HostExecutablePath{
        g_host_runtime.path_a,static_cast<size_t>(a),
        g_host_runtime.path_w,static_cast<size_t>(w)};
    return true;
}

int32_t dispatch_host_reg_open_a(uintptr_t original,void* root,const char* subkey,uint32_t options,uint32_t access,void** result) noexcept {
    HostBridgeState invocation=g_host_runtime.bridge;
    invocation.originals=HostOriginalFunctions{};
    invocation.originals.reg_open_a=reinterpret_cast<RegOpenAOriginal>(original);
    return bridge_reg_open_a(&invocation,root,subkey,options,access,result);
}
int32_t dispatch_host_reg_open_w(uintptr_t original,void* root,const uint16_t* subkey,uint32_t options,uint32_t access,void** result) noexcept {
    HostBridgeState invocation=g_host_runtime.bridge;
    invocation.originals=HostOriginalFunctions{};
    invocation.originals.reg_open_w=reinterpret_cast<RegOpenWOriginal>(original);
    return bridge_reg_open_w(&invocation,root,subkey,options,access,result);
}
int32_t dispatch_host_reg_query_a(uintptr_t original,void* key,const char* name,uint32_t* reserved,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    HostBridgeState invocation=g_host_runtime.bridge;
    invocation.originals=HostOriginalFunctions{};
    invocation.originals.reg_query_a=reinterpret_cast<RegQueryAOriginal>(original);
    return bridge_reg_query_a(&invocation,key,name,reserved,type,data,bytes);
}
int32_t dispatch_host_reg_query_w(uintptr_t original,void* key,const uint16_t* name,uint32_t* reserved,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    HostBridgeState invocation=g_host_runtime.bridge;
    invocation.originals=HostOriginalFunctions{};
    invocation.originals.reg_query_w=reinterpret_cast<RegQueryWOriginal>(original);
    return bridge_reg_query_w(&invocation,key,name,reserved,type,data,bytes);
}
int32_t dispatch_host_reg_close(uintptr_t original,void* key) noexcept {
    HostBridgeState invocation=g_host_runtime.bridge;
    invocation.originals=HostOriginalFunctions{};
    invocation.originals.reg_close=reinterpret_cast<RegCloseOriginal>(original);
    return bridge_reg_close(&invocation,key);
}
} } }

namespace preloader_l { namespace platform {
using namespace host_bridge_windows_detail;
bool initialize_windows_host_bridge() noexcept { return initialize_host_bridge_runtime(); }
bool install_windows_registry_minhook_early() noexcept {
    if(registry_hook_mode()!=WindowsRegistryHookMode::none)return true;
    if(registry_hook_install_blocked())return false;
    if(!initialize_windows_host_bridge())return false;
    set_registry_policy_active(false);
    return install_registry_inline_global()==RegistryInlineInstallResult::installed;
}
bool install_windows_registry_iat_hooks() noexcept {
    if(registry_hook_mode()==WindowsRegistryHookMode::none &&
       !install_windows_registry_minhook_early()) return false;
    set_registry_policy_active(true);
    return true;
}
bool prepare_windows_registry_teardown() noexcept {
    // F01: do not deactivate/restore registry interception until every pseudo
    // handle has been retired. A failed preflight is deliberately non-mutating
    // so the host can still close the pseudo handle through RegCloseKey.
    return begin_registry_policy_teardown();
}
bool uninstall_windows_registry_iat_hooks() noexcept {
    if (!prepare_windows_registry_teardown()) return false;
    set_registry_policy_active(false);
    if (!uninstall_registry_hooks()) return false;
    clear_registry_policy_state_for_test();
    return true;
}
WindowsRegistryHookMode windows_registry_hook_mode() noexcept { return registry_hook_mode(); }
uintptr_t resolve_windows_host_symbol(const char* module,const char* name,void*) noexcept {
    if(!module||!name)return 0;HMODULE loaded=GetModuleHandleA(module);if(!loaded)return 0;return reinterpret_cast<uintptr_t>(GetProcAddress(loaded,name));
}
} }
