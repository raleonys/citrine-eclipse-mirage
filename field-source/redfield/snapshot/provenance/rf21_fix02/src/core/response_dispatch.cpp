// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/response_dispatch.hpp"
#include "store_formatter.hpp"
#include "redfield/content_response.hpp"
#include "redfield/entitlement_response.hpp"
#include "redfield/achievement_response.hpp"
#include "redfield/auth_code_session.hpp"
#include "redfield/settings_response.hpp"
#include "redfield/profile_response.hpp"
#include "redfield/game_info_response.hpp"
#include "redfield/query_presence_response.hpp"
#include "redfield/image_response.hpp"
#include "redfield/chunks_response.hpp"
#include "preloader_l/handlers/utc_time_route.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/handlers/config_route.hpp"
#include "preloader_l/handlers/progressive_installation_route.hpp"
#include "preloader_l/handlers/friends_route.hpp"
#include "preloader_l/handlers/group_info_route.hpp"
#include "preloader_l/handlers/integration_routes.hpp"
namespace redfield {
namespace {
ResponseResult dispatch(const xml::Request& request,const ConfigSnapshot* configuration,AuthCodeSession* auth,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;
    if(!output || !capacity)return ResponseResult::rejected;
    const auto* descriptor=preloader_l::router::find_known_route(request.command);
    if(!descriptor)return ResponseResult::no_response;
    using namespace preloader_l::handlers;
    using preloader_l::router::RouteId;
    HandlerStatus status;
    switch(preloader_l::router::descriptor_route_id(*descriptor)) {
    case RouteId::query_achievements:
        return configuration?dispatch_achievement_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::query_entitlements:
        return configuration?dispatch_entitlement_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::query_content:
        return configuration?dispatch_content_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::query_offers:status=store_detail::handle_query_offers(request,output,capacity,written);break;
    case RouteId::select_store:status=store_detail::handle_select_store(request,output,capacity,written);break;
    case RouteId::get_wallet_balance:status=store_detail::handle_get_wallet_balance(request,output,capacity,written);break;
    case RouteId::get_auth_code:
        return configuration && auth?auth->respond(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::go_online:status=handle_go_online(request,output,capacity,written);break;
    case RouteId::enable_voip:status=handle_enable_voip(request,output,capacity,written);break;
    case RouteId::set_downloader_utilization:status=handle_set_downloader_utilization(request,output,capacity,written);break;
    case RouteId::set_presence_visibility:status=handle_set_presence_visibility(request,output,capacity,written);break;
    case RouteId::set_presence:status=handle_set_presence(request,output,capacity,written);break;
    case RouteId::start_download:status=handle_start_download(request,output,capacity,written);break;
    case RouteId::restart_game:status=handle_restart_game(request,output,capacity,written);break;
    case RouteId::are_chunks_installed:
        return dispatch_chunks_response(request,output,capacity,written);
    case RouteId::query_image:
        return configuration?dispatch_image_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::query_presence:
        return configuration?dispatch_query_presence_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::get_game_info:
    case RouteId::get_all_game_info:
        return configuration?dispatch_game_info_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::get_utc_time:status=handle_get_utc_time(request,output,capacity,written);break;
    case RouteId::get_profile:
    case RouteId::get_presence:
        return configuration?dispatch_profile_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    case RouteId::get_config:status=handle_get_config(request,output,capacity,written);break;
    case RouteId::is_progressive_installation_available:status=handle_is_progressive_installation_available(request,output,capacity,written);break;
    case RouteId::query_friends:status=handle_query_friends(request,output,capacity,written);break;
    case RouteId::get_block_list:status=handle_get_block_list(request,output,capacity,written);break;
    case RouteId::get_presence_visibility:status=handle_get_presence_visibility(request,output,capacity,written);break;
    case RouteId::get_group_info:status=handle_get_group_info(request,output,capacity,written);break;
    case RouteId::get_voip_status:status=handle_get_voip_status(request,output,capacity,written);break;
    case RouteId::query_chunk_status:status=handle_query_chunk_status(request,output,capacity,written);break;
    case RouteId::check_permission:
    case RouteId::get_setting:
    case RouteId::get_settings:
    case RouteId::get_internet_connected_state:
        return configuration?dispatch_settings_response(request,*configuration,output,capacity,written):ResponseResult::no_response;
    default:return ResponseResult::no_response;
    }
    if(status==HandlerStatus::no_response_required || status==HandlerStatus::unsupported){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
 } // namespace
ResponseResult dispatch_response_with_session(const xml::Request& request,const ConfigSnapshot& configuration,AuthCodeSession& auth,char* output,std::size_t capacity,std::size_t* written) noexcept {
    return dispatch(request,&configuration,&auth,output,capacity,written);
}
ResponseResult dispatch_response(const xml::Request& request,char* output,std::size_t capacity,std::size_t* written) noexcept {
    return dispatch(request,nullptr,nullptr,output,capacity,written);
}
ResponseResult dispatch_response_with_config(const xml::Request& request,const ConfigSnapshot& configuration,char* output,std::size_t capacity,std::size_t* written) noexcept {
    return dispatch(request,&configuration,nullptr,output,capacity,written);
}
}
