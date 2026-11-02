// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/control.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
struct Module {
    HMODULE module=nullptr;
    DWORD (WINAPI* stop)(DWORD)=nullptr;
    void (*mode)(unsigned)=nullptr;
    unsigned (*value)(unsigned)=nullptr;
    void (*resume)()=nullptr;
    void (*hold_after_failure)()=nullptr;
    void (*release_failure_wait)()=nullptr;
    explicit Module(const char* path){
        module=LoadLibraryA(path);require(module!=nullptr,"fixture DLL load");
        stop=reinterpret_cast<decltype(stop)>(GetProcAddress(module,"DllShutdown"));
        mode=reinterpret_cast<decltype(mode)>(GetProcAddress(module,"FixtureMode"));
        value=reinterpret_cast<decltype(value)>(GetProcAddress(module,"FixtureValue"));
        resume=reinterpret_cast<decltype(resume)>(GetProcAddress(module,"FixtureReleaseWorker"));
        hold_after_failure=reinterpret_cast<decltype(hold_after_failure)>(GetProcAddress(module,"FixtureHoldAfterFailure"));
        release_failure_wait=reinterpret_cast<decltype(release_failure_wait)>(GetProcAddress(module,"FixtureReleaseFailureWait"));
        require(stop&&mode&&value&&resume&&hold_after_failure&&release_failure_wait,
                "fixture methods");
    }
    ~Module(){
        if(release_failure_wait)release_failure_wait();
        if(mode&&stop){mode(0);if(stop(10000)==DLL_SHUTDOWN_OK)FreeLibrary(module);}
    }
};
void scenario(const char* path,unsigned fault){
    Module m(path);
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!m.value(0)&&std::chrono::steady_clock::now()<until)Sleep(1);
    require(m.value(0)&&m.value(6),"real application and hook binding ready");
    m.mode(fault);
    const auto result=m.stop(1000);
    require(result==DLL_SHUTDOWN_CLEANUP_FAILED,"incomplete hook cleanup must return failure for an explicit retry");
    require(m.value(1)==1&&m.value(2)==1&&m.value(3)==0&&m.value(6),"failed cleanup retains sole application and binding");
    require(m.value(4)==1,"single shutdown request must not busy-retry hook disable");
    require(m.value(7)==(fault==2?1u:0u),"real in-flight lease survives failed cleanup");
    m.mode(0);require(m.stop(10000)==DLL_SHUTDOWN_OK,"same application cleanup retry");
    require(m.value(1)==2&&m.value(2)==1&&m.value(3)==1&&!m.value(6),"retry destroys sole owner only after binding retirement");
    require(m.value(4)==(fault==1?2u:1u),"successful disable must not repeat during retry");
    require(m.value(5)==(fault==3?2u:1u),"retry releases only unfinished engine ownership");
    require(m.stop(0)==DLL_SHUTDOWN_OK,"completed shutdown is idempotent");
    std::cout<<"PASS: actual entry/Application/hook cleanup fault "<<fault<<'\n';
}
}
void queued_shutdown_requests(const char* path,bool persistent=false){
    // Keep the worker before its first event wait. Both public calls must be
    // enqueued before the first ResetEvent, not raced by test scheduling.
    const char* env="SURGE_TEST_HOLD_WORKER_BEFORE_STOP";
    require(SetEnvironmentVariableA(env,"1")!=FALSE,"set worker pause fixture");
    struct Restore {const char* name;~Restore(){SetEnvironmentVariableA(name,nullptr);}} restore{env};
    Module m(path);
    const auto ready_until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while((!m.value(0)||!m.value(8))&&std::chrono::steady_clock::now()<ready_until)Sleep(1);
    require(m.value(0)&&m.value(8)&&m.value(1)==0,
        "actual worker must be live, ready and held before both requests");
    // One deliberate scan failure gives the second caller's queued request
    // a distinct, observable successful retry. No third call may mask loss.
    m.mode(persistent?4:5);
    const auto first=m.stop(0),second=m.stop(0);
    m.resume();
    require(first==DLL_SHUTDOWN_TIMEOUT&&second==DLL_SHUTDOWN_TIMEOUT,
        "two zero-wait caller requests must be accepted before worker cleanup");
    if(persistent){
        const auto attempts_until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
        while(m.value(1)<2&&std::chrono::steady_clock::now()<attempts_until)Sleep(1);
        Sleep(100);
        require(m.value(1)==2&&m.value(2)==1&&m.value(3)==0&&m.value(6),
            "two failed requests must retain the owner without an unsolicited third attempt");
        m.mode(0);
        require(m.stop(10000)==DLL_SHUTDOWN_OK,"third explicit request retries retained cleanup");
        require(m.value(1)==3&&m.value(3)==1&&m.value(4)==1&&m.value(5)==1,
            "third request destroys the sole owner without duplicate hook disable");
        std::cout<<"PASS: two queued failures wait for a third explicit shutdown request\n";
        return;
    }
    const auto done_until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(!m.value(3)&&std::chrono::steady_clock::now()<done_until)Sleep(1);
    require(m.value(1)==2&&m.value(2)==1&&m.value(3)==1,
        "coalesced event must not discard second shutdown attempt after first fails");
    require(m.value(4)==1&&m.value(5)==1,
        "queued retry retains sole engine owner and avoids duplicate disable");
    require(m.stop(10000)==DLL_SHUTDOWN_OK,
        "completed queued shutdown must retain the public success contract");
    std::cout<<"PASS: two queued public shutdown requests preserve exactly two cleanup attempts\n";
}
void stale_cleanup_status_window(const char* path){
    // C08-CAND-001: the previous cleanup failure can still be visible to
    // the next caller while its newly enqueued attempt has not executed.
    Module m(path);
    const auto ready_until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!m.value(0)&&std::chrono::steady_clock::now()<ready_until)Sleep(1);
    require(m.value(0)&&m.value(6)&&m.value(1)==0,
            "live application/worker required before testing shutdown status");
    m.hold_after_failure();
    m.mode(4); // Persistently fail the owned native unload scan.
    const auto first=m.stop(1000);
    require(first==DLL_SHUTDOWN_CLEANUP_FAILED&&m.value(1)==1&&m.value(3)==0,
            "first request must finish with a retained owner and ERROR_BUSY");
    const auto hold_until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(!m.value(9)&&std::chrono::steady_clock::now()<hold_until)Sleep(1);
    require(m.value(9)==1&&m.value(1)==1&&m.value(6),
            "worker must be paused before handling the second public request");
    const auto second=m.stop(0);
    require(second==DLL_SHUTDOWN_CLEANUP_FAILED&&m.value(1)==1,
            "zero-wait second request can report prior ERROR_BUSY before its own stop");
    // The observation is classification of an incomplete request, not a
    // spurious OK, lost retry, premature image unload or new bug verdict.
    m.mode(0);
    m.release_failure_wait();
    const auto done_until=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(m.value(3)!=1&&std::chrono::steady_clock::now()<done_until)Sleep(1);
    require(m.value(1)==2&&m.value(2)==1&&m.value(3)==1&&!m.value(6),
            "released worker must process exactly the queued retry and retire once");
    require(m.stop(10000)==DLL_SHUTDOWN_OK&&m.value(1)==2,
            "completed worker retains idempotent public shutdown success");
    std::cout<<"PASS: C08 prior cleanup status is observable before queued retry begins\n";
}
int main(int argc,char** argv)try{
    require(argc==2||argc==3,"fixture DLL path and optional mode required");
    bool ran=false;
    for(unsigned fault=1;fault<=4;++fault)
        if(argc==2||std::string_view(argv[2])==std::to_string(fault)){scenario(argv[1],fault);ran=true;}
    if(argc==2||std::string_view(argv[2])=="queued"){
        queued_shutdown_requests(argv[1]);ran=true;
    }
    if(argc==2||std::string_view(argv[2])=="queued-failure"){
        queued_shutdown_requests(argv[1],true);ran=true;
    }
    if(argc==2||std::string_view(argv[2])=="stale-status"){
        stale_cleanup_status_window(argv[1]);ran=true;
    }
    require(ran,"unknown fault mode");
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
