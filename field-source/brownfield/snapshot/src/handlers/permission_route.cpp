#include "preloader_l/handlers/permission_route.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "error_success_support.hpp"

namespace preloader_l { namespace handlers {
namespace {
constexpr char command_name[] = "CheckPermission";
constexpr char granted_body[] = "<CheckPermissionResponse Access=\"GRANTED\"/>";

bool offline() noexcept {
    preloader_l::config::ValueView online=preloader_l::config::missing_value();
    const auto& provider=preloader_l::config::runtime_settings_provider();
    if(!preloader_l::config::lookup_setting(provider,preloader_l::config::SettingKey::online,&online))
        online=preloader_l::config::literal_value("true");
    return !preloader_l::config::online_enabled(online);
}
HandlerStatus render_body(const preloader_l::xml::RequestView& request,const char* body,size_t body_size,char* output,size_t capacity,size_t* written) noexcept {
    const auto sender=preloader_l::xml::response_sender_policy(request);
    const auto status=preloader_l::xml::build_response_envelope_from_source_attributes(request.id,sender,{body,body_size},output,capacity,written);
    return status==preloader_l::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}
}
HandlerStatus handle_check_permission(const preloader_l::xml::RequestView& request,char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity!=0))return HandlerStatus::invalid_request;
    if(!preloader_l::xml::span_equals(request.command,command_name))return HandlerStatus::wrong_command;
    if(!request.id.data||request.id.size==0)return HandlerStatus::invalid_request;
    if(offline())return detail::render_error_success(
        request,route_error_contracts::check_permission_offline,output,capacity,written);
    return render_body(request,granted_body,sizeof(granted_body)-1u,output,capacity,written);
}
} }
