#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_get_utc_time(const xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
