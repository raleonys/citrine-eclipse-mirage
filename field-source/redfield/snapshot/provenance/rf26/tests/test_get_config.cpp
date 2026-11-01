// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/handlers/config_route.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
 constexpr char in[]="<LSX><Request id=\"21\"><GetConfig/></Request></LSX>"; xml::RequestView req{};
 if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok)return 1;
 auto* route=router::find_known_route(req.command); if(!route||router::descriptor_route_id(*route)!=router::RouteId::get_config||route->original_handler_rva!=0x2ad5fu)return 2;
 char out[8192]{};size_t n=0;if(handlers::handle_get_config(req,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 3;
 constexpr char prefix[]=" <LSX>\n  <Response id=\"21\" sender=\"EbisuSDK\">\n   <GetConfigResponse>\n"; if(std::strncmp(out,prefix,sizeof(prefix)-1)!=0)return 4;
 const char* ordered[]={
  "Name=\"EbisuSDK\" Facility=\"SDK\"","Name=\"EbisuSDK\" Facility=\"PROFILE\"","Name=\"XMPP\" Facility=\"PRESENCE\"","Name=\"XMPP\" Facility=\"FRIENDS\"","Name=\"Commerce\" Facility=\"COMMERCE\"","Name=\"EbisuSDK\" Facility=\"RECENTPLAYER\"","Name=\"EbisuSDK\" Facility=\"IGO\"","Name=\"EbisuSDK\" Facility=\"MISC\"","Name=\"EALS\" Facility=\"LOGIN\"","Name=\"Utility\" Facility=\"UTILITY\"","Name=\"XMPP\" Facility=\"XMPP\"","Name=\"XMPP\" Facility=\"CHAT\"","Name=\"EbisuSDK\" Facility=\"IGO_EVENT\"","Name=\"EALS\" Facility=\"EALS_EVENTS\"","Name=\"EbisuSDK\" Facility=\"LOGIN_EVENT\"","Name=\"XMPP\" Facility=\"INVITE_EVENT\"","Name=\"EbisuSDK\" Facility=\"PROFILE_EVENT\"","Name=\"XMPP\" Facility=\"PRESENCE_EVENT\"","Name=\"XMPP\" Facility=\"FRIENDS_EVENT\"","Name=\"Commerce\" Facility=\"COMMERCE_EVENT\"","Name=\"XMPP\" Facility=\"CHAT_EVENT\"","Name=\"EbisuSDK\" Facility=\"DOWNLOAD_EVENT\"","Name=\"EbisuSDK\" Facility=\"PERMISSION\"","Name=\"EbisuSDK\" Facility=\"RESOURCES\"","Name=\"EbisuSDK\" Facility=\"BLOCKED_USERS\"","Name=\"EbisuSDK\" Facility=\"BLOCKED_USER_EVENT\"","Name=\"EbisuSDK\" Facility=\"GET_USERID\"","Name=\"EbisuSDK\" Facility=\"ONLINE_STATUS_EVENT\"","Name=\"EbisuSDK\" Facility=\"ACHIEVEMENT\"","Name=\"EbisuSDK\" Facility=\"ACHIEVEMENT_EVENT\"","Name=\"EbisuSDK\" Facility=\"BROADCAST_EVENT\"","Name=\"PI\" Facility=\"PROGRESSIVE_INSTALLATION\"","Name=\"PI\" Facility=\"PROGRESSIVE_INSTALLATION_EVENT\"","Name=\"EbisuSDK\" Facility=\"CONTENT\""};
 const char* cursor=out; for(const char* q:ordered){const char* hit=std::strstr(cursor,q);if(!hit)return 5;cursor=hit+std::strlen(q);}
 size_t services=0;for(const char* p=out;(p=std::strstr(p,"<Service "))!=nullptr;p+=9)++services;if(services!=34)return 6;
 if(n!=std::strlen(out))return 7;

 // Request-owned source attributes must be decoded and escaped exactly once.
 constexpr char entity_in[]="<LSX><Request id=\"21&amp;22\" recipient=\"E&#38;ALS\"><GetConfig/></Request></LSX>";
 xml::RequestView entity_req{};
 if(xml::parse_request_view(entity_in,sizeof(entity_in)-1,&entity_req)!=xml::RequestStatus::ok)return 8;
 std::memset(out,0,sizeof out);n=0;
 if(handlers::handle_get_config(entity_req,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 9;
 if(!std::strstr(out,"<Response id=\"21&amp;22\" sender=\"E&amp;ALS\">"))return 10;
 if(std::strstr(out,"&amp;amp;"))return 11;
 if(n!=std::strlen(out))return 12;

 constexpr char malformed_in[]="<LSX><Request id=\"bad&bogus;\"><GetConfig/></Request></LSX>";
 xml::RequestView malformed_req{};
 if(xml::parse_request_view(malformed_in,sizeof(malformed_in)-1,&malformed_req)!=xml::RequestStatus::ok)return 13;
 n=123;
 if(handlers::handle_get_config(malformed_req,out,sizeof out,&n)!=handlers::HandlerStatus::invalid_request)return 14;
 if(n!=0)return 15;
 return 0;
}
