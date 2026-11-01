#include "foreign_test_fixture.hpp"

namespace {
using namespace b18_test;
struct Engine final:recovered_hooks::Api {
    b14_test::Engine calls;n::Resolver resolver;std::map<std::uintptr_t,std::size_t> slots;
    bool foreign_enable=false,reentry_original=false;unsigned during_enable=0;
    int initialize()override{return calls.initialize();}
    int create(void* target,void* detour,void** out)override{
        std::size_t slot=0;while(slot<10&&resolver.rebuilt_address(static_cast<n::Callback>(slot))!=reinterpret_cast<std::uintptr_t>(detour))++slot;
        if(slot==10)throw std::logic_error("unknown callback");slots[reinterpret_cast<std::uintptr_t>(target)]=slot;
        auto originals=b15_test::originals();originals[1]=reinterpret_cast<std::uintptr_t>(&b16_test::failing_open_a);originals[2]=reinterpret_cast<std::uintptr_t>(&b16_test::failing_open_w);
        if(reentry_original)originals[3]=reinterpret_cast<std::uintptr_t>(&b17_test::reentering_close);
        const auto result=calls.create(target,detour,out);*out=reinterpret_cast<void*>(originals[slot]);return result;
    }
    int enable(void* target)override{
        const auto result=calls.enable(target);
        if(slots.at(reinterpret_cast<std::uintptr_t>(target))==3){
            if(foreign_enable){std::thread thread([]{b17_test::bad_entry(0);});thread.join();}
            if(!reentry_original){if(n::reg_close_entry(reinterpret_cast<HKEY>(0xabc))!=0)throw std::logic_error("own close");++during_enable;}
        }
        return result;
    }
    int disable(void* target)override{return calls.disable(target);}
    int uninitialize()override{return calls.uninitialize();}
};
struct Graph {
    Fixture policies;b4_test::Fixture runtime;b14_test::Graph host;
    std::shared_ptr<Engine> engine=std::make_shared<Engine>();std::shared_ptr<n::Resolver> resolver=std::make_shared<n::Resolver>();
    std::shared_ptr<fc::Domain> domain;bootstrap_backend::RuntimeSeed* state=nullptr;
    std::set<threads_recovered::Context*> completed;
    bool publication=false,external=true;unsigned proofs=0;
    Graph(bool fail_registration=false,n::Errors& errors=b15_test::permanent_errors){
        domain=std::make_shared<fc::Domain>(policies.heap.domain,[this]{return publication;});
        host.domain=std::make_shared<host_platform::Domain>(host.output,engine,resolver,host.callback_lifetime,host.resources());
        auto resources=runtime.resources();runtime.cleanup=nullptr;
        auto original_wait=runtime.api_state.wait_hook;
        runtime.api_state.wait_hook=[this,original_wait](HANDLE handle,DWORD timeout){
            if(runtime.timeout_once){runtime.timeout_once=false;return DWORD{WAIT_TIMEOUT};}
            auto it=runtime.joins.find(b3_tracking_test::number(handle));
            if(it!=runtime.joins.end()&&completed.contains(it->second)){
                runtime.threads->release(it->second);runtime.joins.erase(it);return DWORD{WAIT_OBJECT_0};
            }
            return original_wait(handle,timeout);
        };
        const auto at=runtime.files->base.find("\"SDK\" \"1.2.3\"");runtime.files->base.replace(at,std::strlen("\"SDK\" \"1.2.3\""),"\"SDK\" \"1.2.3\" \"Documents\" \"C:/own/docs\"");
        resources=host_platform::bind_host_platform(std::move(resources),host.domain);
        auto attachment=fc::make_factory(domain,b15_test::seed(),policies.resources(),errors,[this]{++proofs;return external&&engine->calls.uninit_calls!=0&&engine->calls.disabled.size()==engine->calls.enabled.size();});
        resources.callback_attachment=[this,attachment=std::move(attachment)](bootstrap_backend::CallbackStateView view){state=&view.runtime;return attachment(view);};
        runtime.host.exit_result=fail_registration?23:0;
        runtime.runtime=std::make_unique<bootstrap_runtime::Runtime>(b2_test::seed(),runtime.runtime_seed(),std::move(resources),b2_test::permanent_errors,b4_test::host_api());
        runtime.lifetime->state=&runtime.runtime->configuration();runtime.lifetime.reset();
        require(state&&!n::is_bound()&&!domain->ready(),"actual Runtime owns unpublished warming foreign Context");
    }
    void start(){runtime.runtime->publish();require(runtime.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"actual source application attaches");}
    void activate(){
        // Own fake worker jobs are completed on the controller before proof;
        // later join only releases their test metadata, never runs them twice.
        for(const auto& [handle,context]:runtime.jobs){
            (void)handle;if(completed.insert(context).second){
                auto entry=reinterpret_cast<bootstrap_native_recovered::ThreadStart>(context->entry);
                if(entry(reinterpret_cast<void*>(context->argument))!=0)throw std::runtime_error("own worker completion");
            }
        }
        publication=true;require(runtime.server->runs==1&&domain->activate_foreign()&&domain->ready(),"completed own workers plus explicit frozen-state proof enable sequential foreign phase");
    }
    void manual_cleanup(){for(auto target:engine->calls.enabled)engine->disable(reinterpret_cast<void*>(target));engine->uninitialize();}
    ~Graph(){
        if(runtime.runtime&&runtime.runtime->phase()!=bootstrap_owner::Phase::Closed){
            runtime.timeout_once=false;external=true;
            if(n::is_bound()&&!engine->calls.uninit_calls)manual_cleanup();
            try{if(!runtime.runtime->close())std::abort();}catch(...){std::abort();}
        }
        runtime.runtime.reset();domain.reset();
    }
};
void sequential_foreign(){
    Graph g;g.start();require(n::is_bound()&&g.engine->during_enable==1&&!g.domain->ready(),"controller setup callback works before foreign publication");
    g.activate();const auto index=g.policies.heap.domain->native_fls_index();const auto refs=g.policies.heap.domain->locale_references();
    HKEY key{};const auto client=g.state->registry_values.at("ClientKey");
    in_thread([&]{
        require(!::FlsGetValue(index),"first foreign thread has no preexisting domain PTD");
        require(n::reg_open_w_entry(HKEY_CURRENT_USER,reinterpret_cast<const wchar_t*>(client.c_str()),0,0,&key)==0&&key,"first foreign thread fabricates via exact Runtime original and memory domain");
        require(!::FlsGetValue(index),"native callback leaves no PTD for later foreign thread exit");
    });
    require(g.policies.memory->outstanding_allocations()==1&&g.policies.heap.domain->locale_references()==refs,"fabricated handle retained across callbacks but temporary PTD refs released");
    in_thread([&]{
        std::array<wchar_t,260> documents{};require(n::folder_w_entry(nullptr,5,nullptr,0,documents.data())==0&&std::wstring(documents.data())==L"C:/own/docs","second foreign callback sees published Runtime map");
        DWORD size=0;const auto value=g.state->registry_values.at("ClientValue");
        require(n::reg_query_w_entry(key,reinterpret_cast<const wchar_t*>(value.c_str()),nullptr,nullptr,nullptr,&size)==0&&size>0,"later foreign callback sees same tracked handle and exact shared values");
        require(n::reg_close_entry(key)==0,"different foreign thread frees previously fabricated key");
        require(!::FlsGetValue(index),"all sequential entries clear temporary PTD before returning");
    });
    require(!g.domain->active()&&g.policies.memory->outstanding_allocations()==0&&g.policies.heap.domain->locale_references()==refs,"sequential thread handoff balances callbacks/heap/locale");
    require(g.runtime.runtime->close()&&!n::is_bound()&&!g.domain->ready(),"actual Runtime closes foreign admission before cleanup/unpublish");
}
void close_during_original(){
    Graph g;g.start();g.activate();auto probe=g.policies.driver.probe;
    {std::lock_guard lock(probe->gate);probe->block=true;probe->release=false;}
    std::exception_ptr failure;
    std::thread callback([&]{try{if(n::reg_close_entry(reinterpret_cast<HKEY>(0x111))!=0)throw std::runtime_error("own original result");}catch(...){failure=std::current_exception();}});
    {std::unique_lock lock(probe->gate);probe->cv.wait(lock,[&]{return probe->entered;});}
    const bool retained=!g.runtime.runtime->close()&&n::is_bound()&&g.domain->active()&&g.engine->calls.disabled.empty()&&g.engine->calls.uninit_calls==0&&g.proofs==0;
    {std::lock_guard lock(probe->gate);probe->release=true;}probe->cv.notify_all();callback.join();if(failure)std::rethrow_exception(failure);
    require(retained,"B4 close checks active foreign original before join/exit hook cleanup");
    require(!g.domain->active()&&!g.domain->ready(),"paused active callback completes PTD cleanup without losing owner permission");
    g.external=false;require(!g.runtime.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==1,"after drain cleanup may run but external proof remains separate");
    const auto disabled=g.engine->calls.disabled.size();g.external=true;
    require(g.runtime.runtime->close()&&!n::is_bound()&&g.engine->calls.uninit_calls==1&&g.engine->calls.disabled.size()==disabled,"external proof retry does not repeat source cleanup");
}
void timeout_and_registration(){
    {
        Graph g;g.start();g.activate();g.runtime.timeout_once=true;
        require(!g.runtime.runtime->close()&&n::is_bound()&&!g.domain->ready()&&g.engine->calls.disabled.empty(),"join timeout keeps providers while closing gate remains stopped");
        require(g.runtime.runtime->close()&&!n::is_bound(),"join retry completes source cleanup and native close");
    }
    {
        Graph g(true);g.start();g.activate();require(!g.runtime.runtime->close()&&n::is_bound()&&g.engine->calls.uninit_calls==0,"failed exit registration does not invent hook cleanup");
        g.manual_cleanup();require(g.runtime.runtime->close()&&!n::is_bound()&&g.engine->calls.uninit_calls==1,"explicit fake engine cleanup permits later external proof");
    }
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,expected_abort);
    Graph g(false,permanent_errors);
    if(mode=="--startup"){expected_code=160;expected_fault=fc::Fault::Warming;g.engine->foreign_enable=true;g.start();}
    else if(mode=="--overlap"){
        expected_code=161;expected_fault=fc::Fault::Overlap;g.start();g.activate();auto probe=g.policies.driver.probe;
        {std::lock_guard lock(probe->gate);probe->block=true;probe->release=false;}
        std::thread first([]{(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));});
        {std::unique_lock lock(probe->gate);probe->cv.wait(lock,[&]{return probe->entered;});}
        std::thread second([]{b17_test::bad_entry(0);});second.join();first.join();
    }else if(mode=="--reentry"){
        expected_code=162;expected_fault=fc::Fault::Reentry;g.engine->reentry_original=true;g.start();g.activate();
        in_thread([]{(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));});
    }else if(mode=="--cleanup-failure"){
        expected_code=163;g.start();g.activate();deny_root_free=true;in_thread([]{denied_cleanup_marker=0;std::signal(SIGABRT,native_cleanup_abort);(void)n::reg_close_entry(reinterpret_cast<HKEY>(1));terminate_own(230);});
    }
    return 230;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{sequential_foreign();close_during_original();timeout_and_registration();
    int code=160;for(const auto* mode:{"--startup","--overlap","--reentry"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child proves warming/overlap/reentry before native body");
    const auto cleanup_status=static_cast<std::uint32_t>(_spawnl(_P_WAIT,argv[0],argv[0],"--cleanup-failure",static_cast<const char*>(nullptr)));
    require(cleanup_status==163,"own abort observer requires failed raw release marker; native callback must not return");
    std::cout<<"PASS: "<<b18_test::checks<<" foreign callback integration checks; sequential own host threads and pre-cleanup drain\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
