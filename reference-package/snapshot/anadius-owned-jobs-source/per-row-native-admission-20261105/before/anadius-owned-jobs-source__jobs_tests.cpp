#include "jobs.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <optional>
#include <thread>

namespace oj=anadius::owned_jobs;namespace sp=anadius::session_profile;namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace r=anadius::server_runtime;namespace b=b6_core_test;
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
struct AdmissionKeeper {std::function<void()> retiring;~AdmissionKeeper(){if(retiring)retiring();}};
struct JobsFixture {
 Fixture host;std::unique_ptr<oj::InlineJobs> jobs;std::optional<r::Ports> ports;std::function<void()> admission_hook;std::shared_ptr<AdmissionKeeper> keeper=std::make_shared<AdmissionKeeper>();unsigned admissions=0;
 explicit JobsFixture(std::size_t capacity=2){host.initialize();host.submit();ports.emplace(*host.state->ports);jobs=std::make_unique<oj::InlineJobs>(capacity);operation([&](o::ServerView&){jobs->bind(*ports,[&,pin=keeper]{(void)pin;++admissions;(void)ports->view.memory.buffer_context();if(admission_hook)admission_hook();});});}
 template<class F>void operation(F fn){host.backend->with_operation([&](o::ServerView& view){fn(view);});}
 void finish(){if(host.backend->snapshot().phase==o::Phase::Ready)host.run();else check(host.backend->snapshot().phase==o::Phase::Returned,"job fixture retains actual worker return stage");host.drain_wake();operation([&](o::ServerView&){check(jobs->close(true),"inline jobs retire only after actual source worker/storage drains");});jobs.reset();ports.reset();host.close();}
};
void strong_release(s::IoReference* control){if(control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}}
struct TargetStatus {unsigned calls=0,destroyed=0;std::function<void()> call,destroy;};
struct Target {TargetStatus& status;explicit Target(TargetStatus& s):status(s){}~Target(){++status.destroyed;if(status.destroy)status.destroy();}void invoke(){++status.calls;if(status.call)status.call();}};
void tickets_and_retirement(){
 auto* retained=new JobsFixture;auto& f=*retained;TargetStatus status;s::FrameThreadHandle first{},second{};
 f.operation([&](o::ServerView& view){check(view.memory.snapshot().provider_leases==3,"bound jobs add one Parent lease to actual Base/Accept providers");auto pair=view.construct_accepted({nullptr,move_executor});status.call=[&]{const auto snap=f.jobs->snapshot();check(snap.active&&snap.pending==1&&snap.started==1&&!snap.completed,"runner executes inline while row is running BEFORE completed ticket return");check(pair.control->strong_08==1&&pair.control->weak_0c==2,"job backend adds no extra session strong or weak ownership");};status.destroy=[&]{const auto snap=f.jobs->snapshot();check(snap.active&&snap.pending==1,"final runner target retires outside gate under active before row reuse");};
   auto target=std::make_shared<Target>(status);std::function<void()> runner=[target]{target->invoke();};first=f.jobs->start({0,1},std::move(runner));runner={};target.reset();check(first.handle&&first.handle==first.id&&status.calls==1&&!status.destroyed,"completed ticket is unique synthetic identity and retains callable target until detach");const auto done=f.jobs->snapshot();check(!done.active&&done.pending==1&&done.completed==1&&done.started==1,"start returns only after exact normal completion but retains row until detach");check(f.jobs->detach(first)==0&&status.destroyed==1&&!f.jobs->snapshot().pending,"exact completed detach retires target then releases fixed row");
   second=f.jobs->start({0,2},[]{});check(second.handle!=first.handle&&second.handle==second.id,"same row with greater JobToken generation receives new globally unique synthetic ticket");check(f.jobs->detach(second)==0,"second exact ticket detaches once");strong_release(pair.control);
   ::SetLastError(0x642a);const auto snapshot=f.jobs->snapshot();check(::GetLastError()==0x642a&&snapshot.started==2&&snapshot.completed==2&&snapshot.detached==2&&!snapshot.pending&&!snapshot.active,"snapshot preserves LastError and exact source-execution counters");
 });f.finish();delete retained;
}
void global_tickets_and_drain(){
 auto* retained=new JobsFixture(1);auto& f=*retained;auto peer=std::make_unique<oj::InlineJobs>(1);s::FrameThreadHandle one{},two{};
 f.operation([&](o::ServerView& view){peer->bind(*f.ports,[&]{(void)view.memory.buffer_context();});one=f.jobs->start({0,1},[]{});two=peer->start({0,1},[]{});check(one.handle&&two.handle&&one.handle!=two.handle,"distinct controllers share monotonic synthetic ticket namespace");check(!f.jobs->close(true),"completed but undetached row prevents close despite external bool");check(f.jobs->detach(one)==0&&peer->detach(two)==0,"each controller detaches only its own completed exact ticket");check(!f.jobs->close(true)&&view.memory.snapshot().provider_leases==4,"rows0 cannot retire jobs while actual accept/source work remains");});
 f.host.run();f.host.drain_wake();f.operation([&](o::ServerView& view){check(!f.jobs->close(false),"false external drain cannot retire empty inline backend");view.scheduler.work_count_30=1;check(!f.jobs->close(true),"nonzero scheduler work blocks close after native worker returned");view.scheduler.work_count_30=0;s::CancelOperation queued;view.scheduler.completion.head=&queued;view.scheduler.completion.tail=&queued;check(!f.jobs->close(true),"completion queue blocks close independent of completed-ticket counters");view.scheduler.completion.head=nullptr;view.scheduler.completion.tail=nullptr;auto& storage=view.io.allocate_04b0bc(41);check(!f.jobs->close(true)&&view.io.snapshot().allocations==1,"real source IO allocation blocks close with rows0 and trusted attestation");view.io.free_04b788(storage);check(peer->close(true),"second controller closes after complete shared source drain");});peer.reset();f.finish();delete retained;
}
void passive_native_selection(){
 rejects<std::invalid_argument>([]{oj::NativeJobs jobs({});},"Native Jobs requires certified same-Domain NativeThreads");
 NativeDomain native;
 std::shared_ptr<anadius::bootstrap_providers::NativeThreads> threads=native.root->make_threads();
 rejects<std::invalid_argument>([&]{oj::NativeJobs jobs(threads,0);},"Native Jobs rejects zero physical row bound");
 {oj::NativeJobs jobs(threads,65);const auto snap=jobs.snapshot();check(snap.phase==oj::Phase::Fresh&&!snap.pending&&!snap.started&&!snap.active,"Initial native row allocation above64 is passive and no longer coupled to a memory-offer ceiling");}
 auto api=oj::NativeJobApi{};api.post=nullptr;rejects<std::invalid_argument>([&]{oj::NativeJobs jobs(threads,1,api);},"Native Jobs needs explicit native notification API");
 {oj::NativeJobs jobs(threads,2);bool ran=false;rejects<std::logic_error>([&]{(void)jobs.start({0,1},[&]{ran=true;});},"Native Jobs refuses legacy callable fallback before any launch");const auto snap=jobs.snapshot();check(!ran&&snap.phase==oj::Phase::Fresh&&!snap.pending&&!snap.active&&!snap.started&&!snap.completed&&!snap.detached,"Native Jobs construction/legacy rejection remains passive, without event/SDK/thread publication");}
 threads.reset();
 unsigned retired=0;oj::NativeJobs* observed=nullptr;
 auto own=native.root->make_threads();std::shared_ptr<anadius::bootstrap_providers::NativeThreads> last(own.release(),[&](auto* p){check(observed&&observed->snapshot().phase==oj::Phase::Retiring,"last native thread-provider deleter sees attached Retiring facade");rejects<std::logic_error>([&]{(void)observed->start({0,1},[]{});},"thread-provider retirement cannot execute legacy work");check(observed->snapshot().phase==oj::Phase::Retiring,"rejected retirement reentry preserves seal");++retired;delete p;});
 auto owner=std::make_unique<oj::NativeJobs>(last,1);observed=owner.get();last.reset();owner.reset();observed=nullptr;check(retired==1,"Native Jobs retires last-owned provider once before facade metadata is destroyed");
}
void validation_and_affinity(){
 rejects<std::invalid_argument>([]{oj::InlineJobs jobs(0);},"inline ticket rows reject zero capacity");rejects<std::invalid_argument>([]{oj::InlineJobs jobs(65537);},"inline ticket rows have bounded maximum");oj::InlineJobs unbound;unsigned callbacks=0;rejects<std::logic_error>([&]{(void)unbound.start({0,1},[&]{++callbacks;});},"fresh unbound backend denies execution before callback");check(!callbacks&&unbound.snapshot().phase==oj::Phase::Fresh,"unbound precondition preserves fresh controller and runner remains uninvoked");
 auto* retained=new JobsFixture;auto& f=*retained;bool denied=false;std::thread foreign([&]{try{(void)f.jobs->start({0,1},[]{});}catch(const std::logic_error&){denied=true;}});foreign.join();check(denied&&f.jobs->snapshot().phase==oj::Phase::Bound&&!f.jobs->snapshot().started,"foreign worker rejected before Root or row mutation");f.finish();delete retained;
}
struct SelectionPolicy final:sp::Policy {
 Control control;Guard guard;Log log;explicit SelectionPolicy(State& state):control(state){}
 s::DispatcherEffects& guards()override{return guard;}s::WriteFinishApi& logging()override{return log;}
 void construct_length_error(s::ReadLengthError&,std::uintptr_t)override{unused();}[[noreturn]]void throw_length_error(s::ReadLengthError&)override{unused();}[[noreturn]]void thread_error(int)override{unused();}void cleanup_exit_waits()override{}[[noreturn]]void raise_write(s::Error&,std::uintptr_t)override{unused();}
 s::IocpSchedulerEntry* completion_owner(sp::Application&,s::IocpSchedulerEntry* owner,s::CancelOperation&,const s::Error&,std::uint32_t)override{return owner;}void after_completion(sp::Application&)override{}void worker_returned(sp::Application&,std::uint64_t,const s::WorkerRunResult&)override{}bool external_quiescent(o::ServerView,sp::Application&)override{return true;}
 std::uint32_t exchange_stop(s::IocpSchedulerEntry& e,std::uint32_t n)override{return control.exchange_stop(e,n);}std::uint32_t exchange_wake(s::IocpSchedulerEntry& e,std::uint32_t n)override{return control.exchange_wake(e,n);}void initialize(std::array<std::uintptr_t,2>& value)override{control.initialize(value);}void finish(std::array<std::uintptr_t,2>& value)override{control.finish(value);}[[noreturn]]void raise(s::Error,std::uintptr_t)override{unused();}[[noreturn]]void bad_weak_ptr_026860()override{unused();}
};
struct EmptyLogic final:sp::SessionLogic {std::string greeting()override{return {};}std::string process(std::string&)override{return {};}};
struct SendOverride final:s::AsyncSendApi {s::AsyncSendResult send(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override{unused();}};
unsigned imports_called=0;
int WSAAPI receive_spy(SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){++imports_called;unused();}
int WSAAPI send_spy(SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){++imports_called;unused();}
int WSAAPI error_spy(){++imports_called;unused();}
void passive_profile_selection(){
 auto* retained=new Fixture;auto& f=*retained;f.initialize();auto dependencies=[&]{sp::Dependencies d;d.errors=f.errors;d.policy=std::make_shared<SelectionPolicy>(*f.state);d.logic=[](sp::SessionView){return std::make_unique<EmptyLogic>();};return d;};
 {sp::Profile profile(dependencies());check(profile.snapshot().phase==sp::Phase::Fresh&&!profile.snapshot().live_jobs&&!profile.snapshot().live_sessions,"empty jobs/receive/send select passive owned/native defaults without creating application");}
 {auto d=dependencies();d.receive_imports={receive_spy,error_spy};d.send_imports={send_spy,error_spy};sp::Profile profile(std::move(d));check(!imports_called,"explicit SDK import selection constructs providers without calling imports");}
 {auto d=dependencies();d.receive_imports.receive=nullptr;rejects<std::invalid_argument>([&]{sp::Profile profile(std::move(d));},"null receive import rejected only on default receive provider selection");}
 {auto d=dependencies();d.send_imports.get_error=nullptr;rejects<std::invalid_argument>([&]{sp::Profile profile(std::move(d));},"null send error import rejected only on default send provider selection");}
 {auto d=dependencies();d.receive=std::make_shared<Receive>();d.send=std::make_shared<SendOverride>();d.jobs=std::make_shared<oj::InlineJobs>(1);d.receive_imports={nullptr,nullptr};d.send_imports={nullptr,nullptr};sp::Profile profile(std::move(d));check(profile.snapshot().phase==sp::Phase::Fresh&&!imports_called,"explicit receive/send/jobs overrides preserve legacy path despite unused null SDK imports");}
 f.submit();f.run();f.close();delete retained;
}
}
struct DonorState {bool armed=false;unsigned retired=0,called=0;std::function<void()> hook;};
struct SmallDonor {std::shared_ptr<DonorState> state;explicit SmallDonor(std::shared_ptr<DonorState> s):state(std::move(s)){}SmallDonor(const SmallDonor&)noexcept=default;~SmallDonor(){if(state&&state->armed){++state->retired;if(state->hook)state->hook();}}void operator()(){++state->called;}};
[[noreturn]]void retained_child(std::string_view mode){
 auto* retained=new JobsFixture;auto& f=*retained;
 if(mode=="retirecheckpoint"){
   f.host.run();f.host.drain_wake();bool caught=false,observed=false;f.keeper->retiring=[&]{observed=f.jobs->snapshot().pending==0;try{(void)f.jobs->close(true);}catch(const std::logic_error&){caught=true;}};f.keeper.reset();f.operation([&](o::ServerView& view){rejects<std::logic_error>([&]{(void)f.jobs->close(true);},"checkpoint target destructor reentry cannot retire provider behind close gate");check(observed&&caught&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().active&&view.memory.snapshot().provider_leases==3,"unknown checkpoint retirement keeps parent lease and source metadata alive");});::ExitProcess(413);
 }
 if(mode=="probe"||mode=="unknownprobe"){
   bool swallowed=false;unsigned calls=0;f.admission_hook=[&]{if(mode=="unknownprobe")throw std::runtime_error("controlled unknown job probe");try{(void)f.jobs->start({1,1},[]{});}catch(const std::logic_error&){swallowed=true;}};f.operation([&](o::ServerView& view){rejects<std::exception>([&]{(void)f.jobs->start({0,1},[&]{++calls;});},"unknown or swallowed recursive admission cannot execute incoming source job");check((mode=="unknownprobe"||swallowed)&&!calls&&f.admissions==1&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().pending&&!f.jobs->snapshot().active&&view.memory.snapshot().provider_leases==3,"probe failure retains incoming callback/lease before row reservation and blocks recursion");});::ExitProcess(mode=="probe"?411:412);
 }
 if(mode=="donor"){
   auto state=std::make_shared<DonorState>();std::function<void()> runner=SmallDonor(state);bool caught=false;state->hook=[&]{check(f.jobs->snapshot().active,"moved-from small callable donor is cleared under active guard");try{(void)f.jobs->close(true);}catch(const std::logic_error&){caught=true;}};state->armed=true;
   f.operation([&](o::ServerView&){rejects<std::logic_error>([&]{(void)f.jobs->start({0,1},std::move(runner));},"small donor destructor swallowing lifecycle rejection cannot advance start");state->armed=false;check(caught&&state->retired>=1&&!state->called&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().started&&!f.jobs->snapshot().pending,"donor retirement stops before token allocation or invocation and retains incoming target");});::ExitProcess(414);
 }
 if(mode=="startreentry"||mode=="earlydetach"||mode=="close"||mode=="unknown"){
   unsigned calls=0,nested=0;bool caught=false;f.operation([&](o::ServerView&){rejects<std::exception>([&]{(void)f.jobs->start({0,1},[&]{++calls;if(mode=="unknown")throw std::runtime_error("controlled unknown runner prefix");try{if(mode=="startreentry")(void)f.jobs->start({1,1},[&]{++nested;});else if(mode=="earlydetach")(void)f.jobs->detach({1,1});else(void)f.jobs->close(true);}catch(const std::logic_error&){caught=true;}});},"runner unknown or swallowed execution/lifecycle reentry stops outer start");check(calls==1&&!nested&&(mode=="unknown"||caught)&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().active&&f.jobs->snapshot().pending==1&&f.jobs->snapshot().started==1&&!f.jobs->snapshot().completed&&!f.jobs->snapshot().detached,"failed runner prefix retains exact row/target and never returns completed ticket");});::ExitProcess(mode=="startreentry"?401:mode=="earlydetach"?402:mode=="close"?403:404);
 }
 TargetStatus status;auto target=std::make_shared<Target>(status);std::function<void()> runner=[target]{target->invoke();};
 if(mode=="invalidslot"){
   f.operation([&](o::ServerView&){rejects<std::invalid_argument>([&]{(void)f.jobs->start({2,1},std::move(runner));},"out-of-capacity source token rejected after incoming retention");runner={};target.reset();check(!status.calls&&!status.destroyed&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().pending&&!f.jobs->snapshot().started,"invalid token retains incoming owned target without execution or destructor cleanup");});::ExitProcess(405);
 }
 f.operation([&](o::ServerView&){const auto ticket=f.jobs->start({0,1},std::move(runner));runner={};target.reset();
   if(mode=="targetdestructor"){bool caught=false;status.destroy=[&]{check(f.jobs->snapshot().active,"row target destructor executes under active retirement gate");try{(void)f.jobs->start({1,1},[]{});}catch(const std::logic_error&){caught=true;}};rejects<std::logic_error>([&]{(void)f.jobs->detach(ticket);},"raw callable target destructor cannot reenter row reuse");check(caught&&status.calls==1&&status.destroyed==1&&f.jobs->snapshot().phase==oj::Phase::Poisoned&&f.jobs->snapshot().pending==1&&!f.jobs->snapshot().detached,"unknown callable retirement retains row with no second invocation/detach");::ExitProcess(410);}
   if(mode=="occupied"){rejects<std::invalid_argument>([&]{(void)f.jobs->start({0,2},[]{});},"completed but undetached slot cannot be reused");check(f.jobs->snapshot().phase==oj::Phase::Poisoned&&f.jobs->snapshot().pending==1&&status.calls==1&&!status.destroyed,"occupied slot failure retains completed target and incoming prefix");::ExitProcess(407);}
   if(mode=="forged"){rejects<std::invalid_argument>([&]{(void)f.jobs->detach({ticket.handle+1,ticket.id});},"forged handle/id pair cannot detach exact row");check(f.jobs->snapshot().phase==oj::Phase::Poisoned&&f.jobs->snapshot().pending==1&&!status.destroyed&&!f.jobs->snapshot().detached,"forged synthetic ticket causes no target destruction or OS handle action");::ExitProcess(408);}
   check(f.jobs->detach(ticket)==0&&status.destroyed==1,"initial exact ticket retires normally before stale misuse");if(mode=="stale"){rejects<std::invalid_argument>([&]{(void)f.jobs->start({0,1},[]{});},"reused row rejects nonincreasing generation");check(f.jobs->snapshot().phase==oj::Phase::Poisoned&&!f.jobs->snapshot().pending&&f.jobs->snapshot().started==1,"stale generation cannot publish another ticket");::ExitProcess(406);}rejects<std::invalid_argument>([&]{(void)f.jobs->detach(ticket);},"exact ticket cannot detach twice");check(f.jobs->snapshot().phase==oj::Phase::Poisoned&&f.jobs->snapshot().detached==1&&status.destroyed==1,"duplicate detach never repeats callable target retirement");::ExitProcess(409);
 });::ExitProcess(499);
}
void isolated(std::wstring mode,DWORD expected){std::array<wchar_t,32768> path{};const auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));check(n&&n<path.size(),"own inline-jobs fixture executable located");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --"+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};check(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"isolated fail-closed inline-jobs fixture launched");const auto wait=::WaitForSingleObject(process.hProcess,30000);DWORD code=0;check(wait==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"inline-jobs child reached exact retained-prefix assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
int main(int argc,char** argv){try{if(argc==2)retained_child(std::string_view(argv[1]).substr(2));tickets_and_retirement();global_tickets_and_drain();passive_native_selection();validation_and_affinity();passive_profile_selection();isolated(L"startreentry",401);isolated(L"earlydetach",402);isolated(L"close",403);isolated(L"unknown",404);isolated(L"invalidslot",405);isolated(L"stale",406);isolated(L"occupied",407);isolated(L"forged",408);isolated(L"duplicate",409);isolated(L"targetdestructor",410);isolated(L"probe",411);isolated(L"unknownprobe",412);isolated(L"retirecheckpoint",413);isolated(L"donor",414);std::cout<<"PASS: "<<checks<<" owned-jobs core checks; inline/default and passive Native Jobs source selection, fourteen isolated retained prefixes\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
