#include "resources.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>

namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace r=anadius::server_runtime;namespace b=b6_core_test;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v){std::cerr<<"FAIL: "<<why<<'\n';throw std::runtime_error(why);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected isolated runtime dependency");}
template<class E,class F>void rejects(F fn,const char* why){bool caught=false;try{fn();}catch(const E&){caught=true;}check(caught,why);}
void* WINAPI heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){auto& state=*b::observed;++state.allocations;check(heap==::GetProcessHeap()&&(flags==0||flags==HEAP_ZERO_MEMORY),"owned source heap allocation preserves zero or uninitialized policy");auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw)state.blocks.emplace(raw,bytes);::SetLastError(error);return raw;}
struct NativeDomain {
 b::Observation observation;b::RuntimeState runtime;std::shared_ptr<anadius::thread_domain::DomainStorage> root;
 NativeDomain(){if(b::observed||b::runtime_state)unused();b::observed=&observation;b::runtime_state=&runtime;auto api=b::apis();api.heap.heap_alloc=heap_allocate;root=anadius::thread_domain::DomainStorage::create(b::seed(),api);check(root->initialize(),"own controlled FLS/heap domain initialized");}
 ~NativeDomain(){if(root){if(!root->close())std::terminate();root.reset();}b::observed=nullptr;b::runtime_state=nullptr;}
};
struct Packet {std::uint32_t bytes;std::uintptr_t key;OVERLAPPED* operation;std::uint32_t error;};
struct State {
 std::uint32_t index=17;std::int32_t epoch=0;void* tls=nullptr;std::uint32_t last=0;
 unsigned environment=0,created=0,destroyed=0,bindings=0,accepts=0,factories=0,posts=0,gets=0,after=0,returned=0,external=0,init=0,finish=0,starts=0,cleanups=0,ports_closed=0;
 std::uint64_t handled=0;bool attested=false,poison_reentry=false,poison_backend=false,invalid_bindings=false,reentered=false;s::OwnedAcceptMemory* accept_memory=nullptr;
 std::optional<o::ServerView> view;r::Runtime* runtime=nullptr;std::function<void(std::string_view)> hook;std::vector<std::string> order;
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
 App(std::shared_ptr<State> st,r::Ports p,Errors& e,Table& t):state(std::move(st)),ports(p),errors(e),table(t),control(*state),receive{p.environment,p.worker,p.memory.posted(),receive_api,p.completion,e,p.memory},read{p.view.memory.buffer_context(),receive,completion,read_api,{},{}},frame{p.view.memory.buffer_context(),completion,frame_api,{},{},{},terminal,{0x1111},&read}{state->event("app.create");}
 ~App(){++state->destroyed;state->event("app.destroy");}
 r::Bindings bindings()override{++state->bindings;state->event("bindings");if(state->invalid_bindings)frame.read=nullptr;return {frame,log,completion,guard,control,{}, {},nullptr};}
 void construct_accept_environment(s::IoHandlerEnvironment& out,std::uintptr_t allocator)override{check(allocator==ports.server_identity+0x68,"actual rearm uses runtime-owned stable server identity+68");out={{1,2},&table,0,0,nullptr};state->event("construct.env");}
 void reset_pending_owner(s::AcceptPending&)override{unused();}s::ReceiveBufferString& greeting(s::SessionOwner&,s::ReceiveBufferString&)override{unused();}[[noreturn]]void raise_write(s::Error&,std::uintptr_t)override{unused();}
 s::IocpSchedulerEntry* completion_owner(s::IocpSchedulerEntry* owner,s::CancelOperation&,const s::Error& error,std::uint32_t)override{check(owner==&ports.view.scheduler&&error.value==995,"only observed terminal995 packet selects explicit owner0 policy");(void)ports.view.memory.buffer_context();state->event("owner0");return nullptr;}
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

void constructor_and_factories(){
 Fixture f;auto valid=f.runtime_dependencies();
 {r::Runtime value(valid);const auto q=value.snapshot();check(q.phase==r::Phase::Fresh&&!q.active_calls&&!q.graph_present&&!q.handled,"constructor creates no graph or callbacks");}
 auto invalid=[&](auto mutate){auto d=valid;mutate(d);rejects<std::invalid_argument>([&]{r::Runtime value(d);},"invalid dependency rejected before publication");};
 invalid([](auto& d){d.errors.reset();});invalid([](auto& d){d.sockets.reset();});invalid([](auto& d){d.dispatcher.reset();});invalid([](auto& d){d.worker.reset();});invalid([](auto& d){d.accept.reset();});invalid([](auto& d){d.factory.reset();});invalid([](auto& d){d.cancel.reset();});invalid([](auto& d){d.profile.reset();});invalid([](auto& d){d.tls_index=nullptr;});invalid([](auto& d){d.guard_epoch=nullptr;});invalid([](auto& d){d.capacity=0;});invalid([](auto& d){d.capacity=65537;});invalid([](auto& d){d.executor.move=nullptr;});
 f.state->index=TLS_OUT_OF_INDEXES;rejects<std::invalid_argument>([&]{r::Runtime value(valid);},"invalid retained TLS slot rejected");f.state->index=17;
 auto server=f.server_dependencies();
 auto bad_factory=[&](auto mutate){auto d=server;auto rd=valid;mutate(d,rd);rejects<std::invalid_argument>([&]{auto value=r::make_server(f.native.root,d,rd,4);},"factory rejects incompatible component dependencies");};
 bad_factory([](auto& d,auto&){d.errors.reset();});bad_factory([&](auto& d,auto&){d.errors=std::make_shared<Errors>(f.state);});bad_factory([&](auto& d,auto&){d.sockets=std::make_shared<Sockets>(f.state);});bad_factory([&](auto& d,auto&){d.dispatcher=std::make_shared<Dispatcher>(f.state);});bad_factory([](auto&,auto& rd){rd.capacity=5;});bad_factory([&](auto& d,auto&){d.runtime=std::make_shared<r::Runtime>(valid);});
 check(!f.state->environment&&!f.state->created&&!f.state->starts,"validation performs no profile, network or server initialization");
 {auto value=r::make_server(f.native.root,server,valid,4);check(value->snapshot().phase==o::Phase::Constructed,"factory builds owned server with concrete fresh runtime");value->destroy_context();}
 anadius::bootstrap_backend::Resources resources;
 r::install_server(resources,f.native.root,server,valid,4);auto* identity=resources.server.get();
 rejects<std::logic_error>([&]{r::install_server(resources,f.native.root,server,valid,4);},"install rejects occupied slot before publication");check(resources.server.get()==identity,"failed install preserves exact server identity");resources.server->destroy_context();resources.server.reset();
 check(!f.state->starts&&!f.state->environment,"uninitialized factory teardown does not enter SDK or Profile");
}
struct RetirementState {std::function<void()> callback;~RetirementState(){if(callback)callback();}};
void provider_retirement(){
 Fixture f;auto lifetime=std::make_shared<RetirementState>();f.initialize(true,lifetime);
 std::optional<o::ServerView> view;f.backend->with_operation([&](o::ServerView& live){view.emplace(live);});
 auto observe=[&](r::Runtime* runtime,unsigned& count){const auto state=runtime->snapshot();check(state.phase==r::Phase::Retiring&&!state.active_calls&&!state.graph_present,"final-owned dependency destructor observes live sealed Retiring metadata");const auto submits=f.state->accepts,posts=f.state->posts,gets=f.state->gets,factories=f.state->factories;rejects<std::logic_error>([&]{runtime->submit_accept(*view);},"dependency destructor lifecycle attempt denied before moved provider access");rejects<std::logic_error>([&]{(void)runtime->quiescent(*view);},"dependency destructor cannot reenter retirement lifecycle");check(runtime->snapshot().phase==r::Phase::Retiring&&!runtime->snapshot().active_calls&&submits==f.state->accepts&&posts==f.state->posts&&gets==f.state->gets&&factories==f.state->factories,"caught destructor reentry leaves Retiring and source/SDK effects unchanged");++count;};
 unsigned fresh_profile=0,fresh_provider=0,fresh_state=0;
 {
   auto d=f.runtime_dependencies();auto profile=std::make_shared<Profile>(f.state,*f.errors);auto worker=std::make_shared<Worker>(f.state);auto state=std::make_shared<RetirementState>();d.profile=profile;d.worker=worker;d.state_lifetime=state;auto fresh=std::make_unique<r::Runtime>(std::move(d));auto* identity=fresh.get();profile->on_destroy=[&]{observe(identity,fresh_profile);};worker->on_destroy=[&]{observe(identity,fresh_provider);};state->callback=[&]{observe(identity,fresh_state);};profile.reset();worker.reset();state.reset();fresh.reset();
 }
 check(fresh_profile==1&&fresh_provider==1&&fresh_state==1&&!f.state->environment,"fresh runtime retires last-owned Profile/SDK/state once without binding graph");
 f.submit();f.run();f.close();
 unsigned closed_profile=0,closed_provider=0,closed_state=0;auto* identity=f.runtime.get();f.profile->on_destroy=[&]{observe(identity,closed_profile);};f.worker->on_destroy=[&]{observe(identity,closed_provider);};lifetime->callback=[&]{observe(identity,closed_state);};f.profile.reset();f.worker.reset();lifetime.reset();
 // The explicit test adapter borrows Runtime, so the real closed Backend and
 // every ServerView referent remain alive while its final owners retire.
 f.runtime.reset();
 check(closed_profile==1&&closed_provider==1&&closed_state==1&&f.backend->snapshot().phase==o::Phase::Closed,"closed runtime retires last-owned Profile/SDK/state with live valid server referents");
}
void lifecycle_and_foreign_views(){
 auto* retained=new Fixture;auto& f=*retained;f.initialize();check(f.runtime->snapshot().phase==r::Phase::Fresh&&!f.state->environment,"backend initialization leaves runtime graph lazy");
 f.submit();check(f.runtime->snapshot().phase==r::Phase::Ready&&f.runtime->snapshot().graph_present&&!f.runtime->snapshot().active_calls,"submit publishes stable ready graph");
 check(f.state->environment==1&&f.state->created==1&&f.state->bindings==1&&f.state->accepts==1,"exactly one environment/application and source accept submission");
 check(f.state->view->memory.snapshot().provider_leases==2&&f.state->view->memory.snapshot().buffer_borrows==1,"accept and BaseIO leases retain one physical pending borrower");
 f.backend->with_operation([&](o::ServerView& view){
   auto wrong=view;wrong.identity.value^=1;rejects<std::invalid_argument>([&]{f.runtime->submit_accept(wrong);},"foreign identity rejected before pending or source access");
   s::SocketService service{view.service.gate};auto service_view=o::ServerView{view.backend,view.identity,service,view.acceptor,view.pending,view.scheduler,view.memory,view.io};rejects<std::invalid_argument>([&]{f.runtime->submit_accept(service_view);},"foreign service pointer rejected");
   s::AcceptorState socket_state;s::SocketObject socket{socket_state};auto acceptor_view=o::ServerView{view.backend,view.identity,view.service,socket,view.pending,view.scheduler,view.memory,view.io};rejects<std::invalid_argument>([&]{f.runtime->submit_accept(acceptor_view);},"foreign listener pointer rejected");
   auto pending_view=o::ServerView{view.backend,view.identity,view.service,view.acceptor,socket,view.scheduler,view.memory,view.io};rejects<std::invalid_argument>([&]{f.runtime->submit_accept(pending_view);},"foreign pending pointer rejected");
   struct Scheduler:s::IocpSchedulerEntry{using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unused();}}scheduler{view.service.gate,view.scheduler.section};auto scheduler_view=o::ServerView{view.backend,view.identity,view.service,view.acceptor,view.pending,scheduler,view.memory,view.io};rejects<std::invalid_argument>([&]{f.runtime->submit_accept(scheduler_view);},"foreign scheduler pointer rejected");
 });
 check(f.runtime->snapshot().phase==r::Phase::Ready&&f.state->accepts==1,"foreign rejections preserve ready graph without resubmission");
 bool thread_rejected=false;std::thread foreign([&]{try{f.runtime->submit_accept(*f.state->view);}catch(const std::logic_error&){thread_rejected=true;}});foreign.join();check(thread_rejected&&f.runtime->snapshot().phase==r::Phase::Ready,"foreign thread rejected before parent access without poisoning");
 f.run();f.drain_wake();
 check(f.runtime->snapshot().phase==r::Phase::Returned&&f.runtime->snapshot().handled==1&&!f.runtime->snapshot().active_calls,"actual source worker returns one observed completion");
 check(f.state->gets==2&&f.state->after==1&&f.state->returned==1&&f.state->init==1&&f.state->finish==1,"worker routes exact completion and lower lifecycle once");
 check(f.state->closed.size()==1&&f.state->closed[0]==201&&f.state->view->io.snapshot().pending==0,"owner0 completion retires exact accepted socket and physical borrower");
 check(f.state->view->memory.snapshot().controls==0,"owner0 does not fabricate accepted SessionControl");
 rejects<std::logic_error>([&]{f.runtime->submit_accept(*f.state->view);},"returned runtime cannot submit after worker lifetime");
 auto wrong=*f.state->view;wrong.identity.value^=1;rejects<std::invalid_argument>([&]{(void)f.runtime->quiescent(wrong);},"foreign retirement tuple rejected");
 check(!f.runtime->quiescent(*f.state->view)&&f.runtime->snapshot().phase==r::Phase::Quiescing&&f.runtime->snapshot().graph_present&&!f.state->destroyed,"false external attestation preserves all graph contexts");
 f.state->attested=true;
 f.state->view->scheduler.work_count_30=1;check(!f.runtime->quiescent(*f.state->view),"attestation cannot override nonzero source work count");f.state->view->scheduler.work_count_30=0;
 s::CancelOperation queued;f.state->view->scheduler.completion.head=&queued;f.state->view->scheduler.completion.tail=&queued;check(!f.runtime->quiescent(*f.state->view),"attestation cannot override source completion queue");f.state->view->scheduler.completion.head=nullptr;f.state->view->scheduler.completion.tail=nullptr;
 auto lease=f.state->view->memory.acquire_provider_lease();check(!f.runtime->quiescent(*f.state->view),"third provider lease blocks graph retirement");lease.reset();
 s::OperationStorage* allocation=nullptr;f.backend->with_operation([&](o::ServerView& view){allocation=&view.io.allocate_04b0bc(41);});check(!f.runtime->quiescent(*f.state->view),"unretired raw allocation blocks graph destruction");f.backend->with_operation([&](o::ServerView& view){view.io.free_04b788(*allocation);});
 std::uintptr_t bytes=0;f.backend->with_operation([&](o::ServerView& view){bytes=view.memory.buffer_context().api.allocate_04b0bc(32);});check(f.state->view->io.snapshot().allocations==0&&f.state->view->memory.snapshot().buffers==1&&!f.runtime->quiescent(*f.state->view),"whole parent byte allocation blocks retirement even with empty BaseIO");f.backend->with_operation([&](o::ServerView& view){view.memory.buffer_context().api.free_sized_04b0f8(bytes,32);});
 check(!f.state->destroyed&&f.runtime->snapshot().graph_present,"all false drain cases retain application identity");
 check(f.runtime->quiescent(*f.state->view)&&f.state->destroyed==1,"source zero checks permit explicit graph and application retirement");
 check(f.runtime->quiescent(*f.state->view)&&f.state->destroyed==1,"closed runtime retirement is idempotent");
 f.close();check(f.state->cleanups==1&&f.state->ports_closed==1&&f.state->closed.back()==200,"backend teardown follows runtime graph retirement and closes listener/context once");delete retained;
}
[[noreturn]]void poison_child(std::string_view kind){
 auto* retained=new Fixture;auto& f=*retained;f.initialize();
 if(kind=="backend"){
   f.state->poison_backend=true;bool rejected=false;try{f.submit();}catch(const std::logic_error&){rejected=true;}
   check(rejected&&f.state->reentered&&f.backend->snapshot().phase==o::Phase::Poisoned&&f.runtime->snapshot().phase==r::Phase::Poisoned,"swallowed backend-only reentry poisons runtime binding immediately after Profile.create");
   check(f.state->view->memory.snapshot().phase==s::SessionMemoryPhase::Open&&f.state->view->io.snapshot().phase==s::IoMemoryPhase::Open&&f.state->accept_memory&&!f.state->accept_memory->snapshot().poisoned,"backend-only poison regression leaves Parent/Base/Own providers independently open");
   check(!f.state->accepts&&!f.state->factories&&!f.state->posts&&!f.state->gets&&!f.state->bindings&&!f.state->view->scheduler.work_count_30.load(),"poisoned backend stops before initial factory/SDK/source work or binding continuation");
   check(f.runtime->snapshot().graph_present&&!f.runtime->snapshot().active_calls&&f.state->created==1&&!f.state->destroyed,"binding poison retains returned application and all graph ownership");::ExitProcess(246);
 }
 if(kind=="quiescent"){
   f.submit();f.run();f.drain_wake();f.state->attested=false;f.state->hook=[&](std::string_view event){if(event=="external")f.state->index=18;};
   rejects<std::logic_error>([&]{(void)f.runtime->quiescent(*f.state->view);},"false external attestation cannot hide changed TLS binding");
   check(f.runtime->snapshot().phase==r::Phase::Poisoned&&f.runtime->snapshot().graph_present&&!f.state->destroyed&&f.state->view->memory.snapshot().provider_leases==2,"mutating false attestation retains graph and both source leases");::ExitProcess(245);
 }
 if(kind=="tls"||kind=="control"){
   unsigned before_gets=0,before_posts=0;
   f.submit();if(kind=="tls")f.state->index=18;
   else f.state->hook=[&](std::string_view event){if(event=="control.exchange"){before_gets=f.state->gets;before_posts=f.state->posts;try{f.runtime->submit_accept(*f.state->view);}catch(const std::logic_error&){f.state->reentered=true;}}};
   bool rejected=false;try{f.run();}catch(const std::logic_error&){rejected=true;}
   check(rejected&&f.runtime->snapshot().phase==r::Phase::Poisoned&&f.runtime->snapshot().graph_present&&!f.runtime->snapshot().active_calls,"run callback or changed TLS yields sticky poison and retained graph");
   check(f.state->gets==before_gets&&f.state->posts==before_posts&&!f.state->returned&&!f.state->destroyed&&f.state->accepts==1,"run poison stops before later SDK waits/posts or invented worker return/cleanup");
   if(kind=="control")check(f.state->reentered,"caught WorkerControl reentry cannot bypass post-policy checkpoint");
   else{f.state->index=17;rejects<std::logic_error>([&]{f.runtime->submit_accept(*f.state->view);},"restoring TLS value cannot restore poisoned run state");}
   ::ExitProcess(kind=="tls"?243:244);
 }
 f.state->poison_reentry=kind=="reentry";f.state->invalid_bindings=kind=="bindings";
 bool rejected=false;try{f.submit();}catch(const std::exception&){rejected=true;}
 check(rejected&&f.runtime->snapshot().phase==r::Phase::Poisoned&&f.runtime->snapshot().graph_present&&!f.runtime->snapshot().active_calls,"failed publication retains poisoned graph without guessed rollback");
 check(f.state->accepts==0&&f.state->destroyed==0,"poisoned graph never submits SDK or destroys borrowed application");
 if(kind=="reentry")check(f.state->reentered&&f.state->created==0,"swallowed profile reentry remains sticky and stops binding");
 else check(f.state->created==1&&f.state->bindings==1,"invalid coherent bindings rejected before ready publication");
 ::ExitProcess(kind=="reentry"?241:242);
}
void subprocess(std::wstring mode,DWORD expected){
 std::array<wchar_t,32768> path{};auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));check(n&&n<path.size(),"own controlled fixture executable located");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --"+mode;
 STARTUPINFOW startup_info{};startup_info.cb=sizeof(startup_info);PROCESS_INFORMATION process{};
 check(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup_info,&process)!=0,"isolated irreversible fixture child started");
 const auto wait=::WaitForSingleObject(process.hProcess,30000);DWORD code=0;check(wait==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"controlled poison child reached exact assertions and retained unknown state");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);
}
}
int main(int argc,char** argv){try{if(argc==2){const std::string_view arg=argv[1];if(arg=="--reentry")poison_child("reentry");if(arg=="--bindings")poison_child("bindings");if(arg=="--tls")poison_child("tls");if(arg=="--control")poison_child("control");if(arg=="--quiescent")poison_child("quiescent");if(arg=="--backend")poison_child("backend");throw std::invalid_argument("Unknown controlled mode");}constructor_and_factories();provider_retirement();lifecycle_and_foreign_views();subprocess(L"reentry",241);subprocess(L"bindings",242);subprocess(L"tls",243);subprocess(L"control",244);subprocess(L"quiescent",245);subprocess(L"backend",246);std::cout<<"B39 runtime core PASS "<<checks<<" checks; real source worker, synthetic SDK, six isolated poison children\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}







