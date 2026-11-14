// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include "laureondo/runtime/application.hpp"
#include "laureondo/platform/hook_engine_api.hpp"
#include "laureondo/platform/hook_unload_guard.hpp"
#include "src/runtime/application_internal.hpp"
#include "laureondo/codec/crypto.hpp"
#include "laureondo/core/protocol_identity.hpp"
#include "laureondo/net/asio_service.hpp"
#include "laureondo/net/session_handler.hpp"
#include "laureondo/net/session_routing.hpp"
#include "laureondo/runtime/startup.hpp"
#include "laureondo/runtime/libraries.hpp"
#include "laureondo/platform/callback_lock.hpp"
#include "laureondo/platform/hook_transaction.hpp"
#include <algorithm>
#include <MinHook.h>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <future>
#include <exception>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>
#include <stdexcept>

namespace fixture {
std::atomic<unsigned> mode{},ready{},attempts{},created{},destroyed{},disables{},releases{};
std::atomic<unsigned> held_before_loop{},release_before_loop{},fail_next_scan{};
// C08 test-only gate: pause the worker before its next event wait *after*
// one cleanup attempt failed, so the public caller sees the earlier status.
std::atomic<unsigned> hold_after_failure{},entered_wait{},release_wait{};
std::atomic<unsigned> startup_phase{},startup_held{},startup_release{},startup_visits{};
std::atomic<unsigned> startup_returns{},startup_failures{},startup_throw{},startup_thread{},paused_thread{},startup_aborting{};
// No native patching or thread suspension is permitted in this fixture. Engine,
// unload-scan and test startup barriers are controlled; owners are real.
std::atomic<unsigned> file_recovery_visits{},fail_after_file_recovery{};
std::atomic<unsigned> requested_file_recovery_visits{};
int initialize(){return MH_OK;}
int create(void* target,void*,void** original){*original=target;return MH_OK;}
int queue(void*){return MH_OK;}
int apply(){return MH_OK;}
int disable(void*){++disables;return mode==1?MH_ERROR_MEMORY_PROTECT:MH_OK;}
int release(){++releases;return mode==3?MH_ERROR_MEMORY_PROTECT:MH_OK;}
}
namespace laureondo {bool fixture_threads_outside_module(){
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
namespace fixture {std::unique_ptr<laureondo::native::Lease> held_call;}
// Observe the real startup calls before readiness publication. All application
// headers above are included before this local type substitution. Asio retains
// its ordinary Execution base reference; no shipping class or DLL is changed.
namespace laureondo::execution {
class StartupExecution:public Execution {
    FlagReferences flags_;
public:
    StartupExecution(MutexState& mutex,FlagReferences flags,std::int64_t& frequency,Platform& platform)
        :Execution(mutex,flags,frequency,platform),flags_(flags){}
    void finish_flag_036820(std::uint8_t& flag){
        // Asio publishes server readiness through its compiled base reference.
        // The included Application emits this first flag only to unblock abort
        // cleanup. Never mistake that cleanup path for the startup barrier.
        if(&flag==&flags_.first_0a3c3d)fixture::startup_aborting=1;
        const unsigned phase=&flag==&flags_.second_0a3c3c?1u:
            (&flag==&flags_.third_0a3c3e?2u:0u);
        if(phase&&phase==fixture::startup_phase.load()&&!fixture::startup_aborting.load()
            &&!fixture::startup_visits.exchange(1)){
            const auto error=GetLastError();
            fixture::paused_thread=GetCurrentThreadId();
            fixture::startup_held.store(phase,std::memory_order_release);
            while(!fixture::startup_release.load(std::memory_order_acquire))Sleep(1);
            fixture::startup_held=0;
            SetLastError(error);
            if(fixture::startup_throw.load())throw std::runtime_error("controlled startup publication failure");
        }
        Execution::finish_flag_036820(flag);
    }
};
}
#define Execution StartupExecution
#define external_threads_outside_module fixture_threads_outside_module
namespace fixture {
void after_file_recovery() {
    file_recovery_visits.fetch_add(1,std::memory_order_acq_rel);
    if(fail_after_file_recovery.exchange(0,std::memory_order_acq_rel))
        throw std::runtime_error("test-only fault after real NativeFactory recovery");
}
}
// Only in the source-included Application. Calls the real recovery first.
// This is not an injected native atomic-rollback failure.
#define recover_atomic() recover_atomic(), ::fixture::after_file_recovery()
#include "src/runtime/application.cpp"
#undef recover_atomic
#undef external_threads_outside_module
#undef Execution

namespace laureondo::application {
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
        char selection[32]{};
        const auto count=GetEnvironmentVariableA("SURGE_TEST_PAUSE_STARTUP",selection,sizeof selection);
        if(count){
            if(count>=sizeof selection)throw std::invalid_argument("invalid startup fixture phase");
            const std::string_view phase(selection,count);
            if(phase=="provider"||phase=="provider-fail")fixture::startup_phase=1;
            else if(phase=="ready"||phase=="ready-fail")fixture::startup_phase=2;
            else throw std::invalid_argument("invalid startup fixture phase");
            fixture::startup_throw=phase.ends_with("-fail");
        }
        fixture::startup_thread=GetCurrentThreadId();
        const bool hold=GetEnvironmentVariableA("SURGE_TEST_HOLD_WORKER_BEFORE_STOP",nullptr,0)!=0;
        try{app.start();}
        catch(...){++fixture::startup_failures;throw;}
        ++fixture::startup_returns;fixture::ready=1;
        if(hold){
            fixture::held_before_loop=1;
            while(!fixture::release_before_loop.load(std::memory_order_acquire))Sleep(1);
            fixture::held_before_loop=0;
        }
    }
    void stop(){
        // Count only recovery calls made during this requested stop. Member
        // destruction performs a separate idempotent Impl::stop afterward.
        struct Visits {
            unsigned before=fixture::file_recovery_visits.load(std::memory_order_acquire);
            ~Visits(){fixture::requested_file_recovery_visits.fetch_add(
                fixture::file_recovery_visits.load(std::memory_order_acquire)-before,
                std::memory_order_acq_rel);}
        } visits;
        ++fixture::attempts;app.stop();
    }
};
}
// Exercise the actual entry worker and DllShutdown implementation. The wrapper
// supplies isolated Options and counters, delegating every lifecycle operation.
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
    if(mode==6)fixture::fail_after_file_recovery.store(1,std::memory_order_release);
    if(mode==2)fixture::held_call=std::make_unique<laureondo::native::Lease>();
    else fixture::held_call.reset();
}
extern "C" __declspec(dllexport) void FixtureReleaseWorker(){fixture::release_before_loop=1;}
extern "C" __declspec(dllexport) void FixtureReleaseStartup(){fixture::startup_release=1;}
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
    case 5:return fixture::releases;case 6:return laureondo::native::is_bound();
    case 7:return static_cast<unsigned>(laureondo::native::active_calls());
    case 8:return fixture::held_before_loop.load(std::memory_order_acquire);
    case 9:return fixture::file_recovery_visits.load(std::memory_order_acquire);
    case 10:return fixture::requested_file_recovery_visits.load(std::memory_order_acquire);
    case 11:return fixture::startup_held.load(std::memory_order_acquire);
    case 12:return fixture::startup_visits;
    case 13:return fixture::startup_returns;
    case 14:return fixture::startup_failures;
    case 15:return static_cast<unsigned>(shutdown_requests.load(std::memory_order_acquire));
    case 16:return WaitForSingleObject(worker_thread,0);
    case 17:return fixture::startup_thread;
    case 18:return fixture::paused_thread;
    case 19:return fixture::startup_aborting;
    case 20:return fixture::entered_wait.load(std::memory_order_acquire);
    default:return ~0u;}
}
