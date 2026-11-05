#include <winsock2.h>
#include <exception>
#include "worker.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>
namespace {
namespace m=anadius::messages_recovered;namespace a=anadius::application_worker;namespace b=anadius::bootstrap_backend;namespace d=anadius::thread_domain;namespace t=anadius::threads_recovered;namespace s=anadius::socket_recovered;namespace e=anadius::execution_recovered;namespace sp=anadius::session_profile;namespace o=anadius::owned_server;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n'<<std::flush;throw std::runtime_error(text);}}
template<class E=std::logic_error,class F>void rejects(F f,const char* text){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,text);}
struct Ledger {
 std::mutex gate;std::map<void*,std::size_t> blocks;std::atomic<std::size_t> allocations=0,frees=0;std::atomic<bool> valid=true;
 std::size_t size(){std::lock_guard lock(gate);return blocks.size();}
 bool contains(void* p){std::lock_guard lock(gate);return blocks.contains(p);}
};
Ledger* ledger=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T size){auto* p=::HeapAlloc(heap,flags,size);const auto error=::GetLastError();if(p){std::lock_guard lock(ledger->gate);if(!ledger->blocks.emplace(p,size).second)ledger->valid=false;++ledger->allocations;}::SetLastError(error);return p;}
BOOL WINAPI free_heap(HANDLE heap,DWORD flags,LPVOID p){BOOL result;DWORD error;{std::lock_guard lock(ledger->gate);result=::HeapFree(heap,flags,p);error=::GetLastError();if(result){if(ledger->blocks.erase(p)!=1)ledger->valid=false;++ledger->frees;}}::SetLastError(error);return result;}
std::atomic<bool> module_valid=true;
BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* result){if(flags!=4||!address||!result)module_valid=false;if(result)*result=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
anadius::thread_native::Api thread_api(){auto api=anadius::thread_native::native_api();api.get_module=no_module;return api;}
d::Seed seed(){auto identity=std::make_shared<std::array<std::byte,8>>();return{0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity};}
struct Event {
 HANDLE handle=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
 Event(){require(handle!=nullptr,"Own worker coordination event created");}
 ~Event(){if(handle)::CloseHandle(handle);}
 void set(){require(::SetEvent(handle)!=0,"Own worker coordination event signaled");}
 void wait(){require(::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0,"Own worker coordination event observed within finite bound");}
};
struct Task {
 std::function<void()> function;std::exception_ptr error;HANDLE handle=nullptr;std::uint32_t id=0;
 static unsigned __stdcall run(void* raw){auto& self=*static_cast<Task*>(raw);try{self.function();}catch(...){self.error=std::current_exception();}return 63;}
 void launch(anadius::bootstrap_providers::NativeThreads& effects,std::function<void()> f){function=std::move(f);const auto result=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&run),reinterpret_cast<std::uintptr_t>(this),0,&id,effects);handle=reinterpret_cast<HANDLE>(result);require(handle&&id,"Actual Domain NativeThreads source creates own native worker");}
 void join(){require(handle&&::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0,"Actual native worker handle signals after source wrapper/FLS callbacks");DWORD code=0;require(::GetExitCodeThread(handle,&code)&&code==63,"Actual source worker exits with selected own callback return");require(::CloseHandle(handle)!=0,"Own original worker handle closed after join");handle=nullptr;if(error)std::rethrow_exception(error);}
};
struct NativeFixture {
 Ledger heap;std::shared_ptr<d::DomainStorage> domain;std::unique_ptr<anadius::bootstrap_providers::NativeThreads> threads;
 NativeFixture(){if(ledger)throw std::logic_error("Overlapping handoff fixtures");ledger=&heap;auto api=d::Apis{};api.heap.heap_alloc=allocate;api.heap.heap_free=free_heap;domain=d::DomainStorage::create(seed(),api);require(domain->initialize(),"Real own Domain initializes heap/FLS/initial locale/PTD graph");require(domain->current_ptd()&&::FlsGetValue(domain->native_fls_index())==domain->current_ptd(),"Controller has exact real native Domain FLS/PTD");threads=domain->make_threads(thread_api());}
 void close(){threads.reset();require(domain->provider_leases()==0,"All actual thread and prepared-memory provider leases retired on controller");require(domain->close()&&heap.valid&&heap.size()==0&&heap.allocations==heap.frees,"Actual main+worker PTD/Context/application heap allocations balance after FLS exit");require(domain->locale_references()==1&&domain->multibyte_references()==0&&module_valid,"Real locale counters return to initial baselines and own-EXE module boundary stayed valid");domain.reset();ledger=nullptr;}
};
struct ExecutionPlatform final:e::Platform {
 std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}void acquire_srw_exclusive(std::uintptr_t& v)override{::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&v));}void release_srw_exclusive(std::uintptr_t& v)override{::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&v));}std::int64_t query_performance_frequency()override{LARGE_INTEGER v{};if(!::QueryPerformanceFrequency(&v))throw std::runtime_error("Own QPF");return v.QuadPart;}std::int64_t query_performance_counter()override{LARGE_INTEGER v{};if(!::QueryPerformanceCounter(&v))throw std::runtime_error("Own QPC");return v.QuadPart;}void sleep_milliseconds(std::uint32_t n)override{::Sleep(n);}[[noreturn]]void raise_thread_error_04a648(int)override{throw std::runtime_error("Own Execution terminal");}
};
struct State:std::enable_shared_from_this<State> {
 std::uint8_t show_lsx=0;
 NativeFixture native;anadius::configuration::State config{};b::RuntimeSeed runtime{};ExecutionPlatform platform;e::Execution execution;
 Event armed;std::atomic<unsigned> configured=0,startups=0,cleanups=0,socket_closes=0,port_closes=0,gate_deletes=0,accepts=0,gets=0,posts=0,relays=0,cancels=0,external=0,observed=0,retired_markers=0,retirement_calls=0;std::atomic<bool> retirement_valid=false;std::atomic<a::Inputs*> inputs=nullptr;std::atomic<OVERLAPPED*> accept_ov=nullptr;
 std::weak_ptr<void> marker;std::function<void(a::Inputs&)> configure_hook;std::function<void()> retirement_hook,dispatch_hook;std::function<bool()> close_hook;
 std::atomic<const anadius::license_recovered::AuthenticationCode*> authentication=nullptr;
 std::atomic<unsigned> authentication_binds=0,authentication_retirements=0;
 std::vector<s::Socket> closed_sockets;
 State():execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform){e::initialize_mutex_04a698(runtime.execution_mutex,2);config.username="controller-config";}
};
State* selected=nullptr;
[[noreturn]]void unused(){throw std::logic_error("Unselected Worker core SDK/effect");}
int WSAAPI startup(WORD version,LPWSADATA){require(version==MAKEWORD(2,2),"Actual server prefix requests WSA2.2 at injected boundary");++selected->startups;return 0;}int WSAAPI cleanup(){++selected->cleanups;return 0;}
int WSAAPI local_name(SOCKET socket,sockaddr* address,int* size){require(socket==200&&*size>=28,"Actual source local endpoint receives selected listener");std::memset(address,0,*size);address->sa_family=2;reinterpret_cast<unsigned char*>(address)[2]=0x30;reinterpret_cast<unsigned char*>(address)[3]=0x39;*size=28;return 0;}
BOOL WINAPI environment(LPCSTR key,LPCSTR value){require(std::string_view(key)=="EALsxPort"&&std::string_view(value)=="12345","Actual server029ea0 publishes converted selected port before readiness/Accept");return FALSE;}
VOID WINAPI delete_gate(LPCRITICAL_SECTION p){++selected->gate_deletes;::DeleteCriticalSection(p);}HANDLE WINAPI create_port(HANDLE file,HANDLE prior,ULONG_PTR key,DWORD n){require(file==INVALID_HANDLE_VALUE&&!prior&&!key&&n==0xffffffffU,"Injected IOCP constructor receives actual source arguments");return reinterpret_cast<HANDLE>(0x900);}BOOL WINAPI close_port(HANDLE p){require(p==reinterpret_cast<HANDLE>(0x900),"Only selected synthetic IOCP handle reaches close");++selected->port_closes;return TRUE;}
int WINAPI convert(UINT page,DWORD flags,LPCCH text,int n,LPWSTR out,int count){require(page==0&&!flags&&std::string_view(text)=="127.0.0.1"&&n==-1&&count==10,"Actual Policy parses loopback through exact ACP SDK boundary");std::memcpy(out,L"127.0.0.1",10*sizeof(wchar_t));return 10;}
INT WSAAPI parse(LPWSTR,INT family,LPWSAPROTOCOL_INFOW,LPSOCKADDR out,LPINT count){if(family==23){::WSASetLastError(10022);return SOCKET_ERROR;}require(family==2&&*count==128,"IPv4 fallback uses initialized SDK sockaddr capacity");std::memset(out,0,*count);out->sa_family=2;*count=16;return 0;}
int WSAAPI send_sync(SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){unused();}int WSAAPI select_write(int,fd_set*,fd_set*,fd_set*,const timeval*){unused();}
BOOL PASCAL accept_sdk(SOCKET listener,SOCKET accepted,PVOID buffer,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED ov){require(listener==200&&accepted==201&&buffer&&!receive&&local==0x90&&remote==0x90&&ov,"Real Runtime arms one native accept with exact owned buffer and source endpoint sizes");selected->accept_ov=ov;++selected->accepts;*bytes=1234;selected->armed.set();::WSASetLastError(997);return FALSE;}
VOID PASCAL addresses(PVOID,DWORD,DWORD,DWORD,sockaddr**,int*,sockaddr**,int*){unused();}int WSAAPI context(SOCKET,int,int,const char*,int){unused();}
SOCKET WSAAPI socket_factory(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){require(family==2&&type==1&&protocol==6&&!info&&!group&&flags==1,"Actual pending factory uses exact selected source socket arguments");return 201;}
BOOL WINAPI cancel(HANDLE,LPOVERLAPPED){++selected->cancels;return TRUE;}
struct Sockets final:s::AcceptorApi {
 s::Socket create_socket(std::int32_t family)override{require(family==2,"Source opens IPv4 listener after injected IPv6 parse failure");return 200;}std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require(socket==200&&port==0x900,"Listener binds to selected owned completion identity");return port;}std::uint32_t get_win32_error()override{return ::GetLastError();}std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t option,std::int32_t value)override{require(socket==200&&level==65535&&option==4&&value==1,"Source reuse-address option forwarded exactly");return 0;}std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t size)override{require(socket==200&&size==16,"IPv4 source bind uses16-byte sockaddr");return 0;}std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{require(socket==200&&backlog==INT_MAX,"Source server listener uses INT_MAX backlog");return 0;}std::int32_t close_socket(s::Socket socket)override{selected->closed_sockets.push_back(socket);++selected->socket_closes;return 0;}std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Dispatcher final:s::DispatcherApi {
 DWORD error=0;void set_last_error(std::uint32_t n)override{error=n;}std::uint32_t get_last_error()override{return error;}
 std::int32_t get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& op,std::uint32_t)override{require(port==0x900&&selected->accepts==1,"Physical dispatch starts only after actual armed accept");const auto n=++selected->gets;if(selected->dispatch_hook)selected->dispatch_hook();bytes=0;key=0;if(n==1){op=selected->accept_ov.load();error=995;return 0;}require(n==2&&selected->posts==1,"Second actual physical wait consumes only the emitted stop relay");require(selected->relays.fetch_sub(1)==1,"Physical SDK driver consumes first actual posted stop relay");op=nullptr;error=0;return 1;}
 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* op)override{require(port==0x900&&!bytes&&!key&&!op,"No-session stop posts only selected empty relay");++selected->posts;++selected->relays;return 1;}std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct CancelEffects final:s::CancellationEffects {void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{++selected->cancels;}};
