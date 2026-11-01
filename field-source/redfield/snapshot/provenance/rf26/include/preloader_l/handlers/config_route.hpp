// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_get_config(const preloader_l::xml::RequestView& request, char* output, size_t capacity, size_t* written) noexcept;
} }
