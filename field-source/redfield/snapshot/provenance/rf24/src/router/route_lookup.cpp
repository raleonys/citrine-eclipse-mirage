// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/router/route_lookup.hpp"
#include "redfield/hash/fnv_hash.hpp"

namespace redfield { namespace router {
namespace {
bool exact_name(redfield::xml::TextSpan command, const RouteDescriptor& route) noexcept
{
    if (command.size != route.name_length || (!route.name && route.name_length != 0)) return false;
    for (size_t i = 0; i < command.size; ++i) {
        if (command.data[i] != route.name[i]) return false;
    }
    return true;
}
} // namespace

const RouteDescriptor* find_route(
    redfield::xml::TextSpan command,
    const RouteDescriptor* routes,
    size_t route_count) noexcept
{
    if ((!command.data && command.size != 0) || (!routes && route_count != 0)) return nullptr;
    const uint64_t hash = redfield::hash::fnv1a64_signed_bytes(
        reinterpret_cast<const uint8_t*>(command.data), command.size);

    for (size_t i = 0; i < route_count; ++i) {
        if (routes[i].hash == hash && exact_name(command, routes[i])) return routes + i;
    }
    return nullptr;
}

} } // namespace redfield::router
