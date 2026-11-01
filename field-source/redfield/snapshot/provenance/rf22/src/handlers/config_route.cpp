// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/config_route.hpp"
#include "redfield/core/checked_math.hpp"
#include "redfield/xml/value_codec.hpp"

namespace redfield { namespace handlers {
namespace {
constexpr char command_name[] = "GetConfig";
constexpr char prefix[] = " <LSX>\n  <Response id=\"";
constexpr char middle_a[] = "\" sender=\"";
constexpr char middle_b[] = "\">\n";
constexpr char suffix[] =
"   <GetConfigResponse>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"SDK\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"PROFILE\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"PRESENCE\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"FRIENDS\"/>\n"
"    <Service Name=\"Commerce\" Facility=\"COMMERCE\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"RECENTPLAYER\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"IGO\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"MISC\"/>\n"
"    <Service Name=\"EALS\" Facility=\"LOGIN\"/>\n"
"    <Service Name=\"Utility\" Facility=\"UTILITY\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"XMPP\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"CHAT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"IGO_EVENT\"/>\n"
"    <Service Name=\"EALS\" Facility=\"EALS_EVENTS\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"LOGIN_EVENT\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"INVITE_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"PROFILE_EVENT\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"PRESENCE_EVENT\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"FRIENDS_EVENT\"/>\n"
"    <Service Name=\"Commerce\" Facility=\"COMMERCE_EVENT\"/>\n"
"    <Service Name=\"XMPP\" Facility=\"CHAT_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"DOWNLOAD_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"PERMISSION\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"RESOURCES\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"BLOCKED_USERS\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"BLOCKED_USER_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"GET_USERID\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"ONLINE_STATUS_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"ACHIEVEMENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"ACHIEVEMENT_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"BROADCAST_EVENT\"/>\n"
"    <Service Name=\"PI\" Facility=\"PROGRESSIVE_INSTALLATION\"/>\n"
"    <Service Name=\"PI\" Facility=\"PROGRESSIVE_INSTALLATION_EVENT\"/>\n"
"    <Service Name=\"EbisuSDK\" Facility=\"CONTENT\"/>\n"
"   </GetConfigResponse>\n"
"  </Response>\n"
" </LSX>\n";

bool source_attribute_size(redfield::xml::TextSpan value,size_t* encoded) noexcept {
    return redfield::xml::normalize_source_attribute_value(value,nullptr,0,encoded)==redfield::xml::ValueCodecStatus::ok;
}

bool emit_source_attribute(redfield::xml::TextSpan value,char*& dst,size_t remaining,size_t expected) noexcept {
    size_t actual=0;
    const auto status=redfield::xml::normalize_source_attribute_value(value,dst,remaining,&actual);
    if(status!=redfield::xml::ValueCodecStatus::ok||actual!=expected)return false;
    dst+=actual;
    return true;
}

void copy(char*& d,const char* s,size_t n) noexcept { for(size_t i=0;i<n;++i)d[i]=s[i]; d+=n; }
}

HandlerStatus handle_get_config(const redfield::xml::RequestView& request, char* output, size_t capacity, size_t* written) noexcept {
    if (written) *written=0;
    if (!written || (!output && capacity!=0)) return HandlerStatus::invalid_request;
    if (!redfield::xml::span_equals(request.command,command_name)) return HandlerStatus::wrong_command;
    if (!request.id.data||request.id.size==0) return HandlerStatus::invalid_request;
    const auto sender = redfield::xml::response_sender_policy(request);
    size_t id_size=0;
    size_t sender_size=0;
    if(!source_attribute_size(request.id,&id_size)||!source_attribute_size(sender,&sender_size)) return HandlerStatus::invalid_request;
    size_t need=sizeof(prefix)-1;
    size_t t=0;
    if (!redfield::core::checked_add_size(need,id_size,t)) return HandlerStatus::response_error;
    need=t;
    if (!redfield::core::checked_add_size(need,sizeof(middle_a)-1,t)) return HandlerStatus::response_error;
    need=t;
    if (!redfield::core::checked_add_size(need,sender_size,t)) return HandlerStatus::response_error;
    need=t;
    if (!redfield::core::checked_add_size(need,sizeof(middle_b)-1,t)) return HandlerStatus::response_error;
    need=t;
    if (!redfield::core::checked_add_size(need,sizeof(suffix)-1,t)) return HandlerStatus::response_error;
    need=t;
    *written=need;
    if (!redfield::core::checked_add_size(need,1,t) || !output || capacity<t) return HandlerStatus::response_error;
    char* d=output;
    copy(d,prefix,sizeof(prefix)-1);
    if(!emit_source_attribute(request.id,d,capacity-static_cast<size_t>(d-output),id_size)) return HandlerStatus::response_error;
    copy(d,middle_a,sizeof(middle_a)-1);
    if(!emit_source_attribute(sender,d,capacity-static_cast<size_t>(d-output),sender_size)) return HandlerStatus::response_error;
    copy(d,middle_b,sizeof(middle_b)-1);
    copy(d,suffix,sizeof(suffix)-1);
    *d='\0';
    *written=need;
    return HandlerStatus::ok;
}
} }
