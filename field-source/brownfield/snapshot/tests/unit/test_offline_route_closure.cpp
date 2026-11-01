#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
#include <cstdlib>
using namespace preloader_l;

extern "C" void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
extern "C" void* HeapAlloc(void*, unsigned long, std::size_t n){ return std::malloc(n); }
extern "C" int HeapFree(void*, unsigned long, void* p){ std::free(p); return 1; }

static bool parse(const char* s, xml::RequestView* r) {
    return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;
}
static bool has(const char* s,const char* needle) {
    return s && std::strstr(s,needle)!=nullptr;
}

int main() {
    config::clear_runtime_overrides();
    config::clear_runtime_configuration();
    runtime::clear_queued_events();
    constexpr char cfg[]=R"CFG("Config2" {
      "Game" { "Name" "Offline Closure" }
      "Emulator" { "Online" "false" }
      "Achievements" {
        "AchievementsSet" "SET"
        "AchievementNames" { "ZERO" "Zero" "FIRST_WIN" "First Win" }
        "AchievementTotals" { "ZERO" "5" "FIRST_WIN" "1" }
      }
    })CFG";
    if(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=config::RuntimeConfigurationStatus::ok) return 1;

    char out[16384]{}; size_t n=0; xml::RequestView r{};

    if(!parse("<LSX><Request id=\"1\"><QueryOffers/></Request></LSX>",&r)) return 2;
    if(handlers::handle_query_offers(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 3;
    if(!has(out,"<QueryOffersResponse/>") || has(out,"Unknown Error") || has(out,"Code=\"-1\"")) return 4;

    if(!parse("<LSX><Request id=\"2\"><QueryAchievements><All>true</All></QueryAchievements></Request></LSX>",&r)) return 5;
    if(handlers::handle_query_achievements(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 6;
    if(!has(out,"Id=\"ZERO\"") || has(out,"Code=\"-1576468080\"")) return 7;

    if(!parse("<LSX><Request id=\"3\"><GrantAchievement><AchievementId>FIRST_WIN</AchievementId></GrantAchievement></Request></LSX>",&r)) return 8;
    if(handlers::handle_grant_achievement(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 9;
    if(!has(out,"<Achievement Id=\"FIRST_WIN\"") || has(out,"Code=\"-1576468080\"")) return 10;

    runtime::clear_queued_events();
    config::clear_runtime_configuration();
    config::clear_runtime_overrides();
    return 0;
}