struct Executor final:s::RegistryExecutorEffects {~Executor(){if(selected&&selected->retirement_hook){++selected->retirement_calls;selected->retirement_hook();}}void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}};
void move_executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t table){require(table==0x180084928ULL,"Pending source factory uses selected executor move identity");to.context=from.context;to.zero_74=from.zero_74;to.self_reference_80=true;}
struct Information final:anadius::handlers_recovered::Effects {
    void log_profile_name_084f48(std::string_view text)override{(void)text;unused();}
    std::string current_time_041b60()override{return "2000-01-01T00:00:00";}
};
struct ContentEffects final:anadius::content_handlers_recovered::Effects {
    std::int32_t atoi_05ee08(std::string_view)override{unused();}
    void unknown_entitlement_field(std::string_view)override{unused();}
    std::string utc_time_041b60(std::uint64_t&)override{unused();}
    std::unique_ptr<anadius::content_handlers_recovered::DocumentWriter> printer_0019b0()override{unused();}
    void write_xml_03c1d0(const anadius::content_handlers_recovered::Path&,std::string_view)override{unused();}
    void save_steam_01f9a0(const anadius::content_handlers_recovered::AchievementMap&)override{unused();}
    anadius::content_handlers_recovered::Path filesystem_widen_006d70(std::string_view)override{unused();}
    anadius::content_handlers_recovered::Path path_append_016b50(const anadius::content_handlers_recovered::Path&,const anadius::content_handlers_recovered::Path&)override{unused();}
    bool online_avatar_virtual_28(const anadius::content_handlers_recovered::Path&)override{unused();}
    bool copy_overwrite_048d20(const anadius::content_handlers_recovered::Path&,const anadius::content_handlers_recovered::Path&)override{unused();}
    std::string path_to_utf8_031ca0(const anadius::content_handlers_recovered::Path&)override{unused();}
    void log_saved_size_085530(std::int32_t)override{unused();}
};
struct LicenseBoundary final:anadius::dispatch_recovered::LicenseHandlers,anadius::license_recovered::AuthenticationBinding {
    unsigned calls=0;
    const anadius::license_recovered::AuthenticationCode* authentication=nullptr;
    void bind_authentication_code(const anadius::license_recovered::AuthenticationCode& code)override{
        require(!authentication,"Worker binds configured licensing provider once before Graph creation");
        authentication=&code;selected->authentication=&code;++selected->authentication_binds;
    }
    ~LicenseBoundary()override{if(authentication){require(authentication->value()=="SGV5LCB5b3UsIHN0b3AgcmlnaHQgdGhlcmUhIDsp","Root auth cell outlives actual worker Graph/licensing retirement");++selected->authentication_retirements;}}
    bool auth_code_02e3b0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool request_license_0320c0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool invalidate_license_030c50(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
};

void configure(State& state,a::Inputs& input){
 ++state.configured;state.inputs=&input;require(!input.graph.state.configuration&&!input.graph.state.online&&!input.graph.state.saved_avatar_size&&!input.graph.state.achievement_file&&!input.graph.state.lifetime&&!input.runtime.tls_index,"Opaque configure receives empty protected graph StateView/TLS fields before Worker installs exact live references");
 if(state.configure_hook)state.configure_hook(input);
 auto& g=input.graph;g.policy.stream=anadius::shared_log::make_stream(std::make_shared<std::stringbuf>());g.policy.imports={::WSASetLastError,::WSAGetLastError,convert,parse,send_sync,select_write};g.information=std::make_shared<Information>();g.content=std::make_shared<ContentEffects>();g.licensing=std::make_shared<LicenseBoundary>();g.profile.sessions=1;g.profile.jobs_capacity=2;g.profile.headers=32;g.profile.environment_capacity=128;

 g.policy.lifecycle.lifetime=state.shared_from_this();
 g.policy.lifecycle.select_owner=[&state](sp::Application& app,s::IocpSchedulerEntry*,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){auto ports=app.ports();require(ports.accept.contains(&op)&&error.value==995&&!bytes&&!app.snapshot().live_sessions,"Selected actual observed accept995 uses owner0 without fabricated accepted session");++state.observed;return static_cast<s::IocpSchedulerEntry*>(nullptr);};
 g.policy.lifecycle.after=[](sp::Application& app){require(!app.snapshot().live_sessions&&!app.snapshot().live_jobs,"No-session actual completion retires without message/job work");};
 g.policy.lifecycle.returned=[](sp::Application& app,std::uint64_t count,const s::WorkerRunResult& result){require(count==1&&result.error&&result.error->value==0&&!app.snapshot().live_sessions&&!app.snapshot().live_jobs,"Actual source worker returns after exactly one selected observed owner0 packet");};
 g.policy.lifecycle.external_drained=[&state](o::ServerView view,sp::Application& app){bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}require(outside&&state.observed==1&&state.gets==2&&state.posts==2&&!app.snapshot().live_sessions&&!app.snapshot().live_jobs,"Selected lower external proof observes exact consumed995 and stop relay outside Parent operation");const auto prior=state.external.fetch_add(1);require(state.relays.exchange(0)==(prior?0U:1U),"Selected lower consumes source-posted relay once; later proofs recheck empty injected SDK queue");return state.close_hook?state.close_hook():true;};
 auto sockets=std::make_shared<Sockets>();auto dispatcher=std::make_shared<Dispatcher>();auto& server=input.server;server.state_lifetime=state.shared_from_this();server.sockets=sockets;server.dispatcher=dispatcher;s::IocpConstructorImports iocp;iocp.create_completion_port=create_port;iocp.close_handle=close_port;server.iocp=std::make_shared<s::NativeIocpConstructorApi>(iocp);server.gates=std::make_shared<s::NativeServiceGateApi>();server.cancellation=std::make_shared<CancelEffects>();server.executor=std::make_shared<Executor>();server.kernel={startup,cleanup,local_name,environment,delete_gate};
 auto& runtime=input.runtime;runtime.state_lifetime=state.shared_from_this();runtime.sockets=sockets;runtime.dispatcher=dispatcher;runtime.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,addresses,context,::WSAGetLastError});runtime.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{socket_factory});runtime.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel);runtime.executor={nullptr,move_executor};runtime.capacity=16;
}
a::Settings settings(const std::shared_ptr<State>& state){a::Settings value;value.lifetime=state;value.configure=[state](a::Inputs& in){configure(*state,in);};value.state=a::Settings::StateSeed{17,u"core-host-seed"};return value;}
std::unique_ptr<a::Worker> make_worker(const std::shared_ptr<State>& state,a::Settings opts){return a::Worker::create(state->native.domain,{state->config,state->runtime,state->execution,&state->show_lsx},std::move(opts));}
void finish_state(std::shared_ptr<State>& state){state->native.close();state.reset();selected=nullptr;}
void passive_validation(){auto state=std::make_shared<State>();selected=state.get();auto base=settings(state);auto bad=[&](auto edit){auto copy=base;edit(copy);rejects<std::invalid_argument>([&]{auto worker=make_worker(state,std::move(copy));},"Incomplete explicit Worker settings rejected before native worker/Graph/SDK startup");};bad([](auto& x){x.lifetime.reset();});bad([](auto& x){x.configure={};});bad([](auto& x){x.state.reset();});bad([](auto& x){x.startup_timeout_ms=0;});bad([](auto& x){x.startup_timeout_ms=INFINITE;});rejects<std::invalid_argument>([&]{(void)a::Worker::create(nullptr,{state->config,state->runtime,state->execution},base);},"Worker requires ready controller-owned Domain");
 bad([](auto& x){x.close_retry_timeout_ms=INFINITE;});
 auto worker=make_worker(state,base);auto snap=worker->snapshot();require(snap.phase==a::Phase::Prepared&&!snap.worker_thread&&!snap.pending_retained&&!state->configured&&!state->startups&&snap.memory.phase==anadius::worker_handoff::Phase::Prepared&&!snap.close_epoch&&!snap.close_retries&&!worker->retry_close(1),"Concrete Worker creation owns only controller-prepared memory; close retry is unavailable before a current wait");
 require(worker->cancel_unpublished()&&worker->snapshot().phase==a::Phase::Closed&&worker->finish_after_join(),"Known never-started worker can explicitly cancel and finish without fabricated OS exit");worker.reset();base={};finish_state(state);
}
struct ComposeOrder {bool composer_alive=false,valid=true;unsigned capture_retired=0,composer_retired=0;};
struct OrderedComposer final:anadius::application_effects::Composer {
 ComposeOrder& order;explicit OrderedComposer(ComposeOrder& o):order(o){order.composer_alive=true;}
 ~OrderedComposer()override{order.composer_alive=false;++order.composer_retired;}
 void compose(anadius::application_effects::Providers&,const anadius::application_effects::Context&)override{unused();}
};
struct OrderedCapture {ComposeOrder& order;~OrderedCapture(){++order.capture_retired;order.valid&=order.composer_alive;}};
void composition_constructor_failure(){
 auto state=std::make_shared<State>();selected=state.get();auto closed=state->native.domain;state->native.threads.reset();require(closed->close(),"Known Domain can be closed before selected Worker construction failure");
 ComposeOrder order;auto opts=settings(state);opts.effects=std::make_shared<OrderedComposer>(order);auto capture=std::shared_ptr<OrderedCapture>(new OrderedCapture{order});opts.configure=[capture](a::Inputs&){unused();};capture.reset();
 rejects([&]{(void)a::Worker::create(closed,{state->config,state->runtime,state->execution},a::take_settings(opts));},"Closed Domain rejects PreparedMemory after owning Settings adoption");
 require(order.valid&&order.capture_retired==1&&order.composer_retired==1&&!opts.configure&&!opts.effects&&!opts.lifetime&&!state->configured&&!state->startups,"Failed Impl construction retires configure before Composer and clears all callable ownership donors");
 opts={};closed.reset();finish_state(state);
}
void startup_gate_normal(bool host_first){anadius::license_recovered::AuthenticationCode authentication;e::MutexState achievement_mutex{};e::initialize_mutex_04a698(achievement_mutex,2);auto state=std::make_shared<State>();selected=state.get();auto worker=make_worker(state,settings(state));
 state->show_lsx=7;state->configure_hook=[](a::Inputs& in){in.graph.show_lsx=3;};
 if(host_first){
  e::MutexState invalid{};rejects<std::invalid_argument>([&]{worker->bind_achievement_mutex(invalid,state->platform);},"Worker rejects noninitialized mutex before effect");
  std::atomic<bool> wrong_thread=false;std::thread other([&]{try{worker->bind_achievement_mutex(achievement_mutex,state->platform);}catch(const std::logic_error&){wrong_thread=true;}});other.join();require(wrong_thread,"Worker achievement binding requires original controller thread");
  worker->bind_achievement_mutex(achievement_mutex,state->platform);rejects([&]{worker->bind_achievement_mutex(achievement_mutex,state->platform);},"Worker rejects repeated achievement binding");
 }
 if(host_first){worker->bind_authentication_code(authentication);rejects([&]{worker->bind_authentication_code(authentication);},"Worker rejects repeat auth binding in Prepared");}
 require(!state->authentication_binds,"Prepared binding invokes no licensing/provider callback");
 Task task;Event exited;task.launch(*state->native.threads,[&]{worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);exited.set();});state->armed.wait();
 require(host_first?(state->authentication==&authentication&&state->authentication_binds==1):(!state->authentication&&state->authentication_binds==0),"Actual Graph prefix selects exact bound auth identity; unbound legacy route remains unchanged");
 rejects([&]{worker->bind_achievement_mutex(achievement_mutex,state->platform);},"Waiting Worker rejects late achievement mutex binding");
 rejects([&]{worker->bind_authentication_code(authentication);},"Serving/waiting Worker rejects late binding");
 for(unsigned i=0;i<5000&&worker->snapshot().phase!=a::Phase::WaitingStartup;++i)::Sleep(1);auto waiting=worker->snapshot();require(waiting.phase==a::Phase::WaitingStartup&&state->config.server_port==12345&&state->runtime.server_ready==1&&state->accepts==1&&state->gets==0,"Actual029ea0 publishes ready/port and arms Accept before controller startup barrier; no physical dispatch occurred");
 auto* input=state->inputs.load();require(input&&input->graph.state.configuration==&state->config&&input->graph.state.online==&state->runtime.online&&input->graph.state.saved_avatar_size&&*input->graph.state.saved_avatar_size==17&&input->graph.state.achievement_file&&*input->graph.state.achievement_file==u"core-host-seed"&&input->runtime.tls_index&&*input->runtime.tls_index!=TLS_OUT_OF_INDEXES,"Worker installs exact live CallbackStateView and owns fresh TLS selector only after opaque configure returns");
 require(input->graph.show_lsx==3,"Explicit Worker keeps its configured ShowLSX setting despite a different live Runtime flag");
 const auto first=host_first?e::ReadyStage::Host:e::ReadyStage::Online;const auto second=host_first?e::ReadyStage::Online:e::ReadyStage::Host;worker->startup_complete(first,&state->execution);require(::WaitForSingleObject(exited.handle,20)==WAIT_TIMEOUT&&state->gets==0,"One genuine identity-matched milestone is insufficient for dispatch even with server_ready already1");auto half=worker->snapshot();require((host_first?half.host_complete:half.online_complete)&&!(host_first?half.online_complete:half.host_complete),"Only explicitly signaled startup stage is recorded");
 state->runtime.online=1;worker->startup_complete(second,&state->execution);exited.wait();task.join();if(task.error)std::rethrow_exception(task.error);auto returned=worker->snapshot();require(returned.phase==a::Phase::Returned&&returned.memory.phase==anadius::worker_handoff::Phase::Returned&&state->configured==1&&state->external==1&&state->socket_closes==2&&state->port_closes==1&&state->gate_deletes==2&&state->cleanups==1,"Both genuine milestones permit actual native worker/no-session995 drain; Graph/Resources/Inputs retire on original worker before memory Returned");
 require(worker->finish_after_join()&&worker->snapshot().phase==a::Phase::Closed&&worker->snapshot().memory.worker_exit_verified,"Controller finishes only after actual Domain NativeThreads/FLS exit and independent memory self-handle join");
 require(state->authentication_retirements==(host_first?1U:0U),"Bound licensing provider retired on worker before joined controller retirement");
 require(achievement_mutex.type==2&&achievement_mutex.owner_48==0xffffffffU&&!achievement_mutex.count_4c&&!achievement_mutex.srw_10,"No-session actual worker Graph leaves shared mutex idle through retirement");
 rejects([&]{worker->bind_achievement_mutex(achievement_mutex,state->platform);},"Closed Worker rejects achievement binding");
 rejects([&]{worker->bind_authentication_code(authentication);},"Closed Worker rejects auth binding");worker.reset();state->configure_hook={};finish_state(state);
}

