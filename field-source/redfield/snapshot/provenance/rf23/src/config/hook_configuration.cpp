// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/hook_configuration.hpp"
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
    // HF19 runtime_configuration_parser reserves hook settings to Game.
    // The immutable publication remains globally retained after this lookup.
    const char* section="Game";
    for (auto i=snapshot->leaves.rbegin();i!=snapshot->leaves.rend();++i)
        if (i->path.size()==1 && i->path[0]==section && i->key==key)
            return {i->raw_value.data(),i->raw_value.size(),true};
    return missing_value();
}
}
