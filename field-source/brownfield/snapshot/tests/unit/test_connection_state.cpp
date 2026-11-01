#include "preloader_l/handlers/connection_state_route.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
 constexpr char in[]="<LSX><Request id=\"25\"><GetInternetConnectedState/></Request></LSX>"; xml::RequestView req{};
 if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok)return 1;
 auto* route=router::find_known_route(req.command); if(!route||router::descriptor_route_id(*route)!=router::RouteId::get_internet_connected_state||route->original_handler_rva!=0x30e62u)return 2;
 char out[256]{};size_t n=0;
 if(handlers::format_get_internet_connected_state(req,true,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 3;
 constexpr char yes[]=" <LSX>\n  <Response id=\"25\" sender=\"EbisuSDK\">\n    <InternetConnectedState connected=\"true\"/>\n  </Response>\n </LSX>\n";
 if(n!=sizeof(yes)-1||std::strcmp(out,yes)!=0)return 4;
 if(handlers::format_get_internet_connected_state(req,false,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return 5;
 constexpr char no[]=" <LSX>\n  <Response id=\"25\" sender=\"EbisuSDK\">\n    <InternetConnectedState connected=\"false\"/>\n  </Response>\n </LSX>\n";
 if(n!=sizeof(no)-1||std::strcmp(out,no)!=0)return 6;
 return 0;
}
