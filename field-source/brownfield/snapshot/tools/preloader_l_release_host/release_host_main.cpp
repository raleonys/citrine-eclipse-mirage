#if !defined(_WIN32)
#error preloader_l_release_host is Windows-only.
#endif
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include <stddef.h>
#include <stdint.h>

extern "C" {
using BOOL=int; using HMODULE=void*; using FARPROC=void(*)(); using SOCKET=uintptr_t;
struct SockAddr { uint16_t family; char data[14]; };
__declspec(dllimport) HMODULE __stdcall LoadLibraryW(const wchar_t*);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE,const char*);
__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE);
__declspec(dllimport) void __stdcall Sleep(unsigned long);
__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(unsigned int);
__declspec(dllimport) int __stdcall WSAStartup(uint16_t,void*);
__declspec(dllimport) int __stdcall WSACleanup();
__declspec(dllimport) SOCKET __stdcall socket(int,int,int);
__declspec(dllimport) int __stdcall connect(SOCKET,const SockAddr*,int);
__declspec(dllimport) int __stdcall send(SOCKET,const char*,int,int);
__declspec(dllimport) int __stdcall recv(SOCKET,char*,int,int);
__declspec(dllimport) int __stdcall setsockopt(SOCKET,int,int,const char*,int);
__declspec(dllimport) int __stdcall closesocket(SOCKET);
}

