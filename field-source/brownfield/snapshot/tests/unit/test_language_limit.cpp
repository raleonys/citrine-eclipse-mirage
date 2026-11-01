#include "preloader_l/platform/environment_policy.hpp"
#include "preloader_l/config/settings_provider.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <cstring>
#include <string>
using namespace preloader_l;
struct Ctx { char locale[1024]{}; };
static uint32_t len(const char*, void*) noexcept { return 0; }
static bool set(const char* name,const char* value,void* raw) noexcept {
    auto* c=static_cast<Ctx*>(raw);
    if(std::strcmp(name,"EAGameLocale")==0){
        const size_t n=std::strlen(value);
        if(n>=sizeof(c->locale)) return false;
        std::memcpy(c->locale,value,n+1u);
    }
    return true;
}
struct Settings { config::ValueView online, language; };
static bool lookup(const void* raw,config::SettingKey key,config::ValueView* out) noexcept {
    if(!out) return false;
    auto* s=static_cast<const Settings*>(raw);
    if(key==config::SettingKey::online){*out=s->online;return true;}
    if(key==config::SettingKey::language){*out=s->language;return true;}
    *out=config::missing_value(); return false;
}
int main(){
    std::string language(512u,'L');
    Settings settings{config::literal_value("0"),config::ValueView{language.data(),language.size(),true}};
    config::SettingsProvider provider{&settings,lookup};
    Ctx ctx{}; platform::EnvironmentBackend backend{len,set,&ctx}; uint32_t written=0,preserved=0;
    const auto status=platform::apply_environment(provider,&backend,&written,&preserved);
    if(status!=platform::EnvironmentStatus::ok) return 1;
    if((written&2u)==0u || preserved!=0u) return 2;
    if(std::strlen(ctx.locale)!=language.size()) return 3;
    if(std::memcmp(ctx.locale,language.data(),language.size())!=0) return 4;
    return 0;
}
