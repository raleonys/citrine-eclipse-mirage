// SPDX-License-Identifier: GPL-3.0-only
// RF12 exact permission formatter, caller-normalized Online policy.
#pragma once
#include "preloader_l/xml/response_builder.hpp"
#include "../handlers/error_success_support.hpp"
namespace preloader_l { namespace handlers { namespace permission_detail { namespace {
constexpr char command_name[] = "CheckPermission";
constexpr char granted_body[] = "<CheckPermissionResponse Access=\"GRANTED\"/>";

HandlerStatus render_body(const preloader_l::xml::RequestView& request,const char* body,size_t body_size,char* output,size_t capacity,size_t* written) noexcept {
    const auto sender=preloader_l::xml::response_sender_policy(request);
    const auto status=preloader_l::xml::build_response_envelope_from_source_attributes(request.id,sender,{body,body_size},output,capacity,written);
    return status==preloader_l::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
HandlerStatus format_check_permission(const preloader_l::xml::RequestView& request,bool online,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!preloader_l::xml::span_equals(request.command,command_name))return HandlerStatus::wrong_command;
    if(!request.id.data||request.id.size==0)return HandlerStatus::invalid_request;
    if(!online)return detail::render_error_success(
        request,route_error_contracts::check_permission_offline,output,capacity,written);
    return render_body(request,granted_body,sizeof(granted_body)-1u,output,capacity,written);
}
} } } }
