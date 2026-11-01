#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>

using namespace preloader_l;

static bool parse(const char* s, xml::RequestView* r) {
    return xml::parse_request_view(s, std::strlen(s), r) == xml::RequestStatus::ok;
}
static bool has(const char* s, const char* needle) { return std::strstr(s, needle) != nullptr; }

template<class F>
static bool call(const char* xml_text, F fn, char* out, size_t cap) {
    xml::RequestView r{}; size_t n=0;
    if (!parse(xml_text, &r)) return false;
    if (fn(r, out, cap, &n) != handlers::HandlerStatus::ok) return false;
    return n > 0 && out[n] == '\0';
}

int main() {
    config::clear_runtime_overrides();
    config::clear_runtime_configuration();
    constexpr char cfg[] = R"CFG("Config2"
{
 "Game" { "Name" "SinkGame" "Version" "7.6.5" "Language" "zz_ZZ" "Languages" "L1,L2" }
 "Emulator" { "Online" "true" "Environment" "sandbox" }
 "User" { "Country" "ZX" "Currency" "ZC" "Username" "BEHAVIORAL_USER" "UserId" "BEHAVIORAL_ID" "PersonaId" "PERS_X" "IsSubscriber" "false" "IsSteamSubscriber" "true" "SubscriberLevel" "3" "IsUnderAge" "true" "AvatarImage" "avx.png" }
}
)CFG";
    if (config::set_runtime_configuration_cfg(cfg, sizeof(cfg)-1) != config::RuntimeConfigurationStatus::ok) return 1;
    if (config::set_runtime_current_directory("C:\\Sink", 7) != config::RuntimeConfigurationStatus::ok) return 2;

    char out[8192]{};
    if (!call("<LSX><Request id=\"p\" recipient=\"EALS\"><GetProfile index=\"7\"/></Request></LSX>", handlers::handle_get_profile, out, sizeof out)) return 10;
    if (!has(out,"sender=\"EALS\"") || !has(out,"CommerceCountry=\"ZX\"") ||
        !has(out,"CommerceCurrency=\"ZC\"") || !has(out,"Persona=\"BEHAVIORAL_USER\"") ||
        !has(out,"UserId=\"BEHAVIORAL_ID\"") || !has(out,"GeoCountry=\"ZX\"") ||
        !has(out,"PersonaId=\"PERS_X\"") || !has(out,"Country=\"ZX\"") ||
        !has(out,"AvatarId=\"\"") || !has(out,"UserIndex=\"7\"") ||
        !has(out,"IsSubscriber=\"false\"") || !has(out,"IsSteamSubscriber=\"true\"") ||
        !has(out,"SubscriberLevel=\"3\"") || !has(out,"IsUnderAge=\"true\"")) return 11;

    // Phase x002 request policy: index is a required XML attribute.
    if (!call("<LSX><Request id=\"pa\"><GetProfile index=\"8\"/></Request></LSX>", handlers::handle_get_profile, out, sizeof out)) return 12;
    if (!has(out,"UserIndex=\"8\"")) return 13;
    if (!call("<LSX><Request id=\"pe\"><GetProfile index=\"&#55;\"/></Request></LSX>", handlers::handle_get_profile, out, sizeof out)) return 113;
    if (!has(out,"UserIndex=\"7\"")) return 114;
    { xml::RequestView rr{}; size_t nn=9; if(!parse("<LSX><Request id=\"pem\"><GetProfile index=\"&bogus;\"/></Request></LSX>",&rr) || handlers::handle_get_profile(rr,out,sizeof out,&nn)!=handlers::HandlerStatus::invalid_request || nn!=0) return 115; }
    { xml::RequestView rr{}; size_t nn=9; if(!parse("<LSX><Request id=\"pm\"><GetProfile/></Request></LSX>",&rr) || handlers::handle_get_profile(rr,out,sizeof out,&nn)!=handlers::HandlerStatus::unsupported || nn!=0) return 14; }
    { xml::RequestView rr{}; size_t nn=9; if(!parse("<LSX><Request id=\"pc2\"><GetProfile><index>8</index></GetProfile></Request></LSX>",&rr) || handlers::handle_get_profile(rr,out,sizeof out,&nn)!=handlers::HandlerStatus::unsupported || nn!=0) return 15; }

    // Phase x003 loaded-config semantics: missing behavioral User keys overwrite
    // constructor defaults with empty strings. Config2 uses behavioral Username/UserId directly; no legacy XML aliases remain.
    config::clear_runtime_configuration();
    constexpr char sparse_cfg[] = R"CFG("Config2" { "User" { "Username" "SPARSE_USER" "UserId" "SPARSE_ID" "PersonaId" "P9" "Country" "TR" } })CFG";
    if (config::set_runtime_configuration_cfg(sparse_cfg, sizeof(sparse_cfg)-1) != config::RuntimeConfigurationStatus::ok) return 16;
    if (!call("<LSX><Request id=\"pc\"><GetProfile index=\"2\"/></Request></LSX>", handlers::handle_get_profile, out, sizeof out)) return 17;
    if (!has(out,"Persona=\"SPARSE_USER\"") || !has(out,"UserId=\"SPARSE_ID\"") ||
        !has(out,"PersonaId=\"P9\"") || !has(out,"Country=\"TR\"") ||
        !has(out,"CommerceCurrency=\"\"") || !has(out,"IsSubscriber=\"\"") ||
        !has(out,"IsSteamSubscriber=\"\"") || !has(out,"SubscriberLevel=\"\"") ||
        !has(out,"IsUnderAge=\"\"")) return 18;

    config::clear_runtime_configuration();
    if (config::set_runtime_configuration_cfg(cfg, sizeof(cfg)-1) != config::RuntimeConfigurationStatus::ok) return 19;

    if (!call("<LSX><Request id=\"a\"><GetAllGameInfo/></Request></LSX>", handlers::handle_get_all_game_info, out, sizeof out)) return 20;
    if (!has(out,"Languages=\"L1,L2\"") || !has(out,"InstalledVersion=\"7.6.5\"") ||
        !has(out,"DisplayName=\"SinkGame\"") || !has(out,"AvailableVersion=\"7.6.5\"")) return 21;

    if (!call("<LSX><Request id=\"r\"><GetPresence/></Request></LSX>", handlers::handle_get_presence, out, sizeof out)) return 30;
    if (!has(out,"UserId=\"0\"") || !has(out,"Title=\"\"") || !has(out,"SessionId=\"\"") || !has(out,"RichPresence=\"SinkGame\"")) return 31;

    if (!call("<LSX><Request id=\"s\"><GetSettings/></Request></LSX>", handlers::handle_runtime_get_settings, out, sizeof out)) return 40;
    if (!has(out,"Environment=\"sandbox\"") || !has(out,"Language=\"zz_ZZ\"")) return 41;

    if (!call("<LSX><Request id=\"i\"><QueryImage><Width>416</Width><Height>208</Height><ImageId>user:BEHAVIORAL_ID</ImageId></QueryImage></Request></LSX>", handlers::handle_query_image, out, sizeof out)) return 50;
    if (!has(out,"ImageId=\"user:BEHAVIORAL_ID\"") || !has(out,"ResourcePath=\"avx.png\"") || !has(out,"Width=\"416\"") || !has(out,"Height=\"208\"")) return 51;

    // Before any configuration is loaded, x003 recovers the behavioral constructor
    // defaults as the persistent profile policy.
    config::clear_runtime_configuration();
    if (!call("<LSX><Request id=\"pd\"><GetProfile index=\"0\"/></Request></LSX>", handlers::handle_get_profile, out, sizeof out)) return 60;
    if (!has(out,"Persona=\"SURGE\"") || !has(out,"UserId=\"1000200030000\"") ||
        !has(out,"PersonaId=\"1144668899\"") || !has(out,"Country=\"US\"") ||
        !has(out,"CommerceCurrency=\"USD\"") || !has(out,"IsSubscriber=\"true\"") ||
        !has(out,"IsSteamSubscriber=\"false\"") || !has(out,"SubscriberLevel=\"2\"") ||
        !has(out,"IsUnderAge=\"false\"")) return 61;

    config::clear_runtime_overrides();
    return 0;
}
