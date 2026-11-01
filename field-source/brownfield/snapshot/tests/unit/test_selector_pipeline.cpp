#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/handlers/selector_pipeline.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
static bool eq(config::ValueView v,const char* s){return v.present && v.size==std::strlen(s) && std::memcmp(v.data,s,v.size)==0;}
static void gs(const char* id,const char* expected,bool fallback=false){
    const xml::TextSpan s{id,std::strlen(id)}; const auto r=handlers::resolve_get_setting_selector(s);
    assert(r.recognized); assert(eq(r.value,expected)); assert(r.used_fallback==fallback);
}
static void gi(const char* id,const char* expected,bool fallback=false){
    const xml::TextSpan s{id,std::strlen(id)}; const auto r=handlers::resolve_get_game_info_selector(s);
    assert(r.recognized); assert(eq(r.value,expected)); assert(r.used_fallback==fallback);
}
int main(){
    config::clear_runtime_overrides(); config::clear_runtime_configuration();
    constexpr char on[]=R"CFG("Config2" { "Game" { "Name" "Game_MUT_E" "Version" "9.8.7.6" "Language" "zz_MUT_B" "Languages" "L1_MUT_C,L2_MUT_D" } "Emulator" { "Online" "true" "Environment" "sandbox_MUT_A" } })CFG";
    assert(config::set_runtime_configuration_cfg(on,sizeof(on)-1)==config::RuntimeConfigurationStatus::ok);
    gs("IS_IGO_ENABLED","true",true); gs("IS_IGO_AVAILABLE","true",true); gs("ENVIRONMENT","sandbox_MUT_A");
    gs("LANGUAGE","zz_MUT_B"); gs("IS_TELEMETRY_ENABLED","false",true); gs("IS_MANUAL_OFFLINE","false");
    { const char x[]="UNKNOWN_SELECTOR"; auto z=handlers::resolve_get_setting_selector({x,sizeof(x)-1}); assert(!z.recognized && eq(z.value,"")); }

    gi("DISPLAY_NAME","Game_MUT_E");
    gi("INSTALLED_VERSION","9.8.7.6");
    gi("AVAILABLE_VERSION","9.8.7.6");
    gi("UPTODATE","true");
    gi("FULLGAME_IS_RELEASED","true");
    gi("FULLGAME_RELEASE_DATE","2010-01-01T00:00:00");
    gi("FULLGAME_PURCHASED","true");
    gi("FREETRIAL","false");
    gi("EXPIRATION","0000-00-00T00:00:00");
    gi("ENTITLEMENT_SOURCE","ORIGIN");
    gi("MAX_GROUP_SIZE","16");
    gi("EXPIRATION_DURATION","36000000");
    gi("LANGUAGES","L1_MUT_C,L2_MUT_D");
    gi("INSTALLED_LANGUAGE","zz_MUT_B");
    { const char x[]="LANGUAGE"; auto z=handlers::resolve_get_game_info_selector({x,sizeof(x)-1}); assert(!z.recognized); }
    { const char x[]="UNKNOWN_SELECTOR"; auto z=handlers::resolve_get_game_info_selector({x,sizeof(x)-1}); assert(!z.recognized && eq(z.value,"")); }

    config::clear_runtime_configuration();
    constexpr char off[]=R"CFG("Config2" { "Game" { "Language" "zz_MUT_B" "Languages" "L1_MUT_C,L2_MUT_D" } "Emulator" { "Online" "false" "Environment" "sandbox_MUT_A" } })CFG";
    assert(config::set_runtime_configuration_cfg(off,sizeof(off)-1)==config::RuntimeConfigurationStatus::ok);
    gs("IS_IGO_ENABLED","DOWN",true); gs("IS_IGO_AVAILABLE","DOWN",true); gs("IS_MANUAL_OFFLINE","true");

    config::clear_runtime_configuration();
    constexpr char missing[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG";
    assert(config::set_runtime_configuration_cfg(missing,sizeof(missing)-1)==config::RuntimeConfigurationStatus::ok);
    gs("ENVIRONMENT","production",true); gs("LANGUAGE","en_US",true);
    gi("DISPLAY_NAME","Game",true); gi("INSTALLED_VERSION","1.0.0.0",true); gi("AVAILABLE_VERSION","1.0.0.0",true);
    gi("INSTALLED_LANGUAGE","en_US",true);
    gi("LANGUAGES","ar_SA,cs_CZ,da_DK,de_DE,el_GR,en_AU,en_CA,en_GB,en_US,es_ES,es_MX,fi_FI,fr_CA,fr_FR,hu_HU,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,pt_PT,ru_RU,sv_SE,th_TH,tr_TR,zh_CN,zh_HK,zh_TW",true);
    config::clear_runtime_configuration(); config::clear_runtime_overrides();
    return 0;
}
