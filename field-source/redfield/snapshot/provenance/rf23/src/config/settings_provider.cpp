// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config/settings_provider.hpp"

namespace redfield { namespace config {

bool lookup_setting(const SettingsProvider& provider, SettingKey key, ValueView* value) noexcept {
    if (!value) return false;
    *value = missing_value();
    if (!provider.lookup) return false;
    ValueView candidate = missing_value();
    if (!provider.lookup(provider.context, key, &candidate)) return false;
    if (!candidate.present || (!candidate.data && candidate.size != 0)) return false;
    *value = candidate;
    return true;
}

SettingsSnapshot resolve_settings(const SettingsProvider& provider) noexcept {
    SettingsSnapshot out = empty_settings();
    (void)lookup_setting(provider, SettingKey::online, &out.online);
    (void)lookup_setting(provider, SettingKey::launch_offline_mode, &out.launch_offline_mode);
    (void)lookup_setting(provider, SettingKey::language, &out.language);
    (void)lookup_setting(provider, SettingKey::environment, &out.environment);
    (void)lookup_setting(provider, SettingKey::telemetry_enabled, &out.telemetry_enabled);
    (void)lookup_setting(provider, SettingKey::igo_available, &out.igo_available);
    (void)lookup_setting(provider, SettingKey::igo_enabled, &out.igo_enabled);
    return out;
}

const char* setting_key_name(SettingKey key) noexcept {
    switch (key) {
    case SettingKey::online: return "Online";
    case SettingKey::launch_offline_mode: return "EALaunchOfflineMode";
    case SettingKey::language: return "Language";
    case SettingKey::environment: return "Environment";
    case SettingKey::telemetry_enabled: return "IsTelemetryEnabled";
    case SettingKey::igo_available: return "IGOAvailable";
    case SettingKey::igo_enabled: return "IGOEnabled";
    }
    return nullptr;
}

} }
