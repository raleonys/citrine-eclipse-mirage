// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config/settings_values.hpp"

namespace redfield { namespace config {
namespace {
size_t cstr_size(const char* text) noexcept {
    if (!text) return 0;
    size_t n = 0;
    while (text[n] != '\0') ++n;
    return n;
}
}

ValueView literal_value(const char* text) noexcept {
    if (!text) return missing_value();
    return ValueView{text, cstr_size(text), true};
}

bool value_equals(ValueView value, const char* literal) noexcept {
    if (!value.present || !value.data || !literal) return false;
    const size_t n = cstr_size(literal);
    if (value.size != n) return false;
    for (size_t i = 0; i < n; ++i) {
        if (value.data[i] != literal[i]) return false;
    }
    return true;
}

bool behavioral_config_boolean(ValueView value, bool unrecognized_default) noexcept {
    if (!value.present || !value.data || value.size == 0) return false;
    if (value_equals(value, "true") || value_equals(value, "1")) return true;
    if (value_equals(value, "false") || value_equals(value, "0")) return false;
    return unrecognized_default;
}

bool online_enabled(ValueView online) noexcept {
    if (!online.present || !online.data) return true;
    if (value_equals(online, "false")) return false;
    if (value_equals(online, "true")) return true;
    // 0/1 and all other tokens are deliberately not Online aliases.
    return true;
}

bool explicit_true(ValueView value) noexcept {
    return value_equals(value, "1") || value_equals(value, "true");
}

bool explicit_false(ValueView value) noexcept {
    return value_equals(value, "0") || value_equals(value, "false");
}

SettingsSnapshot empty_settings() noexcept {
    const auto missing = missing_value();
    return SettingsSnapshot{missing, missing, missing, missing, missing, missing, missing};
}

} }
