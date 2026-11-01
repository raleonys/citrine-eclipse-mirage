#pragma once

#include <stddef.h>
#include <stdint.h>

#include "preloader_l/router/route_lookup.hpp"

namespace preloader_l { namespace router {

// HF08 HOTFIX 08: project-approved Anadius-aligned internal RouteId order.
// All 42 normal LSX commands are source-registered and handler-bound. Route matching
// uses exact command names; unknown commands must not become synthetic successes.
enum class RouteId : uint16_t {
    get_config = 1,
    get_profile = 2,
    get_internet_connected_state = 3,
    get_setting = 4,
    get_settings = 5,
    query_content = 6,
    query_entitlements = 7,
    query_offers = 8,
    get_game_info = 9,
    get_all_game_info = 10,
    query_image = 11,
    go_online = 12,
    get_auth_code = 13,
    show_igo = 14,
    show_igo_window = 15,
    get_utc_time = 16,
    start_download = 17,
    set_presence_visibility = 18,
    set_presence = 19,
    get_presence_visibility = 20,
    get_presence = 21,
    query_presence = 22,
    get_group_info = 23,
    is_progressive_installation_available = 24,
    query_chunk_status = 25,
    are_chunks_installed = 26,
    set_downloader_utilization = 27,
    query_achievements = 28,
    grant_achievement = 29,
    query_friends = 30,
    get_block_list = 31,
    get_voip_status = 32,
    enable_voip = 33,
    select_store = 34,
    get_wallet_balance = 35,
    restart_game = 36,
    check_permission = 37,
    block_user = 38,
    unblock_user = 39,
    request_friend = 40,
    remove_friend = 41,
    send_invite = 42,
};

// Primary and compatibility descriptor arrays retain their established physical
// sizes (21 and 6); the other named commands are registered in extension tables.
// ShowIGOWindow is an explicit descriptor, not an array alias.
constexpr size_t route_count = 21;
constexpr size_t compatibility_route_count = 6;

const RouteDescriptor* route_inventory() noexcept;
const RouteDescriptor* compatibility_route_inventory() noexcept;
const RouteDescriptor* find_known_route(preloader_l::xml::TextSpan command) noexcept;
RouteId descriptor_route_id(const RouteDescriptor& descriptor) noexcept;

} } // namespace preloader_l::router
