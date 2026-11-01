#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include <cstring>
using namespace preloader_l;
static bool eq(config::ValueView v,const char* s){return v.present&&v.size==std::strlen(s)&&std::memcmp(v.data,s,v.size)==0;}
static bool manual_is(const char* expected,bool fallback){
    constexpr char id[]="IS_MANUAL_OFFLINE";
    const auto r=handlers::resolve_get_setting_selector({id,sizeof(id)-1});
    if(!r.recognized||r.used_fallback!=fallback||!eq(r.value,expected))return false;
    const auto all=handlers::resolve_runtime_settings_policy();
    return eq(all.manual_offline,expected);
}
int main(){
    config::clear_runtime_overrides();config::clear_runtime_configuration();
    constexpr char online_with_compatibility[]=R"CFG("Config2" { "Emulator" { "Online" "true" "EALaunchOfflineMode" "1" } })CFG";
    if(config::set_runtime_configuration_cfg(online_with_compatibility,sizeof(online_with_compatibility)-1)!=config::RuntimeConfigurationStatus::ok)return 1;
    config::ValueView compatibility=config::missing_value();
    if(config::lookup_setting(config::runtime_settings_provider(),config::SettingKey::launch_offline_mode,&compatibility))return 2;
    if(!manual_is("false",false))return 3;

    config::clear_runtime_configuration();
    constexpr char offline_with_compatibility[]=R"CFG("Config2" { "Emulator" { "Online" "false" "EALaunchOfflineMode" "0" } })CFG";
    if(config::set_runtime_configuration_cfg(offline_with_compatibility,sizeof(offline_with_compatibility)-1)!=config::RuntimeConfigurationStatus::ok)return 4;
    if(config::lookup_setting(config::runtime_settings_provider(),config::SettingKey::launch_offline_mode,&compatibility))return 5;
    if(!manual_is("true",false))return 6;

    config::clear_runtime_configuration();
    constexpr char compatibility_only[]=R"CFG("Config2" { "Emulator" { "EALaunchOfflineMode" "1" } })CFG";
    if(config::set_runtime_configuration_cfg(compatibility_only,sizeof(compatibility_only)-1)!=config::RuntimeConfigurationStatus::ok)return 7;
    if(!manual_is("false",true))return 8;
    config::clear_runtime_configuration();config::clear_runtime_overrides();
    return 0;
}
