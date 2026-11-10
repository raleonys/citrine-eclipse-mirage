// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/control.h"
#include <iostream>
#include <stdexcept>

namespace attach_fixture {
enum class Failure {event,thread};
struct Trace {
    Failure failure=Failure::event;
    unsigned events=0,threads=0,closes=0;
    HANDLE acquired=nullptr,closed=nullptr;
    BOOL close_result=FALSE;
} trace;
constexpr DWORD event_error=ERROR_NOT_ENOUGH_MEMORY;
constexpr DWORD thread_error=ERROR_MAX_THRDS_REACHED;
constexpr DWORD cleanup_error=ERROR_CANCELLED;
HANDLE WINAPI create_event(LPSECURITY_ATTRIBUTES security,BOOL manual,BOOL initial,LPCWSTR name){
    ++trace.events;
    if(trace.failure==Failure::event){SetLastError(event_error);return nullptr;}
    trace.acquired=::CreateEventW(security,manual,initial,name);
    return trace.acquired;
}
HANDLE WINAPI create_thread(LPSECURITY_ATTRIBUTES,SIZE_T,LPTHREAD_START_ROUTINE,LPVOID,DWORD,LPDWORD){
    ++trace.threads;
    // Never invoke the supplied worker or start an Application in this fixture.
    SetLastError(thread_error);return nullptr;
}
BOOL WINAPI close_handle(HANDLE handle){
    ++trace.closes;trace.closed=handle;trace.close_result=::CloseHandle(handle);
    // Make saved-error restoration observable even when native close succeeds.
    SetLastError(cleanup_error);return trace.close_result;
}
}
#define CreateEventW attach_fixture::create_event
#define CreateThread attach_fixture::create_thread
#define CloseHandle attach_fixture::close_handle
#include "src/platform/dll_entry.cpp"
#undef CloseHandle
#undef CreateThread
#undef CreateEventW

namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
void failed_attach(attach_fixture::Failure failure){
    using namespace attach_fixture;
    require(!stop_event&&!worker_thread&&!image_lease,"failure case starts without owned entry resources");
    trace={};trace.failure=failure;
    struct Cleanup {
        ~Cleanup(){
            DWORD flags=0;
            if(trace.acquired&&::GetHandleInformation(trace.acquired,&flags))
                ::CloseHandle(trace.acquired); // Also clean up a failing regression.
        }
    } cleanup;
    const auto image=GetModuleHandleW(nullptr);
    require(image!=nullptr,"own fixture image available");
    SetLastError(ERROR_SUCCESS);
    const auto result=DllMain(image,DLL_PROCESS_ATTACH,nullptr);
    const auto error=GetLastError(); // Capture before observations can alter it.
    require(result==FALSE,"selected SDK creation failure rejects attach");
    require(trace.events==1,"attach attempts event creation once");
    if(failure==Failure::event){
        require(error==event_error,"event creation error preserved");
        require(!trace.threads&&!trace.closes&&!trace.acquired,"event failure cannot create a worker or close an unowned handle");
    }else{
        require(trace.acquired!=nullptr,"thread failure case acquired a real private event");
        require(trace.threads==1,"thread creation attempted once without executing its callback");
        require(trace.closes==1&&trace.closed==trace.acquired&&trace.close_result,
            "thread creation failure closes exactly the acquired event once");
        require(error==thread_error&&error!=cleanup_error,"thread error survives cleanup clobber");
        DWORD flags=0;
        require(!::GetHandleInformation(trace.acquired,&flags)&&GetLastError()==ERROR_INVALID_HANDLE,
            "failed attach physically released the kernel event");
    }
    require(!stop_event&&!worker_thread&&!image_lease,"failed attach leaves no private handle or image lease");
}
}
int main()try{
    failed_attach(attach_fixture::Failure::event);
    failed_attach(attach_fixture::Failure::thread);
    std::cout<<"PASS: "<<checks<<" actual DllMain attach failure checks; real event cleanup, controlled creation failures, no worker execution\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
