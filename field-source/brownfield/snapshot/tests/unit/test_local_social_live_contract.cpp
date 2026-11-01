#include "preloader_l/handlers/local_social_list_route.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/runtime/local_social_state.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
static xml::RequestView parse(const char* x){xml::RequestView r{};assert(xml::parse_request_view(x,std::strlen(x),&r)==xml::RequestStatus::ok);return r;}
int main(){
 const char* names[]={"BlockUser","UnblockUser","RequestFriend","RemoveFriend","SendInvite"};
 const unsigned ids[]={38,39,40,41,42};
 for(unsigned i=0;i<5;++i){auto* x=router::find_known_route({names[i],std::strlen(names[i])});
  assert(x&&static_cast<unsigned>(router::descriptor_route_id(*x))==ids[i]);}
 runtime::LocalSocialStore s{};assert(s.register_local_actor(10));
 assert(s.register_local_actor(20));assert(s.register_local_actor(21));
 s.set_enabled_for_test_only(true);
 auto lease=s.trusted_lease_for_test_only(10);
 runtime::LocalSocialPending p{};
 assert(s.prepare(lease,runtime::LocalSocialAction::block_user,33,&p)==runtime::LocalSocialResult::ok);
 runtime::LocalSocialSnapshot snap{};assert(s.read_snapshot(lease,&snap)==runtime::LocalSocialResult::ok && snap.blocked_n==0);
 s.rollback(&p);
 assert(s.prepare(lease,runtime::LocalSocialAction::block_user,33,&p)==runtime::LocalSocialResult::ok);
 assert(s.commit(&p)==runtime::LocalSocialResult::ok);
 assert(s.read_snapshot(lease,&snap)==runtime::LocalSocialResult::ok && snap.blocked_n==1);
 const auto request=parse("<LSX><Request id=\"q&amp;amp;1\"><GetBlockList/></Request></LSX>");
 char out[8192]{};size_t n=0;
 const auto id=router::RouteId::get_block_list;
 auto status=handlers::render_local_social_list(request,id,snap,nullptr,0,&n);
 assert(status==handlers::HandlerStatus::response_error&&n>0);
 status=handlers::render_local_social_list(request,id,snap,out,sizeof(out),&n);
 assert(status==handlers::HandlerStatus::ok&&std::strstr(out,"GetBlockListResponse")&&
    std::strstr(out,"UserId=\"33\"")&&std::strstr(out,"<Friend "));
 const uint64_t targets[]={20,21};
 assert(s.prepare_invites(lease,targets,2,"Hi & Bye",8,&p)==runtime::LocalSocialResult::ok);
 assert(p.active&&p.invitee_n==2);s.rollback(&p);
 assert(!s.inbox(20,10)&&!s.inbox(21,10));
 assert(s.prepare_invites(lease,targets,2,"Hi & Bye",8,&p)==runtime::LocalSocialResult::ok);
 assert(s.commit(&p)==runtime::LocalSocialResult::ok);
 assert(s.inbox(20,10)&&s.inbox(21,10));
 const char* msg=nullptr;size_t msg_n=0;
 assert(s.inbox_message(20,10,&msg,&msg_n)&&msg_n==8&&std::memcmp(msg,"Hi & Bye",8)==0);
 assert(s.prepare_invites(lease,targets,2,"Again",5,&p)==runtime::LocalSocialResult::already_exists);
 assert(!p.active);
 return 0;
}
