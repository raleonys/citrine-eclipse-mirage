// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

// Common config-backed query boundary for the three settings-related routes.
// GetSetting's selector is explicit until the binary's request-side field is
// proven; callers pass `has_get_setting_key=false` to fail closed otherwise.
HandlerStatus handle_settings_query(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsProvider& provider,
    bool has_get_setting_key,
    preloader_l::config::SettingKey get_setting_key,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
