#include "backend.hpp"
#include "backend_test_fixture.hpp"
#include "../anadius-bootstrap-owner-source/owner_test_fixture.hpp"
#include <map>
#include <type_traits>

namespace {
using namespace anadius;
namespace b=bootstrap_backend;namespace o=bootstrap_owner;namespace n=bootstrap_native_recovered;namespace t=threads_recovered;
using b2_test::require;using b2_test::rejects;using b2_test::Events;
struct Execution final:execution_recovered::Platform {
    Events& events;std::int64_t ticks=0;std::function<void()> sleep_hook;unsigned sleeps=0;
    explicit Execution(Events& e):events(e){}
    std::uint32_t current_thread_id()override{return 7;}
    void acquire_srw_exclusive(std::uintptr_t&)override{events.emplace_back("execution-lock");}
    void release_srw_exclusive(std::uintptr_t&)override{events.emplace_back("execution-unlock");}
    std::int64_t query_performance_frequency()override{return 10000000;}
    std::int64_t query_performance_counter()override{return ticks;}
    void sleep_milliseconds(std::uint32_t ms)override{++sleeps;events.emplace_back("execution-sleep");if(sleep_hook)sleep_hook();ticks+=std::int64_t{ms}*10000;}
    [[noreturn]] void raise_thread_error_04a648(int)override{throw std::runtime_error("execution-error");}
};
struct Server final:server_recovered::ServerBackend {
    Events& events;unsigned runs=0;
    explicit Server(Events& e):events(e){}
    void initialize_context_0241b0()override{events.emplace_back("server-context");}
    server_recovered::ParsedAddress parse_address_028690(std::uint16_t f,std::string_view text)override{require(f==23&&text=="127.0.0.1","source server chooses observed first address");return {1,{},{},0};}
    void construct_acceptor_020e20(const server_recovered::Endpoint& p,bool reuse,int backlog)override{require(p.family==23&&reuse&&backlog==0x7fffffff,"source server acceptor operands");events.emplace_back("server-acceptor");}
    void initialize_pending_socket_0211b0()override{events.emplace_back("server-pending");}
    server_recovered::LocalEndpoint local_endpoint()override{return {{},0x3412,28};}
    bool set_environment_variable(std::string_view key,std::string_view value)override{require(key=="EALsxPort"&&value=="4660","source server publishes converted live port");events.emplace_back("server-port");return false;}
    void async_accept_021b00()override{events.emplace_back("server-accept");}
    server_recovered::Error run_context_0298b0()override{++runs;events.emplace_back("server-run");return {};}
    void destroy_pending_socket_024db0()override{events.emplace_back("server-destroy-pending");}
    void destroy_acceptor_024db0()override{events.emplace_back("server-destroy-acceptor");}
    void destroy_context()override{events.emplace_back("server-destroy-context");}
    [[noreturn]] void raise_error(server_recovered::Error,std::string_view)override{throw std::runtime_error("server-error");}
};
struct Online final:online_recovered::Effects {
    Events& events;bool found=true;unsigned waits=0,finishes=0,loads=0;
    explicit Online(Events& e):events(e){}
    std::uint32_t suppress_errors_0450a0()override{events.emplace_back("online-suppress");return 77;}
    void restore_error_mode(std::uint32_t v)override{require(v==77,"online source restores prior error mode");}
    configuration::Path filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    configuration::Path parent_path_019820(const configuration::Path&)override{return u"fixture";}
    configuration::Path join_016b00(const configuration::Path& a,const configuration::Path& c)override{return a+u"/"+c;}
    online_recovered::Module load_copy_0450d0(const configuration::Path&)override{++loads;events.emplace_back("online-load");return found?0x80:0;}
    online_recovered::Procedure get_proc_address(online_recovered::Module m,std::string_view name)override{require(m==0x80,"online module state is shared");return name=="version_check"?0x81:0x82;}
    std::int32_t call_version_check(online_recovered::Procedure p)override{require(p==0x81,"online version provider identity");return 2;}
    online_recovered::Object call_create_online(online_recovered::Procedure p,const char16_t*)override{require(p==0x82,"online factory provider identity");return 0x90;}
    std::int32_t status_virtual_08(online_recovered::Object o)override{require(o==0x90,"online live object");return 0;}
    const char* username_virtual_10(online_recovered::Object)override{return "network-user";}
    const char* persona_virtual_18(online_recovered::Object)override{return "network-persona";}
    const char* user_id_virtual_20(online_recovered::Object)override{return "network-id";}
    void destroy_virtual_00(online_recovered::Object o)override{require(o==0x90,"online cleanup uses live object");events.emplace_back("online-destroy");}
    bool free_library(online_recovered::Module m)override{require(m==0x80,"online cleanup uses live module");events.emplace_back("online-free");return false;}
    status_recovered::Status status_01ea20(const configuration::Path&,std::uint32_t f)override{require(f==3,"online file-status flags");return {0,0,0};}
    [[noreturn]] void filesystem_error_018130(std::string_view,std::uint32_t,const configuration::Path&)override{throw std::runtime_error("online-fs");}
    std::string read_binary_03b3f0(const configuration::Path&)override{throw std::logic_error("no profile contents read");}
    bool write_binary_03c1d0(const configuration::Path&,std::string_view)override{throw std::logic_error("no profile contents written");}
    void log_literal_006880(std::string_view)override{}
    void log_finish_0088e0()override{}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{throw std::logic_error("no dialog");}
    void wait_steps_036880(std::int32_t)override{++waits;throw std::logic_error("online wait must use shared Execution source");}
    void finish_flag_036820()override{++finishes;throw std::logic_error("online finish must use shared Execution source");}
};
struct LibraryOS final:recovered::libraries::SystemCalls {
    Events& events;std::vector<std::wstring> paths;std::vector<std::uintptr_t> unloaded;std::uint32_t mode=45;
    explicit LibraryOS(Events& e):events(e){}
    std::uint32_t get_error_mode()override{return mode;}
    std::uint32_t set_error_mode(std::uint32_t v)override{auto old=mode;mode=v;return old;}
    std::uintptr_t load_library(const wchar_t* p)override{paths.emplace_back(p);events.emplace_back("library-load");return 0x500+paths.size();}
    bool free_library(std::uintptr_t m)override{unloaded.push_back(m);events.emplace_back("library-free");return false;}
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* p,int count,char* out,int capacity,bool* used)override{require(cp==65001&&flags==0x80&&!used,"library source UTF8 conversion operands");if(out){require(capacity==count,"library conversion capacity");for(int i=0;i<count;++i)out[i]=static_cast<char>(p[i]);}return count;}
    std::uint32_t last_error()override{return 0;}
};
struct LibraryEffects final:recovered::libraries::Effects {
    unsigned waits=0,finishes=0;
    void begin_load_log_0863e0()override{}
    void append_log_path_and_flush(std::string_view)override{}
    void message_038bc0(std::string,std::string,std::uint32_t)override{throw std::logic_error("no library UI");}
    [[noreturn]] void throw_runtime_00cd70(int)override{throw std::runtime_error("library-runtime");}
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t)override{throw std::runtime_error("library-fs");}
    void wait_steps_036880(int)override{++waits;throw std::logic_error("library wait must use shared Execution source");}
    void finish_flag_036820()override{++finishes;throw std::logic_error("library finish must use shared Execution source");}
};
struct Host final:host_init_recovered::Effects {
    Events& events;std::uintptr_t next=0x1000;
    explicit Host(Events& e):events(e){}
    std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t,std::uint8_t)override{return m+1;}
    bool verify_version(const host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t)override{return true;}
    std::int32_t initialize_engine_046a20()override{events.emplace_back("host-init");return 0;}
    std::uintptr_t get_module_handle(std::u16string_view)override{return 1;}
    std::uintptr_t get_proc_address(std::uintptr_t,std::string_view)override{return ++next;}
    std::int32_t create_hook_0466e0(std::uintptr_t target,host_init_recovered::Callback,std::uintptr_t& original)override{original=target+0x100000;return 0;}
    std::int32_t enable_hook_046a10(std::uintptr_t)override{return 0;}
    void log_text_006880(std::string_view)override{}
    void log_integer_00af90(std::int32_t)override{}
    void finish_log_0088e0()override{}
};
struct Cleanup final:network_hook_recovered::CleanupEffects {
    Events& events;std::vector<std::uintptr_t> targets;unsigned uninitializations=0;
    explicit Cleanup(Events& e):events(e){}
    std::int32_t disable_hook_046a00(std::uintptr_t a)override{targets.push_back(a);events.emplace_back("hook-disable");return 0;}
    std::int32_t uninitialize_engine_046ac0()override{++uninitializations;events.emplace_back("hook-uninitialize");return 0;}
    void log_literal_006880(std::string_view)override{}
    void log_finish_0088e0()override{}
};
struct Lifetime {const configuration::State* state=nullptr;std::string* result;~Lifetime(){if(state)*result=state->username;}};
struct Fixture {
    b2_test::Track track; b3_tracking_test::ApiState api_state;std::string destroyed_config;
    b2_test::Effects* config=nullptr;b2_test::Files* files=nullptr;b2_test::Backend* remaining=nullptr;
    b3_tracking_test::Lower* threads=nullptr;Execution* execution=nullptr;Server* server=nullptr;Online* online=nullptr;
    LibraryOS* library_os=nullptr;LibraryEffects* library_effects=nullptr;Cleanup* cleanup=nullptr;
    bool stop_allowed=true,timeout_once=false;unsigned stops=0;std::function<void()> stop_hook;
    std::map<std::uintptr_t,t::Context*> jobs;std::map<std::uintptr_t,t::Context*> joins;
    std::shared_ptr<Lifetime> lifetime; b::SourceBackend* backend=nullptr;std::unique_ptr<o::RuntimeOwner> owner;
    Fixture(){b3_tracking_test::current=&api_state;}
    b::RuntimeSeed runtime_seed(){b::RuntimeSeed s{};execution_recovered::initialize_mutex_04a698(s.execution_mutex,2);s.frequency_cache=10000000;s.libraries.loaded={0x777};s.libraries.from_main={L"stale-main"};s.libraries.from_worker={L"stale-worker"};return s;}
    b::Resources resources(){
        b::Resources r;
        lifetime=std::make_shared<Lifetime>();lifetime->result=&destroyed_config;r.state_lifetime=lifetime;
        auto c=std::make_unique<b2_test::Effects>(track.events);config=c.get();r.configuration=std::move(c);
        auto f=std::make_unique<b2_test::Files>(track.events);files=f.get();f->base=b2_test::base_text();f->over=R"("Config2" {"User" {"Username" "override-user"} "Emulator" {"LoadExtraDLLsFromMain" "main.dll" "LoadExtraDLLs" "worker.dll"}})";r.files=std::move(f);
        r.input=std::make_unique<b2_test::InputPlatform>(track.events);r.console=std::make_unique<b2_test::Console>(track.events);
        auto t=std::make_unique<b3_tracking_test::Lower>();threads=t.get();t->create_hook=[&](t::Context* p,std::uint32_t* id){const auto h=0x1100+threads->creations;jobs[h]=p;if(id)*id=threads->creations+10;track.events.emplace_back("thread-"+std::to_string(threads->creations));return h;};r.threads=std::move(t);
        auto m=std::make_unique<b2_test::Backend>(track);remaining=m.get();r.remaining=std::move(m);
        auto e=std::make_unique<Execution>(track.events);execution=e.get();r.execution=std::move(e);
        auto s=std::make_unique<Server>(track.events);server=s.get();r.server=std::move(s);
        auto o=std::make_unique<Online>(track.events);online=o.get();r.online=std::move(o);
        auto l=std::make_unique<LibraryOS>(track.events);library_os=l.get();r.library_os=std::move(l);
        auto le=std::make_unique<LibraryEffects>();library_effects=le.get();r.library_effects=std::move(le);
        r.host=std::make_unique<Host>(track.events);auto cl=std::make_unique<Cleanup>(track.events);cleanup=cl.get();r.hook_cleanup=std::move(cl);
        r.stop_sources=[&]{++stops;track.events.emplace_back("stop-sources");if(stop_hook)stop_hook();return stop_allowed;};r.join_api=b3_tracking_test::api();r.join_timeout_ms=7;
        api_state.duplicate_hook=[&](HANDLE source,LPHANDLE out){const auto h=api_state.next_join++;*out=b3_tracking_test::handle(h);joins[h]=jobs.at(b3_tracking_test::number(source));return TRUE;};
        api_state.wait_hook=[&](HANDLE h,DWORD timeout){require(timeout==7,"owner uses configured finite join timeout");if(timeout_once){timeout_once=false;return DWORD{WAIT_TIMEOUT};}auto it=joins.find(b3_tracking_test::number(h));require(it!=joins.end(),"join uses duplicate belonging to pending source job");auto* context=it->second;const auto entry=reinterpret_cast<n::ThreadStart>(context->entry);require(entry(reinterpret_cast<void*>(context->argument))==0,"queued source bootstrap callback returns normally");threads->release(context);joins.erase(it);return DWORD{WAIT_OBJECT_0};};
        return r;
    }
    void start(){auto source=std::make_unique<b::SourceBackend>(runtime_seed(),resources());backend=source.get();owner=std::make_unique<o::RuntimeOwner>(b2_test::seed(),std::move(source),b2_test::permanent_errors);lifetime->state=&owner->configuration();lifetime.reset();owner->publish();require(owner->application_entry()(nullptr,1,nullptr)==TRUE,"source-owned application attach succeeds");}
    ~Fixture(){if(owner&&(owner->phase()==o::Phase::Bound||owner->phase()==o::Phase::Stopping)){stop_allowed=true;timeout_once=false;stop_hook={};try{owner->close();}catch(...){std::terminate();}}b3_tracking_test::current=nullptr;}
};
std::size_t pos(const Events& e,std::string_view s){return b2_test::position(e,s);}
void full_owner_sources_and_retry(){
    Fixture f;f.start();
    require(f.owner->configuration().username=="override-user"&&f.owner->configuration().sdk=="1,2,3","actual CLI/config/FilePass complete before workers");
    require(f.backend->tracked_jobs()==3&&f.threads->original_closes.size()==3&&f.api_state.duplicated.size()==3,"three duplicate join tickets survive source detach closing originals");
    require(f.library_os->paths==std::vector<std::wstring>{L"main.dll"},"main library source reads newly loaded config instead of stale seed");
    require(pos(f.track.events,"log-override")<pos(f.track.events,"thread-1")&&pos(f.track.events,"thread-1")<pos(f.track.events,"host-init")&&pos(f.track.events,"host-init")<pos(f.track.events,"thread-2")&&pos(f.track.events,"library-load")<pos(f.track.events,"thread-3"),"normal config/server/host/online/mainlibrary/third-worker order retained");
    require(f.backend->state().server_ready==0&&f.backend->state().online_ready==0&&f.backend->state().libraries_ready==0,"pending callbacks are not mistaken for already-finished workers");
    f.stop_allowed=false;require(!f.owner->close()&&f.owner->phase()==o::Phase::Stopping&&n::is_bound()&&f.backend->tracked_jobs()==3&&f.api_state.waited.empty(),"false stop policy keeps all pending jobs,router and nativebinding alive");
    f.stop_allowed=true;f.timeout_once=true;require(!f.owner->close()&&f.backend->tracked_jobs()==3&&n::is_bound(),"join timeout retains owner and source routes for retry");
    require(f.owner->close()&&f.owner->phase()==o::Phase::Closed&&!n::is_bound()&&f.backend->tracked_jobs()==0,"retry drains actual pending source callbacks before unpublishing");
    require(f.backend->state().server_ready==1&&f.backend->state().online_ready==1&&f.backend->state().libraries_ready==1,"server,online andlibrary share one actual Execution flag graph");
    require(f.server->runs==1&&f.online->loads==1&&f.owner->configuration().server_port==0x1234&&f.owner->configuration().username=="network-user","source workers publish shared server port and online profile into owned configuration");
    require(f.library_os->paths==std::vector<std::wstring>{L"main.dll",L"worker.dll"}&&f.backend->state().libraries.loaded==std::vector<std::uintptr_t>{0x777,0x501,0x502},"main and worker source phases append to one retained loaded vector");
    require(f.online->waits==0&&f.online->finishes==0&&f.library_effects->waits==0&&f.library_effects->finishes==0,"remaining wait/finish substitutes are bypassed by shared Execution source");
    require(std::none_of(f.track.events.begin(),f.track.events.end(),[](const auto& e){return e=="worker-first"||e=="worker-second"||e=="sync-boolean"||e=="worker-boolean"||e=="between-workers";}),"worker selection,boolean phase and host initialization never delegate to remaining source substitutes");
    require(f.api_state.closed.size()==3&&f.library_os->unloaded.empty()&&f.cleanup->targets.empty()&&f.backend->state().online_module==0x80,"quiescence does not invent application exit cleanup");
    require(f.owner->close()&&f.stops==3,"closed owner is idempotent without another stop policy call");
    rejects<std::logic_error>([&]{(void)f.backend->worker(recovered::Worker::first_029ea0);},"closed router does not issue a usable new worker route");
    f.owner.reset();require(f.destroyed_config=="network-user","backend backing state is destroyed while owned configuration is still alive");
}
void native_exit_cleanup_after_explicit_drain(){
    Fixture f;f.start();auto p=f.backend->providers();auto& tracked=dynamic_cast<b::TrackingThreads&>(p.threads);
    require(tracked.stop_and_join(7)&&f.backend->tracked_jobs()==0&&n::is_bound(),"explicit drain leaves native binding live for existing source exit callback");
    require(f.remaining->exit_callback==n::Resolver{}.exit_callback()&&f.remaining->terminate_callback==n::Resolver{}.terminate_callback(),"remaining registrar receives rebuilt typed callback identities");
    const auto loaded=f.backend->state().libraries.loaded;f.remaining->exit_callback();
    require(f.library_os->unloaded==loaded&&f.backend->state().libraries.loaded==loaded,"actual unload source walks retained module vector without clearing it");
    require(f.backend->state().online_module==0&&f.backend->state().online_object==0,"actual online cleanup clears shared live module/object slots");
    require(f.cleanup->targets==f.backend->state().enabled_targets&&f.cleanup->targets.size()==6&&f.cleanup->uninitializations==1,"host install and source hook cleanup use same owned target collection");
    require(pos(f.track.events,"library-free")<pos(f.track.events,"online-destroy")&&pos(f.track.events,"online-free")<pos(f.track.events,"hook-disable"),"native Bootstrap exit preserves library thenonline thenhook cleanup order");
    require(std::none_of(f.track.events.begin(),f.track.events.end(),[](const auto& e){return e=="cleanup-first"||e=="cleanup-second"||e=="cleanup-network";}),"all three application cleanup algorithms bypass remaining Services substitutes");
    const auto unloads=f.library_os->unloaded.size();require(f.owner->close()&&f.library_os->unloaded.size()==unloads,"owner quiescence adds no duplicate application exit cleanup");
}
void delayed_worker_paths_and_control(){
    Fixture f;auto seed=f.runtime_seed();seed.server_ready=1;auto r=f.resources();auto backend=std::make_unique<b::SourceBackend>(std::move(seed),std::move(r));f.backend=backend.get();
    configuration::State state{};state.executable_path=u"fixture/app.exe";state.extra_dlls={u"early.dll"};std::uint8_t a=0,c=0,d=0;backend->bind_state(state,{a,c,d});
    backend->register_exit(n::Resolver{}.exit_callback());f.online->found=false;
    bool dispatched=false;
    f.execution->sleep_hook=[&]{if(!dispatched){dispatched=true;state.extra_dlls={u"late.dll"};backend->worker(recovered::Worker::second_035b60)();}};
    backend->boolean_worker()(false);
    f.execution->sleep_hook={};
    require(f.execution->sleeps==1&&f.library_os->paths==std::vector<std::wstring>{L"late.dll"},"worker library paths reload after actual wait2 completes rather than beforewait");
    require(backend->state().online_ready==1&&backend->state().libraries_ready==1&&f.online->waits==0&&f.library_effects->waits==0,"nested online source wait1 andlibrary wait2 complete through same flags");
    rejects<std::logic_error>([&]{backend->bind_state(state,{a,c,d});},"provider shared state cannot be rebound");
    f.stop_hook=[&]{rejects<std::logic_error>([&]{backend->stop_and_join();},"backend rejects stop_sources lifecycle reentry");};f.stop_allowed=false;
    require(!backend->stop_and_join(),"reentrant policy can fail while worker routes remain owned");f.stop_hook={};f.stop_allowed=true;require(backend->stop_and_join(),"backend control guard resets for normal retry");
}
void resource_and_route_prefixes(){
    rejects<std::invalid_argument>([]{b::SourceBackend missing(b::RuntimeSeed{},b::Resources{});},"missing owned provider graph is rejected before publication");
    Fixture first;auto r1=first.resources();auto one=std::make_unique<b::SourceBackend>(first.runtime_seed(),std::move(r1));configuration::State c1{};std::uint8_t a=0,c=0,d=0;one->bind_state(c1,{a,c,d});
    first.remaining->service_failure="register-exit";rejects<std::runtime_error>([&]{one->register_exit(n::Resolver{}.exit_callback());},"registration failure leaves source route owner alive for controlled close");
    first.remaining->service_failure.clear();require(one->worker(recovered::Worker::first_029ea0)!=nullptr,"published route is retained after registrar failure");
    Fixture second;auto r2=second.resources();auto two=std::make_unique<b::SourceBackend>(second.runtime_seed(),std::move(r2));configuration::State c2{};two->bind_state(c2,{a,c,d});
    rejects<std::logic_error>([&]{two->register_exit(n::Resolver{}.exit_callback());},"second backend cannot replace a live per-image worker route");
    require(two->stop_and_join()&&one->worker(recovered::Worker::first_029ea0)!=nullptr,"never-published backend closes without stealing another router");
    require(one->stop_and_join(),"first route owner can unpublish normally after failed registration");
}
}
int main(){try{full_owner_sources_and_retry();native_exit_cleanup_after_explicit_drain();delayed_worker_paths_and_control();resource_and_route_prefixes();
    std::cout<<"PASS: "<<b2_test::checks+b3_tracking_test::checks<<" bootstrap-backend integration checks; concrete ProviderOwner with actual config/bootstrap/server/online/library/host/cleanup sources,shared flags,post-wait path reload,duplicate joins and shutdown retry; OS/foreign-provider/registrar/PE-entry remain controlled/open\n";
}catch(const std::exception& e){std::cerr<<"FAIL after "<<b2_test::checks+b3_tracking_test::checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<b2_test::checks+b3_tracking_test::checks<<" checks\n";return 1;}}
