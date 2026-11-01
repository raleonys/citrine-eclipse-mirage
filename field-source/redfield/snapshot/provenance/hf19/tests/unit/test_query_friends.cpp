#include "preloader_l/handlers/friends_route.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
 constexpr char in[]="<LSX><Request id=\"17\"><QueryFriends/></Request></LSX>"; xml::RequestView req{};
 if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok) return 1;
 const auto* route=router::find_known_route(req.command);
 if(!route || router::descriptor_route_id(*route)!=router::RouteId::query_friends || route->original_handler_rva!=0x3018du) return 2;
 char out[1024]{}; size_t written=7;
 config::clear_runtime_overrides();
 if(handlers::handle_query_friends(req,out,sizeof out,&written)!=handlers::HandlerStatus::ok) return 3;
 if(!std::strstr(out,"<QueryFriendsResponse Return=\"Success\"/>")) return 4;
 if(std::strstr(out,"<Friend ") || std::strstr(out,"ErrorSuccess") || std::strstr(out,"SURGE")) return 5;
 config::set_runtime_online_override(config::literal_value("false"));
 written=7; out[0]='\0';
 if(handlers::handle_query_friends(req,out,sizeof out,&written)!=handlers::HandlerStatus::ok) return 6;
 if(!std::strstr(out,"<QueryFriendsResponse Return=\"Success\"/>") || std::strstr(out,"<Friend ") || std::strstr(out,"ErrorSuccess") || std::strstr(out,"SURGE")) return 7;
 config::set_runtime_online_override(config::literal_value("0"));
 written=7; out[0]='\0';
 if(handlers::handle_query_friends(req,out,sizeof out,&written)!=handlers::HandlerStatus::ok) return 8;
 if(!std::strstr(out,"<QueryFriendsResponse Return=\"Success\"/>") || std::strstr(out,"<Friend ") || std::strstr(out,"ErrorSuccess") || std::strstr(out,"SURGE")) return 9;
 config::clear_runtime_overrides();
 return 0;
}
