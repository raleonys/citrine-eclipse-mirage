#include "source_stop.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <condition_variable>
#include <process.h>

namespace {
using namespace anadius;using b18_test::require;using b18_test::rejects;
using Callback=native_recovered::Callback;
void drained_retry(){
    b18_test::Fixture f;auto domain=std::make_shared<foreign_callback::Domain>(f.heap.domain,[]{return true;});foreign_callback::Admission admission(domain);
    admission.host_setup_begin();admission.host_setup_complete();require(domain->ready(),"Own callback domain activated before stop");
    std::mutex gate;std::condition_variable cv;bool entered=false,signalled=false,noticed=false,leave=false;std::exception_ptr failure;
    std::thread thread([&]{try{admission.enter(Callback::reg_close_00f5b0);{
        std::unique_lock lock(gate);entered=true;cv.notify_all();if(!cv.wait_for(lock,std::chrono::seconds(5),[&]{return signalled;}))throw std::runtime_error("stop signal missing");noticed=true;cv.notify_all();if(!cv.wait_for(lock,std::chrono::seconds(5),[&]{return leave;}))throw std::runtime_error("callback release missing");
    }admission.leave(Callback::reg_close_00f5b0);}catch(...){failure=std::current_exception();}});
    {std::unique_lock lock(gate);if(!cv.wait_for(lock,std::chrono::seconds(5),[&]{return entered;}))std::abort();}
    unsigned calls=0;std::function<bool()> source=[&]{++calls;require(!domain->ready()&&domain->active(),"New admissions sealed before external stop while existing callback stays active");
        rejects<foreign_callback::Rejected>([&]{admission.enter(Callback::folder_w_00fc80);},"Stopped callback gate rejects new controller entry before external cancellation");
        require(::GetLastError()==0x8811,"External stopper sees incoming LastError");
        {std::unique_lock lock(gate);signalled=true;cv.notify_all();if(!cv.wait_for(lock,std::chrono::seconds(5),[&]{return noticed;}))throw std::runtime_error("callback did not observe external stop");}
        ::SetLastError(0x8822);return true;};
    auto stop=host_platform::bind_source_stop(domain,source);auto alias=stop;require(!source&&domain->ready()&&!calls,"Passive construction consumes source donor without pausing or calling it");
    ::SetLastError(0x8811);const bool first=stop();const auto error=::GetLastError();require(!first&&calls==1&&error==0x8822&&domain->active(),"External true does not substitute for active callback drain and preserves its LastError");
    {std::lock_guard lock(gate);leave=true;}cv.notify_all();thread.join();if(failure)std::rethrow_exception(failure);
    ::SetLastError(0x8833);require(alias()&&stop()&&calls==1&&::GetLastError()==0x8833,"Copied stopper shares memoized true; retry drains without repeating external stop");
}
void false_retry_and_affinity(){
    b18_test::Fixture f;auto domain=std::make_shared<foreign_callback::Domain>(f.heap.domain,[]{return true;});unsigned calls=0;
    std::function<bool()> source=[&]{++calls;return calls>1;};auto stop=host_platform::bind_source_stop(domain,source);
    b18_test::in_thread([&]{rejects<std::logic_error>([&]{(void)stop();},"Foreign stop caller rejected before source or callback metadata");});require(calls==0,"Wrong controller does not execute external stopper or poison valid owner");
    require(!stop()&&calls==1,"External false remains retryable while callback admissions stay sealed");foreign_callback::Admission admission(domain);
    rejects<foreign_callback::Rejected>([&]{admission.enter(Callback::reg_close_00f5b0);},"External false cannot reopen callback gate");require(stop()&&stop()&&calls==2,"Only false external outcomes are retried; first true is memoized");
    std::function<bool()> other=[]{return true;};rejects<std::invalid_argument>([&]{(void)host_platform::bind_source_stop({},other);},"Missing callback owner rejects before consuming source");require(bool(other),"Rejected bind preserves source donor");
    other={};rejects<std::invalid_argument>([&]{(void)host_platform::bind_source_stop(domain,other);},"Missing external stopper is never replaced by success");
}
[[noreturn]]void retained_child(bool nested){
    auto* f=new b18_test::Fixture;auto domain=std::make_shared<foreign_callback::Domain>(f->heap.domain,[]{return true;});
    auto* stop=new std::function<bool()>;unsigned calls=0;bool swallowed=false;
    std::function<bool()> source=[&]()->bool{++calls;if(!nested)throw std::runtime_error("unknown external stopper outcome");try{(void)(*stop)();}catch(const std::logic_error&){swallowed=true;}return true;};
    *stop=host_platform::bind_source_stop(domain,source);bool rejected=false;try{(void)(*stop)();}catch(const std::exception&){rejected=true;}
    bool sticky=false;try{(void)(*stop)();}catch(const std::logic_error&){sticky=true;}
    foreign_callback::Admission admission(domain);bool sealed=false;try{admission.enter(Callback::reg_close_00f5b0);}catch(const foreign_callback::Rejected& e){sealed=e.fault==foreign_callback::Fault::Stopped;}
    if(rejected&&sticky&&sealed&&calls==1&&swallowed==nested&&!source&&!domain->active())::ExitProcess(nested?202:201);::ExitProcess(239);
}
}
int main(int argc,char** argv){try{if(argc==2)retained_child(std::string_view(argv[1])=="--nested");drained_retry();false_retry_and_affinity();require(_spawnl(_P_WAIT,argv[0],argv[0],"--unknown",static_cast<const char*>(nullptr))==201,"Unknown external outcome retains poisoned source owner without retry/cleanup");require(_spawnl(_P_WAIT,argv[0],argv[0],"--nested",static_cast<const char*>(nullptr))==202,"Swallowed nested stop rejection cannot become successful stop");std::cout<<"PASS: "<<b18_test::checks<<" source stop checks; real foreign PTD, active cancellation and two retained prefixes\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';std::_Exit(1);}}
