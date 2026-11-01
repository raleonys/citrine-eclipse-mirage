#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

constexpr int32_t host_error_success = 0;
constexpr int32_t host_error_file_not_found = 2;
constexpr int32_t host_error_invalid_parameter = 87;
constexpr int32_t host_error_more_data = 234;
constexpr uint32_t host_reg_sz = 1u;

// URSORYX registry policy. The bridge always executes the retained-original
// Win32 API first; these helpers then apply only the curio special-key
// semantics for tracked client/language handles.
int32_t registry_after_open_a(const char* subkey, int32_t original_status,
                              uintptr_t original_result, uintptr_t* result) noexcept;
int32_t registry_after_open_w(const uint16_t* subkey, int32_t original_status,
                              uintptr_t original_result, uintptr_t* result) noexcept;

int32_t registry_after_query_a(uintptr_t key, const char* value_name,
                               int32_t original_status,uint32_t input_capacity,
                               const char* executable_path,size_t executable_path_length,
                               uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;
int32_t registry_after_query_w(uintptr_t key, const uint16_t* value_name,
                               int32_t original_status,uint32_t input_capacity,
                               const uint16_t* executable_path,size_t executable_path_units,
                               uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept;

// Pseudo handles are closed locally. Real tracked handles are removed from the
// class sets only after the retained-original RegCloseKey succeeds.
bool registry_close_pseudo(uintptr_t key,int32_t* status) noexcept;
void registry_after_real_close(uintptr_t key,int32_t original_status) noexcept;

// Teardown fence: succeeds only when no pseudo HKEY remains outstanding. Once
// established, new pseudo handles cannot be published until a successful reset.
bool begin_registry_policy_teardown() noexcept;

// Test/reset boundary used by source-level and native-acceptance harnesses.
// A successful reset reopens pseudo-handle publication.
void clear_registry_policy_state_for_test() noexcept;

} }
