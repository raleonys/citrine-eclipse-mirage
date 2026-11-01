#pragma once
#include <stddef.h>
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus format_get_internet_connected_state(
    const preloader_l::xml::RequestView& request,
    bool connected,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

HandlerStatus handle_get_internet_connected_state(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept;
} }
