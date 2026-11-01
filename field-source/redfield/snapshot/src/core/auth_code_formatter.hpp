// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "response_formatter_support.hpp"
#include "../handlers/error_success_support.hpp"
namespace preloader_l { namespace {
HandlerStatus check_auth_code_request(const preloader_l::xml::RequestView& r,size_t* written) noexcept {
    if(!eq(r.command,"GetAuthCode")){if(written)*written=0;return HandlerStatus::wrong_command;}
    preloader_l::xml::TextSpan client_id{};
    const auto client_id_status=preloader_l::xml::find_attribute(r.command_open_tag,"ClientId",&client_id);
    if(client_id_status==preloader_l::xml::AttributeStatus::malformed||client_id_status==preloader_l::xml::AttributeStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(client_id_status!=preloader_l::xml::AttributeStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}
    // Approved RC04 remediation: ClientId participates only as a required
    // request attribute. A present-empty value remains valid and the value is
    // deliberately excluded from Auth8 generation/entropy and backend policy.
    (void)client_id;
    return HandlerStatus::ok;
}
HandlerStatus render_auth_offline(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    namespace detail=preloader_l::handlers::detail;
    namespace route_error_contracts=preloader_l::handlers::route_error_contracts;
    return detail::render_error_success(r,route_error_contracts::get_auth_code_offline,out,cap,written);
}
HandlerStatus render_auth_code(const preloader_l::xml::RequestView& r,Span auth,char* out,size_t cap,size_t* written) noexcept {
    if(!safe_semantic_attr_value({auth.data,auth.size}))return HandlerStatus::response_error;
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <AuthCode value=\""),semantic_span(auth),lit("\"/>\n  </Response>\n </LSX>\n")};
    const auto status=format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
    return status;
}
} }
