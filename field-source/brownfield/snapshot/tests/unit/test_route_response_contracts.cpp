#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/handlers/handler_registry.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char* out,const char* needle){return std::strstr(out,needle)!=nullptr;}
int main(){
 char out[16384]{};
 struct Case{const char* tag;router::RouteId id;const char* expected;};
 const Case cases[]={
 {"GoOnline",router::RouteId::go_online,"<ErrorSuccess Description=\"\" Code=\"0\"/>"},
 {"EnableVoip",router::RouteId::enable_voip,"Code=\"-1593835518\""},
 {"SetDownloaderUtilization",router::RouteId::set_downloader_utilization,"Code=\"0\""},
 {"SetPresenceVisibility",router::RouteId::set_presence_visibility,"Code=\"0\""},
 {"GetGroupInfo",router::RouteId::get_group_info,"Code=\"-1577058296\""},
 {"SelectStore",router::RouteId::select_store,"Code=\"0\""},
 {"GetWalletBalance",router::RouteId::get_wallet_balance,"<GetWalletBalanceResponse Balance=\"0\"/>"},
 {"GetUTCTime",router::RouteId::get_utc_time,"<GetUTCTimeResponse Time=\""},
 {"QueryPresence",router::RouteId::query_presence,"<QueryPresenceResponse>"}
 };
 for(unsigned i=0;i<sizeof(cases)/sizeof(cases[0]);++i){
  char in[256]={};const char* a="<LSX><Request id=\"abc\"><";const char* b="/></Request></LSX>";
  std::strcat(in,a);std::strcat(in,cases[i].tag);std::strcat(in,b);
  auto r=core::dispatch_plaintext_request(in,std::strlen(in),out,sizeof(out));
  if(r.status!=core::DispatchStatus::ok||r.route!=cases[i].id||!has(out,cases[i].expected))return 10+i;
  if(!handlers::route_has_handler_boundary(cases[i].id))return 25+i;
 }
 // All social names resolve but the direct dispatcher is NOT authorized.
 if(!handlers::route_has_handler_boundary(router::RouteId::block_user))return 80;
 if(!handlers::route_has_handler_boundary(router::RouteId::send_invite))return 81;
 const char social[]="<LSX><Request id=\"social\"><BlockUser UserId=\"10\" UserIdToBlock=\"20\"/></Request></LSX>";
 auto sx=core::dispatch_plaintext_request(social,sizeof(social)-1,out,sizeof(out));
 if(sx.status!=core::DispatchStatus::ok||!has(out,"Local social principal is not authenticated."))return 82;
 // One arbitrary-named direct child, encoded UserId, validated XML writer.
 const char qp[]="<LSX><Request id=\"4\"><QueryPresence><Arbitrary>User&amp;Name</Arbitrary></QueryPresence></Request></LSX>";
 auto r=core::dispatch_plaintext_request(qp,sizeof(qp)-1u,out,sizeof(out));
 if(r.status!=core::DispatchStatus::ok||!has(out,"UserId=\"User&amp;Name\"")||!has(out,"State=\"NONE\""))return 90;
 const char no_text[]="<LSX><Request id=\"5\"><QueryPresence><X/></QueryPresence></Request></LSX>";
 r=core::dispatch_plaintext_request(no_text,sizeof(no_text)-1u,out,sizeof(out));
 if(r.status==core::DispatchStatus::ok||r.written!=0)return 91;
 return 0;
}
