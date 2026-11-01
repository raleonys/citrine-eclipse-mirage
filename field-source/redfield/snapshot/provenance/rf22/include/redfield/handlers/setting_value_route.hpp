// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "redfield/config/settings_provider.hpp"
#include "redfield/handlers/handler_status.hpp"
#include "redfield/xml/request_parser.hpp"

namespace redfield { namespace handlers {

// Contextual GetSetting implementation. `key` is explicit because the exact
// request-side selector field has not yet been proven from the binary; this
// keeps response/config behavior usable without guessing protocol semantics.
HandlerStatus handle_get_setting(
    const redfield::xml::RequestView& request,
    redfield::config::SettingKey key,
    const redfield::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus format_get_setting_response(
    const redfield::xml::RequestView& request,
    redfield::config::ValueView setting,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
