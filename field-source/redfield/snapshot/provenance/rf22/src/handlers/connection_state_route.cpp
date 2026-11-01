// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/connection_state_route.hpp"
#include "redfield/config/settings_values.hpp"
#include "redfield/xml/request_parser.hpp"
#include "redfield/xml/response_builder.hpp"

namespace redfield { namespace handlers {
HandlerStatus format_get_internet_connected_state(
    const redfield::xml::RequestView& request,
    bool connected,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written=0;
    if (!written || (!output && capacity!=0)) return HandlerStatus::invalid_request;
    if (!redfield::xml::span_equals(request.command,"GetInternetConnectedState")) return HandlerStatus::wrong_command;
    if (!request.id.data || request.id.size==0) return HandlerStatus::invalid_request;
    constexpr char body_true[]="<InternetConnectedState connected=\"true\"/>";
    constexpr char body_false[]="<InternetConnectedState connected=\"false\"/>";
    const char* body=connected?body_true:body_false;
    const size_t body_len=connected?(sizeof(body_true)-1):(sizeof(body_false)-1);
    const redfield::xml::TextSpan sender = redfield::xml::response_sender_policy(request);
    const redfield::xml::TextSpan body_span{body,body_len};
    const auto status=redfield::xml::build_response_envelope_from_source_attributes(request.id,sender,body_span,output,capacity,written);
    return status==redfield::xml::ResponseStatus::ok?HandlerStatus::ok:HandlerStatus::response_error;
}

HandlerStatus handle_get_internet_connected_state(
    const redfield::xml::RequestView& request,
    const redfield::config::SettingsProvider& provider,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    redfield::config::ValueView online=redfield::config::missing_value();
    (void)redfield::config::lookup_setting(provider,redfield::config::SettingKey::online,&online);
    return format_get_internet_connected_state(request,redfield::config::online_enabled(online),output,capacity,written);
}
} }
