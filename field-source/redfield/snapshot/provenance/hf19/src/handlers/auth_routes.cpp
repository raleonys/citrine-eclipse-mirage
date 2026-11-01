#include "preloader_l/handlers/auth_routes.hpp"
#include "route_response_support.hpp"
#include "error_success_support.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus handle_get_auth_code(const preloader_l::xml::RequestView& r,char* out,size_t cap,size_t* written) noexcept {
    if(!eq(r.command,"GetAuthCode")){if(written)*written=0;return HandlerStatus::wrong_command;}
    preloader_l::xml::TextSpan client_id{};
    const auto client_id_status=preloader_l::xml::find_attribute(r.command_open_tag,"ClientId",&client_id);
    if(client_id_status==preloader_l::xml::AttributeStatus::malformed||client_id_status==preloader_l::xml::AttributeStatus::invalid_argument){if(written)*written=0;return HandlerStatus::invalid_request;}
    if(client_id_status!=preloader_l::xml::AttributeStatus::found){if(written)*written=0;return HandlerStatus::unsupported;}
    // Approved RC04 remediation: ClientId participates only as a required
    // request attribute. A present-empty value remains valid and the value is
    // deliberately excluded from Auth8 generation/entropy and backend policy.
    (void)client_id;
    if(offline()) return detail::render_error_success(r,route_error_contracts::get_auth_code_offline,out,cap,written);
    preloader_l::runtime::AuthCodeSessionLease lease{};
    if(!preloader_l::runtime::begin_session_auth_code_lease(&lease)){if(written)*written=0;return HandlerStatus::response_error;}
    const auto generated=preloader_l::runtime::session_auth_code_lease_value(&lease);
    const Span auth{generated.data,generated.size};
    if(!safe_semantic_attr_value({auth.data,auth.size})){preloader_l::runtime::finish_session_auth_code_lease(&lease,false);return HandlerStatus::response_error;}
    const Span p[]={lit(" <LSX>\n  <Response id=\""),source_span(r.id),lit("\" sender=\""),response_sender_span(r),lit("\">\n    <AuthCode value=\""),semantic_span(auth),lit("\"/>\n  </Response>\n </LSX>\n")};
    const auto status=format_parts(r,p,sizeof(p)/sizeof(p[0]),out,cap,written);
    preloader_l::runtime::finish_session_auth_code_lease(&lease,status==HandlerStatus::ok);
    return status;
}

} }
