// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/control.h"
#include "../runtime_probe.hpp"
#include <chrono>
#include <iphlpapi.h>
#include <vector>
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
// Snapshot the listener owner for attribution only; no failure is suppressed.
std::string ipv4_listener_owners(unsigned short port) {
    const HMODULE module=LoadLibraryExW(L"iphlpapi.dll",nullptr,LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return "iphlpapi_load_failed="+std::to_string(GetLastError());
    const auto query=reinterpret_cast<decltype(&::GetExtendedTcpTable)>(
        GetProcAddress(module,"GetExtendedTcpTable"));
    std::string result;
    if(!query) {
        result="owner_api_missing="+std::to_string(GetLastError());
    }else{
        ULONG bytes=0;
        const DWORD probe=query(nullptr,&bytes,FALSE,AF_INET,TCP_TABLE_OWNER_PID_LISTENER,0);
        if(probe!=ERROR_INSUFFICIENT_BUFFER||bytes<sizeof(DWORD)){
            result="owner_size_probe="+std::to_string(probe);
        }else{
            std::vector<unsigned char> storage(bytes);
            const DWORD status=query(storage.data(),&bytes,FALSE,AF_INET,
                                     TCP_TABLE_OWNER_PID_LISTENER,0);
            if(status!=NO_ERROR||bytes<sizeof(DWORD)){
                result="owner_query="+std::to_string(status);
            }else{
                const auto* table=reinterpret_cast<const MIB_TCPTABLE_OWNER_PID*>(storage.data());
                const auto capacity=(bytes-sizeof(DWORD))/sizeof(MIB_TCPROW_OWNER_PID);
                result="owner_pids=";
                bool found=false;
                for(DWORD n=0;n<table->dwNumEntries&&n<capacity;++n) {
                    const auto& row=table->table[n];
                    if(row.dwState!=MIB_TCP_STATE_LISTEN||
                       ntohs(static_cast<u_short>(row.dwLocalPort))!=port)continue;
                    if(found)result+=",";
                    result+=std::to_string(row.dwOwningPid);
                    found=true;
                }
                if(!found)result+="none";
            }
        }
    }
    FreeLibrary(module);
    return result;
}
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
    const auto listener_before=ipv4_listener_owners(static_cast<unsigned short>(bound));
    auto complete_shutdown=[&](const char* failure) {
        const DWORD result=shutdown(20000);
        if(result!=DLL_SHUTDOWN_OK)
            std::cerr<<"SHUTDOWN_LISTENER_OWNER_DIAGNOSTIC status="<<result
                     <<" port="<<bound<<" caller_pid="<<GetCurrentProcessId()
                     <<" before={"<<listener_before<<"}"
                     <<" after={"<<ipv4_listener_owners(static_cast<unsigned short>(bound))<<"}\n";
        require_status(result,failure);
    };
    if(index==0) {
        const auto immediate=shutdown(0);
        std::cout<<"zero-timeout shutdown status: "<<immediate<<'\n';
        require(immediate==DLL_SHUTDOWN_OK||immediate==DLL_SHUTDOWN_TIMEOUT,
                "unexpected immediate shutdown status");
        // Keep the caller lease and all function pointers on a timeout.
        complete_shutdown("shutdown retry failed");
    }else complete_shutdown("shutdown failed");
    asio::ip::tcp::socket stopped(io);asio::error_code stopped_error;
    stopped.connect({asio::ip::address_v4::loopback(),bound},stopped_error);
    if(!stopped_error)
        std::cerr<<"SHUTDOWN_LISTENER_OWNER_DIAGNOSTIC unexpected_connect"
                 <<" port="<<bound<<" caller_pid="<<GetCurrentProcessId()
                 <<" before={"<<listener_before<<"}"
                 <<" after={"<<ipv4_listener_owners(static_cast<unsigned short>(bound))<<"}\n";
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
