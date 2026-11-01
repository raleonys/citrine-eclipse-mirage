#include "owned.hpp"
#include "../anadius-owned-server-source/server.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include "../anadius-socket-source/worker_run_native.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

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
struct State {
 bool ipv6=false,accept_pending=false,job_done=false,worker_done=false,queue_drained=false,session_deleted=false,accept_packets_drained=false,swallow_quiescent_poison=false;
 std::uint8_t* ready=nullptr;o::ServerView* view=nullptr;
 s::OwnedAcceptMemory* accept_memory=nullptr;s::SessionMemoryDomain* parent_memory=nullptr;
 unsigned sdk_accepts=0,address_reads=0,context_updates=0,pending_resets=0,holder_resets=0,accept_callbacks=0,cancel_callbacks=0;
 std::vector<OVERLAPPED*> accept_packets;std::vector<std::uintptr_t> accept_raw;std::vector<std::size_t> accept_extents;
 unsigned starts=0,cleanups=0,gate_inits=0,gate_deletes=0,ports_created=0,ports_closed=0,submits=0,moves=0,gets=0,posts=0,wait_scopes=0,quiescent_calls=0,executor_cleanups=0,cancellations=0,functor_calls=0,cache_frees=0;
 std::vector<int> families;std::vector<s::Socket> closed;std::vector<std::string> order;
 std::string environment_key,environment_value;std::vector<std::byte> bound;std::uintptr_t port=0;s::SocketObject* pending_identity=nullptr;unsigned swallowed=0;
};
State* running=nullptr;
int WSAAPI startup(WORD version,LPWSADATA data){auto& st=*running;require(version==MAKEWORD(2,2),"Owned context requests WSA2.2");++st.starts;st.order.push_back("startup");return ::WSAStartup(version,data);}
int WSAAPI cleanup(){auto& st=*running;++st.cleanups;st.order.push_back("cleanup");return ::WSACleanup();}
int WSAAPI local_name(SOCKET socket,sockaddr* output,int* bytes){
 require(socket==200&&*bytes>=28,"Local endpoint uses actual owned acceptor token and initialized SDK buffer");auto& st=*running;std::memset(output,0,*bytes);
 output->sa_family=st.ipv6?23:2;reinterpret_cast<unsigned char*>(output)[2]=0x30;reinterpret_cast<unsigned char*>(output)[3]=0x39;*bytes=28;st.order.push_back("local");return 0;
}
BOOL WINAPI environment(LPCSTR key,LPCSTR value){auto& st=*running;require(st.ready&&!*st.ready,"Bound port environment publication precedes source readiness flag");st.environment_key=key;st.environment_value=value;st.order.push_back("environment");return FALSE;}
BOOL WINAPI gate_initialize(LPCRITICAL_SECTION section,DWORD spin){++running->gate_inits;return ::InitializeCriticalSectionAndSpinCount(section,spin);}
VOID WINAPI gate_delete(LPCRITICAL_SECTION section){++running->gate_deletes;running->order.push_back("delete-gate");::DeleteCriticalSection(section);}
HANDLE WINAPI create_port(HANDLE file,HANDLE previous,ULONG_PTR key,DWORD threads){
 require(file==INVALID_HANDLE_VALUE&&!previous&&!key&&threads==0xffffffffU,"Actual024990 forwards observed selected IOCP construction arguments");
 auto* port=::CreateIoCompletionPort(file,previous,key,threads);if(port){++running->ports_created;running->port=reinterpret_cast<std::uintptr_t>(port);}return port;
}
BOOL WINAPI close_port(HANDLE port){require(reinterpret_cast<std::uintptr_t>(port)==running->port,"Owned teardown closes the same actual kernel IOCP handle");++running->ports_closed;running->order.push_back("close-port");return ::CloseHandle(port);}
// Native SDK adapter is exercised with injected MSWSOCK calls only. These
// sockets are tokens; no network socket or AcceptEx kernel request is created.
BOOL PASCAL accept_sdk(SOCKET listener,SOCKET accepted,PVOID buffer,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED overlapped){
 auto& st=*running;require(listener==200&&accepted==200+st.sdk_accepts+1&&receive==0&&local==0x90&&remote==0x90,"Native AcceptEx adapter forwards exact listener/fresh socket/zero-data and90+90 endpoint lengths");
 require(st.accept_memory&&st.parent_memory&&st.accept_memory->snapshot().pending==1&&st.parent_memory->snapshot().controls==(st.sdk_accepts?1U:0U)&&st.parent_memory->snapshot().buffer_borrows==1,"Accept SDK borrower pins raw storage without fabricating a SessionControl for its pending accept");
 auto bindings=st.accept_memory->dispatcher_bindings();auto storage=st.accept_memory->storage_bindings();std::size_t slot=bindings.size();
 for(std::size_t i=0;i<bindings.size();++i)if(bindings[i].overlapped_identity==overlapped)slot=i;
 require(slot<bindings.size()&&storage[slot].storage&&storage[slot].storage->bytes.size()==473,"Distinct typed accept OVERLAPPED has fixed base binding and exact473-byte native payload+marker allocation");
 const auto raw=storage[slot].storage->identity;
 require(reinterpret_cast<std::uintptr_t>(buffer)==raw+0x68&&std::to_integer<unsigned char>(storage[slot].storage->bytes[472])==118&&reinterpret_cast<std::uintptr_t>(overlapped)!=raw,"Accept address buffer is owned raw+68/288; marker118 is separate from actual typed OVERLAPPED");
 st.accept_packets.push_back(overlapped);st.accept_raw.push_back(raw);st.accept_extents.push_back(473);++st.sdk_accepts;
 *bytes=0x4567;::WSASetLastError(st.ipv6&&st.sdk_accepts==1?1234:997);return st.ipv6&&st.sdk_accepts==1?TRUE:FALSE;
}
VOID PASCAL accept_addresses(PVOID buffer,DWORD received,DWORD local_size,DWORD remote_size,sockaddr** local,int* local_bytes,sockaddr** remote,int* remote_bytes){
 auto& st=*running;require(received==0&&local_size==0x90&&remote_size==0x90&&st.accept_memory->snapshot().pending==0&&st.parent_memory->snapshot().buffer_borrows==0,"Observed completion releases accept loan before native endpoint address extraction");
 auto* bytes=static_cast<std::byte*>(buffer);std::fill_n(bytes,288,std::byte{});auto* address=bytes+144;
 address[0]=static_cast<std::byte>(st.ipv6?23:2);address[2]=std::byte{0x12};address[3]=std::byte{0x34};address[st.ipv6?23:7]=std::byte{0x7f};
 *local=reinterpret_cast<sockaddr*>(bytes);*local_bytes=st.ipv6?28:16;*remote=reinterpret_cast<sockaddr*>(address);*remote_bytes=*local_bytes;++st.address_reads;
}
int WSAAPI update_context(SOCKET socket,int level,int option,const char* value,int length){
 auto& st=*running;s::Socket listener=0;std::memcpy(&listener,value,sizeof(listener));
 require(socket==201&&level==0xffff&&option==0x700b&&length==8&&listener==200,"Native0271a0 updates accepted context with captured listener pointer value and exact option700b/size8");++st.context_updates;return 0;
}
struct Errors:s::Api {
 State& state;explicit Errors(State& s):state(s){}
 void set_last_error(std::int32_t n)override{::SetLastError(n);}std::int32_t get_last_error()override{return static_cast<std::int32_t>(::GetLastError());}
 s::Category system_category_028620()override{return {0x2222,server::ErrorCategory::system};}
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
 s::Socket create_socket(std::int32_t family)override{require(family==(state.ipv6?23:2),"Actual020e20 opens selected family through synthetic socket SDK boundary");state.order.push_back("socket");return 200;}
 std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require((socket==200||socket==201)&&port==state.port,"Actual acceptor/accepted transfer attaches selected socket to same owned kernel completion port");return port;}
 std::uint32_t get_win32_error()override{return ::GetLastError();}
 std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t option,std::int32_t value)override{
  if(level==41){require(state.ipv6&&socket==200&&option==27&&value==0,"Actual IPv6 acceptor clears IPV6_V6ONLY before IOCP attachment");return 0;}
  require(socket==200&&level==65535&&option==4&&value==1,"Acceptor enables source reuse-address option");return 0;
 }
 std::int32_t bind_socket(s::Socket socket,std::span<const std::byte> address,std::int32_t size)override{require(socket==200&&size==(state.ipv6?28:16),"Actual bind lower receives family-specific initialized sockaddr length");state.bound.assign(address.begin(),address.begin()+size);return 0;}
 std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{require(socket==200&&backlog==INT_MAX,"Actual listen uses observed INT_MAX backlog");return 0;}
 std::int32_t close_socket(s::Socket socket)override{state.closed.push_back(socket);state.order.push_back("socket-close");return 0;}
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
 std::uint32_t index;s::DispatcherApi& admitted;s::NativeWorkerRunApi native;
 WorkerApi(std::uint32_t i,s::DispatcherApi& a):index(i),admitted(a){}
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
 o::ServerView& view;State& state;s::OwnedAcceptMemory& accept;unsigned init=0,finish=0;
 Worker(o::ServerView& v,State& s,s::DispatcherState& dispatcher,s::DispatcherEnvironment environment,o::AdmittedDispatcherApi& api,s::DispatcherEffects& effects,WorkerApi& worker,Errors& errors,s::OwnedAcceptMemory& a):RecoveredDispatcherWorkerEffects(dispatcher,environment,api,effects,worker,errors),view(v),state(s),accept(a){}
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry& entry,std::uint32_t value)override{return std::atomic_ref(entry.high_34).exchange(value);}
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry& entry,std::uint32_t value)override{const auto old=static_cast<std::uint32_t>(entry.word_38);entry.word_38=(entry.word_38&0xffffffff00000000ULL)|value;return old;} // selected serialized worker
 void initialize_04a240(std::array<std::uintptr_t,2>& value)override{value.fill(0);++init;}
 void finish_04a2cc(std::array<std::uintptr_t,2>& value)override{require(value==std::array<std::uintptr_t,2>{},"Actual worker finishes unchanged explicit exception context");++finish;}
 void free_cache_04b788(std::uintptr_t identity)override{accept.free_cached_identity(identity);++state.cache_frees;}
 void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override{unused();}[[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unused();}
};
struct SessionJob final:s::PostedFunctor {
 State& state;s::SessionPair owned;explicit SessionJob(State& s,s::SessionPair pair):state(s),owned(pair){}
 void call_slot_00(std::uint8_t flag)override{
  require(flag==1&&owned.object&&owned.control&&owned.control->strong_08==1,"Actual posted native completion invokes job owning sole accepted factory strong");
  ++state.functor_calls;auto* residual=owned.control;owned={};strong_release(residual);state.job_done=true;
 }
};
struct CompletionProxy:s::DispatcherApi {
 s::DispatcherApi& lower;o::AdmittedDispatcherApi* admitted=nullptr;explicit CompletionProxy(s::DispatcherApi& d):lower(d){}
 s::DispatcherApi& target(){return admitted?static_cast<s::DispatcherApi&>(*admitted):lower;}
 void set_last_error(std::uint32_t n)override{target().set_last_error(n);}std::uint32_t get_last_error()override{return target().get_last_error();}
 std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t)override{unused();}
 std::int32_t post(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* o)override{return target().post(p,n,k,o);}
 std::int32_t set_timer(std::uintptr_t p,LARGE_INTEGER& d,std::int32_t n,PTIMERAPCROUTINE f,void* a,std::int32_t r)override{return target().set_timer(p,d,n,f,a,r);}
};
struct Table:s::IoHandlerVtable {
 Table():IoHandlerVtable(0x180017a30ULL){}
 void destroy_slot_00(s::IoHandlerEnvironment&)override{}
 void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{to.payload_00=from.payload_00;to.state_18=from.state_18;}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}
};
struct Runtime:o::Runtime,s::AcceptLower {
 State& state;Errors& errors;Sockets& sockets;CompletionProxy completion;std::uint32_t index;WorkerApi api;Table table;
 s::NativeAsyncAcceptApi sdk{s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError}};
 std::unique_ptr<s::OwnedAcceptMemory> memory;std::unique_ptr<s::AsyncAcceptContext> context;std::unique_ptr<s::AcceptPending> pending;
 s::AsyncAcceptService service;s::AcceptEndpoint endpoints[2];const std::uintptr_t raw_this=reinterpret_cast<std::uintptr_t>(this);
 s::IoReference* observer=nullptr;std::unique_ptr<SessionJob> job;s::OwnedNoopControlFactory control_factory;
 Runtime(State& s,Errors& e,Sockets& sockets,s::DispatcherApi& dispatcher):state(s),errors(e),sockets(sockets),completion(dispatcher),index(::TlsAlloc()),api(index,completion){if(index==TLS_OUT_OF_INDEXES)unused();}
 ~Runtime(){if(!::TlsFree(index))std::terminate();}
 static void move(void* p,s::ExecutorFields& destination,s::ExecutorFields& source,std::uintptr_t table){auto& state=*static_cast<State*>(p);require(table==0x180084928ULL,"Actual accepted factory calls selected executor slot8 lower");++state.moves;destination.context=source.context;destination.zero_74=source.zero_74;destination.self_reference_80=true;}
 void construct_environment(s::IoHandlerEnvironment& out,std::uintptr_t allocator)override{require(allocator==raw_this,"Actual021b00 passes selected allocator identity to023dd0 lower before publication");out={{1,2},&table,0,0,nullptr};}
 s::Socket create_socket(std::uint32_t family,std::int32_t type,std::int32_t protocol,s::Error& error)override{require(family==(state.ipv6?23U:2U)&&type==1&&protocol==6&&error.value==0,"Native accept create lower gets live protocol low32, SOCK_STREAM1/TCP6 and initialized generic error");return 201+state.sdk_accepts;}
 void reset_owned_socket(s::Socket& holder)override{s::close_025310(holder,errors,sockets);holder=s::invalid_socket;++state.holder_resets;}
 void reset_pending_owner(s::AcceptPending& value)override{s::replace_owner_0237f0(value.object.cancellation,0,control_factory);++state.pending_resets;}
 s::Category misc_category()override{return {0x3333,server::ErrorCategory::asio_misc};}s::Category success_category()override{return {0x1111,server::ErrorCategory::system};}
 void retry(s::AsyncAcceptOperation&,s::CancelOperation&)override{unused();} //29660 remains mandatory, no invented algorithm
 void invoke(s::AcceptHandler& handler)override{
  require(state.view&&handler.server_this==raw_this,"Actual0271a0 invokes copied raw server pointer, not a pending-session IoReference");++state.accept_callbacks;state.accept_pending=false;
  if(handler.error.value){require(handler.error.value==995,"Controlled final cancelled accept preserves observed995 error");++state.cancel_callbacks;return;}
  auto& view=*state.view;require(view.pending.state.socket==201&&view.pending.state.socket_state==0x10&&view.pending.state.protocol_40==(state.ipv6?23:2)&&view.pending.state.byte_44==1&&state.pending_resets==1,"Actual0271a0 transfers fresh socket/protocol and initialized endpoint flag into live pending implementation");
  require(endpoints[0].bytes[0]==static_cast<std::byte>(state.ipv6?23:2)&&endpoints[0].bytes[2]==std::byte{0x12}&&endpoints[0].bytes[3]==std::byte{0x34},"Optional endpoint output preserves actual SDK-extracted remote bytes before factory moves pending");
  auto pair=view.construct_accepted({&state,move});observer=pair.control;observer->weak_0c.fetch_add(1);
  require(pair.object->socket_state.socket==201&&observer->strong_08==1&&observer->weak_0c==3&&view.pending.state.socket==s::invalid_socket,"Completion-created actual023000 SessionControl appears only after successful accepted socket transfer");
  s::resize_receive_buffer_010040(pair.object->input,6000,'x',view.memory.buffer_context());job=std::make_unique<SessionJob>(state,pair);s::PostedFunctor* slot=job.get();
  s::PostedExecutor executor{view.identity.value,&view.scheduler,1};s::dispatch_posted_022e40(executor,slot,context->environment.storage,api,completion,*memory);
  bool reused=false;for(const auto& b:memory->storage_bindings())if(b.storage&&b.identity==state.accept_raw[0])reused=b.storage->bytes.size()==473;
  require(!slot&&reused&&memory->snapshot().projections==0,"Reentrant posted session job reuses original473-byte accept cache extent after retiring old typed accept row");
  view.rearm_accept();require(state.submits==2&&state.accept_pending&&state.sdk_accepts==2,"Existing admitted callback rearms a distinct real native accept operation without opening nested ROOT scope");
  const auto handle=memory->handle_for(state.accept_packets[1]);memory->note_cancel_requested(handle);
  bool denied=false;try{view.memory.free_sized_04b0f8(state.accept_raw[1],473);}catch(const std::logic_error&){denied=true;}
  require(denied&&memory->snapshot().pending==1&&view.memory.snapshot().buffer_borrows==1,"Cancellation intent cannot release entire473-byte kernel buffer pin before observed terminal packet");
  for(auto& binding:memory->dispatcher_bindings())if(binding.overlapped_identity==state.accept_packets[1])s::publish_send_status_028b50(view.scheduler,*binding.operation,binding,995,0,completion,errors);
 }
 void submit_accept(o::ServerView& view)override{
  require(state.ready&&*state.ready==1&&view.acceptor.state.socket==200&&!state.accept_pending,"Source readiness and distinct pending obligation precede every native accept submission");
  if(!memory){
   memory=std::make_unique<s::OwnedAcceptMemory>(view.memory.shared_from_this(),view.io,16,*this,sdk);pending=std::make_unique<s::AcceptPending>(s::AcceptPending{view.pending,&view.scheduler});service={&view.scheduler,view.identity.value+0x28};
   s::IoCompletionEnvironment environment{{index,memory->storage_bindings()},{&table,0,nullptr,table.destroy_identity}};
   context=std::make_unique<s::AsyncAcceptContext>(s::AsyncAcceptContext{environment,api,*memory,memory->pinned_api(),completion,errors,sockets,*memory,{}});memory->bind_context(*context);
   state.accept_memory=memory.get();state.parent_memory=&view.memory;state.pending_identity=&view.pending;
   // A mutable typed projection is not authority for SDK memory. Exercise the
   // real provider's range guard before any accept is submitted to the SDK.
   auto& raw=memory->allocate_04b0bc(473);auto& projection=memory->project(raw);
   const auto canonical=projection.address_buffer;const auto base_slot=view.io.handle_for(raw).slot;
   const auto handle=memory->handle_for(&projection.operation.overlapped);auto* old_operation=&projection.operation;
   std::array<std::byte,288> foreign{};projection.address_buffer=foreign;bool changed_projection_rejected=false;
   try{(void)memory->pinned_api().accept(200,201,foreign,projection.operation.overlapped);}catch(const s::UnsupportedInput&){changed_projection_rejected=true;}
   require(changed_projection_rejected&&state.sdk_accepts==0&&view.memory.snapshot().buffer_borrows==0&&!memory->snapshot().poisoned,"Mutable foreign accept span cannot authorize SDK access or create a kernel pin/unknown-outcome poison");
   projection.address_buffer=canonical;bool mismatched_argument_rejected=false;
   try{(void)memory->pinned_api().accept(200,201,foreign,projection.operation.overlapped);}catch(const s::UnsupportedInput&){mismatched_argument_rejected=true;}
   require(mismatched_argument_rejected&&state.sdk_accepts==0&&view.memory.snapshot().buffer_borrows==0&&!memory->snapshot().poisoned,"Foreign SDK span is rejected even while operation retains its authoritative raw+68/288 region");
   auto& sibling_raw=memory->allocate_04b0bc(473);auto& sibling=memory->project(sibling_raw);
   projection.address_buffer=sibling.address_buffer;bool cross_row_completion_rejected=false;
   try{memory->complete(handle,1,{0,success_category()});}catch(const s::UnsupportedInput&){cross_row_completion_rejected=true;}
   require(cross_row_completion_rejected&&state.address_reads==0&&state.sdk_accepts==0&&state.accept_callbacks==0&&memory->snapshot().projections==2&&view.io.snapshot().allocations==2&&!memory->snapshot().pending&&!memory->snapshot().poisoned&&view.memory.snapshot().buffer_borrows==0,"Sibling live canonical span cannot authorize another accept row completion or reach SDK/handler/loan retirement");
   bool cross_row_submission_rejected=false;
   try{(void)memory->pinned_api().accept(200,201,sibling.address_buffer,projection.operation.overlapped);}catch(const s::UnsupportedInput&){cross_row_submission_rejected=true;}
   require(cross_row_submission_rejected&&state.sdk_accepts==0&&state.accept_callbacks==0&&view.io.snapshot().allocations==2&&!memory->snapshot().poisoned&&view.memory.snapshot().buffer_borrows==0,"SDK submission rejects a sibling owned region as authority for the wrong accept row without poisoning either allocation");
   projection.address_buffer=canonical;memory->free_04b788(sibling_raw);
   memory->free_04b788(raw);const auto binding=memory->dispatcher_bindings()[base_slot];
   require(!binding.operation&&!binding.overlapped_identity&&!memory->contains(old_operation)&&memory->snapshot().projections==0&&view.io.snapshot().allocations==0,"Retired typed accept leaves no base packet binding to the ended variant and frees its real473-byte allocation");
   bool stale_rejected=false;try{memory->complete(handle,1,{});}catch(const std::invalid_argument&){stale_rejected=true;}
   require(stale_rejected&&state.sdk_accepts==0&&!memory->snapshot().pending&&!memory->snapshot().poisoned,"Old accept generation cannot complete or invoke native/SDK work after explicit unsubmitted storage retirement");
  }
  state.accept_pending=true;++state.submits;memory->start(service,view.acceptor.state,*pending,&endpoints[state.submits-1],raw_this,raw_this);
 }
 server::Error run(o::ServerView& view,o::AdmittedDispatcherApi& admitted)override{
  state.view=&view;completion.admitted=&admitted;require(state.submits==1&&state.sdk_accepts==1&&memory->snapshot().pending==1&&view.memory.snapshot().controls==0,"Actual accept is pending without factory-created session before observed kernel completion");
  const auto handle=memory->handle_for(state.accept_packets[0]);memory->note_cancel_requested(handle);
  bool denied=false;for(const auto& b:memory->storage_bindings())if(b.identity==state.accept_raw[0]){try{memory->free_04b788(*b.storage);}catch(const std::logic_error&){denied=true;}}
  require(denied&&memory->snapshot().pending==1&&view.memory.snapshot().buffer_borrows==1,"Accept memory rejects freeing its pending473-byte SDK borrower even after cancel intent");
  require(admitted.post(view.scheduler.completion.port,17,0,state.accept_packets[0])!=0,"Controlled SDK final completion is observed through real own IOCP, not direct complete helper");
  Completion io_completion;s::IoCompletionEnvironment ioenv=context->environment;DispatchEffects io_router(view.io,ioenv,api,errors,io_completion);s::AcceptDispatcherEffects router(*memory,io_router);
  s::DispatcherState dispatch{view.scheduler,nullptr,0,0,memory->dispatcher_bindings()};std::int32_t epoch=0;const s::Category generic{0x1111,server::ErrorCategory::system},system{0x2222,server::ErrorCategory::system};
  Worker worker(view,state,dispatch,{epoch,generic,system},admitted,router,api,errors,*memory);s::WorkerRunResult result;
  const auto handled=s::run_worker_0298b0(view.scheduler,result,{index,generic},api,errors,worker);
  require(handled==3&&result.error&&result.error->value==0&&worker.init==1&&worker.finish==1&&state.accept_callbacks==2&&state.cancel_callbacks==1&&state.functor_calls==1&&state.job_done,"Actual0298b0/027c10 routes normal accept, reentrant posted job and cancelled accept through composite owned completion routers");
  require(::TlsGetValue(index)==nullptr&&state.cache_frees==2&&!view.io.snapshot().allocations&&!memory->snapshot().projections&&!memory->snapshot().pending&&view.scheduler.work_count_30==0,"Actual worker restores TLS and retires both original473-byte accept/posted cache identities after observed callbacks");
  require(observer&&observer->strong_08==0&&observer->weak_0c==1&&view.memory.snapshot().allocations==1,"Factory session disposal frees actual input/proxy/socket while final observer weak retains only control");
  weak_release(observer);observer=nullptr;state.session_deleted=view.memory.snapshot().controls_destroyed==1;state.worker_done=true;job.reset();
  unsigned relays=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;const auto got=admitted.get(view.scheduler.completion.port,bytes,key,raw,0);const auto error=admitted.get_last_error();
   if(!got){require(error==WAIT_TIMEOUT&&!raw,"Observed queue timeout follows real stop relay drain");break;}require(!raw&&!key&&!bytes,"Only source stop relay remains after accepted/cancelled callbacks");++relays;
  }
  require(relays==1&&state.gets==state.wait_scopes&&!admitted.poisoned(),"Every physical accept/posted/cancel/stop wait releases and reacquires parent admission before lookup");state.queue_drained=true;state.accept_packets_drained=true;state.view=nullptr;completion.admitted=nullptr;
  require(!view.memory.snapshot().allocations&&!memory->snapshot().active_calls&&view.service.head==&view.pending&&view.pending.next==&view.acceptor,"All actual accept allocations/sessions/cache loans drain before trusted server shutdown check");return {};
 }
 bool quiescent(o::ServerView& view)override{
  ++state.quiescent_calls;bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}
  require(outside&&view.backend.snapshot().phase==o::Phase::Quiescing&&view.memory.snapshot().provider_leases==2,"Lower accept quiescence runs outside ROOT with own provider lease still retaining backing domain");
  const bool drained=state.worker_done&&state.job_done&&state.session_deleted&&state.queue_drained&&state.accept_packets_drained&&!state.accept_pending;
  require(drained&&memory->close()&&view.memory.snapshot().provider_leases==1,"Accept provider closes only after observed final packets/raw jobs/cache drain, before B35 source counts/list attestation");return drained;
 }
};
struct ExecutionPlatform:anadius::execution_recovered::Platform {
 unsigned locks=0,unlocks=0;std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}
 void acquire_srw_exclusive(std::uintptr_t&)override{++locks;}void release_srw_exclusive(std::uintptr_t&)override{++unlocks;}
 std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}
};
void normal_server(bool ipv6){
 NativeDomain native;auto state=std::make_shared<State>();state->ipv6=ipv6;running=state.get();State* observed=state.get();std::weak_ptr<State> lifetime=state;
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);
 std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto errors=std::make_shared<Errors>(*state);o::Dependencies dependencies;dependencies.state_lifetime=state;dependencies.errors=errors;auto sockets=std::make_shared<Sockets>(*state);dependencies.sockets=sockets;
  s::IocpConstructorImports port_imports;port_imports.create_completion_port=create_port;port_imports.close_handle=close_port;
  dependencies.iocp=std::make_shared<s::NativeIocpConstructorApi>(port_imports);dependencies.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});
  auto dispatcher=std::make_shared<Dispatcher>(*state);dependencies.dispatcher=dispatcher;dependencies.cancellation=std::make_shared<Cancel>(*state);dependencies.executor=std::make_shared<Executor>(*state);dependencies.runtime=std::make_shared<Runtime>(*state,*errors,*sockets,*dispatcher);
  dependencies.kernel={startup,cleanup,local_name,environment,gate_delete};
  o::OwnedServerBackend backend(native.root,std::move(dependencies),16);errors.reset();sockets.reset();dispatcher.reset();state.reset();
  require(!lifetime.expired()&&native.root->provider_leases()==1,"Backend dependency state and dedicated real ApplicationMemory lease own their required provider graph");
  std::uint16_t port=ipv6?7777:0;
  server::server_start_029ea0(backend,port,ready,execution);
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1,"Actual server source publishes bound port and one readiness flag through actual Execution mutex route");
  require(observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Actual server path publishes decimal bound port despite ignored FALSE environment result");
  require(observed->families==(ipv6?std::vector<int>{23}:std::vector<int>{23,2}),"Actual source address selection uses IPv6 success or IPv4 fallback without network operations");
  require(observed->bound.size()==(ipv6?28U:16U)&&std::to_integer<unsigned char>(observed->bound[0])==(ipv6?23:2)&&std::to_integer<unsigned char>(observed->bound[2])==(ipv6?0x1e:0)&&std::to_integer<unsigned char>(observed->bound[3])==(ipv6?0x61:0),"Actual acceptor constructs exact family/initial-port sockaddr bytes");
  require(observed->closed==std::vector<s::Socket>({201,202,200})&&observed->starts==1&&observed->cleanups==1&&observed->ports_created==1&&observed->ports_closed==1&&observed->gate_inits==2&&observed->gate_deletes==2,"Accepted session and listener close once; real IOCP, owned gates and WSA scope tear down after attested drain");
  const auto snapshot=backend.snapshot();require(snapshot.phase==o::Phase::Closed&&!snapshot.active_calls&&!snapshot.acceptor_initialized&&!snapshot.pending_initialized&&snapshot.quiescence_attested&&observed->quiescent_calls==1,"Full actual029ea0 source teardown reaches Closed only after explicit trusted lower quiescence");
  require(native.root->provider_leases()==0&&observed->executor_cleanups==4&&!observed->cancellations,"Source lifecycle destroys temporary/session/pending/acceptor while no invented cancellation engine exists");
  auto close=std::find(observed->order.begin(),observed->order.end(),"close-port"),clean=std::find(observed->order.begin(),observed->order.end(),"cleanup");
  require(close!=observed->order.end()&&clean>close&&std::count(close,clean,"delete-gate")==2,"Actual resource order closes IOCP before both owned gates and WSA cleanup");
 }
 require(lifetime.expired(),"State lifetime root outlives dependent providers and is released after closed backend destruction");running=nullptr;
 require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Backing actual process heap/PTD/FLS domain has no leaked session/IO/provider resources");
}

}
int main(){try{normal_server(false);normal_server(true);std::cout<<"PASS: "<<checks<<" accept integration checks; actual021b00/0271a0 and owned-server admitted native worker, canonical heap-backed473 storage, injectedAcceptEx only, actualfactory/disposal/cancelled-packet/cache drain\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected accept integration exception\n";return 1;}}
