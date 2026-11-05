#include "server.hpp"
#include "../anadius-socket-source/native.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include "../anadius-socket-source/worker_run_native.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>
#include <array>

namespace {
namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace server=anadius::server_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected owned-server integration dependency");}
void weak_release(s::IoReference* c){if(c&&c->weak_0c.fetch_sub(1)==1)c->delete_slot_08();}
void strong_release(s::IoReference* c){if(c&&c->strong_08.fetch_sub(1)==1){c->dispose_slot_00();weak_release(c);}}
void* WINAPI heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){
 auto& state=*b6_core_test::observed;++state.allocations;if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))unused();
 auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw)state.blocks.emplace(raw,bytes);::SetLastError(error);return raw;
}
struct NativeDomain {
 b6_core_test::Observation memory;b6_core_test::RuntimeState runtime;std::shared_ptr<anadius::thread_domain::DomainStorage> root;
 NativeDomain(){if(b6_core_test::observed||b6_core_test::runtime_state)unused();b6_core_test::observed=&memory;b6_core_test::runtime_state=&runtime;
  auto apis=b6_core_test::apis();apis.heap.heap_alloc=heap_allocate;root=anadius::thread_domain::DomainStorage::create(b6_core_test::seed(),apis);if(!root->initialize())unused();}
 ~NativeDomain(){if(root){try{if(!root->close())std::terminate();}catch(...){std::terminate();}root.reset();}b6_core_test::observed=nullptr;b6_core_test::runtime_state=nullptr;}
};
enum class Fault {None,ServiceGate,Open,Attach,Reuse,Bind,Listen};
enum class ContextCase {Host,Original,CachedFailure,RegistryFailure};
struct State {
 bool native_context=false,initial_winsock=false,initial_winsock_held=false;int startup_status=0;std::shared_ptr<o::WinsockModuleState> winsock;
 Fault fault=Fault::None;std::string diagnostic_trace;std::uint32_t diagnostic_read=0;
 bool ipv6=false,accept_pending=false,job_done=false,worker_done=false,queue_drained=false,session_deleted=false,accept_packets_drained=false,swallow_quiescent_poison=false;
 std::uint8_t* ready=nullptr;o::ServerView* view=nullptr;
 unsigned starts=0,cleanups=0,gate_inits=0,gate_deletes=0,port_attempts=0,ports_created=0,ports_closed=0,categories=0,submits=0,moves=0,gets=0,posts=0,wait_scopes=0,quiescent_calls=0,executor_cleanups=0,cancellations=0,functor_calls=0,cache_frees=0;
 std::vector<int> families;std::vector<s::Socket> closed;std::vector<std::string> order;
 std::string environment_key,environment_value;std::vector<std::byte> bound;std::uintptr_t port=0;s::SocketObject* pending_identity=nullptr;unsigned swallowed=0;
};
State* running=nullptr;
void diagnostic_failure(State& state,std::uint32_t error){state.diagnostic_trace="F";::SetLastError(error);}
std::uint32_t diagnostic_code(Fault fault){switch(fault){case Fault::Open:return 10013;case Fault::Attach:return ERROR_ACCESS_DENIED;case Fault::Reuse:return 10022;case Fault::Bind:return 10048;case Fault::Listen:return 10045;default:return ERROR_NOT_ENOUGH_MEMORY;}}
int WSAAPI startup(WORD version,LPWSADATA data){auto& st=*running;require(version==(st.native_context?WORD{2}:MAKEWORD(2,2)),"Explicit context profile requests its selected native WORD2 or HOST2.2");if(st.native_context)require(st.gate_inits==(st.initial_winsock?0U:1U),"Cold context startup follows its registry gate; unchecked image claim may precede the context");++st.starts;st.order.push_back("startup");return st.startup_status?st.startup_status: ::WSAStartup(version,data);}
int WSAAPI cleanup(){auto& st=*running;++st.cleanups;st.order.push_back("cleanup");const auto result=::WSACleanup();return st.native_context?SOCKET_ERROR:result;}
int WSAAPI local_name(SOCKET socket,sockaddr* output,int* bytes){
 require(socket==200&&*bytes>=28,"Local endpoint uses actual owned acceptor token and initialized SDK buffer");auto& st=*running;std::memset(output,0,*bytes);
 output->sa_family=st.ipv6?23:2;reinterpret_cast<unsigned char*>(output)[2]=0x30;reinterpret_cast<unsigned char*>(output)[3]=0x39;*bytes=28;st.order.push_back("local");return 0;
}
BOOL WINAPI environment(LPCSTR key,LPCSTR value){auto& st=*running;require(st.ready&&!*st.ready,"Bound port environment publication precedes source readiness flag");st.environment_key=key;st.environment_value=value;st.order.push_back("environment");return FALSE;}
BOOL WINAPI gate_initialize(LPCRITICAL_SECTION section,DWORD spin){++running->gate_inits;if(running->native_context)running->order.push_back("init-gate");if(running->fault==Fault::ServiceGate&&running->gate_inits==2){require(section&&spin==0x80000000U&&running->ports_created==1,"Service gate failure follows actual scheduler gate and owned port creation");::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}return ::InitializeCriticalSectionAndSpinCount(section,spin);}
VOID WINAPI gate_delete(LPCRITICAL_SECTION section){++running->gate_deletes;running->order.push_back("delete-gate");::DeleteCriticalSection(section);}
HANDLE WINAPI create_port(HANDLE file,HANDLE previous,ULONG_PTR key,DWORD threads){
 ++running->port_attempts;
 require(file==INVALID_HANDLE_VALUE&&!previous&&!key&&threads==0xffffffffU,"Actual024990 forwards observed selected IOCP construction arguments");
 auto* port=::CreateIoCompletionPort(file,previous,key,threads);if(port){++running->ports_created;running->port=reinterpret_cast<std::uintptr_t>(port);}return port;
}
BOOL WINAPI close_port(HANDLE port){require(reinterpret_cast<std::uintptr_t>(port)==running->port,"Owned teardown closes the same actual kernel IOCP handle");++running->ports_closed;running->order.push_back("close-port");return ::CloseHandle(port);}
struct Errors:s::Api {
 State& state;explicit Errors(State& s):state(s){}
 void set_last_error(std::int32_t n)override{::SetLastError(n);}std::int32_t get_last_error()override{const auto error=::GetLastError();if(!state.diagnostic_trace.empty()){state.diagnostic_trace+='E';state.diagnostic_read=error;}return static_cast<std::int32_t>(error);}
 s::Category system_category_028620()override{const auto error=::GetLastError();++state.categories;if(!state.diagnostic_trace.empty())state.diagnostic_trace+='C';::SetLastError(error);return {0x2222,server::ErrorCategory::system};}
 s::Conversion convert_acp(const char* text,std::int32_t size)override{require(std::string_view(text)=="127.0.0.1"&&size==10,"Actual address source passes exact loopback C string and bounded conversion");return {10,std::wstring(L"127.0.0.1")};}
 s::AddressCall string_to_address(s::Conversion& text,std::int32_t family,std::int32_t size)override{
  require(text.text==L"127.0.0.1"&&size==128,"Address parser lower receives defined owned wide string and sockaddr capacity");state.families.push_back(family);
  if(family==23&&!state.ipv6){::SetLastError(10022);return {};}
  s::AddressCall result;result.result=0;result.scope=state.ipv6?9:0;if(state.ipv6)result.bytes[15]=1;else{result.bytes[0]=127;result.bytes[3]=1;}::SetLastError(0);return result;
 }
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Sockets:s::AcceptorApi {
 State& state;explicit Sockets(State& s):state(s){}
 s::Socket create_socket(std::int32_t family)override{require(family==(state.ipv6?23:2),"Actual020e20 opens selected family through synthetic socket SDK boundary");state.order.push_back("socket");if(state.fault==Fault::Open){diagnostic_failure(state,diagnostic_code(state.fault));return s::invalid_socket;}return 200;}
 std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require(socket==200&&port==state.port,"Acceptor uses same concrete owned kernel completion port");if(state.fault==Fault::Attach){diagnostic_failure(state,diagnostic_code(state.fault));return 0;}return port;}
 std::uint32_t get_win32_error()override{const auto error=::GetLastError();if(!state.diagnostic_trace.empty()){state.diagnostic_trace+='G';state.diagnostic_read=error;}return error;}
 std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t option,std::int32_t value)override{
  if(level==41){require(state.ipv6&&socket==200&&option==27&&value==0,"Actual IPv6 acceptor clears IPV6_V6ONLY before IOCP attachment");return 0;}
  require(socket==200&&level==65535&&option==4&&value==1,"Acceptor enables source reuse-address option");if(state.fault==Fault::Reuse){diagnostic_failure(state,diagnostic_code(state.fault));return -1;}return 0;
 }
 std::int32_t bind_socket(s::Socket socket,std::span<const std::byte> address,std::int32_t size)override{require(socket==200&&size==(state.ipv6?28:16),"Actual bind lower receives family-specific initialized sockaddr length");state.bound.assign(address.begin(),address.begin()+size);if(state.fault==Fault::Bind){diagnostic_failure(state,diagnostic_code(state.fault));return -1;}return 0;}
 std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{require(socket==200&&backlog==INT_MAX,"Actual listen uses observed INT_MAX backlog");if(state.fault==Fault::Listen){diagnostic_failure(state,diagnostic_code(state.fault));return -1;}return 0;}
 std::int32_t close_socket(s::Socket socket)override{state.closed.push_back(socket);state.order.push_back("socket-close");if(!state.diagnostic_trace.empty())state.diagnostic_trace+='X';return 0;}
 std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Cancel:s::CancellationEffects {State& state;explicit Cancel(State& s):state(s){}void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{++state.cancellations;unused();}};
