// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config/settings_provider.hpp"
using namespace redfield::config;
namespace {
struct Fixture { const char* online; const char* language; };
bool lookup(const void* p, SettingKey key, ValueView* out) noexcept {
    const auto* f = static_cast<const Fixture*>(p);
    if (!out || !f) return false;
    if (key == SettingKey::online) { *out = literal_value(f->online); return true; }
    if (key == SettingKey::language) { *out = literal_value(f->language); return true; }
    return false;
}
}
int main() {
    Fixture f{"0","en_US"}; SettingsProvider p{&f,&lookup};
    auto s=resolve_settings(p);
    if (!s.online.present || !value_equals(s.online,"0")) return 1;
    if (!s.language.present || !value_equals(s.language,"en_US")) return 2;
    if (s.environment.present) return 3;
    ValueView v{}; if (lookup_setting(p,SettingKey::environment,&v)) return 4;
    if (v.present) return 5;
    if (!setting_key_name(SettingKey::launch_offline_mode)) return 6;
    SettingsProvider empty{nullptr,nullptr}; s=resolve_settings(empty);
    // Missing Online keeps the documented compatibility default: enabled.
    if (s.online.present || !online_enabled(s.online)) return 7;
    return 0;
}
