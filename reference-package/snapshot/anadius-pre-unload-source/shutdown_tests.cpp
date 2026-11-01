#include "shutdown.hpp"
#include "../anadius-thread-domain-source/factory.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <process.h>
#include <thread>

namespace {
using namespace anadius;namespace pu=pre_unload;unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
std::shared_ptr<thread_domain::DomainStorage> domain(){thread_domain::InitialInputs in({0x31795abc6e4d280fULL,0,nullptr,nullptr,1});auto root=thread_domain::make_initial_domain(in);require(root->initialize(),"own real Domain initialized");return root;}
void retries_and_retirement(){
    b4_test::Fixture fixture;fixture.start();auto root=domain();auto borrow=root;auto provider=root->make_memory();
    const auto entry=pu::Controller::entry();require(!entry(nullptr)&&::GetLastError()==ERROR_INVALID_PARAMETER,"null opaque context rejected");
    auto owner=pu::Controller::take(fixture.runtime,root);require(!fixture.runtime&&!root&&!owner->closed()&&owner->snapshot().phase==pu::Phase::Managed,"passive consuming controller owns actual Runtime and Domain");
    bool wrong=false;std::thread thread([&]{wrong=!entry(owner->context())&&::GetLastError()==ERROR_INVALID_THREAD_ID;});thread.join();require(wrong&&owner->snapshot().phase==pu::Phase::Managed&&fixture.stops==0,"wrong thread cannot run source shutdown");
    fixture.stop_allowed=false;require(!entry(owner->context())&&::GetLastError()==ERROR_IO_PENDING&&owner->snapshot().phase==pu::Phase::ClosingRuntime&&fixture.stops==1&&fixture.destroyed_config.empty(),"source false retains actual Runtime and all providers");
    fixture.stop_allowed=true;require(!entry(owner->context())&&owner->snapshot().phase==pu::Phase::ClosingDomain&&!fixture.destroyed_config.empty()&&borrow->phase()==thread_domain::Phase::Stopping,"Runtime closure and destruction precede Domain provider-lease retry");
    const auto stopped=fixture.stops;const auto events=fixture.track.events.size();require(!entry(owner->context())&&fixture.stops==stopped&&fixture.track.events.size()==events,"Domain retry never repeats Runtime stop/cleanup/destruction");
    provider.reset();::SetLastError(0x8181);require(entry(owner->context())&&owner->closed()&&borrow->phase()==thread_domain::Phase::Closed&&::GetLastError()==0x8181,"real Domain closes after provider retirement and reports managed completion");
    require(entry(owner->context())&&fixture.stops==stopped&&fixture.track.events.size()==events,"completed shutdown is idempotent");const auto receipt=owner->snapshot();require(receipt.phase==pu::Phase::Closed&&!receipt.busy&&!receipt.error,"metadata-only completion receipt available");owner.reset();borrow.reset();
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,[](int){std::_Exit(1472);});
    b4_test::Fixture fixture;fixture.start();auto root=domain();auto owner=pu::Controller::take(fixture.runtime,root);
    if(mode=="live-delete"){owner.reset();return 1999;}
    if(mode=="reentry")fixture.stop_hook=[&]{require(!owner->shutdown(),"nested shutdown denied");};
    else fixture.stop_hook=[]{throw std::runtime_error("unknown external source stop outcome");};
    const auto before=fixture.stops;require(!owner->shutdown()&&owner->snapshot().phase==pu::Phase::Poisoned&&!owner->snapshot().busy&&!owner->closed(),"unknown/reentrant source prefix retained");
    require(!owner->shutdown()&&fixture.stops==before+1,"poisoned shutdown never retries opaque source");std::_Exit(1471);
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{retries_and_retirement();for(const char* mode:{"throw","reentry"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==1471,"isolated unknown prefixes retained");require(_spawnl(_P_WAIT,argv[0],argv[0],"live-delete",static_cast<const char*>(nullptr))==1472,"live pre-unload controller cannot be abandoned");std::cout<<"PASS: "<<checks<<" pre-unload checks; actual Runtime close/retirement then real Domain closure, native callback ABI and three isolated retained/terminal prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';std::_Exit(1);}}
