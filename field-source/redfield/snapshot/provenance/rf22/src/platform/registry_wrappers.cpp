// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#error registry_wrappers.cpp is Windows-only.
#endif

#include "registry_hook_runtime_internal.hpp"
#include "redfield/platform/registry_policy.hpp"

namespace redfield { namespace platform { namespace host_bridge_windows_detail {

extern "C" int32_t __stdcall preloader_l_RegOpenKeyExA(
    void* root,const char* subkey,uint32_t options,uint32_t access,void** result) noexcept {
    MinHookCallGuard quiescence(registry_quiescence_gate(0u));
    const uintptr_t original = minhook_gate_forwarding_target(registry_quiescence_gate(0u));
    if (!registry_policy_active()) {
        const auto fn = reinterpret_cast<RegOpenAOriginal>(original);
        return fn ? fn(root,subkey,options,access,result) : host_error_invalid_parameter;
    }
    return dispatch_host_reg_open_a(original,root,subkey,options,access,result);
}

extern "C" int32_t __stdcall preloader_l_RegOpenKeyExW(
    void* root,const uint16_t* subkey,uint32_t options,uint32_t access,void** result) noexcept {
    MinHookCallGuard quiescence(registry_quiescence_gate(1u));
    const uintptr_t original = minhook_gate_forwarding_target(registry_quiescence_gate(1u));
    if (!registry_policy_active()) {
        const auto fn = reinterpret_cast<RegOpenWOriginal>(original);
        return fn ? fn(root,subkey,options,access,result) : host_error_invalid_parameter;
    }
    return dispatch_host_reg_open_w(original,root,subkey,options,access,result);
}

extern "C" int32_t __stdcall preloader_l_RegQueryValueExA(
    void* key,const char* name,uint32_t* reserved,uint32_t* type,
    uint8_t* data,uint32_t* bytes) noexcept {
    MinHookCallGuard quiescence(registry_quiescence_gate(2u));
    const uintptr_t original = minhook_gate_forwarding_target(registry_quiescence_gate(2u));
    if (!registry_policy_active()) {
        const auto fn = reinterpret_cast<RegQueryAOriginal>(original);
        return fn ? fn(key,name,reserved,type,data,bytes) : host_error_invalid_parameter;
    }
    return dispatch_host_reg_query_a(original,key,name,reserved,type,data,bytes);
}

extern "C" int32_t __stdcall preloader_l_RegQueryValueExW(
    void* key,const uint16_t* name,uint32_t* reserved,uint32_t* type,
    uint8_t* data,uint32_t* bytes) noexcept {
    MinHookCallGuard quiescence(registry_quiescence_gate(3u));
    const uintptr_t original = minhook_gate_forwarding_target(registry_quiescence_gate(3u));
    if (!registry_policy_active()) {
        const auto fn = reinterpret_cast<RegQueryWOriginal>(original);
        return fn ? fn(key,name,reserved,type,data,bytes) : host_error_invalid_parameter;
    }
    return dispatch_host_reg_query_w(original,key,name,reserved,type,data,bytes);
}

extern "C" int32_t __stdcall preloader_l_RegCloseKey(void* key) noexcept {
    MinHookCallGuard quiescence(registry_quiescence_gate(4u));
    const uintptr_t original = minhook_gate_forwarding_target(registry_quiescence_gate(4u));
    if (!registry_policy_active()) {
        const auto fn = reinterpret_cast<RegCloseOriginal>(original);
        return fn ? fn(key) : host_error_invalid_parameter;
    }
    return dispatch_host_reg_close(original,key);
}

const uintptr_t* registry_replacement_addresses() noexcept {
    static const uintptr_t replacements[registry_hook_count]={
        reinterpret_cast<uintptr_t>(&preloader_l_RegOpenKeyExA),
        reinterpret_cast<uintptr_t>(&preloader_l_RegOpenKeyExW),
        reinterpret_cast<uintptr_t>(&preloader_l_RegQueryValueExA),
        reinterpret_cast<uintptr_t>(&preloader_l_RegQueryValueExW),
        reinterpret_cast<uintptr_t>(&preloader_l_RegCloseKey),
    };
    return replacements;
}

} } }
