#pragma once
#include <stddef.h>
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

// Contextual GetSetting implementation. `key` is explicit because the exact
// request-side selector field has not yet been proven from the binary; this
// keeps response/config behavior usable without guessing protocol semantics.
HandlerStatus handle_get_setting(
    const preloader_l::xml::RequestView& request,
    preloader_l::config::SettingKey key,
    const preloader_l::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus format_get_setting_response(
    const preloader_l::xml::RequestView& request,
    preloader_l::config::ValueView setting,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
