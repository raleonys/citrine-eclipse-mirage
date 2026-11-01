#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;

int main()
{
    constexpr char in[] = "<LSX><Request id=\"19\"><StartDownload/></Request></LSX>";
    xml::RequestView req{};
    if (xml::parse_request_view(in, sizeof(in)-1, &req) != xml::RequestStatus::ok) return 1;
    const auto* route_desc = router::find_known_route(req.command);
    if (!route_desc || router::descriptor_route_id(*route_desc) != router::RouteId::start_download || route_desc->original_handler_rva != 0x3500cu) return 2;

    char out[512]{};
    size_t written = 0;
    config::clear_runtime_overrides();
    if (handlers::handle_start_download(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 3;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"Requested a download of a DLC offer.\" Code=\"0\"/>")) return 4;

    config::set_runtime_online_override(config::literal_value("false"));
    written = 0;
    out[0] = '\0';
    if (handlers::handle_start_download(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 5;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"Requested a download of a DLC offer.\" Code=\"0\"/>")) return 6;

    config::set_runtime_online_override(config::literal_value("0"));
    written = 0;
    out[0] = '\0';
    if (handlers::handle_start_download(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 7;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"Requested a download of a DLC offer.\" Code=\"0\"/>")) return 8;
    config::clear_runtime_overrides();
    return 0;
}
