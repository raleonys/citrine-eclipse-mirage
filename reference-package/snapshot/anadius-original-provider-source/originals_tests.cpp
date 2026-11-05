#include "originals.hpp"
#include "../anadius-callback-policy-source/policy_test_fixture.hpp"
#include <climits>
#include <csignal>
#include <cstdlib>
#include <process.h>

namespace {
using namespace anadius;namespace p=original_provider;namespace n=native_recovered;namespace c=callback_policy;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
constexpr auto close_slot=host_init_recovered::Callback::reg_close_00f5b0;
void protocol(){
    n::OriginalSlots slots{};slots[3]=1234;auto code=std::make_shared<int>(9);std::weak_ptr<int> weak=code;
    rejects<std::invalid_argument>([&]{p::bind(slots,{});},"missing code owner rejected");
    rejects<std::invalid_argument>([]{p::make_factory({});},"missing factory code owner rejected");
    auto provider=p::bind(slots,code);code.reset();require(!weak.expired(),"bound provider retains supplied owner");
    require(provider->slots_identity()==&slots,"exact array identity retained");
    rejects<std::logic_error>([&]{provider->load(close_slot);},"Created load denied");
    rejects<std::logic_error>([&]{provider->setup_complete();},"publication before setup rejected");
    ::SetLastError(0x1122);provider->setup_begin();require(::GetLastError()==0x1122,"setup metadata preserves LastError");
    require(provider->load(close_slot)==1234&&::GetLastError()==0x1122,"setup controller can load current slot preserving LastError");
    bool denied=false;std::thread foreign([&]{try{provider->load(close_slot);}catch(const std::logic_error&){denied=true;}});foreign.join();require(denied,"foreign setup load rejected");
    slots[3]=5678;require(provider->load(close_slot)==5678,"setup does not freeze before engine writes finish");
    rejects<std::logic_error>([&]{provider->setup_begin();},"setup cannot be repeated");
    bool wrong=false;std::thread complete([&]{try{provider->setup_complete();}catch(const std::logic_error&){wrong=true;}});complete.join();require(wrong,"only controller may publish");
    provider->setup_complete();std::uintptr_t loaded=0;std::thread reader([&]{loaded=provider->load(close_slot);});reader.join();require(loaded==5678,"Ready snapshot admits selected foreign load");
    rejects<std::logic_error>([&]{provider->setup_complete();},"Ready cannot reseal");
    rejects<std::out_of_range>([&]{provider->load(static_cast<n::Callback>(255));},"invalid callback index rejected");
    rejects<std::logic_error>([&]{provider->load(n::Callback::folder_a_00fc00);},"sealed empty slot never becomes callable");
    require(provider->load(close_slot)==5678,"unbound slot rejection does not alter other sealed slots");
    slots[3]=9012;::SetLastError(0x3344);rejects<std::logic_error>([&]{provider->load(close_slot);},"changed nonzero slot rejected");
    require(::GetLastError()==0x3344,"mismatch bookkeeping preserves LastError");slots[3]=5678;
    rejects<std::logic_error>([&]{provider->load(close_slot);},"observed mismatch remains stopped after restoration");
    rejects<std::logic_error>([&]{provider->setup_begin();},"stopped provider cannot reset");
    provider->stop();provider->stop();provider.reset();require(weak.expired(),"last bound owner releases code token");
    auto zero=p::bind(slots,std::make_shared<int>(2));zero->setup_begin();zero->setup_complete();slots[3]=0;
    rejects<std::logic_error>([&]{zero->load(close_slot);},"published slot changed to zero rejected");slots[3]=5678;
    rejects<std::logic_error>([&]{zero->load(close_slot);},"observed zero mismatch also remains stopped");
}
void binding_and_lifetime(){
    b16_test::Fixture fixture;auto lifetime=std::make_shared<int>(11);std::weak_ptr<int> weak=lifetime;
    auto factory=p::make_factory(lifetime);auto copied=factory;factory={};lifetime.reset();
    require(!weak.expired(),"copied provider factory keeps supplied code owner");
    auto provider=copied(fixture.driver.runtime.originals);copied={};require(!weak.expired(),"bound provider keeps owner after factory copies released");
    auto resources=fixture.resources();resources.originals=[provider](const n::OriginalSlots&){return provider;};
    c::Policy policy(fixture.view(),std::move(resources));require(policy.original_provider()==provider.get(),"Policy passes retained provider to final OriginalEffects base");
    provider->setup_begin();provider->setup_complete();::SetLastError(0x7788);
    require(policy.original_close(0x123)==0&&fixture.driver.probe->closes==1&&::GetLastError()==0x1234,"selected final typed original route invokes own exact stub and preserves outgoing error");
    const auto other=fixture.driver.runtime.originals;auto wrong=p::bind(other,std::make_shared<int>(3));auto r=fixture.resources();
    r.originals=[wrong](const n::OriginalSlots&){return wrong;};
    rejects<std::invalid_argument>([&]{c::Policy rejected(fixture.view(),std::move(r));},"different array rejected even if all slot values equal");
    auto missing=fixture.resources();missing.originals=[](const n::OriginalSlots&){return std::shared_ptr<n::OriginalProvider>{};};
    rejects<std::invalid_argument>([&]{c::Policy rejected(fixture.view(),std::move(missing));},"selected factory may not silently return legacy null");
    const auto saved=fixture.driver.runtime.originals[3];fixture.driver.runtime.originals[3]=0;
    rejects<std::logic_error>([&]{policy.original_close(0x123);},"missing published slot rejects before SDK call");
    require(fixture.driver.probe->closes==1,"rejected missing slot does not invoke original stub");fixture.driver.runtime.originals[3]=saved;
    rejects<std::logic_error>([&]{policy.original_close(0x123);},"caught mismatch cannot be repaired into another dispatch");
    require(!weak.expired(),"policy retains provider owner through rejected calls");
}
void owner_protocol(){
    b16_test::Fixture f;auto token=std::make_shared<int>(3);std::weak_ptr<int> weak=token;
    auto r=f.resources();r.originals=p::make_factory(token);token.reset();
    bool proof=false;unsigned proofs=0;
    {
        native_context::Owner owner(f.driver.view(),b15_test::seed(),c::make_policy_factory(std::move(r)),b15_test::permanent_errors,[&]{++proofs;return proof;});
        // std::function's moved-from small callable may still own its capture.
        r.originals={};
        owner.publish();require(n::is_bound()&&!weak.expired(),"published Context retains selected original owner");
        owner.host_setup_begin();const auto raw=f.driver.runtime.originals[3];
        require(n::reg_close_entry(reinterpret_cast<HKEY>(1))==0,"controller original callback works during host setup");
        owner.host_setup_complete();
        require(n::reg_close_entry(reinterpret_cast<HKEY>(1))==0,"sealed selected original dispatch works after setup");
        require(!owner.close_after_cleanup()&&n::is_bound()&&!weak.expired()&&proofs==1,"false quiescence preserves binding and original owner");
        require(owner.pause_before_cleanup(),"known drain stops selected provider");
        proof=true;require(owner.close_after_cleanup()&&!n::is_bound()&&!weak.expired(),"successful close retains code until Context destruction");
        f.driver.runtime.originals[3]=raw;
    }
    require(weak.expired(),"Context destruction releases selected code owner");
}
void progressive_publication(){
    n::OriginalSlots slots{};slots.fill(0xabc);
    auto code=std::make_shared<int>(4);std::weak_ptr<int> weak=code;
    auto publication=std::make_shared<p::Publication>(code);code.reset();
    auto factory=p::make_publishing_factory(publication);auto copied=factory;
    auto provider=factory(slots);factory={};
    require(provider.get()==publication.get()&&provider->slots_identity()==&slots,"matched factory binds exact stable slots to the shared publisher");
    rejects<std::logic_error>([&]{copied(slots);},"copied publishing factory cannot bind a second time");copied={};
    rejects<std::logic_error>([&]{publication->load(close_slot);},"seeded nonzero cell is not a successful-create receipt");
    publication->setup_begin();const auto index=host_init_recovered::slot(close_slot);
    std::uintptr_t wrong=0;
    rejects<std::invalid_argument>([&]{publication->before_create(close_slot,0x300,wrong);},"equal-looking external output is not the exact bound cell");
    ::SetLastError(0x7711);publication->before_create(close_slot,0x300,slots[index]);
    require(publication->snapshot().creating&&::GetLastError()==0x7711,"create reservation preserves LastError before any lower effect");
    slots[index]=0x400;publication->after_create(close_slot,0x300,slots[index],0);
    require(::GetLastError()==0x7711&&!publication->snapshot().creating&&publication->snapshot().phase==p::PublicationPhase::Setup,
        "successful receipt publishes one slot while global setup remains incomplete");
    std::uintptr_t loaded=0;std::exception_ptr failure;
    std::thread reader([&]{try{loaded=provider->load(close_slot);}catch(...){failure=std::current_exception();}});reader.join();
    if(failure)std::rethrow_exception(failure);
    require(loaded==0x400,"foreign reader can load the published immutable slot before global setup completion");
    publication->before_enable(0x300);
    rejects<std::logic_error>([&]{publication->before_enable(0x301);},"enable for an unreceived target is rejected");
    rejects<std::logic_error>([&]{publication->before_create(close_slot,0x302,slots[index]);},"published slot rejects a later rewrite before lower create");
    const auto open=n::Callback::reg_open_a_00f800;const auto open_index=host_init_recovered::slot(open);
    publication->before_create(open,0x500,slots[open_index]);slots[open_index]=0x600;
    publication->after_create(open,0x500,slots[open_index],8);
    rejects<std::logic_error>([&]{publication->load(open);},"normal failed create with changed nonzero output publishes no pointer");
    publication->before_create(open,0x500,slots[open_index]);slots[open_index]=0x700;
    publication->after_create(open,0x500,slots[open_index],0);publication->before_enable(0x500);
    publication->setup_complete();require(publication->load(open)==0x700,"known failed create can be explicitly retried before setup closes");
    publication->stop();rejects<std::logic_error>([&]{publication->load(close_slot);},"stop excludes later original loads without manufacturing engine cleanup");
    provider.reset();publication.reset();require(weak.expired(),"shared publisher retains code through all source-provider aliases");
}
[[noreturn]]void unknown_publication(){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::signal(SIGABRT,[](int){std::_Exit(188);});
    n::OriginalSlots slots{};auto publication=std::make_shared<p::Publication>(std::make_shared<int>(1));
    publication->bind_slots(slots);publication->setup_begin();publication->before_create(close_slot,1,slots[3]);
    publication->unknown();publication->stop();
    if(publication->snapshot().phase!=p::PublicationPhase::Poisoned||!publication->snapshot().creating)std::_Exit(189);
    publication.reset();std::_Exit(190); // unknown create cannot be retired by stop
}
}
int main(int argc,char** argv){if(argc==2&&std::string(argv[1])=="--publication-unknown")unknown_publication();try{protocol();binding_and_lifetime();owner_protocol();progressive_publication();require(_spawnl(_P_WAIT,argv[0],argv[0],"--publication-unknown",static_cast<const char*>(nullptr))==188,"unknown publication remains terminal after stop");std::cout<<"PASS: "<<checks<<" original provider core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
