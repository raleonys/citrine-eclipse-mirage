// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "redfield/config/settings_provider.hpp"
#include "redfield/handlers/handler_status.hpp"
#include "redfield/xml/request_parser.hpp"

namespace redfield { namespace handlers {

// Common config-backed query boundary for the three settings-related routes.
// GetSetting's selector is explicit until the binary's request-side field is
// proven; callers pass `has_get_setting_key=false` to fail closed otherwise.
HandlerStatus handle_settings_query(
    const redfield::xml::RequestView& request,
    const redfield::config::SettingsProvider& provider,
    bool has_get_setting_key,
    redfield::config::SettingKey get_setting_key,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
