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
// Publish the real bootstrap Runtime without invoking application attach. The
// certified native provider owns this Domain, but no worker/SDK work is started.
void bound_runtime(b4_test::Fixture& fixture,const std::shared_ptr<thread_domain::DomainStorage>& root){
    auto resources=fixture.resources();resources.threads=root->make_threads();
    fixture.construct(bootstrap_backend::take_resources(resources));fixture.runtime->publish();
    require(fixture.runtime->memory_domain()==root.get(),"Runtime captures actual Domain factory provenance before Resources transfer");
}
void rejected_domains(){
    auto root=domain();
    {
        b4_test::Fixture fixture;bound_runtime(fixture,root);auto* runtime=fixture.runtime.get();std::shared_ptr<thread_domain::DomainStorage> missing;
        bool rejected=false;try{(void)pu::Controller::take(fixture.runtime,missing);}catch(const std::invalid_argument&){rejected=true;}
        require(rejected&&fixture.runtime.get()==runtime&&!missing&&root->phase()==thread_domain::Phase::Ready&&fixture.stops==0,"Missing Domain rejected before consumption or shutdown");
    }
    {
        b4_test::Fixture fixture;fixture.construct(fixture.resources());auto* runtime=fixture.runtime.get();
        bool rejected=false;try{(void)pu::Controller::take(fixture.runtime,root);}catch(const std::invalid_argument&){rejected=true;}
        require(rejected&&fixture.runtime.get()==runtime&&root&&fixture.stops==0&&!fixture.runtime->memory_domain(),"Legacy custom thread provider has no certified Domain and leaves donors intact");
    }
    // Two real native Domains cannot coexist: Binding reserves one image-wide
    // callback slot at construction. Different-Ready-root rejection is static
    // coverage only; do not fabricate a second root or bypass that reservation.
    require(root->close(),"Rejected Domain donor remains caller-owned and closes normally");
}
void retries_and_retirement(){
    b4_test::Fixture fixture;auto root=domain();bound_runtime(fixture,root);auto borrow=root;auto provider=root->make_memory();
    const auto entry=pu::Controller::entry();require(!entry(nullptr)&&::GetLastError()==ERROR_INVALID_PARAMETER,"null opaque context rejected");
    auto owner=pu::Controller::take(fixture.runtime,root);require(!fixture.runtime&&!root&&!owner->closed()&&owner->snapshot().phase==pu::Phase::Managed,"passive consuming controller owns actual Runtime and Domain");
    bool wrong=false;std::thread thread([&]{wrong=!entry(owner->context())&&::GetLastError()==ERROR_INVALID_THREAD_ID;});thread.join();require(wrong&&owner->snapshot().phase==pu::Phase::Managed&&fixture.stops==0,"wrong thread cannot run source shutdown");
    fixture.stop_allowed=false;require(!entry(owner->context())&&::GetLastError()==ERROR_IO_PENDING&&owner->snapshot().phase==pu::Phase::ClosingRuntime&&fixture.stops==1&&fixture.destroyed_config.empty(),"source false retains actual Runtime and all providers");
    fixture.stop_allowed=true;require(!entry(owner->context())&&owner->snapshot().phase==pu::Phase::ClosingDomain&&!fixture.destroyed_config.empty()&&borrow->phase()==thread_domain::Phase::Stopping,"Runtime closure and destruction precede Domain provider-lease retry");
    const auto stopped=fixture.stops;const auto events=fixture.track.events.size();require(!entry(owner->context())&&fixture.stops==stopped&&fixture.track.events.size()==events,"Domain retry never repeats Runtime stop/cleanup/destruction");
    provider.reset();::SetLastError(0x8181);require(entry(owner->context())&&owner->closed()&&borrow->phase()==thread_domain::Phase::Closed&&::GetLastError()==0x8181,"real Domain closes after provider retirement and reports managed completion");
    require(entry(owner->context())&&fixture.stops==stopped&&fixture.track.events.size()==events,"completed shutdown is idempotent");const auto receipt=owner->snapshot();require(receipt.phase==pu::Phase::Closed&&!receipt.busy&&!receipt.error,"metadata-only completion receipt available");owner.reset();borrow.reset();
}
void owned_attach_preconditions(){
    b4_test::Fixture fixture;auto root=domain();auto borrowed=root;
    auto resources=fixture.resources();resources.threads=root->make_threads();fixture.construct(bootstrap_backend::take_resources(resources));
    auto* runtime=fixture.runtime.get();auto owner=pu::Controller::take(fixture.runtime,root);
    require(!fixture.runtime&&!root&&runtime->phase()==bootstrap_owner::Phase::Constructed,"Unpublished roots adopted without starting source application");
    bool null_rejected=false;try{owner->start_application(nullptr,nullptr);}catch(const std::invalid_argument&){null_rejected=true;}
    require(null_rejected&&owner->snapshot().phase==pu::Phase::Managed&&!owner->snapshot().busy&&!fixture.host.registrations,"Null image rejects before publication");
    const auto module=::GetModuleHandleW(nullptr);bool wrong_rejected=false;
    std::thread other([&]{try{owner->start_application(module,nullptr);}catch(const std::logic_error&){wrong_rejected=true;}});other.join();
    require(wrong_rejected&&owner->snapshot().phase==pu::Phase::Managed&&!owner->snapshot().busy&&runtime->phase()==bootstrap_owner::Phase::Constructed&&!fixture.host.registrations,"Wrong actor cannot publish owned roots or poison the valid controller");
    require(owner->shutdown()&&owner->closed()&&borrowed->phase()==thread_domain::Phase::Closed,"Rejected pre-attach requests leave roots available for complete unpublished retirement");
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,[](int){std::_Exit(1472);});
    b4_test::Fixture fixture;auto root=domain();bound_runtime(fixture,root);auto owner=pu::Controller::take(fixture.runtime,root);
    if(mode=="live-delete"){owner.reset();return 1999;}
    if(mode=="reentry")fixture.stop_hook=[&]{require(!owner->shutdown(),"nested shutdown denied");};
    else fixture.stop_hook=[]{throw std::runtime_error("unknown external source stop outcome");};
    const auto before=fixture.stops;require(!owner->shutdown()&&owner->snapshot().phase==pu::Phase::Poisoned&&!owner->snapshot().busy&&!owner->closed(),"unknown/reentrant source prefix retained");
    require(!owner->shutdown()&&fixture.stops==before+1,"poisoned shutdown never retries opaque source");std::_Exit(1471);
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{rejected_domains();owned_attach_preconditions();retries_and_retirement();for(const char* mode:{"throw","reentry"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==1471,"isolated unknown prefixes retained");require(_spawnl(_P_WAIT,argv[0],argv[0],"live-delete",static_cast<const char*>(nullptr))==1472,"live pre-unload controller cannot be abandoned");std::cout<<"PASS: "<<checks<<" pre-unload checks; exact certified Domain selection, owned attach preconditions, Runtime retirement then Domain closure, native callback ABI and three isolated retained/terminal prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';std::_Exit(1);}}
