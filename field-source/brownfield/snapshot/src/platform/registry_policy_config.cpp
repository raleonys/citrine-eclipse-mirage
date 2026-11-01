#include "registry_policy_internal.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <stdint.h>
#if defined(_WIN32)
extern "C" {
__declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int,unsigned long,const char*,int,uint16_t*,int);
__declspec(dllimport) int __stdcall CompareStringOrdinal(const uint16_t*,int,const uint16_t*,int,int);
}
#endif

namespace preloader_l { namespace platform { namespace host_policy_detail {
namespace {
constexpr unsigned int k_cp_utf8=65001u;
#if defined(_WIN64)
constexpr char k_default_client_key[]="SOFTWARE\\Wow6432Node\\Origin";
#else
constexpr char k_default_client_key[]="SOFTWARE\\Origin";
#endif
constexpr char k_default_client_key2[]="SOFTWARE\\Electronic Arts\\EA Desktop";
constexpr char k_project_client_value[]="ClientPath";
unsigned fold_ascii(unsigned c) noexcept {
    return c>=static_cast<unsigned>('a')&&c<=static_cast<unsigned>('z')
        ? c-static_cast<unsigned>('a')+static_cast<unsigned>('A'):c;
}
}

size_t cstr_len(const char* s) noexcept { size_t n=0;if(s)while(s[n])++n;return n; }
bool eq_ci(const char* a,size_t an,const char* b,size_t bn) noexcept {
    if(!a||!b||an!=bn)return false;
    for(size_t i=0;i<an;++i)if(fold_ascii(static_cast<unsigned char>(a[i]))!=fold_ascii(static_cast<unsigned char>(b[i])))return false;
    return true;
}
bool wide_eq_utf8_ci(const uint16_t* a,const char* b,size_t bn) noexcept {
    if(!a||(!b&&bn))return false;
    size_t an=0;while(a[an])++an;
    bool ascii=an==bn;
    if(ascii){
        for(size_t i=0;i<bn;++i){
            const unsigned bc=static_cast<unsigned char>(b[i]);
            if(a[i]>0x7Fu||bc>0x7Fu){ascii=false;break;}
            if(fold_ascii(static_cast<unsigned>(a[i]))!=fold_ascii(bc))return false;
        }
        if(ascii)return true;
    }
#if defined(_WIN32)
    if(an>static_cast<size_t>(INT32_MAX)||bn>static_cast<size_t>(INT32_MAX))return false;
    if(bn==0u)return an==0u;
    const int need=MultiByteToWideChar(k_cp_utf8,0,b,static_cast<int>(bn),nullptr,0);
    if(need<=0||static_cast<size_t>(need)!=an)return false;
    auto* wide=static_cast<uint16_t*>(heap_alloc(static_cast<size_t>(need)*sizeof(uint16_t)));
    if(!wide)return false;
    const int got=MultiByteToWideChar(k_cp_utf8,0,b,static_cast<int>(bn),wide,need);
    if(got!=need){heap_free(wide);return false;}
    const int result=CompareStringOrdinal(a,static_cast<int>(an),wide,need,1);
    heap_free(wide);
    return result==2;
#else
    return false;
#endif
}
preloader_l::config::ValueView cfg(const char* key) noexcept {
    return preloader_l::config::runtime_configuration_setting(key);
}
static preloader_l::config::ValueView client_key1() noexcept {
    const auto v=cfg("ClientKey");return v.present?v:preloader_l::config::literal_value(k_default_client_key);
}
static preloader_l::config::ValueView client_key2() noexcept {
    const auto v=cfg("ClientKey2");return v.present?v:preloader_l::config::literal_value(k_default_client_key2);
}
preloader_l::config::ValueView configured_client_value_name() noexcept { return cfg("ClientValue"); }
preloader_l::config::ValueView active_client_value_name() noexcept {
    const auto v=configured_client_value_name();
    return v.present?v:preloader_l::config::literal_value(k_project_client_value);
}
LanguagePolicy language_policy() noexcept {
    const auto raw=cfg("LanguageRegistryKey");
    bool enabled=raw.present&&raw.size!=0u;
    preloader_l::config::ValueView key{};preloader_l::config::ValueView value{};
    if(enabled){
        size_t split=raw.size;
        for(size_t i=0;i<raw.size;++i)if(raw.data[i]=='\\'||raw.data[i]=='/')split=i;
        if(split<raw.size){
            key=preloader_l::config::ValueView{raw.data,split,true};
            value=preloader_l::config::ValueView{raw.data+split+1u,raw.size-split-1u,true};
        } else {
            key=preloader_l::config::ValueView{raw.data,raw.size,true};
            value=preloader_l::config::ValueView{"",0,true};
        }
    }
    const auto gate=cfg("LanguageRegistrySpoof");
    if(gate.present)enabled=preloader_l::config::behavioral_config_boolean(gate,true);
    return LanguagePolicy{enabled,key,value};
}
uint32_t classify_a(const char* subkey) noexcept {
    if(!subkey)return 0u;
    const size_t n=cstr_len(subkey); const auto c1=client_key1(),c2=client_key2();
    if(c1.present&&eq_ci(subkey,n,c1.data,c1.size))return k_client_active;
    if(c2.present&&eq_ci(subkey,n,c2.data,c2.size))return k_client_secondary;
    const auto l=language_policy();
    if(l.enabled&&l.key.present&&eq_ci(subkey,n,l.key.data,l.key.size))return k_language;
    return 0u;
}
uint32_t classify_w(const uint16_t* subkey) noexcept {
    if (!subkey) return 0u;
    const auto c1 = client_key1(), c2 = client_key2();
    if(c1.present&&wide_eq_utf8_ci(subkey,c1.data,c1.size))return k_client_active;
    if(c2.present&&wide_eq_utf8_ci(subkey,c2.data,c2.size))return k_client_secondary;
    const auto l=language_policy();
    if(l.enabled&&l.key.present&&wide_eq_utf8_ci(subkey,l.key.data,l.key.size))return k_language;
    return 0u;
}
} } }
