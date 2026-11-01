// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "redfield/config/settings_provider.hpp"
#include "redfield/handlers/handler_status.hpp"
#include "redfield/xml/request_parser.hpp"
namespace redfield { namespace handlers {
HandlerStatus format_get_internet_connected_state(
    const redfield::xml::RequestView& request,
    bool connected,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus handle_get_internet_connected_state(
    const redfield::xml::RequestView& request,
    const redfield::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept;
} }
