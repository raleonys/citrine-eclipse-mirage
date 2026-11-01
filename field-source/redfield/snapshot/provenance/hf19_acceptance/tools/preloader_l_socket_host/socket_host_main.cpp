#if !defined(_WIN32)
#error preloader_l_socket_host is Windows-only.
#endif
#include <stddef.h>
#include <stdint.h>

extern "C" {
using BOOL=int; using HMODULE=void*; using FARPROC=void(*)(); using SOCKET=uintptr_t;
__declspec(dllimport) HMODULE __stdcall LoadLibraryW(const wchar_t*);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE,const char*);
__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE);
__declspec(dllimport) __declspec(noreturn) void __stdcall ExitProcess(unsigned int);
__declspec(dllimport) int __stdcall WSAStartup(uint16_t,void*);
__declspec(dllimport) int __stdcall WSACleanup();
__declspec(dllimport) SOCKET __stdcall socket(int,int,int);
struct SockAddr { uint16_t family; char data[14]; };
__declspec(dllimport) int __stdcall connect(SOCKET,const SockAddr*,int);
__declspec(dllimport) int __stdcall send(SOCKET,const char*,int,int);
__declspec(dllimport) int __stdcall recv(SOCKET,char*,int,int);
__declspec(dllimport) int __stdcall closesocket(SOCKET);
}
struct SockAddrIn { uint16_t family; uint16_t port; uint32_t address; uint8_t zero[8]; };
static_assert(sizeof(SockAddrIn)==16,"sockaddr_in");
constexpr SOCKET invalid_socket=static_cast<SOCKET>(~static_cast<SOCKET>(0));
constexpr uint16_t swap16(uint16_t v) noexcept { return static_cast<uint16_t>((v<<8)|(v>>8)); }
using U32=uint32_t; using USize=size_t;
using RuntimeFn=U32(*)(); using StartFn=U32(*)(uint16_t,uint16_t*); using AcceptFn=U32(*)(U32);
using PumpFn=U32(*)(char*,USize,USize*); using StopFn=U32(*)();
namespace { char g_response[4096]; char g_client_response[4096]; alignas(void*) uint8_t g_wsa[408];
[[noreturn]] void finish(unsigned int c) noexcept { ExitProcess(c); } }
int main() noexcept {
    HMODULE m=LoadLibraryW(L"preloader_l_debug.dll"); if(!m) finish(10);
#define R(t,n) reinterpret_cast<t>(GetProcAddress(m,n))
    auto runtime_init=R(RuntimeFn,"preloader_l_debug_runtime_init"); auto runtime_shutdown=R(RuntimeFn,"preloader_l_debug_runtime_shutdown");
    auto start=R(StartFn,"preloader_l_debug_socket_start"); auto accept_client=R(AcceptFn,"preloader_l_debug_socket_accept");
    auto pump=R(PumpFn,"preloader_l_debug_socket_pump"); auto stop=R(StopFn,"preloader_l_debug_socket_stop");
#undef R
    if(!runtime_init||!runtime_shutdown||!start||!accept_client||!pump||!stop){FreeLibrary(m);finish(11);}
    if(runtime_init()!=0u){FreeLibrary(m);finish(12);} uint16_t port=0; if(start(0,&port)!=0u||port==0){FreeLibrary(m);finish(13);}
    if(WSAStartup(0x0202u,g_wsa)!=0){stop();FreeLibrary(m);finish(14);} SOCKET s=socket(2,1,6); if(s==invalid_socket){WSACleanup();stop();FreeLibrary(m);finish(15);}
    SockAddrIn addr; addr.family=2; addr.port=swap16(port); addr.address=0x0100007Fu;
    if(connect(s,reinterpret_cast<const SockAddr*>(&addr),sizeof(addr))!=0){closesocket(s);WSACleanup();stop();FreeLibrary(m);finish(16);}
    if(accept_client(50u)!=0u){closesocket(s);WSACleanup();stop();FreeLibrary(m);finish(17);}
    static const char request[]="<LSX><Request id=\"50\"><GetConfig/></Request></LSX>";
    if(send(s,request,static_cast<int>(sizeof(request)-1),0)!=static_cast<int>(sizeof(request)-1)){closesocket(s);WSACleanup();stop();FreeLibrary(m);finish(18);}
    USize written=0; if(pump(g_response,sizeof(g_response),&written)!=0u||written==0){closesocket(s);WSACleanup();stop();FreeLibrary(m);finish(19);}
    int got=recv(s,g_client_response,sizeof(g_client_response),0); if(got<=0||g_client_response[0]!='<'){closesocket(s);WSACleanup();stop();FreeLibrary(m);finish(20);}
    closesocket(s); WSACleanup(); if(stop()!=0u){FreeLibrary(m);finish(21);} if(runtime_shutdown()!=0u){FreeLibrary(m);finish(22);} if(!FreeLibrary(m))finish(23); finish(0);
}
