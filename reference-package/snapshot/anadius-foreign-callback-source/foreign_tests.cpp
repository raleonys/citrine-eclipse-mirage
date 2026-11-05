#include "foreign_test_fixture.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-original-provider-source/originals.hpp"

namespace {
using namespace b18_test;
bool registry_nested_denied(fc::Admission& admission){
    n::RegistryNestedFrame frame;
    try{if(!admission.try_enter_registry_nested(n::Callback::reg_query_a_00fac0,frame))return true;}
    catch(const fc::Rejected&){return true;}
    admission.leave_registry_nested(frame);return false;
}
void registry_sdk_frames(){
    Fixture f;std::function<void()> on_proof;std::optional<n::Callback> deny;
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},[&](n::Callback c){if(on_proof)on_proof();return !deny||*deny!=c;});
    fc::Admission admission(domain);admission.host_setup_begin();const auto fls=f.heap.domain->native_fls_index();
    admission.enter(n::Callback::reg_open_a_00f800);const auto ptd=::FlsGetValue(fls);
    require(ptd&&domain->permits_current()&&registry_nested_denied(admission),"General body cannot nest before its exact typed SDK interval");
    n::RegistrySdkFrame outer,inner,unsupported;
    require(!admission.begin_registry_sdk(n::Callback::folder_a_00fc00,unsupported),"Nonregistry SDK calls do not select general registry nesting");
    ::SetLastError(0x8110);require(admission.begin_registry_sdk(n::Callback::reg_open_a_00f800,outer)&&::GetLastError()==0x8110,"Root SDK metadata preserves incoming LastError");
    n::RegistryNestedFrame nonregistry;require(!admission.try_enter_registry_nested(n::Callback::folder_a_00fc00,nonregistry),"Registry SDK interval does not admit an unrelated general callback");
    bool checking_denied=false;on_proof=[&]{checking_denied=registry_nested_denied(admission)&&!domain->permits_current()&&!domain->permits_original_current();};
    n::RegistryNestedFrame child;require(admission.try_enter_registry_nested(n::Callback::reg_open_a_00f800,child),"Same-slot nested general body can enter from its parent SDK interval");
    on_proof={};require(checking_denied&&child.parent==nullptr&&child.sdk==&outer&&child.admitted&&domain->permits_current()&&::FlsGetValue(fls)==ptd,"Child owns exact SDK parent, checks publication with permissions masked and borrows root PTD");
    require(registry_nested_denied(admission),"Ancestor SDK marker cannot admit reentry from a child body with the same callback enum");
    in_thread([&]{require(registry_nested_denied(admission),"SDK marker on another thread grants no nested admission");rejects<fc::Rejected>([&]{admission.enter(n::Callback::reg_query_a_00fac0);},"General root remains active and rejects foreign overlap while descendants run");});
    require(admission.begin_registry_sdk(n::Callback::reg_open_a_00f800,inner)&&inner.body==&child,"Nested SDK marker binds exact current body identity");
    n::RegistryNestedFrame leaf;require(admission.try_enter_registry_nested(n::Callback::reg_query_w_00fb60,leaf)&&leaf.parent==&child&&leaf.sdk==&inner,"A child SDK interval can admit the next distinct registry body");
    require(::FlsGetValue(fls)==ptd&&domain->permits_current(),"Deepest general body shares the exact retained root PTD");
    admission.leave_registry_nested(leaf);admission.end_registry_sdk(inner);require(registry_nested_denied(admission),"Ending child SDK interval restores child body with ancestor SDK marker still masked");
    admission.leave_registry_nested(child);require(domain->permits_current()&&::FlsGetValue(fls)==ptd,"Exact child leave restores parent body/PTD before parent SDK completes");
    deny=n::Callback::reg_query_w_00fb60;n::RegistryNestedFrame absent;
    rejects<fc::Rejected>([&]{admission.try_enter_registry_nested(*deny,absent);},"Missing selected hook receipt rejects nested body without losing parent SDK interval");deny.reset();
    require(!absent.owner&&!absent.admitted&&domain->permits_current(),"Known missing receipt clears failed child and restores exact parent");
    ::SetLastError(0x8120);admission.end_registry_sdk(outer);require(::GetLastError()==0x8120&&registry_nested_denied(admission),"Exact SDK leave preserves outgoing LastError and removes nesting permission");
    admission.leave(n::Callback::reg_open_a_00f800);require(!domain->active(),"Only root leave retires its callback PTD ownership");
    n::OriginalForwardFrame pure;require(admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,pure),"Controlled pure-forward parent enters its existing route");
    n::RegistrySdkFrame pure_sdk;require(!admission.begin_registry_sdk(n::Callback::reg_open_a_00f800,pure_sdk)&&registry_nested_denied(admission),"Pure-forward SDK parent is excluded from general registry nesting");admission.leave_original_forward(pure);
    admission.enter(n::Callback::connect_00fd00);n::RegistrySdkFrame connect_sdk;require(!admission.begin_registry_sdk(n::Callback::connect_00fd00,connect_sdk)&&registry_nested_denied(admission),"Connect SDK parent does not grant general registry nesting");admission.leave(n::Callback::connect_00fd00);
    require(admission.pause_before_cleanup(),"All SDK/body frame metadata drains through existing pause");
}
void original_forward_frames(){
    Fixture f;bool published=true;std::optional<n::Callback> denied;std::function<void()> on_proof;
    auto cooperative=std::make_shared<fc::Domain>(f.heap.domain,[]{return true;});fc::Admission cooperative_admission(cooperative);n::OriginalForwardFrame declined;
    require(!cooperative_admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,declined)&&!declined.owner&&!declined.admitted,"Cooperative profile declines pure forwarding without changing the token");
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},[&](n::Callback callback){if(on_proof)on_proof();return published&&(!denied||*denied!=callback);});fc::Admission admission(domain);
    auto& port=f.driver.configuration.server_port;domain->bind_connect_server_port(port);admission.host_setup_begin();
    n::OriginalForwardFrame missing;published=false;
    rejects<fc::Rejected>([&]{admission.try_enter_original_forward(n::Callback::folder_w_00fc80,missing);},"Pure route requires its own successful hook receipt");
    require(!missing.owner&&!missing.admitted&&!domain->active(),"Known missing pure receipt unwinds its pending count and token");published=true;
    denied=n::Callback::folder_w_00fc80;n::OriginalForwardFrame allowed,denied_frame;
    require(admission.try_enter_original_forward(n::Callback::reg_open_w_00f960,allowed),"A different hook receipt may admit its own pure route");
    rejects<fc::Rejected>([&]{admission.try_enter_original_forward(n::Callback::folder_w_00fc80,denied_frame);},"Another published hook cannot stand in for the requested pure callback receipt");
    require(allowed.admitted&&!denied_frame.admitted&&!denied_frame.owner&&domain->permits_original_current(),"Failed nested per-hook proof restores the admitted parent frame");admission.leave_original_forward(allowed);denied.reset();
    for(auto callback:{n::Callback::reg_close_00f5b0,n::Callback::reg_query_a_00fac0,n::Callback::reg_query_w_00fb60,n::Callback::connect_00fd00}){
        n::OriginalForwardFrame unsupported;require(!admission.try_enter_original_forward(callback,unsupported)&&!unsupported.owner&&!unsupported.admitted,"Close/query/connect do not select the six pure forwarding callbacks");
    }
    const auto fls=f.heap.domain->native_fls_index();auto* prior=::FlsGetValue(fls);
    admission.enter(n::Callback::reg_close_00f5b0);require(domain->permits_current(),"Controlled Full parent begins with its ordinary permission");
    bool proof_masked=true,nested_from_check=false;
    on_proof=[&]{
        proof_masked=proof_masked&&!domain->permits_current()&&!domain->permits_original_current()&&!domain->permits_connect_wait_current()&&!domain->permits_connect_body_current();
        if(!nested_from_check){nested_from_check=true;n::OriginalForwardFrame nested_check;const bool entered=admission.try_enter_original_forward(n::Callback::folder_a_00fc00,nested_check);proof_masked=proof_masked&&entered&&domain->permits_original_current()&&!domain->permits_current();if(entered)admission.leave_original_forward(nested_check);proof_masked=proof_masked&&!domain->permits_original_current()&&!domain->permits_current();}
    };
    n::OriginalForwardFrame outer,inner;::SetLastError(0x6811);
    require(admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,outer)&&::GetLastError()==0x6811,"Pure frame metadata preserves entry LastError");
    on_proof={};require(proof_masked&&nested_from_check,"Checking frame masks Full and can admit an independent nested pure frame without an SDK-phase gate");
    require(outer.owner==domain.get()&&outer.parent==nullptr&&outer.admitted&&outer.thread==::GetCurrentThreadId()&&outer.fiber==::GetCurrentFiber()&&outer.callback==n::Callback::reg_open_a_00f800,"Selected frame retains exact owner, actor, callback and stack identity");
    require(domain->permits_original_current()&&!domain->permits_current()&&!domain->permits_connect_wait_current()&&!domain->permits_connect_body_current()&&::FlsGetValue(fls)==prior,"Pure frame masks Full permission and borrows the existing actor without PTD replacement");
    rejects<std::logic_error>([&]{admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,outer);},"An admitted stack token cannot be reused");
    require(admission.try_enter_original_forward(n::Callback::create_process_w_00f4d0,inner)&&inner.parent==&outer,"Nested pure frame links to the exact existing stack top");
    rejects<fc::Rejected>([&]{admission.enter(n::Callback::reg_close_00f5b0);},"Ordinary body cannot enter through a nested pure original-only scope");
    ::SetLastError(0x6822);admission.leave_original_forward(inner);require(::GetLastError()==0x6822&&!inner.admitted&&!inner.owner&&outer.admitted&&domain->permits_original_current(),"Exact inner leave restores parent original scope and outgoing LastError");
    admission.leave_original_forward(outer);require(!outer.admitted&&!outer.owner&&domain->permits_current()&&::FlsGetValue(fls)==prior,"Outermost pure leave restores Full permission and original PTD identity");admission.leave(n::Callback::reg_close_00f5b0);
    require(admission.try_enter_connect_original(port,4000,true),"Controlled connect parent owns its wait phase");
    n::OriginalForwardFrame during_wait;require(admission.try_enter_original_forward(n::Callback::folder_a_00fc00,during_wait)&&!domain->permits_connect_wait_current()&&domain->permits_original_current(),"Nested pure frame masks an outer connect wait phase");
    require(!admission.connect_original_wait_pending()&&!admission.connect_original_redirect_pending(),"Pure top suppresses outer connect phase queries");
    rejects<std::logic_error>([&]{admission.complete_connect_original_wait();},"Pure top cannot mutate the suspended connect wait phase");
    admission.leave_original_forward(during_wait);require(domain->permits_connect_wait_current()&&!domain->permits_original_current(),"Pure leave restores the exact connect wait phase");
    f.driver.execution.wait_steps_036880(-1);admission.complete_connect_original_wait();
    n::OriginalForwardFrame during_sdk;require(admission.try_enter_original_forward(n::Callback::reg_open_w_00f960,during_sdk)&&domain->permits_original_current(),"Pure frame can nest above an admitted connect SDK phase");admission.leave_original_forward(during_sdk);admission.leave(n::Callback::connect_00fd00);
    in_thread([&]{for(auto callback:{n::Callback::reg_open_a_00f800,n::Callback::reg_open_w_00f960,n::Callback::create_process_a_00f3f0,n::Callback::create_process_w_00f4d0,n::Callback::folder_a_00fc00,n::Callback::folder_w_00fc80}){
        n::OriginalForwardFrame frame;require(!::FlsGetValue(fls)&&admission.try_enter_original_forward(callback,frame)&&domain->permits_original_current()&&!domain->permits_current()&&!::FlsGetValue(fls),"Each selected pure callback admits a foreign actor without creating private PTD");admission.leave_original_forward(frame);
    }});
    n::OriginalForwardFrame held;require(admission.try_enter_original_forward(n::Callback::folder_w_00fc80,held)&&!admission.pause_before_cleanup()&&domain->active(),"Pure pending count prevents a paused domain from reporting drain");
    n::OriginalForwardFrame stopped;rejects<fc::Rejected>([&]{admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,stopped);},"Pause excludes new pure frames while the held frame retains its SDK permission");
    require(domain->permits_original_current(),"Pause preserves an already admitted pure frame");admission.leave_original_forward(held);require(admission.pause_before_cleanup()&&!domain->active(),"Exact held-frame leave drains the paused domain");
}
void original_hook_publication(){
    Fixture f;auto publication=std::make_shared<original_provider::Publication>(std::make_shared<int>(1));n::OriginalSlots slots{};publication->bind_slots(slots);
    fc::HookPublicationProof proof=[publication](n::Callback callback){const auto state=publication->snapshot();const auto index=host_init_recovered::slot(callback);return index<host_init_recovered::callback_count&&(state.phase==original_provider::PublicationPhase::Setup||state.phase==original_provider::PublicationPhase::Ready)&&(state.published_mask&(1u<<index));};
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},std::move(proof));fc::Admission admission(domain);
    admission.host_setup_begin();publication->setup_begin();
    in_thread([&]{rejects<fc::Rejected>([&]{admission.enter(n::Callback::folder_w_00fc80);},"Unpublished original hook rejects before PTD/body admission");require(!domain->active(),"Known missing publication releases the source admission reservation");});
    constexpr auto callback=n::Callback::folder_w_00fc80;auto& output=slots[host_init_recovered::slot(callback)];publication->before_create(callback,0x111,output);output=0x222;publication->after_create(callback,0x111,output,0);
    HANDLE entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr),release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);require(entered&&release,"Own original-publication test events created");bool permitted=false;std::exception_ptr failure;
    std::thread thread([&]{try{admission.enter(callback);permitted=domain->permits_current();::SetEvent(entered);const auto waited=::WaitForSingleObject(release,10000);admission.leave(callback);if(waited!=WAIT_OBJECT_0)throw std::runtime_error("Original-publication fixture release timeout");}catch(...){failure=std::current_exception();::SetEvent(entered);}});
    const bool observed=::WaitForSingleObject(entered,10000)==WAIT_OBJECT_0;
    if(observed){publication->setup_complete();admission.host_setup_complete();}
    ::SetEvent(release);thread.join();::CloseHandle(entered);::CloseHandle(release);if(failure)std::rethrow_exception(failure);
    require(observed&&permitted&&domain->ready(),"Host completion proceeds while an actual published Full callback is active; no all-worker readiness proof is substituted");
    require(admission.pause_before_cleanup()&&!domain->active(),"Original admission still seals actual callback bodies");publication->stop();
}
void original_web_lane_selection(){
    Fixture f;auto publication=std::make_shared<original_provider::Publication>(std::make_shared<int>(1));n::OriginalSlots slots{};publication->bind_slots(slots);
    auto& port=f.driver.configuration.server_port;port=0;bool grant=true;
    fc::HookPublicationProof proof=[publication,&grant](n::Callback callback){const auto state=publication->snapshot();const auto index=host_init_recovered::slot(callback);return grant&&index<10&&(state.phase==original_provider::PublicationPhase::Setup||state.phase==original_provider::PublicationPhase::Ready)&&(state.published_mask&(1u<<index));};
    rejects<std::invalid_argument>([&]{(void)std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},[]{return std::optional<std::uint16_t>{80};},proof);},"Original live port cannot mix a Cooperative snapshot provider");
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},std::move(proof));fc::Admission admission(domain);
    rejects<std::logic_error>([&]{admission.try_enter_connect_original(port,80);},"Original connect cannot reserve before its exact live cell is bound");
    rejects<std::logic_error>([&]{admission.try_enter_connect_original(port);},"Untouched Original admission also requires its Context cell identity before publication");
    require(!domain->active(),"Missing cell binding leaves no counted reservation");
    in_thread([&]{rejects<std::logic_error>([&]{domain->bind_connect_server_port(port);},"Only controller may bind Original live cell");});
    domain->bind_connect_server_port(port);
    rejects<std::logic_error>([&]{domain->bind_connect_server_port(port);},"Live cell identity binds once even when the same address is supplied again");
    std::uint16_t other=0;
    rejects<std::logic_error>([&]{domain->bind_connect_server_port(other);},"Live cell cannot be rebound to a different address");
    admission.host_setup_begin();publication->setup_begin();
    const auto callback=n::Callback::connect_00fd00;publication->before_create(callback,0x111,slots[0]);slots[0]=0x222;publication->after_create(callback,0x111,slots[0],0);
    admission.enter(callback);
    in_thread([&]{
        auto finish_body=[&]{
            require(admission.connect_original_redirect_pending()&&domain->permits_connect_body_current()&&!domain->permits_original_current()&&!domain->permits_connect_wait_current()&&!domain->permits_current(),"Redirect decision grants only its exact body without returning a scalar copy");
            admission.complete_connect_original_redirect();
            require(domain->permits_original_current()&&!domain->permits_connect_body_current(),"Completed redirect transitions to original-only permission");admission.leave(callback);
        };
        rejects<std::logic_error>([&]{admission.try_enter_connect_original(other,80);},"Admission must receive the bound live Context cell, not an equal scalar copy");
        rejects<std::logic_error>([&]{admission.try_enter_connect_original(other);},"Untouched route cannot bypass exact live-cell identity with an equal copy");
        require(admission.try_enter_connect_original(port)&&domain->permits_original_current()&&!domain->permits_current()&&!admission.connect_original_wait_pending()&&!admission.connect_original_redirect_pending(),"Untouched route gains only original permission without a scalar decision");admission.leave(callback);
        ::SetLastError(0x5133);require(admission.try_enter_connect_original(port,80)&&::GetLastError()==0x5133&&!admission.connect_original_redirect_pending()&&domain->permits_original_current(),"Cold live zero port immediately selects HTTP passthrough with metadata LastError preserved");admission.leave(callback);
        port=80;
        require(admission.try_enter_connect_original(port,80)&&!admission.connect_original_wait_pending(),"Matching web port selects redirect without adding a native startup wait");port=443;finish_body();
        require(!admission.try_enter_connect_original(port,3216),"Special redirect port3216 cannot request the web-only source lane");
        port=80;require(admission.try_enter_connect_original(port,443)&&domain->permits_original_current()&&!domain->permits_current()&&!::FlsGetValue(f.heap.domain->native_fls_index()),"Different live web port retries on the same thread with SDK-only permission and no private PTD");
        admission.leave(callback);require(!domain->permits_original_current(),"Web SDK permission ends with its exact source leave");
        require(admission.try_enter_connect_original(port,3216,true)&&!domain->permits_connect_body_current(),"3216 enters wait-only before unconditional redirect selection");
        f.driver.execution.wait_steps_036880(-1);port=0;admission.complete_connect_original_wait();finish_body();
        port=80;require(admission.try_enter_connect_original(port,4000,true),"Non-web decision is deferred until after its actual wait");
        f.driver.execution.wait_steps_036880(-1);port=4000;admission.complete_connect_original_wait();port=81;finish_body();
        port=4000;require(admission.try_enter_connect_original(port,4000,true)&&admission.connect_original_wait_pending()&&domain->permits_connect_wait_current()&&!domain->permits_original_current()&&!domain->permits_current(),"Non-web port initially grants only startup wait permission");
        port=80;
        f.driver.execution.wait_steps_036880(-1);admission.complete_connect_original_wait();
        require(!admission.connect_original_wait_pending()&&!domain->permits_connect_wait_current()&&domain->permits_original_current(),"Actual recovered wait return compares the changed live scalar and selects original-only permission");
        admission.leave(callback);
        port=0;require(admission.try_enter_connect_original(port,4000,true),"Cold non-web entry uses its live cell without a Server milestone");
        f.driver.execution.wait_steps_036880(-1);
        grant=false;rejects<fc::Rejected>([&]{admission.complete_connect_original_wait();},"Lost actual hook receipt after wait cannot fabricate a completed route");
        require(!domain->permits_connect_wait_current()&&!domain->permits_original_current()&&!domain->permits_connect_body_current()&&!domain->permits_current(),"Failed wait checkpoint retains no provider authority until leave");admission.leave(callback);
        grant=true;port=0;require(admission.try_enter_connect_original(port,3216,true),"Cold special-port entry is also admitted only for waiting");
        port=4660;f.driver.execution.wait_steps_036880(-1);admission.complete_connect_original_wait();finish_body();
        port=80;require(admission.try_enter_connect_original(port,80),"New web body can be admitted after a prior failed lane leaves");
        grant=false;rejects<fc::Rejected>([&]{admission.complete_connect_original_redirect();},"Lost hook receipt after body cannot grant SDK permission");
        require(!domain->permits_connect_body_current()&&!domain->permits_original_current()&&!domain->permits_connect_wait_current(),"Failed body checkpoint leaves the actor permissionless");admission.leave(callback);
    });
    require(domain->permits_current(),"Concurrent web selection does not replace the original body's PTD permission");
    admission.leave(callback);require(admission.pause_before_cleanup()&&!domain->active(),"Rejected and accepted web reservations leave no active-count residue");publication->stop();
}
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
void staged_connect_permissions(){
    Fixture f;bool published=false,web_ready=false;std::uint16_t bound_port=0;
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,[]{return false;},[&]{return published;},[&](std::uint16_t requested){return web_ready&&requested!=bound_port;});fc::Admission admission(domain);
    auto resources=f.resources();resources.callback_scope=[domain]{return domain->permits_current();};
    resources.original_scope=[domain]{return domain->permits_original_current();};nc::CallbackSeed seed=b15_test::seed();
    callback_policy::Policy policy({f.driver.view(),seed},std::move(resources));admission.host_setup_begin();
    in_thread([&]{
        admission.enter(n::Callback::connect_00fd00);
        require(admission.connect_probe()&&!domain->permits_current()&&!domain->permits_original_current(),"warming probe grants neither body nor original permission");
        rejects<fc::Rejected>([&]{admission.connect_classified(true);},"successful-create proof remains required for untouched route");
        require(admission.connect_probe(),"known false original proof leaves a releasable probe");
        published=true;::SetLastError(0x7712);admission.connect_classified(true);
        require(::GetLastError()==0x7712&&!domain->permits_current()&&domain->permits_original_current(),"original-only scope retains PTD and error without full Policy permission");
        rejects<std::logic_error>([&]{policy.log_finish_0088e0();},"original-only scope cannot log through full Policy");
        rejects<std::logic_error>([&]{policy.wait_steps_036880(-1);},"original-only scope cannot enter Execution wait");
        rejects<fc::Rejected>([&]{admission.enter(n::Callback::connect_00fd00);},"staged route still rejects same-thread reentry");
        admission.leave(n::Callback::connect_00fd00);
    });
    in_thread([&]{
        admission.enter(n::Callback::connect_00fd00);
        require(!admission.connect_web_passthrough(80),"missing published port cannot grant HTTP SDK permission");
        rejects<fc::Rejected>([&]{admission.connect_classified(false);},"recognized family remains warming even with published original");
        require(!domain->permits_current()&&!domain->permits_original_current(),"rejected body did not gain SDK permission");
        admission.leave(n::Callback::connect_00fd00);
        rejects<fc::Rejected>([&]{admission.enter(n::Callback::reg_close_00f5b0);},"other warming foreign callbacks keep existing rejection");
    });
    for(std::uint16_t bound:{0,80,443,65535})for(std::uint16_t requested:{80,443,3216}){
        bound_port=bound;web_ready=true;
        in_thread([&]{
            admission.enter(n::Callback::connect_00fd00);const bool accepted=admission.connect_web_passthrough(requested);
            require(accepted==((requested==80||requested==443)&&requested!=bound),"web original-only decision excludes redirect and non-web wait branches");
            require(!domain->permits_current()&&domain->permits_original_current()==accepted,"web decision never grants general Policy permission");
            admission.leave(n::Callback::connect_00fd00);
        });
    }
    require(!domain->ready()&&domain->pause(),"staged success never promotes global readiness and can drain");
}
void staged_wait_permissions(){
    Fixture f;bool published=false;
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,[]{return false;},[&]{return published;},std::function<bool(std::uint16_t)>{},[](std::uint16_t port){return port!=4660;});
    fc::Admission admission(domain);admission.host_setup_begin();
    auto resources=f.resources();resources.callback_scope=[domain]{return domain->permits_current();};resources.original_scope=[domain]{return domain->permits_original_current();};
    resources.connect_wait_scope=[domain]{return domain->permits_connect_wait_current();};nc::CallbackSeed seed=b15_test::seed();
    callback_policy::Policy policy({f.driver.view(),seed},std::move(resources));
    admission.enter(n::Callback::connect_00fd00);require(!admission.connect_wait_begin(4000),"controller warming is never given a new blocking wait route");admission.leave(n::Callback::connect_00fd00);
    in_thread([&]{
        admission.enter(n::Callback::connect_00fd00);
        for(std::uint16_t port:{80,443,3216,4660})require(!admission.connect_wait_begin(port),"wait route excludes web and redirect ports");
        require(!admission.connect_wait_begin(4000),"wait route still requires successful original publication");published=true;
        require(!admission.connect_wait_begin(4660),"valid original publication does not admit a matching server-port redirect");
        ::SetLastError(0x7744);require(admission.connect_wait_begin(4000)&&::GetLastError()==0x7744,"eligible foreign wait reservation preserves metadata LastError");
        require(domain->permits_connect_wait_current()&&!domain->permits_original_current()&&!domain->permits_current(),"waiting scope grants only network Execution wait");
        rejects<std::logic_error>([&]{policy.log_finish_0088e0();},"waiting cannot log");
        rejects<std::logic_error>([&]{policy.wait_036880(-1);},"waiting cannot use registry effect route");
        rejects<std::logic_error>([&]{policy.wait_steps_036880(1);},"waiting cannot substitute a different step count");
        policy.wait_steps_036880(-1);admission.connect_wait_complete();
        require(!domain->permits_connect_wait_current()&&domain->permits_original_current()&&!domain->permits_current(),"normal wait completion closes wait permission and grants only original");
        rejects<std::logic_error>([&]{policy.wait_steps_036880(-1);},"completed original-only scope cannot repeat a startup wait");
        admission.leave(n::Callback::connect_00fd00);
    });
    in_thread([&]{
        admission.enter(n::Callback::connect_00fd00);require(admission.connect_wait_begin(4000),"second controlled wait starts from a known published receipt");
        policy.wait_steps_036880(-1);published=false;
        rejects<fc::Rejected>([&]{admission.connect_wait_complete();},"completed wait cannot reuse revoked original proof");
        require(!domain->permits_original_current()&&!domain->permits_connect_wait_current(),"failed post-wait proof grants neither SDK nor another wait");
        admission.leave(n::Callback::connect_00fd00);
    });
    require(domain->pause()&&!domain->ready(),"completed wait never activates global readiness");
}
void redirect_permissions(){
    Fixture f;std::optional<std::uint16_t> snapshot;
    auto domain=std::make_shared<fc::Domain>(f.heap.domain,[]{return false;},[]{return true;},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},[&]{return snapshot;});
    rejects<std::invalid_argument>([&]{fc::make_factory(domain,b15_test::seed(),f.resources(),b15_test::permanent_errors,[]{return true;});},"early redirect factory rejects unmanaged log before binding");
    auto buffer=std::make_shared<std::stringbuf>();auto resources=f.resources();resources.stream=shared_log::make_stream(buffer);
    resources.callback_scope=[domain]{return domain->permits_current();};resources.original_scope=[domain]{return domain->permits_original_current();};
    resources.connect_wait_scope=[domain]{return domain->permits_connect_wait_current();};resources.connect_body_scope=[domain]{return domain->permits_connect_body_current();};
    nc::CallbackSeed seed=b15_test::seed();callback_policy::Policy policy({f.driver.view(),seed},std::move(resources));fc::Admission admission(domain);admission.host_setup_begin();
    admission.enter(n::Callback::connect_00fd00);require(!admission.connect_redirect_begin(3216),"controller warming redirect remains excluded");admission.leave(n::Callback::connect_00fd00);
    in_thread([&]{
        admission.enter(n::Callback::connect_00fd00);require(!admission.connect_redirect_begin(3216),"redirect needs actual published port snapshot");snapshot=4660;
        require(admission.connect_redirect_begin(3216)&&domain->permits_connect_wait_current()&&!domain->permits_connect_body_current(),"non-web redirect initially grants only wait");
        policy.wait_steps_036880(-1);auto port=admission.connect_redirect_ready();
        require(port==4660&&domain->permits_connect_body_current()&&!domain->permits_current()&&!domain->permits_original_current()&&!domain->permits_connect_wait_current(),"post-wait body has isolated network permission");
        rejects<std::logic_error>([&]{policy.allocate_4_04b0bc();},"redirect cannot allocate through general Policy");
        rejects<std::logic_error>([&]{policy.finish_log_0088e0();},"redirect cannot invoke registry flush route");
        rejects<std::logic_error>([&]{policy.original_connect_0a3838(1,{},-1);},"redirect body cannot call original before completion checkpoint");
        std::array<std::byte,4> bytes{std::byte{2},std::byte{0},std::byte{0x0c},std::byte{0x90}};std::uint32_t attempts=0;
        network_hook_recovered::redirect_body_00fd00(policy,bytes,3216,port,attempts);
        require(attempts==1&&bytes[2]==std::byte{0x12}&&bytes[3]==std::byte{0x34},"permitted network body logs/counts/stores published port");
        admission.connect_redirect_complete();require(domain->permits_original_current()&&!domain->permits_connect_body_current(),"body completion closes effects and grants original only");
        admission.leave(n::Callback::connect_00fd00);
        snapshot=80;admission.enter(n::Callback::connect_00fd00);require(admission.connect_redirect_begin(80)&&!domain->permits_connect_wait_current(),"matching HTTP redirect adds no native wait");
        snapshot=443;rejects<fc::Rejected>([&]{admission.connect_redirect_ready();},"changed port snapshot cannot enter redirect body");admission.leave(n::Callback::connect_00fd00);
    });
    require(buffer->str()=="game connected on port 3216; redirecting to 4660\n"&&f.driver.probe->connects==0&&domain->pause(),"body metadata/core scenario invokes no original SDK");
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);Fixture f;
    if(mode=="--forward-lifo"||mode=="--forward-actor"||mode=="--forward-thread"){
        auto domain=std::make_shared<fc::Domain>(f.heap.domain,std::function<bool()>{},std::function<bool()>{},std::function<bool(std::uint16_t)>{},std::function<bool(std::uint16_t)>{},std::function<std::optional<std::uint16_t>()>{},[](n::Callback){return true;});fc::Admission admission(domain);n::OriginalForwardFrame outer,inner;
        if(!admission.try_enter_original_forward(n::Callback::reg_open_a_00f800,outer))terminate_own(230);
        if(mode=="--forward-lifo"){if(!admission.try_enter_original_forward(n::Callback::folder_w_00fc80,inner))terminate_own(230);admission.leave_original_forward(outer);}
        else if(mode=="--forward-thread"){std::thread wrong_actor([&]{admission.leave_original_forward(outer);});wrong_actor.join();}
        else{outer.fiber=reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(outer.fiber)^std::uintptr_t{1});admission.leave_original_forward(outer);}
        terminate_own(230);
    }
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
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{std::cerr<<"ptd scopes\n";ptd_scopes();std::cerr<<"cleanup retry\n";cleanup_retry();std::cerr<<"permissions\n";permissions_and_publication();registry_sdk_frames();original_forward_frames();original_hook_publication();original_web_lane_selection();staged_connect_permissions();staged_wait_permissions();redirect_permissions();
    int code=151;for(const auto* mode:{"--publish-failure","--alloc-unknown","--construct-unknown","--free-unknown","--ptd-mutation"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child retains poisoned publication/unknown allocation/constructor/free/mutation outcome");
    for(const auto* mode:{"--forward-lifo","--forward-actor","--forward-thread"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==239,"Synthetic invalid frame leave rejects non-LIFO stack identity, mismatched thread or tampered fiber metadata before parent release");
    std::cout<<"PASS: "<<b18_test::checks<<" foreign callback core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
