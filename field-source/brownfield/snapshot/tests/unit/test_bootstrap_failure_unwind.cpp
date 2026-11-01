#include "preloader_l/platform/environment_policy.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace preloader_l;

namespace {
struct BackendState {
    bool offline_present = false;
    bool language_present = false;
    bool fail_language_set = false;
    unsigned offline_sets = 0;
    unsigned language_sets = 0;
    unsigned offline_deletes = 0;
    unsigned language_deletes = 0;
};

uint32_t length_cb(const char* name, void* opaque) noexcept {
    auto* s = static_cast<BackendState*>(opaque);
    if (std::strcmp(name, "EALaunchOfflineMode") == 0)
        return s->offline_present ? 2u : 0u;
    if (std::strcmp(name, "EAGameLocale") == 0)
        return s->language_present ? 6u : 0u;
    return 0u;
}

bool set_cb(const char* name, const char* value, void* opaque) noexcept {
    auto* s = static_cast<BackendState*>(opaque);
    if (std::strcmp(name, "EALaunchOfflineMode") == 0) {
        if (value) {
            ++s->offline_sets;
            s->offline_present = true;
        } else {
            ++s->offline_deletes;
            s->offline_present = false;
        }
        return true;
    }
    if (std::strcmp(name, "EAGameLocale") == 0) {
        if (value) {
            ++s->language_sets;
            if (s->fail_language_set) return false;
            s->language_present = true;
        } else {
            ++s->language_deletes;
            s->language_present = false;
        }
        return true;
    }
    return false;
}

bool lookup_cb(const void*, config::SettingKey key, config::ValueView* out) noexcept {
    if (!out) return false;
    switch (key) {
    case config::SettingKey::online:
        *out = config::literal_value("false");
        return true;
    case config::SettingKey::language:
        *out = config::literal_value("tr_TR");
        return true;
    default:
        *out = config::missing_value();
        return false;
    }
}
}

int main() {
    BackendState state{};
    state.fail_language_set = true;
    const config::SettingsProvider settings{nullptr, &lookup_cb};
    const platform::EnvironmentBackend backend{&length_cb, &set_cb, &state};

    uint32_t written = 0;
    uint32_t preserved = 0;
    const auto status = platform::apply_environment(settings, &backend, &written, &preserved);
    if (status != platform::EnvironmentStatus::set_failed) return 1;
    if (written != 0x1u || preserved != 0u) return 2;
    if (!state.offline_present || state.language_present) return 3;
    if (state.offline_sets != 1u || state.language_sets != 1u) return 4;

    uint32_t restored_mask = 0;
    const auto restore = platform::restore_environment(&backend, written, &restored_mask);
    if (restore != platform::EnvironmentStatus::ok) return 5;
    if (restored_mask != written) return 6;
    if (state.offline_present || state.language_present) return 7;
    if (state.offline_deletes != 1u || state.language_deletes != 0u) return 8;
    return 0;
}
