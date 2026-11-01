// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config/settings_values.hpp"
#include "redfield/xml/request_parser.hpp"

namespace redfield { namespace handlers {

struct SelectorResolution {
    redfield::config::ValueView value;
    bool recognized;
    bool used_fallback;
};

// Clean-source semantic equivalents of the curio GetSetting/GetGameInfo
// selector -> lookup -> optional materialization chain. These functions do not
// emulate private MSVC std::string object layout; they preserve the externally
// observed values/defaults from the frozen research contract. GetGameInfo
// uses the complete 14-key Phase x001 map; true misses remain strict.
SelectorResolution resolve_get_setting_selector(redfield::xml::TextSpan selector) noexcept;


struct RuntimeSettingsPolicy {
    redfield::config::ValueView manual_offline;
    redfield::config::ValueView environment;
    redfield::config::ValueView telemetry_enabled;
    redfield::config::ValueView igo_available;
    redfield::config::ValueView igo_enabled;
    redfield::config::ValueView language;
};

// Shared normalized policy used by both GetSetting and GetSettings.
// Manual-offline is derived only from normalized Emulator.Online. Unrelated
// route-specific booleans retain their existing explicit-key behavior.
RuntimeSettingsPolicy resolve_runtime_settings_policy() noexcept;
SelectorResolution resolve_get_game_info_selector(redfield::xml::TextSpan selector) noexcept;

} }
