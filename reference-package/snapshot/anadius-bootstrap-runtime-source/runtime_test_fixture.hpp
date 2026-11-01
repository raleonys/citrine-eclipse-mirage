#pragma once
#include "factory.hpp"
#include "../anadius-bootstrap-backend-source/backend_test_fixture.hpp"
#include "../anadius-bootstrap-owner-source/owner_test_fixture.hpp"
#include <map>
#include <type_traits>

namespace b4_test {
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
namespace r=bootstrap_runtime;
inline void previous_terminate(){std::abort();}
inline void foreign_terminate(){std::abort();}
struct HostState {
    std::vector<n::ExitCallback> callbacks;
    std::terminate_handler handler=previous_terminate;
    int exit_result=0;unsigned registrations=0,sets=0,gets=0;
};
inline HostState* host_state=nullptr;
inline int at_exit(n::ExitCallback callback) noexcept {
    ++host_state->registrations;
    if(!host_state->exit_result)host_state->callbacks.push_back(callback);
    return host_state->exit_result;
}
inline std::terminate_handler set_terminate(std::terminate_handler callback) noexcept {
    ++host_state->sets;const auto before=host_state->handler;host_state->handler=callback;return before;
}
inline std::terminate_handler get_terminate() noexcept {++host_state->gets;return host_state->handler;}
inline r::HostApi host_api(){return {at_exit,set_terminate,get_terminate};}
struct Lifetime {const configuration::State* state=nullptr;std::string* result;~Lifetime(){if(state)*result=state->username;}};
struct Fixture {
    b2_test::Track track;b3_tracking_test::ApiState api_state;HostState host;
    std::string destroyed_config;
    b2_test::Effects* config=nullptr;b2_test::Files* files=nullptr;b2_test::Backend* remaining=nullptr;
    b3_tracking_test::Lower* threads=nullptr;Execution* execution=nullptr;Server* server=nullptr;Online* online=nullptr;
    LibraryOS* library_os=nullptr;LibraryEffects* library_effects=nullptr;Cleanup* cleanup=nullptr;
    bool stop_allowed=true,timeout_once=false;unsigned stops=0;std::function<void()> stop_hook;
    std::map<std::uintptr_t,t::Context*> jobs,joins;
    std::shared_ptr<Lifetime> lifetime;std::unique_ptr<r::Runtime> runtime;
    Fixture(){b3_tracking_test::current=&api_state;host_state=&host;}
    b::RuntimeSeed runtime_seed(){
        b::RuntimeSeed s{};execution_recovered::initialize_mutex_04a698(s.execution_mutex,2);s.frequency_cache=10000000;
        s.libraries.loaded={0x777};s.libraries.from_main={L"stale-main"};s.libraries.from_worker={L"stale-worker"};return s;
    }
    b::Resources resources(){
        b::Resources v{};
        lifetime=std::make_shared<Lifetime>();lifetime->result=&destroyed_config;v.state_lifetime=lifetime;
        auto c=std::make_unique<b2_test::Effects>(track.events);config=c.get();v.configuration=std::move(c);
        auto f=std::make_unique<b2_test::Files>(track.events);files=f.get();f->base=b2_test::base_text();
        f->over=R"("Config2" {"User" {"Username" "override-user"} "Emulator" {"LoadExtraDLLsFromMain" "main.dll" "LoadExtraDLLs" "worker.dll"}})";v.files=std::move(f);
        v.input=std::make_unique<b2_test::InputPlatform>(track.events);v.console=std::make_unique<b2_test::Console>(track.events);
        auto th=std::make_unique<b3_tracking_test::Lower>();threads=th.get();
        th->create_hook=[&](t::Context* p,std::uint32_t* id){const auto h=0x1100+threads->creations;jobs[h]=p;if(id)*id=threads->creations+10;track.events.emplace_back("thread-"+std::to_string(threads->creations));return h;};v.threads=std::move(th);
        auto m=std::make_unique<b2_test::Backend>(track);remaining=m.get();v.remaining=std::move(m);
        auto e=std::make_unique<Execution>(track.events);execution=e.get();v.execution=std::move(e);
        auto s=std::make_unique<Server>(track.events);server=s.get();v.server=std::move(s);
        auto o=std::make_unique<Online>(track.events);online=o.get();v.online=std::move(o);
        auto l=std::make_unique<LibraryOS>(track.events);library_os=l.get();v.library_os=std::move(l);
        auto le=std::make_unique<LibraryEffects>();library_effects=le.get();v.library_effects=std::move(le);
        v.host=std::make_unique<Host>(track.events);
        auto cl=std::make_unique<Cleanup>(track.events);cleanup=cl.get();v.hook_cleanup=std::move(cl);
        v.stop_sources=[&]{++stops;track.events.emplace_back("stop-sources");if(stop_hook)stop_hook();return stop_allowed;};
        v.join_api=b3_tracking_test::api();v.join_timeout_ms=7;
        api_state.duplicate_hook=[&](HANDLE source,LPHANDLE out){const auto h=api_state.next_join++;*out=b3_tracking_test::handle(h);joins[h]=jobs.at(b3_tracking_test::number(source));return TRUE;};
        api_state.wait_hook=[&](HANDLE h,DWORD timeout){
            require(timeout==7,"factory preserves configured finite join timeout");
            if(timeout_once){timeout_once=false;return DWORD{WAIT_TIMEOUT};}
            auto it=joins.find(b3_tracking_test::number(h));require(it!=joins.end(),"join targets owned pending source job");
            auto* context=it->second;const auto entry=reinterpret_cast<n::ThreadStart>(context->entry);
            require(entry(reinterpret_cast<void*>(context->argument))==0,"pending actual Bootstrap source callback completes");
            threads->release(context);joins.erase(it);track.events.emplace_back("join-complete");return DWORD{WAIT_OBJECT_0};
        };
        api_state.close_hook=[&](HANDLE){track.events.emplace_back("join-handle-close");return TRUE;};
        return v;
    }
    void construct(b::Resources v){
        runtime=std::make_unique<r::Runtime>(b2_test::seed(),runtime_seed(),std::move(v),b2_test::permanent_errors,host_api());
        lifetime->state=&runtime->configuration();lifetime.reset();
    }
    void start(){construct(resources());runtime->publish();require(runtime->application_entry()(nullptr,1,nullptr)==TRUE,"source Runtime application attach succeeds");}
    ~Fixture(){
        if(runtime&&(runtime->phase()==o::Phase::Bound||runtime->phase()==o::Phase::Stopping)){
            stop_allowed=true;timeout_once=false;stop_hook={};try{runtime->close();}catch(...){std::terminate();}
        }
        runtime.reset();b3_tracking_test::current=nullptr;host_state=nullptr;
    }
};
inline std::size_t pos(const Events& e,std::string_view s){return b2_test::position(e,s);}
inline bool absent(const Events& events,std::string_view name){return std::none_of(events.begin(),events.end(),[&](const auto& s){return s==name;});}
} // namespace b4_test
