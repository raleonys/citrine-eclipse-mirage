// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/control.h"
#include <chrono>
#include <iostream>
#include <initializer_list>
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
    void (*resume_startup)()=nullptr;
    explicit Module(const char* path){
        module=LoadLibraryA(path);require(module!=nullptr,"fixture DLL load");
        stop=reinterpret_cast<decltype(stop)>(GetProcAddress(module,"DllShutdown"));
        mode=reinterpret_cast<decltype(mode)>(GetProcAddress(module,"FixtureMode"));
        value=reinterpret_cast<decltype(value)>(GetProcAddress(module,"FixtureValue"));
        resume=reinterpret_cast<decltype(resume)>(GetProcAddress(module,"FixtureReleaseWorker"));
        resume_startup=reinterpret_cast<decltype(resume_startup)>(GetProcAddress(module,"FixtureReleaseStartup"));
        require(stop&&mode&&value&&resume&&resume_startup,"fixture methods");
    }
    ~Module(){
        // Release either controlled barrier even when an assertion throws.
        if(resume_startup)resume_startup();
        if(resume)resume();
        if(mode&&stop){mode(0);if(stop(10000)==DLL_SHUTDOWN_OK)FreeLibrary(module);}
    }
};
struct EnvironmentValue {
    const char* name;
    std::string previous;
    bool present=false;
    EnvironmentValue(const char* key,const char* value):name(key){
        SetLastError(ERROR_SUCCESS);
        const auto size=GetEnvironmentVariableA(name,nullptr,0);
        if(size){
            previous.resize(size);
            SetLastError(ERROR_SUCCESS);
            const auto copied=GetEnvironmentVariableA(name,previous.data(),size);
            require(copied<size&&(copied||GetLastError()==ERROR_SUCCESS),"capture prior fixture environment value");
            previous.resize(copied);present=true;
        }else{
            const auto error=GetLastError();
            require(error==ERROR_SUCCESS||error==ERROR_ENVVAR_NOT_FOUND,"query prior fixture environment value");
            present=error==ERROR_SUCCESS;
        }
        require(SetEnvironmentVariableA(name,value)!=FALSE,"set startup fixture environment");
    }
    ~EnvironmentValue(){SetEnvironmentVariableA(name,present?previous.c_str():nullptr);}
};
void shutdown_during_startup(const char* path,unsigned phase,bool fail){
    const std::string selection=std::string(phase==1?"provider":"ready")+(fail?"-fail":"");
    EnvironmentValue startup("SURGE_TEST_PAUSE_STARTUP",selection.c_str());
    EnvironmentValue no_post_start_pause("SURGE_TEST_HOLD_WORKER_BEFORE_STOP",nullptr);
    Module m(path);
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!m.value(11)&&std::chrono::steady_clock::now()<until)Sleep(1);
    require(m.value(11)==phase&&m.value(12)==1,"real startup must reach the selected pre-publication barrier");
    require(m.value(17)&&m.value(18),"actual startup and paused thread identities recorded");
    require((m.value(17)==m.value(18))==(phase==2),
        "provider barrier runs on provider worker; ready barrier runs inside entry startup worker");
    auto retained=[&]{
        require(m.value(11)==phase&&!m.value(0)&&!m.value(13)&&!m.value(14),
            "startup remains in progress without reporting ready, return or failure");
        require(!m.value(19),"paused publication is actual startup, not abort-cleanup gate release");
        require(m.value(2)==1&&!m.value(3)&&!m.value(1),
            "queued shutdown cannot stop or destroy the single starting application");
        require(m.value(6)&&!m.value(4)&&!m.value(5),
            "real callback binding and engine ownership remain live during startup");
        require(m.value(16)==WAIT_TIMEOUT,"entry worker remains alive while startup is paused");
    };
    retained();
    require(m.value(15)==0,"no shutdown request exists before the public calls");
    require(m.stop(0)==DLL_SHUTDOWN_TIMEOUT,"zero-wait shutdown during startup must forbid unload");
    require(m.value(15)==1,"first public request is queued during startup");
    retained();
    require(m.stop(25)==DLL_SHUTDOWN_TIMEOUT,"bounded shutdown must time out while startup is held");
    require(m.value(15)==2,"second public request remains distinct before startup returns");
    retained();
    m.resume_startup();
    // Observe actual worker termination without another shutdown call: a late
    // caller must not hide loss of the requests queued before startup ended.
    const auto done_until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(m.value(16)==WAIT_TIMEOUT&&std::chrono::steady_clock::now()<done_until)Sleep(1);
    require(m.value(16)==WAIT_OBJECT_0,"queued requests finish the actual worker without a third public call");
    require(!m.value(11)&&m.value(12)==1,"startup barrier retires once and does not rearm during abort cleanup");
    require(m.value(13)==(fail?0u:1u)&&m.value(14)==(fail?1u:0u)&&m.value(0)==(fail?0u:1u),
        "real startup takes exactly the selected success or exception path");
    require(m.value(15)==(fail?3u:2u),"only a startup exception adds the worker's implicit cleanup request");
    require(m.value(1)==1&&m.value(2)==1&&m.value(3)==1,
        "successful queued cleanup stops and destroys the single owner exactly once");
    require(!m.value(6)&&!m.value(7)&&m.value(4)==1&&m.value(5)==1,
        "callback leases drain and engine ownership retires without duplicate cleanup");
    require(m.stop(0)==DLL_SHUTDOWN_OK,"completed startup-overlap shutdown remains idempotent");
    std::cout<<"PASS: public shutdown queued during "<<(phase==1?"provider":"ready")
        <<" startup; "<<(fail?"exception unwind":"normal return")
        <<" retains ownership until release and exits without a third request\n";
}
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
void staged_file_hook_public_shutdown(const char* path){
    // A11-06-X1 limited test: actual recovery returns before the injected
    // boundary exception; no native rollback failure is fabricated.
    Module m(path);
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(10);
    while(!m.value(0)&&std::chrono::steady_clock::now()<until)Sleep(1);
    require(m.value(0)&&m.value(6),"staged retry starts with one live Application and binding");
    m.mode(6);
    require(m.stop(1000)==DLL_SHUTDOWN_CLEANUP_FAILED,
        "first request reports the file-recovery boundary exception");
    require(m.value(1)==1&&m.value(2)==1&&m.value(3)==0&&m.value(6)&&
        m.value(9)==1&&m.value(10)==1&&m.value(4)==0&&m.value(5)==0,
        "file boundary fault retains owner and prevents hook disable and engine release");
    m.mode(1);
    require(m.stop(1000)==DLL_SHUTDOWN_CLEANUP_FAILED,
        "second request reports the real hook-disable failure");
    require(m.value(1)==2&&m.value(2)==1&&m.value(3)==0&&m.value(6)&&
        m.value(9)==2&&m.value(10)==2&&m.value(4)==1&&m.value(5)==0,
        "hook failure retains same owner and engine for an explicit retry");
    m.mode(0);
    require(m.stop(10000)==DLL_SHUTDOWN_OK,"third explicit request retires same Application");
    // The worker's successful stop is followed by owner.reset(). Impl's
    // destructor repeats idempotent recovery, outside the requested-stop scope.
    require(m.value(1)==3&&m.value(2)==1&&m.value(3)==1&&!m.value(6)&&
        m.value(9)==4&&m.value(10)==3&&m.value(4)==2&&m.value(5)==1,
        "final attempt retries unfinished hook disable and releases engine exactly once");
    require(m.stop(0)==DLL_SHUTDOWN_OK,"completed public shutdown is idempotent");
    require(m.value(1)==3&&m.value(9)==4&&m.value(10)==3&&m.value(4)==2&&m.value(5)==1,
        "idempotent shutdown never replays completed stages");
    std::cout<<"PASS: staged public shutdown file boundary and hook retries\n";
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
    if(argc==2||std::string_view(argv[2])=="startup"){
        for(const unsigned phase:{1u,2u})for(const bool fail:{false,true})
            shutdown_during_startup(argv[1],phase,fail);
        ran=true;
    }
    if(argc==2||std::string_view(argv[2])=="stage-file-hook"){
        staged_file_hook_public_shutdown(argv[1]);ran=true;
    }
    require(ran,"unknown fault mode");
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
