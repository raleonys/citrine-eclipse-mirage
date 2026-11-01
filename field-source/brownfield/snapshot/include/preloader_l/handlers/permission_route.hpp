#pragma once

#include <stddef.h>

#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

HandlerStatus handle_check_permission(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} } // namespace preloader_l::handlers
