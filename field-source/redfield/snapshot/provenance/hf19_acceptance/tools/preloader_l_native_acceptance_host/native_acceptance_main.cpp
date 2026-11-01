#include "achievement_frame_contract.hpp"
#if !defined(_WIN32)
#error preloader_l_native_acceptance_host is Windows-only.
#endif
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include <stddef.h>
#include <stdint.h>
#include <cstdio> // Native acceptance diagnostics only; never linked into shipping DLL.
extern "C" {
using BOOL=int; using HMODULE=void*; using FARPROC=void(*)(); using SOCKET=uintptr_t;
__declspec(dllimport) HMODULE __stdcall LoadLibraryW(const wchar_t*) noexcept;
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE,const char*) noexcept;
__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE) noexcept;
__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(unsigned int) noexcept;
__declspec(dllimport) void __stdcall Sleep(unsigned long) noexcept;
__declspec(dllimport) unsigned long long __stdcall GetTickCount64() noexcept;
__declspec(dllimport) int __stdcall WSAGetLastError() noexcept;
__declspec(dllimport) unsigned long __stdcall GetEnvironmentVariableA(const char*,char*,unsigned long) noexcept;
__declspec(dllimport) int __stdcall WSAStartup(uint16_t,void*) noexcept;
__declspec(dllimport) int __stdcall WSACleanup() noexcept;
__declspec(dllimport) SOCKET __stdcall socket(int,int,int) noexcept;
__declspec(dllimport) int __stdcall connect(SOCKET,const void*,int) noexcept;
__declspec(dllimport) int __stdcall recv(SOCKET,char*,int,int) noexcept;
__declspec(dllimport) int __stdcall send(SOCKET,const char*,int,int) noexcept;
__declspec(dllimport) int __stdcall shutdown(SOCKET,int) noexcept;
__declspec(dllimport) int __stdcall closesocket(SOCKET) noexcept;
__declspec(dllimport) int __stdcall setsockopt(SOCKET,int,int,const char*,int) noexcept;
__declspec(dllimport) uint16_t __stdcall htons(uint16_t) noexcept;
// Keep real main-module Advapi32 IAT slots present for forced fallback acceptance.
__declspec(dllimport) long __stdcall RegOpenKeyExA(void*,const char*,unsigned long,unsigned long,void**) noexcept;
__declspec(dllimport) long __stdcall RegOpenKeyExW(void*,const wchar_t*,unsigned long,unsigned long,void**) noexcept;
__declspec(dllimport) long __stdcall RegQueryValueExA(void*,const char*,unsigned long*,unsigned long*,unsigned char*,unsigned long*) noexcept;
__declspec(dllimport) long __stdcall RegQueryValueExW(void*,const wchar_t*,unsigned long*,unsigned long*,unsigned char*,unsigned long*) noexcept;
__declspec(dllimport) long __stdcall RegCloseKey(void*) noexcept;
}
using NoArgStatusFn=uint32_t(*)() noexcept;
using WorkerMetricFn=uint32_t(*)(uint32_t) noexcept;
using ConfigScenarioFn=uint32_t(*)(uint32_t) noexcept;
using ShutdownFn=uint32_t(*)(uint32_t) noexcept;
using ConnectMetricFn=uint64_t(*)(uint32_t) noexcept;
using F02ControlFn=uint32_t(*)(uint32_t) noexcept;
using Wave4ControlFn=uint32_t(*)(uint32_t) noexcept;
using Campaign18ScenarioFn=uint32_t(*)(uint32_t) noexcept;
constexpr SOCKET kInvalidSocket=static_cast<SOCKET>(~static_cast<SOCKET>(0));
constexpr int kAfInet=2, kSockStream=1, kIpProtoTcp=6, kSdBoth=2;
constexpr int kSolSocket=0xffff, kSoRcvTimeo=0x1006, kSoSndTimeo=0x1005;
struct SockaddrIn16 { uint16_t family; uint16_t port; uint32_t address; uint8_t zero[8]; };
static_assert(sizeof(SockaddrIn16)==16,"sockaddr_in geometry");
[[noreturn]] static void finish(unsigned int code) noexcept { ExitProcess(code); }
// HF08/S0241: acceptance failures must never unload a live DLL. If DllShutdown
// is available, attempt the explicit shipping shutdown first; unload only after
// it reports success. Otherwise retain the module reference until ExitProcess.
static bool unload_after_confirmed_shutdown(HMODULE module) noexcept {
    return module && FreeLibrary(module)!=0;
}
static bool shutdown_then_unload(HMODULE module, ShutdownFn shutdown_runtime) noexcept {
    if(!module||!shutdown_runtime) return false;
    if(shutdown_runtime(30000u)!=0u) return false;
    return unload_after_confirmed_shutdown(module);
}
static uint16_t net16(uint16_t v) noexcept { return static_cast<uint16_t>((v>>8)|(v<<8)); }
static bool equal(const char* a,const char* b) noexcept {
    if(!a||!b) return false; size_t i=0; for(;;++i){ if(a[i]!=b[i]) return false; if(a[i]==0) return true; }
}
static bool wait_metric(WorkerMetricFn metric,uint32_t selector,uint32_t target,uint32_t timeout_ms) noexcept {
    for(uint32_t elapsed=0;elapsed<timeout_ms;elapsed+=10u){
        const uint32_t value=metric(selector);
        if(value!=0xffffffffu&&value>=target) return true;
        Sleep(10u);
    }
    const uint32_t value=metric(selector);
    return value!=0xffffffffu&&value>=target;
}
static bool wait_metric_equal(WorkerMetricFn metric,uint32_t selector,uint32_t target,uint32_t timeout_ms) noexcept {
    for(uint32_t elapsed=0;elapsed<timeout_ms;elapsed+=10u){ if(metric(selector)==target) return true; Sleep(10u); }
    return metric(selector)==target;
}
static bool wait_f02_metric(F02ControlFn control,uint32_t selector,uint32_t target,uint32_t timeout_ms) noexcept {
    for(uint32_t elapsed=0;elapsed<timeout_ms;elapsed+=10u){ if(control(selector)>=target) return true; Sleep(10u); }
    return control(selector)>=target;
}
static SOCKET open_prehandshake_client(uint16_t port) noexcept {
    SOCKET s=socket(kAfInet,kSockStream,kIpProtoTcp); if(s==kInvalidSocket) return kInvalidSocket;
    int timeout=3000; (void)setsockopt(s,kSolSocket,kSoRcvTimeo,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    (void)setsockopt(s,kSolSocket,kSoSndTimeo,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    SockaddrIn16 addr{}; addr.family=static_cast<uint16_t>(kAfInet); addr.port=net16(port); addr.address=0x0100007fu;
    if(connect(s,&addr,sizeof(addr))!=0){ closesocket(s); return kInvalidSocket; }
    char buffer[1024]{};
    if(recv(s,buffer,sizeof(buffer),0)<=0){ closesocket(s); return kInvalidSocket; }
    return s;
}
static void close_client(SOCKET s) noexcept { if(s==kInvalidSocket) return; (void)shutdown(s,kSdBoth); (void)closesocket(s); }

struct HookPrerequisiteModules { HMODULE advapi; };
static HookPrerequisiteModules load_hook_prerequisites() noexcept {
    HookPrerequisiteModules modules{};
    modules.advapi=LoadLibraryW(L"ADVAPI32.dll");
    if(!modules.advapi) return modules;
    static const char* names[]={"RegOpenKeyExA","RegOpenKeyExW","RegQueryValueExA","RegQueryValueExW","RegCloseKey"};
    for(size_t i=0;i<sizeof(names)/sizeof(names[0]);++i){
        if(!GetProcAddress(modules.advapi,names[i])){
            (void)FreeLibrary(modules.advapi);
            modules=HookPrerequisiteModules{};return modules;
        }
    }
    return modules;
}
static void unload_hook_prerequisites(HookPrerequisiteModules* modules) noexcept {
    if(!modules)return;
    if(modules->advapi){(void)FreeLibrary(modules->advapi);modules->advapi=nullptr;}
}

static bool contains_bytes(const char* data,size_t size,const char* needle,size_t n) noexcept {
    if(!data||!needle||size<n) return false;
    for(size_t i=0;i+n<=size;++i){ size_t j=0; for(;j<n;++j) if(data[i+j]!=needle[j]) break; if(j==n) return true; }
    return false;
}
static size_t find_bytes(const char* data,size_t size,const char* needle,size_t n,size_t start=0u) noexcept {
    if(!data||!needle||n==0u||start>size||size-start<n)return static_cast<size_t>(-1);
    for(size_t i=start;i+n<=size;++i){size_t j=0;for(;j<n;++j)if(data[i+j]!=needle[j])break;if(j==n)return i;}
    return static_cast<size_t>(-1);
}
static size_t count_bytes(const char* data,size_t size,const char* needle,size_t n) noexcept {
    size_t count=0u,pos=0u;
    while(pos<=size){const size_t hit=find_bytes(data,size,needle,n,pos);if(hit==static_cast<size_t>(-1))break;++count;pos=hit+n;}
    return count;
}
static bool exact_bytes(const char* data,size_t size,const char* expected,size_t expected_n) noexcept {
    if(!data||!expected||size!=expected_n)return false;
    for(size_t i=0;i<size;++i)if(data[i]!=expected[i])return false;
    return true;
}
static bool send_all_socket(SOCKET s,const char* data,size_t size) noexcept {
    size_t total=0; while(total<size){ const size_t remain=size-total; const int chunk=remain>0x7fffffffu?0x7fffffff:static_cast<int>(remain); const int n=send(s,data+total,chunk,0); if(n<=0||static_cast<size_t>(n)>remain)return false; total+=static_cast<size_t>(n); } return true;
}
// Bytewise receive is deliberate: it cannot consume and discard a second frame
// when the server coalesces multiple NUL-terminated responses in one TCP packet.
static bool recv_nul_frame(SOCKET s,char* out,size_t cap,size_t* written) noexcept {
    if(written)*written=0; if(!out||!written||cap==0)return false; size_t n=0;
    while(n<cap){ const int got=recv(s,out+n,1,0); if(got!=1)return false; ++n; if(out[n-1]=='\0'){*written=n;return true;} } return false;
}
struct ProtocolClient { SOCKET socket; uint8_t key[16]; };
static char g_protocol_frame[16384]{};
static char g_protocol_wire_a[8192]{};
static char g_protocol_wire_b[8192]{};
static char g_protocol_coalesced[16384]{};
static uint8_t g_protocol_decoded[16384]{};

static ProtocolClient open_protocol_client(uint16_t port) noexcept {
    ProtocolClient c{kInvalidSocket,{}};
    SOCKET s=socket(kAfInet,kSockStream,kIpProtoTcp); if(s==kInvalidSocket)return c;
    int timeout=4000; (void)setsockopt(s,kSolSocket,kSoRcvTimeo,reinterpret_cast<const char*>(&timeout),sizeof(timeout)); (void)setsockopt(s,kSolSocket,kSoSndTimeo,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    SockaddrIn16 addr{}; addr.family=static_cast<uint16_t>(kAfInet); addr.port=net16(port); addr.address=0x0100007fu;
    if(connect(s,&addr,sizeof(addr))!=0){closesocket(s);return c;}
    size_t greeting_n=0; if(!recv_nul_frame(s,g_protocol_frame,sizeof(g_protocol_frame),&greeting_n)||!contains_bytes(g_protocol_frame,greeting_n,"<Challenge key=\"",16u)){closesocket(s);return c;}
    static constexpr char handshake[]="<LSX><Request id=\"n18h\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
    if(!send_all_socket(s,handshake,sizeof(handshake))){closesocket(s);return c;}
    char reply[128]{}; size_t reply_n=0;
    if(preloader_l::codec::make_challenge_reply(reinterpret_cast<const uint8_t*>("a"),1,"3",1,c.key,reply,sizeof(reply),&reply_n)!=preloader_l::codec::SessionStatus::ok){closesocket(s);return c;}
    char expected[512]{};size_t expected_n=0;
    static constexpr char accepted_a[]=" <LSX>\n  <Response id=\"n18h\" sender=\"EALS\">\n    <ChallengeAccepted response=\"";
    static constexpr char accepted_b[]="\"/>\n  </Response>\n </LSX>\n";
    if((sizeof(accepted_a)-1u)+reply_n+(sizeof(accepted_b)-1u)+1u>sizeof(expected)){closesocket(s);return c;}
    for(size_t i=0;i<sizeof(accepted_a)-1u;++i)expected[expected_n++]=accepted_a[i];
    for(size_t i=0;i<reply_n;++i)expected[expected_n++]=reply[i];
    for(size_t i=0;i<sizeof(accepted_b)-1u;++i)expected[expected_n++]=accepted_b[i];
    expected[expected_n++]='\0';
    size_t accepted_n=0; if(!recv_nul_frame(s,g_protocol_frame,sizeof(g_protocol_frame),&accepted_n)||
       !exact_bytes(g_protocol_frame,accepted_n,expected,expected_n)){closesocket(s);return c;}
    c.socket=s; return c;
}
static bool encode_request(const ProtocolClient& c,const char* xml,size_t n,char* wire,size_t cap,size_t* wire_n) noexcept {
    return preloader_l::codec::encode_response(c.key,16,reinterpret_cast<const uint8_t*>(xml),n,wire,cap,wire_n)==preloader_l::codec::MessageCodecStatus::ok && *wire_n!=0u;
}
static bool recv_decoded(ProtocolClient& c,const char* marker) noexcept {
    size_t wire_n=0; if(!recv_nul_frame(c.socket,g_protocol_frame,sizeof(g_protocol_frame),&wire_n))return false; size_t decoded_n=0;
    if(preloader_l::codec::decode_payload(c.key,16,g_protocol_frame,wire_n,g_protocol_decoded,sizeof(g_protocol_decoded),&decoded_n)!=preloader_l::codec::MessageCodecStatus::ok)return false;
    size_t marker_n=0; while(marker[marker_n])++marker_n; return contains_bytes(reinterpret_cast<const char*>(g_protocol_decoded),decoded_n,marker,marker_n);
}
static bool request_expect(ProtocolClient& c,const char* xml,const char* marker) noexcept {
    size_t n=0; while(xml[n])++n; size_t wire_n=0; if(!encode_request(c,xml,n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n))return false; return send_all_socket(c.socket,g_protocol_wire_a,wire_n)&&recv_decoded(c,marker);
}
static bool recv_decoded_without(ProtocolClient& c,const char* required,const char* forbidden) noexcept {
    size_t wire_n=0; if(!recv_nul_frame(c.socket,g_protocol_frame,sizeof(g_protocol_frame),&wire_n))return false; size_t decoded_n=0;
    if(preloader_l::codec::decode_payload(c.key,16,g_protocol_frame,wire_n,g_protocol_decoded,sizeof(g_protocol_decoded),&decoded_n)!=preloader_l::codec::MessageCodecStatus::ok)return false;
    size_t required_n=0,forbidden_n=0; while(required[required_n])++required_n; while(forbidden[forbidden_n])++forbidden_n;
    const char* decoded=reinterpret_cast<const char*>(g_protocol_decoded);
    return contains_bytes(decoded,decoded_n,required,required_n)&&!contains_bytes(decoded,decoded_n,forbidden,forbidden_n);
}
static bool request_expect_without(ProtocolClient& c,const char* xml,const char* required,const char* forbidden) noexcept {
    size_t n=0; while(xml[n])++n; size_t wire_n=0; if(!encode_request(c,xml,n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n))return false;
    return send_all_socket(c.socket,g_protocol_wire_a,wire_n)&&recv_decoded_without(c,required,forbidden);
}
static SOCKET open_established_client(uint16_t port) noexcept {
    ProtocolClient c=open_protocol_client(port);
    if(c.socket==kInvalidSocket)return kInvalidSocket;
    static constexpr char probe[]="<LSX><Request id=\"life\"><GetConfig/></Request></LSX>";
    if(!request_expect(c,probe,"<GetConfigResponse>")){close_client(c.socket);return kInvalidSocket;}
    return c.socket;
}
static bool recv_achievement_frames(ProtocolClient& c,const char* achievement_id) noexcept {
    // Transaction policy B: Response and Event are TWO independently encoded
    // AES/hex+NUL frames. Do not validate both in the first decoded frame.
    char achievement[160]{};size_t achievement_n=0;
    static constexpr char prefix[]="<Achievement Id=\"";
    size_t id_n=0;while(achievement_id[id_n])++id_n;
    if((sizeof(prefix)-1u)+id_n+1u>sizeof(achievement))return false;
    for(size_t i=0;i<sizeof(prefix)-1u;++i)achievement[achievement_n++]=prefix[i];
    for(size_t i=0;i<id_n;++i)achievement[achievement_n++]=achievement_id[i];
    achievement[achievement_n++]='\"';

    size_t response_wire_n=0;
    if(!recv_nul_frame(c.socket,g_protocol_frame,sizeof(g_protocol_frame),&response_wire_n)){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=response receive_failed\n");return false;
    }
    size_t response_n=0;
    if(preloader_l::codec::decode_payload(c.key,16,g_protocol_frame,response_wire_n,g_protocol_decoded,sizeof(g_protocol_decoded),&response_n)!=preloader_l::codec::MessageCodecStatus::ok){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=response decode_failed\n");return false;
    }
    const char* response_xml=reinterpret_cast<const char*>(g_protocol_decoded);
    size_t response_xml_n=0;
    if(!achievement_frame_contract::strict_pkcs7_xml_length(response_xml,response_n,&response_xml_n)){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=response padding_failed padded_len=%zu\n",response_n);
        return false;
    }
    response_n=response_xml_n;
    if(!achievement_frame_contract::response(response_xml,response_n,achievement,achievement_n)){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=response shape_failed len=%zu id_count=%zu event_open_count=%zu\n",
            response_n,achievement_frame_contract::count(response_xml,response_n,achievement,achievement_n),
            achievement_frame_contract::count(response_xml,response_n,"<LSX><Event sender=\"EbisuSDK\">",sizeof("<LSX><Event sender=\"EbisuSDK\">")-1u));
        return false;
    }
    // recv_nul_frame reads one byte at a time; coalesced TCP packets cannot
    // cause the second encrypted frame to be discarded with the first.
    size_t event_wire_n=0;
    if(!recv_nul_frame(c.socket,g_protocol_frame,sizeof(g_protocol_frame),&event_wire_n)){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=event receive_failed response_len=%zu\n",response_n);return false;
    }
    size_t event_n=0;
    if(preloader_l::codec::decode_payload(c.key,16,g_protocol_frame,event_wire_n,g_protocol_decoded,sizeof(g_protocol_decoded),&event_n)!=preloader_l::codec::MessageCodecStatus::ok){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=event decode_failed response_len=%zu\n",response_n);return false;
    }
    const char* event_xml=reinterpret_cast<const char*>(g_protocol_decoded);
    size_t event_xml_n=0;
    if(!achievement_frame_contract::strict_pkcs7_xml_length(event_xml,event_n,&event_xml_n)){
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=event padding_failed response_len=%zu padded_len=%zu\n",response_n,event_n);
        return false;
    }
    event_n=event_xml_n;
    const bool event_ok=achievement_frame_contract::event(event_xml,event_n,achievement,achievement_n);
    if(!event_ok){
        // Privacy-safe structural evidence: disclose extents/offsets only, never XML contents.
        static constexpr char close[]="</Event></LSX>";
        const size_t at=achievement_frame_contract::find(event_xml,event_n,close,sizeof(close)-1u);
        const size_t suffix_gap=(at!=achievement_frame_contract::npos&&at+sizeof(close)-1u<=event_n)
            ? event_n-(at+sizeof(close)-1u):achievement_frame_contract::npos;
        std::fprintf(stderr,"NATIVE_ACH_TWO_FRAME_DIAG: stage=event shape_failed response_len=%zu event_len=%zu id_count=%zu event_open_count=%zu close_count=%zu suffix_gap=%zu nul_count=%zu\n",
            response_n,event_n,achievement_frame_contract::count(event_xml,event_n,achievement,achievement_n),
            achievement_frame_contract::count(event_xml,event_n,"<LSX><Event sender=\"EbisuSDK\">",sizeof("<LSX><Event sender=\"EbisuSDK\">")-1u),
            achievement_frame_contract::count(event_xml,event_n,close,sizeof(close)-1u),suffix_gap,
            achievement_frame_contract::count(event_xml,event_n,"\0",1u));
        return false;
    }
    return event_ok;
}
static bool request_expect_achievement_frames(ProtocolClient& c,const char* xml,const char* achievement_id) noexcept {
    size_t n=0;while(xml[n])++n;size_t wire_n=0;
    if(!encode_request(c,xml,n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n))return false;
    return send_all_socket(c.socket,g_protocol_wire_a,wire_n)&&recv_achievement_frames(c,achievement_id);
}
static uint32_t run_f02_delivery_stall_parallel_scenario(uint16_t port,WorkerMetricFn metric,F02ControlFn control) noexcept {
    ProtocolClient a=open_protocol_client(port);
    if(a.socket==kInvalidSocket)return 0x2C01u;
    ProtocolClient b{kInvalidSocket,{}};
    bool injection=false;
    uint32_t rc=0u;
    static constexpr char show[]="<LSX><Request id=\"f02a\"><ShowIGO/></Request></LSX>";
    static constexpr char get_config[]="<LSX><Request id=\"f02b\"><GetConfig/></Request></LSX>";
    if(control(0u)!=1u){rc=0x2C02u;goto done;}
    injection=true;
    {
        size_t show_n=0,wire_n=0; while(show[show_n])++show_n;
        if(!encode_request(a,show,show_n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n)||
           !send_all_socket(a.socket,g_protocol_wire_a,wire_n)){rc=0x2C03u;goto done;}
    }
    if(!wait_f02_metric(control,2u,1u,2000u)||control(3u)==0u){rc=0x2C04u;goto done;}
    b=open_protocol_client(port);
    if(b.socket==kInvalidSocket){rc=0x2C05u;goto done;}
    if(!request_expect(b,get_config,"<GetConfigResponse>")){rc=0x2C06u;goto done;}
    {
        // A persistent LSX client is NOT a completed worker merely because
        // GetConfig got a response. Close B, then prove its session retirement
        // progressed while A's prepared response remains artificially stalled.
        const uint32_t completed_before=metric(3u);
        close_client(b.socket);
        b.socket=kInvalidSocket;
        if(!wait_metric(metric,3u,completed_before+1u,7000u)){rc=0x2C07u;goto done;}
    }
    (void)control(1u); injection=false;
    // Verify no early IGO event was published to a separate post-stall client.
    b=open_protocol_client(port);
    if(b.socket==kInvalidSocket){rc=0x2C09u;goto done;}
    if(!request_expect_without(b,get_config,"<GetConfigResponse>","<IGOEvent State=\"DOWN\"/>")){rc=0x2C08u;goto done;}
 done:
    if(injection)(void)control(1u);
    close_client(a.socket); close_client(b.socket); return rc;
}
// IDS07: deterministic test-only asynchronous prepared-write hold. The native
// shipping session, strand, 5-second deadline timer and transaction rollback
// are real; the write is intentionally NOT submitted to Windows IOCP. Real
// kernel-pending async_write backpressure is separately tested by IDS07 fixture.
static uint32_t run_ids07_send_idle_timeout(uint16_t port,WorkerMetricFn metric,F02ControlFn control) noexcept {
    ProtocolClient a=open_protocol_client(port);
    if(a.socket==kInvalidSocket)return 0x3D01u;
    ProtocolClient b{kInvalidSocket,{}};
    bool injection=false;
    uint32_t rc=0u;
    static constexpr char show[]="<LSX><Request id=\"ids07a\"><ShowIGO/></Request></LSX>";
    static constexpr char get_config[]="<LSX><Request id=\"ids07b\"><GetConfig/></Request></LSX>";
    if(control(0u)!=1u || control(5u)!=0u){rc=0x3D02u;goto done;}
    injection=true;
    {
        size_t n=0,wire_n=0;while(show[n])++n;
        if(!encode_request(a,show,n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n) ||
           !send_all_socket(a.socket,g_protocol_wire_a,wire_n)){rc=0x3D03u;goto done;}
    }
    {
        const unsigned long long start=GetTickCount64();
        if(!wait_f02_metric(control,2u,1u,2000u) || control(3u)==0u){rc=0x3D04u;goto done;}
        // A second real Asio IOCP session must service ordinary requests while
        // A's write remains stalled. No global queue/send lock is held.
        b=open_protocol_client(port);
        if(b.socket==kInvalidSocket || !request_expect(b,get_config,"<GetConfigResponse>")){
            rc=0x3D05u;goto done;
        }
        close_client(b.socket);b.socket=kInvalidSocket;
        // Receive blocks until server CLOSE or 9.5s deadline; no Response or
        // Event byte may escape A before the 5-second timeout.
        const int timeout_ms=9500;
        (void)setsockopt(a.socket,kSolSocket,kSoRcvTimeo,
            reinterpret_cast<const char*>(&timeout_ms),sizeof(timeout_ms));
        char item=0;
        const int received=recv(a.socket,&item,1,0);
        const int socket_error=received<0?WSAGetLastError():0;
        const unsigned long long elapsed=GetTickCount64()-start;
        if(received>0){rc=0x3D06u;goto done;}
        if(received<0 && socket_error!=10054 && socket_error!=10053 && socket_error!=10058){
            rc=0x3D07u;goto done;
        }
        if(elapsed<4500ull || elapsed>9000ull){rc=0x3D08u;goto done;}
        if(control(5u)!=1u || control(2u)==0u){rc=0x3D09u;goto done;}
        std::printf("IDS07_NATIVE_SEND_TIMEOUT PASS elapsed_ms=%llu timeout_count=%u\n",
            elapsed,control(5u));
    }
    (void)control(1u);injection=false;
    b=open_protocol_client(port);
    if(b.socket==kInvalidSocket ||
       !request_expect_without(b,get_config,"<GetConfigResponse>","<IGOEvent State=\"DOWN\"/>")){
        rc=0x3D0Au;goto done;
    }
    std::puts("IDS07_PARALLEL_AND_NO_EVENT_LEAK PASS");
    std::fflush(stdout);
 done:
    if(injection)(void)control(1u);
    close_client(a.socket);close_client(b.socket);
    return rc;
}
static uint32_t run_protocol_scenario(const char* scenario,uint16_t port) noexcept {
    ProtocolClient c=open_protocol_client(port); if(c.socket==kInvalidSocket)return 0x2B01u;
    uint32_t rc=0u;
    static constexpr char get_config[]="<LSX><Request id=\"n18c\"><GetConfig/></Request></LSX>";
    if(equal(scenario,"frame_fragmentation")){
        size_t n=0,wire_n=0; while(get_config[n])++n; if(!encode_request(c,get_config,n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wire_n)||wire_n<4u)rc=0x2B10u;
        else { const size_t cut=wire_n/2u; if(!send_all_socket(c.socket,g_protocol_wire_a,cut)||!send_all_socket(c.socket,g_protocol_wire_a+cut,wire_n-cut)||!recv_decoded(c,"<GetConfigResponse>"))rc=0x2B11u; }
    } else if(equal(scenario,"frame_coalescing")){
        static constexpr char q2[]="<LSX><Request id=\"n18b\"><GetBlockList/></Request></LSX>";
        size_t a_n=0,b_n=0,wa=0,wb=0; while(get_config[a_n])++a_n; while(q2[b_n])++b_n;
        if(!encode_request(c,get_config,a_n,g_protocol_wire_a,sizeof(g_protocol_wire_a),&wa)||!encode_request(c,q2,b_n,g_protocol_wire_b,sizeof(g_protocol_wire_b),&wb)||wa+wb>sizeof(g_protocol_coalesced))rc=0x2B20u;
        else { for(size_t i=0;i<wa;++i)g_protocol_coalesced[i]=g_protocol_wire_a[i]; for(size_t i=0;i<wb;++i)g_protocol_coalesced[wa+i]=g_protocol_wire_b[i]; if(!send_all_socket(c.socket,g_protocol_coalesced,wa+wb)||!recv_decoded(c,"<GetConfigResponse>")||!recv_decoded(c,"<GetBlockListResponse"))rc=0x2B21u; }
    } else if(equal(scenario,"frame_malformed")){
        static constexpr char bad[]="zz\0"; if(!send_all_socket(c.socket,bad,sizeof(bad))||!request_expect(c,get_config,"<GetConfigResponse>"))rc=0x2B30u;
    } else if(equal(scenario,"protocol_route_surface")){
        if(!request_expect(c,get_config,"<GetConfigResponse>"))rc=0x2B41u;
        else if(!request_expect(c,"<LSX><Request id=\"n18p\"><GetPresenceVisibility/></Request></LSX>","<GetPresenceVisibilityResponse Visible=\"false\"/>"))rc=0x2B42u;
        else if(!request_expect(c,"<LSX><Request id=\"n18r\"><RestartGame/></Request></LSX>","<ErrorSuccess Description=\"Origin is waiting for the game to shutdown.\" Code=\"1\"/>"))rc=0x2B43u;
        else if(!request_expect(c,"<LSX><Request id=\"n18v\"><GetVoipStatus/></Request></LSX>","<ErrorSuccess Description=\"The function you are calling is not implemented.\" Code=\"-1593835518\"/>"))rc=0x2B44u;
        else if(!request_expect(c,"<LSX><Request id=\"n18l\"><GetBlockList/></Request></LSX>","<GetBlockListResponse Return=\"Success\"/>"))rc=0x2B45u;
    } else if(equal(scenario,"store_content_entitlement")){
        if(!request_expect(c,"<LSX><Request id=\"n18q1\"><QueryContent/></Request></LSX>","<QueryContentResponse")||
           !request_expect(c,"<LSX><Request id=\"n18q2\"><QueryEntitlements/></Request></LSX>","<QueryEntitlementsResponse")||
           !request_expect(c,"<LSX><Request id=\"n18q3\"><QueryOffers/></Request></LSX>","<QueryOffersResponse"))rc=0x2B50u;
    } else if(equal(scenario,"achievement_event_delivery")){
        if(!request_expect(c,"<LSX><Request id=\"n18a1\"><QueryAchievements/></Request></LSX>","<AchievementSets>"))rc=0x2B60u;
        else if(!request_expect_achievement_frames(c,"<LSX><Request id=\"n18a2\"><GrantAchievement><AchievementId>NATIVE_CAMPAIGN18</AchievementId></GrantAchievement></Request></LSX>",
                                                    "NATIVE_CAMPAIGN18"))rc=0x2B61u;
    } else rc=0x2BFFu;
    close_client(c.socket); return rc;
}

static bool wait_wave4_metric(Wave4ControlFn control,uint32_t selector,uint32_t baseline,uint32_t timeout_ms) noexcept {
    for(uint32_t elapsed=0;elapsed<timeout_ms;elapsed+=10u){ if(control(selector)>baseline) return true; Sleep(10u); }
    return control(selector)>baseline;
}
static unsigned int run_wave4_resource_retry_scenario() noexcept {
    // Generation 1: the IOCP session socket close is withheld while fault
    // injection is armed. Exact Asio session ownership must remain registered
    // and shutdown after disarming must physically close and retire it.
    {
        HMODULE module=LoadLibraryW(L"preloader_l_native_test.dll");
        if(!module) return 210u;
        auto* ready_proc=GetProcAddress(module,"preloader_l_debug_wait_ready");
        auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
        auto* metric_proc=GetProcAddress(module,"preloader_l_native_acceptance_worker_metric");
        auto* control_proc=GetProcAddress(module,"preloader_l_native_acceptance_wave4_control");
        if(!ready_proc||!shutdown_proc||!metric_proc||!control_proc){ return 211u; }
        auto ready=reinterpret_cast<NoArgStatusFn>(ready_proc);
        auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
        auto metric=reinterpret_cast<WorkerMetricFn>(metric_proc);
        auto control=reinterpret_cast<Wave4ControlFn>(control_proc);
        if(ready()!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); return 212u; }
        const uint16_t port=static_cast<uint16_t>(metric(0u));
        if(!port){ (void)shutdown_then_unload(module,shutdown_runtime); return 213u; }
        SOCKET client=open_prehandshake_client(port);
        if(client==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); return 214u; }
        const uint32_t retained0=control(3u);
        (void)control(1u);
        close_client(client);
        if(!wait_wave4_metric(control,3u,retained0,5000u)){ (void)control(0u); (void)shutdown_then_unload(module,shutdown_runtime); return 215u; }
        if(control(5u)==0u){ (void)control(0u); (void)shutdown_then_unload(module,shutdown_runtime); return 216u; }
        (void)control(0u);
        const uint32_t shutdown_rc=shutdown_runtime(30000u);
        if(shutdown_rc!=0u) return 217u+(shutdown_rc&0x0fu);
        if(control(5u)!=0u) return 218u;
        if(!unload_after_confirmed_shutdown(module)) return 219u;
    }

    // Generation 2: the Asio listener acceptor close is withheld while fault
    // injection is armed. No fabricated ModelJthread or raw worker HANDLE.
    // First shutdown must report cleanup pending (10); after disarm the
    // second call must retry the same acceptor and converge to success.
    {
        HMODULE module=LoadLibraryW(L"preloader_l_native_test.dll");
        if(!module) return 220u;
        auto* ready_proc=GetProcAddress(module,"preloader_l_debug_wait_ready");
        auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
        auto* metric_proc=GetProcAddress(module,"preloader_l_native_acceptance_worker_metric");
        auto* control_proc=GetProcAddress(module,"preloader_l_native_acceptance_wave4_control");
        if(!ready_proc||!shutdown_proc||!metric_proc||!control_proc){ return 221u; }
        auto ready=reinterpret_cast<NoArgStatusFn>(ready_proc);
        auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
        auto metric=reinterpret_cast<WorkerMetricFn>(metric_proc);
        auto control=reinterpret_cast<Wave4ControlFn>(control_proc);
        if(ready()!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); return 222u; }
        const uint16_t port=static_cast<uint16_t>(metric(0u));
        if(!port){ (void)shutdown_then_unload(module,shutdown_runtime); return 223u; }
        SOCKET client=open_prehandshake_client(port);
        if(client==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); return 224u; }
        const uint32_t retained0=control(4u);
        (void)control(2u);
        const uint32_t first=shutdown_runtime(30000u);
        if(first!=10u){ close_client(client); (void)control(0u); return 225u+(first&0x0fu); }
        if(control(4u)<=retained0){ close_client(client); (void)control(0u); return 226u; }
        (void)control(0u);
        const uint32_t second=shutdown_runtime(30000u);
        close_client(client);
        if(second!=0u) return 227u+(second&0x0fu);
        if(!unload_after_confirmed_shutdown(module)) return 228u;
    }
    return 0u;
}
static unsigned int run_reload_generation_scenario() noexcept {
    for(uint32_t generation=0; generation<2u; ++generation){
        HMODULE module=LoadLibraryW(L"preloader_l_native_test.dll");
        if(!module) return 100u+generation;
        auto* ready_proc=GetProcAddress(module,"preloader_l_debug_wait_ready");
        auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
        if(!ready_proc||!shutdown_proc){ return 110u+generation; }
        auto ready=reinterpret_cast<NoArgStatusFn>(ready_proc);
        auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
        const uint32_t ready_rc=ready();
        if(ready_rc!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); return 120u+(ready_rc&0x0fu); }
        const uint32_t shutdown_rc=shutdown_runtime(30000u);
        if(shutdown_rc!=0u) return 140u+(shutdown_rc&0x0fu);
        if(!unload_after_confirmed_shutdown(module)) return 160u+generation;
    }
    return 0u;
}
static unsigned int run_connect_redirect_shipping_lifecycle_scenario() noexcept {
    for(uint32_t generation=0; generation<2u; ++generation){
        HMODULE module=LoadLibraryW(L"preloader_l_native_test.dll");
        if(!module) return 170u+generation;
        auto* ready_proc=GetProcAddress(module,"preloader_l_debug_wait_ready");
        auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
        auto* metric_proc=GetProcAddress(module,"preloader_l_native_acceptance_connect_metric");
        if(!ready_proc||!shutdown_proc||!metric_proc){ return 172u+generation; }
        auto ready=reinterpret_cast<NoArgStatusFn>(ready_proc);
        auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
        auto connect_metric=reinterpret_cast<ConnectMetricFn>(metric_proc);
        const uint32_t ready_rc=ready();
        if(ready_rc!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); return 174u+(ready_rc&0x0fu); }
        const uint64_t selected=connect_metric(0u);
        if(selected<3226u||selected>3236u||connect_metric(3u)==0u||connect_metric(4u)==0u){ (void)shutdown_then_unload(module,shutdown_runtime); return 176u+generation; }

        // The old process-wide htons interception must be gone.
        if(htons(3216u)!=net16(3216u)){ (void)shutdown_then_unload(module,shutdown_runtime); return 178u+generation; }

        const uint64_t calls0=connect_metric(1u), mapped0=connect_metric(2u);
        SOCKET redirected=open_prehandshake_client(3216u);
        if(redirected==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); return 180u+generation; }
        close_client(redirected);
        if(connect_metric(1u)<=calls0||connect_metric(2u)<=mapped0){ (void)shutdown_then_unload(module,shutdown_runtime); return 182u+generation; }

        // A direct connection to the selected listener port must pass through
        // unchanged and therefore must not increase the mapped counter.
        const uint64_t mapped1=connect_metric(2u);
        SOCKET direct=open_prehandshake_client(static_cast<uint16_t>(selected));
        if(direct==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); return 184u+generation; }
        close_client(direct);
        if(connect_metric(2u)!=mapped1){ (void)shutdown_then_unload(module,shutdown_runtime); return 186u+generation; }

        const uint32_t shutdown_rc=shutdown_runtime(30000u);
        if(shutdown_rc!=0u) return 188u+(shutdown_rc&0x0fu);
        if(connect_metric(3u)!=0u||connect_metric(4u)!=0u) return 190u+generation;
        const uint64_t calls1=connect_metric(1u), mapped2=connect_metric(2u);
        if(htons(3216u)!=net16(3216u)) return 192u+generation;
        if(connect_metric(1u)!=calls1||connect_metric(2u)!=mapped2) return 194u+generation;
        if(!unload_after_confirmed_shutdown(module)) return 196u+generation;
    }
    return 0u;
}
int main() {
    char scenario[32]{}; const unsigned long n=GetEnvironmentVariableA("PRELOADER_L_ACCEPTANCE_SCENARIO",scenario,sizeof(scenario));
    if(n==0||n>=sizeof(scenario)){ scenario[0]='o';scenario[1]='n';scenario[2]='e';scenario[3]=0; }
    HookPrerequisiteModules hook_prerequisites=load_hook_prerequisites();
    if(!hook_prerequisites.advapi) finish(4);
    uint8_t wsa[512]{}; if(WSAStartup(0x0202u,wsa)!=0) finish(3);
    if(equal(scenario,"reload_generation")){
        const unsigned int reload_rc=run_reload_generation_scenario();
        WSACleanup();
        unload_hook_prerequisites(&hook_prerequisites);
        finish(reload_rc);
    }
    if(equal(scenario,"connect_redirect_lifecycle")){
        const unsigned int connect_rc=run_connect_redirect_shipping_lifecycle_scenario();
        WSACleanup();
        unload_hook_prerequisites(&hook_prerequisites);
        finish(connect_rc);
    }
    if(equal(scenario,"wave4_resource_retry")){
        const unsigned int wave4_rc=run_wave4_resource_retry_scenario();
        WSACleanup();
        unload_hook_prerequisites(&hook_prerequisites);
        finish(wave4_rc);
    }
    HMODULE module=LoadLibraryW(L"preloader_l_native_test.dll");
    if(!module){ WSACleanup(); finish(10); }
    auto* ready_proc=GetProcAddress(module,"preloader_l_debug_wait_ready");
    auto* teardown_proc=GetProcAddress(module,"preloader_l_debug_controlled_teardown");
    auto* metric_proc=GetProcAddress(module,"preloader_l_native_acceptance_worker_metric");
    auto* config_proc=GetProcAddress(module,"preloader_l_native_acceptance_config_scenario");
    auto* f02_proc=GetProcAddress(module,"preloader_l_native_acceptance_f02_control");
    auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
    auto* campaign18_proc=GetProcAddress(module,"preloader_l_native_acceptance_campaign18_scenario");
    if(!ready_proc||!teardown_proc||!metric_proc||!config_proc||!f02_proc||!shutdown_proc||!campaign18_proc){ WSACleanup(); finish(11); }
    auto ready=reinterpret_cast<NoArgStatusFn>(ready_proc);
    auto teardown=reinterpret_cast<NoArgStatusFn>(teardown_proc);
    auto metric=reinterpret_cast<WorkerMetricFn>(metric_proc);
    auto config_scenario=reinterpret_cast<ConfigScenarioFn>(config_proc);
    auto f02_control=reinterpret_cast<F02ControlFn>(f02_proc);
    auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
    auto campaign18=reinterpret_cast<Campaign18ScenarioFn>(campaign18_proc);

    if(equal(scenario,"prestart_stop")){
        const uint32_t rc=shutdown_runtime(30000u);
        if(rc!=0u){ WSACleanup(); finish(120u+(rc&0x0fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(139); }
        WSACleanup(); finish(0);
    }

    const uint32_t ready_rc=ready(); if(ready_rc!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(12u+(ready_rc&0x0fu)); }
    const uint16_t port=static_cast<uint16_t>(metric(0u)); if(!port){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(14); }

    if(equal(scenario,"ids07_send_idle_timeout")){
        const uint32_t rr=run_ids07_send_idle_timeout(port,metric,f02_control);
        if(rr!=0u){ std::fprintf(stderr,"IDS07_NATIVE_SEND_TIMEOUT FAIL code=%u\n",rr);
            (void)shutdown_then_unload(module,shutdown_runtime);
            WSACleanup();finish(160u+(rr&0x3fu)); }
        if(!shutdown_then_unload(module,shutdown_runtime)){WSACleanup();finish(222u);}
        WSACleanup();finish(0);
    }
    if(equal(scenario,"f02_delivery_stall_parallel")){
        const uint32_t fr=run_f02_delivery_stall_parallel_scenario(port,metric,f02_control);
        if(fr!=0u){ const uint32_t tr_fail=teardown(); if(tr_fail==0u)(void)(void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(192u+(fr&0x3fu)); }
        const uint32_t tr=teardown();
        if(tr!=0u){ WSACleanup(); finish(232u+(tr&0x0fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(249); }
        WSACleanup(); finish(0);
    }

    uint32_t config_selector=0xffffffffu;
    if(equal(scenario,"path_ascii_sso7")) config_selector=0u;
    else if(equal(scenario,"path_unicode_heap")) config_selector=1u;
    else if(equal(scenario,"path_long_replace")) config_selector=2u;
    else if(equal(scenario,"xml_no_bom")) config_selector=3u;
    else if(equal(scenario,"xml_bom")) config_selector=4u;
    else if(equal(scenario,"xml_large")) config_selector=5u;
    else if(equal(scenario,"xml_invalid_retain")) config_selector=6u;
    else if(equal(scenario,"xml_replace")) config_selector=7u;
    else if(equal(scenario,"xml_clear")) config_selector=8u;
    if(config_selector!=0xffffffffu){
        const uint32_t cr=config_scenario(config_selector);
        if(cr!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(60u+(cr&0x1fu)); }
        const uint32_t rc=teardown();
        if(rc!=0u){ WSACleanup(); finish(80u+(rc&0x1fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(90); }
        WSACleanup(); finish(0);
    }

    uint32_t campaign_selector=0xffffffffu;
    if(equal(scenario,"ptd_fls_lifecycle")) campaign_selector=0u;
    if(campaign_selector!=0xffffffffu){
        const uint32_t cr=campaign18(campaign_selector);
        if(cr!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(140u+(cr&0x1fu)); }
        const uint32_t tr=teardown();
        if(tr!=0u){ WSACleanup(); finish(160u+(tr&0x1fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(190); }
        WSACleanup(); finish(0);
    }

    if(equal(scenario,"frame_fragmentation")||equal(scenario,"frame_coalescing")||equal(scenario,"frame_malformed")||
       equal(scenario,"protocol_route_surface")||equal(scenario,"store_content_entitlement")||equal(scenario,"achievement_event_delivery")){
        const uint32_t pr=run_protocol_scenario(scenario,port);
        if(pr!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(200u+(pr&0x1fu)); }
        const uint32_t tr=teardown();
        if(tr!=0u){ WSACleanup(); finish(220u+(tr&0x1fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(250); }
        WSACleanup(); finish(0);
    }

    if(equal(scenario,"shipping_shutdown")) {
        const uint32_t rc=shutdown_runtime(30000u);
        if(rc!=0u){ WSACleanup(); finish(31u+(rc&0x0fu)); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(39); }
        WSACleanup(); finish(0);
    } else if(equal(scenario,"zero")) {
        if(metric(1u)!=0u||metric(5u)!=0u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(40); }
    } else if(equal(scenario,"one")) {
        SOCKET s=open_established_client(port); if(s==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(41); }
        close_client(s);
        if(!wait_metric(metric,3u,1u,5000u)) { (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(42); }
        if(!wait_metric(metric,7u,1u,5000u)||!wait_metric_equal(metric,5u,0u,5000u)){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(43); }
        if(metric(1u)<1u||metric(2u)<1u||metric(4u)<1u||metric(6u)<1u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(44); }
    } else if(equal(scenario,"growth")) {
        for(uint32_t i=0;i<6u;++i){
            SOCKET s=open_established_client(port); if(s==kInvalidSocket){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(44); }
            close_client(s);
            if(!wait_metric(metric,3u,i+1u,5000u)){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(45); }
        }
        if(!wait_metric(metric,7u,6u,5000u)||!wait_metric_equal(metric,5u,0u,5000u)){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(46); }
        if(metric(1u)<6u||metric(2u)<6u||metric(3u)<6u||metric(4u)<6u||metric(6u)<1u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(70); }
    } else if(equal(scenario,"parallel")) {
        SOCKET clients[3]{kInvalidSocket,kInvalidSocket,kInvalidSocket};
        for(uint32_t i=0;i<3u;++i){ clients[i]=open_established_client(port); if(clients[i]==kInvalidSocket){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(47); } }
        if(!wait_metric(metric,2u,3u,5000u)||metric(4u)<3u||metric(5u)<3u){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(48); }
        for(auto c:clients) close_client(c);
        if(!wait_metric(metric,3u,3u,5000u)){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(49); }
    } else if(equal(scenario,"active_shutdown")) {
        SOCKET clients[3]{kInvalidSocket,kInvalidSocket,kInvalidSocket};
        for(uint32_t i=0;i<3u;++i){ clients[i]=open_established_client(port); if(clients[i]==kInvalidSocket){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(47); } }
        if(!wait_metric(metric,2u,3u,5000u)||metric(4u)<3u||metric(5u)<3u){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(48); }
        const uint32_t rc=teardown();
        for(auto c:clients) close_client(c);
        if(rc!=0u){ WSACleanup(); finish(49u+(rc&0x0fu)); }
        if(metric(3u)<3u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(50); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(51); }
        WSACleanup(); finish(0);
    } else if(equal(scenario,"blocked_shutdown")) {
        SOCKET clients[3]{kInvalidSocket,kInvalidSocket,kInvalidSocket};
        for(uint32_t i=0;i<3u;++i){ clients[i]=open_prehandshake_client(port); if(clients[i]==kInvalidSocket){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(53); } }
        if(!wait_metric(metric,2u,3u,10000u)){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(54); }
        if(!wait_metric(metric,5u,3u,10000u)){ for(auto c:clients) close_client(c); (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(58); }
        const uint32_t rc=teardown();
        for(auto c:clients) close_client(c);
        if(rc!=0u){ WSACleanup(); finish(55u+(rc&0x0fu)); }
        if(metric(3u)<3u){ (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(56); }
        if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(57); }
        WSACleanup(); finish(0);
    } else { (void)shutdown_then_unload(module,shutdown_runtime); WSACleanup(); finish(69); }

    const uint32_t rc=teardown();
    if(rc!=0u){ WSACleanup(); finish(20u+(rc&0xffu)); }
    if(!unload_after_confirmed_shutdown(module)){ WSACleanup(); finish(30); }
    WSACleanup(); finish(0);
}
