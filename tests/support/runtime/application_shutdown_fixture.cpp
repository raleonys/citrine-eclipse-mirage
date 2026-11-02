// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include "preloader_l/runtime/application.hpp"
#include "preloader_l/platform/hook_engine_api.hpp"
#include "preloader_l/platform/hook_unload_guard.hpp"
#include <atomic>
#include <filesystem>

namespace fixture {
std::atomic<unsigned> mode{},ready{},attempts{},created{},destroyed{},disables{},releases{};
std::atomic<unsigned> held_before_loop{},release_before_loop{},fail_next_scan{};
// C08 test-only gate: pause the worker before its next event wait *after*
// one cleanup attempt failed, so the public caller sees the earlier status.
std::atomic<unsigned> hold_after_failure{},entered_wait{},release_wait{};
// No native patching or thread suspension is permitted in this fixture. Only
// the engine and unload-scan boundaries are controlled; owners are real.
int initialize(){return MH_OK;}
int create(void* target,void*,void** original){*original=target;return MH_OK;}
int queue(void*){return MH_OK;}
int apply(){return MH_OK;}
int disable(void*){++disables;return mode==1?MH_ERROR_MEMORY_PROTECT:MH_OK;}
int release(){++releases;return mode==3?MH_ERROR_MEMORY_PROTECT:MH_OK;}
}
namespace preloader_l {bool fixture_threads_outside_module(){
    if(fixture::fail_next_scan.exchange(0,std::memory_order_acq_rel))return false;
    return fixture::mode!=4;
}}
#define MH_Initialize fixture::initialize
#define MH_CreateHook fixture::create
#define MH_EnableHook fixture::queue
#define MH_QueueEnableHook fixture::queue
#define MH_ApplyQueued fixture::apply
#define MH_DisableHook fixture::disable
#define MH_Uninitialize fixture::release
#include "src/platform/hook_engine_api.cpp"
#undef MH_Initialize
#undef MH_CreateHook
#undef MH_EnableHook
#undef MH_QueueEnableHook
#undef MH_ApplyQueued
#undef MH_DisableHook
#undef MH_Uninitialize
#include "src/platform/hook_native_callbacks.cpp"
namespace fixture {std::unique_ptr<preloader_l::native::Lease> held_call;}
#define external_threads_outside_module fixture_threads_outside_module
#include "src/runtime/application.cpp"
#undef external_threads_outside_module

namespace preloader_l::application {
class FixtureApplication {
    struct Directory {
        std::filesystem::path path=std::filesystem::temp_directory_path()/
            ("surge-shutdown-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
        Directory(){std::filesystem::create_directories(path);}
        ~Directory(){std::error_code ec;std::filesystem::remove_all(path,ec);}
        Options options(){Options o;o.executable_path=(path/"game.exe").u16string();o.data_root=(path/"data").u16string();o.program_data=(path/"program").u16string();return o;}
    } directory;
    Application app{directory.options()};
public:
    explicit FixtureApplication(Options={}){++fixture::created;}
    ~FixtureApplication(){++fixture::destroyed;}
    void start(){
        const bool hold=GetEnvironmentVariableA("SURGE_TEST_HOLD_WORKER_BEFORE_STOP",nullptr,0)!=0;
        app.start();fixture::ready=1;
        if(hold){
            fixture::held_before_loop=1;
            while(!fixture::release_before_loop.load(std::memory_order_acquire))Sleep(1);
            fixture::held_before_loop=0;
        }
    }
    void stop(){++fixture::attempts;app.stop();}
};
}
// Exercise the actual entry worker and DllShutdown implementation. The wrapper
// supplies isolated Options and counters, delegating every lifecycle operation.
// Intercept only the fixture's worker event wait, not the public
// DllShutdown worker-thread wait, to expose the precise stale-status window.
DWORD WINAPI FixtureWorkerWait(HANDLE handle,DWORD timeout);
#define WaitForSingleObject FixtureWorkerWait
#define Application FixtureApplication
#include "src/platform/dll_entry.cpp"
#undef Application
#undef WaitForSingleObject
DWORD WINAPI FixtureWorkerWait(HANDLE handle,DWORD timeout){
    if(timeout==INFINITE&&handle==stop_event&&fixture::hold_after_failure.load()&&
       fixture::attempts.load()>=1&&!fixture::entered_wait.exchange(1)){
        // Release automatically even on a broken test to avoid leaving a
        // live fixture thread stuck inside an unloadable DLL indefinitely.
        const auto start=GetTickCount64();
        while(!fixture::release_wait.load(std::memory_order_acquire)&&
              GetTickCount64()-start<6000)Sleep(1);
    }
    return ::WaitForSingleObject(handle,timeout);
}
extern "C" __declspec(dllexport) void FixtureMode(unsigned mode){
    fixture::mode=mode;
    if(mode==5)fixture::fail_next_scan=1;
    if(mode==2)fixture::held_call=std::make_unique<preloader_l::native::Lease>();
    else fixture::held_call.reset();
}
extern "C" __declspec(dllexport) void FixtureReleaseWorker(){fixture::release_before_loop=1;}
extern "C" __declspec(dllexport) void FixtureHoldAfterFailure(){
    fixture::release_wait=0;
    fixture::entered_wait=0;
    fixture::hold_after_failure=1;
}
extern "C" __declspec(dllexport) void FixtureReleaseFailureWait(){
    fixture::release_wait.store(1,std::memory_order_release);
}
extern "C" __declspec(dllexport) unsigned FixtureValue(unsigned key){
    switch(key){case 0:return fixture::ready;case 1:return fixture::attempts;
    case 2:return fixture::created;case 3:return fixture::destroyed;case 4:return fixture::disables;
    case 5:return fixture::releases;case 6:return preloader_l::native::is_bound();
    case 7:return static_cast<unsigned>(preloader_l::native::active_calls());
    case 8:return fixture::held_before_loop.load(std::memory_order_acquire);
    case 9:return fixture::entered_wait.load(std::memory_order_acquire);
    default:return ~0u;}
}
