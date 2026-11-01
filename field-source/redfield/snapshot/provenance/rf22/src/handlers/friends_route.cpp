// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/friends_route.hpp"
#include "redfield/xml/request_parser.hpp"
#include "redfield/xml/response_builder.hpp"

namespace redfield { namespace handlers {
namespace {
constexpr char command_name[] = "QueryFriends";
constexpr char success_body[] = "<QueryFriendsResponse Return=\"Success\"/>";
HandlerStatus render_body(const redfield::xml::RequestView& request,const char* body,size_t body_size,char* output,size_t capacity,size_t* written) noexcept {
    const auto sender=redfield::xml::response_sender_policy(request);
    const auto status=redfield::xml::build_response_envelope_from_source_attributes(request.id,sender,{body,body_size},output,capacity,written);
    return status==redfield::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
}
HandlerStatus handle_query_friends(const redfield::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!redfield::xml::span_equals(request.command,command_name))return HandlerStatus::wrong_command;
    if(!request.id.data||request.id.size==0)return HandlerStatus::invalid_request;
    // Donor1 exact contract: success-only response with zero Friend children,
    // independent of the project Online setting. Do not synthesize friend state.
    return render_body(request,success_body,sizeof(success_body)-1u,output,capacity,written);
}
} }
