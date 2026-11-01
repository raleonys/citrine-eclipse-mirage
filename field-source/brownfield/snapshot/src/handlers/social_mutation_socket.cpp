#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/protocol/origin_error.hpp"
#include "error_success_support.hpp"
namespace preloader_l { namespace handlers {
namespace {
bool decimal_id(xml::TextSpan raw,uint64_t* value) noexcept {
    if(!value||!raw.data||raw.size==0||raw.size>20)return false;
    uint64_t n=0;
    for(size_t i=0;i<raw.size;++i){
        const char ch=raw.data[i];if(ch<'0'||ch>'9')return false;
        const uint64_t digit=static_cast<uint64_t>(ch-'0');
        if(n>(UINT64_MAX-digit)/10u)return false;
        n=n*10u+digit;
    }
    if(n==0)return false;
    *value=n;return true;
}
}
// Strict, bounded local-only source grammar. An unsupported multi-recipient
// invite fails closed rather than claiming an incomplete local inbox accept.
bool prepare_local_social_socket_request(const xml::RequestView& request,
    const runtime::LocalSocialSession& session,runtime::LocalSocialPending* pending) noexcept {
    if(!pending||!request.id.data||request.id.size==0)return false;
    runtime::LocalSocialAction action{};
    const char* target_name=nullptr;
    bool invite=false;
    if(xml::span_equals(request.command,"BlockUser")){
        action=runtime::LocalSocialAction::block_user;target_name="UserIdToBlock";
    }else if(xml::span_equals(request.command,"UnblockUser")){
        action=runtime::LocalSocialAction::unblock_user;target_name="UserIdToUnblock";
    }else if(xml::span_equals(request.command,"RequestFriend")){
        action=runtime::LocalSocialAction::request_friend;target_name="UserToAdd";
    }else if(xml::span_equals(request.command,"RemoveFriend")){
        action=runtime::LocalSocialAction::remove_friend;target_name="UserToRemove";
    }else if(xml::span_equals(request.command,"SendInvite")){
        action=runtime::LocalSocialAction::send_invite;invite=true;
    }else return false;
    xml::TextSpan source{};
    if(xml::find_attribute(request.command_open_tag,"UserId",&source)!=xml::AttributeStatus::found)return false;
    uint64_t actor=0;
    if(!decimal_id(source,&actor)||actor!=session.actor)return false;
    uint64_t target=0;
    uint64_t invitee_ids[runtime::LocalSocialStore::edge_limit]{};
    size_t invitee_count=0;
    char invitation_value[1025]{};size_t invitation_n=0;
    if(!invite){
        if(xml::find_attribute(request.command_open_tag,target_name,&source)!=xml::AttributeStatus::found||
           !decimal_id(source,&target))return false;
    }else{
        // Consumer serializer uses Invitation attribute and repeated Invitees
        // children. This is local inbox acceptance only, never remote delivery.
        xml::TextSpan invitation{};
        if(xml::find_attribute(request.command_open_tag,"Invitation",&invitation)!=xml::AttributeStatus::found||
           !invitation.data||invitation.size==0||invitation.size>1024)return false;
        if(xml::decode_source_value(invitation,invitation_value,
            sizeof(invitation_value),&invitation_n)!=xml::ValueCodecStatus::ok||
            invitation_n==0||invitation_n>1024)return false;
        size_t cursor=0;
        for(;;){
            xml::DirectChildView child{};
            const auto rc=xml::next_direct_child(request.command_content,&cursor,&child);
            if(rc==xml::DirectChildStatus::end)break;
            if(rc!=xml::DirectChildStatus::found||!xml::span_equals(child.name,"Invitees")||
               child.self_closing||invitee_count>=runtime::LocalSocialStore::edge_limit)return false;
            if(!decimal_id(child.content,&target))return false;
            for(size_t j=0;j<invitee_count;++j)if(invitee_ids[j]==target)return false;
            invitee_ids[invitee_count++]=target;
        }
        if(invitee_count==0)return false;
    }
    if(invite){
        for(size_t i=0;i<invitee_count;++i)if(invitee_ids[i]==actor)return false;
        return runtime::prepare_local_social_invites(session,actor,invitee_ids,invitee_count,
            invitation_value,invitation_n,pending)==runtime::LocalSocialResult::ok;
    }
    if(target==actor)return false;
    return runtime::prepare_local_social_mutation(session,actor,action,target,pending)==runtime::LocalSocialResult::ok;
}
HandlerStatus render_local_social_success(const xml::RequestView& request,
                                         char* out,size_t cap,size_t* written) noexcept {
    return detail::render_error_success(request,protocol::OriginError::success,
        "",out,cap,written);
}
} }
