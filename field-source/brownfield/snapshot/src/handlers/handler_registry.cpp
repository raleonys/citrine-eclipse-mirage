#include "preloader_l/handlers/handler_registry.hpp"
#include "preloader_l/handlers/progressive_installation_route.hpp"
#include "preloader_l/handlers/permission_route.hpp"
#include "preloader_l/handlers/friends_route.hpp"
#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/handlers/config_route.hpp"
#include "preloader_l/handlers/connection_state_binding.hpp"
#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/handlers/query_presence_route.hpp"
#include "preloader_l/handlers/group_info_route.hpp"
#include "preloader_l/handlers/utc_time_route.hpp"
#include "preloader_l/handlers/social_mutation_routes.hpp"

namespace preloader_l { namespace handlers {

HandlerFn find_bound_handler(preloader_l::router::RouteId route) noexcept
{
    using preloader_l::router::RouteId;
    switch (route) {
    case RouteId::get_setting: return &handle_runtime_get_setting;
    case RouteId::get_settings: return &handle_runtime_get_settings;
    case RouteId::get_all_game_info: return &handle_get_all_game_info;
    case RouteId::query_entitlements: return &handle_query_entitlements;
    case RouteId::get_profile: return &handle_get_profile;
    case RouteId::query_offers: return &handle_query_offers;
    case RouteId::get_presence: return &handle_get_presence;
    case RouteId::get_game_info: return &handle_get_game_info;
    case RouteId::grant_achievement: return &handle_grant_achievement;
    case RouteId::query_achievements: return &handle_query_achievements;
    case RouteId::query_image: return &handle_query_image;
    case RouteId::get_auth_code: return &handle_get_auth_code;
    case RouteId::query_content: return &handle_query_content;
    case RouteId::is_progressive_installation_available: return &handle_is_progressive_installation_available;
    case RouteId::check_permission: return &handle_check_permission;
    case RouteId::query_friends: return &handle_query_friends;
    case RouteId::show_igo:
    case RouteId::show_igo_window: return &handle_show_igo;
    case RouteId::start_download: return &handle_start_download;
    case RouteId::set_presence: return &handle_set_presence;
    case RouteId::get_config: return &handle_get_config;
    case RouteId::get_internet_connected_state: return &handle_runtime_internet_state;
    case RouteId::get_presence_visibility: return &handle_get_presence_visibility;
    case RouteId::are_chunks_installed: return &handle_are_chunks_installed;
    case RouteId::restart_game: return &handle_restart_game;
    case RouteId::get_voip_status: return &handle_get_voip_status;
    case RouteId::query_chunk_status: return &handle_query_chunk_status;
    case RouteId::get_block_list: return &handle_get_block_list;
    case RouteId::go_online: return &handle_go_online;
    case RouteId::get_utc_time: return &handle_get_utc_time;
    case RouteId::set_presence_visibility: return &handle_set_presence_visibility;
    case RouteId::query_presence: return &handle_query_presence;
    case RouteId::get_group_info: return &handle_get_group_info;
    case RouteId::set_downloader_utilization: return &handle_set_downloader_utilization;
    case RouteId::enable_voip: return &handle_enable_voip;
    case RouteId::select_store: return &handle_select_store;
    case RouteId::get_wallet_balance: return &handle_get_wallet_balance;
    case RouteId::block_user: return &handle_block_user;
    case RouteId::unblock_user: return &handle_unblock_user;
    case RouteId::request_friend: return &handle_request_friend;
    case RouteId::remove_friend: return &handle_remove_friend;
    case RouteId::send_invite: return &handle_send_invite;
    default: return nullptr;
    }
}

bool route_has_handler_boundary(preloader_l::router::RouteId route) noexcept
{
    return find_bound_handler(route) != nullptr;
}

} }
