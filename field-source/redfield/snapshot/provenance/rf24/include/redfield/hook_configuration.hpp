// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "redfield/config/settings_values.hpp"
namespace redfield {
// One immutable generation stays pinned until every hook gate is drained.
bool bind_hook_configuration(std::shared_ptr<const ConfigSnapshot>) noexcept;
void release_hook_configuration() noexcept;
redfield::config::ValueView hook_setting(const char*) noexcept;
}
