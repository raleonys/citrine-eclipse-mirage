#include "observed_runtime.hpp"
#include "../anadius-initial-cells-source/runtime.hpp"
#include "runtime_test_fixture.hpp"
#include <optional>
#include <type_traits>

// Actual Runtime/SourceBackend construction and source entry/cleanup. The B4
// services and queued thread recorders are controlled external boundaries;
// this does not execute native OS workers, DLLs, real files or hook engines.
namespace {
using namespace anadius;namespace br=bootstrap_runtime;namespace bb=bootstrap_backend;
namespace h=host_init_recovered;namespace n=bootstrap_native_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
static_assert(!std::is_default_constructible_v<br::OutsideConstructorState>);
struct Probe {
    std::optional<bb::CallbackStateView> bound;
    unsigned factories=0,published=0,paused=0,closed=0,destroyed=0;
    std::string retired_username;
};
struct Attachment final:bb::CallbackAttachment {
    Probe& probe;bb::CallbackStateView view;
    Attachment(Probe& p,bb::CallbackStateView v):probe(p),view(v){}
    ~Attachment()override{++probe.destroyed;probe.retired_username=view.configuration.username;}
    void publish()override{++probe.published;}
    bool pause_before_cleanup()override{++probe.paused;return true;}
    bool close_after_cleanup()override{++probe.closed;return true;}
};
void observe(bb::Resources& resources,Probe& probe){
    resources.callback_attachment=[&probe](bb::CallbackStateView view){++probe.factories;probe.bound.emplace(view);return std::make_unique<Attachment>(probe,view);};
}
br::OutsideConstructorState outside(std::int64_t frequency,std::uintptr_t marker){
    h::Values values{{"external-cell",std::u16string(u"X\0Y",3)}};
    std::array<h::Address,h::callback_count> originals{};for(std::size_t i=0;i<originals.size();++i)originals[i]=marker+i;
    return {frequency,std::move(values),{marker+0x100,marker+0x200},originals};
}
void check_bound(b4_test::Fixture& fixture,Probe& probe,const br::OutsideConstructorState& expected){
    require(probe.factories==1&&probe.bound&&(&probe.bound->configuration==&fixture.runtime->configuration()),"Observed constructor uses actual stable Runtime configuration in SourceBackend bind_state");
    const auto& view=*probe.bound;const auto& seed=view.runtime;
    require(seed.frequency_cache==expected.frequency_cache&&seed.registry_values==expected.registry_values&&seed.enabled_targets==expected.enabled_targets&&seed.originals==expected.originals,"All mandatory outside-constructor cells reach actual Backend unchanged, including counted NUL registry values");
    require(!seed.server_ready&&!seed.online_ready&&!seed.libraries_ready&&!seed.online&&!seed.online_module&&!seed.online_object,"Observed constructor initializes ready/online/module/object cells to their recovered zeros");
    require(seed.execution_mutex.type==2&&seed.execution_mutex.owner_48==0xffffffffU&&!seed.execution_mutex.count_4c&&!seed.execution_mutex.storage_08&&!seed.execution_mutex.srw_10,"Observed constructor invokes actual type2 execution-mutex initialization");
    require(seed.libraries.loaded.empty()&&seed.libraries.from_main.empty()&&seed.libraries.from_worker.empty(),"Observed constructor starts native loaded storage and host path projections empty");
    const auto& config=fixture.runtime->configuration();const auto& console=fixture.runtime->console();
    require(config.username=="anadius"&&config.content_id=="0"&&config.sdk=="10,5,96,47512"&&config.game_attributes.at("AVAILABLE_VERSION")=="1.0.0.0","Observed constructor selects representative actual036d20 configuration defaults");
    require(!console.show_lsx&&!console.show_console&&!console.no_console,"Observed constructor selects all three recovered zero console cells");
    require(fixture.runtime->phase()==bootstrap_owner::Phase::Constructed&&!n::is_bound()&&!fixture.host.registrations&&!fixture.host.sets&&!fixture.threads->creations&&fixture.files->opened.empty()&&fixture.library_os->paths.empty()&&!fixture.online->loads&&!probe.published&&!probe.closed,"Constructor/binding remains passive before publication or source startup");
}
void passive_and_independent(){
    for(const auto marker:{std::uintptr_t{0x4110},std::uintptr_t{0x9220}}){
        Probe probe;b4_test::Fixture fixture;auto resources=fixture.resources();observe(resources,probe);const auto expected=outside(static_cast<std::int64_t>(marker)+123,marker);
        fixture.runtime=br::make_observed_runtime_036d20(expected,resources,b2_test::permanent_errors,b4_test::host_api());fixture.lifetime->state=&fixture.runtime->configuration();fixture.lifetime.reset();check_bound(fixture,probe,expected);
        require(!resources.configuration&&!resources.callback_attachment&&!resources.stop_sources,"Observed factory uses existing consuming Resources sink rather than retaining callable donors");
        require(fixture.runtime->close()&&fixture.stops==0&&fixture.api_state.waited.empty(),"Known never-published observed Runtime closes without starting/joining synthetic jobs");
        fixture.runtime.reset();require(probe.destroyed==1&&probe.retired_username=="anadius"&&fixture.destroyed_config=="anadius","Passive provider/backing retirement retains the actual default state through destructors");
    }
}
void controlled_startup_and_close(){
    Probe probe;b4_test::Fixture fixture;auto resources=fixture.resources();observe(resources,probe);fixture.online->found=false;fixture.config->mutate_persistence=false;
    fixture.files->base=R"("Config2" {"Game" {"Version" "base-version"} "Emulator" {"SDK" "1.2.3"} "User" {"Username" "observed-override"}})";fixture.files->over.clear();
    const auto expected=outside(10000000,0x1530);fixture.runtime=br::make_observed_runtime_036d20(expected,resources,b2_test::permanent_errors,b4_test::host_api());fixture.lifetime->state=&fixture.runtime->configuration();fixture.lifetime.reset();check_bound(fixture,probe,expected);
    fixture.runtime->publish();require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"Observed Runtime executes actual reconstructed Bootstrap configuration/startup over memory input");
    require(fixture.runtime->configuration().username=="observed-override"&&fixture.runtime->configuration().sdk=="1,2,3"&&fixture.runtime->configuration().content_id=="0","Partial synthetic CFG overrides selected defaults while untouched content ID survives");
    require(fixture.host.registrations==1&&fixture.threads->creations==3&&fixture.joins.size()==3&&probe.published==1,"Actual source attach registers exits and queues three controlled source workers");
    require(fixture.runtime->close()&&fixture.runtime->phase()==bootstrap_owner::Phase::Closed&&!n::is_bound()&&fixture.joins.empty(),"Actual source close joins all controlled entries and closes native binding");
    require(fixture.server->runs==1&&fixture.online->loads==1&&fixture.runtime->configuration().username=="observed-override","Controlled online missing-module result preserves configuration rather than overwriting from a profile");
    require(probe.paused==1&&probe.closed==1&&fixture.cleanup->uninitializations==1&&fixture.library_os->unloaded.empty(),"Existing attachment and exit cleanup routes run once without inventing initial loaded modules");
    fixture.runtime.reset();require(probe.destroyed==1&&probe.retired_username=="observed-override"&&fixture.destroyed_config=="observed-override","Source providers retire before owned aggregate storage is released");
}
void mandatory_and_legacy_contracts(){
    {
        b4_test::Fixture fixture;auto resources=fixture.resources();resources.console.reset();
        rejects<std::invalid_argument>([&]{(void)br::make_observed_runtime_036d20(outside(7,0x2000),resources,b2_test::permanent_errors,b4_test::host_api());},"Observed constructor still requires real mandatory Resources providers");
        require(!fixture.runtime&&!n::is_bound()&&!fixture.host.registrations&&!fixture.host.sets&&fixture.api_state.waited.empty(),"Incomplete provider rejection starts or publishes nothing");fixture.lifetime.reset();
    }
    {
        b4_test::Fixture fixture;auto resources=fixture.resources();fixture.construct(bb::take_resources(resources));
        require(fixture.runtime->configuration().username=="seed-user"&&fixture.runtime->configuration().sdk=="seed-sdk"&&fixture.runtime->configuration().content_id.empty(),"Existing explicit Runtime constructor preserves caller seeds instead of selecting observed defaults");
        require(fixture.runtime->close()&&fixture.stops==0,"Legacy explicit factory retains known-unpublished close behavior");
    }
}
void fresh_outside_and_owning_runtime(){
    auto first=initial_cells::make_outside_defaults();
    require(!first.frequency_cache&&first.registry_values.empty()&&first.enabled_targets.empty()&&std::all_of(first.originals.begin(),first.originals.end(),[](auto p){return p==0;}),"Fresh outside defaults match observed image cells and empty map constructor");
    first.frequency_cache=42;first.registry_values.emplace("own",u"value");first.enabled_targets.push_back(0x123);first.originals[4]=0x456;
    const auto second=initial_cells::make_outside_defaults();
    require(!second.frequency_cache&&second.registry_values.empty()&&second.enabled_targets.empty()&&!second.originals[4]&&first.frequency_cache==42&&first.registry_values.at("own")==u"value"&&first.enabled_targets[0]==0x123&&first.originals[4]==0x456,"Fresh factories never reset or share existing outside-cell state");
    Probe probe;b4_test::Fixture fixture;auto resources=fixture.resources();observe(resources,probe);unsigned frequencies=0;
    struct Counting final:execution_recovered::Platform {
        std::unique_ptr<execution_recovered::Platform> lower;unsigned& frequencies;
        Counting(std::unique_ptr<execution_recovered::Platform> p,unsigned& n):lower(std::move(p)),frequencies(n){}
        std::uint32_t current_thread_id()override{return lower->current_thread_id();}
        void acquire_srw_exclusive(std::uintptr_t& p)override{lower->acquire_srw_exclusive(p);}void release_srw_exclusive(std::uintptr_t& p)override{lower->release_srw_exclusive(p);}
        std::int64_t query_performance_frequency()override{++frequencies;return lower->query_performance_frequency();}
        std::int64_t query_performance_counter()override{return lower->query_performance_counter();}
        void sleep_milliseconds(std::uint32_t n)override{lower->sleep_milliseconds(n);}
        [[noreturn]]void raise_thread_error_04a648(int n)override{lower->raise_thread_error_04a648(n);std::terminate();}
    };
    resources.execution=std::make_unique<Counting>(std::move(resources.execution),frequencies);
    fixture.runtime=initial_cells::make_fresh_runtime(resources,b2_test::permanent_errors,b4_test::host_api());fixture.lifetime->state=&fixture.runtime->configuration();fixture.lifetime.reset();check_bound(fixture,probe,second);
    require(!frequencies&&!resources.configuration&&!resources.callback_attachment&&!resources.stop_sources,"Fresh owning construction is passive and consumes Resources through existing sink");
    (void)probe.bound->execution.now_nanoseconds_036740();(void)probe.bound->execution.now_nanoseconds_036740();
    require(frequencies==1&&probe.bound->runtime.frequency_cache==10000000,"Zero factory cache binds actual Backend Execution and performs first lower frequency query only once");
    require(fixture.runtime->close()&&fixture.stops==0&&fixture.api_state.waited.empty(),"Never-published fresh Runtime closes without startup or joins");fixture.runtime.reset();
    require(probe.destroyed==1&&probe.retired_username=="anadius"&&fixture.destroyed_config=="anadius","Fresh Runtime provider retirement retains actual default configuration through destruction");
}
}
int main(){try{passive_and_independent();controlled_startup_and_close();mandatory_and_legacy_contracts();fresh_outside_and_owning_runtime();std::cout<<"PASS: "<<checks<<" observed-runtime checks; actual source assembly with controlled external service/thread boundaries\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
