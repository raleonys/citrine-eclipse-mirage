#include "foreign_test_fixture.hpp"

namespace {
using namespace b18_test;
void ptd_scopes(){
    Fixture f;auto threads=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();
    auto* prior=::FlsGetValue(index);const auto locale=f.heap.domain->locale_references(),mb=f.heap.domain->multibyte_references();
    require(prior&&f.memory->domain_identity()==f.heap.domain.get(),"memory and callback PTD domain identities exact");
    threads->enter();require(threads->active()&&!threads->owned()&&threads->permits_current(),"existing controller PTD is borrowed");
    require(threads->leave()&&::FlsGetValue(index)==prior&&f.heap.domain->locale_references()==locale,"borrowed PTD retained without destructor");
    rejects<std::logic_error>([&]{threads->leave();},"unmatched scope leave rejected");
    ::FlsSetValue(index,crt_ptd_recovered::construction_sentinel());
    rejects<std::logic_error>([&]{threads->enter();},"existing construction sentinel is never adopted");
    require(!threads->active()&&!threads->poisoned(),"sentinel rejection reserves no new scope");::FlsSetValue(index,prior);
    const auto frees=f.heap.memory.frees;
    in_thread([&]{
        require(!::FlsGetValue(index),"own foreign thread starts with no domain PTD");::SetLastError(0x8811);threads->enter();const auto entered_error=::GetLastError();
        auto* current=::FlsGetValue(index);
        require(current&&threads->owned()&&threads->permits_current()&&entered_error==0x8811,"fresh PTD installed before foreign body with incoming error retained");
        require(f.heap.domain->locale_references()==locale+1&&f.heap.domain->multibyte_references()==mb+1,"fresh scope retains initial locale and multibyte once");
        auto* bytes=f.memory->allocate(4);f.memory->deallocate(bytes,4);
        crt_ptd_data_recovered::View(current).store<std::uint32_t>(0x20,99);
        crt_ptd_data_recovered::View(current).store<std::uint32_t>(0x24,5);
        ::SetLastError(0x8822);const auto left=threads->leave();const auto left_error=::GetLastError();
        require(left&&!::FlsGetValue(index)&&left_error==0x8822,"fresh scope detaches before return without error pollution");
    });
    require(!threads->active()&&f.heap.domain->locale_references()==locale&&f.heap.domain->multibyte_references()==mb,"foreign thread exit has no delayed PTD destructor/ref leak");
    require(f.heap.memory.frees==frees+2,"one callback allocation and one PTD root freed exactly once");
    in_thread([&]{f.heap.memory.deny_alloc=true;rejects<std::bad_alloc>([&]{threads->enter();},"raw PTD allocation failure propagates");f.heap.memory.deny_alloc=false;
        require(!threads->active()&&!threads->poisoned()&&!::FlsGetValue(index),"known allocation failure clears sentinel and reservation");threads->enter();require(threads->leave(),"allocation failure permits later explicit scope retry");});
    in_thread([&]{deny_set_sentinel=true;rejects<std::system_error>([&]{threads->enter();},"sentinel publication failure is checked");deny_set_sentinel=false;
        require(!threads->active()&&!threads->poisoned()&&!::FlsGetValue(index),"failed sentinel store does not poison a clean scope");});
    require(!f.heap.domain->close(),"callback and memory provider leases prevent domain release");
}
void cleanup_retry(){
    Fixture f;auto threads=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();
    const auto locale=f.heap.domain->locale_references(),mb=f.heap.domain->multibyte_references();
    in_thread([&]{
        threads->enter();auto* root=::FlsGetValue(index);deny_set_sentinel=true;
        require(!threads->leave()&&threads->active()&&threads->owned()&&::FlsGetValue(index)==root,"detach failure keeps live published PTD for same-thread retry");
        require(threads->last_failure()==ERROR_ACCESS_DENIED&&f.heap.domain->locale_references()==locale+1,"failed detach did not run PTD destructor");deny_set_sentinel=false;
        deny_root_free=true;require(!threads->leave()&&threads->active()&&threads->owned()&&::FlsGetValue(index)==crt_ptd_recovered::construction_sentinel(),"known root free failure retains destroyed PTD and sentinel");
        require(f.heap.domain->locale_references()==locale&&f.heap.domain->multibyte_references()==mb,"destroy completed once before failed root free");deny_root_free=false;
        const auto frees=f.heap.memory.frees;deny_set_null=true;
        require(!threads->leave()&&threads->active()&&!threads->owned()&&f.heap.memory.frees==frees+1,"root free success retained when final FLS clear fails");
        deny_set_null=false;require(threads->leave()&&!::FlsGetValue(index)&&f.heap.memory.frees==frees+1,"clear-only retry repeats neither destructor nor free");
    });
    require(!threads->poisoned()&&f.heap.domain->locale_references()==locale,"all known cleanup failures recovered without replacement PTD");
}
void permissions_and_publication(){
    Fixture f;bool proof=false;auto domain=std::make_shared<fc::Domain>(f.heap.domain,[&]{return proof;});fc::Admission admission(domain);
    auto resources=f.resources();resources.callback_scope=[domain]{return domain->permits_current();};nc::CallbackSeed seed=b15_test::seed();
    callback_policy::Policy policy({f.driver.view(),seed},std::move(resources));
    rejects<std::logic_error>([&]{policy.log_finish_0088e0();},"direct Policy call cannot bypass admitted scope");
    require(!domain->activate_foreign(),"ready proof cannot publish before host setup completes");
    admission.host_setup_begin();admission.host_setup_complete();require(!domain->ready(),"false shared-state proof retains warming phase");
    in_thread([&]{rejects<fc::Rejected>([&]{admission.enter(n::Callback::reg_close_00f5b0);},"foreign callback rejected during warming");});
    proof=true;require(domain->activate_foreign()&&domain->ready(),"completed setup plus explicit proof publishes foreign phase");
    in_thread([&]{admission.enter(n::Callback::reg_close_00f5b0);require(domain->permits_current(),"sequential foreign owner permission active");
        auto key=policy.allocate_4_04b0bc();policy.free_04b0f8(key);require(policy.owned_handles()==0,"foreign scope uses exact application memory provider");
        rejects<fc::Rejected>([&]{admission.enter(n::Callback::folder_w_00fc80);},"same-thread reentry is not silently allowed");admission.leave(n::Callback::reg_close_00f5b0);});
    require(!domain->active()&&!domain->permits_current(),"permission ends only after PTD cleanup");
    require(domain->pause()&&!domain->ready(),"controller pause permanently stops new admissions");
    rejects<fc::Rejected>([&]{admission.enter(n::Callback::reg_close_00f5b0);},"stopped controller callback rejected");
    auto existing=f.resources();existing.callback_scope=[]{return true;};rejects<std::invalid_argument>([&]{fc::make_factory(domain,b15_test::seed(),std::move(existing),b15_test::permanent_errors,[]{return true;});},"preexisting policy permission cannot be overwritten");
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);Fixture f;
    auto scopes=f.heap.domain->make_callback_threads();const auto index=f.heap.domain->native_fls_index();
    if(mode=="--publish-failure"||mode=="--alloc-unknown"||mode=="--construct-unknown"){
        expected_code=mode=="--publish-failure"?151:mode=="--alloc-unknown"?152:153;
        in_thread([&]{deny_set_nonnull=mode=="--publish-failure";throw_alloc=mode=="--alloc-unknown";throw_construct=mode=="--construct-unknown";
            try{scopes->enter();}catch(...){if(scopes->active()&&scopes->poisoned())terminate_own(expected_code);}terminate_own(230);});
    }else if(mode=="--free-unknown"||mode=="--ptd-mutation"){
        expected_code=mode=="--free-unknown"?154:155;
        in_thread([&]{scopes->enter();if(mode=="--ptd-mutation")crt_ptd_data_recovered::View(::FlsGetValue(index)).store<std::uint64_t>(0x48,1);else throw_free=true;
            try{scopes->leave();}catch(...){if(scopes->active()&&scopes->poisoned())terminate_own(expected_code);}terminate_own(230);});
    }
    return 230;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{std::cerr<<"ptd scopes\n";ptd_scopes();std::cerr<<"cleanup retry\n";cleanup_retry();std::cerr<<"permissions\n";permissions_and_publication();
    int code=151;for(const auto* mode:{"--publish-failure","--alloc-unknown","--construct-unknown","--free-unknown","--ptd-mutation"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child retains poisoned publication/unknown allocation/constructor/free/mutation outcome");
    std::cout<<"PASS: "<<b18_test::checks<<" foreign callback core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
