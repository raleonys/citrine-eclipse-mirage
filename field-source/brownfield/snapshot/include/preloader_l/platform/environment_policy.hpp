#pragma once
#include <stdint.h>
#include "preloader_l/config/settings_provider.hpp"

namespace preloader_l { namespace platform {

constexpr uint32_t environment_length_query_error = UINT32_C(0xffffffff);

struct EnvironmentBackend {
    // 0 means absent; nonzero means present (including oversized values);
    // environment_length_query_error means the backend could not classify it.
    uint32_t (*length)(const char* name, void* context) noexcept;
    bool (*set)(const char* name, const char* value, void* context) noexcept;
    void* context;
};

enum class EnvironmentStatus : uint8_t {
    ok = 0,
    invalid_argument,
    backend_required,
    set_failed,
    query_failed,
};

// bit0 EALaunchOfflineMode, bit1 EAGameLocale.
EnvironmentStatus apply_environment(const preloader_l::config::SettingsProvider& settings,
                                    const EnvironmentBackend* backend,
                                    uint32_t* written_mask,
                                    uint32_t* preserved_mask) noexcept;

// Removes only variables that apply_environment actually created (written_mask).
// A pre-existing/preserved host value is never deleted by this rollback.
EnvironmentStatus restore_environment(const EnvironmentBackend* backend,
                                      uint32_t written_mask,
                                      uint32_t* restored_mask) noexcept;

} }
