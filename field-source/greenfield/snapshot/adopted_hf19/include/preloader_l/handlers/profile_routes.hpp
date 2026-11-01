#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_get_profile(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_get_presence(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
