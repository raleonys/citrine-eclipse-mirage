#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
static xml::RequestView parse(const char* s){xml::RequestView r{}; assert(xml::parse_request_view(s,std::strlen(s),&r)==xml::RequestStatus::ok); return r;}
static void strict_setting(const char* req,const char* value){char out[4096]{}; size_t n=0; auto r=parse(req); assert(handlers::handle_runtime_get_setting(r,out,sizeof out,&n)==handlers::HandlerStatus::ok); char expected[4096]{}; const char* a=" <LSX>\n  <Response id=\"s\" sender=\"EbisuSDK\">\n    <GetSettingResponse Setting=\""; const char* b="\"/>\n  </Response>\n </LSX>\n"; std::strcpy(expected,a); std::strcat(expected,value); std::strcat(expected,b); assert(n==std::strlen(expected)); assert(std::strcmp(out,expected)==0);}
static void strict_game(const char* req,const char* value){char out[4096]{}; size_t n=0; auto r=parse(req); assert(handlers::handle_get_game_info(r,out,sizeof out,&n)==handlers::HandlerStatus::ok); char expected[4096]{}; const char* a=" <LSX>\n  <Response id=\"g\" sender=\"EbisuSDK\">\n    <GetGameInfoResponse GameInfo=\""; const char* b="\"/>\n  </Response>\n </LSX>\n"; std::strcpy(expected,a); std::strcat(expected,value); std::strcat(expected,b); assert(n==std::strlen(expected)); assert(std::strcmp(out,expected)==0);}
int main(){
 config::clear_runtime_overrides(); config::clear_runtime_configuration();
 constexpr char cfg[]=R"CFG("Config2" { "Game" { "Name" "Strict_MUT_E" "Version" "4.3.2.1" "Language" "zz_MUT_B" "Languages" "L1_MUT_C,L2_MUT_D" } "Emulator" { "Online" "true" "Environment" "sandbox_MUT_A" } })CFG";
 assert(config::set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)==config::RuntimeConfigurationStatus::ok);
 strict_setting("<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRONMENT\"/></Request></LSX>","sandbox_MUT_A");
 // F1425-002: parser-owned selector attributes are semantic values, not raw entity spelling.
 strict_setting("<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRON&#77;ENT\"/></Request></LSX>","sandbox_MUT_A");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"FREETRIAL\"/></Request></LSX>","false");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"DISPLAY_NAME\"/></Request></LSX>","Strict_MUT_E");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"DISPLAY&#95;NAME\"/></Request></LSX>","Strict_MUT_E");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"AVAILABLE_VERSION\"/></Request></LSX>","4.3.2.1");
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"s\"><GetSetting SettingId=\"UNKNOWN_SELECTOR\"/></Request></LSX>");assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"UNKNOWN_SELECTOR\"/></Request></LSX>");assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 // A02-B: unrecognized named entity survives stock TinyXML2 as a literal.
 // It MUST remain an unsupported selector and must never resolve to ENVIRONMENT.
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRON&bogus;MENT\"/></Request></LSX>");assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"DISPLAY&bogus;NAME\"/></Request></LSX>");assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 // F319-P111-001: raw invalid XML characters are invalid requests, not selector misses.
 {const char req[]="<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRON\x01" "MENT\"/></Request></LSX>";char out[128]{};size_t n=7;auto q=parse(req);assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::invalid_request&&n==0);}
 {const char req[]="<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"DISPLAY\x01" "_NAME\"/></Request></LSX>";char out[128]{};size_t n=7;auto q=parse(req);assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::invalid_request&&n==0);}
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"s\"><GetSetting/></Request></LSX>");assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"g\"><GetGameInfo/></Request></LSX>");assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 // Phase x002: child-only selector representations are not behavioral request policy.
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"s\"><GetSetting><SettingId>LANGUAGE</SettingId></GetSetting></Request></LSX>");assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 {char out[128]{};size_t n=7;auto q=parse("<LSX><Request id=\"g\"><GetGameInfo><GameInfoId>FREETRIAL</GameInfoId></GetGameInfo></Request></LSX>");assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::unsupported&&n==0);}
 // Attribute policy wins even if a misleading child with the same name is present.
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"FREETRIAL\"><GameInfoId>UPTODATE</GameInfoId></GetGameInfo></Request></LSX>","false");
 config::clear_runtime_configuration();
 constexpr char missing[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG";
 assert(config::set_runtime_configuration_cfg(missing,sizeof(missing)-1)==config::RuntimeConfigurationStatus::ok);
 strict_setting("<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRONMENT\"/></Request></LSX>","production");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"INSTALLED_LANGUAGE\"/></Request></LSX>","en_US");
 strict_game("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"INSTALLED_VERSION\"/></Request></LSX>","1.0.0.0");

 config::clear_runtime_configuration();
 constexpr char special_cfg[]=R"CFG("Config2" { "Game" { "Name" "A&B<C>" } "Emulator" { "Online" "true" "Environment" "stage&dev" } })CFG";
 assert(config::set_runtime_configuration_cfg(special_cfg,sizeof(special_cfg)-1)==config::RuntimeConfigurationStatus::ok);
 {char out[4096]{};size_t n=0;auto q=parse("<LSX><Request id=\"g\"><GetGameInfo GameInfoId=\"DISPLAY_NAME\"/></Request></LSX>");assert(handlers::handle_get_game_info(q,out,sizeof out,&n)==handlers::HandlerStatus::ok);assert(std::strstr(out,"GameInfo=\"A&amp;B&lt;C&gt;\"")!=nullptr);}
 {char out[4096]{};size_t n=0;auto q=parse("<LSX><Request id=\"s\"><GetSetting SettingId=\"ENVIRONMENT\"/></Request></LSX>");assert(handlers::handle_runtime_get_setting(q,out,sizeof out,&n)==handlers::HandlerStatus::ok);assert(std::strstr(out,"Setting=\"stage&amp;dev\"")!=nullptr);}
 config::clear_runtime_configuration(); return 0;
}
