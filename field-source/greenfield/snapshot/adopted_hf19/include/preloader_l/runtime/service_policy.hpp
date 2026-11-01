#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/runtime/service_abi.hpp"

namespace preloader_l { namespace runtime {
struct ReleaseServerContext;

struct ModelSettingInput {
    const char* key;
    size_t key_size;
    const char* value;
    size_t value_size;
};

bool initialize_model_service_policy(
    model_abi::ReleaseServiceObject548* service,
    ReleaseServerContext* adapter) noexcept;
bool release_model_service_policy_retryable(
    model_abi::ReleaseServiceObject548* service) noexcept;
void release_model_service_policy(
    model_abi::ReleaseServiceObject548* service) noexcept;

// Transactional configuration ownership transfer. The live current-directory
// path is intentionally not part of this swap; document/settings and the four
// curio collection strings are transferred as one prepared unit.
void swap_model_service_configuration_policy(
    model_abi::ReleaseServiceObject548* live,
    model_abi::ReleaseServiceObject548* candidate) noexcept;

ReleaseServerContext* model_service_adapter(
    model_abi::ReleaseServiceObject548* service) noexcept;

bool commit_model_service_document(
    model_abi::ReleaseServiceObject548* service,
    const char* xml, size_t length) noexcept;
bool commit_model_service_settings(
    model_abi::ReleaseServiceObject548* service,
    const ModelSettingInput* entries, size_t count) noexcept;
void set_model_service_configuration_state(
    model_abi::ReleaseServiceObject548* service,
    bool loaded, uint32_t present_sections) noexcept;
bool commit_model_service_current_directory(
    model_abi::ReleaseServiceObject548* service,
    const char* value, size_t length) noexcept;
bool commit_model_service_current_directory_wide(
    model_abi::ReleaseServiceObject548* service,
    const uint16_t* value, size_t length) noexcept;

bool model_service_configuration_loaded(
    const model_abi::ReleaseServiceObject548* service) noexcept;
preloader_l::config::ValueView model_service_setting(
    const model_abi::ReleaseServiceObject548* service,
    const char* key) noexcept;
uint32_t model_service_present_sections(
    const model_abi::ReleaseServiceObject548* service) noexcept;

preloader_l::config::ValueView model_service_string_view(
    const model_abi::MsvcString32& value) noexcept;
bool assign_model_service_string(
    model_abi::MsvcString32* destination,
    preloader_l::config::ValueView source) noexcept;
void release_model_service_string(
    model_abi::MsvcString32* value) noexcept;

} }
