#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
#include <cstdio>
using namespace preloader_l;
static bool parse(const char*s,xml::RequestView*r){return xml::parse_request_view(s,std::strlen(s),r)==xml::RequestStatus::ok;}
static bool has(const char*s,const char*n){return std::strstr(s,n)!=nullptr;}
static bool setting_attr(const char* id,const char* expected){char req[256]{},out[2048]{};std::snprintf(req,sizeof req,"<LSX><Request id=\"s\"><GetSetting SettingId=\"%s\"/></Request></LSX>",id);xml::RequestView r{};size_t n=0;if(!parse(req,&r))return false;if(handlers::handle_runtime_get_setting(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return false;return out[n]=='\0'&&has(out,expected);}
static bool game_attr(const char* id,const char* expected){char req[256]{},out[4096]{};std::snprintf(req,sizeof req,"<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"%s\"/></Request></LSX>",id);xml::RequestView r{};size_t n=0;if(!parse(req,&r))return false;if(handlers::handle_get_game_info(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok)return false;return out[n]=='\0'&&has(out,expected);}
static bool game_miss(const char* body){char out[256]{};xml::RequestView r{};size_t n=9;if(!parse(body,&r))return false;return handlers::handle_get_game_info(r,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0;}
static bool setting_miss(const char* body){char out[256]{};xml::RequestView r{};size_t n=9;if(!parse(body,&r))return false;return handlers::handle_runtime_get_setting(r,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0;}
int main(){
 config::clear_runtime_overrides();config::clear_runtime_configuration();
 constexpr char on[]=R"CFG("Config2" { "Game" { "Name" "Matrix Game" "Version" "9.9.9.9" "Language" "zz_ZZ" "Languages" "L1,L2" } "Emulator" { "Online" "true" "Environment" "sandbox" } })CFG";
 if(config::set_runtime_configuration_cfg(on,sizeof(on)-1)!=config::RuntimeConfigurationStatus::ok)return 1;
 if(!setting_attr("IS_IGO_ENABLED","Setting=\"true\""))return 10;
 if(!setting_attr("IS_IGO_AVAILABLE","Setting=\"true\""))return 11;
 if(!setting_attr("ENVIRONMENT","Setting=\"sandbox\""))return 12;
 if(!setting_attr("LANGUAGE","Setting=\"zz_ZZ\""))return 13;
 if(!setting_attr("IS_TELEMETRY_ENABLED","Setting=\"false\""))return 14;
 if(!setting_attr("IS_MANUAL_OFFLINE","Setting=\"false\""))return 15;
 if(!setting_miss("<LSX><Request id=\"s\"><GetSetting SettingId=\"UNKNOWN_SELECTOR\"/></Request></LSX>"))return 16;
 if(!game_attr("DISPLAY_NAME","GameInfo=\"Matrix Game\""))return 20;
 if(!game_attr("INSTALLED_VERSION","GameInfo=\"9.9.9.9\""))return 21;
 if(!game_attr("AVAILABLE_VERSION","GameInfo=\"9.9.9.9\""))return 22;
 if(!game_attr("FREETRIAL","GameInfo=\"false\""))return 23;
 if(!game_attr("ENTITLEMENT_SOURCE","GameInfo=\"ORIGIN\""))return 24;
 if(!game_attr("EXPIRATION_DURATION","GameInfo=\"36000000\""))return 25;
 if(!game_attr("LANGUAGES","GameInfo=\"L1,L2\""))return 26;
 if(!game_attr("INSTALLED_LANGUAGE","GameInfo=\"zz_ZZ\""))return 27;
 if(!game_miss("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"LANGUAGE\"/></Request></LSX>"))return 28;
 if(!game_miss("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"UNKNOWN_SELECTOR\"/></Request></LSX>"))return 29;
 if(!setting_miss("<LSX><Request id=\"s\"><GetSetting><SettingId>LANGUAGE</SettingId></GetSetting></Request></LSX>"))return 31;
 if(!game_miss("<LSX><Request id=\"g\"><GetGameInfo><GameInfoId>FREETRIAL</GameInfoId></GetGameInfo></Request></LSX>"))return 32;
 config::clear_runtime_configuration();config::clear_runtime_overrides();return 0;
}
