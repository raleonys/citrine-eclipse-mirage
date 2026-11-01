#pragma once
#include "preloader_l/config/settings_provider.hpp"
namespace preloader_l { namespace config {
// Connectivity policy.
//
// SettingKey::online is backed directly by Config2 Emulator.Online.
// Only exact text values "false" and "true" are semantic Online tokens.
// No secondary compatibility flag or authentication latch participates in
// the normalized connectivity decision. SettingKey::launch_offline_mode is
// intentionally not backed by Config2: manual-offline is derived from Online.
const SettingsProvider& runtime_settings_provider() noexcept;
void set_runtime_online_override(ValueView value) noexcept;
// Process command-line language policy. A present value overrides Config2
// Game.Language for every shared runtime-settings consumer; missing retracts it.
void set_runtime_language_override(ValueView value) noexcept;
void clear_runtime_overrides() noexcept;
} }
