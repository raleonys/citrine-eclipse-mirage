// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/hook_configuration.hpp"
#include "preloader_l/config_snapshot_view.hpp"
#include <cstring>
namespace {
std::atomic<std::shared_ptr<const preloader_l::ConfigSnapshot>> configuration;
}
namespace preloader_l {
bool bind_hook_configuration(std::shared_ptr<const ConfigSnapshot> snapshot) noexcept {
    if (!snapshot) return false;
    std::shared_ptr<const ConfigSnapshot> empty;
    return configuration.compare_exchange_strong(empty,std::move(snapshot));
}
void release_hook_configuration() noexcept { configuration.store(nullptr); }
preloader_l::config::ValueView hook_setting(const char* key) noexcept {
    using namespace preloader_l::config;
    if (!key) return missing_value();
    const auto snapshot=configuration.load();
    if (!snapshot) return missing_value();
    // WindowsHooks retains this generation until every hook gate drains.
    return ConfigSnapshotView{*snapshot}.find("Game",key);
}
}
