#include "errors.hpp"
#include "../anadius-native-context-source/context_test_fixture.hpp"
#include <array>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <process.h>
#include <thread>

namespace {
using namespace anadius;namespace pe=permanent_errors;namespace native=native_recovered;namespace bootstrap=bootstrap_native_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
void identities(){
    ::SetLastError(0x1212);const auto first=pe::compiler_abort_policy();require(::GetLastError()==0x1212,"selection preserves LastError");
    require(dynamic_cast<void*>(&first.host)==dynamic_cast<void*>(&first.bootstrap),"both typed bases belong to same permanent concrete object");
    std::array<native::Errors*,8> hosts{};std::array<bootstrap::Errors*,8> bootstraps{};std::array<std::thread,8> threads;std::atomic<bool> errors=true;
    for(std::size_t i=0;i<threads.size();++i)threads[i]=std::thread([&,i]{::SetLastError(static_cast<DWORD>(0x2000+i));const auto selected=pe::compiler_abort_policy();hosts[i]=&selected.host;bootstraps[i]=&selected.bootstrap;if(::GetLastError()!=0x2000+i)errors=false;});
    for(auto& thread:threads)thread.join();require(errors,"per-thread LastError preserved");for(std::size_t i=0;i<threads.size();++i)require(hosts[i]==&first.host&&bootstraps[i]==&first.bootstrap,"typed identities stable across callers");
    b15_test::Driver driver;{
        native_context::Owner context(driver.view(),b15_test::seed(),driver.policies(),first.host,[]{return true;});context.publish();
        require(native::reg_close_entry(nullptr)==0&&driver.probe->closes==1&&!native::active_calls(),"actual native entry works with permanent policy");require(context.close_after_cleanup(),"actual Context closes with permanent policy");
    }
    require(!native::is_bound()&&driver.probe->destroyed==1,"actual policy/context providers retire independently of error storage");
    b2_test::Track track;{
        auto backend=std::make_unique<b2_test::Backend>(track);bootstrap_owner::RuntimeOwner owner(b2_test::seed(),std::move(backend),first.bootstrap);owner.publish();require(bootstrap::is_bound(),"actual RuntimeOwner publishes with permanent policy");require(owner.close(),"unstarted actual RuntimeOwner closes with permanent policy");
    }
    require(!bootstrap::is_bound()&&track.destroyed==1,"RuntimeOwner and backend retire independently of error storage");const auto again=pe::compiler_abort_policy();require(&again.host==&first.host&&&again.bootstrap==&first.bootstrap,"both identities survive actual owner destruction");
}
LSTATUS WINAPI throwing_close(HKEY){throw std::runtime_error("own callback exception");}
bootstrap::ApplicationEntry cached_application=nullptr;
void late_host(){native::reg_close_entry(nullptr);std::_Exit(1997);}
void late_bootstrap(){cached_application(nullptr,1,nullptr);std::_Exit(1997);}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::signal(SIGABRT,[](int){std::_Exit(!native::active_calls()&&!bootstrap::active_calls()?1451:1996);});
    if(mode=="host-late-exit"&&std::atexit(late_host))return 1994;
    if(mode=="bootstrap-late-exit"&&std::atexit(late_bootstrap))return 1994;
    const auto policies=pe::compiler_abort_policy();
    if(mode=="host-lifecycle")policies.host.lifecycle(native::LifecycleFault::Busy);
    if(mode=="bootstrap-lifecycle")policies.bootstrap.lifecycle(bootstrap::LifecycleFault::Busy);
    if(mode=="host-error")policies.host.callback_exception(native::Callback::connect_00fd00,{});
    if(mode=="bootstrap-error")policies.bootstrap.callback_exception(bootstrap::Entry::Terminate,{});
    if(mode.starts_with("host-")){
        b15_test::Driver driver;if(mode=="host-throw")driver.runtime.originals[3]=reinterpret_cast<std::uintptr_t>(&throwing_close);
        {native_context::Owner context(driver.view(),b15_test::seed(),driver.policies(),policies.host,[]{return true;});context.publish();
         if(mode=="host-throw")native::reg_close_entry(nullptr);
         if(mode=="host-occupied"){native_context::Owner another(driver.view(),b15_test::seed(),driver.policies(),policies.host,[]{return true;});another.publish();}
         if(!context.close_after_cleanup())return 1995;}
        if(mode=="host-late-exit")return 0;
        native::reg_close_entry(nullptr);
    }else{
        b2_test::Track track;{
            auto backend=std::make_unique<b2_test::Backend>(track);auto* raw=backend.get();bootstrap_owner::RuntimeOwner owner(b2_test::seed(),std::move(backend),policies.bootstrap);owner.publish();cached_application=owner.application_entry();
            if(mode=="bootstrap-throw"){raw->service_failure="register-exit";cached_application(nullptr,1,nullptr);}
            if(mode=="bootstrap-occupied"){b2_test::Track other_track;bootstrap_owner::RuntimeOwner other(b2_test::seed(),std::make_unique<b2_test::Backend>(other_track),policies.bootstrap);other.publish();}
            if(!owner.close())return 1995;
        }
        if(mode=="bootstrap-late-exit")return 0;
        cached_application(nullptr,1,nullptr);
    }
    return 1993;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{identities();for(const char* mode:{"host-lifecycle","bootstrap-lifecycle","host-error","bootstrap-error","host-closed","bootstrap-closed","host-throw","bootstrap-throw","host-occupied","bootstrap-occupied","host-late-exit","bootstrap-late-exit"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==1451,"explicit terminal policy after actual callback unwind, conflicts and owner/static teardown");std::cout<<"PASS: "<<checks<<" permanent-error checks; stable typed identities, actual native Context/RuntimeOwner and twelve isolated terminal routes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