struct Executor:s::RegistryExecutorEffects {State& state;explicit Executor(State& s):state(s){}void executor_cleanup(s::SocketObject&,std::uintptr_t table)override{require(table!=0,"Socket source destructor supplies selected executor identity");++state.executor_cleanups;}};
struct Dispatcher:s::DispatcherApi {
 State& state;explicit Dispatcher(State& s):state(s){}
 void set_last_error(std::uint32_t n)override{::SetLastError(n);}std::uint32_t get_last_error()override{return ::GetLastError();}
 std::int32_t get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& operation,std::uint32_t timeout)override{
  require(state.view&&port==state.port,"Every physical wait belongs to current owned scheduler");
  bool released=false;try{(void)state.view->memory.buffer_context();}catch(const std::logic_error&){released=true;}
  require(released,"AdmittedDispatcherApi releases root admission before physical SDK wait");
  {auto independent=state.view->memory.enter();++state.wait_scopes;}
  ++state.gets;DWORD transferred=0;ULONG_PTR token=0;LPOVERLAPPED raw=nullptr;
  const auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&transferred,&token,&raw,timeout);
  bytes=transferred;key=token;operation=raw;return result;
 }
 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation)override{++state.posts;return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),bytes,key,operation);}
 std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct WorkerApi:s::WorkerRunApi {
 std::uint32_t index;o::AdmittedDispatcherApi& admitted;s::NativeWorkerRunApi native;
 WorkerApi(std::uint32_t i,o::AdmittedDispatcherApi& a):index(i),admitted(a){}
 void* tls_get(std::uint32_t i)override{if(i!=index)unused();return native.tls_get(i);}std::int32_t tls_set(std::uint32_t i,void* p)override{if(i!=index)unused();return native.tls_set(i,p);}
 std::int32_t post_stop(std::uintptr_t p,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* raw)override{return admitted.post(p,bytes,key,raw);}
 std::uint32_t get_last_error()override{return admitted.get_last_error();}
};
struct Completion:s::IoCompletionEffects {
 s::Category misc_category_0285b0()override{return {0x3333,server::ErrorCategory::asio_misc};}
 void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}
 void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}
};
struct DispatchEffects:s::OwnedIoDispatcherEffects {
 unsigned completed=0;explicit DispatchEffects(s::OwnedIoMemory& memory,s::IoCompletionEnvironment env,s::WorkerRunApi& worker,s::Api& errors,s::IoCompletionEffects& completion):OwnedIoDispatcherEffects(memory,env,worker,errors,completion){}
 std::uintptr_t capture_tls_slot()override{return 1;}std::int32_t thread_epoch(std::uintptr_t slot)override{if(slot!=1)unused();return 0;}
 void guard_header_04b838(std::int32_t&)override{unused();}void register_atexit_04b770(std::uintptr_t)override{unused();}void guard_footer_04b7cc(std::int32_t&)override{unused();}
 std::uint32_t read_stop_34(s::IocpSchedulerEntry& entry)override{return entry.high_34;}
 std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>&,const std::array<std::uintptr_t,2>&)override{unused();}
 [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{unused();}
};
struct Worker:s::RecoveredDispatcherWorkerEffects {
 o::ServerView& view;State& state;unsigned init=0,finish=0;
 Worker(o::ServerView& v,State& s,s::DispatcherState& dispatcher,s::DispatcherEnvironment environment,o::AdmittedDispatcherApi& api,DispatchEffects& effects,WorkerApi& worker,Errors& errors):RecoveredDispatcherWorkerEffects(dispatcher,environment,api,effects,worker,errors),view(v),state(s){}
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry& entry,std::uint32_t value)override{return std::atomic_ref(entry.high_34).exchange(value);}
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry& entry,std::uint32_t value)override{const auto old=static_cast<std::uint32_t>(entry.word_38);entry.word_38=(entry.word_38&0xffffffff00000000ULL)|value;return old;} // selected serialized worker
 void initialize_04a240(std::array<std::uintptr_t,2>& value)override{value.fill(0);++init;}
 void finish_04a2cc(std::array<std::uintptr_t,2>& value)override{require(value==std::array<std::uintptr_t,2>{},"Actual worker finishes unchanged explicit exception context");++finish;}
 void free_cache_04b788(std::uintptr_t identity)override{view.io.free_cached_identity(identity);++state.cache_frees;}
 void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override{unused();}[[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unused();}
};
struct SessionJob:s::PostedFunctor {
 State& state;s::SessionPair owned;explicit SessionJob(State& s,s::SessionPair pair):state(s),owned(pair){}
 void call_slot_00(std::uint8_t flag)override{
  require(flag==1&&owned.object&&owned.control&&owned.control->strong_08==1,"Actual posted native completion invokes job owning sole accepted factory strong");
  ++state.functor_calls;auto* residual=owned.control;owned={};strong_release(residual);state.job_done=true;
 }
};
struct Runtime:o::Runtime {
 State& state;Errors& errors;explicit Runtime(State& s,Errors& e):state(s),errors(e){}
 static void move(void* p,s::ExecutorFields& destination,s::ExecutorFields& source,std::uintptr_t table){auto& state=*static_cast<State*>(p);require(table==0x180084928ULL,"Accepted factory calls explicit selected executor slot8 lower");++state.moves;destination.context=source.context;destination.zero_74=source.zero_74;destination.self_reference_80=true;}
 void submit_accept(o::ServerView& view)override{
  require(state.ready&&*state.ready==1&&view.acceptor.state.socket==200,"Initial and rearmed accept are submitted only after source readiness on real owned acceptor");
  (void)view.memory.buffer_context();require(!state.accept_pending,"Synthetic accept lower has exactly one distinct pending accept obligation");
  if(!state.pending_identity)state.pending_identity=&view.pending;else require(state.pending_identity==&view.pending,"Rearm reuses stable actual pending SocketObject without nested root Operation");
  state.accept_pending=true;++state.submits;state.order.push_back("accept");
 }
 server::Error run(o::ServerView& view,o::AdmittedDispatcherApi& admitted)override{
  state.view=&view;require(state.accept_pending&&view.pending.state.socket==s::invalid_socket,"Mandatory accept provider retains separate pending state before synthetic final accept result");
  state.accept_pending=false;view.pending.state.socket=201;view.pending.state.socket_state=0x10;
  auto pair=view.construct_accepted({&state,move});auto* control=pair.control;auto* object=pair.object;control->weak_0c.fetch_add(1);
  require(pair.object&&pair.object->socket_state.socket==201&&view.pending.state.socket==s::invalid_socket&&control->strong_08==1&&control->weak_0c==3,"Actual023000 factory transfers accepted socket and publishes one strong/self/implicit plus observer weak");
  s::resize_receive_buffer_010040(object->input,6000,'x',view.memory.buffer_context());
  view.rearm_accept();require(state.submits==2&&state.accept_pending,"Source callback rearm invokes mandatory lower under existing admitted Operation");
  const auto tls=::TlsAlloc();require(tls!=TLS_OUT_OF_INDEXES,"Own worker TLS index allocated independently of backing PTD FLS");std::uint32_t index=tls;
  WorkerApi api(index,admitted);s::IoCompletionEnvironment ioenv{{index,view.io.storage_bindings()},{}};Completion completion;
  DispatchEffects effects(view.io,ioenv,api,errors,completion);s::DispatcherState dispatch{view.scheduler,nullptr,0,0,view.io.dispatcher_bindings()};std::int32_t epoch=0;
  const s::Category generic{0x1111,server::ErrorCategory::system},system{0x2222,server::ErrorCategory::system};
  Worker worker(view,state,dispatch,{epoch,generic,system},admitted,effects,api,errors);
  SessionJob job(state,pair);pair={};s::PostedFunctor* slot=&job;s::PostedExecutor executor{view.identity.value,&view.scheduler,1};
  s::dispatch_posted_022e40(executor,slot,ioenv.storage,api,admitted,view.io);
  require(!slot&&view.scheduler.work_count_30==1&&view.io.snapshot().allocations==1&&view.io.snapshot().pending==0,"Actual022e40 publishes concrete posted operation and scheduler work without faking an SDK receive loan");
  s::WorkerRunResult result;const auto handled=s::run_worker_0298b0(view.scheduler,result,{index,generic},api,errors,worker);
  require(handled==1&&result.error&&result.error->value==0&&worker.init==1&&worker.finish==1&&state.functor_calls==1&&state.job_done,"Actual0298b0/027c10 routes real IOCP posted packet via concrete loan router toactual027120 and owned session job");
  require(::TlsGetValue(index)==nullptr&&state.cache_frees==1&&!view.io.snapshot().allocations&&view.scheduler.work_count_30==0,"Actual source worker restores TLS and frees concrete terminal cache after callback drain");
  const auto disposed=view.memory.snapshot();require(control->strong_08==0&&control->weak_0c==1&&disposed.controls==1&&disposed.allocations==1&&!disposed.buffers&&!disposed.proxies,"Actual session disposer closes accepted socket and input heap while final observer weak retains only control");
  weak_release(control);state.session_deleted=view.memory.snapshot().controls_destroyed==1;state.worker_done=true;
  // Actual worker relays a stop packet. Drain it through the SAME admitted API;
  // the lower probes admission and only then performs real kernel GQCS.
  unsigned relays=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;const auto got=admitted.get(view.scheduler.completion.port,bytes,key,raw,0);const auto error=admitted.get_last_error();
   if(!got){require(error==WAIT_TIMEOUT&&!raw,"Final observed queue timeout follows real stop-packet drain");break;}require(!raw&&!key&&!bytes,"Only actual source worker stop relay remains after completion");++relays;
  }
  require(relays==1&&state.gets==state.wait_scopes&&!admitted.poisoned(),"Every physical completion/stop/drain wait releases and reacquires one real parent Operation");state.queue_drained=true;
  require(::TlsFree(index)!=0,"Own worker TLS slot released after real worker exit");
  state.accept_pending=false;state.accept_packets_drained=true;state.view=nullptr;
  require(!view.memory.snapshot().allocations&&view.io.snapshot().active_calls==0&&view.service.head==&view.pending&&view.pending.next==&view.acceptor,"Observed worker/job/session drains leave only source pending and acceptor list nodes");
  return {};
 }
 bool quiescent(o::ServerView& view)override{
  ++state.quiescent_calls;bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}
  require(outside&&view.backend.snapshot().phase==o::Phase::Quiescing,"Trusted lower drain attestation occurs outside root Operation after new server admission stops");
  if(state.swallow_quiescent_poison){
   try{view.backend.with_operation([](o::ServerView&){throw std::runtime_error("owned-server-quiescence-prefix");});}
   catch(const std::runtime_error& error){if(std::string_view(error.what())=="owned-server-quiescence-prefix")++state.swallowed;else throw;}
  }
  // This is the selected synthetic accept-provider attestation, not a count-based
  // assertion about missing original021b00/0271a0 or arbitrary kernel/raw jobs.
  return state.worker_done&&state.job_done&&state.session_deleted&&state.queue_drained&&state.accept_packets_drained&&!state.accept_pending;
 }
};
struct ExecutionPlatform:anadius::execution_recovered::Platform {
 unsigned locks=0,unlocks=0;std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}
 void acquire_srw_exclusive(std::uintptr_t&)override{++locks;}void release_srw_exclusive(std::uintptr_t&)override{++unlocks;}
 std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}
};
HANDLE WINAPI rejected_port(HANDLE file,HANDLE previous,ULONG_PTR key,DWORD threads){++running->port_attempts;require(file==INVALID_HANDLE_VALUE&&!previous&&!key&&threads==0xffffffffU,"Rejected port receives exact native024990 SDK arguments");::SetLastError(ERROR_ACCESS_DENIED);return nullptr;}
BOOL WINAPI rejected_gate(LPCRITICAL_SECTION section,DWORD spin){++running->gate_inits;require(section&&spin==0x80000000U&&running->gate_inits==1,"First selected context gate receives exact native027be0 initialize arguments");::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
void normal_server(bool ipv6,bool sticky_child=false,bool error_child=false,bool gate_child=false,Fault fault=Fault::None,ContextCase context_case=ContextCase::Host){
 NativeDomain native;auto state=std::make_shared<State>();state->ipv6=ipv6;state->fault=fault;state->native_context=context_case!=ContextCase::Host;running=state.get();State* observed=state.get();std::weak_ptr<State> lifetime=state;
 if(state->native_context){state->winsock=std::make_shared<o::WinsockModuleState>(o::WinsockImports{startup,cleanup});require(!state->starts&&!state->cleanups,"Explicit native shared state constructs passively");if(context_case==ContextCase::CachedFailure){state->startup_status=10091;state->initial_winsock=true;state->winsock->acquire(state->initial_winsock_held);state->initial_winsock=false;require(state->initial_winsock_held&&state->starts==1&&state->winsock->cached_status()==10091,"Unchecked image owner keeps its cached startup failure before any context gate");}}
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);
 std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto errors=std::make_shared<Errors>(*state);o::Dependencies dependencies;dependencies.state_lifetime=state;dependencies.errors=errors;dependencies.sockets=std::make_shared<Sockets>(*state);
  s::IocpConstructorImports port_imports;port_imports.create_completion_port=create_port;port_imports.close_handle=close_port;
  if(error_child)port_imports.create_completion_port=rejected_port;
  dependencies.iocp=std::make_shared<s::NativeIocpConstructorApi>(port_imports);dependencies.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_child||context_case==ContextCase::RegistryFailure?rejected_gate:gate_initialize,::GetLastError});
  dependencies.dispatcher=std::make_shared<Dispatcher>(*state);dependencies.cancellation=std::make_shared<Cancel>(*state);dependencies.executor=std::make_shared<Executor>(*state);dependencies.runtime=std::make_shared<Runtime>(*state,*errors);
  dependencies.kernel={startup,cleanup,local_name,environment,gate_delete};
  if(state->native_context){dependencies.context_profile=o::ContextProfile::Original;dependencies.winsock_module=state->winsock.get();}
  if(context_case==ContextCase::Original){for(const unsigned kind:{0U,1U,2U,3U}){auto invalid=dependencies;if(kind==0)invalid.winsock_module=nullptr;if(kind==1)invalid.context_profile=o::ContextProfile::Host;if(kind==2)invalid.winsock_module=&o::native_winsock_module();if(kind==3)invalid.context_profile=static_cast<o::ContextProfile>(99);bool rejected=false;try{o::OwnedServerBackend bad(native.root,std::move(invalid),16);}catch(const std::invalid_argument&){rejected=true;}require(rejected&&!state->starts&&!state->cleanups&&!state->gate_inits&&native.root->provider_leases()==0,"Missing/foreign state, HOST/native mixing and unknown profile reject passively before a backend claim/provider lease");}}
  o::OwnedServerBackend backend(native.root,std::move(dependencies),16);errors.reset();state.reset();
  require(!lifetime.expired()&&native.root->provider_leases()==1,"Backend dependency state and dedicated real ApplicationMemory lease own their required provider graph");
  std::uint16_t port=ipv6?7777:0;
  if(context_case==ContextCase::CachedFailure||context_case==ContextCase::RegistryFailure){
   const bool cached=context_case==ContextCase::CachedFailure;const int code=cached?10091:ERROR_NOT_ENOUGH_MEMORY;const std::string operation=cached?"winsock":"mutex";const auto address=cached?0x180084500ULL:0x180084428ULL;const auto& category=s::system_error_category_028620();const auto expected=operation+": "+category.message(code);bool caught=false;
   try{backend.initialize_context_0241b0();}catch(const o::SourceError& error){caught=error.code().value()==code&&&error.code().category()==&category&&error.source_error().value==code&&error.source_error().category.name_identity==0x2222&&error.operation()==operation&&error.context_address()==address&&std::string(error.what())==expected;}
   require(caught&&backend.snapshot().phase==o::Phase::Poisoned&&observed->gate_inits==1&&observed->starts==unsigned(cached)&&observed->categories==(cached?2U:1U)&&!observed->port_attempts&&!observed->ports_created&&!observed->ports_closed&&!observed->gate_deletes&&!observed->cleanups,"Native registry-gate/cached Winsock failure retains exact typed prefix before scheduler and performs no rollback");
   if(cached){observed->winsock->release(observed->initial_winsock_held);bool extra=false;observed->winsock->acquire(extra);require(!observed->initial_winsock_held&&extra&&observed->starts==1&&!observed->cleanups&&observed->winsock->cached_status()==10091,"Failed checked context retained its own increment after image claim release; another owner sees cached failure without retry");observed->winsock->release(extra);require(!extra&&!observed->cleanups,"Actual poisoned context claim remains after all explicit fixture claims retire");}
   ::ExitProcess(0);
  }
  if(fault!=Fault::None){
   const auto code=diagnostic_code(fault);const std::string operation=fault==Fault::ServiceGate?"mutex":fault==Fault::Open||fault==Fault::Attach?"open":fault==Fault::Reuse?"set_option":fault==Fault::Bind?"bind":"listen";const auto& category=s::system_error_category_028620();const auto expected=operation+": "+category.message(static_cast<int>(code));
   bool caught=false;try{backend.initialize_context_0241b0();if(fault!=Fault::ServiceGate){server::Endpoint endpoint{};endpoint.family=2;endpoint.port_host_order=12345;backend.construct_acceptor_020e20(endpoint,true,INT_MAX);}}
   catch(const o::SourceError& error){const o::SourceError copy=error;const std::system_error base=error;const std::runtime_error sliced=error;caught=error.code().value()==static_cast<int>(code)&&&error.code().category()==&category&&error.source_error().value==static_cast<int>(code)&&error.source_error().category.name_identity==0x2222&&error.source_error().category.logical==server::ErrorCategory::system&&error.operation()==operation&&!error.context_address()&&std::string(error.what())==expected&&std::string(copy.what())==expected&&std::string(base.what())==expected&&std::string(sliced.what())==expected&&!copy.context_address();}
   require(caught&&backend.snapshot().phase==o::Phase::Poisoned&&observed->starts==1&&observed->gate_inits==2&&observed->port_attempts==1&&observed->ports_created==1&&!observed->ports_closed&&!observed->gate_deletes&&!observed->cleanups&&!observed->submits,"Actual service/acceptor nonzero error retains native typed diagnostic and initialized scheduler/port/WSA prefix without synthetic cleanup");
   if(fault==Fault::ServiceGate)require(observed->categories==2&&observed->diagnostic_trace.empty()&&observed->closed.empty()&&observed->order==std::vector<std::string>{"startup"},"Second service gate fails only after scheduler gate/port and retains context0 mutex without entering socket construction");
   else{
    const auto trace=fault==Fault::Attach?"FGCX":"FCE";require(observed->diagnostic_trace==trace&&observed->diagnostic_read==code&&observed->categories==3,"Actual WinSock category-before-error and Win32 error-before-category/close retain their distinct exact failure suffixes");
    require(observed->closed==(fault==Fault::Attach?std::vector<s::Socket>{200}:std::vector<s::Socket>{})&&observed->bound.size()==((fault==Fault::Bind||fault==Fault::Listen)?16U:0U),"Only native attachment failure closes its temporary socket; later operation failures retain their existing ownership prefix");
   }
   ::ExitProcess(0); // retained selected failure prefix; no guessed unwinding
  }
  if(error_child||gate_child){
   const auto code=gate_child?ERROR_NOT_ENOUGH_MEMORY:ERROR_ACCESS_DENIED;const auto address=gate_child?0x180084428ULL:0x180084528ULL;const std::string operation=gate_child?"mutex":"iocp";const auto& category=s::system_error_category_028620();
   // One independent default-native SDK oracle; constructor formatting itself
   // is not injected/counted, and no second code.message() is requested below.
   const auto expected=operation+": "+category.message(static_cast<int>(code));
   bool caught=false;try{backend.initialize_context_0241b0();}
   catch(const o::SourceError& error){const o::SourceError copied=error;const std::system_error base=error;const std::runtime_error sliced=error;caught=error.code().value()==static_cast<int>(code)&&&error.code().category()==&category&&error.source_error().value==static_cast<int>(code)&&error.source_error().category.name_identity==0x2222&&error.source_error().category.logical==server::ErrorCategory::system&&error.operation()==operation&&error.context_address()==address&&std::string(error.what())==expected&&std::string(copied.what())==expected&&std::string(base.what())==expected&&std::string(sliced.what())==expected&&copied.context_address()==address;}
   require(caught&&backend.snapshot().phase==o::Phase::Poisoned&&observed->starts==1&&observed->gate_inits==1&&observed->port_attempts==(gate_child?0U:1U)&&observed->categories==(gate_child?1U:2U)&&!observed->ports_created&&!observed->ports_closed&&!observed->gate_deletes&&!observed->cleanups,"Actual024990 nonzero gate/port failure retains native mutex/iocp text, typed source code/category/address and ordered initialized prefix without fabricated cleanup");
   ::ExitProcess(0); // own retained-prefix child; no unknown-owner unwinding
  }
  if(sticky_child){
   backend.initialize_context_0241b0();server::construct_server_024480(backend,port,ready,execution);
   const auto result=backend.run_context_0298b0();require(!result.value&&observed->worker_done&&observed->session_deleted&&observed->queue_drained,"Sticky child completes actual native worker/session/cache drain before adversarial quiescence lower");
   auto* pending=observed->pending_identity;auto* service=pending->service;auto* head=service->head;auto* next=pending->next;
   const auto close_count=observed->closed.size();const auto executor_count=observed->executor_cleanups;
   const auto pending_socket=pending->state.socket;const auto starts=observed->starts,cleanups=observed->cleanups,port_closes=observed->ports_closed,gate_deletes=observed->gate_deletes;
   observed->swallow_quiescent_poison=true;bool rejected=false;
   try{backend.destroy_pending_socket_024db0();}catch(const std::logic_error&){rejected=true;}
   const auto poisoned=backend.snapshot();
   require(rejected&&observed->swallowed==1&&poisoned.phase==o::Phase::Poisoned&&!poisoned.quiescence_attested&&poisoned.acceptor_initialized&&poisoned.pending_initialized&&!poisoned.active_calls,"Caught nested poison cannot become successful quiescence attestation or pending teardown");
   require(service->head==head&&pending->next==next&&pending->service==service&&pending->state.socket==pending_socket&&observed->closed.size()==close_count&&observed->executor_cleanups==executor_count,"Sticky poisoned postcheck precedes actual024db0 socket/list/executor effects");
   unsigned denied=0;
   try{backend.destroy_pending_socket_024db0();}catch(const std::logic_error&){++denied;}
   try{backend.destroy_acceptor_024db0();}catch(const std::logic_error&){++denied;}
   try{backend.destroy_context();}catch(const std::logic_error&){++denied;}
   try{backend.async_accept_021b00();}catch(const std::logic_error&){++denied;}
   require(denied==4&&backend.snapshot().phase==o::Phase::Poisoned&&!backend.snapshot().quiescence_attested&&observed->closed.size()==close_count&&observed->executor_cleanups==executor_count&&observed->starts==starts&&observed->cleanups==cleanups&&observed->ports_closed==port_closes&&observed->gate_deletes==gate_deletes,"All later teardown/accept APIs reject sticky poison without further physical resource effects");
   // Failed prefix intentionally retained; never invent native EH rollback or
   // unwind mandatory-Closed owners. Only this independently created child exits.
   std::_Exit(0);
  }
  server::server_start_029ea0(backend,port,ready,execution);
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1,"Actual server source publishes bound port and one readiness flag through actual Execution mutex route");
  require(observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Actual server path publishes decimal bound port despite ignored FALSE environment result");
  require(observed->families==(ipv6?std::vector<int>{23}:std::vector<int>{23,2}),"Actual source address selection uses IPv6 success or IPv4 fallback without network operations");
  require(observed->bound.size()==(ipv6?28U:16U)&&std::to_integer<unsigned char>(observed->bound[0])==(ipv6?23:2)&&std::to_integer<unsigned char>(observed->bound[2])==(ipv6?0x1e:0)&&std::to_integer<unsigned char>(observed->bound[3])==(ipv6?0x61:0),"Actual acceptor constructs exact family/initial-port sockaddr bytes");
  const auto gates=observed->native_context?3U:2U;require(observed->closed==std::vector<s::Socket>({201,200})&&observed->starts==1&&observed->cleanups==1&&observed->ports_created==1&&observed->ports_closed==1&&observed->gate_inits==gates&&observed->gate_deletes==gates,"Accepted session and listener close once with exact selected context gates and one startup/cleanup");
  const auto snapshot=backend.snapshot();require(snapshot.phase==o::Phase::Closed&&!snapshot.active_calls&&!snapshot.acceptor_initialized&&!snapshot.pending_initialized&&snapshot.quiescence_attested&&observed->quiescent_calls==1,"Full actual029ea0 source teardown reaches Closed only after explicit trusted lower quiescence");
  require(native.root->provider_leases()==0&&observed->executor_cleanups==4&&!observed->cancellations,"Source lifecycle destroys temporary/session/pending/acceptor while no invented cancellation engine exists");
  auto close=std::find(observed->order.begin(),observed->order.end(),"close-port"),clean=std::find(observed->order.begin(),observed->order.end(),"cleanup");
  if(observed->native_context)require(close!=observed->order.end()&&clean<close&&observed->order.size()>1&&observed->order[0]=="init-gate"&&observed->order[1]=="startup"&&std::count(close,observed->order.end(),"delete-gate")==3,"Cold native context registry gate precedes context acquire/startup; last release ignores cleanup result before port/service/registry teardown");else require(close!=observed->order.end()&&clean>close&&std::count(close,clean,"delete-gate")==2,"HOST resource order retains port then two gates then per-Core WSA cleanup");
 }
 require(lifetime.expired(),"State lifetime root outlives dependent providers and is released after closed backend destruction");running=nullptr;
 require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Backing actual process heap/PTD/FLS domain has no leaked session/IO/provider resources");
}
void integration_child(const wchar_t* mode){
 wchar_t executable[32768]{};const auto count=::GetModuleFileNameW(nullptr,executable,32768);require(count>0&&count<32768,"Own regression child executable path is bounded");
 std::wstring command=L"\""+std::wstring(executable)+L"\" "+mode;STARTUPINFOW startup_info{};startup_info.cb=sizeof(startup_info);PROCESS_INFORMATION process{};
 require(::CreateProcessW(executable,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup_info,&process)!=0,"Own sticky-quiescence regression child starts without original targets");
 const auto wait=::WaitForSingleObject(process.hProcess,30000);if(wait!=WAIT_OBJECT_0){(void)::TerminateProcess(process.hProcess,250);(void)::WaitForSingleObject(process.hProcess,5000);}
 DWORD code=251;const auto queried=::GetExitCodeProcess(process.hProcess,&code);::CloseHandle(process.hThread);::CloseHandle(process.hProcess);
 require(wait==WAIT_OBJECT_0&&queried&&code==0,"Own integration child observes its selected retained source prefix before explicit process exit");
}
Fault diagnostic_mode(std::string_view mode){if(mode=="--service-gate-error")return Fault::ServiceGate;if(mode=="--open-error")return Fault::Open;if(mode=="--attach-error")return Fault::Attach;if(mode=="--reuse-error")return Fault::Reuse;if(mode=="--bind-error")return Fault::Bind;if(mode=="--listen-error")return Fault::Listen;return Fault::None;}
}
int main(int argc,char** argv){try{if(argc==2&&std::string_view(argv[1])=="--native-context"){normal_server(false,false,false,false,Fault::None,ContextCase::Original);return 0;}if(argc==2&&std::string_view(argv[1])=="--native-cached-error"){normal_server(false,false,false,false,Fault::None,ContextCase::CachedFailure);return 253;}if(argc==2&&std::string_view(argv[1])=="--native-registry-error"){normal_server(false,false,false,false,Fault::None,ContextCase::RegistryFailure);return 254;}if(argc==2){const auto fault=diagnostic_mode(argv[1]);if(fault!=Fault::None){normal_server(false,false,false,false,fault);return 255;}}if(argc==2&&std::string_view(argv[1])=="--constructor-error"){normal_server(false,false,true);return 253;}if(argc==2&&std::string_view(argv[1])=="--gate-error"){normal_server(false,false,false,true);return 254;}if(argc==2&&std::string_view(argv[1])=="--sticky-quiescent"){normal_server(false,true);return 252;}normal_server(false);normal_server(true);normal_server(false,false,false,false,Fault::None,ContextCase::Original);integration_child(L"--native-cached-error");integration_child(L"--native-registry-error");integration_child(L"--sticky-quiescent");integration_child(L"--constructor-error");integration_child(L"--gate-error");for(const auto* mode:std::array<const wchar_t*,6>{L"--service-gate-error",L"--open-error",L"--attach-error",L"--reuse-error",L"--bind-error",L"--listen-error"})integration_child(mode);std::cout<<"PASS: "<<checks<<" owned-server integration checks; actual029ea0/024990/0228e0/023000/022e40/0298b0/027c10/027120/disposal, real own IOCP/heap/FLS/TLS, synthetic accept/socket/runtime boundaries; retained sticky-poison and native mutex/iocp diagnostic children\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected owned server integration exception\n";return 1;}}
