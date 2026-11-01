// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include "redfield/handlers/handler_status.hpp"
#include "redfield/xml/request_parser.hpp"
namespace redfield { namespace handlers {
HandlerStatus handle_get_utc_time(const xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
