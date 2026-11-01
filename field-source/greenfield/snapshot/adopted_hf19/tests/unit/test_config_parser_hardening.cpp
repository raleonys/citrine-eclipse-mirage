#include "preloader_l/config/runtime_configuration.hpp"
#include <cstring>

using namespace preloader_l::config;

namespace {
bool eq(ValueView value, const char* expected) {
    const size_t n = std::strlen(expected);
    return value.present && value.size == n && std::memcmp(value.data, expected, n) == 0;
}
bool missing(ValueView value) { return !value.present; }
}

int main() {
    clear_runtime_configuration();

    // Direct leaves remain authoritative, but nested collection/object leaves do not.
    constexpr char nested[] = R"CFG("Config2" {
        "User" {
            "Username" "DIRECT"
            "Profile" { "Username" "NESTED" "Country" "XX" }
        }
        "Achievements" {
            "AchievementsSet" "set-a"
            "AchievementNames" { "1" "Winners!" }
        }
    })CFG";
    if (set_runtime_configuration_cfg(nested, sizeof(nested) - 1u) != RuntimeConfigurationStatus::ok) return 1;
    if (!eq(runtime_configuration_setting("Username"), "DIRECT")) return 2;
    if (!eq(runtime_configuration_setting("AchievementsSet"), "set-a")) return 3;
    if (!missing(runtime_configuration_setting("Country"))) return 4;
    if (!missing(runtime_configuration_setting("1"))) return 5;

    // A nested reserved key cannot overwrite the direct top-level-section policy.
    constexpr char nested_poison[] = R"CFG("Config2" {
        "Emulator" { "Online" "true" "Nested" { "Online" "false" } }
    })CFG";
    if (set_runtime_configuration_cfg(nested_poison, sizeof(nested_poison) - 1u) != RuntimeConfigurationStatus::ok) return 6;
    if (!eq(runtime_configuration_setting("Online"), "true")) return 7;

    // Unterminated block comments are lexical errors and must not commit candidate state.
    constexpr char stable[] = R"CFG("Config2" { "User" { "Username" "STABLE" } })CFG";
    if (set_runtime_configuration_cfg(stable, sizeof(stable) - 1u) != RuntimeConfigurationStatus::ok) return 8;
    constexpr char unterminated[] = R"CFG("Config2" { "User" { "Username" "BROKEN" } } /* trailing)CFG";
    if (set_runtime_configuration_cfg(unterminated, sizeof(unterminated) - 1u) != RuntimeConfigurationStatus::invalid_cfg) return 9;
    if (!eq(runtime_configuration_setting("Username"), "STABLE")) return 10;

    // Properly terminated comments remain accepted.
    constexpr char terminated[] = R"CFG("Config2" { /* ok */ "User" { "Username" "VALID" } } /* tail */)CFG";
    if (set_runtime_configuration_cfg(terminated, sizeof(terminated) - 1u) != RuntimeConfigurationStatus::ok) return 11;
    if (!eq(runtime_configuration_setting("Username"), "VALID")) return 12;

    clear_runtime_configuration();
    return 0;
}
