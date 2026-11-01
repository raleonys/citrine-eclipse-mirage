#include "preloader_l/handlers/connection_state_route.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"

namespace preloader_l { namespace handlers {
HandlerStatus format_get_internet_connected_state(
    const preloader_l::xml::RequestView& request,
    bool connected,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written=0;
    if (!written || (!output && capacity!=0)) return HandlerStatus::invalid_request;
    if (!preloader_l::xml::span_equals(request.command,"GetInternetConnectedState")) return HandlerStatus::wrong_command;
    if (!request.id.data || request.id.size==0) return HandlerStatus::invalid_request;
    constexpr char body_true[]="<InternetConnectedState connected=\"true\"/>";
    constexpr char body_false[]="<InternetConnectedState connected=\"false\"/>";
    const char* body=connected?body_true:body_false;
    const size_t body_len=connected?(sizeof(body_true)-1):(sizeof(body_false)-1);
    const preloader_l::xml::TextSpan sender = preloader_l::xml::response_sender_policy(request);
    const preloader_l::xml::TextSpan body_span{body,body_len};
    const auto status=preloader_l::xml::build_response_envelope_from_source_attributes(request.id,sender,body_span,output,capacity,written);
    return status==preloader_l::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}

HandlerStatus handle_get_internet_connected_state(
    const preloader_l::xml::RequestView& request,
    const preloader_l::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    preloader_l::config::ValueView online=preloader_l::config::missing_value();
    (void)preloader_l::config::lookup_setting(provider,preloader_l::config::SettingKey::online,&online);
    return format_get_internet_connected_state(request,preloader_l::config::online_enabled(online),output,capacity,written);
}
} }
