#include "publication_test_fixture.hpp"

namespace {
using namespace b19_test;using namespace std::chrono_literals;
struct Server final:server_recovered::ServerBackend {
    std::unique_ptr<server_recovered::ServerBackend> lower;
    std::mutex gate;std::condition_variable cv;bool entered=false,release=false;
    explicit Server(std::unique_ptr<server_recovered::ServerBackend> value):lower(std::move(value)){}
    void initialize_context_0241b0()override{lower->initialize_context_0241b0();}
    server_recovered::ParsedAddress parse_address_028690(std::uint16_t f,std::string_view s)override{return lower->parse_address_028690(f,s);}
    void construct_acceptor_020e20(const server_recovered::Endpoint& e,bool b,int n)override{lower->construct_acceptor_020e20(e,b,n);}
    void initialize_pending_socket_0211b0()override{lower->initialize_pending_socket_0211b0();}
    server_recovered::LocalEndpoint local_endpoint()override{return lower->local_endpoint();}
    bool set_environment_variable(std::string_view a,std::string_view b)override{return lower->set_environment_variable(a,b);}
    void async_accept_021b00()override{lower->async_accept_021b00();}
    server_recovered::Error run_context_0298b0()override{
        {std::unique_lock lock(gate);entered=true;cv.notify_all();if(!cv.wait_for(lock,5s,[&]{return release;}))throw std::runtime_error("own server hold timed out");}
        return lower->run_context_0298b0();
    }
    void destroy_pending_socket_024db0()override{lower->destroy_pending_socket_024db0();}
    void destroy_acceptor_024db0()override{lower->destroy_acceptor_024db0();}
    void destroy_context()override{lower->destroy_context();}
    [[noreturn]]void raise_error(server_recovered::Error e,std::string_view s)override{lower->raise_error(e,s);std::terminate();}
};
struct Engine final:recovered_hooks::Api {
    b14_test::Engine calls;native_recovered::Resolver resolver;
    int initialize()override{return calls.initialize();}
    int create(void* target,void* detour,void** out)override{
        std::size_t slot=0;while(slot<10&&resolver.rebuilt_address(static_cast<host_init_recovered::Callback>(slot))!=reinterpret_cast<std::uintptr_t>(detour))++slot;
        if(slot==10)throw std::logic_error("own resolver callback");const auto result=calls.create(target,detour,out);*out=reinterpret_cast<void*>(b15_test::originals()[slot]);return result;
    }
    int enable(void* target)override{return calls.enable(target);}
    int disable(void* target)override{return calls.disable(target);}
    int uninitialize()override{return calls.uninitialize();}
};
struct Graph {
    b18_test::Fixture policies;b4_test::Fixture runtime;b14_test::Graph host;
    std::shared_ptr<Engine> engine=std::make_shared<Engine>();std::shared_ptr<native_recovered::Resolver> resolver=std::make_shared<native_recovered::Resolver>();
    std::shared_ptr<sp::Milestones> milestones;std::shared_ptr<foreign_callback::Domain> callbacks;
    std::vector<threads_recovered::Context*> jobs;std::set<threads_recovered::Context*> started;
    std::thread server_thread;std::exception_ptr server_failure;Server* server=nullptr;
    bool providers=true,external=true;unsigned provider_checks=0;
    Graph(){
        host.domain=std::make_shared<host_platform::Domain>(host.output,engine,resolver,host.callback_lifetime,host.resources());
        auto resources=runtime.resources();runtime.cleanup=nullptr;
        auto wrapped=std::make_unique<Server>(std::move(resources.server));server=wrapped.get();resources.server=std::move(wrapped);
        auto original_wait=runtime.api_state.wait_hook;
        runtime.api_state.wait_hook=[this,original_wait](HANDLE handle,DWORD timeout){
            auto it=runtime.joins.find(b3_tracking_test::number(handle));
            if(it!=runtime.joins.end()&&started.contains(it->second)){runtime.threads->release(it->second);runtime.joins.erase(it);return DWORD{WAIT_OBJECT_0};}
            return original_wait(handle,timeout);
        };
        const auto at=runtime.files->base.find("\"SDK\" \"1.2.3\"");runtime.files->base.replace(at,std::strlen("\"SDK\" \"1.2.3\""),"\"SDK\" \"1.2.3\" \"Documents\" \"C:/own/docs\"");
        resources=host_platform::bind_host_platform(std::move(resources),host.domain);
        auto bound=sp::bind_foreign(std::move(resources),policies.heap.domain,b15_test::seed(),policies.resources(),b15_test::permanent_errors,
            [this]{++provider_checks;return providers&&engine->calls.init==0;},
            [this]{return external&&engine->calls.uninit_calls!=0&&engine->calls.disabled.size()==engine->calls.enabled.size();});
        milestones=std::move(bound.milestones);callbacks=std::move(bound.callbacks);
        runtime.runtime=std::make_unique<bootstrap_runtime::Runtime>(b2_test::seed(),runtime.runtime_seed(),std::move(bound.resources),b2_test::permanent_errors,b4_test::host_api());
        runtime.lifetime->state=&runtime.runtime->configuration();runtime.lifetime.reset();
    }
    void start(){
        runtime.runtime->publish();require(runtime.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"source application attaches matched publication/Context");
        for(const auto& [handle,context]:runtime.jobs){(void)handle;jobs.push_back(context);}require(jobs.size()==3,"source creates server/online/library jobs");
    }
    static void run(threads_recovered::Context* context){auto entry=reinterpret_cast<bootstrap_native_recovered::ThreadStart>(context->entry);if(entry(reinterpret_cast<void*>(context->argument))!=0)throw std::runtime_error("source worker result");}
    void begin_server(){
        started.insert(jobs[0]);server_thread=std::thread([&]{try{run(jobs[0]);}catch(...){server_failure=std::current_exception();}});
        std::unique_lock lock(server->gate);if(!server->cv.wait_for(lock,2s,[&]{return server->entered;}))throw std::runtime_error("server did not publish before loop");
    }
    void finish_other_jobs(){for(std::size_t i=1;i<jobs.size();++i){started.insert(jobs[i]);b18_test::in_thread([&]{run(jobs[i]);});}}
    void release_server(){if(server_thread.joinable()){{std::lock_guard lock(server->gate);server->release=true;}server->cv.notify_all();server_thread.join();if(server_failure)std::rethrow_exception(server_failure);}}
    ~Graph(){
        try{release_server();if(runtime.runtime&&runtime.runtime->phase()!=bootstrap_owner::Phase::Closed){external=true;if(!runtime.runtime->close())std::abort();}}catch(...){std::abort();}
        runtime.runtime.reset();callbacks.reset();milestones.reset();
    }
};
void actual_source_publication(){
    Graph g;g.start();require(g.milestones->bits()==8&&!g.callbacks->ready()&&g.provider_checks==0,"host event precedes activation but worker bits still missing");
    require(!g.callbacks->activate_foreign()&&g.provider_checks==0,"caller cannot substitute a true provider proof for missing source milestones");
    g.begin_server();require(g.milestones->bits()==9&&g.server_thread.joinable(),"server prefix published while actual own source worker remains in run_context");
    g.finish_other_jobs();require(g.milestones->observed_all()&&!g.callbacks->ready(),"worker events alone do not activate foreign callbacks on worker threads");
    g.providers=false;require(!g.callbacks->activate_foreign()&&g.provider_checks==1,"remaining provider proof stays mandatory after concrete source events");
    g.providers=true;require(g.callbacks->activate_foreign()&&g.callbacks->ready(),"explicit controller activation consumes concrete source publication");
    b18_test::in_thread([&]{
        std::array<wchar_t,260> path{};
        require(native_recovered::folder_w_entry(nullptr,5,nullptr,0,path.data())==0&&std::wstring(path.data())==L"C:/own/docs","foreign callback sees host map after atomic source publication");
        sockaddr_in address{};address.sin_family=AF_INET;address.sin_port=htons(3216);
        require(native_recovered::connect_entry(1,reinterpret_cast<const sockaddr*>(&address),16)==-10035&&ntohs(address.sin_port)==4660,"foreign callback sees worker-published bound port while server loop is alive");
    });
    require(!g.callbacks->active()&&g.policies.memory->outstanding_allocations()==0,"published callback completes same-domain transient PTD scope");
    g.release_server();require(g.runtime.runtime->close()&&!native_recovered::is_bound(),"normal worker join/cleanup still required after phase readiness");
}
void engine_failure_is_not_success(){
    Graph g;g.engine->calls.init=7;g.start();
    require(g.milestones->bits()==8&&g.engine->calls.targets.empty(),"host normal return event does not mean engine initialization succeeded");
    g.begin_server();g.finish_other_jobs();require(g.milestones->observed_all()&&!g.callbacks->activate_foreign(),"all milestones cannot override failed external hook/provider conditions");g.release_server();
}
}
int main(){try{actual_source_publication();engine_failure_is_not_success();std::cout<<"PASS: "<<b19_test::checks<<" startup publication integration checks; real source milestones and live own server worker\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
