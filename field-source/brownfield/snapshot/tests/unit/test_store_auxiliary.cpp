#include "preloader_l/config/store_runtime.hpp"
#include "preloader_l/config/store_payload.hpp"
#include <stddef.h>
#include <stdint.h>
using namespace preloader_l::config;
static bool eq(StoreByteView v,const uint8_t* p,size_t n){if(v.size!=n)return false;for(size_t i=0;i<n;++i)if(v.data[i]!=p[i])return false;return true;}
int main(){
  clear_runtime_store();
  const char xml[]="<R><Store PreambleB64=\"AQID\" TrailerB64=\"BAUG\"><Entitlement NumericId=\"7\"/></Store></R>";
  if(set_runtime_store_xml(xml,sizeof(xml)-1)!=StoreRuntimeStatus::ok)return 1;
  const uint8_t pre[]={1,2,3},trail[]={4,5,6};
  if(!eq(runtime_store_preamble(),pre,sizeof(pre)))return 2;
  if(!eq(runtime_store_trailer(),trail,sizeof(trail)))return 3;
  size_t need=0;if(runtime_store_payload_size(&need)!=StorePayloadStatus::ok||need!=8)return 4;
  uint8_t out[8]{};size_t got=0;if(build_runtime_store_payload(out,sizeof(out),&got)!=StorePayloadStatus::ok||got!=8)return 5;
  const uint8_t exp[]={1,2,3,0x08,0x07,4,5,6};for(size_t i=0;i<8;++i)if(out[i]!=exp[i])return 6;
  const char entity[]="<R><Store PreambleB64=\"AQI&#61;\"><Entitlement NumericId=\"7\"/></Store></R>";
  if(set_runtime_store_xml(entity,sizeof(entity)-1)!=StoreRuntimeStatus::ok)return 7;
  const uint8_t pre2[]={1,2};if(!eq(runtime_store_preamble(),pre2,sizeof(pre2)))return 8;
  if(runtime_store_trailer().size!=0)return 9;
  const char incomplete[]="<R><Store PreambleB64=\"AQ\"><Entitlement NumericId=\"7\"/></Store></R>";
  if(set_runtime_store_xml(incomplete,sizeof(incomplete)-1)!=StoreRuntimeStatus::ok)return 10;
  if(runtime_store_preamble().size!=0)return 11;
  const char invalid[]="<R><Store PreambleB64=\"!!!!\"><Entitlement NumericId=\"7\"/></Store></R>";
  if(set_runtime_store_xml(invalid,sizeof(invalid)-1)!=StoreRuntimeStatus::ok)return 12;
  if(runtime_store_preamble().size!=0)return 13;
  // Root itself named Store is not selected by the curio AE00 selection rule.
  const char root_store[]="<Store PreambleB64=\"AQID\"><Entitlement NumericId=\"7\"/></Store>";
  if(set_runtime_store_xml(root_store,sizeof(root_store)-1)!=StoreRuntimeStatus::ok)return 14;
  if(runtime_store_present()||runtime_store_preamble().size||runtime_store_entitlement_count())return 15;
  return 0;
}
