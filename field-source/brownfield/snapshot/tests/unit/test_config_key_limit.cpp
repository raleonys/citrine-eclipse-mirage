#include "preloader_l/config/runtime_configuration.hpp"
#include <cstring>
#include <string>

int main() {
    using namespace preloader_l::config;
    clear_runtime_configuration();

    const std::string key(512u, 'K');
    const std::string value(384u, 'V');
    const std::string cfg =
        std::string("\"Config2\" { \"User\" { \"") + key + "\" \"" + value + "\" } }";

    if (set_runtime_configuration_cfg(cfg.data(), cfg.size()) != RuntimeConfigurationStatus::ok)
        return 1;
    const auto actual = runtime_configuration_setting(key.c_str());
    if (!actual.present || actual.size != value.size()) return 2;
    if (std::memcmp(actual.data, value.data(), value.size()) != 0) return 3;

    // Ensure the historical aliases are still exact while arbitrary leaf keys
    // no longer pass through a fixed 128-byte canonicalization scratch buffer.
    const char alias_cfg[] = "\"Config2\" { \"Game\" { \"Name\" \"LongKeyGame\" \"Version\" \"8.0\" } }";
    if (set_runtime_configuration_cfg(alias_cfg, sizeof(alias_cfg)-1u) != RuntimeConfigurationStatus::ok)
        return 4;
    const auto name = runtime_configuration_setting("GameName");
    const auto version = runtime_configuration_setting("GameVersion");
    if (!name.present || name.size != 11u || std::memcmp(name.data, "LongKeyGame", 11u) != 0)
        return 5;
    if (!version.present || version.size != 3u || std::memcmp(version.data, "8.0", 3u) != 0)
        return 6;

    clear_runtime_configuration();
    return 0;
}
