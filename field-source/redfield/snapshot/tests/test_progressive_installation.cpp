// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/handlers/progressive_installation_route.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"

#include <cstring>

using namespace preloader_l;

int main()
{
    constexpr char request_xml[] =
        "<LSX><Request id=\"314\" sender=\"Client\">"
        "<IsProgressiveInstallationAvailable ItemId=\"ignored\"/>"
        "</Request></LSX>";
    xml::RequestView request{};
    if (xml::parse_request_view(request_xml, sizeof(request_xml) - 1, &request) != xml::RequestStatus::ok) return 1;

    const router::RouteDescriptor* route = router::find_known_route(request.command);
    if (!route || router::descriptor_route_id(*route) != router::RouteId::is_progressive_installation_available) return 2;
    if (route->original_handler_rva != 0x29990u) return 3;

    char output[512]{};
    size_t written = 0;
    if (handlers::handle_is_progressive_installation_available(request, output, sizeof output, &written) != handlers::HandlerStatus::ok) return 4;

    constexpr char expected[] =
        " <LSX>\n"
        "  <Response id=\"314\" sender=\"EbisuSDK\">\n"
        "    <IsProgressiveInstallationAvailableResponse ItemId=\"\" Available=\"false\"/>\n"
        "  </Response>\n"
        " </LSX>\n";
    if (written != sizeof(expected) - 1 || std::strcmp(output, expected) != 0) return 5;

    // The curio template is static: the request's ItemId is not copied to
    // the response and Available remains false.
    if (std::strstr(output, "ignored") != nullptr) return 6;

    xml::RequestView wrong = request;
    constexpr char wrong_name[] = "GetProfile";
    wrong.command = {wrong_name, sizeof(wrong_name) - 1};
    if (handlers::handle_is_progressive_installation_available(wrong, output, sizeof output, &written) != handlers::HandlerStatus::wrong_command) return 7;

    xml::RequestView missing_id = request;
    missing_id.id = {nullptr, 0};
    if (handlers::handle_is_progressive_installation_available(missing_id, output, sizeof output, &written) != handlers::HandlerStatus::invalid_request) return 8;

    char tiny[8];
    for (char& c : tiny) c = '#';
    if (handlers::handle_is_progressive_installation_available(request, tiny, sizeof tiny, &written) != handlers::HandlerStatus::response_error) return 9;
    if (written != sizeof(expected) - 1) return 10;
    for (char c : tiny) if (c != '#') return 11;

    return 0;
}
