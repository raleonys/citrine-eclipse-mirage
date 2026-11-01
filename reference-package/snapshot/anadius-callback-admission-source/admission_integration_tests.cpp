#include "admission_test_fixture.hpp"

namespace {
using namespace b17_test;
struct Engine final:recovered_hooks::Api {
    b14_test::Engine calls;n::Resolver resolver;std::map<std::uintptr_t,std::size_t> slots;
    std::vector<bool> published;unsigned during_enable=0;
    int initialize()override{return calls.initialize();}
    int create(void* target,void* detour,void** out)override{
        std::size_t slot=0;while(slot<10&&resolver.rebuilt_address(static_cast<n::Callback>(slot))!=reinterpret_cast<std::uintptr_t>(detour))++slot;
        if(slot==10)throw std::logic_error("unknown native resolver callback");
        published.push_back(n::is_bound());slots[reinterpret_cast<std::uintptr_t>(target)]=slot;
        const auto result=calls.create(target,detour,out);auto originals=b15_test::originals();
        originals[1]=reinterpret_cast<std::uintptr_t>(&b16_test::failing_open_a);originals[2]=reinterpret_cast<std::uintptr_t>(&b16_test::failing_open_w);
        *out=reinterpret_cast<void*>(originals[slot]);return result;
    }
    int enable(void* target)override{
        const auto result=calls.enable(target);
        if(slots.at(reinterpret_cast<std::uintptr_t>(target))==3){
            if(n::reg_close_entry(reinterpret_cast<HKEY>(0xabc))!=0)throw std::logic_error("own original result");++during_enable;
        }
        return result;
    }
    int disable(void* target)override{return calls.disable(target);}
    int uninitialize()override{return calls.uninitialize();}
};
struct Graph {
    b16_test::Fixture policies;
    b4_test::Fixture runtime;
    b14_test::Graph host;
    std::shared_ptr<Engine> engine=std::make_shared<Engine>();
    std::shared_ptr<n::Resolver> resolver=std::make_shared<n::Resolver>();
    callback_policy::Policy* policy=nullptr;
    bootstrap_backend::RuntimeSeed* state=nullptr;
    nc::CallbackSeed* callbacks=nullptr;
    const configuration::State* config=nullptr;
    bool external=true,throw_proof=false;unsigned proofs=0;
    Graph(bool fail_registration=false){
        host.domain=std::make_shared<host_platform::Domain>(host.output,engine,resolver,host.callback_lifetime,host.resources());
        auto resources=runtime.resources();runtime.cleanup=nullptr;
        const auto at=runtime.files->base.find("\"SDK\" \"1.2.3\"");
        runtime.files->base.replace(at,std::strlen("\"SDK\" \"1.2.3\""),"\"SDK\" \"1.2.3\" \"Documents\" \"C:/own/docs\"");
        resources=host_platform::bind_host_platform(std::move(resources),host.domain);
        auto factory=callback_policy::make_policy_factory(policies.resources());
        resources.callback_attachment=ca::make_factory(b15_test::seed(),[this,factory=std::move(factory)](nc::PolicyView view){
            state=&view.backend.runtime;callbacks=&view.callbacks;config=&view.backend.configuration;
            auto result=factory(view);policy=dynamic_cast<callback_policy::Policy*>(result.get());return result;
        },b15_test::permanent_errors,[this]{
            ++proofs;if(throw_proof)throw std::runtime_error("own proof exception");
            return external&&engine->calls.uninit_calls!=0&&engine->calls.disabled.size()==engine->calls.enabled.size();
        });
        runtime.host.exit_result=fail_registration?23:0;
        runtime.runtime=std::make_unique<bootstrap_runtime::Runtime>(b2_test::seed(),runtime.runtime_seed(),std::move(resources),b2_test::permanent_errors,b4_test::host_api());
        runtime.lifetime->state=&runtime.runtime->configuration();runtime.lifetime.reset();
        require(policy&&state&&callbacks&&config==&runtime.runtime->configuration(),"concrete admitted policy binds final actual Runtime state");
        require(!n::is_bound()&&engine->calls.events.empty(),"construction publishes neither callbacks nor fake hooks");
    }
    void start(){runtime.runtime->publish();require(!n::is_bound(),"application publication does not prepublish native Context");require(runtime.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"actual Runtime application path attaches admitted Context");}
    void manual_cleanup(){for(auto target:engine->calls.enabled)engine->disable(reinterpret_cast<void*>(target));engine->uninitialize();}
    ~Graph(){
        if(runtime.runtime&&runtime.runtime->phase()!=bootstrap_owner::Phase::Closed){
            runtime.timeout_once=false;external=true;throw_proof=false;
            if(n::is_bound()&&!engine->calls.uninit_calls)manual_cleanup();
            try{if(!runtime.runtime->close())std::abort();}catch(...){std::abort();}
        }
        runtime.runtime.reset();
    }
};
void admitted_runtime(){
    Graph g;g.start();
    require(g.engine->during_enable==1&&g.policies.driver.probe->closes==1,"fake enable calls actual admitted native shim and original");
    require(std::all_of(g.engine->published.begin(),g.engine->published.end(),[](bool v){return v;}),"native Context precedes every hook create");
    require(g.policy->original_slots_identity()==&g.state->originals,"concrete policy retains exact internal Runtime slots");
    std::array<wchar_t,260> path{};
    require(n::folder_w_entry(nullptr,5,nullptr,0,path.data())==0&&std::wstring(path.data())==L"C:/own/docs","admitted wide callback uses projected real Runtime map and concrete byte bridge");
    g.state->registry_values["Documents"]=u"later-own-documents";
    require(n::folder_w_entry(nullptr,5,nullptr,0,path.data())==0&&std::wstring(path.data())==L"later-own-documents","same controller live mutation reaches callback");
    const auto client=g.state->registry_values.at("ClientKey");HKEY key{};
    require(n::reg_open_w_entry(HKEY_CURRENT_USER,reinterpret_cast<const wchar_t*>(client.c_str()),0,0,&key)==0,"actual admitted W registry open fabricates through same-domain allocator");
    require(g.policy->owned_handles()==1&&g.policies.memory->outstanding_allocations()==1&&g.callbacks->fabricated.contains(reinterpret_cast<std::uintptr_t>(key)),"Runtime factory owns one exact callback allocation");
    require(n::reg_close_entry(key)==0&&g.policy->owned_handles()==0&&g.policies.memory->outstanding_allocations()==0,"admitted close frees exact owned handle before lifecycle shutdown");
    g.runtime.timeout_once=true;require(!g.runtime.runtime->close()&&n::is_bound()&&g.engine->calls.disabled.empty(),"join timeout keeps admission and Context available before cleanup");
    require(n::folder_w_entry(nullptr,5,nullptr,0,path.data())==0,"controller callback still admitted after incomplete join");
    g.external=false;require(!g.runtime.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==1,"false external proof keeps gate open after source cleanup");
    const auto disabled=g.engine->calls.disabled.size();
    require(n::reg_close_entry(reinterpret_cast<HKEY>(0x1234))==0,"proof false does not prematurely stop admission");
    g.throw_proof=true;bool failed=false;try{g.runtime.runtime->close();}catch(const std::runtime_error&){failed=true;}
    require(failed&&n::is_bound(),"proof exception retains native binding and admission owner");g.throw_proof=false;
    require(n::folder_w_entry(nullptr,5,nullptr,0,path.data())==0,"proof throw preserves controller callback admission");
    g.external=true;require(g.runtime.runtime->close()&&!n::is_bound(),"successful external proof stops admission before native close");
    require(g.engine->calls.disabled.size()==disabled&&g.engine->calls.uninit_calls==1&&g.proofs==3,"retry consumes no duplicate hook cleanup prefix");
    require(g.runtime.runtime->close()&&g.proofs==3,"Runtime final close is idempotent");
    g.runtime.runtime.reset();require(!bootstrap_native_recovered::is_bound()&&g.policies.memory->outstanding_allocations()==0,"native/application bindings and callback memory close before providers disappear");
}
void failed_registration_and_unattached(){
    {
        Graph g(true);g.start();
        require(!g.runtime.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==0,"failed exit registration empty queue does not prove cleanup/quiescence");
        require(n::reg_close_entry(reinterpret_cast<HKEY>(0x1234))==0,"failed registration keeps usable admitted controller Context");
        g.manual_cleanup();require(g.runtime.runtime->close()&&!n::is_bound()&&g.engine->calls.uninit_calls==1,"explicit own engine cleanup permits proof stop and close");
    }
    {
        Graph g;g.runtime.runtime->publish();require(g.runtime.runtime->close()&&!n::is_bound()&&g.proofs==0,"unattached Runtime closes unpublished admission without proof");
        require(g.engine->calls.events.empty(),"unattached path adds no cleanup operation");
    }
}
}
int main(){try{admitted_runtime();failed_registration_and_unattached();std::cout<<"PASS: "<<b17_test::checks<<" callback admission integration checks; actual Runtime and concrete policy, own engine/SDK\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
