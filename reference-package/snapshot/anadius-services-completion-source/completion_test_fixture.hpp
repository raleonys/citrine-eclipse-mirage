#pragma once
#include "completion.hpp"
#include "../anadius-bootstrap-policies-source/services.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <process.h>
#include <sstream>
#include <thread>
#include <vector>

namespace b10_test {
using namespace anadius;
namespace sc=services_completion;
inline unsigned checks=0;
inline void require(bool v,const char* message){++checks;if(!v)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& f,const char* message){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,message);}
struct Buffer:std::stringbuf {
    unsigned syncs=0;
    int sync() override{++syncs;return 0;}
};
struct Sink {
    Buffer buffer;
    std::ostream stream{&buffer};
};
inline std::shared_ptr<std::ostream> stream_owner(const std::shared_ptr<Sink>& sink){return {sink,&sink->stream};}
inline std::shared_ptr<sc::BootstrapLogger> logger(const std::shared_ptr<Sink>& sink){return std::make_shared<sc::BootstrapLogger>(stream_owner(sink));}
inline std::shared_ptr<sc::ExitWaitRegistry> registry(sc::ExitWaitApi a=sc::native_exit_wait_api()){
    return std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned,a);
}
struct Probe {
    std::vector<std::string> events;
    sc::WaitTarget* target=nullptr;
    bool ready_before_release=false,unlocked_before_wake=false;
    unsigned initializes=0,destroys=0,wakes=0;
};
inline Probe* probe=nullptr;
inline DWORD WINAPI thread_id(){probe->events.emplace_back("thread");return ::GetCurrentThreadId();}
inline BOOL WINAPI initialize(LPCRITICAL_SECTION p,DWORD spin,DWORD flags){
    ++probe->initializes;const auto result=::InitializeCriticalSectionEx(p,spin,flags);return result?0x100:0;
}
inline BOOL WINAPI fail_initialize(LPCRITICAL_SECTION,DWORD,DWORD){++probe->initializes;::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
inline void WINAPI destroy(LPCRITICAL_SECTION p){++probe->destroys;::DeleteCriticalSection(p);}
inline void WINAPI enter(LPCRITICAL_SECTION p){probe->events.emplace_back("enter");::EnterCriticalSection(p);}
inline void WINAPI leave(LPCRITICAL_SECTION p){probe->events.emplace_back("leave");::LeaveCriticalSection(p);}
inline void WINAPI acquire(PSRWLOCK p){probe->events.emplace_back("acquire");::AcquireSRWLockExclusive(p);}
inline void WINAPI release(PSRWLOCK p){
    probe->events.emplace_back("release");
    if(probe->target)probe->ready_before_release=probe->target->ready_value_quiescent()==1;
    ::ReleaseSRWLockExclusive(p);
}
inline void WINAPI wake(PCONDITION_VARIABLE p){
    probe->events.emplace_back("wake");++probe->wakes;
    if(probe->target)probe->unlocked_before_wake=!probe->target->is_locked();
    ::WakeAllConditionVariable(p);
}
inline sc::ExitWaitApi probe_api(){
    return {thread_id,initialize,destroy,enter,leave,acquire,release,wake,::GetLastError};
}
inline std::size_t pos(const std::vector<std::string>& v,std::string_view s){
    const auto it=std::find(v.begin(),v.end(),s);
    if(it==v.end())throw std::runtime_error("Expected event absent");
    return static_cast<std::size_t>(it-v.begin());
}
} // namespace b10_test
