#include "environment.hpp"
#include "../anadius-socket-source/message_dispatch.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

namespace oe=anadius::owned_environment;namespace sp=anadius::session_profile;namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace r=anadius::server_runtime;namespace b=b6_core_test;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v){std::cerr<<"FAIL: "<<why<<'\n';throw std::runtime_error(why);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected isolated runtime dependency");}
template<class E,class F>void rejects(F fn,const char* why){bool caught=false;try{fn();}catch(const E&){caught=true;}check(caught,why);}
void* WINAPI heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){auto& state=*b::observed;++state.allocations;check(heap==::GetProcessHeap()&&(flags==0||flags==HEAP_ZERO_MEMORY),"owned source heap allocation preserves zero or uninitialized policy");auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw)state.blocks.emplace(raw,bytes);::SetLastError(error);return raw;}
std::function<void(void*)> before_heap_free;
BOOL WINAPI heap_release(HANDLE heap,DWORD flags,void* raw){if(before_heap_free)before_heap_free(raw);return b::free_heap(heap,flags,raw);}
struct NativeDomain {
 b::Observation observation;b::RuntimeState runtime;std::shared_ptr<anadius::thread_domain::DomainStorage> root;
 NativeDomain(){if(b::observed||b::runtime_state)unused();b::observed=&observation;b::runtime_state=&runtime;auto api=b::apis();api.heap.heap_alloc=heap_allocate;api.heap.heap_free=heap_release;root=anadius::thread_domain::DomainStorage::create(b::seed(),api);check(root->initialize(),"own controlled FLS/heap domain initialized");}
 ~NativeDomain(){if(root){if(!root->close())std::terminate();root.reset();}b::observed=nullptr;b::runtime_state=nullptr;}
};
struct Packet {std::uint32_t bytes;std::uintptr_t key;OVERLAPPED* operation;std::uint32_t error;};
struct State {
 std::uint32_t index=17;std::int32_t epoch=0;void* tls=nullptr;std::uint32_t last=0;
 unsigned environment=0,created=0,destroyed=0,bindings=0,accepts=0,factories=0,posts=0,gets=0,after=0,returned=0,external=0,init=0,finish=0,starts=0,cleanups=0,ports_closed=0;
 std::uint64_t handled=0;bool attested=false,poison_reentry=false,poison_backend=false,invalid_bindings=false,reentered=false;s::OwnedAcceptMemory* accept_memory=nullptr;
 std::optional<r::Ports> ports;std::optional<o::ServerView> view;r::Runtime* runtime=nullptr;std::function<void(std::string_view)> hook;std::vector<std::string> order;
 std::deque<Packet> packets;std::vector<s::Socket> closed;std::vector<std::uint32_t> tls_indices;
 void event(std::string_view name){order.emplace_back(name);if(hook)hook(name);}
};
State* active=nullptr;
int WSAAPI startup(WORD version,LPWSADATA){check(version==MAKEWORD(2,2),"server init requests exact WSA2.2 without real WSAStartup");++active->starts;return 0;}
int WSAAPI cleanup(){++active->cleanups;return 0;}
int WSAAPI local_name(SOCKET,sockaddr*,int*){unused();}
BOOL WINAPI environment(LPCSTR,LPCSTR){unused();}
VOID WINAPI delete_gate(LPCRITICAL_SECTION section){::DeleteCriticalSection(section);}
struct Errors final:s::Api {
 std::shared_ptr<State> state;explicit Errors(std::shared_ptr<State> p):state(std::move(p)){}
 s::Category system_category_028620()override{return {0x2222};}std::int32_t get_last_error()override{return static_cast<std::int32_t>(state->last);}void set_last_error(std::int32_t n)override{state->last=static_cast<std::uint32_t>(n);}
 s::Conversion convert_acp(const char*,std::int32_t)override{unused();}s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Sockets final:s::AcceptorApi {
 std::shared_ptr<State> state;explicit Sockets(std::shared_ptr<State> p):state(std::move(p)){}
 s::Socket create_socket(std::int32_t family)override{check(family==2,"source acceptor uses selected IPv4 family");return 200;}
 std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{check(socket==200&&port==0x900,"initial acceptor attaches selected fake kernel handle");return port;}
 std::uint32_t get_win32_error()override{return state->last;}
 std::int32_t set_option(s::Socket,int level,int name,int value)override{check(level==65535&&name==4&&value==1,"initial reuse option preserved");return 0;}
 std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t size)override{check(socket==200&&size==16,"source bind stays at controlled SDK token");return 0;}
 std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{check(socket==200&&backlog==INT_MAX,"source listen passes native INTMAX");return 0;}
 std::int32_t close_socket(s::Socket socket)override{state->closed.push_back(socket);state->event("socket.close");return 0;}
 std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Iocp final:s::IocpConstructorApi {
 std::shared_ptr<State> state;explicit Iocp(std::shared_ptr<State> p):state(std::move(p)){}
 std::uint64_t set_version_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t condition)override{check(!mask&&type==2&&condition==3,"source version-mask args");return 0x55;}
 std::int32_t verify_version(OSVERSIONINFOEXA& v,std::uint32_t type,std::uint64_t mask)override{check(v.dwMajorVersion==6&&type==2&&mask==0x55,"source IOCP version predicate");return 1;}
 std::uintptr_t create_completion_port(std::uintptr_t file,std::uintptr_t prior,std::uintptr_t key,std::uint32_t threads)override{check(file==~std::uintptr_t{0}&&!prior&&!key&&threads==UINT32_MAX,"source IOCP creation arguments to synthetic handle");return 0x900;}
 std::uint32_t get_last_error()override{return state->last;}std::int32_t close_handle(std::uintptr_t h)override{check(h==0x900,"owned server closes matching synthetic IOCP");++state->ports_closed;return 1;}
};
struct Gates final:s::ServiceGateApi {std::int32_t initialize(CRITICAL_SECTION& cs,std::uint32_t spin)override{return ::InitializeCriticalSectionAndSpinCount(&cs,spin);}std::uint32_t get_last_error()override{return ::GetLastError();}};
struct Cancellation final:s::CancellationEffects {void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{unused();}};
struct Executor final:s::RegistryExecutorEffects {void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}}
;
struct Dispatcher final:s::DispatcherApi {
 std::shared_ptr<State> state;explicit Dispatcher(std::shared_ptr<State> p):state(std::move(p)){}
 void set_last_error(std::uint32_t n)override{state->last=n;}std::uint32_t get_last_error()override{return state->last;}
 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* op)override{check(port==0x900,"runtime publication stays on exact server port");++state->posts;state->packets.push_back({bytes,key,op,0});state->event("post");return 1;}
 std::int32_t get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& op,std::uint32_t timeout)override{check(port==0x900&&timeout==UINT32_MAX&&!state->packets.empty(),"actual worker waits through selected deterministic queue");bool outside=false;try{(void)state->view->memory.buffer_context();}catch(const std::logic_error&){outside=true;}check(outside,"admitted dispatcher releases root before physical wait");{auto independent=state->view->memory.enter();}auto packet=state->packets.front();state->packets.pop_front();bytes=packet.bytes;key=packet.key;op=packet.operation;state->last=packet.error;++state->gets;state->event("get");return packet.error?0:1;}
 std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct Worker final:s::WorkerRunApi {
 std::shared_ptr<State> state;std::function<void()> on_destroy;explicit Worker(std::shared_ptr<State> p):state(std::move(p)){}~Worker()override{if(on_destroy)on_destroy();}
 void* tls_get(std::uint32_t index)override{check(index==state->index,"worker uses stable externally retained TLS slot");state->tls_indices.push_back(index);return state->tls;}
 std::int32_t tls_set(std::uint32_t index,void* value)override{check(index==state->index,"worker restores same TLS binding");state->tls=value;return 1;}
 std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{unused();}std::uint32_t get_last_error()override{unused();}
};
struct Accept final:s::AsyncAcceptApi {
 std::shared_ptr<State> state;explicit Accept(std::shared_ptr<State> p):state(std::move(p)){}
 s::AcceptExResult accept(s::Socket listener,s::Socket accepted,std::span<std::byte> buffer,OVERLAPPED& ov)override{check(listener==200&&accepted==201+state->accepts&&buffer.size()==288,"actual runtime021b00/helper factory reaches bounded fakeAcceptEx");++state->accepts;state->packets.push_back({0,0,&ov,995});state->event("accept");return {0,997};}
 s::AcceptAddresses addresses(std::span<std::byte>)override{unused();}std::int32_t update_context(s::Socket,s::Socket&)override{unused();}
};
struct Factory final:s::SocketFactoryApi {std::shared_ptr<State> state;explicit Factory(std::shared_ptr<State> p):state(std::move(p)){}s::Socket open(std::int32_t family,std::int32_t type,std::int32_t protocol)override{++state->factories;check(family==2&&type==1&&protocol==6,"actual029df0 uses pending protocol/type/TCP lower");return 201+state->accepts;}};
struct Cancel final:s::RetryCancelApi {std::int32_t cancel(s::Socket,OVERLAPPED&)override{unused();}};
struct Table final:s::IoHandlerVtable {Table():IoHandlerVtable(0x777){}void destroy_slot_00(s::IoHandlerEnvironment&)override{}void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{to.payload_00=from.payload_00;to.state_18=from.state_18;}std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}};
struct Types final:s::IoEnvironmentTypeApi {std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)override{unused();}};
struct Completion final:s::IoCompletionEffects {s::Category misc_category_0285b0()override{return {0x3333,anadius::server_recovered::ErrorCategory::asio_misc};}void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}};
struct Guard final:s::DispatcherEffects {std::uintptr_t capture_tls_slot()override{return 1;}std::int32_t thread_epoch(std::uintptr_t n)override{check(n==1,"dispatcher passes selected TLS identity");return 0;}void guard_header_04b838(std::int32_t&)override{unused();}void register_atexit_04b770(std::uintptr_t)override{unused();}void guard_footer_04b7cc(std::int32_t&)override{unused();}std::uint32_t read_stop_34(s::IocpSchedulerEntry& e)override{return e.high_34;}std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& d,const std::array<std::uintptr_t,2>& src)override{d=src;return d;}[[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{unused();}void complete(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{unused();}};
struct Control final:r::WorkerControl {State& state;explicit Control(State& s):state(s){}std::uint32_t exchange_stop(s::IocpSchedulerEntry& e,std::uint32_t n)override{state.event("control.exchange");return std::atomic_ref(e.high_34).exchange(n);}std::uint32_t exchange_wake(s::IocpSchedulerEntry& e,std::uint32_t n)override{const auto old=static_cast<std::uint32_t>(e.word_38);e.word_38=(e.word_38&0xffffffff00000000ULL)|n;return old;}void initialize(std::array<std::uintptr_t,2>& p)override{p.fill(0);++state.init;}void finish(std::array<std::uintptr_t,2>& p)override{check(p==std::array<std::uintptr_t,2>{},"actual worker finishes clean exception context");++state.finish;}[[noreturn]]void raise(s::Error,std::uintptr_t)override{unused();}};
struct Log final:s::WriteFinishApi {void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{unused();}[[noreturn]]void invalid_parameter_05c974()override{unused();}void next_write_02a500(std::uintptr_t)override{unused();}s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}};
struct Terminal final:s::FrameReadTerminal {[[noreturn]]void bad_weak_ptr_026860()override{unused();}};
struct FrameApi final:s::FrameReadApi {void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override{unused();}s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t)override{unused();}std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::FrameMessageClosure*,std::uint32_t,std::uint32_t&)override{unused();}std::int32_t detach_04a458(const s::FrameThreadHandle&)override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{unused();}[[noreturn]]void invalid_parameter_05c974()override{unused();}void next_write_02a500(std::uintptr_t)override{unused();}s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}};
struct Receive final:s::AsyncReceiveApi {s::AsyncReceiveResult receive(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override{unused();}};
struct ReadApi final:s::ComposedReadApi {void finish_frame_025480(s::ReceiveHandler&,s::Error&,std::uint64_t)override{unused();}void construct_length_error_024390(s::ReadLengthError&,std::uintptr_t)override{unused();}[[noreturn]]void throw_length_error_023930(s::ReadLengthError&)override{unused();}};
struct App final:r::Application {
 std::shared_ptr<State> state;r::Ports ports;Errors& errors;Table& table;Completion completion;Guard guard;Control control;Log log;Terminal terminal;FrameApi frame_api;Receive receive_api;ReadApi read_api;
 s::AsyncReceiveContext receive;s::ComposedReadContext read;s::FrameReadContext frame;
 App(std::shared_ptr<State> st,r::Ports p,Errors& e,Table& t):state(std::move(st)),ports(p),errors(e),table(t),control(*state),receive{p.environment,p.worker,p.memory.posted(),receive_api,p.completion,e,p.memory},read{p.view.memory.buffer_context(),receive,completion,read_api,{},{}},frame{p.view.memory.buffer_context(),completion,frame_api,{},{},{},terminal,{0x1111},&read}{state->ports.emplace(p);state->event("app.create");}
 ~App(){++state->destroyed;state->event("app.destroy");}
 r::Bindings bindings()override{++state->bindings;state->event("bindings");if(state->invalid_bindings)frame.read=nullptr;return {frame,log,completion,guard,control,{}, {},nullptr};}
 void construct_accept_environment(s::IoHandlerEnvironment& out,std::uintptr_t allocator)override{check(allocator==ports.server_identity+0x68,"actual rearm uses runtime-owned stable server identity+68");out={{1,2},&table,0,0,nullptr};state->event("construct.env");}
 void reset_pending_owner(s::AcceptPending&)override{unused();}s::ReceiveBufferString& greeting(s::SessionOwner&,s::ReceiveBufferString&)override{unused();}[[noreturn]]void raise_write(s::Error&,std::uintptr_t)override{unused();}
 s::IocpSchedulerEntry* completion_owner(s::IocpSchedulerEntry* owner,s::CancelOperation&,const s::Error& error,std::uint32_t)override{check(owner==&ports.view.scheduler&&(error.value==995||error.value==0),"only observed terminal995 packet selects explicit owner0 policy");(void)ports.view.memory.buffer_context();state->event(error.value==995?"owner0":"owner1");return error.value==995?nullptr:owner;}
 void after_completion()override{++state->after;state->event("after");}
 void worker_returned(std::uint64_t n,const s::WorkerRunResult& result)override{check(result.error&&result.error->value==0&&state->tls==nullptr,"real worker restoredTLS andreturneddefinedsuccess");state->handled=n;++state->returned;state->event("worker.returned");}
};
struct Profile final:r::Profile {
 std::shared_ptr<State> state;Errors& errors;Table table;Types types;s::IoEnvironmentConstructorContext constructor{types,{},{}};std::function<void()> on_destroy;
 Profile(std::shared_ptr<State> p,Errors& e):state(std::move(p)),errors(e){}
 ~Profile()override{if(on_destroy)on_destroy();}
 r::Environment environment(o::ServerView view,std::uintptr_t server)override{++state->environment;state->view.emplace(view);check(server!=0&&view.identity.value!=0,"profile seescopied complete view andstable runtime server token");state->event("profile.environment");if(state->poison_reentry){try{state->runtime->submit_accept(view);}catch(const std::logic_error&){state->reentered=true;}}return {{&table,0,nullptr,table.destroy_identity},constructor,{}};}
 std::unique_ptr<r::Application> create(r::Ports ports)override{++state->created;state->accept_memory=&ports.accept;check(&ports.view.memory==&state->view->memory&&&ports.view.io==&state->view->io,"Profilecreate uses sameexact view owners");if(state->poison_backend){try{ports.view.backend.with_operation([](o::ServerView&){unused();});}catch(const std::logic_error&){state->reentered=true;}}return std::make_unique<App>(state,ports,errors,table);}
 bool external_quiescent(o::ServerView view,r::Application&)override{++state->external;bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}check(outside,"externalattestation isoutside parentOperation");state->event("external");return state->attested;}
};
void move_executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t){to=from;}
struct BorrowedRuntime final:o::Runtime {
 r::Runtime& runtime;explicit BorrowedRuntime(r::Runtime& value):runtime(value){}
 void submit_accept(o::ServerView& view)override{runtime.submit_accept(view);}
 anadius::server_recovered::Error run(o::ServerView& view,o::AdmittedDispatcherApi& api)override{return runtime.run(view,api);}
 bool quiescent(o::ServerView& view)override{return runtime.quiescent(view);}
};
struct Fixture {
 NativeDomain native;std::shared_ptr<State> state=std::make_shared<State>();std::shared_ptr<Errors> errors=std::make_shared<Errors>(state);std::shared_ptr<Sockets> sockets=std::make_shared<Sockets>(state);std::shared_ptr<Dispatcher> dispatcher=std::make_shared<Dispatcher>(state);std::shared_ptr<Worker> worker=std::make_shared<Worker>(state);std::shared_ptr<Accept> accept=std::make_shared<Accept>(state);std::shared_ptr<Factory> factory=std::make_shared<Factory>(state);std::shared_ptr<Cancel> cancel=std::make_shared<Cancel>();std::shared_ptr<Profile> profile=std::make_shared<Profile>(state,*errors);
 std::shared_ptr<r::Runtime> runtime;std::unique_ptr<o::OwnedServerBackend> backend;
 Fixture(){check(!active,"isolated runtimefixtures do not overlap");active=state.get();}
 ~Fixture(){if(backend){if(backend->snapshot().phase!=o::Phase::Closed)std::terminate();backend.reset();}runtime.reset();active=nullptr;}
 r::Dependencies runtime_dependencies(){return {state,errors,sockets,dispatcher,worker,accept,factory,cancel,profile,&state->index,&state->epoch,{0x1111},{0x2222},{0x3333,anadius::server_recovered::ErrorCategory::asio_misc},{nullptr,move_executor},4};}
 o::Dependencies server_dependencies(){o::Dependencies d;d.state_lifetime=state;d.errors=errors;d.sockets=sockets;d.dispatcher=dispatcher;d.iocp=std::make_shared<Iocp>(state);d.gates=std::make_shared<Gates>();d.cancellation=std::make_shared<Cancellation>();d.executor=std::make_shared<Executor>();d.kernel={startup,cleanup,local_name,environment,delete_gate};return d;}
 void initialize(bool borrowed=false,std::shared_ptr<void> lifetime={}){auto rd=runtime_dependencies();if(lifetime)rd.state_lifetime=std::move(lifetime);runtime=std::make_shared<r::Runtime>(std::move(rd));state->runtime=runtime.get();auto d=server_dependencies();d.runtime=borrowed?std::static_pointer_cast<o::Runtime>(std::make_shared<BorrowedRuntime>(*runtime)):runtime;backend=std::make_unique<o::OwnedServerBackend>(native.root,d,4);backend->initialize_context_0241b0();anadius::server_recovered::Endpoint endpoint;backend->construct_acceptor_020e20(endpoint,true,INT_MAX);backend->initialize_pending_socket_0211b0();}
 void submit(){backend->async_accept_021b00();}
 void run(){const auto error=backend->run_context_0298b0();check(error.value==0,"concrete runtime returns actual source-worker success");}
 void drain_wake(){while(!state->packets.empty()){const auto packet=state->packets.front();state->packets.pop_front();check(!packet.operation&&!packet.key&&!packet.bytes,"only observed wake token remains outside completed worker");}}
 void close(){drain_wake();state->attested=true;backend->destroy_pending_socket_024db0();backend->destroy_acceptor_024db0();backend->destroy_context();check(runtime->snapshot().phase==r::Phase::Closed&&!runtime->snapshot().graph_present,"explicit retirement closesconcreteRuntime graph beforeBackendcontext");}
};
// Controlled cap for Environment row/pin ownership tests. Actual Policy and
// prepared-child authorization/error ordering are covered by separate fixtures.
struct WorkStop final:r::EnvironmentWorkStop,s::WorkerStopApi,s::WorkerStopEffects {
 s::SessionMemoryDomain* parent=nullptr;s::IocpSchedulerEntry* scheduler=nullptr;DWORD thread=0;
 Errors& errors;Dispatcher& dispatcher;Control control;
 unsigned entered=0,consumed_count=0,last_releases=0,stop_exchanges=0,wake_exchanges=0,posts=0;
 std::function<void()> before_finish,after_finish;
 WorkStop(State& state,Errors& e,Dispatcher& d):errors(e),dispatcher(d),control(state){}
 bool matches(const s::SessionMemoryDomain& p,const s::IocpSchedulerEntry& e)const noexcept override{return parent==&p&&scheduler==&e;}
 void finish_original(s::IocpSchedulerEntry& e,const std::function<void()>& checkpoint,bool& consumed)override{
  check(parent&&scheduler==&e&&thread==::GetCurrentThreadId()&&!consumed,"Controlled original finish requires exact parent/scheduler/actor and unconsumed pin");
  (void)parent->buffer_context();checkpoint();++entered;if(before_finish)before_finish();
  const auto old=e.work_count_30.fetch_sub(1,std::memory_order_seq_cst);consumed=true;++consumed_count;
  if(old==1){++last_releases;s::stop_worker_02a190(e,*this,errors,*this);}
  if(after_finish)after_finish();checkpoint();
 }
 void finish_job(s::IocpSchedulerEntry&,const s::SessionMemoryDomain::JobPermit&,const std::function<void()>&,bool&)override{throw std::logic_error("This sole-owned fixture has no prepared child authority");}
 std::int32_t post_stop(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* ov)override{++posts;check(!bytes&&!key&&!ov,"Tracked final release posts only the native zero packet");return dispatcher.post(port,bytes,key,ov);}
 std::uint32_t get_last_error()override{return dispatcher.get_last_error();}
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry& e,std::uint32_t value)override{++stop_exchanges;return control.exchange_stop(e,value);}
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry& e,std::uint32_t value)override{++wake_exchanges;return control.exchange_wake(e,value);}
 [[noreturn]]void raise_028240(s::Error error,std::uintptr_t context)override{control.raise(error,context);}
};
struct EnvFixture {
 Fixture host;WorkStop work{*host.state,*host.errors,*host.dispatcher};std::unique_ptr<oe::Environment> environment;sp::Environment config,second_config;std::optional<s::IoEnvironmentConstructorContext> constructor;std::unique_ptr<s::CoherentIoMemory> coherent;std::optional<r::Ports> ports;unsigned admissions=0;std::function<void()> admission_hook;
 explicit EnvFixture(std::size_t capacity=32,oe::ClassifierProfile classifier=oe::ClassifierProfile::Host){host.initialize();host.submit();environment=std::make_unique<oe::Environment>(capacity,classifier);config=environment->configuration();second_config=environment->configuration();host.backend->with_operation([&](o::ServerView& view){const auto original=*host.state->ports;constructor.emplace(s::IoEnvironmentConstructorContext{*config.types,config.queries,config.factories});coherent=std::make_unique<s::CoherentIoMemory>(original.accept,view.io,config.sentinels,*constructor,std::span<s::IoEnvironmentSourceBinding>{});auto completion=original.environment;completion.sentinels=config.sentinels;work.parent=&view.memory;work.scheduler=&view.scheduler;work.thread=::GetCurrentThreadId();ports.emplace(r::Ports{view,original.accept,*coherent,original.worker,original.completion,completion,original.server_identity,&work});environment->bind(*ports,[&]{++admissions;(void)ports->view.memory.buffer_context();if(admission_hook)admission_hook();});});}
 template<class F>void operation(F fn){host.backend->with_operation([&](o::ServerView& view){fn(view);});}
 s::SessionPair owner(o::ServerView& view){return view.construct_accepted({nullptr,move_executor});}
 void finish(){if(host.backend->snapshot().phase==o::Phase::Ready)host.run();else check(host.backend->snapshot().phase==o::Phase::Returned,"environment fixture preserves actual returned worker stage");host.drain_wake();operation([&](o::ServerView&){check(environment->close(true),"owned environment closes only after real source worker returns and storage drains");});environment.reset();coherent.reset();constructor.reset();ports.reset();host.close();}
};
void release_strong(s::IoReference* control){if(control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}}
std::uintptr_t query(s::IoHandlerEnvironment& value){return value.operations_10->query_slot_18(value);}
void copy_environment(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from,const sp::Environment& config){to.operations_10=from.operations_10;to.schedule_20=from.schedule_20;bool copied=false;for(const auto& binding:config.copies)if(binding.table==from.operations_10){check(!copied,"owned copy binding is unique");binding.copy_slot_08(to,from);copied=true;}check(copied,"owned environment provides exact native message copy slot");to.executor_28=from.executor_28;}
bool same_projection_fields(const s::IoHandlerEnvironment& left,const s::IoHandlerEnvironment& right){return left.payload_00==right.payload_00&&left.operations_10==right.operations_10&&left.state_18==right.state_18&&left.schedule_20==right.schedule_20&&left.executor_28==right.executor_28;}
void moves_and_copies(){
 auto* retained=new EnvFixture(1);auto& f=*retained;
 f.operation([&](o::ServerView& view){const auto& passive=f.second_config;check(passive.sentinels.empty_operations_846c8==f.config.sentinels.empty_operations_846c8&&passive.factories[0].executor_identity==f.config.factories[0].executor_identity&&passive.queries[0].query_slot_00==f.config.queries[0].query_slot_00&&!f.config.factories.empty()&&!f.config.handlers.empty()&&!f.config.responses.empty(),"passive configuration retains stable owned tables and all callback registries");auto pair=f.owner(view);s::IoHandlerEnvironment source,constructed,moved,copy;f.environment->session(source,{{1,7},*pair.object});const auto original=query(source);check(source.state_18==1&&original&&f.environment->snapshot().environments==1,"real session projection publishes active state and exact query capability");
   s::construct_io_environment_023dd0(constructed,source,f.config.sentinels,*f.constructor);check(f.environment->snapshot().environments==2&&query(source)==original&&query(constructed)!=original,"actual023dd0 active path constructs one independent reference and preserves original");const auto cap=query(constructed);s::IoHandlerExecutor unknown_executor;moved.payload_00={UINTPTR_MAX,0xafafafaf};moved.state_18=UINTPTR_MAX;moved.executor_28=&unknown_executor;s::move_io_environment_023cb0(moved,constructed,f.config.sentinels);check(query(moved)==cap&&constructed.operations_10==f.config.sentinels.empty_operations_846c8&&f.environment->snapshot().environments==2,"actual023cb0 ignores undefined destination payload/state/executor while transferring exact capability");
   copy_environment(copy,source,f.config);check(query(copy)!=original&&f.environment->snapshot().environments==3,"message copy slot adds exactly one unique projection capability");
   f.environment->release(source);f.environment->release(moved);f.environment->release(copy);f.environment->release(constructed);check(!f.environment->snapshot().environments&&f.environment->snapshot().copied==1,"native constructor clone and exact message copy add references then release drains records");
   f.environment->session(source,{{1,8},*pair.object});check(query(source)!=original,"same projection address receives a new non-recycled capability after release");s::IoEnvironmentConstructorScratch scratch;s::IoHandlerEnvironment stale;rejects<s::UnsupportedInput>([&]{f.config.factories[0].construct_slot_58(stale,original,scratch);},"expired query capability cannot resolve reused projection address");check(f.environment->snapshot().phase==oe::Phase::Bound&&f.environment->snapshot().environments==1,"stale capability rejection cannot alias new generation or poison unrelated owner");
   s::IoHandlerEnvironment prepared,response20,response88;s::IoPrepareScratch prepare;s::ResponseExecutorScratch response;source.executor_28->prepare_slot_40(prepared,query(source),prepare);f.config.response_executors[0].factory_20(response20,query(source),0x180084425ULL);f.config.response_executors[0].factory_88(response88,query(source),response);check(f.environment->snapshot().environments==4&&prepared.state_18==1&&response20.state_18==1&&response88.state_18==1,"cached context-free prepare/response factories resolve exact live projection capability");f.environment->release(prepared);f.environment->release(response20);f.environment->release(response88);f.environment->release(source);s::IoHandlerEnvironment accept,empty;f.environment->accept(accept,f.ports->server_identity+0x68);check(accept.state_18==0,"accept projection is explicit empty-type state0");s::construct_io_environment_023dd0(empty,accept,f.config.sentinels,*f.constructor);check(empty.state_18==0&&f.environment->snapshot().environments==2,"actual023dd0 preserves accept state0 while cloning owned environment type");f.environment->release(accept);f.environment->release(empty);
   s::IoHandlerEnvironment unknown{{UINTPTR_MAX,0xabababab},f.config.sentinels.empty_operations_846c8,0,f.config.sentinels.empty_schedule_846e8,f.config.sentinels.empty_executor_84770};f.config.sentinels.empty_operations_846c8->destroy_slot_00(unknown);s::IoHandlerEnvironment unknown_move{{0xbabababa,UINTPTR_MAX},f.config.sentinels.empty_operations_846c8,0,f.config.sentinels.empty_schedule_846e8,f.config.sentinels.empty_executor_84770};s::IoHandlerEnvironment zero;s::move_io_environment_023cb0(zero,unknown_move,f.config.sentinels);f.environment->release(zero);s::IoHandlerEnvironment pure_empty{{0xcccccccc,UINTPTR_MAX},f.config.sentinels.empty_operations_846c8,0,f.config.sentinels.empty_schedule_846e8,f.config.sentinels.empty_executor_84770};s::IoHandlerEnvironment empty_constructed;s::construct_io_environment_023dd0(empty_constructed,pure_empty,f.config.sentinels,*f.constructor);check(empty_constructed.operations_10==f.config.sentinels.empty_operations_846c8,"actual023dd0 empty RTTI path constructs pure sentinel without resolving unknown payload");f.environment->release(empty_constructed);check(!f.environment->snapshot().environments,"pure empty moved-source helpers do not resolve unknown payload capabilities");release_strong(pair.control);
   ::SetLastError(0x6a31);const auto snapshot=f.environment->snapshot();check(::GetLastError()==0x6a31&&snapshot.active_calls==0&&snapshot.token,"environment snapshot preserves LastError and reports no retained active scope");
 });f.finish();delete retained;
}
void reference_ledger_growth(){
 // Allocate every destination before registering its address. These are
 // environment metadata projections, not thousands of network clients.
 constexpr std::size_t count=4100;
 std::vector<s::IoHandlerEnvironment> held(count);
 std::vector<std::uintptr_t> capabilities(count);
 std::vector<std::uintptr_t> ordered(count);
 auto* retained=new EnvFixture(4096);auto& f=*retained;
 f.operation([&](o::ServerView& view){
  auto pair=f.owner(view);auto& source=held[0];
  f.environment->session(source,{{3,9},*pair.object});
  const auto original=source;capabilities[0]=query(source);
  s::IoHandlerEnvironment first_constructed;
  std::uint64_t factories=0,copies=0;
  for(std::size_t i=1;i<count;++i){
   if(i&1){s::construct_io_environment_023dd0(held[i],source,f.config.sentinels,*f.constructor);++factories;}
   else{copy_environment(held[i],source,f.config);++copies;}
   capabilities[i]=query(held[i]);
   check(held[i].state_18==original.state_18&&held[i].operations_10==original.operations_10&&held[i].schedule_20==original.schedule_20&&held[i].executor_28==original.executor_28,"Factory/copy growth preserves each projection's selected state and tables");
   if(i==1)first_constructed=held[i];
  }
  std::copy(capabilities.begin(),capabilities.end(),ordered.begin());std::sort(ordered.begin(),ordered.end());
  check(ordered.front()&&std::adjacent_find(ordered.begin(),ordered.end())==ordered.end(),"4100 simultaneous projection capabilities are nonzero and unique beyond default4096");
  check(source.payload_00==original.payload_00&&source.operations_10==original.operations_10&&source.state_18==original.state_18&&source.schedule_20==original.schedule_20&&source.executor_28==original.executor_28&&query(source)==capabilities[0],"Growing the reference ledger preserves original source fields and query capability");
  check(held[1].payload_00==first_constructed.payload_00&&held[1].operations_10==first_constructed.operations_10&&held[1].state_18==first_constructed.state_18&&held[1].schedule_20==first_constructed.schedule_20&&held[1].executor_28==first_constructed.executor_28&&query(held[1])==capabilities[1],"Old constructed projection remains registered at the same address across ledger growth");
  const auto full=f.environment->snapshot();
  check(full.phase==oe::Phase::Bound&&full.environments==count&&full.created==factories+1&&full.copied==copies&&!full.destroyed&&!full.active_calls,"All4100 held references have exact factory/copy counters without poison or retained active scope");
  constexpr std::array<std::size_t,3> holes{1,2048,count-1};
  for(const auto i:holes)f.environment->release(held[i]);
  check(f.environment->snapshot().environments==count-holes.size()&&f.environment->snapshot().destroyed==holes.size(),"Releasing selected old/new rows removes exactly three references");
  for(const auto i:holes){
   s::construct_io_environment_023dd0(held[i],source,f.config.sentinels,*f.constructor);
   const auto replacement=query(held[i]);
   check(replacement!=capabilities[i]&&query(source)==capabilities[0],"Reused destination address receives a fresh capability while source remains live");
   s::IoHandlerEnvironment stale;s::IoEnvironmentConstructorScratch scratch;
   rejects<s::UnsupportedInput>([&]{f.config.factories[0].construct_slot_58(stale,capabilities[i],scratch);},"Released capability cannot resolve a reused ledger row");
   capabilities[i]=replacement;
  }
  std::copy(capabilities.begin(),capabilities.end(),ordered.begin());std::sort(ordered.begin(),ordered.end());
  check(ordered.front()&&std::adjacent_find(ordered.begin(),ordered.end())==ordered.end(),"Reused rows retain unique capabilities among all4100 live projections");
  const auto reused=f.environment->snapshot();
  check(reused.phase==oe::Phase::Bound&&reused.environments==count&&reused.created==factories+1+holes.size()&&reused.copied==copies&&reused.destroyed==holes.size(),"Row reuse preserves exact reference accounting and rejects expired capability without poisoning");
  for(auto& value:held)f.environment->release(value);
  const auto drained=f.environment->snapshot();
  check(drained.phase==oe::Phase::Bound&&!drained.environments&&!drained.active_calls&&drained.destroyed==drained.created+drained.copied&&drained.destroyed==count+holes.size(),"Every grown/reused environment reference drains exactly once");
  release_strong(pair.control);
 });f.finish();delete retained;
}
void multi_session_projection_lineage(){
 auto* retained=new EnvFixture;auto& f=*retained;
 f.operation([&](o::ServerView& view){
  auto first=f.owner(view),second=f.owner(view);
  s::IoHandlerEnvironment a,b,copy,constructed,moved;
  f.environment->session(a,{{2,7},*first.object});f.environment->session(b,{{2,8},*second.object});
  const auto qa=query(a),qb=query(b);check(qa!=qb&&a.state_18==1&&b.state_18==1,"Distinct session generations/owners remain distinct while public active state is identical");
  copy_environment(copy,a,f.config);
  s::construct_io_environment_023dd0(constructed,copy,f.config.sentinels,*f.constructor);
  const auto copied=query(copy),cloned=query(constructed);
  s::move_io_environment_023cb0(moved,constructed,f.config.sentinels);
  check(query(a)==qa&&query(b)==qb&&query(moved)==cloned&&copied!=qa&&cloned!=copied&&f.environment->snapshot().environments==4,"Origin-carrying copy/factory/move preserve independent source capabilities and exact ref count");
  f.environment->release(copy);f.environment->release(moved);f.environment->release(a);f.environment->release(b);
  check(!f.environment->snapshot().environments,"Original worker retires all projection lineages without a child authority fallback");
  release_strong(first.control);release_strong(second.control);
 });f.finish();delete retained;
}
void root_move_admission_boundary(){
 // This fixture has a controlled sole-worker FLS table and WorkStop. It tests
 // root callback timing/ownership only; it cannot mint a real child JobPermit.
 auto* retained=new EnvFixture(1);auto& f=*retained;
 f.operation([&](o::ServerView& view){
  auto pair=f.owner(view);s::IoHandlerEnvironment source,destination;s::IoHandlerExecutor undefined_executor;
  f.environment->session(source,{{4,3},*pair.object});const auto capability=query(source);const auto before=f.environment->snapshot();
  destination.payload_00={UINTPTR_MAX,0x51a751a7};destination.state_18=UINTPTR_MAX;destination.executor_28=&undefined_executor;const auto original_source=source,original_destination=destination;
  unsigned callbacks=0;bool saw_full_source=false;f.admission_hook=[&]{++callbacks;const auto during=f.environment->snapshot();check(during.phase==oe::Phase::Bound&&during.active_calls==1,"Whole owned root move publishes one admitted call before an opaque source checkpoint");(void)view.memory.buffer_context();if(callbacks==1){saw_full_source=same_projection_fields(source,original_source)&&same_projection_fields(destination,original_destination);check(saw_full_source,"Owned whole-move admission precedes every source/destination native header mutation");}};
  ::SetLastError(0x6a42);s::move_io_environment_023cb0(destination,source,f.config.sentinels);const auto last_error=::GetLastError();f.admission_hook={};
  const auto moved=f.environment->snapshot();check(last_error==0x6a42&&callbacks>=2&&saw_full_source&&query(destination)==capability&&source.operations_10==f.config.sentinels.empty_operations_846c8&&source.executor_28==f.config.sentinels.empty_executor_84770,"Whole root admission retains native023cb0 capability transfer and moved-from sentinels without interpreting undefined destination fields");
  check(moved.phase==oe::Phase::Bound&&moved.environments==before.environments&&moved.created==before.created&&moved.copied==before.copied&&moved.destroyed==before.destroyed&&moved.moved==before.moved+1&&!moved.active_calls,"Successful whole root move transfers one existing reference and retires only its completed call record");
  f.environment->release(source);f.environment->release(destination);check(!f.environment->snapshot().environments,"Normal whole-move source and destination cleanup retire their one real projection once");release_strong(pair.control);
 });f.finish();delete retained;
}
void root_copy_executor_tail_boundary(){
 auto* retained=new EnvFixture(1);auto& f=*retained;
 f.operation([&](o::ServerView& view){
  auto pair=f.owner(view);s::IoHandlerEnvironment source,destination;s::IoHandlerExecutor placeholder;
  f.environment->session(source,{{4,4},*pair.object});const auto source_cap=query(source);const auto before=f.environment->snapshot();const auto original_source=source;destination.executor_28=&placeholder;
  unsigned callbacks=0;bool payload_published=false;
  f.admission_hook=[&]{
   ++callbacks;const auto during=f.environment->snapshot();check(during.phase==oe::Phase::Bound&&during.active_calls==1&&same_projection_fields(source,original_source),"Raw owned copy callback observes one admitted effect and an unchanged exact source");
   // Raw slot08 owns payload/state. Its caller still owns the live executor
   // reload AFTER the slot returns; do not query the incomplete destination.
   if(during.copied==before.copied+1&&destination.payload_00[0]==original_source.payload_00[0]&&destination.payload_00[1]){
    payload_published=true;check(destination.executor_28==&placeholder&&destination.state_18==original_source.state_18&&destination.operations_10==original_source.operations_10&&destination.schedule_20==original_source.schedule_20,"Published raw copy payload keeps the caller's placeholder executor through every post-copy checkpoint until slot08 returns");
   }
   (void)view.memory.buffer_context();
  };
  copy_environment(destination,source,f.config);f.admission_hook={};const auto after=f.environment->snapshot();
  check(callbacks>=2&&payload_published&&destination.executor_28==original_source.executor_28&&query(destination)!=source_cap&&query(source)==source_cap&&same_projection_fields(source,original_source),"Only the caller's after-return tail loads the actual source executor while preserving independent copied capability and unchanged source");
  check(after.phase==oe::Phase::Bound&&!after.active_calls&&after.environments==before.environments+1&&after.copied==before.copied+1&&after.created==before.created&&after.moved==before.moved&&after.destroyed==before.destroyed,"Raw slot08 copy publishes one reference/copy and retires its completed effect without an early executor-tail write");
  f.environment->release(destination);f.environment->release(source);check(!f.environment->snapshot().environments&&f.environment->snapshot().destroyed==2,"Raw copy source and independent destination each retire their actual projection once");release_strong(pair.control);
 });f.finish();delete retained;
}
void root_message_copy_caller_boundary(){
 // Select the actual public copy segment over this owner's exported bindings.
 // Its span overload avoids manufacturing an unused message/dispatch context.
 auto* retained=new EnvFixture(1);auto& f=*retained;
 f.operation([&](o::ServerView& view){
  auto pair=f.owner(view);s::IoHandlerEnvironment source,destination;s::IoHandlerExecutor placeholder;
  f.environment->session(source,{{4,5},*pair.object});const auto source_cap=query(source);const auto original_source=source;const auto before=f.environment->snapshot();destination.executor_28=&placeholder;const auto original_destination=destination;
  unsigned callbacks=0;bool preprefix=false,slot_payload=false,caller_tail=false;
  f.admission_hook=[&]{
   ++callbacks;const auto during=f.environment->snapshot();check(during.phase==oe::Phase::Bound&&during.active_calls==1&&same_projection_fields(source,original_source),"Actual owned message copy caller retains one exact admitted effect and its unchanged source through slot and tail callbacks");
   if(callbacks==1){preprefix=same_projection_fields(destination,original_destination);check(preprefix,"Owned message copy caller claims source and destination before writing operations or schedule");}
   if(during.copied==before.copied+1&&destination.payload_00[0]==original_source.payload_00[0]&&destination.payload_00[1]){
    check(destination.operations_10==original_source.operations_10&&destination.schedule_20==original_source.schedule_20&&destination.state_18==original_source.state_18,"Published message copy slot has exact payload/state and caller-written native prefix");
    if(destination.executor_28==&placeholder){check(!caller_tail,"Raw message slot checkpoint precedes the owning caller executor tail");slot_payload=true;}
    else{check(slot_payload&&destination.executor_28==original_source.executor_28,"Only after a completed raw slot does the owning caller expose its full executor tail under the same admitted effect");caller_tail=true;}
   }
   (void)view.memory.buffer_context();
  };
  s::copy_message_environment_028d30(destination,source,std::span<const s::MessageEnvironmentCopyBinding>{f.config.copies});f.admission_hook={};const auto after=f.environment->snapshot();
  check(callbacks>=3&&preprefix&&slot_payload&&caller_tail&&destination.executor_28==original_source.executor_28&&query(destination)!=source_cap&&query(source)==source_cap,"Actual selected owned Environment protects the message copy caller before prefix, across raw slot08 and through completed executor tail");
  check(after.phase==oe::Phase::Bound&&!after.active_calls&&after.environments==before.environments+1&&after.copied==before.copied+1&&after.created==before.created&&after.moved==before.moved&&after.destroyed==before.destroyed,"Guarded actual caller creates exactly one copied reference and retires its one healthy ownership effect only after the full tail");
  f.environment->release(destination);f.environment->release(source);check(!f.environment->snapshot().environments,"Guarded message copy source and destination drain their real projections normally");release_strong(pair.control);
 });f.finish();delete retained;
}
struct TaskStatus {unsigned called=0,destroyed=0;};
struct SelfDeletingTask final:s::OwnedHandlerTask {TaskStatus& status;s::OwnedHandlerTask*& slot;oe::Environment& environment;s::IoHandlerEnvironment& source;SelfDeletingTask(TaskStatus& t,s::OwnedHandlerTask*& s,oe::Environment& e,s::IoHandlerEnvironment& v):status(t),slot(s),environment(e),source(v){}~SelfDeletingTask()override{++status.destroyed;}void call_slot_00(std::uint8_t flag)override{check(flag==1&&!slot,"inline owned handler consumes caller slot BEFORE callback");++status.called;environment.release(source);delete this;}};
struct Functor final:s::PostedFunctor {unsigned calls=0,discards=0;std::function<void()> hook;void call_slot_00(std::uint8_t flag)override{if(flag)++calls;else++discards;if(hook)hook();}};
void schedulers_and_strict_close(){
 auto* retained=new EnvFixture;auto& f=*retained;TaskStatus status;Functor functor;s::PostedFunctor* slot=&functor;
 f.operation([&](o::ServerView& view){auto pair=f.owner(view);s::IoHandlerEnvironment handler;f.environment->session(handler,{{2,1},*pair.object});s::OwnedHandlerTask* task=nullptr;task=new SelfDeletingTask(status,task,*f.environment,handler);f.config.handlers[0].table->owned_slot_10(handler,task);check(!task&&status.called==1&&status.destroyed==1&&!f.environment->snapshot().environments,"inline handler may destroy itself and environment without scheduler reread");release_strong(pair.control);
   s::IoHandlerEnvironment response;f.environment->accept(response,f.ports->server_identity+0x68);s::WorkerThreadState thread{};s::WorkerTlsContext tls{&view.scheduler.completion.port,&thread,nullptr};f.host.state->tls=&tls;f.config.responses[0].table->owned_slot_10(response,slot);f.host.state->tls=nullptr;check(!slot&&!functor.calls&&!functor.discards&&f.host.state->posts==1,"actual022e40 force-post1 bypasses matching worker TLS inline path and consumes source slot");f.environment->release(response);check(!f.environment->snapshot().environments&&!f.environment->close(true),"refs0 and external attestation cannot retire source posted functor/accept/work allocations");
 });f.host.run();f.host.drain_wake();check(functor.calls==1&&!functor.discards&&f.host.state->handled==2,"actual native worker invokes posted response and observed accept completion exactly once");
 f.operation([&](o::ServerView& view){check(!f.environment->close(false),"false external drain never retires code backing with empty source counters");view.scheduler.work_count_30=1;check(!f.environment->close(true),"nonzero source work count blocks environment retirement");view.scheduler.work_count_30=0;s::CancelOperation queued;view.scheduler.completion.head=&queued;view.scheduler.completion.tail=&queued;check(!f.environment->close(true),"source completion queue blocks close despite external attestation");view.scheduler.completion.head=nullptr;view.scheduler.completion.tail=nullptr;
   auto& storage=f.coherent->allocate_04b0bc(0x49);auto& operation=f.coherent->project_posted(storage);Functor retained_functor;operation.functor_38=&retained_functor;check(!f.environment->close(true)&&view.io.snapshot().allocations==1,"unpublished real posted-functor allocation blocks close after worker return and refs0");s::complete_posted_027120(0,operation,f.ports->environment.storage,*f.host.worker,f.coherent->posted());check(retained_functor.discards==1&&!view.io.snapshot().allocations,"actual owner0 posted cleanup retires controlled unpublished functor/storage");
 });f.finish();delete retained;
}
const s::IoEnvironmentFactoryBinding& factory_binding(const s::IoHandlerEnvironment& value,const sp::Environment& config){const s::IoEnvironmentFactoryBinding* found=nullptr;for(const auto& b:config.factories)if(b.executor_identity==value.executor_28){check(!found,"Native family has one exact constructor binding");found=&b;}check(found&&found->construct_slot_58,"Native family constructor58 is exported");return *found;}
const s::ResponseExecutorBinding& response_binding(const s::IoHandlerEnvironment& value,const sp::Environment& config){const s::ResponseExecutorBinding* found=nullptr;for(const auto& b:config.response_executors)if(b.identity==value.executor_28){check(!found,"Native family has one exact response executor binding");found=&b;}check(found&&found->factory_20&&found->factory_88,"Native family response20/88 are exported");return *found;}
struct CountedHandler final:s::OwnedHandlerTask {unsigned calls=0,discards=0;void call_slot_00(std::uint8_t flag)override{if(flag)++calls;else ++discards;}};
std::uintptr_t environment_type(const s::IoHandlerEnvironment& value,const sp::Environment& config){s::IoEnvironmentTypeQuery found=nullptr;for(const auto& b:config.queries)if(b.schedule_identity==value.schedule_20){check(!found,"Native family has one exact type query");found=b.query_slot_00;}check(found,"Native family type query is exported");return found();}
void publish_response(s::IoHandlerEnvironment& value,Functor& functor,const sp::Environment& config){s::ResponseOwnedSchedule found=nullptr;for(const auto& b:config.responses)if(b.identity==value.schedule_20){check(!found&&b.table&&!b.table->borrowed_slot_18,"Concrete native response schedule selects owned10 without borrowed18");found=b.table->owned_slot_10;}check(found,"Concrete native response owns an exported publication slot");s::PostedFunctor* slot=&functor;found(value,slot);check(!slot,"Native response publication consumes the exact caller slot");}
void publish_handler(s::IoHandlerEnvironment& value,CountedHandler& handler,const sp::Environment& config){s::OwnedHandlerSchedule found=nullptr;for(const auto& b:config.handlers)if(b.identity==value.schedule_20){check(!found&&b.table&&!b.table->borrowed_slot_18,"Concrete native handler schedule selects owned10 without borrowed18");found=b.table->owned_slot_10;}check(found,"Concrete native handler owns an exported publication slot");s::OwnedHandlerTask* slot=&handler;found(value,slot);check(!slot,"Typed native handler publication consumes the exact caller slot");}
void tracked_executor_families(){
 constexpr std::size_t held_count=70;
 std::vector<s::IoHandlerEnvironment> held(held_count);std::vector<std::uintptr_t> capabilities(held_count),ordered(held_count);
 auto* retained=new EnvFixture(1,oe::ClassifierProfile::OriginalDefault);auto& f=*retained;
 s::SessionPair pair{};s::IoHandlerEnvironment source,final;
 Functor inline_initial,inline_initial_clear,inline_tracked,inline_tracked_clear,posted_initial,posted_fork,posted_tracked,posted_clone;
 CountedHandler inline_handler,posted_handler;
 f.operation([&](o::ServerView& view){
  pair=f.owner(view);f.environment->session(source,{{5,1},*pair.object});const auto baseline=view.scheduler.work_count_30.load();const auto original=query(source);
  check(environment_type(source,f.config)==0x1800a25c0ULL&&baseline==1,"Original initial source has native initial type and only the fixture accept work pin");
  s::IoHandlerEnvironment tracked,constructed,copy,moved;s::IoEnvironmentConstructorScratch scratch;
  factory_binding(source,f.config).construct_slot_58(tracked,query(source),scratch);const auto tracked_cap=query(tracked);
  check(tracked.executor_28!=source.executor_28&&tracked.schedule_20!=source.schedule_20&&environment_type(tracked,f.config)==0x1800a2690ULL&&view.scheduler.work_count_30==baseline+1,"Actual exported initial factory58 creates the tracked type/executor and one work retain");
  check(f.config.types->compare_type_names_04d2d0(environment_type(tracked,f.config)+8,0x1800a25c8ULL)!=0,"Tracked type selects actual023dd0 nonempty factory path");
  s::construct_io_environment_023dd0(constructed,tracked,f.config.sentinels,*f.constructor);const auto constructed_cap=query(constructed);
  copy_environment(copy,tracked,f.config);check(view.scheduler.work_count_30==baseline+3&&query(copy)!=tracked_cap&&constructed_cap!=tracked_cap&&constructed.executor_28==tracked.executor_28,"Actual023dd0 tracked factory and message copy each retain one independent work pin");
  s::move_io_environment_023cb0(moved,constructed,f.config.sentinels);f.environment->release(constructed);
  check(query(moved)==constructed_cap&&view.scheduler.work_count_30==baseline+3&&!f.work.entered,"Actual tracked move transfers capability/pin and moved-from cleanup does not finish it");
  f.environment->release(copy);f.environment->release(moved);check(view.scheduler.work_count_30==baseline+1&&f.work.entered==2&&f.work.consumed_count==2&&!f.work.last_releases,"Ordinary tracked destruction invokes one consumed finish per pin without premature last-release stop");
  s::IoHandlerEnvironment prepared,forced,forked,cleared,initial_forced,initial_fork,initial_clear;s::IoPrepareScratch prepare;s::ResponseExecutorScratch response;
  tracked.executor_28->prepare_slot_40(prepared,query(tracked),prepare);
  response_binding(prepared,f.config).factory_20(forced,query(prepared),0x180084425ULL);
  response_binding(forced,f.config).factory_88(forked,query(forced),response);
  forked.executor_28->prepare_slot_40(cleared,query(forked),prepare);
  check(view.scheduler.work_count_30==baseline+5&&prepared.executor_28==tracked.executor_28&&forced.executor_28==tracked.executor_28&&forked.executor_28==tracked.executor_28&&cleared.executor_28==tracked.executor_28,"Tracked prepare40/response20/response88 keep family and each add one owned retain");
  response_binding(source,f.config).factory_20(initial_forced,query(source),0x180084425ULL);
  response_binding(initial_forced,f.config).factory_88(initial_fork,query(initial_forced),response);
  initial_forced.executor_28->prepare_slot_40(initial_clear,query(initial_forced),prepare);
  check(view.scheduler.work_count_30==baseline+5&&initial_forced.executor_28==source.executor_28&&initial_fork.executor_28==source.executor_28&&initial_clear.executor_28==source.executor_28,"Initial response/prepare transformations add ledger projections without tracked work retains");
  s::IoHandlerEnvironment forced_clone;s::construct_io_environment_023dd0(forced_clone,forced,f.config.sentinels,*f.constructor);
  check(view.scheduler.work_count_30==baseline+6&&forced_clone.executor_28==tracked.executor_28&&query(forced_clone)!=query(forced),"Actual023dd0 factory58 preserves a tracked response's bit0 family while retaining one new work pin");
  const auto io_before=view.io.snapshot().allocations;const auto posts_before=f.host.state->posts;
  s::WorkerThreadState thread{};s::WorkerTlsContext tls{&view.scheduler.completion.port,&thread,nullptr};f.host.state->tls=&tls;
  publish_response(source,inline_initial,f.config);publish_response(initial_clear,inline_initial_clear,f.config);publish_response(prepared,inline_tracked,f.config);publish_response(cleared,inline_tracked_clear,f.config);publish_handler(prepared,inline_handler,f.config);
  check(inline_initial.calls==1&&inline_initial_clear.calls==1&&inline_tracked.calls==1&&inline_tracked_clear.calls==1&&inline_handler.calls==1&&!inline_handler.discards&&f.host.state->posts==posts_before&&view.io.snapshot().allocations==io_before,"Native flags0 and prepare-cleared bit0 choose actual matching-TLS inline response/typed-handler paths in both families");
  publish_response(initial_forced,posted_initial,f.config);publish_response(initial_fork,posted_fork,f.config);publish_response(forked,posted_tracked,f.config);publish_response(forced_clone,posted_clone,f.config);publish_handler(forked,posted_handler,f.config);f.host.state->tls=nullptr;
  check(!posted_initial.calls&&!posted_fork.calls&&!posted_tracked.calls&&!posted_clone.calls&&!posted_handler.calls&&f.host.state->posts==posts_before+5&&view.io.snapshot().allocations==io_before+5&&view.scheduler.work_count_30==baseline+11,"Response20 sets force-post bit0, response88/factory58 preserve it, and actual028ca0 adds five response/typed-handler posted pins distinct from six family pins");
  const auto finishes_before_growth=f.work.entered;
  for(std::size_t i=0;i<held_count;++i){if(i&1)copy_environment(held[i],tracked,f.config);else factory_binding(tracked,f.config).construct_slot_58(held[i],query(tracked),scratch);capabilities[i]=query(held[i]);}
  std::copy(capabilities.begin(),capabilities.end(),ordered.begin());std::sort(ordered.begin(),ordered.end());
  check(ordered.front()&&std::adjacent_find(ordered.begin(),ordered.end())==ordered.end()&&view.scheduler.work_count_30==baseline+11+held_count&&f.work.entered==finishes_before_growth&&query(source)==original&&query(tracked)==tracked_cap,"Seventy stable-address tracked factories/copies grow initial1 ledger without relocating old capabilities or duplicating finishes/pins");
  for(auto& value:held)f.environment->release(value);
  f.environment->release(prepared);f.environment->release(forced);f.environment->release(forked);f.environment->release(cleared);f.environment->release(forced_clone);f.environment->release(tracked);
  check(view.scheduler.work_count_30==baseline+5&&f.work.entered==held_count+8&&f.work.consumed_count==held_count+8&&!f.work.last_releases,"All tracked roots/copies/factory results drain exactly one family pin while five posted pins remain");
  factory_binding(source,f.config).construct_slot_58(final,query(source),scratch);
  f.environment->release(initial_forced);f.environment->release(initial_fork);f.environment->release(initial_clear);check(f.environment->snapshot().environments==2&&view.scheduler.work_count_30==baseline+6,"Only the protected initial source and one tracked root remain alongside posted/accept work");
  // Actual explicit stop lets the worker return with one retained family pin.
  // Its later destruction is cleanup, not a new Returned-phase construction.
  s::stop_worker_02a190(view.scheduler,f.work,*f.host.errors,f.work);
 });
 f.host.run();f.host.drain_wake();check(posted_initial.calls==1&&posted_fork.calls==1&&posted_tracked.calls==1&&posted_clone.calls==1&&posted_handler.calls==1&&!posted_initial.discards&&!posted_fork.discards&&!posted_tracked.discards&&!posted_clone.discards&&!posted_handler.discards,"Actual source worker consumes all five posted response/typed-handler pins and calls once");
 f.operation([&](o::ServerView& view){
  check(view.scheduler.work_count_30==1&&view.scheduler.high_34==1,"Actual explicit-stop worker drains all posted/accept work and retains only the tracked root for Returned cleanup");
  const auto stops=f.work.stop_exchanges,wakes=f.work.wake_exchanges,posts=f.work.posts;f.environment->release(final);
  check(view.scheduler.work_count_30==0&&f.work.entered==held_count+9&&f.work.consumed_count==held_count+9&&f.work.last_releases==1&&f.work.stop_exchanges==stops+1&&f.work.wake_exchanges==wakes&&f.work.posts==posts,"Root last tracked release in Returned cleanup invokes actual02a190 immediately and respects already-set stop without a second wake/post");
  f.environment->release(source);check(!f.environment->snapshot().environments,"Returned root cleanup retires final source metadata");release_strong(pair.control);
 });f.finish();delete retained;
}
struct DirectCompletion final:s::IoCompletionEffects {
 unsigned receives=0,sends=0,schedules=0;
 std::function<void(bool,s::IoReference*,std::uint64_t)> body;
 s::Category misc_category_0285b0()override{unused();}
 void receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{check(!e.value&&!flag,"Direct OriginalDefault receive preserves success/flag0");++receives;body(true,h.control_a8,bytes);}
 void send_body_0261d0(s::SendHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{check(!e.value&&!flag,"Direct OriginalDefault send preserves success/flag0");++sends;body(false,h.control_98,bytes);}
 void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{++schedules;unused();}
 void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{++schedules;unused();}
};
void original_default_classifier(){
 auto* retained=new EnvFixture(1,oe::ClassifierProfile::OriginalDefault);auto& f=*retained;Functor response;
 f.operation([&](o::ServerView& view){
  auto pair=f.owner(view);s::IoHandlerEnvironment source,clone,copy;
  f.environment->session(source,{{4,1},*pair.object});const auto source_cap=query(source);
  check(source.state_18==1&&source.operations_10!=f.config.sentinels.empty_operations_846c8&&source_cap&&f.environment->snapshot().environments==1,"OriginalDefault keeps the real session source live with registered context/state1");
  const auto initial=f.environment->snapshot();
  const auto require_empty=[&](const s::IoHandlerEnvironment& value){check(!value.state_18&&value.operations_10==f.config.sentinels.empty_operations_846c8&&value.schedule_20==f.config.sentinels.empty_schedule_846e8&&value.executor_28==f.config.sentinels.empty_executor_84770,"Actual023dd0 OriginalDefault clone is the complete empty state0 sentinel family");};
  s::construct_io_environment_023dd0(clone,source,f.config.sentinels,*f.constructor);require_empty(clone);
  rejects<s::UnsupportedInput>([&]{(void)query(clone);},"Empty clone has no capability query and must not be treated as a registered root");
  const auto cloned=f.environment->snapshot();check(cloned.phase==oe::Phase::Bound&&cloned.environments==1&&cloned.created==initial.created&&cloned.copied==initial.copied&&cloned.moved==initial.moved&&query(source)==source_cap,"Empty RTTI branch invokes no owned factory/move and preserves source reference/capability");
  f.environment->release(clone);
  const auto io_before=view.io.snapshot();const auto memory_before=view.memory.snapshot();const auto strong=pair.control->strong_08.load();const auto work=view.scheduler.work_count_30.load();
  s::resize_receive_buffer_010040(pair.object->input,3,'i',view.memory.buffer_context());
  const s::Buffer buffer{3,reinterpret_cast<char*>(s::receive_buffer_address(pair.object->input))};
  auto& receive_storage=f.coherent->allocate_04b0bc(225);auto& receive=f.coherent->project_async_receive(receive_storage);
  auto& send_storage=f.coherent->allocate_04b0bc(209);auto& send=f.coherent->project_async_send(send_storage);
  pair.control->strong_08.fetch_add(2);receive.io.handler.control_a8=pair.control;receive.io.handler.object_a0=pair.object->identity();send.io.handler.control_98=pair.control;send.io.handler.object_90=pair.object->identity();
  s::construct_io_environment_023dd0(receive.io.environment_b0,source,f.config.sentinels,*f.constructor);require_empty(receive.io.environment_b0);
  s::construct_io_environment_023dd0(send.io.environment_a0,source,f.config.sentinels,*f.constructor);require_empty(send.io.environment_a0);
  const auto rh=view.io.arm_receive(201,receive.io.operation.overlapped,buffer),sh=view.io.arm_send(201,send.io.operation.overlapped,buffer);
  view.io.note_submission(rh,-1,997);view.io.note_submission(sh,-1,997);
  check(view.io.snapshot().pending==io_before.pending+2&&view.memory.snapshot().buffer_borrows==memory_before.buffer_borrows+2&&pair.control->strong_08==strong+4,"Two actual owned SDK loans retain exactly two buffer pins and two extra source strongs");
  DirectCompletion effects;effects.body=[&](bool receiving,s::IoReference* control,std::uint64_t bytes){check(control==pair.control&&bytes==3&&view.io.snapshot().pending==io_before.pending+(receiving?1:0)&&view.io.snapshot().allocations==io_before.allocations+(receiving?1:0)&&pair.control->strong_08==strong+(receiving?3:1),"Actual completion releases its SDK loan/allocation before direct body while retaining only live handler/other-operation ownership");};
  view.io.complete_receive_with_memory(rh,1,{},3,f.ports->environment,*f.host.worker,*f.host.errors,effects,f.coherent->posted());
  view.io.complete_send_with_memory(sh,1,{},3,f.ports->environment,*f.host.worker,*f.host.errors,effects,f.coherent->posted());
  check(effects.receives==1&&effects.sends==1&&!effects.schedules&&view.io.snapshot().allocations==io_before.allocations&&view.io.snapshot().pending==io_before.pending&&view.memory.snapshot().buffer_borrows==memory_before.buffer_borrows&&pair.control->strong_08==strong&&view.scheduler.work_count_30==work,"Actual027710/027a20 invoke each body directly with no scheduler/task path and drain both loans/handler strongs once; classifier adds no work pin");
  rejects<std::invalid_argument>([&]{(void)view.io.loan(rh);},"Released receive generation cannot replay a consumed loan");rejects<std::invalid_argument>([&]{(void)view.io.loan(sh);},"Released send generation cannot replay a consumed loan");
  copy_environment(copy,source,f.config);check(copy.state_18==1&&query(copy)!=source_cap&&f.environment->snapshot().environments==2,"OriginalDefault message copy still creates a live independent registered context");
  s::PostedFunctor* slot=&response;f.config.responses[0].table->owned_slot_10(copy,slot);check(!slot&&!response.calls&&!response.discards&&f.host.state->posts==1,"OriginalDefault response machinery still uses actual force-post source path");
  f.environment->release(copy);f.environment->release(source);const auto drained=f.environment->snapshot();check(!drained.environments&&drained.created==1&&drained.copied==1&&drained.destroyed==2,"Only session source/message copy own ledger references in OriginalDefault");release_strong(pair.control);
 });f.finish();check(response.calls==1&&!response.discards,"Actual source worker completes the OriginalDefault posted response once");delete retained;
}
void validation_and_callback_lifetime(){
 rejects<std::invalid_argument>([]{oe::Environment invalid(0);},"environment registry rejects zero capacity");rejects<std::invalid_argument>([]{oe::Environment invalid(1048577);},"environment registry has explicit bounded capacity");
 auto* retained=new EnvFixture;auto& f=*retained;auto peer=std::make_unique<oe::Environment>(16);const auto config=peer->configuration();const auto peer_token=peer->snapshot().token;check(peer_token!=f.environment->snapshot().token&&config.sentinels.empty_operations_846c8==f.config.sentinels.empty_operations_846c8,"independent controllers own distinct capabilities and share only pure global sentinel");
 s::IoHandlerEnvironment source,other;std::uintptr_t expired=0;
 f.operation([&](o::ServerView& view){auto pair=f.owner(view);oe::Environment unbound;rejects<std::logic_error>([&]{unbound.session(other,{{1,1},*pair.object});},"unbound environment rejects source work before parent/provider use");
   peer->bind(*f.ports,[&]{(void)view.memory.buffer_context();});f.environment->session(source,{{1,1},*pair.object});peer->session(other,{{1,1},*pair.object});const auto before=f.environment->snapshot().environments;rejects<s::UnsupportedInput>([&]{f.environment->release(other);},"foreign controller table cannot release another controller projection");check(f.environment->snapshot().environments==before&&peer->snapshot().environments==1,"foreign release preserves both controllers and exact reference counts");
   s::IoEnvironmentConstructorScratch scratch;s::IoHandlerEnvironment cloned;f.config.factories[0].construct_slot_58(cloned,query(other),scratch);check(peer->snapshot().environments==2&&f.environment->snapshot().environments==1,"cached context-free factory resolves capability owner without ambient controller or TLS singleton");peer->release(cloned);
   bool denied=false;std::thread wrong([&]{try{(void)query(source);}catch(const std::logic_error&){denied=true;}});wrong.join();check(denied&&f.environment->snapshot().phase==oe::Phase::Bound,"foreign worker query rejected before root admission without poisoning owner");expired=query(source);f.environment->release(source);peer->release(other);release_strong(pair.control);
 });f.host.run();f.host.drain_wake();f.operation([&](o::ServerView&){check(peer->close(true),"second controller retires only after shared real source graph drains");});peer.reset();auto types=f.config.types;const auto factory=f.config.factories[0].construct_slot_58;f.finish();s::IoHandlerEnvironment fresh;s::IoEnvironmentConstructorScratch scratch;rejects<s::UnsupportedInput>([&]{factory(fresh,expired,scratch);},"cached static factory rejects expired capability after controller and parent retirement");rejects<s::UnsupportedInput>([&]{(void)types->compare_type_names_04d2d0(0x1800a25c8ULL,0x1800a25c8ULL);},"exported type provider weak ownership rejects retired controller safely");delete retained;
}
}
struct ThrowingTask final:s::OwnedHandlerTask {s::OwnedHandlerTask*& slot;oe::Environment& environment;bool close_reentry;bool& swallowed;ThrowingTask(s::OwnedHandlerTask*& s,oe::Environment& e,bool close,bool& caught):slot(s),environment(e),close_reentry(close),swallowed(caught){}void call_slot_00(std::uint8_t flag)override{check(flag==1&&!slot,"retained handler source slot is consumed before unknown callback");if(close_reentry){try{(void)environment.close(true);}catch(const std::logic_error&){swallowed=true;}return;}throw std::runtime_error("controlled unknown owned handler result");}};
[[noreturn]]void retained_child(std::string_view mode){
 auto* retained=new EnvFixture(16,mode.starts_with("tracked-")?oe::ClassifierProfile::OriginalDefault:oe::ClassifierProfile::Host);auto& f=*retained;
 f.operation([&](o::ServerView& view){auto pair=f.owner(view);s::IoHandlerEnvironment source;
   if(mode=="probe"){bool swallowed=false,snapshot_safe=false;f.admission_hook=[&]{snapshot_safe=f.environment->snapshot().active_calls==1;try{const auto type=f.config.queries[0].query_slot_00();(void)f.config.types->compare_type_names_04d2d0(type+8,0x1800a25c8ULL);}catch(const std::logic_error&){swallowed=true;}};rejects<std::logic_error>([&]{f.environment->session(source,{{0,1},*pair.object});},"admission callback swallowing Types reentry cannot recurse or publish outer row");check(swallowed&&snapshot_safe&&f.admissions==1&&f.environment->snapshot().phase==oe::Phase::Poisoned&&f.environment->snapshot().active_calls==1&&!f.environment->snapshot().environments,"checking seal rejects nested source entry and retains the exact failed admitted effect");::ExitProcess(309);}
   if(mode=="checkpoint"){bool swallowed=false;f.admission_hook=[&]{try{(void)f.environment->close(true);}catch(const std::logic_error&){swallowed=true;}};rejects<std::logic_error>([&]{f.environment->session(source,{{0,1},*pair.object});},"swallowed lifecycle reentry in admission callback cannot publish projection");check(swallowed&&f.environment->snapshot().phase==oe::Phase::Poisoned&&!f.environment->snapshot().environments&&f.environment->snapshot().active_calls==1,"checkpoint reentry retains one admitted unknown effect before any projection row is created");::ExitProcess(301);}
   if(mode=="unknown"){f.admission_hook=[]{throw std::runtime_error("controlled unknown admission result");};rejects<std::runtime_error>([&]{f.environment->session(source,{{0,1},*pair.object});},"unknown admission callback outcome propagates without guessed cleanup");check(f.environment->snapshot().phase==oe::Phase::Poisoned&&!f.environment->snapshot().environments&&f.environment->snapshot().active_calls==1&&view.memory.snapshot().provider_leases==3,"unknown callback retains its exact admitted effect, controller lease and source graph");::ExitProcess(302);}
   if(mode=="owner"){s::SessionOwner forged;forged.weak_self={forged.identity(),pair.control,&forged.input};rejects<std::invalid_argument>([&]{f.environment->session(source,{{0,1},forged});},"real parent control cannot authorize a forged embedded owner");check(f.environment->snapshot().phase==oe::Phase::Poisoned&&!f.environment->snapshot().environments,"forged owner fails before row publication");::ExitProcess(303);}
   f.environment->session(source,{{0,1},*pair.object});
   if(mode=="move-probe"){
    s::IoHandlerEnvironment destination;destination.payload_00={0xabababab,UINTPTR_MAX};destination.state_18=0xcccccccc;const auto before_source=source,before_destination=destination;const auto before=f.environment->snapshot();const auto admissions=f.admissions;bool swallowed=false,preprefix=false;
    f.admission_hook=[&]{preprefix=same_projection_fields(source,before_source)&&same_projection_fields(destination,before_destination);check(preprefix&&f.environment->snapshot().active_calls==1,"Owned whole-move callback sees both full unmodified projections under its published effect before any native header write");try{(void)query(source);}catch(const std::logic_error&){swallowed=true;}};
    rejects<std::logic_error>([&]{s::move_io_environment_023cb0(destination,source,f.config.sentinels);},"Swallowed same-actor source query during whole-move admission cannot commit the outer native prefix");
    const auto after=f.environment->snapshot();check(swallowed&&preprefix&&f.admissions==admissions+1&&after.phase==oe::Phase::Poisoned&&after.active_calls==1&&after.environments==before.environments&&after.created==before.created&&after.moved==before.moved&&after.copied==before.copied&&after.destroyed==before.destroyed&&same_projection_fields(source,before_source)&&same_projection_fields(destination,before_destination),"Failed root whole-move retains its exact admitted unknown effect and both untouched source/destination prefixes without fabricated transfer");
    rejects<std::logic_error>([&]{s::move_io_environment_023cb0(destination,source,f.config.sentinels);},"Failed admitted whole-move cannot retry its prefix");check(f.admissions==admissions+1&&f.environment->snapshot().active_calls==1&&same_projection_fields(source,before_source)&&same_projection_fields(destination,before_destination),"Rejected whole-move retry performs no callback, field mutation or second effect publication");::ExitProcess(312);
   }
   if(mode=="tracked-before"||mode=="tracked-after"){
    s::IoHandlerEnvironment tracked;s::IoEnvironmentConstructorScratch scratch;factory_binding(source,f.config).construct_slot_58(tracked,query(source),scratch);const auto work=view.scheduler.work_count_30.load();
    const bool after=mode=="tracked-after";auto unknown=[](){throw std::runtime_error("Controlled unknown tracked release outcome");};if(after)f.work.after_finish=unknown;else f.work.before_finish=unknown;
    rejects<std::runtime_error>([&]{f.environment->release(tracked);},"Unknown work-stop capability outcome retains the entered tracked release prefix");
    const auto prefix=f.environment->snapshot();check(prefix.phase==oe::Phase::Poisoned&&prefix.environments==2&&!prefix.destroyed&&prefix.active_calls==1&&f.work.entered==1&&f.work.consumed_count==(after?1U:0U)&&view.scheduler.work_count_30==work-(after?1U:0U),"Tracked release failure preserves exact pre/post-decrement count without fabricated metadata retirement or rollback");
    rejects<std::logic_error>([&]{f.environment->release(tracked);},"Poisoned entered release cannot retry the same work-stop effect");check(f.work.entered==1&&f.work.consumed_count==(after?1U:0U),"Rejected release replay never decrements an already-consumed or unknown pin");::ExitProcess(after?311:310);
   }
   if(mode=="executor"){s::IoHandlerExecutor foreign;source.executor_28=&foreign;s::IoHandlerEnvironment destination;const auto before_source=source,before_destination=destination;const auto before=f.environment->snapshot();rejects<s::UnsupportedInput>([&]{s::move_io_environment_023cb0(destination,source,f.config.sentinels);},"whole owned source move rejects a foreign executor before any native header mutation");const auto after=f.environment->snapshot();check(after.phase==oe::Phase::Poisoned&&after.environments==1&&!after.active_calls&&after.created==before.created&&after.moved==before.moved&&after.destroyed==before.destroyed&&same_projection_fields(source,before_source)&&same_projection_fields(destination,before_destination),"wrong full source executor retains the exact unmodified source/destination prefix and one owned reference");::ExitProcess(304);}
   if(mode=="address"){
    s::IoHandlerEnvironment forged=source;const auto genuine_cap=query(source);const auto original_source=source,before_forged=forged;const auto before=f.environment->snapshot();const auto admissions=f.admissions;
    rejects<s::UnsupportedInput>([&]{(void)query(forged);},"Copied capability fields at an unregistered projection address reject before source effect admission");
    const auto after=f.environment->snapshot();check(after.phase==oe::Phase::Bound&&!after.active_calls&&after.environments==1&&after.created==before.created&&after.copied==before.copied&&after.moved==before.moved&&after.destroyed==before.destroyed&&f.admissions==admissions&&same_projection_fields(source,original_source)&&same_projection_fields(forged,before_forged),"Known unregistered address leaves the healthy provider, exact genuine reference, fields and counters unchanged without a checkpoint or admitted effect");
    check(query(source)==genuine_cap&&f.environment->snapshot().phase==oe::Phase::Bound&&!f.environment->snapshot().active_calls&&f.environment->snapshot().environments==1,"Genuine registered source remains normally queryable after the known foreign-address preflight rejection");::ExitProcess(305);
   }
   bool swallowed=false;s::OwnedHandlerTask* slot=nullptr;ThrowingTask task(slot,*f.environment,mode=="handlerclose",swallowed);slot=&task;
   if(mode=="handlerclose")rejects<std::logic_error>([&]{f.config.handlers[0].table->owned_slot_10(source,slot);},"owned handler swallowing active close cannot resume scheduler admission");else rejects<std::runtime_error>([&]{f.config.handlers[0].table->owned_slot_10(source,slot);},"unknown owned handler result propagates after source slot consumption");check(!slot&&f.environment->snapshot().phase==oe::Phase::Poisoned&&f.environment->snapshot().handlers==1&&f.environment->snapshot().environments==1&&f.environment->snapshot().active_calls==1,"unknown scheduled callback retains its exact admitted effect and projection without guessed reschedule/cleanup");if(mode=="handlerclose")check(swallowed,"active close rejection was swallowed inside handler before sticky post-check");::ExitProcess(mode=="handlerclose"?307:308);
 });::ExitProcess(399);
}
void isolated(std::wstring mode,DWORD expected){std::array<wchar_t,32768> path{};const auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));check(n&&n<path.size(),"own environment fixture executable located");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --"+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};check(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"isolated mandatory-close environment fixture launched");const auto wait=::WaitForSingleObject(process.hProcess,30000);DWORD code=0;check(wait==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"isolated environment fixture reaches exact retained-prefix assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
int main(int argc,char** argv){try{if(argc==2)retained_child(std::string_view(argv[1]).substr(2));moves_and_copies();reference_ledger_growth();multi_session_projection_lineage();root_move_admission_boundary();root_copy_executor_tail_boundary();root_message_copy_caller_boundary();schedulers_and_strict_close();original_default_classifier();tracked_executor_families();validation_and_callback_lifetime();isolated(L"checkpoint",301);isolated(L"unknown",302);isolated(L"owner",303);isolated(L"executor",304);isolated(L"move-probe",312);isolated(L"address",305);isolated(L"handlerclose",307);isolated(L"handlerthrow",308);isolated(L"probe",309);isolated(L"tracked-before",310);isolated(L"tracked-after",311);std::cout<<"PASS: "<<checks<<" owned-environment core checks; actual native moves/factories/posting,4100 held metadata projections, OriginalDefault empty IO/direct completions and tracked executor transformations/work pins, capability lifetime and eleven isolated retained prefixes\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}




