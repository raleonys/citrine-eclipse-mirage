// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/hash/fnv_hash.hpp"

namespace preloader_l { namespace router {
namespace {
constexpr RouteDescriptor routes[route_count] = {
    {UINT64_C(0xa925a66a5ac37a7d), 0x2f0e4u, static_cast<uint16_t>(RouteId::get_setting), "GetSetting", sizeof("GetSetting") - 1},
    {UINT64_C(0x2ae153ff0a17a875), 0x3500cu, static_cast<uint16_t>(RouteId::show_igo), "ShowIGO", sizeof("ShowIGO") - 1},
    {UINT64_C(0x00f6d5a7471c7cff), 0x29990u, static_cast<uint16_t>(RouteId::is_progressive_installation_available), "IsProgressiveInstallationAvailable", sizeof("IsProgressiveInstallationAvailable") - 1},
    {UINT64_C(0x05fd32103f8332bc), 0x28380u, static_cast<uint16_t>(RouteId::get_all_game_info), "GetAllGameInfo", sizeof("GetAllGameInfo") - 1},
    {UINT64_C(0x0a1fca6299fc2113), 0x27eadu, static_cast<uint16_t>(RouteId::query_entitlements), "QueryEntitlements", sizeof("QueryEntitlements") - 1},
    {UINT64_C(0x27470f1cda405048), 0x26c50u, static_cast<uint16_t>(RouteId::get_profile), "GetProfile", sizeof("GetProfile") - 1},
    {UINT64_C(0x28415d10fa3aadac), 0x263f5u, static_cast<uint16_t>(RouteId::query_offers), "QueryOffers", sizeof("QueryOffers") - 1},
    {UINT64_C(0x2e73d4b83a2865ca), 0x2e22bu, static_cast<uint16_t>(RouteId::get_settings), "GetSettings", sizeof("GetSettings") - 1},
    {UINT64_C(0x5b9d8ac023fbe855), 0x2ad5fu, static_cast<uint16_t>(RouteId::get_config), "GetConfig", sizeof("GetConfig") - 1},
    {UINT64_C(0x854a033ec9f89e8b), 0x3500cu, static_cast<uint16_t>(RouteId::start_download), "StartDownload", sizeof("StartDownload") - 1},
    {UINT64_C(0x933b9f371216d496), 0x29f2bu, static_cast<uint16_t>(RouteId::get_presence), "GetPresence", sizeof("GetPresence") - 1},
    {UINT64_C(0xe03fcf0117d2c54d), 0x32cf4u, static_cast<uint16_t>(RouteId::get_game_info), "GetGameInfo", sizeof("GetGameInfo") - 1},
    {UINT64_C(0xae37f1ac30db5870), 0x323dcu, static_cast<uint16_t>(RouteId::grant_achievement), "GrantAchievement", sizeof("GrantAchievement") - 1},
    {UINT64_C(0xd3dfa32adc5ffd78), 0x31c7cu, static_cast<uint16_t>(RouteId::check_permission), "CheckPermission", sizeof("CheckPermission") - 1},
    {UINT64_C(0xd424e7c76bbcc117), 0x31679u, static_cast<uint16_t>(RouteId::query_achievements), "QueryAchievements", sizeof("QueryAchievements") - 1},
    {UINT64_C(0xdd8c6a90fbcfeb52), 0x30e62u, static_cast<uint16_t>(RouteId::get_internet_connected_state), "GetInternetConnectedState", sizeof("GetInternetConnectedState") - 1},
    {UINT64_C(0xdecf1968d3efd1fc), 0x3018du, static_cast<uint16_t>(RouteId::query_friends), "QueryFriends", sizeof("QueryFriends") - 1},
    {UINT64_C(0xe0558472a1297182), 0x3500cu, static_cast<uint16_t>(RouteId::set_presence), "SetPresence", sizeof("SetPresence") - 1},
    {UINT64_C(0xead55cb841534af8), 0x3462bu, static_cast<uint16_t>(RouteId::query_image), "QueryImage", sizeof("QueryImage") - 1},
    {UINT64_C(0xf7c634d45f94bb28), 0x33d37u, static_cast<uint16_t>(RouteId::get_auth_code), "GetAuthCode", sizeof("GetAuthCode") - 1},
    {UINT64_C(0xfd77504ac92dc304), 0x3386eu, static_cast<uint16_t>(RouteId::query_content), "QueryContent", sizeof("QueryContent") - 1},
};

constexpr RouteDescriptor show_igo_window_route = {
    UINT64_C(0xc96101f996a3e86d), 0u,
    static_cast<uint16_t>(RouteId::show_igo_window),
    "ShowIGOWindow", sizeof("ShowIGOWindow") - 1u
};

constexpr RouteDescriptor compatibility_routes[compatibility_route_count] = {
    {UINT64_C(0xf21c42469f19cfa6), 0u, static_cast<uint16_t>(RouteId::get_presence_visibility), "GetPresenceVisibility", sizeof("GetPresenceVisibility") - 1},
    {UINT64_C(0xdfae2d60bb09340b), 0u, static_cast<uint16_t>(RouteId::are_chunks_installed), "AreChunksInstalled", sizeof("AreChunksInstalled") - 1},
    {UINT64_C(0xfe55c0b14d39b446), 0u, static_cast<uint16_t>(RouteId::restart_game), "RestartGame", sizeof("RestartGame") - 1},
    {UINT64_C(0x627425d75cc9b05f), 0u, static_cast<uint16_t>(RouteId::get_voip_status), "GetVoipStatus", sizeof("GetVoipStatus") - 1},
    {UINT64_C(0x7edda9d82a5429b2), 0u, static_cast<uint16_t>(RouteId::query_chunk_status), "QueryChunkStatus", sizeof("QueryChunkStatus") - 1},
    {UINT64_C(0xc1644021c24b1ff0), 0u, static_cast<uint16_t>(RouteId::get_block_list), "GetBlockList", sizeof("GetBlockList") - 1},
};
constexpr RouteDescriptor donor_extension_routes[9] = {
    {UINT64_C(0x9a35a28ac8b9b8ac), 0u, static_cast<uint16_t>(RouteId::go_online), "GoOnline", sizeof("GoOnline")-1u},
    {UINT64_C(0x86d3a0c14758541e), 0u, static_cast<uint16_t>(RouteId::get_utc_time), "GetUTCTime", sizeof("GetUTCTime")-1u},
    {UINT64_C(0xab2b4b0d854ca6ea), 0u, static_cast<uint16_t>(RouteId::set_presence_visibility), "SetPresenceVisibility", sizeof("SetPresenceVisibility")-1u},
    {UINT64_C(0x848b8a2eb38965e2), 0u, static_cast<uint16_t>(RouteId::query_presence), "QueryPresence", sizeof("QueryPresence")-1u},
    {UINT64_C(0x769fc8148b2886de), 0u, static_cast<uint16_t>(RouteId::get_group_info), "GetGroupInfo", sizeof("GetGroupInfo")-1u},
    {UINT64_C(0xd8f11521dc93f028), 0u, static_cast<uint16_t>(RouteId::set_downloader_utilization), "SetDownloaderUtilization", sizeof("SetDownloaderUtilization")-1u},
    {UINT64_C(0xf7672aef4d24523c), 0u, static_cast<uint16_t>(RouteId::enable_voip), "EnableVoip", sizeof("EnableVoip")-1u},
    {UINT64_C(0xb68b6c4edeccd7f6), 0u, static_cast<uint16_t>(RouteId::select_store), "SelectStore", sizeof("SelectStore")-1u},
    {UINT64_C(0x77227d68ff60cc66), 0u, static_cast<uint16_t>(RouteId::get_wallet_balance), "GetWalletBalance", sizeof("GetWalletBalance")-1u},
};
// Explicit LOCAL-ONLY descriptors, never a remote EA social contract.
constexpr RouteDescriptor local_social_routes[5] = {
    {UINT64_C(0x8faf6f98d4584765), 0u, static_cast<uint16_t>(RouteId::block_user), "BlockUser", sizeof("BlockUser")-1u},
    {UINT64_C(0xa534bd3d7921876c), 0u, static_cast<uint16_t>(RouteId::unblock_user), "UnblockUser", sizeof("UnblockUser")-1u},
    {UINT64_C(0x364ec1392c89309e), 0u, static_cast<uint16_t>(RouteId::request_friend), "RequestFriend", sizeof("RequestFriend")-1u},
    {UINT64_C(0x1bb83129e39034e5), 0u, static_cast<uint16_t>(RouteId::remove_friend), "RemoveFriend", sizeof("RemoveFriend")-1u},
    {UINT64_C(0x9060f1850546eaca), 0u, static_cast<uint16_t>(RouteId::send_invite), "SendInvite", sizeof("SendInvite")-1u},
};
} // namespace

const RouteDescriptor* route_inventory() noexcept
{
    return routes;
}

const RouteDescriptor* compatibility_route_inventory() noexcept
{
    return compatibility_routes;
}

const RouteDescriptor* find_known_route(preloader_l::xml::TextSpan command) noexcept
{
    if (const auto* direct = find_route(command, routes, route_count)) return direct;

    // HF08 W01: direct named alias descriptor with its own numeric ID.
    // Existing handler and delivery behavior are preserved by explicit binding.
    if (find_route(command, &show_igo_window_route, 1u))
        return &show_igo_window_route;

    if (const auto* extension=find_route(command,donor_extension_routes,9u))return extension;
    if (const auto* local=find_route(command,local_social_routes,5u))return local;

    // HOTFIX02 evidence-backed compatibility surface. These descriptors have
    // no claimed Primary handler RVA; they are explicit Consumer + Donor1/Rust
    // integrations kept separate from the recovered 22-entry Primary set.
    return find_route(command, compatibility_routes, compatibility_route_count);
}

RouteId descriptor_route_id(const RouteDescriptor& descriptor) noexcept
{
    return static_cast<RouteId>(descriptor.route_id);
}

} } // namespace preloader_l::router
