#include "preloader_l/platform/registry_bridge.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdint>

using namespace preloader_l;

namespace { bool g_fail_wide_sizing=false; }
extern "C" {
void* GetProcessHeap(){ return reinterpret_cast<void*>(1); }
void* HeapAlloc(void*, unsigned long, size_t n){ return std::malloc(n); }
int HeapFree(void*, unsigned long, void* p){ std::free(p); return 1; }
int MultiByteToWideChar(unsigned int, unsigned long, const char* src, int n, uint16_t* out, int cap){
    if(!src || n < 0) return 0;
    if(!out){ if(g_fail_wide_sizing && n>0) return 0; return n; }
    if(cap < n) return 0;
    for(int i=0;i<n;++i) out[i]=static_cast<unsigned char>(src[i]);
    return n;
}
int CompareStringOrdinal(const uint16_t* a,int an,const uint16_t* b,int bn,int ignore_case){
    if(!a||!b||an!=bn) return 1;
    for(int i=0;i<an;++i){
        uint16_t x=a[i],y=b[i];
        if(ignore_case){if(x>='a'&&x<='z')x-=32;if(y>='a'&&y<='z')y-=32;}
        if(x!=y) return x<y?1:3;
    }
    return 2;
}
}

namespace { preloader_l::config::ValueView g_client_path=preloader_l::config::missing_value(); }
namespace preloader_l { namespace config {
ValueView runtime_configuration_setting(const char* key) noexcept {
    if(key&&std::strcmp(key,"ClientPath")==0) return g_client_path;
    return missing_value();
}
} }

namespace {
int a_calls=0,w_calls=0,close_calls=0;
int32_t failed_a(void*,const char*,uint32_t,uint32_t,void**) noexcept { ++a_calls; return 2; }
int32_t failed_w(void*,const uint16_t*,uint32_t,uint32_t,void**) noexcept { ++w_calls; return 2; }
int32_t success_a(void*,const char*,uint32_t,uint32_t,void** out) noexcept { ++a_calls; if(out)*out=reinterpret_cast<void*>(0x1234); return 0; }
int32_t failed_query_untouched(void*,const char*,uint32_t*,uint32_t*,uint8_t*,uint32_t*) noexcept { return platform::host_error_file_not_found; }
int32_t close_key(void*) noexcept { ++close_calls; return 0; }
}

