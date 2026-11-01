// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/profile_response.hpp"
#include "redfield/config_snapshot_view.hpp"
namespace redfield {
namespace {
namespace config=redfield::config;
config::ValueView profile_value(const ConfigSnapshot& snapshot,const char* key,const char* initial) noexcept {
    if(!snapshot.overlay_loaded)return config::literal_value(initial);
    const auto found=ConfigSnapshotView{snapshot}.find("User",key);
    return found.present?found:config::literal_value("");
}
}
ProfilePolicyView profile_policy(const ConfigSnapshot& snapshot) noexcept {
    // Exact field/default order retained from the pinned HF19 policy. This
    // projection borrows snapshot bytes only for the synchronous formatter call.
    return {profile_value(snapshot,"Username","SURGE"),
            profile_value(snapshot,"PersonaId","1144668899"),
            profile_value(snapshot,"UserId","1000200030000"),
            profile_value(snapshot,"Country","US"),
            profile_value(snapshot,"Currency","USD"),
            profile_value(snapshot,"IsSubscriber","true"),
            profile_value(snapshot,"IsSteamSubscriber","false"),
            profile_value(snapshot,"SubscriberLevel","2"),
            profile_value(snapshot,"IsUnderAge","false")};
}
ValueView presence_title(const ConfigSnapshot& snapshot) noexcept {
    return ConfigSnapshotView{snapshot}.or_default("Game",{"Name","GameName"},"Game");
}
ResponseResult dispatch_profile_response(const xml::Request& request,const ConfigSnapshot& snapshot,char* output,std::size_t capacity,std::size_t* written) noexcept {
    if(!written)return ResponseResult::rejected;
    *written=0;
    if(!output || !capacity)return ResponseResult::rejected;
    using redfield::handlers::HandlerStatus;
    HandlerStatus status;
    if(redfield::xml::span_equals(request.command,"GetProfile"))
        status=format_profile_response(request,profile_policy(snapshot),output,capacity,written);
    else if(redfield::xml::span_equals(request.command,"GetPresence"))
        status=format_presence_response(request,presence_title(snapshot),output,capacity,written);
    else return ResponseResult::no_response;
    if(status==HandlerStatus::unsupported || status==HandlerStatus::no_response_required){*written=0;return ResponseResult::no_response;}
    if(status!=HandlerStatus::ok || *written>=capacity){*written=0;return ResponseResult::rejected;}
    return ResponseResult::response;
}
}
