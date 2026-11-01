#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/config/settings_values.hpp"
namespace preloader_l { namespace runtime { namespace model_abi { struct ReleaseServiceObject548; } } }
namespace preloader_l { namespace config {
enum class RuntimeConfigurationStatus : uint8_t { ok=0, invalid_argument, invalid_cfg, resource_error };

struct RuntimeConfigurationReadLease {
    uint64_t generation;
    bool active;
};
bool begin_runtime_configuration_read(RuntimeConfigurationReadLease* lease) noexcept;
void end_runtime_configuration_read(RuntimeConfigurationReadLease* lease) noexcept;
uint64_t runtime_configuration_generation() noexcept;
RuntimeConfigurationStatus set_runtime_configuration_cfg(const char* cfg,size_t length) noexcept;
bool clear_runtime_configuration_retryable() noexcept;
void clear_runtime_configuration() noexcept;
// True only when an external Config2 overlay is committed. This is not a
// service-initialization or route-availability gate.
bool runtime_configuration_loaded() noexcept;
// HF18: freeze new Config2 publication before staging hooks. Readers remain permitted.
// Only unfreeze after all executable hook bytes and ticket ownership are retired.
bool freeze_runtime_configuration_for_hooks() noexcept;
void unfreeze_runtime_configuration_after_hooks() noexcept;
ValueView runtime_configuration_setting(const char* key) noexcept;

struct ProfilePolicyView {
    ValueView username;
    ValueView persona_id;
    ValueView user_id;
    ValueView country;
    ValueView currency;
    ValueView is_subscriber;
    ValueView is_steam_subscriber;
    ValueView subscriber_level;
    ValueView is_under_age;
};
ProfilePolicyView runtime_profile_policy() noexcept;
uint32_t runtime_configuration_present_sections() noexcept;
RuntimeConfigurationStatus set_runtime_current_directory(const char* value,size_t length) noexcept;
ValueView runtime_current_directory() noexcept;
#if defined(_WIN32)
bool set_runtime_current_directory_wide_policy(const uint16_t* value,size_t length) noexcept;
void bind_runtime_configuration_model_service(preloader_l::runtime::model_abi::ReleaseServiceObject548* service) noexcept;
void unbind_runtime_configuration_model_service() noexcept;
#endif
} }
