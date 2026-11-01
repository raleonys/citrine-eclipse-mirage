#include "context_test_fixture.hpp"

namespace {
using namespace b15_test;
struct Engine final:recovered_hooks::Api {
    b14_test::Engine calls;n::Resolver resolver;
    std::map<std::uintptr_t,std::size_t> slots;
    std::vector<bool> published_at_create;
    unsigned callbacks_during_enable=0;
    int initialize()override{return calls.initialize();}
    int create(void* target,void* detour,void** out)override{
        const auto code=reinterpret_cast<std::uintptr_t>(detour);std::size_t slot=0;
        while(slot<10&&resolver.rebuilt_address(static_cast<n::Callback>(slot))!=code)++slot;
        if(slot==10)throw std::logic_error("engine received unrecognized rebuilt callback");
        published_at_create.push_back(n::is_bound());slots[reinterpret_cast<std::uintptr_t>(target)]=slot;
        const auto result=calls.create(target,detour,out);*out=reinterpret_cast<void*>(originals()[slot]);return result;
    }
    int enable(void* target)override{
        const auto result=calls.enable(target);
        if(slots.at(reinterpret_cast<std::uintptr_t>(target))==host_init_recovered::slot(n::Callback::reg_close_00f5b0)){
            if(!n::is_bound())throw std::logic_error("callback enabled before native Context publication");
            if(n::reg_close_entry(reinterpret_cast<HKEY>(0xabc))!=0)throw std::logic_error("controlled trampoline result");
            ++callbacks_during_enable;
        }
        return result;
    }
    int disable(void* target)override{return calls.disable(target);}
    int uninitialize()override{return calls.uninitialize();}
};
struct Graph {
    b4_test::Fixture fixture;b14_test::Graph host;
    std::shared_ptr<Probe> probe=std::make_shared<Probe>();
    std::shared_ptr<Engine> engine=std::make_shared<Engine>();
    std::shared_ptr<n::Resolver> resolver=std::make_shared<n::Resolver>();
    bool external_quiescent=true;unsigned proofs=0;bool proof_reentry=false;
    std::array<std::uintptr_t,10> input_originals{};
    Graph(bool fail_registration=false){
        active_probe=probe.get();host.domain=std::make_shared<host_platform::Domain>(host.output,engine,resolver,host.callback_lifetime,host.resources());
        auto resources=fixture.resources();fixture.cleanup=nullptr;
        const auto at=fixture.files->base.find("\"SDK\" \"1.2.3\"");
        fixture.files->base.replace(at,std::strlen("\"SDK\" \"1.2.3\""),"\"SDK\" \"1.2.3\" \"Documents\" \"C:/own/docs\"");
        resources=host_platform::bind_host_platform(std::move(resources),host.domain);
        resources.callback_attachment=nc::make_factory(seed(),[p=probe](nc::PolicyView view){return std::make_unique<Policy>(view,p);},permanent_errors,[this]{
            ++proofs;
            if(proof_reentry){proof_reentry=false;rejects<std::logic_error>([&]{(void)fixture.runtime->close();},"Runtime rejects reentry from quiescence provider");}
            return external_quiescent&&engine->calls.uninit_calls!=0&&engine->calls.disabled.size()==engine->calls.enabled.size();
        });
        fixture.host.exit_result=fail_registration?23:0;
        auto runtime_seed=fixture.runtime_seed();input_originals=runtime_seed.originals;
        const auto* input_slots=&runtime_seed.originals;
        fixture.runtime=std::make_unique<bootstrap_runtime::Runtime>(b2_test::seed(),std::move(runtime_seed),std::move(resources),b2_test::permanent_errors,b4_test::host_api());
        fixture.lifetime->state=&fixture.runtime->configuration();fixture.lifetime.reset();
        require(probe->runtime&&&probe->runtime->originals!=input_slots&&probe->config==&fixture.runtime->configuration(),"factory receives moved stable internal originals and actual Runtime configuration");
        require(!n::is_bound()&&engine->calls.events.empty(),"Runtime construction does not publish native Context or call engine");
    }
    void start(){fixture.runtime->publish();require(!n::is_bound(),"application publication alone does not publish native Context");require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"source application attaches with native Context attachment");}
    ~Graph(){
        if(fixture.runtime&&fixture.runtime->phase()!=bootstrap_owner::Phase::Closed){
            fixture.timeout_once=false;external_quiescent=true;
            // Test-only emergency cleanup if a failed assertion unwinds. It
            // does not change the production source failed-registration path.
            if(n::is_bound()&&engine->calls.uninit_calls==0){for(auto target:engine->calls.enabled)engine->disable(reinterpret_cast<void*>(target));engine->uninitialize();}
            try{fixture.runtime->close();}catch(...){std::abort();}
        }
        fixture.runtime.reset();active_probe=nullptr;
    }
};
void exact_runtime_identity(){
    Graph g;g.start();
    require(n::is_bound()&&g.engine->callbacks_during_enable==1&&g.probe->closes==1,"native Context exists before fakeenable synchronously calls actual typed shim/trampoline");
    require(std::all_of(g.engine->published_at_create.begin(),g.engine->published_at_create.end(),[](bool value){return value;}),"all engine creates observe published Context");
    require(g.probe->runtime->originals[host_init_recovered::slot(n::Callback::reg_close_00f5b0)]==reinterpret_cast<std::uintptr_t>(&original_close),"engine original write reaches actual internal slot used by OriginalEffects");
    require(std::all_of(g.input_originals.begin(),g.input_originals.end(),[](auto value){return value==0;}),"pre-move input slots remain unused copies of initial values");
    std::array<char,260> path{};
    require(n::folder_a_entry(nullptr,5,nullptr,0,path.data())==0&&std::string(path.data())=="C:/own/docs","actual host projection reaches native Context registry map");
    g.probe->runtime->registry_values["Documents"]=u"CHANGED-live-map";
    require(n::folder_a_entry(nullptr,5,nullptr,0,path.data())==0&&std::string(path.data())=="CHANGED-live-map","later internal map mutation proves references rather than snapshot");
    g.fixture.timeout_once=true;require(!g.fixture.runtime->close()&&n::is_bound()&&g.engine->calls.disabled.empty(),"join timeout retains native Context until hook cleanup");
    g.external_quiescent=false;
    require(!g.fixture.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==1,"false external proof retains Context after actual source cleanup");
    const auto disabled=g.engine->calls.disabled.size();const auto uninit=g.engine->calls.uninit_calls;
    g.external_quiescent=true;g.proof_reentry=true;
    require(g.fixture.runtime->close()&&!n::is_bound(),"explicit quiescence closes native binding after drained hook cleanup");
    require(g.engine->calls.disabled.size()==disabled&&g.engine->calls.uninit_calls==uninit&&g.proofs==2,"retry does not repeat drained cleanup prefix");
    require(g.fixture.runtime->close()&&g.proofs==2,"fully closed Runtime is idempotent");
    g.fixture.runtime.reset();require(g.probe->destroyed==1&&!bootstrap_native_recovered::is_bound(),"policy dies after successful native/application binding shutdown");
}
void failed_registration_proof(){
    Graph g(true);g.start();
    require(!g.fixture.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==0&&g.engine->calls.disabled.empty(),"empty exit queue after failed registration does not fabricate disabled hooks");
    require(g.fixture.joins.empty()&&g.fixture.runtime->phase()==bootstrap_owner::Phase::Stopping,"workers joined while native Context/providers retained for external cleanup");
    for(auto target:g.engine->calls.enabled)g.engine->disable(reinterpret_cast<void*>(target));g.engine->uninitialize();
    require(g.fixture.runtime->close()&&!n::is_bound()&&g.engine->calls.uninit_calls==1,"explicit test-owned manual cleanup permits later proof and close without invented exit callback");
}
void construction_rejection_and_no_attach(){
    {
        b4_test::Fixture f;auto resources=f.resources();resources.callback_attachment=[](b::CallbackStateView){return std::unique_ptr<b::CallbackAttachment>{};};
        rejects<std::invalid_argument>([&]{f.construct(std::move(resources));},"null optional attachment factory result rejected");
        require(!n::is_bound()&&!bootstrap_native_recovered::is_bound(),"failed Runtime construction publishes neither callback nor application binding");
    }
    {
        Graph g;g.fixture.runtime->publish();require(g.fixture.runtime->close()&&!n::is_bound()&&g.proofs==0,"Runtime published without application attach closes unpublished owner without native proof");
        require(g.engine->calls.events.empty(),"unattached close invents no engine cleanup");
    }
}
}
int main(){try{exact_runtime_identity();failed_registration_proof();construction_rejection_and_no_attach();std::cout<<"PASS: "<<b15_test::checks<<" native Context integration checks; exact moved Runtime state and own typed callbacks, controlled engine/join\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
