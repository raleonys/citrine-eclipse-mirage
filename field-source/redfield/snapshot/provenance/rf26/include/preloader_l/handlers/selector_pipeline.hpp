// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

struct SelectorResolution {
    preloader_l::config::ValueView value;
    bool recognized;
    bool used_fallback;
};

// Clean-source semantic equivalents of the curio GetSetting/GetGameInfo
// selector -> lookup -> optional materialization chain. These functions do not
// emulate private MSVC std::string object layout; they preserve the externally
// observed values/defaults from the frozen research contract. GetGameInfo
// uses the complete 14-key Phase x001 map; true misses remain strict.
SelectorResolution resolve_get_setting_selector(preloader_l::xml::TextSpan selector) noexcept;


struct RuntimeSettingsPolicy {
    preloader_l::config::ValueView manual_offline;
    preloader_l::config::ValueView environment;
    preloader_l::config::ValueView telemetry_enabled;
    preloader_l::config::ValueView igo_available;
    preloader_l::config::ValueView igo_enabled;
    preloader_l::config::ValueView language;
};

// Shared normalized policy used by both GetSetting and GetSettings.
// Manual-offline is derived only from normalized Emulator.Online. Unrelated
// route-specific booleans retain their existing explicit-key behavior.
RuntimeSettingsPolicy resolve_runtime_settings_policy() noexcept;
SelectorResolution resolve_get_game_info_selector(preloader_l::xml::TextSpan selector) noexcept;

} }
