// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>

namespace redfield { namespace config {

struct ValueView {
    const char* data;
    size_t size;
    bool present;
};

struct SettingsSnapshot {
    ValueView online;
    ValueView launch_offline_mode;
    ValueView language;
    ValueView environment;
    ValueView telemetry_enabled;
    ValueView igo_available;
    ValueView igo_enabled;
};

constexpr ValueView missing_value() noexcept { return ValueView{nullptr, 0, false}; }
ValueView literal_value(const char* text) noexcept;
bool value_equals(ValueView value, const char* literal) noexcept;

// Normalized Online conversion.
// Exact "false" selects offline; exact "true" selects online. Missing or any
// other token follows the inherited default-enabled policy and is not an alias.
bool online_enabled(ValueView online) noexcept;

// Generic behavioral-style Config2 boolean conversion used by unrelated consumers:
// absent or empty -> false; true/1 -> true; false/0 -> false; unknown
// non-empty -> caller-supplied default (startup passes true).
bool behavioral_config_boolean(ValueView value, bool unrecognized_default=true) noexcept;

// Conservative helpers used by query-only formatters. Unlike Online, missing
// values do not acquire an invented true/default value.
bool explicit_true(ValueView value) noexcept;
bool explicit_false(ValueView value) noexcept;

SettingsSnapshot empty_settings() noexcept;

} }
