// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/config/settings_values.hpp"
namespace preloader_l {
// One immutable generation stays pinned until every hook gate is drained.
bool bind_hook_configuration(std::shared_ptr<const ConfigSnapshot>) noexcept;
void release_hook_configuration() noexcept;
preloader_l::config::ValueView hook_setting(const char*) noexcept;
}
