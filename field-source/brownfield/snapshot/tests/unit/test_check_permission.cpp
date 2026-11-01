#include "preloader_l/handlers/permission_route.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;

int main()
{
    constexpr char xml_in[] = "<LSX><Request id=\"16\"><CheckPermission/></Request></LSX>";
    xml::RequestView req{};
    if (xml::parse_request_view(xml_in, sizeof(xml_in)-1, &req) != xml::RequestStatus::ok) return 1;
    const auto* route = router::find_known_route(req.command);
    if (!route || router::descriptor_route_id(*route) != router::RouteId::check_permission || route->original_handler_rva != 0x31c7cu) return 2;

    char out[512]{}; size_t written = 99;
    config::clear_runtime_overrides();
    if (handlers::handle_check_permission(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 3;
    if (!std::strstr(out, "<CheckPermissionResponse Access=\"GRANTED\"/>")) return 4;

    config::set_runtime_online_override(config::literal_value("false"));
    written = 99; out[0] = '\0';
    if (handlers::handle_check_permission(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 5;
    if (!std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"-1576468080\"/>") || std::strstr(out, "CheckPermissionResponse")) return 6;

    config::set_runtime_online_override(config::literal_value("0"));
    written = 99; out[0] = '\0';
    if (handlers::handle_check_permission(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 7;
    if (!std::strstr(out, "<CheckPermissionResponse Access=\"GRANTED\"/>") || std::strstr(out, "ErrorSuccess")) return 8;
    config::clear_runtime_overrides();
    return 0;
}
