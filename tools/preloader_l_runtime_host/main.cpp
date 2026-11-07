// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/control.h"
#include "../runtime_probe.hpp"
#include "../../tests/fixtures/xml_reader/wire_contracts.hpp"
#include <chrono>
#include "listener_owners.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace {
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
void require_status(DWORD result,const char* message) {
    if(result!=0)throw std::runtime_error(std::string(message)+" (status "+std::to_string(result)+")");
}
using Wait=DWORD(WINAPI*)(DWORD);
using Marker=void(*)();
namespace listeners=preloader_l::tools::listener_diagnostics;
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
std::string exchange(asio::ip::tcp::socket& socket,std::string& buffered,std::string request) {
    request.push_back('\0');socket.non_blocking(false);
    asio::write(socket,asio::buffer(request));socket.non_blocking(true);
    return preloader_l::tools::frame(socket,buffered);
}
void xml_contracts(asio::ip::tcp::socket& socket,std::string& buffered,
                   const std::string& canonical_response) {
    // These transformations must preserve the complete real DLL response bytes.
    // The existing profile response contains no timestamp/random challenge field.
    using namespace xml_contract_fixture;
    const auto canonical=profile_request();
    require(canonical_response.find("id=\"public-dll\"")!=std::string::npos&&
            canonical_response.find("UserIndex=\"0\"")!=std::string::npos,
            "real DLL canonical response omitted expected id/index");
    for(const auto& [name,request]:equivalent_forms())
        if(exchange(socket,buffered,request)!=canonical_response)
            throw std::runtime_error("Real DLL XML equivalence failed: "+name);
    for(const auto& value:equivalent_values()) {
        const auto expected=exchange(socket,buffered,profile_request(value.literal));
        // Literal fixtures already contain the exact expected XML escaping.
        require(expected.find("UserIndex=\""+value.literal+"\"")!=std::string::npos,
                "real DLL did not return the explicit expected attribute value");
        require(exchange(socket,buffered,profile_request(value.reference))==expected,
                "real DLL changed decoded/literal/Unicode attribute semantics");
    }
    const auto next=exchange(socket,buffered,profile_request("A","EbisuSDK","wire-next"));
    require(next.find("id=\"wire-next\"")!=std::string::npos&&next.find("UserIndex=\"A\"")!=std::string::npos,
            "real DLL returned stale request id/value");
    for(const auto& invalid:malformed_frames()) {
        // No reply for malformed XML; the same session must process the next
        // valid frame in FIFO order. A bogus reply has a distinct request id.
        auto pipelined=invalid; pipelined.push_back('\0');pipelined+=canonical;
        require(exchange(socket,buffered,std::move(pipelined))==canonical_response,
                "real DLL malformed-frame rejection/recovery changed");
    }
    std::cout<<"PASS: real DLL XML contracts: 8 equivalent forms, 5 value pairs, fresh id/value, 3 malformed/recovery pairs\n";
}
void cycle(const wchar_t* path,unsigned index,bool validate_xml) {
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
    const auto listener_before=listeners::capture(static_cast<unsigned short>(bound));
    socket.non_blocking(false);
    std::string request="<LSX><Request id='public-dll' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>";
    request.push_back('\0');asio::write(socket,asio::buffer(request));socket.non_blocking(true);
    const auto response=preloader_l::tools::frame(socket,buffered);
    require(response.find("GetProfileResponse")!=std::string::npos,"missing real profile response");
    if(validate_xml)xml_contracts(socket,buffered,response);
    auto complete_shutdown=[&](const char* failure) {
        const DWORD result=shutdown(20000);
        if(result!=DLL_SHUTDOWN_OK)
            listeners::report(std::cerr,"shutdown_failed",result,bound,GetCurrentProcessId(),
                              listener_before,listeners::capture(bound));
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
        listeners::report(std::cerr,"unexpected_connect",DLL_SHUTDOWN_OK,bound,GetCurrentProcessId(),
                          listener_before,listeners::capture(bound));
    require(bool(stopped_error),"stopped service still accepts connections");
    require(shutdown(0)==DLL_SHUTDOWN_OK,"completed shutdown is not idempotent");
    asio::error_code ignored;socket.close(ignored);
    module.unload();
    require(!GetModuleHandleW(module_name.c_str()),"worker lease prevented clean unload");
}
}
int wmain(int argc,wchar_t** argv) {
    try {
        require(argc==2||(argc==3&&std::wstring_view(argv[2])==L"--xml-contracts"),
                "usage: preloader_l_runtime_host <application DLL> [--xml-contracts]");
        for(unsigned i=0;i<3;++i)cycle(argv[1],i,argc==3);
        std::cout<<"PASS: public C API names/ordinals; 3 actual load/start/socket/profile/stop/unload cycles; single DllShutdown with bounded wait/retry; removed alias/ordinal absent\n";
        return 0;
    }catch(const std::exception& error) {
        std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;
    }
}
