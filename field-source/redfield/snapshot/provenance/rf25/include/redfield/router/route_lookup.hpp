// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "redfield/xml/request_parser.hpp"

namespace redfield { namespace router {

struct RouteDescriptor {
    uint64_t hash;
    uint32_t original_handler_rva;
    uint16_t route_id;
    const char* name;
    size_t name_length;
};

// Hash-first lookup matching the curio dispatcher primitive, followed by an
// exact name comparison so a clean build does not dispatch an FNV collision.
const RouteDescriptor* find_route(
    redfield::xml::TextSpan command,
    const RouteDescriptor* routes,
    size_t route_count) noexcept;

} } // namespace redfield::router
