#include "preloader_l/platform/registry_bridge.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#include <cstring>
using namespace preloader_l;
namespace {
int open_a_calls=0,query_a_calls=0,close_calls=0;
int32_t open_a(void*,const char*,uint32_t,uint32_t,void** out) noexcept {++open_a_calls;if(out)*out=reinterpret_cast<void*>(0x1234);return 0;}
int32_t open_w(void*,const uint16_t*,uint32_t,uint32_t,void**) noexcept{return 56;}
int32_t query_a(void*,const char*,uint32_t*,uint32_t*,uint8_t*,uint32_t* bytes) noexcept {++query_a_calls;if(bytes)*bytes=4u;return 57;}
int32_t query_w(void*,const uint16_t*,uint32_t*,uint32_t*,uint8_t*,uint32_t*) noexcept{return 58;}
int32_t close_key(void*) noexcept {++close_calls;return 0;}
}
int main(){
 platform::HostBridgeState s{};platform::reset_host_bridge(&s);platform::clear_registry_policy_state_for_test();
 const char exe[]="C:\\Game\\game.exe";const uint16_t wexe[]={'C',':','\\','G','a','m','e','\\','g','a','m','e','.','e','x','e',0};
 s.executable_path={exe,sizeof(exe)-1,wexe,16};s.originals={open_a,open_w,query_a,query_w,close_key};
 void* key=nullptr;
 if(platform::bridge_reg_open_a(&s,reinterpret_cast<void*>(1),"SOFTWARE\\Wow6432Node\\Origin",0,0,&key)!=0||reinterpret_cast<uintptr_t>(key)!=0x1234||open_a_calls!=1)return 1;
 uint32_t type=0,bytes=64;uint8_t out[64]{};
 if(platform::bridge_reg_query_a(&s,key,"ClientPath",nullptr,&type,out,&bytes)!=0||query_a_calls!=1||std::memcmp(out,exe,sizeof(exe))!=0)return 2;
 if(platform::bridge_reg_close(&s,key)!=0||close_calls!=1)return 3;

 // ClientKey2 is behavioral-classified, but absent an explicit ClientValue the old
 // ClientPath compatibility alias must not override EA Desktop queries.
 key=nullptr;
 if(platform::bridge_reg_open_a(&s,reinterpret_cast<void*>(1),"SOFTWARE\\Electronic Arts\\EA Desktop",0,0,&key)!=0||reinterpret_cast<uintptr_t>(key)!=0x1234||open_a_calls!=2)return 4;
 type=0;bytes=sizeof(out);
 if(platform::bridge_reg_query_a(&s,key,"ClientPath",nullptr,&type,out,&bytes)!=57||query_a_calls!=2)return 5;
 if(platform::bridge_reg_close(&s,key)!=0||close_calls!=2)return 6;
 platform::clear_registry_policy_state_for_test();return 0;
}
