#include "preloader_l/config/runtime_configuration.hpp"
#include <cstring>
using namespace preloader_l::config;
static bool eq(ValueView v,const char* s){size_t n=std::strlen(s);return v.present&&v.size==n&&std::memcmp(v.data,s,n)==0;}
int main(){
 clear_runtime_configuration();
 constexpr char cfg[] = R"CFG("Config2"
{
 "Game" { "Name" "Demo Game" "Version" "9.8.7.6" "Language" "tr_TR" "Languages" "L1,L2" }
 "Emulator" { "Online" "true" "Environment" "sandbox" "SDK" "13.764.0.6277" }
 "User" { "Username" "SURGE" "PersonaId" "1144668899" "UserId" "1000200030000" "Country" "TR" }
 "Achievements" { "SteamAppId" "4032350" "SteamIdPrefix" "Achievement_GOSCC_" }
})CFG";
 if(set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=RuntimeConfigurationStatus::ok) return 1;
 if(runtime_configuration_present_sections()!=15u) return 2;
 if(!eq(runtime_configuration_setting("GameName"),"Demo Game")) return 3;
 if(!eq(runtime_configuration_setting("GameVersion"),"9.8.7.6")) return 4;
 if(!eq(runtime_configuration_setting("Language"),"tr_TR")) return 5;
 if(!eq(runtime_configuration_setting("Username"),"SURGE")) return 6;
 if(!eq(runtime_configuration_setting("UserId"),"1000200030000")) return 7;
 if(!eq(runtime_configuration_setting("SteamAppId"),"4032350")) return 8;
 constexpr char dup[] = R"CFG("Config2" { "User" { "Username" "FIRST" "Username" "SECOND" } })CFG";
 if(set_runtime_configuration_cfg(dup,sizeof(dup)-1)!=RuntimeConfigurationStatus::ok) return 9;
 if(!eq(runtime_configuration_setting("Username"),"SECOND")) return 10;
 constexpr char bad[] = R"CFG("Config2" { "User" { "Username" "BROKEN" })CFG";
 if(set_runtime_configuration_cfg(bad,sizeof(bad)-1)!=RuntimeConfigurationStatus::invalid_cfg) return 11;
 if(!eq(runtime_configuration_setting("Username"),"SECOND")) return 12;
 constexpr char comments[] = R"CFG(// lead
"Config2" { /* x */ "Game" { "Name" "Commented" } })CFG";
 if(set_runtime_configuration_cfg(comments,sizeof(comments)-1)!=RuntimeConfigurationStatus::ok) return 13;
 if(!eq(runtime_configuration_setting("GameName"),"Commented")) return 14;
 constexpr char compatibility_xml[] = "<SURGE><Settings Username=\"NO\"/></SURGE>";
 if(set_runtime_configuration_cfg(compatibility_xml,sizeof(compatibility_xml)-1)!=RuntimeConfigurationStatus::invalid_cfg) return 15;
 clear_runtime_configuration();
 constexpr char before_read[]=R"CFG("Config2" { "User" { "Value" "BASE" } })CFG";
 if(set_runtime_configuration_cfg(before_read,sizeof(before_read)-1)!=RuntimeConfigurationStatus::ok) return 19;
 if(!eq(runtime_configuration_setting("Value"),"BASE")) return 20;
 RuntimeConfigurationReadLease lease{};
 if(!begin_runtime_configuration_read(&lease)) return 21;
 auto leased=runtime_configuration_setting("Value");
 if(!eq(leased,"BASE")) return 22;
 const uint64_t held_generation=lease.generation;
 constexpr char during_read[]=R"CFG("Config2" { "User" { "Value" "GEN2" } })CFG";
 if(set_runtime_configuration_cfg(during_read,sizeof(during_read)-1)!=RuntimeConfigurationStatus::resource_error) return 23;
 if(runtime_configuration_generation()!=held_generation) return 24;
 if(!eq(runtime_configuration_setting("Value"),"BASE")) return 25;
 end_runtime_configuration_read(&lease);
 if(set_runtime_configuration_cfg(during_read,sizeof(during_read)-1)!=RuntimeConfigurationStatus::ok) return 26;
 if(runtime_configuration_generation()==held_generation) return 27;
 auto gen2=runtime_configuration_setting("Value");
 if(!eq(gen2,"GEN2")) return 28;
 return 0;
}
