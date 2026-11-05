#include "platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <thread>
#include <exception>

namespace {
using namespace b14_test;
void host_matrix(){
    for(bool modern:{false,true})for(bool block:{false,true})for(bool folders:{false,true}){
        Graph g;State state;g.plan.result=modern?0x100:FALSE;state.configuration.block_reruns=block;
        state.configuration.registry_spoof=1;state.configuration.language_registry_key=u"SOFTWARE\\Own\\Language";
        state.configuration.game_attributes["INSTALLED_LANGUAGE"]="tr";
        if(folders)state.configuration.documents=u"C:\\own\\docs";
        hi::initialize_host_00e310(state.refs(),*g.host);
        const auto count=6U+(block?2U:0U)+(folders?2U:0U);
        require(state.targets.size()==count&&g.engine->targets.size()==count&&g.engine->enabled==state.targets,"actual host source installs selected6to10controlled hooks");
        require(g.plan.masks.size()==3&&g.plan.masks[0].type==2&&g.plan.masks[1].type==1&&g.plan.masks[2].type==0x20,"source version mask order2then1then32 retained");
        require(std::all_of(g.plan.masks.begin(),g.plan.masks.end(),[](const auto& call){return call.condition==3;}),"source condition values all greater-equal3");
        require(g.plan.types==0x23&&g.plan.version.dwMajorVersion==6&&g.plan.version.dwMinorVersion==2&&g.plan.version.wServicePackMajor==0,"actual host default version request reaches typed SDK structure");
        require(g.plan.modules[0]==L"ws2_32.dll"&&g.plan.modules[1]==(modern?L"KernelBase.dll":L"advapi32.dll"),"observed BOOL modern branch selects registry module");
        if(block)require(g.plan.modules[6]==(modern?L"KernelBase.dll":L"Kernel32.dll"),"block-rerun branch selects process module");
        require(state.values.at("ClientPath")==state.configuration.executable_path&&state.values.at("LanguageKey")==u"SOFTWARE\\Own"&&state.values.at("LanguageValue")==u"Language"&&state.values.at("Language")==u"tr","existing host source populates explicit registry projection");
        require(!state.values.contains("old")&&state.configuration.registry_spoof==1&&state.values.contains("Documents")==folders,"source map clear/language flag/documents behavior preserved");
        require(g.output->str()==(folders?"MinHook initialization succeeded\nhooking special folders\n":"MinHook initialization succeeded\n"),"exact successful setup log and optional folders line");
        for(std::size_t i=0;i<g.resolver->seen.size();++i){const auto slot=hi::slot(g.resolver->seen[i]);require(state.originals[slot]==g.engine->targets[i]+0x100000&&g.engine->detours[i]==0x90000+slot*0x100,"source original slot receives selected engine output and rebuilt detour identity");}
        const auto targets=state.targets;network_hook_recovered::cleanup_hooks_0102b0(state.targets,*g.cleanup);
        require(g.engine->disabled==targets&&g.engine->uninit_calls==1&&state.targets==targets,"existing cleanup visits same engine targets in order without clearing vector");
        require(g.output->str().ends_with("MinHook uninitialization succeeded\n"),"actual cleanup uses shared output owner");
    }
}
void failure_prefixes(){
    for(int init:{1,2,9}){
        Graph g;State state;g.engine->init=init;hi::initialize_host_00e310(state.refs(),*g.host);
        require(g.engine->events==std::vector<std::string>({"init"})&&state.targets.empty()&&g.plan.modules.empty(),"nonzero initialization including1 prevents all hook requests");
        require(g.output->str()=="MinHook initialization failed\n"&&std::all_of(state.originals.begin(),state.originals.end(),[](auto value){return value==0xabc;}),"failed init preserves originals and exact log after target vector clear");
    }
    {
        Graph g;State state;g.plan.missing_module=L"ws2_32.dll";hi::initialize_host_00e310(state.refs(),*g.host);
        require(state.targets.size()==5&&state.originals[0]==0xabc&&g.engine->targets.size()==5,"missing module skips create for that slot but continues later hooks");
        require(g.output->str().find("Hooking function connect failed; error 11\n")!=std::string::npos,"missing module reports source status11");
    }
    {
        Graph g;State state;g.plan.missing_proc="connect";hi::initialize_host_00e310(state.refs(),*g.host);
        require(state.targets.size()==5&&state.originals[0]==0xabc&&g.output->str().find("Hooking function connect failed; error 12\n")!=std::string::npos,"missing export preserves slot and reports source status12");
    }
    {
        Graph g;State state;g.engine->create_results[0]=8;hi::initialize_host_00e310(state.refs(),*g.host);
        require(g.engine->targets.size()==6&&g.engine->enabled.size()==5&&state.targets.size()==5,"create failure skips only that enable/append and continues");
        require(state.originals[0]==g.engine->targets[0]+0x100000&&g.output->str().find("Hooking function connect failed; error 8\n")!=std::string::npos,"failed normal create retains copied original slot and log");
    }
    {
        Graph g;State state;g.engine->enable_results[0]=5;hi::initialize_host_00e310(state.refs(),*g.host);
        require(g.engine->enabled.size()==6&&state.targets.size()==5&&state.originals[0]==g.engine->targets[0]+0x100000,"failed enable preserves created original but does not append active target");
        require(g.output->str().find("Enabling hook for connect failed; error 5\n")!=std::string::npos,"enable failure uses distinct prefix");
    }
    {
        Graph g;State state;g.encoding->fail=true;
        rejects<std::runtime_error>([&]{hi::initialize_host_00e310(state.refs(),*g.host);},"encoding exception leaves source map prefix");
        require(state.values.size()==1&&state.values.contains("ClientKey")&&state.values.at("ClientKey").empty()&&state.targets==std::vector<std::uintptr_t>({0x111}),"map insert precedes conversion and target clear remains later");
        require(g.engine->events.empty()&&g.plan.verifies==0&&g.output->str().empty(),"early conversion failure does not reach OS/engine/log");
    }
    {
        Graph g;State state;g.engine->throw_create=true;
        rejects<std::runtime_error>([&]{hi::initialize_host_00e310(state.refs(),*g.host);},"injected create outcome stops source initialization");
        require(state.targets.empty()&&state.originals[0]==0xabc&&g.engine->targets.size()==1&&g.engine->enabled.empty(),"unknown throwing outcome does not invent original copyback/enable or rollback");
        require(g.output->str()=="MinHook initialization succeeded\n","throwing first create preserves earlier success log only");
    }
    {
        Graph g;State state;g.engine->throw_enable=true;
        rejects<std::runtime_error>([&]{hi::initialize_host_00e310(state.refs(),*g.host);},"enable exception propagates");
        require(state.targets.empty()&&state.originals[0]==g.engine->targets[0]+0x100000&&g.engine->enabled.size()==1,"enable exception retains known completed original write before target append");
    }
}
void cleanup_prefixes(){
    {
        Graph g;const std::vector<std::uintptr_t> targets{0x111,0x222};g.engine->disable_results[0]=6;g.engine->uninit=2;
        network_hook_recovered::cleanup_hooks_0102b0(targets,*g.cleanup);
        require(g.engine->disabled==targets&&g.engine->uninit_calls==1,"cleanup continues after nonzero disable and calls uninitialize");
        require(g.output->str()=="disabling hook failed\nMinHook uninitialization failed\n","cleanup failure log order and status mapping retained");
    }
    {
        Graph g;g.engine->throw_uninit=true;
        rejects<std::runtime_error>([&]{network_hook_recovered::cleanup_hooks_0102b0({},*g.cleanup);},"uninitialize exception propagates after prefix");
        require(g.output->str()=="MinHook uninitialization "&&g.output->flushes==0,"unknown uninit outcome leaves prefix without invented result/newline");
    }
    {
        Graph g;g.engine->throw_disable=true;const std::vector<std::uintptr_t> targets{0x111,0x222};
        rejects<std::runtime_error>([&]{network_hook_recovered::cleanup_hooks_0102b0(targets,*g.cleanup);},"disable exception propagates before subsequent targets");
        require(g.engine->disabled==std::vector<std::uintptr_t>({0x111})&&g.engine->uninit_calls==0&&g.output->str().empty(),"throwing disable adds no fabricated later cleanup or rollback");
    }
}
void runtime_source(){
    b4_test::Fixture f;Graph g;auto resources=f.resources();resources=hp::bind_host_platform(std::move(resources),g.domain);f.cleanup=nullptr;
    f.construct(std::move(resources));f.runtime->publish();
    require(f.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B3/B4 source Runtime attaches with shared host/hook owner");
    require(g.engine->init_calls==1&&g.engine->targets.size()==6&&f.joins.size()==3,"actual between-workers host source creates six controlled hooks while workers remain pending");
    require(g.output->str()=="MinHook initialization succeeded\n"&&g.engine->disabled.empty(),"host initialization log precedes joins and hook cleanup");
    f.timeout_once=true;require(!f.runtime->close()&&g.engine->disabled.empty(),"join timeout keeps same engine/callback owners and does not disable early");
    require(f.runtime->close(),"retry joins actual source callbacks then drains cleanup");
    require(g.engine->disabled==g.engine->enabled&&g.engine->uninit_calls==1,"actual Runtime cleanup uses same selected engine instance and enabled targets");
    require(g.output->str()=="MinHook initialization succeeded\nMinHook uninitialization succeeded\n","shared output records startup then quiescent cleanup once");
    require(f.runtime->close()&&g.engine->uninit_calls==1,"closed Runtime does not repeat engine shutdown");
    f.runtime.reset();require(!bootstrap_native_recovered::is_bound(),"source Runtime entry binding closes after callbacks/join cleanup");
    // Fake resolver tokens were never invoked. This does not establish the
    // native Context's identity with the internal moved RuntimeSeed originals.
}
void publication_before_enable(){
    State state;Graph g;
    auto publication=std::make_shared<original_provider::Publication>(g.callback_lifetime);
    publication->bind_slots(state.originals);publication->setup_begin();
    auto resources=g.resources();resources.original_publication=publication;
    auto domain=std::make_shared<hp::Domain>(g.output,g.engine,g.resolver,g.callback_lifetime,std::move(resources));
    auto host=std::make_unique<hp::HostPlatform>(domain);unsigned observed=0;
    g.engine->on_enable=[&](std::uintptr_t target){
        const auto callback=g.resolver->seen.back();std::uintptr_t original=0;std::exception_ptr error;
        std::thread reader([&]{try{original=publication->load(callback);}catch(...){error=std::current_exception();}});reader.join();
        if(error)std::rethrow_exception(error);
        require(original==target+0x100000&&publication->snapshot().phase==original_provider::PublicationPhase::Setup,
            "actual HostPlatform publishes exact adapter copyback for a foreign read before lower enable");++observed;
    };
    g.engine->create_results[0]=8;
    hi::initialize_host_00e310(state.refs(),*host);
    require(observed==5&&g.engine->enabled.size()==5&&state.targets.size()==5,
        "failed create is not published/enabled and later actual host hooks continue in native order");
    rejects<std::logic_error>([&]{publication->load(hi::Callback::connect_00fd00);},"failed create output remains unavailable even though fake engine wrote it");
    require(state.originals[0]==g.engine->targets[0]+0x100000,"normal failed original copyback remains unchanged from native adapter policy");
    publication->setup_complete();publication->stop();g.engine->on_enable={};
}
}
int main(){try{host_matrix();failure_prefixes();cleanup_prefixes();runtime_source();publication_before_enable();std::cout<<"PASS: "<<b14_test::checks<<" host platform integration checks; actual host/cleanup/Runtime sources with controlled SDK/engine/resolver\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
