#pragma once
#include "preloader_l/platform/registry_bridge.hpp"
#include "preloader_l/platform/registry_hook_runtime.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform { namespace host_bridge_windows_detail {
constexpr size_t path_capacity=32768u;
constexpr size_t registry_hook_count=5u;

enum class RegistryInlineInstallResult : uint8_t { installed=0,clean_failure=1,rollback_incomplete=2 };

int32_t dispatch_host_reg_open_a(uintptr_t original,void* root,const char* subkey,uint32_t options,uint32_t access,void** result) noexcept;
int32_t dispatch_host_reg_open_w(uintptr_t original,void* root,const uint16_t* subkey,uint32_t options,uint32_t access,void** result) noexcept;
int32_t dispatch_host_reg_query_a(uintptr_t original,void* key,const char* name,uint32_t* reserved,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;
int32_t dispatch_host_reg_query_w(uintptr_t original,void* key,const uint16_t* name,uint32_t* reserved,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;
int32_t dispatch_host_reg_close(uintptr_t original,void* key) noexcept;

WindowsRegistryHookMode registry_hook_mode() noexcept;
bool registry_hook_install_blocked() noexcept;
MinHookCallGate* registry_quiescence_gate(size_t index) noexcept;
bool registry_policy_active() noexcept;
void set_registry_policy_active(bool active) noexcept;
const uintptr_t* registry_replacement_addresses() noexcept;

RegistryInlineInstallResult install_registry_inline_global() noexcept;
bool uninstall_registry_hooks() noexcept;
} } }
