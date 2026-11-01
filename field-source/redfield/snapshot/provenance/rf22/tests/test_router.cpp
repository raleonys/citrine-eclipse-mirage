// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/router/route_lookup.hpp"
#include "redfield/hash/fnv_hash.hpp"

#include <cstring>

using namespace redfield;

static xml::TextSpan s(const char* p) { return {p, std::strlen(p)}; }

int main()
{
    constexpr char a[] = "GetProfile";
    constexpr char b[] = "QueryOffers";
    const router::RouteDescriptor routes[] = {
        {hash::fnv1a64_offset_basis, 0x111u, 1u, "", 0u}, // not selected for non-empty input
        {UINT64_C(0x27470f1cda405048), 0x26c50u, 2u, a, sizeof(a) - 1},
        {UINT64_C(0x28415d10fa3aadac), 0x263f5u, 3u, b, sizeof(b) - 1},
    };

    const auto* r = router::find_route(s("GetProfile"), routes, 3);
    if (!r || r->route_id != 2 || r->original_handler_rva != 0x26c50u) return 1;
    r = router::find_route(s("QueryOffers"), routes, 3);
    if (!r || r->route_id != 3) return 2;
    if (router::find_route(s("Unknown"), routes, 3) != nullptr) return 3;

    const xml::TextSpan empty{nullptr, 0};
    r = router::find_route(empty, routes, 3);
    if (!r || r->route_id != 1) return 4;

    // A descriptor with the right hash but the wrong spelling must not win.
    const router::RouteDescriptor collision[] = {
        {UINT64_C(0x27470f1cda405048), 0x999u, 9u, "NotProfile", sizeof("NotProfile") - 1},
        {UINT64_C(0x27470f1cda405048), 0x26c50u, 2u, a, sizeof(a) - 1},
    };
    r = router::find_route(s("GetProfile"), collision, 2);
    if (!r || r->route_id != 2) return 5;

    const xml::TextSpan invalid{nullptr, 1};
    if (router::find_route(invalid, routes, 3) != nullptr) return 6;
    if (router::find_route(s("GetProfile"), nullptr, 1) != nullptr) return 7;

    return 0;
}
