#include "preloader_l/platform/environment_policy.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <cstring>

using namespace preloader_l;
struct Ctx { uint32_t offline_len=0, locale_len=0; char offline[16]{}; char locale[32]{}; };
static uint32_t len(const char* n,void* p) noexcept { auto*c=(Ctx*)p; return std::strcmp(n,"EALaunchOfflineMode")==0?c->offline_len:c->locale_len; }
static bool set(const char* n,const char* v,void* p) noexcept { auto*c=(Ctx*)p; if(std::strcmp(n,"EALaunchOfflineMode")==0){std::strncpy(c->offline,v,sizeof(c->offline)-1);return true;} std::strncpy(c->locale,v,sizeof(c->locale)-1);return true; }
struct Settings { config::ValueView online, language; };
static bool lookup(const void* p,config::SettingKey key,config::ValueView* out) noexcept { auto*s=(const Settings*)p; if(!out)return false; if(key==config::SettingKey::online&&s->online.present){*out=s->online;return true;} if(key==config::SettingKey::language&&s->language.present){*out=s->language;return true;} *out=config::missing_value(); return false; }
int main(){
 Settings s{config::literal_value("false"),config::literal_value("tr_TR")}; config::SettingsProvider sp{&s,lookup}; Ctx c{}; platform::EnvironmentBackend b{len,set,&c}; uint32_t w=0,p=0;
 if(platform::apply_environment(sp,&b,&w,&p)!=platform::EnvironmentStatus::ok||w!=3||p!=0) return 1;
 if(std::strcmp(c.offline,"true")||std::strcmp(c.locale,"tr_TR")) return 2;
 Settings zero{config::literal_value("0"),config::literal_value("tr_TR")}; config::SettingsProvider sp_zero{&zero,lookup}; c=Ctx{}; w=p=0;
 if(platform::apply_environment(sp_zero,&b,&w,&p)!=platform::EnvironmentStatus::ok||w!=3||p!=0) return 5;
 if(std::strcmp(c.offline,"false")||std::strcmp(c.locale,"tr_TR")) return 6;
 c=Ctx{}; c.offline_len=5; c.locale_len=7; w=p=0;
 if(platform::apply_environment(sp,&b,&w,&p)!=platform::EnvironmentStatus::ok||w!=0||p!=3) return 3;
 Settings fallback{config::missing_value(),config::missing_value()}; config::SettingsProvider sp2{&fallback,lookup}; c=Ctx{}; w=p=0;
 if(platform::apply_environment(sp2,&b,&w,&p)!=platform::EnvironmentStatus::ok||std::strcmp(c.offline,"false")||std::strcmp(c.locale,"en_US")) return 4;
 c=Ctx{}; c.offline_len=32767u; c.locale_len=40000u; w=p=0;
 if(platform::apply_environment(sp,&b,&w,&p)!=platform::EnvironmentStatus::ok||w!=0u||p!=3u) return 7;
 c=Ctx{}; c.offline_len=platform::environment_length_query_error; c.locale_len=5u; w=p=0;
 if(platform::apply_environment(sp,&b,&w,&p)!=platform::EnvironmentStatus::query_failed||w!=0u||p!=3u) return 8;
 return 0;
}
