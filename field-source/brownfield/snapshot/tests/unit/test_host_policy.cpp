#include "preloader_l/platform/registry_policy.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cstring>
using namespace preloader_l;
int main(){
#if !defined(_WIN32)
    return 0;
#else
    platform::clear_registry_policy_state_for_test();
    uintptr_t key=0;
    // Behavioral original-first success: classify the real returned HKEY.
    if(platform::registry_after_open_a("SOFTWARE\\Wow6432Node\\Origin",0,0x1234,&key)!=0||key!=0x1234) return 1;
    const char exe[]="C:\\Game\\game.exe";uint32_t type=0,bytes=0;
    if(platform::registry_after_query_a(key,"ClientPath",2,0u,exe,sizeof(exe)-1,&type,nullptr,&bytes)!=0||type!=1||bytes!=sizeof(exe))return 2;
    unsigned char out[64]{};uint32_t cap=4;
    if(platform::registry_after_query_a(key,"ClientPath",2,cap,exe,sizeof(exe)-1,&type,out,&cap)!=234||cap!=sizeof(exe))return 3;
    cap=sizeof(out);type=7;
    if(platform::registry_after_query_a(key,"ClientPath",2,cap,exe,sizeof(exe)-1,&type,out,&cap)!=0||type!=platform::host_reg_sz||std::memcmp(out,exe,sizeof(exe))!=0)return 4;
    platform::registry_after_real_close(key,0);
    if(platform::registry_after_query_a(key,"ClientPath",55,cap,exe,sizeof(exe)-1,&type,out,&cap)!=55)return 5;

    // Phase1396 does not prove a default ClientValue. The legacy ClientPath
    // compatibility alias must not leak onto ClientKey2/EA Desktop.
    uintptr_t key2=0;
    if(platform::registry_after_open_a("SOFTWARE\\Electronic Arts\\EA Desktop",0,0x5678,&key2)!=0||key2!=0x5678)return 6;
    cap=sizeof(out);type=0;
    if(platform::registry_after_query_a(key2,"ClientPath",91,cap,exe,sizeof(exe)-1,&type,out,&cap)!=91)return 7;
    platform::registry_after_real_close(key2,0);

    // Phase1401: configured UTF-8 authorities must match W registry calls
    // case-insensitively, including non-ASCII code units.
    config::clear_runtime_configuration();
    constexpr char unicode_cfg[] =
        "\"Config2\" { \"Game\" { "
        "\"ClientKey\" \"SOFTWARE\\Test\\\xC3\x9C" "rigin\" "
        "\"ClientValue\" \"Cl\xC3\xAF" "entPath\" "
        "\"ClientPath\" \"C:\\Unicode\\game.exe\" "
        "\"LanguageRegistryKey\" \"SOFTWARE\\T\xC3\xA9" "st\\L\xC3\xB6" "cale\" "
        "\"LanguageRegistrySpoof\" \"true\" \"Language\" \"tr_TR\" } }";
    if(config::set_runtime_configuration_cfg(unicode_cfg,sizeof(unicode_cfg)-1)!=config::RuntimeConfigurationStatus::ok)return 8;

    const uint16_t unicode_key[]={ 's','o','f','t','w','a','r','e','\\','t','e','s','t','\\',0x00FC,'r','i','g','i','n',0 };
    uintptr_t ukey=0;
    if(platform::registry_after_open_w(unicode_key,0,0x6789,&ukey)!=0||ukey!=0x6789)return 9;
    const uint16_t unicode_value[]={ 'C','L',0x00CF,'E','N','T','P','A','T','H',0 };
    const uint16_t wexe[]={ 'C',':','\\','U','n','i','c','o','d','e','\\','g','a','m','e','.','e','x','e',0 };
    uint8_t wout[128]{};uint32_t wcap=sizeof(wout);type=0;
    if(platform::registry_after_query_w(ukey,unicode_value,2,wcap,wexe,19u,&type,wout,&wcap)!=0||type!=1)return 10;
    platform::registry_after_real_close(ukey,0);

    const uint16_t language_key[]={ 's','o','f','t','w','a','r','e','\\','t',0x00C9,'s','t',0 };
    uintptr_t lkey=0;
    if(platform::registry_after_open_w(language_key,2,0,&lkey)!=0||!lkey)return 11;
    const uint16_t language_value[]={ 'L',0x00D6,'C','A','L','E',0 };
    wcap=sizeof(wout);type=0;
    if(platform::registry_after_query_w(lkey,language_value,2,wcap,nullptr,0u,&type,wout,&wcap)!=0||type!=1)return 12;
    int32_t close_status=0;if(!platform::registry_close_pseudo(lkey,&close_status)||close_status!=0)return 13;

    config::clear_runtime_configuration();
    platform::clear_registry_policy_state_for_test();return 0;
#endif
}
