// SPDX-License-Identifier: GPL-3.0-only

#include <asio.hpp>
#include "laureondo/runtime/control.h"
#include "tools/runtime_probe.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace {
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
using Stop=DWORD(WINAPI*)(DWORD);
void scenario(const wchar_t* path,std::wstring_view selected){
    SetEnvironmentVariableA("EALsxPort",nullptr);
    const auto module=LoadLibraryW(path);require(module!=nullptr,"fixture load");
    const auto stop=reinterpret_cast<Stop>(GetProcAddress(module,"DllShutdown"));
    const auto mode=reinterpret_cast<void(*)(unsigned)>(GetProcAddress(module,"FixtureMode"));
    const auto value=reinterpret_cast<unsigned(*)(unsigned)>(GetProcAddress(module,"FixtureValue"));
    require(stop&&mode&&value,"fixture methods");
    asio::io_context io;asio::ip::tcp::socket peer(io);std::string buffered;
    laureondo::tools::wait_for_application(peer,buffered);
    const auto ready_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!value(0)&&std::chrono::steady_clock::now()<ready_deadline)Sleep(1);
    require(value(0)&&value(2)==1,"real Application ready");
    const unsigned fault=selected==L"retry"?1:selected==L"uncertain"?2:3;
    mode(fault);require(stop(1000)==DLL_SHUTDOWN_CLEANUP_FAILED,"native close failure must block DLL cleanup success");
    require(value(3)==0&&value(2)==1,"failed native close retains the sole Application");
    const auto before=value(4);mode(0);
    if(fault==1){
        require(stop(10000)==DLL_SHUTDOWN_OK&&value(3)==1&&value(2)==1,"native would-block retry retires same Application");
        require(value(4)==before+1,"retry closes retained descriptor exactly once");
        require(stop(0)==DLL_SHUTDOWN_OK&&value(4)==before+1,"completed cleanup is idempotent");
        require(FreeLibrary(module)!=FALSE,"caller reference release");
        require(!GetModuleHandleW(std::filesystem::path(path).filename().c_str()),"successful retirement releases image lease");
    }else{
        require(stop(1000)==DLL_SHUTDOWN_CLEANUP_FAILED&&value(4)==before&&value(3)==0,"uncertain result must never blindly retry a numeric descriptor");
        require(FreeLibrary(module)!=FALSE,"caller reference release after observing retained owner");
        require(GetModuleHandleW(std::filesystem::path(path).filename().c_str())==module,"worker image lease must prevent unsafe unload");
        // Process exit reclaims the quarantined owner. No force-unload or
        // production quarantine-clear API is introduced for this fixture.
    }
    std::cout<<"PASS: actual DLL/Application native retirement case "<<fault<<'\n';
}
void child(const wchar_t* fixture,const wchar_t* selected){
    struct Directory {
        std::filesystem::path path;
        explicit Directory(const wchar_t* name):path(std::filesystem::temp_directory_path()/
            (L"surge-socket-suite-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64())+L"-"+name)){
            require(std::filesystem::create_directory(path),"isolated child directory");
        }
        ~Directory(){std::error_code error;std::filesystem::remove_all(path,error);}
    } directory(selected);
    struct Environment {
        std::wstring previous;bool existed=false;
        explicit Environment(const std::filesystem::path& path){
            const auto size=GetEnvironmentVariableW(L"SURGE_TEST_FIXTURE_ROOT",nullptr,0);existed=size!=0;
            if(size){previous.resize(size);previous.resize(GetEnvironmentVariableW(L"SURGE_TEST_FIXTURE_ROOT",previous.data(),size));}
            require(SetEnvironmentVariableW(L"SURGE_TEST_FIXTURE_ROOT",path.c_str())!=FALSE,"fixture child root environment");
        }
        ~Environment(){SetEnvironmentVariableW(L"SURGE_TEST_FIXTURE_ROOT",existed?previous.c_str():nullptr);}
    } environment(directory.path);
    wchar_t executable[32768];const auto size=GetModuleFileNameW(nullptr,executable,32768);require(size&&size<32768,"test executable path");
    std::wstring command=L"\""+std::wstring(executable,size)+L"\" \""+fixture+L"\" "+selected;
    STARTUPINFOW startup{};startup.cb=sizeof startup;PROCESS_INFORMATION process{};
    require(CreateProcessW(nullptr,command.data(),nullptr,nullptr,TRUE,0,nullptr,nullptr,&startup,&process)!=FALSE,"isolated native retirement child");
    const auto waited=WaitForSingleObject(process.hProcess,30000);
    if(waited!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,124);WaitForSingleObject(process.hProcess,5000);}
    DWORD result=124;GetExitCodeProcess(process.hProcess,&result);
    CloseHandle(process.hThread);CloseHandle(process.hProcess);
    require(waited==WAIT_OBJECT_0&&result==0,"isolated native retirement child failed");
}
}
int wmain(int argc,wchar_t** argv)try{
    require(argc==2||argc==3,"fixture DLL path and optional case required");
    if(argc==3){const std::wstring_view selected=argv[2];require(selected==L"retry"||selected==L"uncertain"||selected==L"uncertain-closed","unknown case");scenario(argv[1],selected);}
    else for(const auto* selected:{L"retry",L"uncertain",L"uncertain-closed"})child(argv[1],selected);
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
