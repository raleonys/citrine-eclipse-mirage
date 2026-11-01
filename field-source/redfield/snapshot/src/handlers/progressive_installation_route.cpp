// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/handlers/progressive_installation_route.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"

namespace preloader_l { namespace handlers {
namespace {
constexpr char command_name[] = "IsProgressiveInstallationAvailable";
constexpr char body[] = "<IsProgressiveInstallationAvailableResponse ItemId=\"\" Available=\"false\"/>";
}

HandlerStatus handle_is_progressive_installation_available(
    const preloader_l::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || (!output && capacity != 0)) return HandlerStatus::invalid_request;
    if (!preloader_l::xml::span_equals(request.command, command_name)) return HandlerStatus::wrong_command;
    if (!request.id.data || request.id.size == 0) return HandlerStatus::invalid_request;

    const preloader_l::xml::TextSpan sender = preloader_l::xml::response_sender_policy(request);
    const preloader_l::xml::TextSpan body_span{body, sizeof(body) - 1};
    const preloader_l::xml::ResponseStatus status = preloader_l::xml::build_response_envelope_from_source_attributes(
        request.id, sender, body_span, output, capacity, written);
    return status == preloader_l::xml::ResponseStatus::ok ? HandlerStatus::ok : HandlerStatus::response_error;
}

} } // namespace preloader_l::handlers
