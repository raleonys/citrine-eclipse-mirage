#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/hash/fnv_hash.hpp"

#include <cstring>

using namespace preloader_l;

int main()
{
    const router::RouteDescriptor* routes = router::route_inventory();
    if (!routes) return 1;

    const char* expected_names[router::route_count] = {
        "GetSetting", "ShowIGO", "IsProgressiveInstallationAvailable", "GetAllGameInfo",
        "QueryEntitlements", "GetProfile", "QueryOffers", "GetSettings", "GetConfig",
        "StartDownload", "GetPresence", "GetGameInfo", "GrantAchievement",
        "CheckPermission", "QueryAchievements", "GetInternetConnectedState", "QueryFriends",
        "SetPresence", "QueryImage", "GetAuthCode", "QueryContent"
    };
    const uint32_t expected_rvas[router::route_count] = {
        0x2f0e4u,0x3500cu,0x29990u,0x28380u,0x27eadu,0x26c50u,0x263f5u,0x2e22bu,
        0x2ad5fu,0x3500cu,0x29f2bu,0x32cf4u,0x323dcu,0x31c7cu,0x31679u,
        0x30e62u,0x3018du,0x3500cu,0x3462bu,0x33d37u,0x3386eu
    };

    const router::RouteId expected_ids[router::route_count] = {
        router::RouteId::get_setting, router::RouteId::show_igo,
        router::RouteId::is_progressive_installation_available, router::RouteId::get_all_game_info,
        router::RouteId::query_entitlements, router::RouteId::get_profile,
        router::RouteId::query_offers, router::RouteId::get_settings, router::RouteId::get_config,
        router::RouteId::start_download, router::RouteId::get_presence, router::RouteId::get_game_info,
        router::RouteId::grant_achievement, router::RouteId::check_permission,
        router::RouteId::query_achievements, router::RouteId::get_internet_connected_state,
        router::RouteId::query_friends, router::RouteId::set_presence,
        router::RouteId::query_image, router::RouteId::get_auth_code, router::RouteId::query_content
    };
    for (size_t i = 0; i < router::route_count; ++i) {
        const size_t n = std::strlen(expected_names[i]);
        if (routes[i].name_length != n || std::memcmp(routes[i].name, expected_names[i], n) != 0) return 2;
        if (routes[i].original_handler_rva != expected_rvas[i]) return 3;
        const uint64_t computed = hash::fnv1a64_signed_bytes(reinterpret_cast<const uint8_t*>(routes[i].name), routes[i].name_length);
        if (computed != routes[i].hash) return 4;
        if (routes[i].route_id != static_cast<uint16_t>(expected_ids[i])) return 5;
        const xml::TextSpan command{routes[i].name, routes[i].name_length};
        if (router::find_known_route(command) != routes + i) return 6;
    }

    // Three commands intentionally converge on the same curio handler RVA.
    if (routes[1].original_handler_rva != 0x3500cu ||
        routes[9].original_handler_rva != 0x3500cu ||
        routes[17].original_handler_rva != 0x3500cu) return 7;

    // HF08 W01: separate route 15, with the same render/event behavior as ShowIGO.
    const xml::TextSpan alias{"ShowIGOWindow", sizeof("ShowIGOWindow") - 1};
    if (hash::fnv1a64_signed_bytes(reinterpret_cast<const uint8_t*>(alias.data), alias.size) != UINT64_C(0xc96101f996a3e86d)) return 8;
    if (router::find_route(alias, routes, router::route_count) != nullptr) return 9;
    const auto* alias_route = router::find_known_route(alias);
    if (!alias_route || alias_route == routes + 1) return 10;
    if (router::descriptor_route_id(*alias_route) != router::RouteId::show_igo_window || alias_route->original_handler_rva != 0u) return 11;

    const auto* compat = router::compatibility_route_inventory();
    if (!compat) return 12;
    const char* compat_names[router::compatibility_route_count] = {
        "GetPresenceVisibility", "AreChunksInstalled", "RestartGame", "GetVoipStatus", "QueryChunkStatus", "GetBlockList"
    };
    const uint64_t compat_hashes[router::compatibility_route_count] = {
        UINT64_C(0xf21c42469f19cfa6), UINT64_C(0xdfae2d60bb09340b), UINT64_C(0xfe55c0b14d39b446),
        UINT64_C(0x627425d75cc9b05f), UINT64_C(0x7edda9d82a5429b2), UINT64_C(0xc1644021c24b1ff0)
    };
    const router::RouteId compat_ids[router::compatibility_route_count] = {
        router::RouteId::get_presence_visibility, router::RouteId::are_chunks_installed,
        router::RouteId::restart_game, router::RouteId::get_voip_status,
        router::RouteId::query_chunk_status, router::RouteId::get_block_list
    };
    for (size_t i = 0; i < router::compatibility_route_count; ++i) {
        const size_t n = std::strlen(compat_names[i]);
        if (compat[i].name_length != n || std::memcmp(compat[i].name, compat_names[i], n) != 0) return 13;
        if (compat[i].hash != compat_hashes[i] || compat[i].original_handler_rva != 0u) return 14;
        if (compat[i].route_id != static_cast<uint16_t>(compat_ids[i])) return 15;
        const xml::TextSpan command{compat[i].name, compat[i].name_length};
        if (router::find_route(command, routes, router::route_count) != nullptr) return 16;
        if (router::find_known_route(command) != compat + i) return 17;
    }

    // HF08 independently authorized LOCAL_ONLY routes have explicit numbered descriptors.
    // This does not assert remote social success or native callback acceptance.
    const char* local_social[] = {"RequestFriend", "RemoveFriend", "BlockUser", "UnblockUser", "SendInvite"};
    for (const char* name : local_social) {
        const xml::TextSpan command{name, std::strlen(name)};
        if (router::find_known_route(command) == nullptr) return 18;
    }

    const xml::TextSpan unknown{"DefinitelyUnknown", sizeof("DefinitelyUnknown") - 1};
    if (router::find_known_route(unknown) != nullptr) return 19;

    return 0;
}
