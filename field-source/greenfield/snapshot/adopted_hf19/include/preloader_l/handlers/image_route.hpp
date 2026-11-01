#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_query_image(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