int main(){
    platform::HostBridgeState s{};
    platform::reset_host_bridge(&s);
    platform::clear_registry_policy_state_for_test();
    s.originals.reg_open_a=&failed_a;
    s.originals.reg_open_w=&failed_w;
    s.originals.reg_close=&close_key;

#if defined(_WIN64)
    constexpr char active[]="SOFTWARE\\Wow6432Node\\Origin";
    constexpr uint16_t active_w[]={'S','O','F','T','W','A','R','E','\\','W','o','w','6','4','3','2','N','o','d','e','\\','O','r','i','g','i','n',0};
#else
    constexpr char active[]="SOFTWARE\\Origin";
    constexpr uint16_t active_w[]={'S','O','F','T','W','A','R','E','\\','O','r','i','g','i','n',0};
#endif

    // Failed originals intentionally leave caller storage untouched. The bridge
    // must synthesize from defined local state, not consume the incoming bits.
    void* a_result=reinterpret_cast<void*>(static_cast<uintptr_t>(0xDEADBEEF));
    if(platform::bridge_reg_open_a(&s,reinterpret_cast<void*>(1),active,0,0,&a_result)!=0) return 1;
    if(a_calls!=1 || !a_result || reinterpret_cast<uintptr_t>(a_result)==0xDEADBEEF) return 2;
    if(platform::bridge_reg_close(&s,a_result)!=0 || close_calls!=0) return 3; // pseudo close is local

    void* w_result=reinterpret_cast<void*>(static_cast<uintptr_t>(0xBAADF00D));
    if(platform::bridge_reg_open_w(&s,reinterpret_cast<void*>(1),active_w,0,0,&w_result)!=0) return 4;
    if(w_calls!=1 || !w_result || reinterpret_cast<uintptr_t>(w_result)==0xBAADF00D) return 5;
    if(platform::bridge_reg_close(&s,w_result)!=0 || close_calls!=0) return 6;

    // Success path still consumes and tracks the retained original handle.
    s.originals.reg_open_a=&success_a;
    void* real=nullptr;
    if(platform::bridge_reg_open_a(&s,reinterpret_cast<void*>(1),active,0,0,&real)!=0) return 7;
    if(reinterpret_cast<uintptr_t>(real)!=0x1234 || a_calls!=2) return 8;
    if(platform::bridge_reg_close(&s,real)!=0 || close_calls!=1) return 9;

    // F323-S0329-001: Win32 reports MultiByteToWideChar sizing failure as 0.
    // A nonempty configured value must return a checked error without
    // publishing type/required-size/output bytes as if it were empty.
    platform::clear_registry_policy_state_for_test();
    s.originals.reg_open_w=&failed_w;
    void* pseudo=nullptr;
    if(platform::bridge_reg_open_w(&s,reinterpret_cast<void*>(1),active_w,0,0,&pseudo)!=0 || !pseudo) return 10;
    constexpr char configured_path[]="C:/EA/App.exe";
    g_client_path=config::ValueView{configured_path,sizeof(configured_path)-1u,true};
    g_fail_wide_sizing=true;
    constexpr uint16_t client_path_w[]={'C','l','i','e','n','t','P','a','t','h',0};
    uint32_t type=0xA5A5A5A5u,bytes=0x5A5A5A5Au;
    uint8_t out[32]; std::memset(out,0xCC,sizeof(out));
    if(platform::registry_after_query_w(reinterpret_cast<uintptr_t>(pseudo),client_path_w,platform::host_error_file_not_found,sizeof(out),nullptr,0,&type,out,&bytes)!=platform::host_error_invalid_parameter) return 11;
    if(type!=0xA5A5A5A5u || bytes!=0x5A5A5A5Au) return 12;
    for(uint8_t b:out) if(b!=0xCCu) return 13;

    // Explicit empty is valid and must not call the failing conversion path.
    g_client_path=config::ValueView{"",0u,true};
    type=0u; bytes=0u; g_fail_wide_sizing=true;
    if(platform::registry_after_query_w(reinterpret_cast<uintptr_t>(pseudo),client_path_w,platform::host_error_file_not_found,0,nullptr,0,&type,nullptr,&bytes)!=platform::host_error_success) return 14;
    if(type!=platform::host_reg_sz || bytes!=2u) return 15;
    if(platform::bridge_reg_close(&s,pseudo)!=0) return 16;

    // Audit3 S0333: a failed original query may leave lpType untouched. The
    // synthesized overlay must publish its authoritative REG_SZ type without
    // reading or preserving those incoming bits.
    s.originals.reg_query_a=&failed_query_untouched;
    g_client_path=config::missing_value();
    void* query_pseudo=nullptr;
    s.originals.reg_open_a=&failed_a;
    if(platform::bridge_reg_open_a(&s,reinterpret_cast<void*>(1),active,0,0,&query_pseudo)!=0 || !query_pseudo) return 17;
    uint32_t query_type=0xA5A5A5A5u,query_bytes=sizeof(out);
    std::memset(out,0xCC,sizeof(out));
    s.executable_path.narrow="C:/Game/game.exe";s.executable_path.narrow_length=16u;
    if(platform::bridge_reg_query_a(&s,query_pseudo,"ClientPath",nullptr,&query_type,out,&query_bytes)!=platform::host_error_success) return 18;
    if(query_type!=platform::host_reg_sz || query_bytes!=17u || std::memcmp(out,"C:/Game/game.exe",17u)!=0) return 19;
    if(platform::bridge_reg_close(&s,query_pseudo)!=0) return 20;

    platform::clear_registry_policy_state_for_test();
    return 0;
}
