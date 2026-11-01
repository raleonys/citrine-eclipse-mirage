// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/hook_configuration.hpp"
#include "redfield/config_snapshot_view.hpp"
#include <cstring>
namespace {
std::atomic<std::shared_ptr<const redfield::ConfigSnapshot>> configuration;
}
namespace redfield {
bool bind_hook_configuration(std::shared_ptr<const ConfigSnapshot> snapshot) noexcept {
    if (!snapshot) return false;
    std::shared_ptr<const ConfigSnapshot> empty;
    return configuration.compare_exchange_strong(empty,std::move(snapshot));
}
void release_hook_configuration() noexcept { configuration.store(nullptr); }
redfield::config::ValueView hook_setting(const char* key) noexcept {
    using namespace redfield::config;
    if (!key) return missing_value();
    const auto snapshot=configuration.load();
    if (!snapshot) return missing_value();
    // WindowsHooks retains this generation until every hook gate drains.
    return ConfigSnapshotView{*snapshot}.find("Game",key);
}
}
