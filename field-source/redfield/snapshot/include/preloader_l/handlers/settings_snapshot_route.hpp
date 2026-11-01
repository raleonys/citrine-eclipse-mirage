// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

HandlerStatus handle_get_settings(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus format_get_settings_response(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsSnapshot& settings,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} }
