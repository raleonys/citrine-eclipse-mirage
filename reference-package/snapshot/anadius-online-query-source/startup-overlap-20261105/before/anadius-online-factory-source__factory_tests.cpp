#include "factory.hpp"
#include "../anadius-application-effects-source/composition.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <process.h>
#include <thread>

namespace {
using namespace anadius;namespace of=online_factory;namespace online=online_recovered;
unsigned checks=0;void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Fault{};
struct Trace {
    of::Online* owner=nullptr;configuration::State* config=nullptr;bootstrap_backend::RuntimeSeed* runtime=nullptr;
    std::array<std::uintptr_t,7> table{};std::array<std::byte,17> bytes{};
    std::vector<std::string> events;std::vector<std::weak_ptr<void>> pins;
    std::string username=std::string(4096,'U'),persona=std::string("person\0tail",11),uid="";
    configuration::Path cookie;std::vector<std::string> exports;
    configuration::Path avatar_path;std::string avatar_owner,auth_client,auth_value=std::string(5000,'A');std::int32_t avatar_size=0;
    unsigned leases=0,pin_deletes=0,versions=0,factories=0,destroys=0,queries=0,proofs=0;
    bool valid=true,finality=true,version_missing=false,factory_missing=false,load_zero=false;
    bool throw_load=false,throw_factory=false,throw_destroy=false,throw_free=false,throw_query=false,throw_version=false;
    bool swallowed_lookup=false,poison_proof=false;std::int32_t version=2,status=0;
    HANDLE entered=nullptr,release=nullptr;bool block_query=false;
    std::exception_ptr failure=std::make_exception_ptr(Fault{});
    online::Object object(){return reinterpret_cast<online::Object>(bytes.data()+1);}
};
Trace* current=nullptr;
struct Pin {Trace& trace;~Pin(){++trace.pin_deletes;::SetLastError(0xdddd);}};
std::int32_t version(){auto& t=*current;++t.versions;t.events.push_back("version");t.valid=t.valid&&t.owner->snapshot().binding.active;::SetLastError(0x5151);if(t.throw_version)std::rethrow_exception(t.failure);return t.version;}
void observe(void* object){auto& t=*current;++t.queries;const auto s=t.owner->snapshot();t.valid=t.valid&&object==reinterpret_cast<void*>(t.object())&&s.binding.active&&!s.binding.object_unknown&&s.binding.object==t.object();if(t.block_query){::SetEvent(t.entered);if(::WaitForSingleObject(t.release,10000)!=WAIT_OBJECT_0)throw std::runtime_error("query release timeout");}::SetLastError(0x7171);if(t.throw_query)std::rethrow_exception(t.failure);}
std::int32_t status(void* object){observe(object);return current->status;}
const char* username(void* object){observe(object);return current->username.c_str();}
const char* persona(void* object){observe(object);return current->persona.c_str();}
const char* uid(void* object){observe(object);return current->uid.c_str();}
std::uint8_t avatar(void* object,const char16_t* path,const char* owner,std::int32_t size){current->avatar_path=path;current->avatar_owner=owner;current->avatar_size=size;observe(object);return 0x80;}
const char* auth(void* object,const char* client){current->auth_client=client;observe(object);return current->auth_value.c_str();}
void destroy(void* object){auto& t=*current;++t.destroys;t.events.push_back("destroy");const auto s=t.owner->snapshot();t.valid=t.valid&&object==reinterpret_cast<void*>(t.object())&&s.binding.active&&s.binding.object_unknown&&s.binding.factory_code_retained;::SetLastError(0x5454);if(t.throw_destroy)std::rethrow_exception(t.failure);}
online::Object factory(const char16_t* cookie){auto& t=*current;++t.factories;t.events.push_back("factory");t.cookie=cookie;const auto s=t.owner->snapshot();t.valid=t.valid&&s.binding.active&&s.binding.object_unknown&&!s.binding.object;::SetLastError(0x5353);if(t.throw_factory)std::rethrow_exception(t.failure);t.table={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&uid),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};const auto table=reinterpret_cast<std::uintptr_t>(t.table.data());std::memcpy(t.bytes.data()+1,&table,8);return t.object();}
FARPROC WINAPI lookup(HMODULE module,LPCSTR name){auto& t=*current;t.valid=t.valid&&reinterpret_cast<std::uintptr_t>(module)==0x80;t.exports.emplace_back(name);if(t.swallowed_lookup){try{t.owner->call_version_check(0);}catch(...){}}const bool v=std::string_view(name)=="version_check";::SetLastError(v?0x4141:0x4242);if(v)return t.version_missing?nullptr:std::bit_cast<FARPROC>(&version);return t.factory_missing?nullptr:std::bit_cast<FARPROC>(&factory);}
struct Os final:recovered::libraries::SystemCalls {
    Trace& t;b12_test::Os inner;explicit Os(Trace& trace):t(trace){}
    std::uint32_t get_error_mode()override{return inner.get_error_mode();}
    std::uint32_t set_error_mode(std::uint32_t mode)override{return inner.set_error_mode(mode);}
    recovered::libraries::Module load_library(const wchar_t* path)override{inner.loads.emplace_back(path);t.events.push_back("load");::SetLastError(0x3131);if(t.throw_load)std::rethrow_exception(t.failure);return t.load_zero?0:0x80;}
    bool free_library(recovered::libraries::Module module)override{t.events.push_back("free");const auto s=t.owner->snapshot().binding;t.valid=t.valid&&s.active&&s.module==module&&!s.procedure&&!s.factory&&!s.object&&s.code_retained;::SetLastError(0x6161);if(t.throw_free)std::rethrow_exception(t.failure);inner.unloaded.push_back(module);return false;}
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* p,int n,char* out,int cap,bool* used)override{return inner.wide_to_narrow(cp,flags,p,n,out,cap,used);}
    std::uint32_t last_error()override{return inner.last_error();}
};
struct Fixture {
    configuration::State config{};bootstrap_backend::RuntimeSeed runtime{};b2_test::Events events;b4_test::Execution platform{events};execution_recovered::Execution execution;
    std::shared_ptr<Trace> trace=std::make_shared<Trace>();b12_test::Ui ui;
    std::shared_ptr<b11_test::OwnedStream> stream=std::make_shared<b11_test::OwnedStream>();
    std::shared_ptr<b13_test::EncodingState> encoding=std::make_shared<b13_test::EncodingState>();
    std::shared_ptr<b13_test::StatsState> stats=std::make_shared<b13_test::StatsState>();
    std::shared_ptr<b8_test::MemoryFilesState> files=std::make_shared<b8_test::MemoryFilesState>();
    Os* os=nullptr;std::unique_ptr<of::Online> owner;
    bootstrap_backend::CallbackStateView view(){return {config,runtime,execution};}
    online::StateRefs refs(){return {config.executable_path,config.cookie_path,config.last_profile_path,config.last_avatar_path,config.avatar,runtime.online_module,runtime.online_object,config.username,config.persona_id,config.user_id,config.use_last_online_profile,runtime.online};}
    of::Options options(){of::Options o;o.lifetime=trace;o.stream=stream;o.dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());auto sdk=std::make_unique<Os>(*trace);os=sdk.get();o.os=std::move(sdk);o.encoding=std::make_unique<b13_test::Encoding>(encoding);o.stats=std::make_unique<b13_test::Stats>(stats);o.files=std::make_unique<b8_test::MemoryFiles>(files);o.get_proc=lookup;
        o.code_lease=[t=trace](online::Module module){++t->leases;t->valid=t->valid&&module==0x80;auto pin=std::shared_ptr<void>(new Pin{*t});t->pins.push_back(pin);::SetLastError(0xeeee);return pin;};
        o.foreign_quiescent=[t=trace]{++t->proofs;if(t->poison_proof){try{t->owner->parent_path_019820(u"x");}catch(...){}}return t->finality;};return o;}
    explicit Fixture(bool construct=true):execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform){
        current=trace.get();b12_test::ui=&ui;execution_recovered::initialize_mutex_04a698(runtime.execution_mutex,2);runtime.frequency_cache=10000000;runtime.server_ready=1;
        config.executable_path=u"C:\\own\\host.exe";config.cookie_path=configuration::Path(48,u'K');config.last_profile_path=u"MEM-profile";config.last_avatar_path=u"MEM-avatar";config.avatar=u"initial-avatar";
        trace->config=&config;trace->runtime=&runtime;if(construct){auto o=options();owner.reset(static_cast<of::Online*>(of::make_online(view(),o).release()));trace->owner=owner.get();}
    }
    void cleanup(){online::cleanup_0366f0(refs(),*owner);}
    ~Fixture(){if(owner){try{if(owner->snapshot().phase==of::Phase::Open){if(runtime.online_module||runtime.online_object)cleanup();trace->finality=true;if(!owner->close_after_cleanup())std::terminate();}owner.reset();}catch(...){std::terminate();}}current=nullptr;b12_test::ui=nullptr;}
};
void original_online_lifetime(){
    of::Options native;::SetLastError(0x7471);of::select_native_original_online(native);require(::GetLastError()==0x7471&&native.code_lease&&!native.foreign_quiescent&&of::lifetime_configuration_valid(native)&&native.lifetime_profile==of::LifetimeProfile::OriginalCrt,"Native original Online selects actual module lease with no external proof callback or module call");
    rejects<std::invalid_argument>([&]{of::select_native_original_online(native);},"Original Online selector cannot replace a selected lifetime/lease");
    Fixture f(false);auto options=f.options();options.foreign_quiescent={};options.lifetime_profile=of::LifetimeProfile::OriginalCrt;
    f.owner.reset(static_cast<of::Online*>(of::make_online(f.view(),options).release()));f.trace->owner=f.owner.get();
    online::worker_035b60(f.refs(),*f.owner);require(f.runtime.online_module&&f.runtime.online_object&&!f.owner->close_after_cleanup()&&!f.trace->proofs,"Original lifetime cannot retire a real live Online module/object");
    f.cleanup();f.trace->finality=false;require(f.owner->close_after_cleanup()&&f.owner->close_after_cleanup()&&!f.trace->proofs&&f.trace->destroys==1&&f.trace->pin_deletes==2,"Actual source destroy/free/code-lease retirement permits local closure without foreign callback substitution");
}
void passive_validation_identity(){
    Fixture f(false);auto o=f.options();require(f.trace->events.empty()&&!f.trace->leases&&!f.trace->proofs&&f.ui.messages.empty(),"preparing owned leaves does not load/query/dialog");
    auto owner=of::make_online(f.view(),o);auto* selected=static_cast<of::Online*>(owner.get());f.trace->owner=selected;
    require(!o.stream&&!o.os&&!o.get_proc&&!o.code_lease&&!o.foreign_quiescent&&!o.lifetime,"named Options sink consumes all owning callable donors");
    const auto snap=selected->snapshot();require(snap.configuration==&f.config&&snap.runtime==&f.runtime&&snap.execution==&f.execution&&selected->matches(f.view()),"exact stable bound config/runtime/Execution identities retained");
    configuration::State foreign{};
    require(!selected->matches({foreign,f.runtime,f.execution}),"foreign configuration identity rejected");bootstrap_backend::RuntimeSeed other_runtime{};require(!selected->matches({f.config,other_runtime,f.execution}),"foreign runtime identity rejected");
    execution_recovered::Execution other_execution(f.runtime.execution_mutex,{f.runtime.server_ready,f.runtime.online_ready,f.runtime.libraries_ready},f.runtime.frequency_cache,f.platform);require(!selected->matches({f.config,f.runtime,other_execution}),"foreign Execution identity rejected");
    owner.reset();require(f.trace->events.empty()&&!f.trace->proofs,"never-used actually empty owner retires without fabricated foreign cleanup/proof");
    using Change=std::function<void(of::Options&)>;const std::vector<Change> bad={[](auto& x){x.stream.reset();},[](auto& x){x.dialog.reset();},[](auto& x){x.os.reset();},[](auto& x){x.encoding.reset();},[](auto& x){x.errors.reset();},[](auto& x){x.stats.reset();},[](auto& x){x.files.reset();},[](auto& x){x.get_proc=nullptr;},[](auto& x){x.code_lease={};},[](auto& x){x.foreign_quiescent={};}};
    for(const auto& change:bad){auto invalid=f.options();change(invalid);rejects<std::invalid_argument>([&]{(void)of::make_online(f.view(),invalid);},"every mandatory owner/SDK/lease/proof input rejected passively");}
    f.runtime.online_module=0x90;auto live=f.options();rejects<std::invalid_argument>([&]{of::make_online(f.view(),live);},"new binding cannot adopt seeded module");f.runtime.online_module=0;f.runtime.online_object=0x91;auto live_object=f.options();rejects<std::invalid_argument>([&]{of::make_online(f.view(),live_object);},"new binding cannot adopt seeded object");f.runtime.online_object=0;
    require(f.trace->events.empty()&&!f.trace->leases,"invalid/live input rejection performs no module operation or guessed free");
}
void exact_routes_and_errors(){
    Fixture f;auto& r=*f.owner;require(r.suppress_errors_0450a0()==0x40,"error suppression returns original OS mode");r.restore_error_mode(0x40);require(f.os->inner.mode==0x40,"explicit normal error-mode restore");
    require(r.filesystem_widen_006d70("name")==u"name"&&r.parent_path_019820(u"C:\\a\\b")==u"C:\\a"&&r.join_016b00(u"C:\\a",u"b")==u"C:\\a\\b","actual filesystem encoding and recovered path leaves");
    f.runtime.online_module=r.load_copy_0450d0(u"C:\\own\\anadius64online.dll");require(f.runtime.online_module==0x80&&::GetLastError()==0x3131,"selected synthetic module load preserves SDK error");
    const auto v=r.get_proc_address(0x80,"version_check");require(v==reinterpret_cast<online::Procedure>(&version)&&::GetLastError()==0x4141&&f.trace->leases==1,"lookup error survives CodeLease mutation");require(r.call_version_check(v)==2&&::GetLastError()==0x5151,"typed version result and actual outgoing error preserved");
    const auto create=r.get_proc_address(0x80,"create_online");require(::GetLastError()==0x4242&&f.trace->leases==2,"factory lookup independently retains callable code/error");f.runtime.online_object=r.call_create_online(create,f.config.cookie_path.c_str());require(f.runtime.online_object==f.trace->object()&&f.trace->cookie==f.config.cookie_path&&::GetLastError()==0x5353,"typed factory adopts exact readable synthetic object");
    require(r.status_virtual_08(f.runtime.online_object)==0&&::GetLastError()==0x7171,"typed status uses active adopted-object reservation");
    require(r.username_virtual_10(f.runtime.online_object)==f.trace->username.c_str()&&r.persona_virtual_18(f.runtime.online_object)==f.trace->persona.c_str()&&r.user_id_virtual_20(f.runtime.online_object)==f.trace->uid.c_str(),"all three raw getters preserve exact borrowed pointer");
    require(r.copy_username(f.runtime.online_object)==f.trace->username&&r.copy_persona(f.runtime.online_object)=="person"&&r.copy_user_id(f.runtime.online_object).empty(),"owned getter copies preserve long/first-NUL/empty boundaries");
    require(r.avatar_virtual_28(f.runtime.online_object,u"cache/utf16-\u0131.png","remote:tail",-2)&&f.trace->avatar_path==u"cache/utf16-\u0131.png"&&f.trace->avatar_owner=="remote:tail"&&f.trace->avatar_size==-2&&::GetLastError()==0x7171,"Production Online forwards all four avatar arguments and consumes nonzero AL");
    require(r.auth_virtual_30(f.runtime.online_object,"client")==f.trace->auth_value.c_str()&&f.trace->auth_client=="client"&&::GetLastError()==0x7171,"Production auth preserves exact borrowed pointer and client argument");
    require(r.copy_auth(f.runtime.online_object,"")==f.trace->auth_value&&f.trace->auth_client.empty()&&::GetLastError()==0x7171,"Production auth owned-copy route preserves empty client and long result with active binding");
    f.stats->values[u"regular"]={0,FILE_ATTRIBUTE_NORMAL,{}};require(r.status_01ea20(u"regular",3).kind==2,"actual status algorithm uses supplied typed stats");
    f.files->inputs[u"binary"]=std::string("a\0b",3);require(r.read_binary_03b3f0(u"binary")==std::string("a\0b",3)&&r.write_binary_03c1d0(u"out",std::string_view("x\0y",3))&&f.files->outputs[u"out"]==std::string("x\0y",3),"actual binary file algorithms preserve counted NUL bytes");
    r.log_literal_006880("authentication ok");r.log_finish_0088e0();require(f.stream->str()=="authentication ok\n"&&f.stream->flushes==1,"retained literal formatted insertion and endl source route");r.dialog_038bc0("title","text",0x10);require(f.ui.messages.size()==1&&f.ui.messages[0].flags==0x10&&f.ui.messages[0].title==L"title","selected dialog SDK receives only owned synthetic strings");
    bool fs_error=false;try{r.filesystem_error_018130("status",5,u"regular");}catch(const std::filesystem::filesystem_error& e){fs_error=e.code().value()==5;}require(fs_error&&r.snapshot().phase==of::Phase::Open,"expected filesystem throw is a source route with balanced admission");
    r.wait_steps_036880(1);require(!f.runtime.online_ready,"real wait checks actual server flag without inventing online ready");r.finish_flag_036820();require(f.runtime.online_ready==1&&f.runtime.server_ready==1&&f.runtime.libraries_ready==0&&!f.events.empty(),"real finish changes exact Runtime online-ready through actual Execution");
    require(!r.close_after_cleanup()&&!f.trace->proofs,"live actual module/object blocks late proof entirely");
    f.cleanup();require(!f.runtime.online_module&&!f.runtime.online_object&&f.trace->destroys==1&&f.trace->pin_deletes==2&&::GetLastError()==0x6161&&f.trace->valid,"actual cleanup destroys before free; FALSE free clears logical prefix while preserving lower error over deleters");
    require(f.trace->events[f.trace->events.size()-2]=="destroy"&&f.trace->events.back()=="free","normal source cleanup preserves destroy/free order");
    f.trace->finality=false;require(!r.close_after_cleanup()&&f.trace->proofs==1&&r.snapshot().phase==of::Phase::Open,"late false proof retains all owners for controller retry");f.trace->finality=true;require(r.close_after_cleanup()&&r.close_after_cleanup()&&f.trace->proofs==2,"true proof seals only final provider/code-owner retirement idempotently");
    rejects<std::logic_error>([&]{r.parent_path_019820(u"a");},"Closed pure-path route rejects before touching leaves");
}
void real_worker_and_query_lease(){
    Fixture f;f.config.use_last_online_profile=1;online::worker_035b60(f.refs(),*f.owner);require(f.runtime.online&&f.runtime.online_ready&&f.config.username==f.trace->username&&f.config.persona_id=="person"&&f.config.user_id.empty(),"actual full online worker commits owned getter fields and actual ready flag");
    require(f.files->outputs[f.config.last_profile_path]=="person;;"+f.trace->username,"actual worker persists counted profile after getter copies");
    f.trace->entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr);f.trace->release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);require(f.trace->entered&&f.trace->release,"own query synchronization events created");f.trace->block_query=true;std::string result;std::exception_ptr failure;
    std::thread worker([&]{try{result=f.owner->copy_username(f.runtime.online_object);}catch(...){failure=std::current_exception();}});
    require(::WaitForSingleObject(f.trace->entered,10000)==WAIT_OBJECT_0&&f.owner->snapshot().binding.active&&f.owner->snapshot().active_calls==1,"real own worker holds Binding reservation through copied getter");require(!f.owner->close_after_cleanup()&&!f.trace->proofs,"active source/getter lease prevents close before callback proof");::SetEvent(f.trace->release);worker.join();if(failure)std::rethrow_exception(failure);f.trace->block_query=false;require(result==f.trace->username&&!f.owner->snapshot().active_calls,"normal getter return balances lease and completes real copy");::CloseHandle(f.trace->entered);::CloseHandle(f.trace->release);f.cleanup();require(f.owner->close_after_cleanup(),"joined getter and actual source cleanup leave component closable");
}
bootstrap_backend::Resources backend_resources(b2_test::Track& t){
    bootstrap_backend::Resources r{};r.configuration=std::make_unique<b2_test::Effects>(t.events);r.files=std::make_unique<b2_test::Files>(t.events);r.input=std::make_unique<b2_test::InputPlatform>(t.events);r.console=std::make_unique<b2_test::Console>(t.events);r.threads=std::make_unique<b3_tracking_test::Lower>();r.remaining=std::make_unique<b2_test::Backend>(t);r.execution=std::make_unique<b4_test::Execution>(t.events);r.server=std::make_unique<b4_test::Server>(t.events);r.library_os=std::make_unique<b4_test::LibraryOS>(t.events);r.library_effects=std::make_unique<b4_test::LibraryEffects>();r.host=std::make_unique<b4_test::Host>(t.events);r.hook_cleanup=std::make_unique<b4_test::Cleanup>(t.events);r.stop_sources=[]{return true;};r.join_api=bootstrap_backend::native_join_api();return r;
}
struct BoundWorker final:bootstrap_backend::ApplicationWorker,application_effects::OnlineBinding {
    bootstrap_backend::OnlineComponent*& observed;unsigned& cancels;
    BoundWorker(bootstrap_backend::OnlineComponent*& p,unsigned& n):observed(p),cancels(n){}
    void bind_effects_online(bootstrap_backend::OnlineComponent& online)override{require(!observed,"Backend lends Online to worker exactly once");observed=&online;}
    void run_server(std::uint16_t&,std::uint8_t&,execution_recovered::Execution&)noexcept override{std::terminate();}
    void startup_complete(execution_recovered::ReadyStage,const execution_recovered::Execution*)noexcept override{}
    void startup_failed(const execution_recovered::Execution*)noexcept override{}
    void request_stop()override{}
    bool finish_after_join()override{return false;}
    bool cancel_unpublished()override{++cancels;require(observed!=nullptr,"Unpublished worker cancels while exact Online remains alive");return true;}
};
void backend_identity_and_install(){
    for(const bool wrong:{false,true}){Fixture f(false);b2_test::Track track;auto resources=backend_resources(track);auto options=f.options();bootstrap_backend::CallbackStateView* observed=nullptr;std::unique_ptr<bootstrap_backend::CallbackStateView> view_copy;
        resources.online_factory=[&](bootstrap_backend::CallbackStateView view){view_copy=std::make_unique<bootstrap_backend::CallbackStateView>(view);observed=view_copy.get();auto made=of::make_online(wrong?f.view():view,options);f.trace->owner=static_cast<of::Online*>(made.get());return made;};
        bootstrap_backend::RuntimeSeed seed{};execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.frequency_cache=10000000;configuration::State actual{};std::uint8_t a=0,b=0,c=0;bootstrap_backend::SourceBackend backend(std::move(seed),bootstrap_backend::take_resources(resources));
        if(wrong)rejects<std::invalid_argument>([&]{backend.bind_state(actual,{a,b,c});},"actual Backend rejects adopted component bound to foreign view");else{backend.bind_state(actual,{a,b,c});const auto snapshot=f.trace->owner->snapshot();require(observed&&f.trace->owner->matches(*observed)&&snapshot.configuration==&actual&&snapshot.runtime==&backend.state()&&snapshot.execution==&observed->execution,"actual Backend factory receives its own stable three-reference view");}
        require(!resources.online_factory&&f.trace->events.empty(),"Backend adopts before donor retirement and performs no module work during binding");
    }
    {
        Fixture f(false);b2_test::Track track;auto resources=backend_resources(track);auto options=f.options();of::install_online(resources,options);
        bootstrap_backend::OnlineComponent* observed=nullptr;unsigned factories=0,cancels=0;
        resources.server.reset();resources.application_worker=[&](bootstrap_backend::CallbackStateView view){++factories;require(&view.configuration!=&f.config,"Worker factory receives actual Backend view");return std::make_unique<BoundWorker>(observed,cancels);};
        bootstrap_backend::RuntimeSeed seed{};execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.frequency_cache=10000000;configuration::State actual{};std::uint8_t a=0,b=0,c=0;
        {bootstrap_backend::SourceBackend backend(std::move(seed),bootstrap_backend::take_resources(resources));backend.bind_state(actual,{a,b,c});auto* concrete=dynamic_cast<of::Online*>(observed);require(factories==1&&concrete&&concrete->snapshot().configuration==&actual&&concrete->snapshot().runtime==&backend.state()&&f.trace->events.empty(),"Backend forwards its actual owned Online to worker without module work or a caller-captured substitute");}
        require(cancels==1,"Backend retires unpublished worker before bound Online");
    }
    Fixture f(false);bootstrap_backend::Resources r{};auto options=f.options();of::install_online(r,options);require(r.online_factory&&!options.code_lease&&!options.foreign_quiescent&&!options.lifetime,"installation publishes one passive owning factory and clears Options donors");auto second=f.options();rejects<std::invalid_argument>([&]{of::install_online(r,second);},"installation refuses mixed target without consuming replacement inputs");require(bool(second.code_lease),"failed mixed install keeps replacement ownership");auto component=r.online_factory(f.view());f.trace->owner=static_cast<of::Online*>(component.get());rejects<std::logic_error>([&]{r.online_factory(f.view());},"installed factory binds exactly once");r.online_factory={};component.reset();
}
void attached_retirement(){
    Fixture f(false);
    struct Result {unsigned retired=0,rejected=0;bool valid=true;};auto result=std::make_shared<Result>();
    struct Probe {
        Trace* trace;std::shared_ptr<Result> result;
        ~Probe(){++result->retired;auto* owner=trace->owner;const auto s=owner->snapshot();result->valid=result->valid&&s.phase==of::Phase::Retiring&&!s.binding.module&&!s.binding.procedure&&!s.binding.factory&&!s.binding.object&&!s.binding.active&&!s.binding.code_retained&&!s.binding.factory_code_retained;
            try{owner->parent_path_019820(u"x");}catch(const std::logic_error&){++result->rejected;}
            try{owner->close_after_cleanup();}catch(const std::logic_error&){++result->rejected;}}
    };
    auto code=std::make_shared<Probe>();code->trace=f.trace.get();code->result=result;
    auto proof=std::make_shared<Probe>();proof->trace=f.trace.get();proof->result=result;
    auto backing=std::make_shared<Probe>();backing->trace=f.trace.get();backing->result=result;
    auto options=f.options();options.lifetime=backing;options.code_lease=[code](online::Module){return std::shared_ptr<void>(code);};options.foreign_quiescent=[proof]{return true;};
    f.owner.reset(static_cast<of::Online*>(of::make_online(f.view(),options).release()));f.trace->owner=f.owner.get();code.reset();proof.reset();backing.reset();
    f.owner->parent_path_019820(u"A\\B");require(f.owner->close_after_cleanup()&&!result->retired,"successful close keeps code-lease factory/predicate/backing captures alive");
    f.owner.reset();require(result->retired==3&&result->rejected==6&&result->valid,"actual Binding/predicate/backing capture destructors probe attached Retiring facade and cached empty Binding after reset");
}
struct ChildExit {const char* mode;int code;};
int child(std::string_view mode){::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);auto* f=new Fixture;auto& t=*f->trace;auto& r=*f->owner;
    if(mode=="proof-reentry"){t.poison_proof=true;const bool closed=r.close_after_cleanup();if(!closed&&r.snapshot().phase==of::Phase::Poisoned&&!r.snapshot().active_calls)::ExitProcess(1301);}
    if(mode=="load"){t.throw_load=true;bool same=false;try{r.load_copy_0450d0(u"synthetic");}catch(...){same=std::current_exception()==t.failure;}if(same&&::GetLastError()==0x3131&&r.snapshot().phase==of::Phase::Poisoned&&r.snapshot().binding.poisoned&&!r.close_after_cleanup())::ExitProcess(1302);}
    f->runtime.online_module=r.load_copy_0450d0(u"synthetic");
    if(mode=="version"){const auto entry=r.get_proc_address(0x80,"version_check");t.throw_version=true;bool same=false;try{r.call_version_check(entry);}catch(...){same=std::current_exception()==t.failure;}if(same&&::GetLastError()==0x5151&&r.snapshot().phase==of::Phase::Poisoned&&!r.snapshot().binding.poisoned&&r.snapshot().binding.module==0x80&&!r.close_after_cleanup())::ExitProcess(1308);}
    if(mode=="swallowed"){t.swallowed_lookup=true;bool caught=false;try{r.get_proc_address(0x80,"version_check");}catch(const std::logic_error&){caught=true;}if(caught&&!t.versions&&r.snapshot().phase==of::Phase::Poisoned&&r.snapshot().binding.code_retained&&!r.close_after_cleanup())::ExitProcess(1303);}
    const auto create=r.get_proc_address(0x80,"create_online");
    if(mode=="factory"){t.throw_factory=true;bool same=false;try{f->runtime.online_object=r.call_create_online(create,u"cookie");}catch(...){same=std::current_exception()==t.failure;}if(same&&r.snapshot().binding.object_unknown&&!f->runtime.online_object&&r.snapshot().phase==of::Phase::Poisoned&&!r.close_after_cleanup())::ExitProcess(1304);}
    f->runtime.online_object=r.call_create_online(create,u"cookie");
    if(mode=="destroy"){t.throw_destroy=true;bool same=false;try{f->cleanup();}catch(...){same=std::current_exception()==t.failure;}if(same&&r.snapshot().binding.object_unknown&&f->runtime.online_object==t.object()&&f->runtime.online_module==0x80&&r.snapshot().phase==of::Phase::Poisoned&&!r.close_after_cleanup())::ExitProcess(1305);}
    if(mode=="query"){t.throw_query=true;bool same=false;try{r.copy_username(f->runtime.online_object);}catch(...){same=std::current_exception()==t.failure;}if(same&&::GetLastError()==0x7171&&r.snapshot().phase==of::Phase::Poisoned&&r.snapshot().binding.object==t.object()&&!r.close_after_cleanup())::ExitProcess(1306);}
    if(mode=="free"){r.destroy_virtual_00(f->runtime.online_object);f->runtime.online_object=0;t.throw_free=true;bool same=false;try{f->cleanup();}catch(...){same=std::current_exception()==t.failure;}if(same&&r.snapshot().binding.poisoned&&f->runtime.online_module==0x80&&r.snapshot().phase==of::Phase::Poisoned&&!r.close_after_cleanup())::ExitProcess(1307);}
    ::ExitProcess(1399);
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{original_online_lifetime();passive_validation_identity();exact_routes_and_errors();real_worker_and_query_lease();backend_identity_and_install();attached_retirement();for(const auto item:{ChildExit{"proof-reentry",1301},{"load",1302},{"swallowed",1303},{"factory",1304},{"destroy",1305},{"query",1306},{"free",1307},{"version",1308}})require(_spawnl(_P_WAIT,argv[0],argv[0],item.mode,static_cast<const char*>(nullptr))==item.code,"isolated unknown prefixes retain exact owners without retry or unwind");std::cout<<"PASS: "<<checks<<" online-factory core checks; actual bound Execution/ready, typed own-EXE exports, source cleanup/leases and eight isolated retained prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
