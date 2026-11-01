#include "preloader_l/handlers/social_mutation_routes.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/protocol/origin_error.hpp"
#include "error_success_support.hpp"

namespace preloader_l { namespace handlers {
namespace {
// Deliberate negative-only W03 ingress boundary. No trusted per-session
// principal lease exists at shipping socket ingress, so no remote request
// is authorized to manipulate the bounded local SocialState model. Never
// turn request.UserId or socket peer address into an authority token.
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
    *value=n;
    return true;
}
HandlerStatus denied(const xml::RequestView& request,const char* expected,
                     const char* target_key,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0;
    if(!xml::span_equals(request.command,expected))return HandlerStatus::wrong_command;
    if(!written||(!out&&cap!=0)||!request.id.data||!request.id.size)return HandlerStatus::invalid_request;
    xml::TextSpan actor{},target{};
    if(xml::find_attribute(request.command_open_tag,"UserId",&actor)!=xml::AttributeStatus::found||
       xml::find_attribute(request.command_open_tag,target_key,&target)!=xml::AttributeStatus::found)
        return HandlerStatus::invalid_request;
    uint64_t id=0,dest=0;
    if(!decimal_id(actor,&id)||!decimal_id(target,&dest)||id==dest)return HandlerStatus::invalid_request;
    // ErrorSuccess is explicitly NEGATIVE. Code=0 would falsely claim that
    // a local or remote social mutation happened, which this path cannot prove.
    return detail::render_error_success(request,protocol::OriginError::not_logged_in,
        "Local social principal is not authenticated.",out,cap,written);
}
}

HandlerStatus handle_block_user(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
    return denied(r,"BlockUser","UserIdToBlock",o,c,w);
}
HandlerStatus handle_unblock_user(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
    return denied(r,"UnblockUser","UserIdToUnblock",o,c,w);
}
HandlerStatus handle_request_friend(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
    return denied(r,"RequestFriend","UserToAdd",o,c,w);
}
HandlerStatus handle_remove_friend(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
    return denied(r,"RemoveFriend","UserToRemove",o,c,w);
}
HandlerStatus handle_send_invite(const xml::RequestView& r,char* o,size_t c,size_t* w) noexcept {
    // No local inbox ACK/recipient binding and invitation-list parser in
    // production yet: do not report any queued or delivered invitation.
    if(w)*w=0;
    if(!xml::span_equals(r.command,"SendInvite"))return HandlerStatus::wrong_command;
    if(!w||(!o&&c!=0)||!r.id.data||!r.id.size)return HandlerStatus::invalid_request;
    xml::TextSpan actor{};
    if(xml::find_attribute(r.command_open_tag,"UserId",&actor)!=xml::AttributeStatus::found)
        return HandlerStatus::invalid_request;
    uint64_t id=0;
    if(!decimal_id(actor,&id))return HandlerStatus::invalid_request;
    return detail::render_error_success(r,protocol::OriginError::not_logged_in,
        "Local social principal is not authenticated.",o,c,w);
}
} }
