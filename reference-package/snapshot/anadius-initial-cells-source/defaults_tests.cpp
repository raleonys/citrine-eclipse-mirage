#include "defaults.hpp"
#include "worker.hpp"
#include "../anadius-host-platform-source/assembly.hpp"
#include "../anadius-resources-factory-source/factory.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <atomic>
#include <cstdlib>
#include <new>

std::atomic<bool> fail_next_initial_cell_allocation=false;
void* operator new(std::size_t n){if(fail_next_initial_cell_allocation.exchange(false))throw std::bad_alloc();if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
namespace {
using namespace anadius;using b18_test::require;using b18_test::rejects;
void fresh_values(){
    auto a=initial_cells::make_callback_defaults();
    require(a.client.empty()&&a.language.empty()&&a.fabricated.empty()&&a.ipv4_length==16&&a.ipv6_length==28&&!a.attempts,"Fresh callback projection matches three empty native set constructors and PE length/counter cells");
    a.client.insert(1);a.language.insert(2);a.fabricated.insert(3);a.attempts=7;a.ipv4_length=99;
    const auto b=initial_cells::make_callback_defaults();
    require(b.client.empty()&&b.language.empty()&&b.fabricated.empty()&&!b.attempts&&b.ipv4_length==16&&a.client.contains(1)&&a.attempts==7&&a.ipv4_length==99,"Factories return independent values without resetting existing handles or attempts");
    auto x=initial_cells::make_license_attempt_defaults();require(!x.attempted_files&&!x.blocked,"Fresh licensing projection matches zero image count and block byte");x.attempted_files=2;x.blocked=1;
    const auto y=initial_cells::make_license_attempt_defaults();require(!y.attempted_files&&!y.blocked&&x.attempted_files==2&&x.blocked==1,"Fresh license defaults neither share mutable cells nor reset prior blocked state");
}
void select_and_failure(){
    b18_test::Fixture f;host_platform::AssemblyInputs host;resources_factory::Inputs resources;
    rejects<std::invalid_argument>([&]{initial_cells::select_defaults(host,resources);},"Missing Domain rejects without selecting either root");require(!host.seed&&!resources.effects.attempts,"Missing Domain leaves destinations empty");
    host.domain=f.heap.domain;rejects<std::invalid_argument>([&]{initial_cells::select_defaults(host,resources);},"Absent worker Domain cannot match host Domain");require(!host.seed&&!resources.effects.attempts,"Domain mismatch leaves both roots empty");resources.domain=f.heap.domain;
    host.seed=initial_cells::make_callback_defaults();host.seed->client.insert(0x123);host.seed->attempts=9;
    rejects<std::invalid_argument>([&]{initial_cells::select_defaults(host,resources);},"Existing callback seed cannot be overwritten");require(host.seed->client.contains(0x123)&&host.seed->attempts==9&&!resources.effects.attempts,"Rejected seed replacement retains exact caller state");host.seed.reset();
    auto prior=std::make_shared<licensing_effects::AttemptState>(licensing_effects::AttemptState{2,1});resources.effects.attempts=prior;
    rejects<std::invalid_argument>([&]{initial_cells::select_defaults(host,resources);},"Existing attempt owner cannot be overwritten");require(!host.seed&&resources.effects.attempts==prior&&prior->blocked==1,"Rejected attempt replacement retains identity and blocked byte");resources.effects.attempts.reset();
    fail_next_initial_cell_allocation=true;bool failed=false;try{initial_cells::select_defaults(host,resources);}catch(const std::bad_alloc&){failed=true;}
    require(failed&&!fail_next_initial_cell_allocation&&!host.seed&&!resources.effects.attempts,"Actual attempt-owner allocation failure publishes neither callback nor license root");
    host.callbacks.memory=f.memory;initial_cells::select_defaults(host,resources);const auto selected=resources.effects.attempts;
    require(host.seed&&host.seed->ipv4_length==16&&host.seed->ipv6_length==28&&!host.seed->attempts&&selected&&!selected->attempted_files&&!selected->blocked,"Same-Domain fresh selection publishes both observed roots passively");
    selected->attempted_files=1;host.seed->attempts=4;rejects<std::invalid_argument>([&]{initial_cells::select_defaults(host,resources);},"Repeated selection is rejected");require(resources.effects.attempts==selected&&selected->attempted_files==1&&host.seed->attempts==4,"Repeated selection preserves already-mutated source cells");
}
void worker_fresh_state(){
    auto a=initial_cells::make_worker_defaults();require(a.saved_avatar_size==-1&&a.achievement_file.empty(),"Worker starts at observed signed avatar sentinel -1 and empty legacy path placeholder");
    a.saved_avatar_size=2;a.achievement_file=u"own-existing.xml";const auto b=initial_cells::make_worker_defaults();require(b.saved_avatar_size==-1&&b.achievement_file.empty()&&a.saved_avatar_size==2&&a.achievement_file==u"own-existing.xml","Worker factory returns fresh state without resetting another avatar/path owner");
    resources_factory::Inputs inputs;rejects<std::invalid_argument>([&]{initial_cells::select_worker_defaults(inputs);},"Worker defaults require a Ready Domain");require(!inputs.worker.state,"Rejected Worker default selection leaves state absent");
    b18_test::Fixture fixture;inputs.domain=fixture.heap.domain;inputs.worker.state=application_worker::Settings::StateSeed{0,u"custom.xml"};
    rejects<std::invalid_argument>([&]{initial_cells::select_worker_defaults(inputs);},"Explicit zero or custom Worker path cannot be silently replaced");require(inputs.worker.state->saved_avatar_size==0&&inputs.worker.state->achievement_file==u"custom.xml","Rejected selection retains exact existing Worker values");inputs.worker.state.reset();
    unsigned configured=0;auto owner=std::make_shared<int>(17);inputs.worker.lifetime=owner;inputs.worker.configure=[&](auto&){++configured;};initial_cells::select_worker_defaults(inputs);
    require(inputs.worker.state&&inputs.worker.state->saved_avatar_size==-1&&inputs.worker.state->achievement_file.empty()&&inputs.worker.lifetime==owner&&inputs.worker.configure&&!configured,"Passive Worker selection binds only fresh state and preserves callback/lifetime owners");
    inputs.worker.state->saved_avatar_size=0;rejects<std::invalid_argument>([&]{initial_cells::select_worker_defaults(inputs);},"Selected Worker state cannot be selected again");require(inputs.worker.state->saved_avatar_size==0&&!configured,"Repeated selection does not reset already-mutated avatar state or execute configure");
}
}
int main(){try{fresh_values();select_and_failure();worker_fresh_state();std::cout<<"PASS: "<<b18_test::checks<<" initial-cell checks; fresh independent values, no overwrite and allocation-atomic selection\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