[[noreturn]]void original_dispatch_without_startup_milestones(){
 auto state=std::make_shared<State>();selected=state.get();auto opts=settings(state);
 auto configure_base=std::move(opts.configure);opts.configure={};
 opts.configure=[configure_base=std::move(configure_base)](a::Inputs& in){
  configure_base(in);auto& lifecycle=in.graph.policy.lifecycle;
  lifecycle.select_owner={};lifecycle.after={};lifecycle.returned={};lifecycle.external_drained={};lifecycle.lifetime.reset();
  lifecycle.mode=anadius::session_policy::LifecycleMode::OriginalApplication;
 };
 opts.startup_timeout_ms=500;
 auto worker=make_worker(state,std::move(opts));Task task;
 // Set after passive construction, before native launch: original Worker must
 // read the actual startup cell at configure time rather than cache initial0.
 state->show_lsx=7;
 state->dispatch_hook=[&]{
  const auto active=worker->snapshot();
  require(active.phase==a::Phase::Serving&&!active.online_complete&&!active.host_complete&&active.pending_retained&&state->runtime.server_ready==1&&state->config.server_port==12345&&state->gets==1&&state->accepts==1,"Original application reaches actual first physical dispatch without Host/Online startup notifications");
  require(::FlsGetValue(state->native.domain->native_fls_index())&&!state->external&&!state->socket_closes&&!state->port_closes&&!state->cleanups,"Observation retains live source/PTD/IO owners without fabricating completion or cleanup");
  require(state->inputs.load()&&state->inputs.load()->graph.show_lsx==7,"Original Worker propagates late startup ShowLSX byte to the actual Graph options before first dispatch");
  // This fixture's normal terminal995 path requires an explicit null-owner
  // selector. OriginalApplication forwards the real owner and rearms accept.
  // End this isolated observation without pretending the server retired.
  ::ExitProcess(825);
 };
 task.launch(*state->native.threads,[&]{worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);});
 task.join();throw std::runtime_error("Original dispatch observation was not reached");
}

