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

    // Intended policy remains unchanged.
    constexpr char intended[] = R"CFG("Config2" { "Emulator" { "Online" "false" } })CFG";
    if (set_runtime_configuration_cfg(intended, sizeof(intended) - 1u) != RuntimeConfigurationStatus::ok) return 1;
    if (!eq(runtime_configuration_setting("Online"), "false")) return 2;

    // A recognized but wrong section cannot publish a reserved policy key.
    constexpr char wrong_recognized[] = R"CFG("Config2" { "User" { "Username" "RIGHT" "Online" "false" } })CFG";
    if (set_runtime_configuration_cfg(wrong_recognized, sizeof(wrong_recognized) - 1u) != RuntimeConfigurationStatus::ok) return 3;
    if (!eq(runtime_configuration_setting("Username"), "RIGHT")) return 4;
    if (!missing(runtime_configuration_setting("Online"))) return 5;

    // Unknown sections remain parse-compatible but cannot publish settings.
    constexpr char unknown_section[] = R"CFG("Config2" { "Bogus" { "Online" "false" "Username" "WRONG" } })CFG";
    if (set_runtime_configuration_cfg(unknown_section, sizeof(unknown_section) - 1u) != RuntimeConfigurationStatus::ok) return 6;
    if (!missing(runtime_configuration_setting("Online")) || !missing(runtime_configuration_setting("Username"))) return 7;
    if (runtime_configuration_present_sections() != 0u) return 8;

    // Root-level leaves are non-authoritative even though the parser remains
    // compatible with their lexical shape.
    constexpr char root_leaf[] = R"CFG("Config2" { "Online" "false" "Username" "WRONG" })CFG";
    if (set_runtime_configuration_cfg(root_leaf, sizeof(root_leaf) - 1u) != RuntimeConfigurationStatus::ok) return 9;
    if (!missing(runtime_configuration_setting("Online")) || !missing(runtime_configuration_setting("Username"))) return 10;

    // Later wrong-section data cannot overwrite an earlier correct policy.
    constexpr char overwrite[] = R"CFG("Config2" { "Emulator" { "Online" "true" } "Bogus" { "Online" "false" } "User" { "Online" "false" } })CFG";
    if (set_runtime_configuration_cfg(overwrite, sizeof(overwrite) - 1u) != RuntimeConfigurationStatus::ok) return 11;
    if (!eq(runtime_configuration_setting("Online"), "true")) return 12;

    // Alias poisoning is isolated as well: Game.Name owns GameName.
    constexpr char alias_poison[] = R"CFG("Config2" { "Game" { "Name" "RIGHT" "Language" "tr_TR" } "User" { "GameName" "WRONG" "Language" "xx_XX" } })CFG";
    if (set_runtime_configuration_cfg(alias_poison, sizeof(alias_poison) - 1u) != RuntimeConfigurationStatus::ok) return 13;
    if (!eq(runtime_configuration_setting("GameName"), "RIGHT")) return 14;
    if (!eq(runtime_configuration_setting("Language"), "tr_TR")) return 15;

    // HF17 / F1476-001: current-directory mirror belongs to the same runtime
    // configuration generation and must not survive clear/shutdown.
    constexpr char cwd[] = "C:/HF17";
    if (set_runtime_current_directory(cwd, sizeof(cwd) - 1u) != RuntimeConfigurationStatus::ok) return 16;
    if (!eq(runtime_current_directory(), cwd)) return 17;
    clear_runtime_configuration();
    if (!missing(runtime_current_directory())) return 18;
    return 0;
}
