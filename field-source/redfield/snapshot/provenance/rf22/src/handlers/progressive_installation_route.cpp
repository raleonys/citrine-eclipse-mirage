// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/handlers/progressive_installation_route.hpp"
#include "redfield/xml/request_parser.hpp"
#include "redfield/xml/response_builder.hpp"

namespace redfield { namespace handlers {
namespace {
constexpr char command_name[] = "IsProgressiveInstallationAvailable";
constexpr char body[] = "<IsProgressiveInstallationAvailableResponse ItemId=\"\" Available=\"false\"/>";
}

HandlerStatus handle_is_progressive_installation_available(
    const redfield::xml::RequestView& request,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || (!output && capacity != 0)) return HandlerStatus::invalid_request;
    if (!redfield::xml::span_equals(request.command, command_name)) return HandlerStatus::wrong_command;
    if (!request.id.data || request.id.size == 0) return HandlerStatus::invalid_request;

    const redfield::xml::TextSpan sender = redfield::xml::response_sender_policy(request);
    const redfield::xml::TextSpan body_span{body, sizeof(body) - 1};
    const redfield::xml::ResponseStatus status = redfield::xml::build_response_envelope_from_source_attributes(
        request.id, sender, body_span, output, capacity, written);
    return status == redfield::xml::ResponseStatus::ok ? HandlerStatus::ok : HandlerStatus::response_error;
}

} } // namespace redfield::handlers
