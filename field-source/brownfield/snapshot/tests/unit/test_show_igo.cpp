#include "preloader_l/handlers/integration_routes.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include <cstdlib>
#include <cstring>
using namespace preloader_l;

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*,unsigned long,void* p){ std::free(p); return 1; }

namespace preloader_l { namespace config {
ValueView runtime_configuration_setting(const char*) noexcept { return missing_value(); }
} }

int main()
{
    constexpr char in[] = "<LSX><Request id=\"18\"><ShowIGO/></Request></LSX>";
    xml::RequestView req{};
    if (xml::parse_request_view(in, sizeof(in)-1, &req) != xml::RequestStatus::ok) return 1;
    const auto* route_desc = router::find_known_route(req.command);
    if (!route_desc || router::descriptor_route_id(*route_desc) != router::RouteId::show_igo || route_desc->original_handler_rva != 0x3500cu) return 2;

    char out[512]{};
    size_t written = 0;
    runtime::clear_queued_events_for_test();
    config::clear_runtime_overrides();
    if (handlers::handle_show_igo(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 3;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"0\"/>")) return 4;
    if (runtime::queued_event_count_for_test() != 1u) return 7;
    { char event_out[256]{}; size_t event_n=0; if(!runtime::drain_one_queued_event(event_out,sizeof event_out,&event_n) || !std::strstr(event_out,"<IGOEvent State=\"DOWN\"/>")) return 8; }


    constexpr char alias_in[] = "<LSX><Request id=\"19\"><ShowIGOWindow WindowId=\"42\" RichPayload=\"ignored\"/></Request></LSX>";
    xml::RequestView alias_req{};
    if (xml::parse_request_view(alias_in, sizeof(alias_in)-1, &alias_req) != xml::RequestStatus::ok) return 13;
    const auto* alias_desc = router::find_known_route(alias_req.command);
    if (!alias_desc || alias_desc == route_desc || router::descriptor_route_id(*alias_desc) != router::RouteId::show_igo_window) return 14;
    runtime::clear_queued_events_for_test();
    config::clear_runtime_overrides();
    written = 0;
    out[0] = '\0';
    if (handlers::handle_show_igo(alias_req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 15;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"0\"/>")) return 16;
    if (runtime::queued_event_count_for_test() != 1u) return 17;
    runtime::clear_queued_events_for_test();

    config::set_runtime_online_override(config::literal_value("false"));
    written = 0;
    out[0] = '\0';
    if (handlers::handle_show_igo(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 5;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"0\"/>")) return 6;
    if (runtime::queued_event_count_for_test() != 1u) return 9;
    { char event_out[256]{}; size_t event_n=0; if(!runtime::drain_one_queued_event(event_out,sizeof event_out,&event_n) || !std::strstr(event_out,"<IGOEvent State=\"DOWN\"/>")) return 18; }

    // Alias follows the same connectivity-independent route contract.
    runtime::clear_queued_events_for_test();
    written = 0;
    out[0] = '\0';
    if (handlers::handle_show_igo(alias_req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 19;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"0\"/>")) return 20;
    if (runtime::queued_event_count_for_test() != 1u) return 21;
    runtime::clear_queued_events_for_test();

    config::set_runtime_online_override(config::literal_value("0"));
    written = 0;
    out[0] = '\0';
    if (handlers::handle_show_igo(req, out, sizeof out, &written) != handlers::HandlerStatus::ok) return 10;
    if (written == 0 || !std::strstr(out, "<ErrorSuccess Description=\"\" Code=\"0\"/>")) return 11;
    if (runtime::queued_event_count_for_test() != 1u) return 12;
    runtime::clear_queued_events_for_test();

    // Measure/size-query must not publish an event.
    config::set_runtime_online_override(config::literal_value("false"));
    written = 0;
    if (handlers::handle_show_igo(req, nullptr, 0, &written) != handlers::HandlerStatus::response_error || written == 0) return 22;
    if (runtime::queued_event_count_for_test() != 0u) return 23;
    config::clear_runtime_overrides();
    return 0;
}