using ShutdownFn=uint32_t(__stdcall*)(uint32_t) noexcept;
struct SockAddrIn { uint16_t family; uint16_t port; uint32_t address; uint8_t zero[8]; };
static_assert(sizeof(SockAddrIn)==16,"sockaddr_in");
constexpr SOCKET invalid_socket=static_cast<SOCKET>(~static_cast<SOCKET>(0));
constexpr uint16_t swap16(uint16_t v) noexcept { return static_cast<uint16_t>((v<<8)|(v>>8)); }
constexpr int sol_socket=0xFFFF;
constexpr int so_rcvtimeo=0x1006;
constexpr int so_sndtimeo=0x1005;
namespace {
alignas(void*) uint8_t g_wsa[408];
char g_frame[8192];
char g_wire[8192];
uint8_t g_decoded[8192];

[[noreturn]] void finish(unsigned int c) noexcept { ExitProcess(c); }

struct HookPrerequisiteModules { HMODULE advapi; };
HookPrerequisiteModules load_hook_prerequisites() noexcept {
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
void unload_hook_prerequisites(HookPrerequisiteModules* modules) noexcept {
    if(!modules)return;
    if(modules->advapi){(void)FreeLibrary(modules->advapi);modules->advapi=nullptr;}
}

bool contains(const char* data,size_t size,const char* needle,size_t n) noexcept {
    if(!data||!needle||size<n) return false;
    for(size_t i=0;i+n<=size;++i){
        size_t j=0; for(;j<n;++j) if(data[i+j]!=needle[j]) break;
        if(j==n) return true;
    }
    return false;
}

bool send_all_socket(SOCKET s,const char* data,size_t size) noexcept {
    if(!data && size!=0) return false;
    size_t total=0;
    while(total<size){
        const size_t remain=size-total;
        const int chunk=remain>static_cast<size_t>(0x7fffffff)?0x7fffffff:static_cast<int>(remain);
        const int sent=send(s,data+total,chunk,0);
        if(sent<=0 || static_cast<size_t>(sent)>remain) return false;
        total+=static_cast<size_t>(sent);
    }
    return true;
}

bool recv_nul_frame(SOCKET s,char* output,size_t capacity,size_t* written) noexcept {
    if(written) *written=0;
    if(!output||capacity==0||!written) return false;
    size_t total=0;
    while(total<capacity){
        const size_t remain=capacity-total;
        const int chunk=remain>static_cast<size_t>(0x7fffffff)?0x7fffffff:static_cast<int>(remain);
        const int got=recv(s,output+total,chunk,0);
        if(got<=0 || static_cast<size_t>(got)>remain) return false;
        const size_t begin=total;
        total+=static_cast<size_t>(got);
        for(size_t i=begin;i<total;++i){
            if(output[i]=='\0'){
                *written=i+1u;
                return true;
            }
        }
    }
    return false;
}

bool exact_challenge_accepted(const char* frame,size_t frame_n,const char* response,size_t response_n) noexcept {
    static const char prefix[]="<ChallengeAccepted response=\"";
    static const char suffix[]="\"/>";
    if(!contains(frame,frame_n,prefix,sizeof(prefix)-1)) return false;
    if(!contains(frame,frame_n,response,response_n)) return false;
    return contains(frame,frame_n,suffix,sizeof(suffix)-1);
}

SOCKET try_port(uint16_t port) noexcept {
    SOCKET s=socket(2,1,6); if(s==invalid_socket) return invalid_socket;
    int timeout_ms=1200;
    (void)setsockopt(s,sol_socket,so_rcvtimeo,reinterpret_cast<const char*>(&timeout_ms),sizeof(timeout_ms));
    (void)setsockopt(s,sol_socket,so_sndtimeo,reinterpret_cast<const char*>(&timeout_ms),sizeof(timeout_ms));
    SockAddrIn addr{}; addr.family=2; addr.port=swap16(port); addr.address=0x0100007Fu;
    if(connect(s,reinterpret_cast<const SockAddr*>(&addr),sizeof(addr))!=0){ closesocket(s); return invalid_socket; }

    // Protocol authority: the server always speaks first with a plaintext,
    // NUL-terminated Challenge greeting.
    size_t greeting_n=0;
    static const char challenge_marker[]="<Challenge key=\"";
    if(!recv_nul_frame(s,g_frame,sizeof(g_frame),&greeting_n) ||
       !contains(g_frame,greeting_n,challenge_marker,sizeof(challenge_marker)-1)){
        closesocket(s); return invalid_socket;
    }

    static const char handshake[]=
        "<LSX><Request id=\"55h\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";
    if(!send_all_socket(s,handshake,sizeof(handshake))){ closesocket(s); return invalid_socket; }

    uint8_t session_key[16]{};
    char challenge_reply[128]{};
    size_t challenge_reply_n=0;
    if(preloader_l::codec::make_challenge_reply(
           reinterpret_cast<const uint8_t*>("a"),1,"3",1,
           session_key,challenge_reply,sizeof(challenge_reply),&challenge_reply_n)!=
           preloader_l::codec::SessionStatus::ok){
        closesocket(s); return invalid_socket;
    }

    size_t accepted_n=0;
    if(!recv_nul_frame(s,g_frame,sizeof(g_frame),&accepted_n) ||
       !exact_challenge_accepted(g_frame,accepted_n,challenge_reply,challenge_reply_n)){
        closesocket(s); return invalid_socket;
    }

    static const char request[]="<LSX><Request id=\"55\"><GetConfig/></Request></LSX>";
    size_t wire_n=0;
    if(preloader_l::codec::encode_response(
           session_key,16,reinterpret_cast<const uint8_t*>(request),sizeof(request)-1,
           g_wire,sizeof(g_wire),&wire_n)!=preloader_l::codec::MessageCodecStatus::ok || wire_n==0){
        closesocket(s); return invalid_socket;
    }
    if(!send_all_socket(s,g_wire,wire_n)){ closesocket(s); return invalid_socket; }

    size_t response_wire_n=0;
    if(!recv_nul_frame(s,g_frame,sizeof(g_frame),&response_wire_n)){
        closesocket(s); return invalid_socket;
    }
    size_t decoded_n=0;
    if(preloader_l::codec::decode_payload(
           session_key,16,g_frame,response_wire_n,g_decoded,sizeof(g_decoded),&decoded_n)!=
           preloader_l::codec::MessageCodecStatus::ok){
        closesocket(s); return invalid_socket;
    }
    static const char marker[]="<GetConfigResponse>";
    if(!contains(reinterpret_cast<const char*>(g_decoded),decoded_n,marker,sizeof(marker)-1)){
        closesocket(s); return invalid_socket;
    }
    return s;
}
}

int main() noexcept
{
    HookPrerequisiteModules hook_prerequisites=load_hook_prerequisites();
    if(!hook_prerequisites.advapi) finish(9);
    HMODULE module=LoadLibraryW(L"preloader_l.dll");
    if(!module) finish(10);
    auto* shutdown_proc=GetProcAddress(module,"DllShutdown");
    if(!shutdown_proc){ FreeLibrary(module); finish(14); }
    auto shutdown_runtime=reinterpret_cast<ShutdownFn>(shutdown_proc);
    if(WSAStartup(0x0202u,g_wsa)!=0){
        if(shutdown_runtime(30000u)==0u) (void)FreeLibrary(module);
        finish(11);
    }

    // The Release worker is created from DllMain and starts after loader-lock exit.
    SOCKET connected=invalid_socket;
    for(unsigned attempt=0; attempt<200 && connected==invalid_socket; ++attempt){
        const uint16_t ports[]={3216,3226,3227,3228,3229,3230,3231,3232,3233,3234,3235,3236};
        for(size_t i=0;i<sizeof(ports)/sizeof(ports[0]);++i){
            connected=try_port(ports[i]);
            if(connected!=invalid_socket) break;
        }
        if(connected==invalid_socket) Sleep(10);
    }
    if(connected==invalid_socket){ WSACleanup(); if(shutdown_runtime(30000u)==0u) (void)FreeLibrary(module); finish(12); }

    // Corrected acceptance host closes only after completing the real handshake
    // and encoded GetConfig transaction.
    closesocket(connected);
    WSACleanup();
    Sleep(100);
    // Public shutdown completes before releasing the final module reference.
    const uint32_t shutdown_rc=shutdown_runtime(30000u);
    if(shutdown_rc!=0u) finish(15u+(shutdown_rc&0x0fu));
    if(!FreeLibrary(module)) finish(13);
    unload_hook_prerequisites(&hook_prerequisites);
    finish(0);
}
