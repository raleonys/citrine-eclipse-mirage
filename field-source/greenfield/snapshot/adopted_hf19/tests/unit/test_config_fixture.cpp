#include "preloader_l/config/runtime_configuration.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
static bool eq(config::ValueView v,const char*s){return v.present&&v.size==std::strlen(s)&&std::memcmp(v.data,s,v.size)==0;}
int main(){
 constexpr char good[]=R"CFG("Config2" { "User" { "Username" "SURGE" "UserId" "1000200030000" "PersonaId" "1144668899" "Country" "US" } "Game" { "Language" "en_US" } })CFG";
 assert(config::set_runtime_configuration_cfg(good,sizeof(good)-1)==config::RuntimeConfigurationStatus::ok);
 assert(eq(config::runtime_configuration_setting("Username"),"SURGE"));
 assert(eq(config::runtime_configuration_setting("Country"),"US"));
 constexpr char bad[]="<SURGE><Settings Username=\"SURGE\"/></SURGE>";
 assert(config::set_runtime_configuration_cfg(bad,sizeof(bad)-1)==config::RuntimeConfigurationStatus::invalid_cfg);
 return 0;
}
