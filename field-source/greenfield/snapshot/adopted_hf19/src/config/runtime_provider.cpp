#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
namespace preloader_l { namespace config {
namespace {
struct State { ValueView online; ValueView language; } g{missing_value(),missing_value()};
const char* key_name(SettingKey key) noexcept {
    switch(key){
    case SettingKey::online: return "Online";
    case SettingKey::launch_offline_mode: return nullptr;
    case SettingKey::language: return "Language";
    case SettingKey::environment: return "Environment";
    case SettingKey::telemetry_enabled: return "IsTelemetryEnabled";
    case SettingKey::igo_available: return "IGOAvailable";
    case SettingKey::igo_enabled: return "IGOEnabled";
    }
    return nullptr;
}
bool lookup(const void*,SettingKey key,ValueView* out) noexcept {
    if(!out) return false;
    *out=missing_value();
    if(key==SettingKey::online && g.online.present){*out=g.online;return true;}
    if(key==SettingKey::language && g.language.present){*out=g.language;return true;}
    const char* name=key_name(key);
    if(!name) return false;
    const auto v=runtime_configuration_setting(name);
    if(v.present){*out=v;return true;}
    return false;
}
const SettingsProvider provider{nullptr,lookup};
}
const SettingsProvider& runtime_settings_provider() noexcept{return provider;}
void set_runtime_online_override(ValueView v) noexcept{g.online=v;}
void set_runtime_language_override(ValueView v) noexcept{g.language=v;}
void clear_runtime_overrides() noexcept{g.online=missing_value();g.language=missing_value();}
} }
