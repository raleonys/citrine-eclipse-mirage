// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/profile_response.hpp"
namespace redfield {
namespace {
namespace config=redfield::config;
config::ValueView value(const ConfigSnapshot& snapshot,const char* section,const char* key) noexcept {
    for(auto i=snapshot.leaves.rbegin();i!=snapshot.leaves.rend();++i)
        if(i->path.size()==1 && i->path[0]==section && i->key==key)
            return {i->raw_value.data(),i->raw_value.size(),true};
    return config::missing_value();
}
config::ValueView profile_value(const ConfigSnapshot& snapshot,const char* key,const char* initial) noexcept {
    if(!snapshot.overlay_loaded)return config::literal_value(initial);
    const auto found=value(snapshot,"User",key);
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
    // Game.Name and Game.GameName share one alias; last occurrence wins.
    for(auto i=snapshot.leaves.rbegin();i!=snapshot.leaves.rend();++i)
        if(i->path.size()==1 && i->path[0]=="Game" && (i->key=="Name" || i->key=="GameName"))
            return {i->raw_value.data(),i->raw_value.size(),true};
    return config::literal_value("Game");
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
