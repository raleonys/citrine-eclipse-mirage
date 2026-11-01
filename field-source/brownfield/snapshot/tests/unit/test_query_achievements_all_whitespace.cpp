#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;

static bool parse(const char* s, xml::RequestView* out) {
    return xml::parse_request_view(s,std::strlen(s),out)==xml::RequestStatus::ok;
}
static bool has(const char* s,const char* needle) { return s&&std::strstr(s,needle)!=nullptr; }
static int query(const char* request,bool expect_all) {
    char out[8192]{}; size_t n=0; xml::RequestView r{};
    if(!parse(request,&r)) return 1;
    if(handlers::handle_query_achievements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 2;
    const bool present=has(out,"Id=\"ZERO\"");
    if(present!=expect_all) return 3;
    return 0;
}

int main() {
    config::clear_runtime_configuration();
    constexpr char cfg[]=R"CFG("Config2" {
      "Game" { "Name" "Whitespace Game" }
      "Emulator" { "Online" "true" }
      "Achievements" {
        "AchievementsSet" "SET"
        "AchievementNames" { "ZERO" "Zero" }
        "AchievementTotals" { "ZERO" "5" }
      }
    })CFG";
    if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 10;

    if(query("<LSX><Request id=\"1\"><QueryAchievements/></Request></LSX>",false)) return 11;
    if(query("<LSX><Request id=\"2\"><QueryAchievements><All>true</All></QueryAchievements></Request></LSX>",true)) return 12;
    if(query("<LSX><Request id=\"3\"><QueryAchievements><All>tr&#117;e</All></QueryAchievements></Request></LSX>",true)) return 13;
    if(query("<LSX><Request id=\"4\"><QueryAchievements><All> true </All></QueryAchievements></Request></LSX>",false)) return 14;
    if(query("<LSX><Request id=\"5\"><QueryAchievements><All>true </All></QueryAchievements></Request></LSX>",false)) return 15;
    if(query("<LSX><Request id=\"6\"><QueryAchievements><All> true</All></QueryAchievements></Request></LSX>",false)) return 16;
    if(query("<LSX><Request id=\"7\"><QueryAchievements><All>True</All></QueryAchievements></Request></LSX>",false)) return 17;
    if(query("<LSX><Request id=\"8\"><QueryAchievements><All>1</All></QueryAchievements></Request></LSX>",false)) return 18;
    if(query("<LSX><Request id=\"9\"><QueryAchievements><All></All></QueryAchievements></Request></LSX>",false)) return 19;
    if(query("<LSX><Request id=\"10\"><QueryAchievements><All/></QueryAchievements></Request></LSX>",false)) return 20;
    if(query("<LSX><Request id=\"11\"><QueryAchievements><All> tr&#117;e </All></QueryAchievements></Request></LSX>",false)) return 21;

    // Full normal QueryAchievements serialization can exceed the old offline-error-sized buffer.
    char out[8192]{}; size_t n=9; xml::RequestView nested{};
    if(!parse("<LSX><Request id=\"12\"><QueryAchievements><All><X>true</X></All></QueryAchievements></Request></LSX>",&nested)) return 22;
    if(handlers::handle_query_achievements(nested,out,sizeof out,&n)!=handlers::HandlerStatus::invalid_request || n!=0) return 23;

    config::set_runtime_online_override(config::literal_value("false"));
    xml::RequestView offline{};
    if(!parse("<LSX><Request id=\"13\"><QueryAchievements><All>true</All></QueryAchievements></Request></LSX>",&offline)) return 24;
    n=0; out[0]='\0';
    if(handlers::handle_query_achievements(offline,out,sizeof out,&n)!=handlers::HandlerStatus::ok || !has(out,"Id=\"ZERO\"") || has(out,"Code=\"-1576468080\"")) return 25;
    config::set_runtime_online_override(config::literal_value("0"));
    n=0; out[0]='\0';
    if(handlers::handle_query_achievements(offline,out,sizeof out,&n)!=handlers::HandlerStatus::ok || !has(out,"Id=\"ZERO\"") || has(out,"Code=\"-1576468080\"")) return 26;
    config::clear_runtime_overrides();

    config::clear_runtime_configuration();
    return 0;
}
