#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/store_runtime.hpp"
#include <stddef.h>
using namespace preloader_l::config;
int main(){
 clear_runtime_configuration();clear_runtime_store();
 constexpr char cfg[]=R"CFG("Config2" { "Emulator" { "Online" "false" } })CFG";
 if(set_runtime_configuration_cfg(cfg,sizeof(cfg)-1)!=RuntimeConfigurationStatus::ok)return 1;
 if(runtime_store_status()!=StoreRuntimeStatus::not_loaded)return 2;
 const char good[]="<Root><Store><Entitlement NumericId=\"42\" Title=\"local\"/></Store></Root>";
 if(set_runtime_store_xml(good,sizeof(good)-1)!=StoreRuntimeStatus::ok)return 3;
 if(!runtime_store_loaded()||!runtime_store_present()||runtime_store_entitlement_count()!=1)return 4;
 const char bad[]="<Root><Store><Entitlement NumericId=\"7\" Pad=\"0\"/></Store></Root>";
 if(set_runtime_store_xml(bad,sizeof(bad)-1)!=StoreRuntimeStatus::invalid_hex)return 5;
 StoreEntitlementView retained{};if(!runtime_store_entitlement(0,&retained)||retained.numeric_id!=42)return 6;
 // Config2 state and Store state have independent lifetimes. Clearing Config2
 // must not destroy an independently committed Store snapshot.
 clear_runtime_configuration();
 if(!runtime_store_loaded()||!runtime_store_present()||runtime_store_entitlement_count()!=1)return 7;
 if(!runtime_store_entitlement(0,&retained)||retained.numeric_id!=42)return 8;
 clear_runtime_store();return 0;
}
