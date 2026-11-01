#pragma once
#include <stddef.h>
#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace handlers {

using HandlerFn = HandlerStatus (*)(const preloader_l::xml::RequestView&, char*, size_t, size_t*) noexcept;

// Returns a clean handler boundary for routes reconstructed so far. A bound
// handler may still return HandlerStatus::unsupported when the original
// behavior would require external authorization, account data, side effects,
// or fabricated remote-service success.
HandlerFn find_bound_handler(preloader_l::router::RouteId route) noexcept;
bool route_has_handler_boundary(preloader_l::router::RouteId route) noexcept;

} }
