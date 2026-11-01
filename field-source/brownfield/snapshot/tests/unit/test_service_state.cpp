#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;

static bool parse(const char* s, xml::RequestView* out) {
    return xml::parse_request_view(s,std::strlen(s),out)==xml::RequestStatus::ok;
}
static bool has(const char* s,const char* needle) { return s&&std::strstr(s,needle)!=nullptr; }

int main() {
    config::clear_runtime_configuration();
    constexpr char cfg[]=R"CFG("Config2" {
      "Game" { "Name" "Consistency Game" "Version" "7.0" }
      "Achievements" {
        "AchievementsSet" "SET-1"
        "AchievementNames" { "A1" "First" "A2" "Second" }
        "AchievementTotals" { "A1" "3" "A2" "2" }
      }
      "Content" {
        "OFFER_A" { "Name" "Game A" "Version" "1" "State" "INSTALLED" }
      }
      "Entitlements" {
        "OFFER_A" { "Group" "G1" "Version" "11" "Type" "DEFAULT" "EntitlementTag" "ONLINE_ACCESS" }
        "OFFER_B" { "Group" "G2" "Version" "12" "Type" "DLC" "EntitlementTag" "ITEM_B" }
      }
    })CFG";
    if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 1;

    char out[8192]{}; size_t n=0; xml::RequestView r{};
    if(!parse("<LSX><Request id=\"1\"><QueryContent/></Request></LSX>",&r)) return 2;
    if(handlers::handle_query_content(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 3;
    if(!has(out,"contentID=\"OFFER_A\"")||!has(out,"availableVersion=\"1\"")||!has(out,"installedVersion=\"1\"")) return 4;

    if(!parse("<LSX><Request id=\"2\"><QueryEntitlements/></Request></LSX>",&r)) return 5;
    if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 6;
    if(!has(out,"ItemId=\"OFFER_A\"")||!has(out,"EntitlementId=\"1000000000001\"")||!has(out,"ItemId=\"OFFER_B\"")||!has(out,"EntitlementId=\"1000000000002\"")) return 7;

    // Filtered policy records still consume their synthetic sequence slot.
    if(!parse("<LSX><Request id=\"3\"><QueryEntitlements OfferId=\"OFFER&#95;B\"/></Request></LSX>",&r)) return 8;
    if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 9;
    if(has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\"")||!has(out,"EntitlementId=\"1000000000002\"")) return 10;

    // F1585 current policy: only direct text-bearing FilterOffers/FilterGroups/FilterItems
    // children are filters. Nested arbitrary children are ignored and must not gain policy.
    if(!parse("<LSX><Request id=\"3c\"><QueryEntitlements><FilterOffers><OfferId>OFFER&#95;B</OfferId></FilterOffers></QueryEntitlements></Request></LSX>",&r)) return 101;
    if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 102;
    if(!has(out,"ItemId=\"OFFER_A\"")||!has(out,"ItemId=\"OFFER_B\"")) return 103;
    if(!parse("<LSX><Request id=\"3m\"><QueryEntitlements OfferId=\"OFFER&bogus;B\"/></Request></LSX>",&r)) return 104;
    if(handlers::handle_query_entitlements(r,out,sizeof out,&n)!=handlers::HandlerStatus::invalid_request) return 105;

    if(!parse("<LSX><Request id=\"4\"><QueryAchievements/></Request></LSX>",&r)) return 11;
    if(handlers::handle_query_achievements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 12;
    if(!has(out,"AchievementSet GameName=\"Consistency Game\" Name=\"SET-1\"/>")||has(out,"Id=\"A1\"")) return 13;

    if(!parse("<LSX><Request id=\"5\"><QueryAchievements><All>tr&#117;e</All></QueryAchievements></Request></LSX>",&r)) return 14;
    if(handlers::handle_query_achievements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 15;
    if(!has(out,"Id=\"A1\" Total=\"3\" Count=\"0\" Progress=\"0\"")||!has(out,"Id=\"A2\" Total=\"2\" Count=\"0\" Progress=\"0\"")) return 16;

    if(!parse("<LSX><Request id=\"6\"><GrantAchievement><AchievementId>A&#49;</AchievementId><Progress>&#50;</Progress></GrantAchievement></Request></LSX>",&r)) return 17;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 18;
    if(!has(out,"Id=\"A1\" Total=\"3\" Count=\"0\" Progress=\"2\" GrantDate=\"")) return 19;
    if(!parse("<LSX><Request id=\"6m\"><GrantAchievement><AchievementId>A1</AchievementId><Progress>&bogus;</Progress></GrantAchievement></Request></LSX>",&r)) return 106;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::invalid_request) return 107;

    if(!parse("<LSX><Request id=\"7\"><GrantAchievement><AchievementId>A1</AchievementId></GrantAchievement></Request></LSX>",&r)) return 20;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 21;
    if(!has(out,"Id=\"A1\" Total=\"3\" Count=\"1\" Progress=\"3\"")) return 22;

    // Phases 350-362 correction: a missing record granted with Progress=0 is
    // inserted as Total=0/Count=0/Progress=0 and skips the mutation/completion
    // branch because Progress is already >= Total at the gate.
    if(!parse("<LSX><Request id=\"8\"><GrantAchievement><AchievementId>ZERO</AchievementId><Progress>0</Progress></GrantAchievement></Request></LSX>",&r)) return 23;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 24;
    if(!has(out,"Id=\"ZERO\" Total=\"0\" Count=\"0\" Progress=\"0\" GrantDate=\"0000-00-00T00:00:00\"")) return 25;

    // The curio Progress parser is signed 32-bit. Negative increments are
    // accepted and serialized as signed values; they do not synthesize completion.
    if(!parse("<LSX><Request id=\"9\"><GrantAchievement><AchievementId>NEG</AchievementId><Progress>-2</Progress></GrantAchievement></Request></LSX>",&r)) return 26;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 27;
    if(!has(out,"Id=\"NEG\" Total=\"-2\" Count=\"0\" Progress=\"0\"")) return 28;

    config::clear_runtime_configuration();
    return 0;
}
