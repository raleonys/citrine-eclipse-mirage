#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus handle_query_content(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_query_entitlements(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_query_offers(const preloader_l::xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_select_store(const xml::RequestView&,char*,size_t,size_t*) noexcept;
HandlerStatus handle_get_wallet_balance(const xml::RequestView&,char*,size_t,size_t*) noexcept;
} }
