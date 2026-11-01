#pragma once
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/runtime/local_social_state.hpp"
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
namespace preloader_l { namespace handlers {
HandlerStatus render_local_social_list(const xml::RequestView&,
    router::RouteId,const runtime::LocalSocialSnapshot&,char*,size_t,size_t*) noexcept;
} }
