#include "preloader_l/runtime/control.h"
#include "../runtime_probe.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
void require_status(DWORD result,const char* message) {
    if(result!=0)throw std::runtime_error(std::string(message)+" (status "+std::to_string(result)+")");
}
using Wait=DWORD(WINAPI*)(DWORD);
using Marker=void(*)();
struct Module {
    HMODULE image=nullptr;
    Wait shutdown=nullptr;
    explicit Module(const wchar_t* path) {
        image=LoadLibraryW(path);
        if(!image)throw std::runtime_error("LoadLibrary failed (Win32 "+std::to_string(GetLastError())+")");
        shutdown=reinterpret_cast<Wait>(GetProcAddress(image,"DllShutdown"));
    }
    ~Module() {
        // A failing shutdown must not turn a test failure into unsafe unloading.
        if(image&&shutdown&&shutdown(20000)==DLL_SHUTDOWN_OK)FreeLibrary(image);
    }
    template<class T>T resolve(const char* name,WORD ordinal) {
        const auto by_name=GetProcAddress(image,name);
        require(by_name!=nullptr,"missing named export");
        require(by_name==GetProcAddress(image,MAKEINTRESOURCEA(ordinal)),"ordinal differs from named export");
        return reinterpret_cast<T>(by_name);
    }
    void unload() {
        require(shutdown!=nullptr,"missing shutdown export");
        require_status(shutdown(20000),"real shutdown did not complete");
        require(FreeLibrary(image)!=FALSE,"FreeLibrary failed");
        image=nullptr;
    }
};
void cycle(const wchar_t* path,unsigned index) {
    const std::wstring supplied(path);
    const auto separator=supplied.find_last_of(L"\\/");
    const auto module_name=supplied.substr(separator==std::wstring::npos?0:separator+1);
    require(!GetModuleHandleW(module_name.c_str()),"previous application image remains loaded");
    // A stale environment value cannot stand in for this image's startup.
    char old_port[32]{};const auto old_size=GetEnvironmentVariableA("EALsxPort",old_port,sizeof old_port);
    require(old_size<sizeof old_port,"unexpected pre-existing EALsxPort length");
    struct RestorePort {const char* value;~RestorePort(){SetEnvironmentVariableA("EALsxPort",value);}} restore{old_size?old_port:nullptr};
    require(SetEnvironmentVariableA("EALsxPort",nullptr)!=FALSE,"cannot reset tool EALsxPort observation");
    Module module(path);
    // Resolve DllMain to verify the loader table, but never manually invoke it.
    module.resolve<FARPROC>("DllMain",1);
    const auto marker=module.resolve<Marker>("SURGE",2);
    const auto shutdown=module.resolve<Wait>("DllShutdown",3);
    for(const auto* removed:{"DllWaitReady","DllServerPort"})
        require(!GetProcAddress(module.image,removed),"removed management export is still public");
    for(WORD ordinal=4;ordinal<=6;++ordinal)
        require(!GetProcAddress(module.image,MAKEINTRESOURCEA(ordinal)),"removed management ordinal is still public");
    marker();
    asio::io_context io;asio::ip::tcp::socket socket(io);
    std::string buffered;
    const auto bound=preloader_l::tools::wait_for_application(socket,buffered);
    socket.non_blocking(false);
    std::string request="<LSX><Request id='public-dll' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>";
    request.push_back('\0');asio::write(socket,asio::buffer(request));socket.non_blocking(true);
    require(preloader_l::tools::frame(socket,buffered).find("GetProfileResponse")!=std::string::npos,"missing real profile response");
    if(index==0) {
        const auto immediate=shutdown(0);
        std::cout<<"zero-timeout shutdown status: "<<immediate<<'\n';
        require(immediate==DLL_SHUTDOWN_OK||immediate==DLL_SHUTDOWN_TIMEOUT,
                "unexpected immediate shutdown status");
        // Keep the caller lease and all function pointers on a timeout.
        require_status(shutdown(20000),"shutdown retry failed");
    }else require_status(shutdown(20000),"shutdown failed");
    asio::ip::tcp::socket stopped(io);asio::error_code stopped_error;
    stopped.connect({asio::ip::address_v4::loopback(),bound},stopped_error);
    require(bool(stopped_error),"stopped service still accepts connections");
    require(shutdown(0)==DLL_SHUTDOWN_OK,"completed shutdown is not idempotent");
    asio::error_code ignored;socket.close(ignored);
    module.unload();
    require(!GetModuleHandleW(module_name.c_str()),"worker lease prevented clean unload");
}
}
int wmain(int argc,wchar_t** argv) {
    try {
        require(argc==2,"usage: preloader_l_runtime_host <application DLL>");
        for(unsigned i=0;i<3;++i)cycle(argv[1],i);
        std::cout<<"PASS: public C API names/ordinals; 3 actual load/start/socket/profile/stop/unload cycles; single DllShutdown with bounded wait/retry; removed alias/ordinal absent\n";
        return 0;
    }catch(const std::exception& error) {
        std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;
    }
}
