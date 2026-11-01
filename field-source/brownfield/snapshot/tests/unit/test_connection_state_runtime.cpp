#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include <cstring>
using namespace preloader_l;
static bool has(const char*s,const char*n){return std::strstr(s,n)!=nullptr;}
static bool load(const char*s){return config::set_runtime_configuration_cfg(s,std::strlen(s))==config::RuntimeConfigurationStatus::ok;}
static bool internet(bool expected){char out[1024]{};const char q[]="<LSX><Request id=\"62\"><GetInternetConnectedState/></Request></LSX>";const auto r=core::dispatch_plaintext_request(q,sizeof(q)-1,out,sizeof(out));return r.status==core::DispatchStatus::ok&&has(out,expected?"connected=\"true\"":"connected=\"false\"");}
int main(){
 config::clear_runtime_overrides();config::clear_runtime_configuration();
 if(!internet(true))return 1;
 constexpr char off[]=R"CFG("Config2" { "Emulator" { "Online" "false" } })CFG"; if(!load(off)||!internet(false))return 2;
 constexpr char on[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG"; if(!load(on)||!internet(true))return 3;
 constexpr char zero[]=R"CFG("Config2" { "Emulator" { "Online" "0" } })CFG"; if(!load(zero)||!internet(true))return 4;
 constexpr char one[]=R"CFG("Config2" { "Emulator" { "Online" "true" } })CFG"; if(!load(one)||!internet(true))return 5;
 config::clear_runtime_configuration();return 0;
}
