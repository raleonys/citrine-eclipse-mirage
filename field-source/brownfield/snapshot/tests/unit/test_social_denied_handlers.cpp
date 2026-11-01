#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/handlers/handler_registry.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char* s,const char* q){return std::strstr(s,q)!=nullptr;}
int main(){
 struct Case{const char* name;const char* target;handlers::HandlerFn handler;};
 const Case cases[]={{"BlockUser","UserIdToBlock",&handlers::handle_block_user},
 {"UnblockUser","UserIdToUnblock",&handlers::handle_unblock_user},
 {"RequestFriend","UserToAdd",&handlers::handle_request_friend},
 {"RemoveFriend","UserToRemove",&handlers::handle_remove_friend}};
 char input[512],out[512];
 for(const auto& c:cases){
  std::strcpy(input,"<LSX><Request id=\"s1\"><");
  std::strcat(input,c.name);std::strcat(input," UserId=\"10\" ");
  std::strcat(input,c.target);std::strcat(input,"=\"20\"/></Request></LSX>");
  xml::RequestView q{};
  if(xml::parse_request_view(input,std::strlen(input),&q)!=xml::RequestStatus::ok)return 1;
  size_t written=0;
  if(c.handler(q,nullptr,0,&written)!=handlers::HandlerStatus::response_error||written==0)return 2;
  if(c.handler(q,out,sizeof(out),&written)!=handlers::HandlerStatus::ok)return 3;
  if(!has(out,"Local social principal is not authenticated.")||has(out,"Code=\"0\""))return 4;
  // Local-only routing is registered, but a direct/no-session dispatch remains negative.
  if(!router::find_known_route(q.command))return 5;
 }
 const char invitation[]="<LSX><Request id=\"s1\"><SendInvite UserId=\"10\" Invitation=\"Hi\"><Invitees>20</Invitees></SendInvite></Request></LSX>";
 xml::RequestView q{};size_t w=0;
 if(xml::parse_request_view(invitation,sizeof(invitation)-1,&q)!=xml::RequestStatus::ok)return 6;
 if(handlers::handle_send_invite(q,out,sizeof(out),&w)!=handlers::HandlerStatus::ok)return 7;
 if(has(out,"Code=\"0\"")||!router::find_known_route(q.command))return 8;
 return 0;
}
