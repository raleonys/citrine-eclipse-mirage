// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <windows.h>
#include <atomic>
namespace fixture {
std::atomic<unsigned> mode{},ready{},attempts{},created{},destroyed{},close_calls{};
std::atomic<SOCKET> target{INVALID_SOCKET};
}
int WSAAPI FixtureCloseSocket(SOCKET socket){
    if(fixture::mode&&fixture::target==INVALID_SOCKET){
        sockaddr_storage address{};int size=sizeof address;
        if(::getpeername(socket,reinterpret_cast<sockaddr*>(&address),&size)==0)fixture::target=socket;
    }
    if(socket==fixture::target){
        ++fixture::close_calls;
        const auto mode=fixture::mode.load();
        if(mode){
            if(mode==3)::closesocket(socket); // Controlled ambiguous outcome.
            WSASetLastError(mode==1?WSAEWOULDBLOCK:WSAENETDOWN);return SOCKET_ERROR;
        }
    }
    return ::closesocket(socket);
}
#define closesocket FixtureCloseSocket
#include "src/net/asio_service.cpp"
#undef closesocket
#include "laureondo/runtime/application.hpp"
#include <filesystem>
namespace laureondo::application {
class FixtureApplication {
    struct Directory {
        static std::filesystem::path location(){
            wchar_t root[32768];const auto size=GetEnvironmentVariableW(L"SURGE_TEST_FIXTURE_ROOT",root,32768);
            if(size>=32768)throw std::runtime_error("fixture root too long");
            if(size)return std::filesystem::path(root)/"application";
            return std::filesystem::temp_directory_path()/("surge-socket-dll-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
        }
        std::filesystem::path path=location();
        Directory(){if(!std::filesystem::create_directory(path))throw std::runtime_error("fixture directory already exists");}
        ~Directory(){std::error_code error;std::filesystem::remove_all(path,error);}
        Options options(){Options o;o.executable_path=(path/"game.exe").u16string();o.data_root=(path/"data").u16string();o.program_data=(path/"program").u16string();o.install_hooks=false;return o;}
    } directory;
    Application app{directory.options()};
public:
    explicit FixtureApplication(Options={}){++fixture::created;}
    ~FixtureApplication(){++fixture::destroyed;}
    void start(){app.start();fixture::ready=1;}
    void stop(){++fixture::attempts;app.stop();}
};
}
#define Application FixtureApplication
#include "src/platform/dll_entry.cpp"
#undef Application
extern "C" __declspec(dllexport) void FixtureMode(unsigned value){fixture::mode=value;}
extern "C" __declspec(dllexport) unsigned FixtureValue(unsigned key){
    switch(key){case 0:return fixture::ready;case 1:return fixture::attempts;case 2:return fixture::created;
    case 3:return fixture::destroyed;case 4:return fixture::close_calls;default:return ~0u;}
}