void close_continuation(std::string_view mode){
 auto state=std::make_shared<State>();selected=state.get();auto opts=settings(state);
 opts.close_retry_timeout_ms=mode=="close-disabled"?0U:mode=="close-timeout"?200U:5000U;
 DWORD original=0;void* fiber=nullptr;void* ptd=nullptr;
 state->close_hook=[&]{
  const auto here=::GetCurrentThreadId();auto* current=::FlsGetValue(state->native.domain->native_fls_index());
  if(!original){original=here;fiber=::GetCurrentFiber();ptd=current;}
  require(current&&original==here&&fiber==::GetCurrentFiber()&&ptd==current,"Every external close retry retains exact original worker/fiber/Domain PTD");
  if(mode=="close-unknown")throw o::QuiescenceDeferred();
  return mode=="close-normal"&&state->external>=3;
 };
 auto worker=make_worker(state,std::move(opts));Task task;
 task.launch(*state->native.threads,[&]{worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);});state->armed.wait();
 worker->startup_complete(e::ReadyStage::Online,&state->execution);worker->startup_complete(e::ReadyStage::Host,&state->execution);
 auto waiting=[&](std::uint64_t epoch){
  for(unsigned i=0;i<3000;++i){const auto snap=worker->snapshot();if(snap.phase==a::Phase::WaitingClose&&snap.close_epoch==epoch)return snap;if(snap.phase==a::Phase::Poisoned)break;::Sleep(1);}
  ::ExitProcess(825); // never unwind owners while our native thread may be live
 };
 if(mode=="close-normal"||mode=="close-poison"){
  const auto first=waiting(1);
  require(first.pending_retained&&first.worker_thread==task.id&&first.memory.phase==anadius::worker_handoff::Phase::Borrowed&&!first.close_retries&&state->closed_sockets==std::vector<s::Socket>{201}&&!state->port_closes&&!state->cleanups,"Known false drain retains worker/providers/TLS/listener after observed accept995 retires only pending socket201");
  require(!worker->finish_after_join()&&!worker->cancel_unpublished(),"Controller cannot finish or cancel a started waiting close attempt");
  if(mode=="close-poison")worker->startup_failed(&state->execution);
  else{
   ::SetLastError(0x48d1);require(!worker->retry_close(0)&&!worker->retry_close(2)&&::GetLastError()==0x48d1,"Zero/future close epoch rejects without wake or LastError loss");
   std::atomic<bool> wrong_thread=false;std::thread other([&]{try{(void)worker->retry_close(1);}catch(const std::logic_error&){wrong_thread=true;}});other.join();require(wrong_thread,"Close continuation signal belongs to exact original controller");
   worker->startup_complete(e::ReadyStage::Online,&state->execution);worker->request_stop();
   require(::WaitForSingleObject(task.handle,20)==WAIT_TIMEOUT&&worker->snapshot().close_epoch==1&&state->external==1,"Startup/stop wake cannot authorize close retry or replay external proof");
   require(!worker->prepare_join(),"Join preparation signals current close epoch while requiring a later native join turn");
   const auto second=waiting(2);require(second.close_retries==1&&state->external==2&&!worker->retry_close(1),"A second false result publishes a new epoch; stale first retry cannot wake it");
   require(worker->retry_close(2),"Second exact epoch authorizes final known drain recheck");
  }
 }
 task.join();const auto result=worker->snapshot();
 if(mode=="close-normal"){
  require(result.phase==a::Phase::Returned&&!result.pending_retained&&result.close_epoch==2&&result.close_retries==2&&state->configured==1&&state->startups==1&&state->accepts==1&&state->external==3&&state->closed_sockets==std::vector<s::Socket>{201,200}&&state->port_closes==1&&state->cleanups==1,"Full concrete Worker resumes teardown; socket201 closes once for accept995 and listener200 only after final drain proof, without startup/IO replay");
  require(!worker->retry_close(2)&&worker->finish_after_join()&&worker->snapshot().memory.worker_exit_verified&&!worker->retry_close(2),"Close signal cannot replace actual native/FLS join or resurrect returned/closed Worker");worker.reset();state->close_hook={};finish_state(state);return;
 }
 require(result.phase==a::Phase::Poisoned&&result.pending_retained&&state->external==1&&state->closed_sockets==std::vector<s::Socket>{201}&&!state->port_closes&&!state->cleanups&&!worker->retry_close(result.close_epoch)&&!worker->finish_after_join(),"Timeout/disabled/opaque/poisoned close retains listener/providers after consumed accept995; no teardown or retry after native return");
 if(mode=="close-timeout"){require(result.close_epoch==1&&!result.close_retries,"Close budget expiration does not invent controller retry");::ExitProcess(821);}
 if(mode=="close-disabled"){require(!result.close_epoch,"Unselected close continuation keeps terminal deferral behavior");::ExitProcess(822);}
 if(mode=="close-unknown"){require(!result.close_epoch,"Opaque exception with deferral type cannot enter close wait");::ExitProcess(823);}
 if(mode=="close-poison")::ExitProcess(824);::ExitProcess(826);
}
struct RouteRecord {
 const std::uint8_t* show_lsx=nullptr;
 unsigned factories=0,runs=0,requests=0,finishes=0,cancels=0,failures=0,probes=0;bool identities=true,finish_allowed=false,active_refused=false;anadius::configuration::State* config=nullptr;b::RuntimeSeed* runtime=nullptr;e::Execution* execution=nullptr;b::SourceBackend* backend=nullptr;std::vector<e::ReadyStage> stages;std::vector<std::string> order;std::function<void()> run_hook;
};
RouteRecord* route_record=nullptr;
unsigned __stdcall own_probe(void*){++route_record->probes;return 0;}
struct RouteWorker:b::ApplicationWorker {
 RouteRecord& record;explicit RouteWorker(RouteRecord& r):record(r){}
 void run_server(std::uint16_t& port,std::uint8_t& ready,e::Execution& execution)noexcept override{++record.runs;record.identities&=&port==&record.config->server_port&&&ready==&record.runtime->server_ready&&&execution==record.execution;port=0x4321;try{execution.finish_flag_036820(ready);if(record.run_hook)record.run_hook();}catch(...){record.identities=false;}}
 void startup_complete(e::ReadyStage stage,const e::Execution* execution)noexcept override{record.identities&=execution==record.execution;if(stage==e::ReadyStage::Online)record.identities&=record.runtime->online_ready==1;record.stages.push_back(stage);}
 void startup_failed(const e::Execution* execution)noexcept override{++record.failures;record.identities&=execution==record.execution;}
 void request_stop()override{++record.requests;record.order.emplace_back("worker-stop");}bool finish_after_join()override{++record.finishes;record.order.emplace_back("worker-finish");return record.finish_allowed;}bool cancel_unpublished()override{++record.cancels;record.order.emplace_back("worker-cancel");return true;}
};
struct JoinRouteWorker final:RouteWorker,b::ApplicationJoinContinuation {
 bool& allowed;unsigned& calls;
 JoinRouteWorker(RouteRecord& r,bool& a,unsigned& c):RouteWorker(r),allowed(a),calls(c){}
 bool prepare_join()override{++calls;record.order.emplace_back("join-prepare");return allowed;}
};
void backend_join_continuation(){
 b4_test::Fixture fixture;RouteRecord record;route_record=&record;bool allowed=false;unsigned calls=0;
 auto resources=fixture.resources();resources.server.reset();
 resources.application_worker=[&](b::CallbackStateView){return std::make_unique<JoinRouteWorker>(record,allowed,calls);};
 auto source=std::make_unique<b::SourceBackend>(fixture.runtime_seed(),std::move(resources));anadius::configuration::State config{};std::uint8_t x=0,y=0,z=0;source->bind_state(config,{x,y,z});source->register_exit(anadius::bootstrap_native_recovered::Resolver{}.exit_callback());
 auto providers=source->providers();std::uint32_t id=0;const auto handle=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&own_probe),0,0,&id,providers.threads);require(handle&&providers.threads.close_handle(handle),"Join-continuation route owns exact synthetic tracked native ticket");
 const auto lower_wait=fixture.api_state.wait_hook;fixture.api_state.wait_hook=[&](HANDLE h,DWORD timeout){require(calls==fixture.stops&&record.requests==calls&&timeout==(allowed?7U:0U),"Controller stop and join continuation precede nonblocking seal/poll; configured blocking timeout used only after readiness");return allowed?lower_wait(h,timeout):DWORD{WAIT_TIMEOUT};};
 require(!source->stop_and_join()&&calls==1&&!record.finishes&&source->tracked_jobs()==1,"False join preparation retains source routes/worker and skips finish after wait0 timeout");
 allowed=true;record.finish_allowed=true;require(source->stop_and_join()&&calls==2&&record.finishes==1&&record.probes==1&&!source->tracked_jobs(),"Known later preparation still requires independent native join and route drain before finish");
 require(source->stop_and_join()&&calls==2&&record.finishes==1,"Completed stop/join cannot repeat continuation or worker finish");source.reset();route_record=nullptr;
}
void backend_routes(){
 b::Resources defaults;require(!defaults.join_api.process&&!defaults.join_api.duplicate&&!defaults.join_api.wait&&!defaults.join_api.close&&!defaults.join_api.error&&!defaults.join_api.set_error&&defaults.join_timeout_ms==0,"Default Resources join selector is known-empty with explicit zero timeout, never indeterminate native operands");
 b4_test::Fixture fixture;RouteRecord record;route_record=&record;
 {auto mixed=fixture.resources();mixed.application_worker=[](b::CallbackStateView){return std::make_unique<RouteWorker>(*route_record);};rejects<std::invalid_argument>([&]{b::SourceBackend wrong(fixture.runtime_seed(),std::move(mixed));},"Resources refuses simultaneous legacy server and application-worker factory");}
 {auto missing=fixture.resources();missing.server.reset();rejects<std::invalid_argument>([&]{b::SourceBackend wrong(fixture.runtime_seed(),std::move(missing));},"Resources requires exactly one server or worker assembly route");}
 auto resources=fixture.resources();resources.server.reset();fixture.online->found=false;const auto source_stop=resources.stop_sources;resources.stop_sources=[&]{record.order.emplace_back("lower-stop");return source_stop();};resources.application_worker=[&](b::CallbackStateView view){++record.factories;record.config=&view.configuration;record.runtime=&view.runtime;record.execution=&view.execution;record.show_lsx=view.show_lsx;return std::make_unique<RouteWorker>(record);};
 auto source=std::make_unique<b::SourceBackend>(fixture.runtime_seed(),std::move(resources));record.backend=source.get();anadius::configuration::State config{};config.executable_path=u"own-synthetic.exe";std::uint8_t lsx=0,console=0,no_console=0;source->bind_state(config,{lsx,console,no_console});require(record.factories==1&&record.config==&config&&record.runtime==&source->state()&&record.execution&&fixture.stops==0,"SourceBackend invokes factory once with exact stable live configuration/runtime/Execution before source publication");rejects([&]{source->bind_state(config,{lsx,console,no_console});},"SourceBackend cannot rebind factory state or replace live application worker");
 source->register_exit(anadius::bootstrap_native_recovered::Resolver{}.exit_callback());source->worker(anadius::recovered::Worker::second_035b60); // obtain route without invoking before server flag
 require(record.show_lsx==&lsx,"Backend factory receives the exact Runtime ConsoleState ShowLSX cell");lsx=7;require(*record.show_lsx==7,"Borrowed startup flag stays live after factory construction");
 source->between_workers();require(record.stages==std::vector<e::ReadyStage>{e::ReadyStage::Host},"Host milestone is emitted only after actual Host source callback returns");
 auto providers=source->providers();std::uint32_t id=0;const auto handle=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&own_probe),0,0,&id,providers.threads);require(handle&&providers.threads.close_handle(handle),"Selected route test owns retained fake TrackingThreads job while original launch handle closes");
 const auto lower_wait=fixture.api_state.wait_hook;fixture.timeout_once=true;fixture.api_state.wait_hook=[&](HANDLE handle,DWORD timeout){record.order.emplace_back("join-wait");require(record.requests==fixture.stops&&record.requests>0,"Application request_stop and lower stop_sources precede every blocking tracker wait");return lower_wait(handle,timeout);};
 record.run_hook=[&]{record.active_refused=!source->stop_and_join();require(record.finishes==0,"Active source route cannot invoke application finish even while stopping");};source->worker(anadius::recovered::Worker::first_029ea0)();record.run_hook={};require(record.identities&&record.runs==1&&config.server_port==0x4321&&record.runtime->server_ready==1&&record.active_refused,"SourceBackend forwards exact server port/ready/Execution once and retains timed-out active route");
 source->worker(anadius::recovered::Worker::second_035b60)();require(record.stages==std::vector<e::ReadyStage>{e::ReadyStage::Host,e::ReadyStage::Online}&&record.identities&&!record.failures,"Actual online source completion emits Online with same Execution after real second-ready publication");
 require(!source->stop_and_join()&&record.probes==1&&record.finishes==1&&source->tracked_jobs()==0,"After join and route lease drain, known application finish false remains retryable");record.finish_allowed=true;require(source->stop_and_join()&&record.finishes==2&&record.requests==2&&fixture.stops==2,"Finish retry neither re-stops lower admissions nor replays already joined jobs");require(source->stop_and_join()&&record.finishes==2,"Completed SourceBackend shutdown is idempotent");source.reset();route_record=nullptr;
}
struct BackendCapture {
 b::SourceBackend* backend=nullptr;unsigned* calls=nullptr;bool* denied=nullptr;
 ~BackendCapture(){if(!backend)return;++*calls;try{backend->register_exit(anadius::bootstrap_native_recovered::Resolver{}.exit_callback());}catch(const std::logic_error&){*denied=true;}catch(...){}}
};
void backend_capture_retirement(){b4_test::Fixture fixture;RouteRecord record;auto resources=fixture.resources();resources.server.reset();unsigned calls=0;bool denied=false;auto capture=std::make_shared<BackendCapture>();capture->calls=&calls;capture->denied=&denied;resources.application_worker=[capture,&record](b::CallbackStateView){return std::make_unique<RouteWorker>(record);};anadius::configuration::State config{};std::uint8_t a=0,c=0,d=0;auto source=std::make_unique<b::SourceBackend>(fixture.runtime_seed(),std::move(resources));resources.application_worker={};source->bind_state(config,{a,c,d});capture->backend=source.get();capture.reset();source.reset();require(record.cancels==1&&!record.finishes&&calls==1&&denied,"Unpublished backend cancels known worker then factory-capture destructor cannot republish routes while Impl referents/gate remain live");require(fixture.remaining->exit_callback==nullptr,"Retiring factory capture never reaches remaining lower exit registrar");}
struct WorkerCapture {
 a::Worker* worker=nullptr;State* state=nullptr;unsigned* calls=nullptr;bool* valid=nullptr;
 ~WorkerCapture(){if(!worker)return;++*calls;try{auto before=worker->snapshot();*valid=before.phase==a::Phase::Retiring&&!before.pending_retained;::SetLastError(0x48d0);worker->startup_complete(e::ReadyStage::Online,&state->execution);worker->startup_failed(&state->execution);worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);worker->request_stop();auto after=worker->snapshot();*valid&=after.phase==a::Phase::Retiring&&!after.online_complete&&!after.host_complete&&!after.stop_requested&&!after.pending_retained&&::GetLastError()==0x48d0;}catch(...){*valid=false;}}
};
void worker_capture_retirement(){auto state=std::make_shared<State>();selected=state.get();unsigned calls=0;bool valid=false;auto capture=std::make_shared<WorkerCapture>();capture->state=state.get();capture->calls=&calls;capture->valid=&valid;auto opts=settings(state);auto configure=std::move(opts.configure);opts.configure=[capture,configure](a::Inputs& in){configure(in);};auto worker=make_worker(state,std::move(opts));opts.configure={};capture->worker=worker.get();capture.reset();require(worker->cancel_unpublished(),"Unstarted concrete worker closes prepared memory and event before capture retirement");worker.reset();require(calls==1&&valid&&!state->configured&&!state->startups,"Settings capture destructor sees live Retiring metadata; late run/milestones/failure/stop neither resurrect phase nor touch closed event/SDK");configure={};finish_state(state);}
struct Marker {State& state;explicit Marker(State& s):state(s){}~Marker(){++state.retired_markers;}};
struct IdentityOnline final:b::OnlineComponent {
 using Path=anadius::configuration::Path;using Module=anadius::online_recovered::Module;using Procedure=anadius::online_recovered::Procedure;using Object=anadius::online_recovered::Object;
 std::function<void()> hook;bool result=true;
 bool matches(b::CallbackStateView)const noexcept override{if(hook)hook();return result;}
 bool close_after_cleanup()override{unused();}
#define IDENTITY_UNUSED(Type,Name,Args) Type Name Args override{unused();}
 IDENTITY_UNUSED(std::uint32_t,suppress_errors_0450a0,())
 IDENTITY_UNUSED(void,restore_error_mode,(std::uint32_t))
 IDENTITY_UNUSED(Path,filesystem_widen_006d70,(std::string_view))
 IDENTITY_UNUSED(Path,parent_path_019820,(const Path&))
 IDENTITY_UNUSED(Path,join_016b00,(const Path&,const Path&))
 IDENTITY_UNUSED(Module,load_copy_0450d0,(const Path&))
 IDENTITY_UNUSED(Procedure,get_proc_address,(Module,std::string_view))
 IDENTITY_UNUSED(std::int32_t,call_version_check,(Procedure))
 IDENTITY_UNUSED(Object,call_create_online,(Procedure,const char16_t*))
 IDENTITY_UNUSED(std::int32_t,status_virtual_08,(Object))
 IDENTITY_UNUSED(const char*,username_virtual_10,(Object))
 IDENTITY_UNUSED(const char*,persona_virtual_18,(Object))
 IDENTITY_UNUSED(const char*,user_id_virtual_20,(Object))
 IDENTITY_UNUSED(std::string,copy_username,(Object))
 IDENTITY_UNUSED(std::string,copy_persona,(Object))
 IDENTITY_UNUSED(std::string,copy_user_id,(Object))
 IDENTITY_UNUSED(void,destroy_virtual_00,(Object))
 IDENTITY_UNUSED(bool,free_library,(Module))
 IDENTITY_UNUSED(anadius::status_recovered::Status,status_01ea20,(const Path&,std::uint32_t))
 [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const Path&)override{unused();}
 IDENTITY_UNUSED(std::string,read_binary_03b3f0,(const Path&))
 IDENTITY_UNUSED(bool,write_binary_03c1d0,(const Path&,std::string_view))
 IDENTITY_UNUSED(void,log_literal_006880,(std::string_view))
 IDENTITY_UNUSED(void,log_finish_0088e0,())
 IDENTITY_UNUSED(void,dialog_038bc0,(std::string_view,std::string_view,std::uint32_t))
 IDENTITY_UNUSED(void,wait_steps_036880,(std::int32_t))
 IDENTITY_UNUSED(void,finish_flag_036820,())
#undef IDENTITY_UNUSED
};
[[noreturn]]void identity_child(std::string_view mode){
 auto state=std::make_shared<State>();selected=state.get();auto worker=make_worker(state,settings(state));IdentityOnline online;
 anadius::achievement_storage::Paths paths;anadius::license_recovered::AuthenticationCode code;e::MutexState mutex{};e::initialize_mutex_04a698(mutex,2);
 unsigned nested=0;DWORD result=0;
 online.hook=[&]{
  try{
   if(mode=="identity-paths"){result=811;worker->bind_achievement_paths(paths);}
   else if(mode=="identity-auth"){result=812;worker->bind_authentication_code(code);}
   else if(mode=="identity-mutex"){result=813;worker->bind_achievement_mutex(mutex,state->platform);}
   else if(mode=="identity-finish"){result=814;require(!worker->cancel_unpublished(),"Identity callback cannot close prepared memory");++nested;}
   else if(mode=="identity-run"){result=815;worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);++nested;}
   else if(mode=="identity-rebind"){result=816;worker->bind_effects_online(online);}
   else if(mode=="identity-ready"){result=817;worker->startup_complete(e::ReadyStage::Online,&state->execution);++nested;}
  }catch(const std::logic_error&){++nested;}
 };
 rejects([&]{worker->bind_effects_online(online);},"Caught identity callback reentry prevents outer binding from resuming");const auto snap=worker->snapshot();
 require(result&&nested==1&&snap.phase==a::Phase::Poisoned&&!snap.pending_retained&&!snap.worker_thread&&!snap.online_complete&&!state->configured&&!state->startups&&snap.memory.phase==anadius::worker_handoff::Phase::Prepared&&!worker->finish_after_join(),"Identity callback cannot mutate roots, enter worker, publish readiness or close prepared memory after caught reentry");::ExitProcess(result);
}
void child(std::string_view mode){if(mode=="original-dispatch")original_dispatch_without_startup_milestones();if(mode.starts_with("close-")){close_continuation(mode);return;}if(mode.starts_with("identity-"))identity_child(mode);auto state=std::make_shared<State>();selected=state.get();auto opts=settings(state);if(mode=="timeout")opts.startup_timeout_ms=20;
 if(mode=="configure")state->configure_hook=[&](a::Inputs& input){auto marker=std::make_shared<Marker>(*state);state->marker=marker;input.server.state_lifetime=marker;throw std::runtime_error("Opaque configured prefix");};
 if(mode=="protected")state->configure_hook=[](a::Inputs& input){static anadius::configuration::State foreign{};input.graph.state.configuration=&foreign;};
 auto worker=make_worker(state,std::move(opts));if(mode=="retirement")state->retirement_hook=[&]{try{const auto before=worker->snapshot();state->retirement_valid=before.phase==a::Phase::Serving&&before.pending_retained;worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);state->retirement_valid=state->retirement_valid.load()&&worker->snapshot().phase==a::Phase::Poisoned;}catch(...){state->retirement_valid=false;}};if(mode=="wrongworker"){worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);require(worker->snapshot().phase==a::Phase::Poisoned&&!state->configured&&!state->startups&&!worker->finish_after_join(),"Controller cannot impersonate native Domain server worker or synthesize successful finish");::ExitProcess(801);}
 Task task;task.launch(*state->native.threads,[&]{worker->run_server(state->config.server_port,state->runtime.server_ready,state->execution);});
 if(mode!="configure"&&mode!="protected"){state->armed.wait();for(unsigned i=0;i<5000&&worker->snapshot().phase!=a::Phase::WaitingStartup&&worker->snapshot().phase!=a::Phase::Poisoned;++i)::Sleep(1);require(state->accepts==1&&state->runtime.server_ready==1&&!state->gets,"Failure fixture has real armed accept/ready prefix before startup barrier outcome");
  if(mode=="retirement"){worker->startup_complete(e::ReadyStage::Online,&state->execution);worker->startup_complete(e::ReadyStage::Host,&state->execution);}if(mode=="stop")worker->request_stop();if(mode=="online-failed")worker->startup_failed(&state->execution);if(mode=="host-failed"){worker->startup_complete(e::ReadyStage::Online,&state->execution);worker->startup_failed(&state->execution);}if(mode=="identity"){e::MutexState mutex{};e::initialize_mutex_04a698(mutex,2);std::uint8_t first=0,second=0,third=0;std::int64_t frequency=0;e::Execution foreign(mutex,{first,second,third},frequency,state->platform);worker->startup_complete(e::ReadyStage::Online,&foreign);}
 }
 task.join();auto snap=worker->snapshot();require(snap.phase==a::Phase::Poisoned&&snap.pending_retained&&!worker->finish_after_join()&&!worker->cancel_unpublished(),"Unknown/failed started Worker retains persistent prefix and refuses fake finish or unstarted cancellation");
 if(mode=="retirement"){require(state->retirement_calls==1&&state->retirement_valid&&state->external==1&&state->socket_closes==2&&state->port_closes==1&&state->gate_deletes==2&&state->cleanups==1,"Actual normal server teardown reaches last narrow provider once; its caught nested run poisons module without fabricating final Returned");auto* input=state->inputs.load();require(input&&input->runtime.tls_index&&*input->runtime.tls_index!=TLS_OUT_OF_INDEXES,"Persistent Inputs and owned TLS selector remain after post-server-retirement poison");require(::TlsSetValue(*input->runtime.tls_index,reinterpret_cast<void*>(0x48c0))!=0&&::TlsSetValue(*input->runtime.tls_index,nullptr)!=0,"Actual Worker-owned TLS index is still allocated, proving no late TLSFree after destructor poisoning");::ExitProcess(809);}if(mode=="configure"){require(state->configured==1&&!state->marker.expired()&&!state->retired_markers&&!state->startups&&state->inputs.load(),"Persistent Inputs published before opaque configure retain returned ownership on throw");::ExitProcess(802);}if(mode=="protected"){require(state->configured==1&&!state->startups,"Caller cannot substitute protected graph configuration or TLS before any source startup");::ExitProcess(803);}
 require(state->gets==0&&!state->socket_closes&&!state->port_closes&&!state->gate_deletes&&!state->cleanups&&!state->cancels&&!state->external,"Stopped/failed/timed-out armed prefix performs no dispatch, fake cancellation, external attestation or partial source cleanup");if(mode=="stop")::ExitProcess(804);if(mode=="online-failed")::ExitProcess(805);if(mode=="host-failed")::ExitProcess(806);if(mode=="timeout")::ExitProcess(807);if(mode=="identity")::ExitProcess(808);::ExitProcess(899);
}
void isolated(std::wstring mode,DWORD expected){std::array<wchar_t,32768> path{};auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));require(n&&n<path.size(),"Own application-worker fixture path found");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --"+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};require(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Isolated retained application-worker fixture launched");DWORD code=0;require(::WaitForSingleObject(process.hProcess,30000)==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"Application-worker child reaches exact retained-prefix assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
}
int main(int argc,char** argv){try{if(argc==2){child(std::string_view(argv[1]).substr(2));return 0;}passive_validation();composition_constructor_failure();backend_routes();backend_join_continuation();backend_capture_retirement();worker_capture_retirement();startup_gate_normal(false);startup_gate_normal(true);isolated(L"original-dispatch",825);isolated(L"close-normal",0);isolated(L"close-timeout",821);isolated(L"close-disabled",822);isolated(L"close-unknown",823);isolated(L"close-poison",824);isolated(L"wrongworker",801);isolated(L"configure",802);isolated(L"protected",803);isolated(L"stop",804);isolated(L"online-failed",805);isolated(L"host-failed",806);isolated(L"timeout",807);isolated(L"identity",808);isolated(L"retirement",809);isolated(L"identity-paths",811);isolated(L"identity-auth",812);isolated(L"identity-mutex",813);isolated(L"identity-finish",814);isolated(L"identity-run",815);isolated(L"identity-rebind",816);isolated(L"identity-ready",817);std::cout<<"PASS: "<<checks<<" application-worker core checks; startup/SourceBackend routes, explicit close continuation and retained-prefix source scenarios\n";return 0;}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){std::cerr<<"Unknown own worker test failure\n"<<std::flush;::ExitProcess(2);}}
