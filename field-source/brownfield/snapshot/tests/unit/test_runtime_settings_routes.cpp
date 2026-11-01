#include "preloader_l/handlers/runtime_settings_routes.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include <cstring>
using namespace preloader_l;
static bool parse(const char* p,size_t n,xml::RequestView* r){return xml::parse_request_view(p,n,r)==xml::RequestStatus::ok;}
int main(){
  char out[2048]{}; size_t n=0; xml::RequestView r{};
  config::clear_runtime_overrides(); config::clear_runtime_configuration();

  constexpr char base[]=R"CFG("Config2" { "Game" { "Language" "en_US" } "Emulator" { "Online" "true" "Environment" "production" } })CFG";
  if(config::set_runtime_configuration_cfg(base,sizeof(base)-1)!=config::RuntimeConfigurationStatus::ok)return 1;

  constexpr char manual[]="<LSX><Request id=\"1\"><GetSetting SettingId=\"IS_MANUAL_OFFLINE\"/></Request></LSX>";
  if(!parse(manual,sizeof(manual)-1,&r)) return 2;
  if(handlers::handle_runtime_get_setting(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 3;
  if(!std::strstr(out,"Setting=\"false\"")) return 4;

  constexpr char all[]="<LSX><Request id=\"2\"><GetSettings/></Request></LSX>";
  if(!parse(all,sizeof(all)-1,&r)) return 5;
  if(handlers::handle_runtime_get_settings(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 6;
  if(!std::strstr(out,"IsManualOffline=\"false\"") ||
     !std::strstr(out,"IsIGOAvailable=\"true\"") ||
     !std::strstr(out,"IsIGOEnabled=\"true\"")) return 7;

  config::clear_runtime_configuration();
  constexpr char explicit_values[]=R"CFG("Config2" { "Game" { "Language" "tr_TR" } "Emulator" { "Online" "true" "EALaunchOfflineMode" "1" "IGOAvailable" "0" "IGOEnabled" "false" "Environment" "test" } })CFG";
  if(config::set_runtime_configuration_cfg(explicit_values,sizeof(explicit_values)-1)!=config::RuntimeConfigurationStatus::ok)return 8;

  if(!parse(manual,sizeof(manual)-1,&r)) return 9;
  if(handlers::handle_runtime_get_setting(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 10;
  if(!std::strstr(out,"Setting=\"false\"")) return 11;

  constexpr char avail[]="<LSX><Request id=\"3\"><GetSetting SettingId=\"IS_IGO_AVAILABLE\"/></Request></LSX>";
  if(!parse(avail,sizeof(avail)-1,&r)) return 12;
  if(handlers::handle_runtime_get_setting(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 13;
  if(!std::strstr(out,"Setting=\"DOWN\"")) return 14;

  if(!parse(all,sizeof(all)-1,&r)) return 15;
  if(handlers::handle_runtime_get_settings(r,out,sizeof out,&n)!=handlers::HandlerStatus::ok) return 16;
  if(!std::strstr(out,"IsManualOffline=\"false\"") ||
     !std::strstr(out,"IsIGOAvailable=\"false\"") ||
     !std::strstr(out,"IsIGOEnabled=\"false\"") ||
     !std::strstr(out,"Environment=\"test\"") ||
     !std::strstr(out,"Language=\"tr_TR\"")) return 17;

  config::clear_runtime_configuration(); config::clear_runtime_overrides();
  return 0;
}
