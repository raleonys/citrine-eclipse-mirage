// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/control.h"
#include "preloader_l/runtime/application.hpp"
#include "preloader_l/platform/hook_unload_guard.hpp"
#include <atomic>
#include <cstdint>
#include <memory>

namespace {
// POD-only loader entry state. No C++ application construction, disk IO,
// sockets, waits, hook patching or provider calls execute under loader lock.
HANDLE stop_event=nullptr,worker_thread=nullptr;
HMODULE image_lease=nullptr;
std::atomic<DWORD> shutdown_result{ERROR_IO_PENDING};
// The manual-reset event is a wake notification, not a request counter.
// Distinct public shutdown calls must not collapse before worker ResetEvent.
std::atomic<std::uint64_t> shutdown_requests{0};
std::atomic_flag shutdown_call=ATOMIC_FLAG_INIT;
DWORD WINAPI application_worker(void*) {
    // The caller retains its LoadLibrary reference until shutdown succeeds.
    // Acquire the extra worker lease outside loader lock. Attach failures can
    // therefore close their handles without leaking a self-reference.
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCWSTR>(&application_worker),&image_lease)) {
        const auto error=GetLastError();shutdown_result=ERROR_SUCCESS;
        return error;
    }
    std::unique_ptr<preloader_l::application::Application> owner;
    try {
        owner=std::make_unique<preloader_l::application::Application>();
        owner->start();
    }catch(...) {
        // Startup failure also requires exactly one cleanup attempt.
        shutdown_requests.fetch_add(1,std::memory_order_release);
        SetEvent(stop_event);
    }
    std::uint64_t completed_requests=0;
    bool stopped=false;
    while(!stopped){
        if(WaitForSingleObject(stop_event,INFINITE)!=WAIT_OBJECT_0){
            shutdown_result=ERROR_BUSY;Sleep(10);continue;
        }
        // Clear the notification, then consume each published request exactly
        // once. A SetEvent arriving before this ResetEvent remains visible via
        // shutdown_requests, including after an unsuccessful cleanup attempt.
        if(!ResetEvent(stop_event)){shutdown_result=ERROR_BUSY;Sleep(10);continue;}
        while(completed_requests!=shutdown_requests.load(std::memory_order_acquire)){
            ++completed_requests;
            shutdown_result=ERROR_IO_PENDING;
            try {
                if(owner)owner->stop();
                owner.reset();
                shutdown_result=ERROR_SUCCESS;
                stopped=true;
                break;
            }catch(const std::exception& error) {
                preloader_l::trace_shutdown_failure(error.what());
                shutdown_result=ERROR_BUSY;
            }catch(...) {
                preloader_l::trace_shutdown_failure("non-standard cleanup exception");
                shutdown_result=ERROR_BUSY;
            }
        }
    }
    auto lease=image_lease;image_lease=nullptr;
    FreeLibraryAndExitThread(lease,0);
}
}
extern "C" DWORD WINAPI DllShutdown(DWORD timeout) {
    if(shutdown_call.test_and_set())return DLL_SHUTDOWN_CONCURRENT;
    preloader_l::set_shutdown_caller(GetCurrentThreadId());
    struct Release {~Release(){
        // A timeout/failure returns control to a live caller. It must become
        // eligible for subsequent unload scans before another shutdown call.
        preloader_l::set_shutdown_caller(0);
        shutdown_call.clear();
    }} release;
    if(!worker_thread||!stop_event)return DLL_SHUTDOWN_WAIT_FAILED;
    // Publish the request before the wake. Even an already-signalled event
    // carries a distinct retry; on signal failure retain the fail-closed intent.
    shutdown_requests.fetch_add(1,std::memory_order_release);
    if(!SetEvent(stop_event))return DLL_SHUTDOWN_WAIT_FAILED;
    const auto result=WaitForSingleObject(worker_thread,timeout);
    if(result==WAIT_TIMEOUT)return shutdown_result.load()==ERROR_BUSY?DLL_SHUTDOWN_CLEANUP_FAILED:DLL_SHUTDOWN_TIMEOUT;
    if(result!=WAIT_OBJECT_0)return DLL_SHUTDOWN_WAIT_FAILED;
    switch(shutdown_result.load()) {
    case ERROR_SUCCESS:return DLL_SHUTDOWN_OK;
    case ERROR_BUSY:return DLL_SHUTDOWN_CLEANUP_FAILED;
    default:return DLL_SHUTDOWN_WAIT_FAILED;
    }
}
extern "C" void SURGE() {}
BOOL WINAPI DllMain(HINSTANCE image,DWORD reason,LPVOID reserved) {
    if(reason==DLL_PROCESS_ATTACH) {
        stop_event=CreateEventW(nullptr,TRUE,FALSE,nullptr);
        if(!stop_event){
            const auto error=GetLastError();
            SetLastError(error);return FALSE;
        }
        worker_thread=CreateThread(nullptr,0,application_worker,nullptr,0,nullptr);
        if(!worker_thread){
            const auto error=GetLastError();
            CloseHandle(stop_event);
            stop_event=nullptr;SetLastError(error);return FALSE;
        }
    }else if(reason==DLL_PROCESS_DETACH) {
        // No join or C++ teardown under loader lock. Normal shutdown joined the
        // worker first; process termination lets Windows reclaim the remaining state.
        if(stop_event)SetEvent(stop_event);
        if(!reserved){if(worker_thread)CloseHandle(worker_thread);if(stop_event)CloseHandle(stop_event);}
    }
    (void)image;return TRUE;
}
