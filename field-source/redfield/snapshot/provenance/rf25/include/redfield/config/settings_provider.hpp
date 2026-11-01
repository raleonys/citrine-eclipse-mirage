// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>
#include "redfield/config/settings_values.hpp"

namespace redfield { namespace config {

enum class SettingKey : uint8_t {
    online = 0,
    launch_offline_mode,
    language,
    environment,
    telemetry_enabled,
    igo_available,
    igo_enabled,
};

using LookupSettingFn = bool (*)(const void* context, SettingKey key, ValueView* value) noexcept;

struct SettingsProvider {
    const void* context;
    LookupSettingFn lookup;
};

bool lookup_setting(const SettingsProvider& provider, SettingKey key, ValueView* value) noexcept;
SettingsSnapshot resolve_settings(const SettingsProvider& provider) noexcept;
const char* setting_key_name(SettingKey key) noexcept;

} }
