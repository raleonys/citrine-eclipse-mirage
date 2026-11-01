#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
namespace preloader_l { namespace config {
static uint64_t gen=42;
bool begin_runtime_configuration_read(RuntimeConfigurationReadLease* x) noexcept {
    if(!x)return false;
    *x={gen,true};return true;
}
void end_runtime_configuration_read(RuntimeConfigurationReadLease* x) noexcept {
    if(x)x->active=false;
}
uint64_t runtime_configuration_generation() noexcept {return gen;}
bool runtime_configuration_loaded() noexcept {return true;}
ValueView runtime_configuration_setting(const char* key) noexcept {
    static constexpr char configured[]="20,21";
    if(std::strcmp(key,"LocalSocialRecipients")==0)
        return ValueView{configured,sizeof(configured)-1,true};
    return ValueView{nullptr,0,false};
}
ProfilePolicyView runtime_profile_policy() noexcept {
    static constexpr char id[]="10";
    const ValueView v{id,2,true};return {v,v,v,v,v,v,v,v,v};
}
} }
static xml::RequestView parse(const char* raw){
    xml::RequestView r{};
    assert(xml::parse_request_view(raw,std::strlen(raw),&r)==xml::RequestStatus::ok);
    return r;
}
int main(){
    runtime::LocalSocialSession session{};
    assert(runtime::bind_host_social_profile(&session));
    runtime::LocalSocialPending pending{};
    auto r=parse("<LSX><Request id=\"1\"><BlockUser UserId=\"11\" UserIdToBlock=\"20\"/></Request></LSX>");
    assert(!handlers::prepare_local_social_socket_request(r,session,&pending));
    r=parse("<LSX><Request id=\"2\"><BlockUser UserId=\"10\" UserIdToBlock=\"20\"/></Request></LSX>");
    assert(handlers::prepare_local_social_socket_request(r,session,&pending));
    assert(pending.active && !pending.new_local_target);
    char output[512]{};size_t n=0;
    assert(handlers::render_local_social_success(r,output,sizeof(output),&n)==handlers::HandlerStatus::ok);
    assert(n>0 && std::strstr(output,"ErrorSuccess") && std::strstr(output,"Code=\"0\""));
    runtime::rollback_local_social_mutation(&pending);
    runtime::LocalSocialSnapshot snapshot{};
    assert(runtime::snapshot_local_social(session,&snapshot)==runtime::LocalSocialResult::ok && snapshot.blocked_n==0);
    assert(handlers::prepare_local_social_socket_request(r,session,&pending));
    assert(runtime::commit_local_social_mutation(session,&pending)==runtime::LocalSocialResult::ok);
    assert(runtime::snapshot_local_social(session,&snapshot)==runtime::LocalSocialResult::ok);
    assert(snapshot.blocked_n==1 && snapshot.blocked[0]==20);
    r=parse("<LSX><Request id=\"3\"><UnblockUser UserId=\"10\" UserIdToUnblock=\"20\"/></Request></LSX>");
    assert(handlers::prepare_local_social_socket_request(r,session,&pending));
    assert(runtime::commit_local_social_mutation(session,&pending)==runtime::LocalSocialResult::ok);
    r=parse("<LSX><Request id=\"4\"><SendInvite UserId=\"10\" Invitation=\"Hi\"><Invitees>20</Invitees></SendInvite></Request></LSX>");
    assert(handlers::prepare_local_social_socket_request(r,session,&pending));
    assert(pending.invitee_n==1 && pending.invitation_n==2);
    runtime::rollback_local_social_mutation(&pending);
    r=parse("<LSX><Request id=\"5\"><SendInvite UserId=\"10\" Invitation=\"Hi&amp;Bye\"><Invitees>20</Invitees><Invitees>21</Invitees></SendInvite></Request></LSX>");
    assert(handlers::prepare_local_social_socket_request(r,session,&pending));
    assert(pending.invitee_n==2 && pending.invitation_n==6);
    runtime::rollback_local_social_mutation(&pending);
    r=parse("<LSX><Request id=\"6\"><RequestFriend UserId=\"10\" UserToAdd=\"30\"/></Request></LSX>");
    assert(!handlers::prepare_local_social_socket_request(r,session,&pending));
    config::gen++;
    assert(runtime::commit_local_social_mutation(session,&pending)==runtime::LocalSocialResult::stale_lease);
    runtime::rollback_local_social_mutation(&pending);
    assert(!runtime::local_social_session_current(session));
    return 0;
}
