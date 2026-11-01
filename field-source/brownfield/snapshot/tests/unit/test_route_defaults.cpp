#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>

using namespace preloader_l;
static bool parse(const char* s, xml::RequestView* r) {
    return xml::parse_request_view(s, std::strlen(s), r) == xml::RequestStatus::ok;
}
static bool has(const char* s, const char* needle) { return std::strstr(s, needle) != nullptr; }
static bool call(const char* request,
                 handlers::HandlerStatus (*fn)(const xml::RequestView&,char*,size_t,size_t*) noexcept,
                 char* out,size_t cap,size_t* n) {
    xml::RequestView r{};
    return parse(request,&r) && fn(r,out,cap,n)==handlers::HandlerStatus::ok;
}

int main() {
    // Settings-only fixture mirrors the user-provided surge.cfg.  Deliberately
    // no Content/Entitlements/Offers/Achievements/Store sections; Online=false.
    constexpr char cfg[] = R"CFG("Config2"
{
 "Game" { "Name" "EA SPORTS College Football 27" "Version" "1.0.139.61806" "Language" "en_US" "Languages" "cs_CZ,da_DK,de_DE,en_US,es_ES,fi_FI,fr_FR,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,ru_RU,sv_SE,zh_CN,zh_TW" }
 "Emulator" { "Online" "false" "Environment" "production" }
 "User" { "Username" "SURGE" "UserId" "1000200030000" "PersonaId" "1144668899" "Country" "US" "AvatarImage" "surge.png" }
}
)CFG";
    if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 1;
    if(config::runtime_configuration_present_sections()!=7u) return 2;
    if(config::set_runtime_current_directory("C:\\Game",7)!=config::RuntimeConfigurationStatus::ok) return 3;

    char out[16384]{}; size_t n=0;

    if(!call("<LSX><Request id=\"1\"><GetProfile index=\"0\"/></Request></LSX>",handlers::handle_get_profile,out,sizeof out,&n)) return 10;
    if(!has(out,"Persona=\"SURGE\"") || !has(out,"UserId=\"1000200030000\"") ||
       !has(out,"IsSubscriber=\"\"") || !has(out,"IsSteamSubscriber=\"\"") ||
       !has(out,"SubscriberLevel=\"\"") || !has(out,"IsUnderAge=\"\"") ||
       !has(out,"CommerceCurrency=\"\"")) return 11;

    if(!call("<LSX><Request id=\"2\"><GetAllGameInfo/></Request></LSX>",handlers::handle_get_all_game_info,out,sizeof out,&n)) return 12;
    if(!has(out,"InstalledVersion=\"1.0.139.61806\"") || !has(out,"FullGamePurchased=\"true\"") ||
       !has(out,"EntitlementSource=\"ORIGIN\"")) return 13;

    if(!call("<LSX><Request id=\"3\"><GetPresence/></Request></LSX>",handlers::handle_get_presence,out,sizeof out,&n)) return 14;
    if(!has(out,"UserId=\"0\"") || !has(out,"Title=\"\"") || !has(out,"RichPresence=\"EA SPORTS College Football 27\"")) return 15;

    if(!call("<LSX><Request id=\"4\"><QueryImage><Width>208</Width><Height>208</Height><ImageId>user:1000200030000</ImageId></QueryImage></Request></LSX>",handlers::handle_query_image,out,sizeof out,&n)) return 16;
    if(!has(out,"ImageId=\"user:1000200030000\"") || !has(out,"ResourcePath=\"surge.png\"") || !has(out,"Width=\"208\"")) return 17;
    if(!call("<LSX><Request id=\"4b\"><QueryImage><Width>-208</Width><Height>208</Height><ImageId>plain</ImageId></QueryImage></Request></LSX>",handlers::handle_query_image,out,sizeof out,&n)) return 18;
    if(!has(out,"ImageId=\"plain\"") || !has(out,"ResourcePath=\"surge.png\"") || !has(out,"Width=\"-208\"")) return 19;

    // Missing Content and Entitlements are empty successful collection responses.
    if(!call("<LSX><Request id=\"5\"><QueryContent/></Request></LSX>",handlers::handle_query_content,out,sizeof out,&n)) return 18;
    if(!has(out,"<QueryContentResponse")) return 19;
    if(!call("<LSX><Request id=\"6\"><QueryEntitlements/></Request></LSX>",handlers::handle_query_entitlements,out,sizeof out,&n)) return 20;
    if(!has(out,"<QueryEntitlementsResponse")) return 21;

    if(!call("<LSX><Request id=\"7\"><QueryOffers/></Request></LSX>",handlers::handle_query_offers,out,sizeof out,&n)) return 22;
    if(!has(out,"<QueryOffersResponse/>") || has(out,"Unknown Error") || has(out,"Code=\"-1\"")) return 23;

    if(!call("<LSX><Request id=\"8\"><QueryAchievements/></Request></LSX>",handlers::handle_query_achievements,out,sizeof out,&n)) return 24;
    if(has(out,"Code=\"-1576468080\"")) return 25;

    { xml::RequestView ga{}; size_t gn=9; if(!parse("<LSX><Request id=\"9m\"><GrantAchievement/></Request></LSX>",&ga) || handlers::handle_grant_achievement(ga,out,sizeof out,&gn)!=handlers::HandlerStatus::unsupported || gn!=0) return 26; }
    if(!call("<LSX><Request id=\"9\"><GrantAchievement><AchievementId>FIRST_WIN</AchievementId></GrantAchievement></Request></LSX>",handlers::handle_grant_achievement,out,sizeof out,&n)) return 27;
    if(!has(out,"<Achievement Id=\"FIRST_WIN\"") || has(out,"Code=\"-1576468080\"")) return 28;

    if(!call("<LSX><Request id=\"10\"><GetAuthCode ClientId=\"client\"/></Request></LSX>",handlers::handle_get_auth_code,out,sizeof out,&n)) return 28;
    if(!has(out,"You need to be online to get an AuthCode.") || !has(out,"Code=\"-1577058293\"")) return 29;


    // Minimal Auth8 adaptation: online GetAuthCode returns a session-stable 8-char alphanumeric code; request ClientId is required by presence only (empty is accepted), and Config2 AuthCode is ignored.
    config::clear_runtime_configuration();
    constexpr char defaults_cfg[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG";
    if(config::set_runtime_configuration_cfg(defaults_cfg,sizeof(defaults_cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 40;
    if(!call("<LSX><Request id=\"12\"><GetProfile index=\"0\"/></Request></LSX>",handlers::handle_get_profile,out,sizeof out,&n)) return 41;
    if(!has(out,"Persona=\"\"") || !has(out,"UserId=\"\"") || !has(out,"PersonaId=\"\"") ||
       !has(out,"Country=\"\"") || !has(out,"CommerceCurrency=\"\"") ||
       !has(out,"IsSubscriber=\"\"") || !has(out,"SubscriberLevel=\"\"")) return 42;
    if(!call("<LSX><Request id=\"13\"><GetAuthCode ClientId=\"client\"/></Request></LSX>",handlers::handle_get_auth_code,out,sizeof out,&n)) return 43;
    { const char* a=std::strstr(out,"<AuthCode value=\""); if(!a) return 44; a+=17; for(size_t i=0;i<8;++i){const char c=a[i];if(!((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z')))return 44;} if(a[8]!='\"')return 44; }

    // With no configuration loaded, the behavioral constructor defaults are visible.
    config::clear_runtime_configuration();
    if(!call("<LSX><Request id=\"14\"><GetProfile index=\"0\"/></Request></LSX>",handlers::handle_get_profile,out,sizeof out,&n)) return 45;
    if(!has(out,"Persona=\"SURGE\"") || !has(out,"UserId=\"1000200030000\"") ||
       !has(out,"PersonaId=\"1144668899\"") || !has(out,"Country=\"US\"") ||
       !has(out,"CommerceCurrency=\"USD\"") || !has(out,"IsSubscriber=\"true\"") ||
       !has(out,"IsSteamSubscriber=\"false\"") || !has(out,"SubscriberLevel=\"2\"") ||
       !has(out,"IsUnderAge=\"false\"")) return 46;

    return 0;
}
