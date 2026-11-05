#include "graph.hpp"
#include "../anadius-application-worker-source/worker.hpp"
#include <sstream>
#include "../anadius-native-profile-source/native.hpp"
#include "../anadius-server-runtime-source/resources.hpp"
#include <optional>
#include "../anadius-socket-source/response_containers.hpp"
#include "../anadius-owned-server-source/server.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include "../anadius-socket-source/worker_run_native.hpp"
#include "../anadius-owned-jobs-source/jobs.hpp"
#include "../anadius-information-effects-source/effects.hpp"
#include "../anadius-content-effects-source/effects.hpp"
#include "../anadius-licensing-effects-source/effects.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <mutex>
#include <process.h>
#include <vector>

namespace {
namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace server=anadius::server_recovered;
namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;namespace sp=anadius::session_profile;namespace np=anadius::native_profile;namespace cp=anadius::session_policy;namespace ag=anadius::application_graph;
namespace ast=anadius::accept_stop;
namespace aw=anadius::application_worker;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected owned-server integration dependency");}
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
struct Connection {
 unsigned received_logs=0,response_logs=0,event_logs=0;bool live_request_seen=false,auto_decrypted=false;std::string expected_response;std::string expected_request;std::vector<std::string> requests,expected_requests;std::string receive_wire;std::size_t cursor=0;bool receive_waiting=false,eof_posted=false;s::Buffer receive_buffer{};OVERLAPPED* receive_ov=nullptr;bool send_waiting=false,send_posted=false;OVERLAPPED* send_ov=nullptr;std::uint32_t send_bytes=0;unsigned completed_sends=0;
 s::Socket socket=0;sp::SessionToken token{};std::uintptr_t owner_id=0,greeting_data=0,greeting_tail=0;bool encrypted=false,greeting_gone=false,eof_seen=false,logic_destroyed=false;unsigned receives=0,sends=0,sync_calls=0,processed=0;std::uint32_t max_sync=0;s::SessionCipher client;std::string greeting_wire,response_wire,event_text;
};
struct NativeTrace;
struct State {
 bool native_mode=false;DWORD original_thread=0;std::string native_failure;std::shared_ptr<NativeTrace> native_trace;std::shared_ptr<anadius::owned_jobs::NativeJobs> native_jobs;
 unsigned native_notices=0,native_delivered=0;std::uint32_t native_before_wait_work=0,delayed_before_decrement_work=0;bool delayed_acknowledged=false,delayed_retention_checked=false;
 unsigned native_external_calls=0;bool pending_close_retried=false;std::shared_ptr<anadius::services_completion::WaitTarget> close_wait;
 bool drain_resume_posted=false,drain_counter_checked=false,resume_denied=false;std::uint32_t drain_before_decrement_work=0,resume_failed_work=0;
 std::shared_ptr<ast::Stop> session_stop;std::vector<OVERLAPPED*> session_cancel_packets;unsigned session_cancel_calls=0;
 std::atomic<bool> worker_source_io_drained=false;std::atomic<unsigned> worker_provider_retirements=0,worker_external_proofs=0;std::atomic<DWORD> provider_retired_thread=0;
 anadius::configuration::State configuration{};std::uint8_t online=0;std::int32_t saved=0;anadius::configuration::Path achievement_file;std::weak_ptr<ag::Graph> graph;
 unsigned sdk_pending=0,sdk_expected_error=0,receive_error_calls=0,send_error_calls=0,receive_success=0,send_success=0;std::vector<char> sdk_trace;
 unsigned receive_packets=0,zero_receive_packets=0,send_packets=0,posted_packets=0,accept_completions=0;
 unsigned plain_stage=0;bool flush_started=false,wrap_seen=false,growth_seen=false,backlog_seen=false;std::uintptr_t old_map=0,new_map=0,current_owner=0;
 bool ipv6=false,accept_pending=false,shutdown_posted=false,shutdown_done=false,terminal_selected=false,overlap_seen=false,cleared_seen=false,worker_done=false,queue_drained=false;
 std::uint8_t* ready=nullptr;o::ServerView* view=nullptr;std::optional<o::ServerView> view_value;sp::Application* application=nullptr;std::array<std::unique_ptr<Connection>,2> connections;
 unsigned sdk_accepts=0,address_reads=0,context_updates=0,starts=0,cleanups=0,gate_inits=0,gate_deletes=0,ports_created=0,ports_closed=0,factory_calls=0,moves=0,gets=0,posts=0,wait_scopes=0,initial_waits=0,executor_cleanups=0,cancellations=0,processed=0;
 std::vector<OVERLAPPED*> accept_packets;std::vector<s::Socket> closed;std::vector<int> families;std::vector<std::string> order;std::string environment_key,environment_value;std::vector<std::byte> bound;std::uintptr_t port=0;
 Connection& connection(s::Socket socket){if(socket<201||socket>202||!connections[socket-201])unused();return *connections[socket-201];}
};
State* running=nullptr;
void join_native_children(State&);
void native_receive(State&,Connection&,s::Buffer,OVERLAPPED*);
void native_observe(State&,sp::SessionView,ag::Log,std::string_view,std::string_view);
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
BOOL PASCAL accept_sdk(SOCKET listener,SOCKET accepted,PVOID buffer,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED ov){
 auto& st=*running;st.accept_pending=true;require(listener==200&&accepted==201+st.sdk_accepts&&buffer&&!receive&&local==0x90&&remote==0x90,"Profile integration uses exact injected AcceptEx boundary with distinct201/202/203 sockets and90+90 endpoints");
 st.accept_packets.push_back(ov);++st.sdk_accepts;
 if(st.sdk_accepts<=(st.native_mode?1U:2U)){if(st.application){auto ports=st.application->ports();require(ports.accept.snapshot().pending==1,"Concrete Profile/native flow retains next accept SDK loan alongside active session");if(!ports.completion.post(st.port,0,0,ov))unused();}else if(!::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(st.port),0,0,ov))unused();}
 *bytes=0x4567;::WSASetLastError(997);return FALSE;
}
VOID PASCAL accept_addresses(PVOID buffer,DWORD received,DWORD local_size,DWORD remote_size,sockaddr** local,int* local_bytes,sockaddr** remote,int* remote_bytes){
 auto& st=*running;require(received==0&&local_size==0x90&&remote_size==0x90&&st.application&&st.application->ports().accept.snapshot().pending==0,"Observed completion releases accept loan before native endpoint address extraction");
 auto* bytes=static_cast<std::byte*>(buffer);std::fill_n(bytes,288,std::byte{});auto* address=bytes+144;
 address[0]=static_cast<std::byte>(st.ipv6?23:2);address[2]=std::byte{0x12};address[3]=std::byte{0x34};address[st.ipv6?23:7]=std::byte{0x7f};
 *local=reinterpret_cast<sockaddr*>(bytes);*local_bytes=st.ipv6?28:16;*remote=reinterpret_cast<sockaddr*>(address);*remote_bytes=*local_bytes;++st.address_reads;
}
int WSAAPI update_context(SOCKET socket,int level,int option,const char* value,int length){
 auto& st=*running;s::Socket listener=0;std::memcpy(&listener,value,sizeof(listener));
 require((socket==201||socket==202)&&level==0xffff&&option==0x700b&&length==8&&listener==200,"Native0271a0 updates accepted context with captured listener pointer value and exact option700b/size8");++st.context_updates;return 0;
}
int WINAPI convert_sdk(UINT page,DWORD flags,LPCCH text,int bytes,LPWSTR output,int capacity){
 require(page==0&&!flags&&bytes==-1&&capacity==10&&std::string_view(text)=="127.0.0.1"&&output,"Concrete NativeApi forwards ACP exact loopback C string and bounded10 wide conversion");
 std::memcpy(output,L"127.0.0.1",10*sizeof(wchar_t));return 10;
}
INT WSAAPI parse_sdk(LPWSTR text,INT family,LPWSAPROTOCOL_INFOW info,LPSOCKADDR address,LPINT size){
 auto& state=*running;require(std::wstring_view(text)==L"127.0.0.1"&&!info&&address&&*size==128,"Concrete NativeApi forwards persistent converted text, null protocol and initialized128byte sockaddr capacity");state.families.push_back(family);
 if(family==23&&!state.ipv6){::WSASetLastError(10022);return SOCKET_ERROR;}
 std::memset(address,0,*size);if(family==23){sockaddr_in6 value{};value.sin6_family=23;value.sin6_addr.u.Byte[15]=1;value.sin6_scope_id=9;std::memcpy(address,&value,sizeof(value));*size=sizeof(value);}else{if(family!=2)unused();sockaddr_in value{};value.sin_family=2;reinterpret_cast<unsigned char*>(&value.sin_addr)[0]=127;reinterpret_cast<unsigned char*>(&value.sin_addr)[3]=1;std::memcpy(address,&value,sizeof(value));*size=sizeof(value);}::WSASetLastError(0);return 0;
}
int WSAAPI sync_send_sdk(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,DWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
 auto& c=running->connection(socket);require(buffers&&count==1&&transferred&&!*transferred&&!flags&&!ov&&!callback,"Concrete NativeApi synchronous greeting uses exact WSASend one-buffer/zeroDWORD/flags0/nullOV/nullcallback imports");auto& b=buffers[0];c.max_sync=std::max(c.max_sync,static_cast<std::uint32_t>(b.len));if(!c.sync_calls)c.greeting_data=reinterpret_cast<std::uintptr_t>(b.buf);
 const auto n=c.sync_calls==0?std::min(static_cast<std::uint32_t>(b.len),30000U):c.sync_calls==1?0U:b.len;++c.sync_calls;c.greeting_wire.append(b.buf,n);if(n)c.greeting_tail=reinterpret_cast<std::uintptr_t>(b.buf)+n-1;*transferred=n;::WSASetLastError(0);return 0;
}
int WSAAPI select_sdk(int,fd_set*,fd_set*,fd_set*,const timeval*){unused();}
struct Sockets:s::AcceptorApi {
 State& state;explicit Sockets(State& s):state(s){}
 s::Socket create_socket(std::int32_t family)override{require(family==(state.ipv6?23:2),"Actual020e20 opens selected family through synthetic socket SDK boundary");state.order.push_back("socket");return 200;}
 std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require((socket==200||socket==201||socket==202)&&port==state.port,"Actual acceptor/accepted transfer attaches selected socket to same owned kernel completion port");return port;}
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
  require(port==state.port,"Every physical wait belongs to own concrete Runtime IOCP");
  if(state.view){
  bool released=false;try{(void)state.view->memory.buffer_context();}catch(const std::logic_error&){released=true;}
  require(released,"AdmittedDispatcherApi releases root admission before physical SDK wait");
  {auto independent=state.view->memory.enter();++state.wait_scopes;}
  }else ++state.initial_waits;
  if(state.native_mode)join_native_children(state);
  ++state.gets;DWORD transferred=0;ULONG_PTR token=0;LPOVERLAPPED raw=nullptr;
  auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&transferred,&token,&raw,state.native_mode&&timeout==INFINITE?10000:timeout);auto error=::GetLastError();
  // This fixture's sockets/CancelIoEx are injected. The own IOCP preserves
  // packet identity; this scripted SDK result is not a real kernel cancel.
  if(raw){const auto found=std::find(state.session_cancel_packets.begin(),state.session_cancel_packets.end(),raw);if(found!=state.session_cancel_packets.end()){state.session_cancel_packets.erase(found);result=FALSE;error=ERROR_OPERATION_ABORTED;}}
  if(state.native_mode){if(!result&&error==WAIT_TIMEOUT&&timeout==INFINITE)::ExitProcess(245);if(token==s::prepared_job_completion_key&&!raw)++state.native_notices;}
  bytes=transferred;key=token;operation=raw;::SetLastError(error);return result;
 }
 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation)override{
  ++state.posts;
  if(state.native_failure.starts_with("early-drain")&&!bytes&&!key&&!operation&&state.native_notices==1&&!state.drain_resume_posted&&state.native_jobs->snapshot().completed==1){
   state.drain_resume_posted=true;state.drain_before_decrement_work=state.view->scheduler.work_count_30.load();
   if(state.native_failure=="early-drain-resume-fail"){state.resume_denied=true;state.resume_failed_work=state.drain_before_decrement_work;::SetLastError(ERROR_INVALID_HANDLE);return FALSE;}
  }
  return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),bytes,key,operation);
 }
 std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct WorkerApi:s::WorkerRunApi {
 std::uint32_t index;s::DispatcherApi& admitted;s::NativeWorkerRunApi native;
 WorkerApi(std::uint32_t i,s::DispatcherApi& a):index(i),admitted(a){}
 void* tls_get(std::uint32_t i)override{if(i!=index)unused();return native.tls_get(i);}std::int32_t tls_set(std::uint32_t i,void* p)override{if(i!=index)unused();return native.tls_set(i,p);}
 std::int32_t post_stop(std::uintptr_t p,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* raw)override{return admitted.post(p,bytes,key,raw);}
 std::uint32_t get_last_error()override{return admitted.get_last_error();}
};
struct Information final:anadius::handlers_recovered::Effects {
    void log_profile_name_084f48(std::string_view)override{unused();}
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
struct LicenseBoundary final:anadius::dispatch_recovered::LicenseHandlers {
    unsigned calls=0;
    bool auth_code_02e3b0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool request_license_0320c0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool invalidate_license_030c50(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
};

struct Time final:anadius::crypto_recovered::TimeSource {std::int64_t unix_seconds_05eeb8()override{return 1000000000;}};
void prepare_peer(State& state,sp::SessionView view){
 const auto socket=view.owner.socket_state.socket;if(socket<201||socket>202||state.connections[socket-201])unused();auto c=std::make_unique<Connection>();c->socket=socket;c->token=view.token;c->owner_id=view.owner.identity();c->encrypted=socket==202;c->expected_response=c->encrypted?"GetConfigResponse":"QueryEntitlementsResponse";
 require(view.owner.cipher.encrypted_c4==0&&view.owner.cipher.unknown_c5==0&&view.owner.cipher.native().mode_00==0,"Graph prepare callback receives actual023000 owner after second native reset and supplies NOlogic/context graph");if(c->encrypted){view.owner.cipher.set_fixed_key_084c98();view.owner.cipher.encrypted_c4=1;c->client.set_fixed_key_084c98();}state.connections[socket-201]=std::move(c);
}
void observe(State& state,sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){
 if(state.native_mode){native_observe(state,view,kind,text,error);return;}
 auto& c=state.connection(view.owner.socket_state.socket);auto graph=state.graph.lock();if(!graph||view.token!=c.token||view.owner.identity()!=c.owner_id)unused();
 require(error.empty()&&graph->retained(view.token),"Graph observer runs on exact retained typed native invocation without caller MessageEffects/Logic");
 if(kind==ag::Log::Received){
  ++c.received_logs;bool mapped=false;auto ports=state.application->ports();for(const auto& region:ports.view.memory.buffer_context().memory.regions)if(region.bytes.size()>=text.size())for(std::size_t i=0;i<=region.bytes.size()-text.size();++i)if(std::equal(text.begin(),text.end(),region.bytes.begin()+i)){mapped=true;break;}
  require(mapped&&text==c.expected_requests.at(c.processed),"ConcreteGraph realXML/native033d70 received observer sees exact live Parent-backed request BEFOREdispatch");c.live_request_seen=true;
  if(c.encrypted){require(view.owner.cipher.encrypted_c4==1&&view.owner.cipher.native().mode_00==2,"Declared synthetic encrypted peer uses actual native prefix AES decrypt with SAMEsession cipher before observer");c.auto_decrypted=true;}
 }else if(kind==ag::Log::Response){++c.response_logs;++c.processed;++state.processed;require(text.find(c.expected_response)!=std::string_view::npos,"Concrete owned Router produces real response BEFORE native encryption/hex/NUL");}
 else if(kind==ag::Log::Event){++c.event_logs;c.event_text.assign(text);const auto snapshot=graph->snapshot();require(text.find("DOWN")!=std::string_view::npos&&snapshot.event_locked&&snapshot.queued_events>0,"Graph-owned lazy DOWNevent is observed while actual native event mutex/front are live");}
 else unused();
}

void deliver_receive(State& state,Connection& c){
 if(!c.receive_waiting)unused();auto ports=state.application->ports();const auto left=c.receive_wire.size()-c.cursor;
 if(!left&&c.socket==201&&(state.native_mode?c.completed_sends<70:state.plain_stage<2))return;
 const auto count=std::min<std::size_t>(left,c.receive_buffer.len);if(count)std::memcpy(c.receive_buffer.buf,c.receive_wire.data()+c.cursor,count);else{if(c.eof_posted)unused();c.eof_posted=true;}
 c.cursor+=count;c.receive_waiting=false;if(!ports.completion.post(state.port,static_cast<std::uint32_t>(count),0,c.receive_ov))unused();
}
void post_send(State& state,Connection& c){if(!c.send_waiting||c.send_posted)unused();auto ports=state.application->ports();c.send_posted=true;if(!ports.completion.post(state.port,c.send_bytes,0,c.send_ov))unused();}
int WSAAPI receive_sdk(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,LPDWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
 auto& state=*running;require(!state.sdk_pending&&buffers&&count==1&&transferred&&!*transferred&&flags&&!*flags&&ov&&!callback,"Default NativeAsyncReceiveApi forwards exact WSARecv one-buffer/privateflags/zeroDWORD/realOV/nullcallback signature");auto& buffer=buffers[0];
  if(!state.application)unused();auto& c=state.connection(socket);auto& app=*state.application;auto ports=app.ports();++c.receives;
  if(state.native_mode){native_receive(state,c,buffer,ov);*flags=0xface;state.sdk_pending=1;state.sdk_expected_error=997;state.sdk_trace.push_back('R');::WSASetLastError(997);return SOCKET_ERROR;}
  if(c.receives==1){
   auto session=app.session(c.token);require(session.owner.identity()==c.owner_id&&ports.view.memory.owns_control(session.owner.weak_self.control_08),"Profile session token resolves actual023000 control, not a caller-prepared owner");
   try{(void)ports.view.memory.buffer_context().memory.view(c.greeting_tail,1);}catch(const s::UnsupportedInput&){c.greeting_gone=true;}
    if(socket==201)require(app.registry().frames.size()==1,"First Graph peer uses initial1 Profile slot and native Logic row before growth");
    if(socket==202){auto registry=app.registry();const auto& first=state.connection(201);require(app.snapshot().live_sessions==2&&ports.view.memory.snapshot().controls==2&&registry.frames.size()==2&&registry.frames[0].owner&&registry.frames[1].owner&&registry.strings[0].string&&registry.strings[1].string&&registry.reads[0].implementation&&registry.reads[1].implementation&&registry.writes[0].implementation&&registry.writes[1].implementation&&registry.finish[0].owner&&registry.finish[1].owner&&registry.queued[0].owner&&registry.queued[1].owner&&registry.messages[0].owner&&registry.messages[1].owner&&registry.environments[0].source&&registry.environments[1].source&&app.session(first.token).owner.identity()==first.owner_id&&app.session(first.token).owner.weak_self.control_08->strong_08>0,"Two actual Graph peers force Profile and native Logic row growth from initial1 while retaining the first plain peer's exact owner and live read/send contexts");require(ports.view.io.storage_bindings().size()>1&&ports.environment.storage.current_storage().data()==ports.view.io.storage_bindings().data()&&ports.environment.storage.current_storage().size()==ports.view.io.storage_bindings().size(),"Runtime I/O tables grow from initial1 while copied Ports follow the current owner view for both overlapping peers");state.overlap_seen=true;}
   const auto build=[&](unsigned index){return "<LSX><Request "+std::string()+"id=\"conn"+std::to_string(socket-200)+(c.encrypted?"":"-"+std::to_string(index))+"\" recipient=\"EbisuSDK\">"+std::string(c.encrypted?"<GetConfig/>":"<QueryEntitlements Group=\"synthetic-group\"><FilterOffers>synthetic-offer</FilterOffers></QueryEntitlements>")+"</Request></LSX>";};
   for(unsigned i=0;i<(c.encrypted?1U:17U);++i){auto request=build(i);auto expected=request;c.requests.push_back(request);c.expected_requests.push_back(expected);}
   c.expected_request=c.expected_requests[0];for(unsigned i=0;i<(c.encrypted?1U:7U);++i){auto request=c.requests[i];if(c.encrypted){if(!c.client.encrypt_042610(request))unused();m::hex_encode_03d190(request);}request.push_back(0);c.receive_wire+=request;}
  }
  if(c.receive_waiting)unused();c.receive_waiting=true;c.receive_buffer=buffer;c.receive_ov=ov;deliver_receive(state,c);

 const bool success=socket==202&&c.receives==1;if(success){++state.receive_success;*transferred=static_cast<DWORD>(c.cursor);}*flags=0xface;
 state.sdk_pending=1;state.sdk_expected_error=success?1234U:997U;state.sdk_trace.push_back('R');::WSASetLastError(state.sdk_expected_error);return success?0:SOCKET_ERROR;
}
int WSAAPI send_sdk(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,DWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
 auto& state=*running;require(!state.sdk_pending&&buffers&&count==1&&transferred&&!*transferred&&!flags&&ov&&!callback,"Default NativeAsyncSendApi forwards exact WSASend one-buffer/zeroDWORD/flags0/realOV/nullcallback signature");auto& buffer=buffers[0];auto& c=state.connection(socket);if(c.send_waiting)unused();++c.sends;c.response_wire.append(buffer.buf,buffer.len);c.send_waiting=true;c.send_posted=false;c.send_ov=ov;c.send_bytes=buffer.len;if(c.encrypted||state.native_mode)post_send(state,c);
 const bool success=socket==202&&c.sends==1;if(success){++state.send_success;*transferred=buffer.len;}state.sdk_pending=2;state.sdk_expected_error=success?1234U:997U;state.sdk_trace.push_back('S');::WSASetLastError(state.sdk_expected_error);return success?0:SOCKET_ERROR;
}
int error_sdk(unsigned kind){auto& state=*running;const auto error=::WSAGetLastError();require(state.sdk_pending==kind&&static_cast<unsigned>(error)==state.sdk_expected_error,"Native async import reads WSAGetLastError immediately and unconditionally, including successful non997 result");state.sdk_pending=0;state.sdk_trace.push_back('E');if(kind==1)++state.receive_error_calls;else ++state.send_error_calls;return error;}
int WSAAPI receive_error_sdk(){return error_sdk(1);}int WSAAPI send_error_sdk(){return error_sdk(2);}
bool clear(const sp::RegistryView& registry){bool result=true;for(const auto& v:registry.frames)result&=!v.identity&&!v.owner;for(const auto& v:registry.strings)result&=!v.identity&&!v.string;for(const auto& v:registry.reads)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.writes)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.finish)result&=!v.identity&&!v.owner;for(const auto& v:registry.queued)result&=!v.identity&&!v.owner;for(const auto& v:registry.messages)result&=!v.identity&&!v.owner;for(const auto& v:registry.environments)result&=!v.allocator_identity&&!v.source;return result;}
// Explicit selected lifecycle driver. It observes packets and external drain;
// those decisions are not supplied by concrete Policy itself.
struct Hooks {
 State& state;Dispatcher& dispatcher;cp::Policy* policy=nullptr;explicit Hooks(State& s,Dispatcher& d):state(s),dispatcher(d){}
 s::IocpSchedulerEntry* completion_owner(sp::Application& application,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){
  state.application=&application;auto ports=application.ports();if(!state.view_value){state.view_value.emplace(ports.view);state.view=&*state.view_value;}
  const bool accept_packet=ports.accept.contains(&op);if(accept_packet)++state.accept_completions;
  if(state.shutdown_posted&&&op.overlapped==state.accept_packets.back()){require(error.value==995&&application.snapshot().live_sessions==0&&application.snapshot().live_jobs==0,"Selected final-owner0 policy requires observed203/995 after both Profile sessions/jobs already reaped");state.terminal_selected=true;return nullptr;}
  if(!accept_packet){const auto handle=ports.view.io.handle_for(&op);if(ports.view.io.kind(handle)==s::IoMemoryKind::Posted)++state.posted_packets;if(ports.view.io.kind(handle)==s::IoMemoryKind::Send){++state.send_packets;const auto id=ports.view.io.send_operation(handle).io.handler.object_90;for(auto& c:state.connections)if(c&&c->owner_id==id){if(!c->send_waiting||!c->send_posted||c->send_ov!=&op.overlapped)unused();c->send_waiting=false;c->send_posted=false;++c->completed_sends;}}if(ports.view.io.kind(handle)==s::IoMemoryKind::Receive){++state.receive_packets;if(ports.view.io.receive_operation(handle).io.requested_58==0)++state.zero_receive_packets;state.current_owner=ports.view.io.receive_operation(handle).io.handler.words_88_98[2];}if(ports.view.io.kind(handle)==s::IoMemoryKind::Receive&&ports.view.io.receive_operation(handle).io.requested_58!=0&&bytes==0&&!error.value){const auto id=ports.view.io.receive_operation(handle).io.handler.words_88_98[2];for(auto& c:state.connections)if(c&&c->owner_id==id)c->eof_seen=true;}}
  return owner;
 }
 void after_completion(sp::Application& application){
  auto ports=application.ports();if(state.terminal_selected){state.terminal_selected=false;state.shutdown_done=true;state.accept_pending=false;}
  for(auto& c:state.connections)if(c&&c->eof_seen&&!application.registry().frames[c->token.slot].owner)c->logic_destroyed=true;
  auto& plain=state.connection(201);
  if(!plain.logic_destroyed){auto session=application.session(plain.token);auto& queue=session.owner.output.fields;
   if(!state.old_map)state.old_map=reinterpret_cast<std::uintptr_t>(queue.map_d0.data());
   if(state.plain_stage==0&&queue.count_e8==7){require(queue.map_size_d8==8&&queue.offset_e0==0&&plain.processed==7&&plain.receive_waiting,"Seven real native responses fill initial map8 while actual next receive and first send remain kernel-pending");state.plain_stage=1;state.flush_started=true;}
   if(state.plain_stage==1&&plain.completed_sends==5){require(queue.map_size_d8==8&&queue.offset_e0==5&&queue.count_e8==2&&plain.receive_waiting,"Five observed native send completions advance real deque head to5/count2 without fixture prefix mutation");state.plain_stage=2;for(unsigned i=7;i<17;++i){plain.receive_wire+=plain.requests[i];plain.receive_wire.push_back(0);}deliver_receive(state,plain);}
   if(state.plain_stage==2&&queue.map_size_d8==8&&queue.offset_e0==5&&queue.count_e8>=4)state.wrap_seen=true;
   if(state.plain_stage==2&&queue.map_size_d8==16&&queue.count_e8==12){
    state.new_map=reinterpret_cast<std::uintptr_t>(queue.map_d0.data());bool old_live=false,new_live=false;for(const auto& map:ports.view.memory.output_map_bindings()){old_live|=map.address==state.old_map;new_live|=map.address==state.new_map&&map.slots.size()==16;}
    require(state.wrap_seen&&queue.offset_e0==5&&plain.processed==17&&plain.completed_sends==5&&state.new_map!=state.old_map&&!old_live&&new_live&&ports.view.memory.owns_output_queue(&queue),"Actual Profile-owned026670 grows wrapped8to16, retires old typed-map ledger row and preserves live exact queue authority");
    for(unsigned i=0;i<12;++i){auto* value=queue.map_d0[(queue.offset_e0+i)&15];if(!value)unused();std::string text;if(value->capacity_18<=15)text.assign(reinterpret_cast<const char*>(value->storage_00.data()),value->size_10);else{auto bytes=ports.view.memory.buffer_context().memory.view(s::receive_buffer_address(*value),value->size_10);text.assign(reinterpret_cast<const char*>(bytes.data()),bytes.size());}if(text.find("id=\"conn1-"+std::to_string(i+5)+"\"")==std::string::npos)unused();}
    require(plain.send_waiting&&!plain.send_posted&&queue.count_e8>=8,"Twelve real queued payloads retain exact FIFO identities after growth while active SDK send is still pinned");state.growth_seen=true;state.backlog_seen=true;state.plain_stage=3;
   }
   if((state.plain_stage==1&&plain.completed_sends<5)||state.plain_stage==3)if(plain.send_waiting&&!plain.send_posted)post_send(state,plain);
  }
  if(application.snapshot().live_sessions==0&&state.processed==18&&!state.shutdown_posted){require(clear(application.registry())&&state.connections[0]->logic_destroyed&&state.connections[1]->logic_destroyed&&ports.view.memory.snapshot().controls_destroyed==2,"Profile clears every fixed binding and logic before final observer delete without fixture cleanup");state.cleared_seen=true;state.shutdown_posted=true;const auto h=ports.accept.handle_for(state.accept_packets.back());ports.accept.note_cancel_requested(h);for(auto& b:ports.accept.dispatcher_bindings())if(b.overlapped_identity==state.accept_packets.back())s::publish_send_status_028b50(ports.view.scheduler,*b.operation,b,995,0,ports.completion,*errors);}
 }
 s::Api* errors=nullptr;
 void worker_returned(sp::Application& application,std::uint64_t count,const s::WorkerRunResult& result){
  auto ports=application.ports();require(count==59&&state.accept_completions==3&&state.receive_packets==state.connections[0]->receives+state.connections[1]->receives+state.zero_receive_packets&&state.zero_receive_packets==11&&state.posted_packets==18&&state.send_packets==18&&result.error&&result.error->value==0&&policy->snapshot().worker_initializations==1&&policy->snapshot().worker_finishes==1,"Actual59 native packets include11 immediate zero-sized receive completions for buffered frames plus all SDK/posted response/send/accept completions");
  require(state.overlap_seen&&state.cleared_seen&&state.shutdown_done&&!application.snapshot().live_sessions&&!application.snapshot().live_jobs&&clear(application.registry()),"Bounded Profile owns and retires overlapping session/job registries without caller-built contexts or observer cleanup");
  for(auto& c:state.connections){
   std::vector<std::string> frames;std::size_t cursor=0;while(cursor<c->response_wire.size()){const auto end=c->response_wire.find('\0',cursor);if(end==std::string::npos)unused();frames.push_back(c->response_wire.substr(cursor,end-cursor));cursor=end+1;}
   const auto messages=c->encrypted?1U:17U;require(c->processed==messages&&c->sends==messages&&c->completed_sends==messages&&c->received_logs==messages&&c->response_logs==messages&&c->live_request_seen&&c->eof_posted,"Every real native input generates one observed FIFO queued response/send and preserves typed live-prefix callbacks");
   if(c->encrypted){require(frames.size()==2,"Encrypted peer retains separate response/event NUL frames during other session backlog");auto response=frames[0];m::hex_decode_03ce30(response);require(c->client.decrypt_042480(response)&&response.find("GetConfigResponse")!=std::string::npos&&response.find("id=\"conn2\"")!=std::string::npos,"Independent client decrypts actual native prefix response while plain queue grows");auto event=frames[1];m::hex_decode_03ce30(event);require(c->client.decrypt_042480(event)&&event==c->event_text&&event.find("DOWN")!=std::string::npos&&c->event_logs==1&&c->auto_decrypted,"Typed prefix AES event chaining/pop remains intact alongside owned deque growth");}
   else {require(frames.size()==17&&c->event_logs==0&&state.flush_started&&state.wrap_seen&&state.growth_seen&&state.backlog_seen,"Actual17-response deque spans initial drain, head wrap and owned8to16 growth without legacy policy ");bool ordered=true;for(unsigned i=0;i<17;++i)ordered&=frames[i].find("QueryEntitlementsResponse")!=std::string::npos&&frames[i].find("ItemId=\"synthetic-offer\"")!=std::string::npos&&frames[i].find("Version=\"synthetic-version\"")!=std::string::npos&&frames[i].find("id=\"conn1-"+std::to_string(i)+"\"")!=std::string::npos;require(ordered,"All17 physical SDK responses preserve exact FIFO IDs and concrete Graph Router reads actual pinned StateView entitlement fields after wrap/growth");}
  }
  bool ordered=state.sdk_trace.size()==2*(state.receive_error_calls+state.send_error_calls);for(std::size_t i=0;i+1<state.sdk_trace.size();i+=2)ordered&=(state.sdk_trace[i]=='R'||state.sdk_trace[i]=='S')&&state.sdk_trace[i+1]=='E';
  require(ordered&&!state.sdk_pending&&state.receive_error_calls==state.connections[0]->receives+state.connections[1]->receives&&state.send_error_calls==18&&state.receive_success==1&&state.send_success==1,"All27 actual WinSock async imports have matching immediate error reads, including BOOLsuccess and pending997 paths");
  require(state.graph.lock()->snapshot().live_logics==0&&!state.graph.lock()->snapshot().event_locked,"Graph retires actual owned Logic rows after Profile clears every live session");
  require(policy->snapshot().exit_cleanups==18&&ports.view.memory.snapshot().controls_destroyed==2&&ports.view.memory.output_map_bindings().empty()&&!ports.view.io.snapshot().allocations&&!ports.view.scheduler.work_count_30,"Profile maps/controls and worker IO drain while legitimate Graph-owned event buffers remain until external attestation");state.worker_done=true;
 }
 bool external_quiescent(o::ServerView view,sp::Application& application){
  bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}require(outside&&state.worker_done&&state.shutdown_done&&!application.snapshot().live_jobs,"External hook checks observed inline job/session drain outside Root; worker counts alone are not attestation");
  unsigned relay=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* op=nullptr;const auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,op,0);const auto error=dispatcher.get_last_error();if(!got){if(error!=WAIT_TIMEOUT||op)unused();break;}if(op||bytes||key)unused();++relay;}
  require(relay==1&&state.gets==state.wait_scopes+state.initial_waits&&state.initial_waits==1&&!view.memory.snapshot().controls&&!view.io.snapshot().allocations,"Real stop relay/timeout observed outside Root after Profile reaps all owners; Graph event buffers are retained until this external proof, without operation lookup");state.queue_drained=true;return true;
 }
};
SOCKET WSAAPI open_socket(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){if(family!=2||type!=1||protocol!=6||info||group||flags!=1)unused();return 200+ ++running->factory_calls;}
BOOL WINAPI cancel_sdk(HANDLE,LPOVERLAPPED){unused();}
void move_executor(void* p,s::ExecutorFields& out,s::ExecutorFields& source,std::uintptr_t table){if(table!=0x180084928ULL)unused();++static_cast<State*>(p)->moves;out.context=source.context;out.zero_74=source.zero_74;out.self_reference_80=true;}
struct ExecutionPlatform:anadius::execution_recovered::Platform {unsigned locks=0,unlocks=0;std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}void acquire_srw_exclusive(std::uintptr_t&)override{++locks;}void release_srw_exclusive(std::uintptr_t&)override{++unlocks;}std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}};

// Source-only prepared/native fixture. Its actual native threads, heap/PTD/FLS
// and own IOCP are isolated in child test processes when explicitly authorized.
// All sockets/files/configuration/provider inputs are synthetic and owned.
struct NativeTrace {
 struct Launch {HANDLE original,joined,proof,barrier;DWORD id;bool original_closed=false,joined_closed=false,barrier_closed=false;};
 std::mutex gate;std::vector<Launch> launches;std::vector<std::pair<std::uint32_t,DWORD>> receipts;
 HANDLE barrier=nullptr;bool valid=true,post_denied=false,wait_denied=false,close_denied=false;unsigned post_calls=0,private_closes=0,barrier_closes=0;
 HANDLE exit_entered=nullptr,exit_release=nullptr;bool delay_first_exit=false,exit_released=false;
 bool grow_pending_rows=false,pending_growth_seen=false;
 HANDLE session_entered=nullptr,session_release=nullptr;
 bool hold_exit_until_close=false;
};
// Controller owns this pointer through actual original/child thread termination.
// The original clears running before its wrapper calls exit, so do not use it.
NativeTrace* exit_trace=nullptr;
DECLSPEC_NORETURN void WINAPI delayed_native_exit(DWORD code){
 try{auto* trace=exit_trace;bool delay=false;if(trace){std::lock_guard lock(trace->gate);delay=trace->delay_first_exit&&!trace->launches.empty()&&trace->launches.front().id==::GetCurrentThreadId();}
  if(delay){if(code||!::SetEvent(trace->exit_entered)||::WaitForSingleObject(trace->exit_release,trace->hold_exit_until_close?30000U:10000U)!=WAIT_OBJECT_0)::ExitProcess(245);}
 }catch(...){::ExitProcess(245);}::ExitThread(code);
}
HANDLE WINAPI native_event(LPSECURITY_ATTRIBUTES security,BOOL manual,BOOL initial,LPCWSTR name){
 try{auto& trace=*running->native_trace;auto handle=::CreateEventW(security,manual,initial,name);std::lock_guard lock(trace.gate);trace.valid&=!security&&manual&&!initial&&!name&&handle;trace.barrier=handle;return handle;}catch(...){::ExitProcess(246);}
}
BOOL WINAPI native_duplicate(HANDLE source_process,HANDLE source,HANDLE target_process,LPHANDLE out,DWORD access,BOOL inherit,DWORD flags){
 try{const auto result=::DuplicateHandle(source_process,source,target_process,out,access,inherit,flags);const auto error=::GetLastError();
  if(result){HANDLE proof=nullptr;if(!::DuplicateHandle(::GetCurrentProcess(),*out,::GetCurrentProcess(),&proof,0,FALSE,DUPLICATE_SAME_ACCESS))::ExitProcess(246);auto& trace=*running->native_trace;std::lock_guard lock(trace.gate);trace.valid&=out&&*out&&proof&&flags==DUPLICATE_SAME_ACCESS&&!access&&!inherit;trace.launches.push_back({source,*out,proof,trace.barrier,::GetThreadId(source)});}
  ::SetLastError(error);return result;
 }catch(...){::ExitProcess(246);}
}
BOOL WINAPI native_close(HANDLE handle){
 try{if(running->native_failure=="close"){auto& trace=*running->native_trace;std::lock_guard lock(trace.gate);for(const auto& row:trace.launches)if(row.joined==handle&&!row.joined_closed&&!trace.close_denied){trace.close_denied=true;::SetLastError(ERROR_INVALID_HANDLE);return FALSE;}}
  const auto result=::CloseHandle(handle);const auto error=::GetLastError();if(result){auto& trace=*running->native_trace;std::lock_guard lock(trace.gate);bool found=false;for(auto it=trace.launches.rbegin();it!=trace.launches.rend()&&!found;++it){if(it->original==handle&&!it->original_closed){it->original_closed=true;found=true;}else if(it->joined==handle&&!it->joined_closed){it->joined_closed=true;++trace.private_closes;found=true;}else if(it->barrier==handle&&!it->barrier_closed){it->barrier_closed=true;++trace.barrier_closes;found=true;}}trace.valid&=found;}::SetLastError(error);return result;}catch(...){::ExitProcess(246);}
}
DWORD WINAPI native_wait(HANDLE handle,DWORD timeout){
 try{if(timeout==0&&running->native_failure=="wait"){auto& trace=*running->native_trace;std::lock_guard lock(trace.gate);for(const auto& row:trace.launches)if(row.joined==handle&&!row.joined_closed&&!trace.wait_denied){trace.wait_denied=true;::SetLastError(ERROR_INVALID_HANDLE);return WAIT_FAILED;}}return ::WaitForSingleObject(handle,timeout);}catch(...){::ExitProcess(246);}
}
BOOL WINAPI native_post(HANDLE port,DWORD receipt,ULONG_PTR key,LPOVERLAPPED operation){
 try{auto& state=*running;{std::lock_guard lock(state.native_trace->gate);auto& trace=*state.native_trace;++trace.post_calls;trace.valid&=port==reinterpret_cast<HANDLE>(state.port)&&receipt&&key==s::prepared_job_completion_key&&!operation&&::GetCurrentThreadId()!=state.original_thread;trace.receipts.emplace_back(receipt,::GetCurrentThreadId());if(state.native_failure=="post"){trace.post_denied=true;::SetLastError(ERROR_INVALID_HANDLE);return FALSE;}}
  if(state.native_failure=="early-stop"||state.native_failure.starts_with("early-drain")||state.native_failure.starts_with("session-")){std::atomic_ref(state.view->scheduler.high_34).store(1);if(!::PostQueuedCompletionStatus(port,0,0,nullptr))::ExitProcess(246);}
  if(state.native_failure.starts_with("session-")){auto& trace=*state.native_trace;if(!::SetEvent(trace.session_entered)||::WaitForSingleObject(trace.session_release,10000)!=WAIT_OBJECT_0)::ExitProcess(244);}
  if(state.native_failure=="early-drain-timeout")return TRUE; // controlled proxy withholds the ready packet; not an observed kernel post
  const auto result=::PostQueuedCompletionStatus(port,receipt,key,operation);const auto error=::GetLastError();if(result&&state.native_failure=="duplicate")if(!::PostQueuedCompletionStatus(port,receipt,key,operation))::ExitProcess(246);::SetLastError(error);return result;
 }catch(...){::ExitProcess(246);}
}
void join_native_children(State& state){
 // This test driver joins independent proof handles OUTSIDE Parent. It makes
 // scheduling deterministic without replacing NativeJobs' own physical polls.
 auto& trace=*state.native_trace;
 if(trace.delay_first_exit&&!trace.hold_exit_until_close&&state.delayed_acknowledged&&!state.delayed_retention_checked){
  require(!state.view->memory.operation_on_this_thread(),"Delayed physical maintenance runs outside Parent");
  const auto work=state.view->scheduler.work_count_30.load();require(state.delayed_before_decrement_work>0&&work==state.delayed_before_decrement_work-1,"Receipt drops exactly one work pin before the next physical wait");
  require(!state.native_jobs->quiescent_prepared(*state.application),"Receipt consumption cannot reap a child blocked before ExitThread/FLS");
  const auto jobs=state.native_jobs->snapshot();
  require(jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.pending==1&&jobs.completed==1&&!trace.private_closes&&!trace.barrier_closes,"Live physical child retains capacity1 private handle/barrier after source completion");
  require(state.view->scheduler.work_count_30.load()==work,"Unsignaled physical maintenance cannot consume another source work pin");
  if(!trace.grow_pending_rows){if(!::SetEvent(trace.exit_release))::ExitProcess(245);std::lock_guard lock(trace.gate);trace.exit_released=true;}
  state.delayed_retention_checked=true;
 }
 std::vector<std::pair<HANDLE,DWORD>> handles;{std::lock_guard lock(trace.gate);for(std::size_t i=0;i<trace.launches.size();++i){if(trace.delay_first_exit&&!trace.exit_released&&i==0)continue;const auto& row=trace.launches[i];handles.emplace_back(row.proof,row.id);}}
 for(const auto& [handle,id]:handles){if(!id||::WaitForSingleObject(handle,10000)!=WAIT_OBJECT_0)::ExitProcess(247);DWORD code=STILL_ACTIVE;if(!::GetExitCodeThread(handle,&code)||code!=(state.native_failure=="post"?1U:0U))::ExitProcess(247);}
 if(trace.grow_pending_rows&&!trace.exit_released){
  HANDLE first_proof=nullptr;bool second_exited=false;
  {std::lock_guard lock(trace.gate);if(trace.launches.size()==2){
    const auto& first=trace.launches[0];const auto& second=trace.launches[1];
    require(first.original_closed&&second.original_closed&&!first.joined_closed&&!first.barrier_closed&&!second.joined_closed&&!second.barrier_closed&&first.id!=second.id&&trace.receipts.size()==2,
      "Growing NativeJobs retains first private handle/barrier and distinct second native actor until source receipts are consumed");
    first_proof=first.proof;second_exited=true;
  }}
  if(second_exited){
   require(!state.view->memory.operation_on_this_thread()&&state.delayed_acknowledged&&state.delayed_retention_checked,"Second child physical proof is observed only at the existing outside-Parent wait point");
   DWORD code=0;require(::WaitForSingleObject(first_proof,0)==WAIT_TIMEOUT&&::GetExitCodeThread(first_proof,&code)&&code==STILL_ACTIVE,"First consumed child remains physically held while the second grown row has completed its native/FLS exit");
   const auto jobs=state.native_jobs->snapshot();
   require(jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.pending==2&&jobs.started==2&&jobs.detached==2&&jobs.completed==1&&state.native_notices==1&&state.processed==2&&!trace.private_closes&&!trace.barrier_closes,
     "Initial NativeJobs1 grows to two retained physical rows while first ready receipt is consumed and second ready receipt is still queued");
   if(!::SetEvent(trace.exit_release))::ExitProcess(245);
   {std::lock_guard lock(trace.gate);trace.exit_released=true;trace.pending_growth_seen=true;}
  }
 }
 if(state.view)state.native_before_wait_work=state.view->scheduler.work_count_30.load();
 if(state.native_failure=="early-drain"&&state.drain_resume_posted&&!state.drain_counter_checked){require(state.drain_before_decrement_work&&state.native_before_wait_work==state.drain_before_decrement_work-1,"Deferred stop receipt still transfers exactly one scheduler pin before resumed stop");state.drain_counter_checked=true;}
 if(state.native_failure=="post"&&!handles.empty()){
  const auto jobs=state.native_jobs->snapshot();require(state.native_trace->post_denied&&state.native_trace->post_calls==1&&jobs.phase==anadius::owned_jobs::Phase::Poisoned&&jobs.pending==1&&jobs.started==1&&jobs.detached==1&&!jobs.completed,"Denied native ready post retains its row/work pin without retry or acknowledgement");
  require(state.processed==1&&state.application->snapshot().live_jobs==1&&state.view->memory.snapshot().controls==1&&state.native_before_wait_work>=2&&!state.native_notices&&!state.native_trace->private_closes&&!state.native_trace->barrier_closes,"Post-denial prefix keeps source closure/control/IO and physical handles despite child logical return and actual exit");std::_Exit(191);
 }
}
std::string native_body(unsigned index){switch(index%5){case 0:return "<GetProfile index=\"0\"/>";case 1:return "<GetAllGameInfo/>";case 2:return "<GrantAchievement AchievementId=\"one\" Progress=\"1\"/>";case 3:return "<GetAuthCode ClientId=\"owned-client\"/>";default:return "<QueryEntitlements Group=\"synthetic-group\"><FilterOffers>synthetic-offer</FilterOffers></QueryEntitlements>";}}
std::string_view native_response(unsigned index){static constexpr std::string_view names[]{"GetProfileResponse","GetAllGameInfoResponse","Achievement","AuthCode","QueryEntitlementsResponse"};return names[index%5];}
void native_receive(State& state,Connection& c,s::Buffer buffer,OVERLAPPED* ov){
 if(c.receives==1){try{(void)state.view->memory.buffer_context().memory.view(c.greeting_tail,1);}catch(const s::UnsupportedInput&){c.greeting_gone=true;}
  for(unsigned i=0;i<70;++i){auto text="<LSX><Request id=\"native-"+std::to_string(i)+"\" recipient=\"EbisuSDK\">"+native_body(i)+"</Request></LSX>";c.requests.push_back(text);c.expected_requests.push_back(text);}c.receive_wire=c.requests[0];c.receive_wire.push_back(0);state.native_delivered=1;
 }
 require(!c.receive_waiting,"Prepared source has one SDK receive loan at a time");c.receive_waiting=true;c.receive_buffer=buffer;c.receive_ov=ov;deliver_receive(state,c);
}
void native_observe(State& state,sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){
 auto& c=state.connection(view.owner.socket_state.socket);require(error.empty()&&view.token==c.token&&view.owner.identity()==c.owner_id&&::GetCurrentThreadId()!=state.original_thread,"Prepared Graph observer belongs to exact session on a distinct real child actor");
 if(kind==ag::Log::Received){require(text==c.expected_requests.at(c.processed),"Child033d70 observes exact Parent-backed request in sequence");(void)state.view->memory.buffer_context();++c.received_logs;}
 else if(kind==ag::Log::Response){
  const bool valid=text.find(native_response(c.processed))!=std::string_view::npos&&text.find("id=\"native-"+std::to_string(c.processed)+"\"")!=std::string_view::npos;
  if(!valid)std::cerr<<"Prepared response index="<<c.processed<<" expected="<<native_response(c.processed)<<" actual="<<text<<'\n'<<std::flush;
  require(valid,"Concrete child Router preserves response kind and request id before original-worker continuation");++c.response_logs;++c.processed;++state.processed;
 }
 else unused(); // plain peer and non-completing achievement emit no events
}
int WINAPI native_message(HWND,LPCWSTR,LPCWSTR,UINT){::ExitProcess(248);}
BOOL WINAPI session_cancel(HANDLE handle,LPOVERLAPPED operation){
 try{auto& state=*running;++state.session_cancel_calls;
  require(state.native_failure.starts_with("session-")&&state.view->memory.operation_on_this_thread(),"Prepared session cancellation executes only on original admitted worker");
  if(state.native_failure=="session-cancel-fail"){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
  auto& c=state.connection(201);
  if(operation==c.send_ov&&c.send_posted){require(handle==reinterpret_cast<HANDLE>(201),"Already-posted synthetic send retains exact source socket");::SetLastError(ERROR_NOT_FOUND);return FALSE;}
  const bool receive=operation==c.receive_ov&&c.receive_waiting;
  require((receive&&handle==reinterpret_cast<HANDLE>(201))||(operation==state.accept_packets.back()&&handle==reinterpret_cast<HANDLE>(200)),"Synthetic cancel targets exact pending accept/receive loan");
  if(receive){c.receive_waiting=false;if(state.native_failure=="session-missing-io")return TRUE;}
  state.session_cancel_packets.push_back(operation);if(!::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(state.port),0,0,operation))::ExitProcess(246);return TRUE;
 }catch(...){::ExitProcess(246);}
}
struct FixtureClock final:anadius::achievement_runtime::Clock {
 std::int64_t time_05eeb8()override{return 1577836800;}
 std::optional<anadius::achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t value)override{require(value==1577836800,"Prepared concrete providers use controlled own clock");return anadius::achievement_runtime::BrokenDown{0,0,0,1,0,120,3,0,0};}
};
struct FixtureFormatter final:anadius::achievement_runtime::Formatter {std::string put_time_03d610(const anadius::achievement_runtime::BrokenDown&,std::string_view)override{return "2020-01-01T00:00:00";}};
struct ConcreteProviders {
 State& state;anadius::bootstrap_backend::RuntimeSeed runtime{};anadius::achievement_storage::Paths paths{};anadius::license_recovered::AuthenticationCode authentication;
 std::shared_ptr<std::stringbuf> buffer=std::make_shared<std::stringbuf>();std::shared_ptr<std::ostream> stream=std::make_shared<std::ostream>(buffer.get());
 std::shared_ptr<anadius::library_effects::Dialog> dialog=std::make_shared<anadius::library_effects::Dialog>(anadius::library_effects::DialogApi{::MultiByteToWideChar,native_message});
 std::shared_ptr<b8_test::MemoryFilesState> files=std::make_shared<b8_test::MemoryFilesState>();
 std::unique_ptr<anadius::online_factory::Online> online;std::shared_ptr<anadius::information_effects::Information> information;std::shared_ptr<anadius::content_effects::Content> content;std::shared_ptr<anadius::licensing_effects::Handlers> licensing;
 ConcreteProviders(State& state,anadius::execution_recovered::Execution& execution,const anadius::bootstrap_backend::CallbackStateView* exact=nullptr):state(state){
  require(!exact||(&exact->configuration==&state.configuration&&&exact->execution==&execution),"Worker provider aliases borrow exact live bound configuration/Execution");
  state.configuration.fake_auth=1;state.configuration.username="prepared-user";state.configuration.achievements.emplace("one",anadius::configuration::Achievement{1000,0,0,"0000-00-00T00:00:00","own","steam-own",0});
  const auto view=exact?*exact:anadius::bootstrap_backend::CallbackStateView{state.configuration,runtime,execution};anadius::online_factory::Options o;o.stream=stream;o.dialog=dialog;o.os=std::make_unique<b12_test::Os>();o.files=std::make_unique<b8_test::MemoryFiles>(files);o.code_lease=[](auto){return std::shared_ptr<void>{};};o.foreign_quiescent=[]{return true;};online=std::make_unique<anadius::online_factory::Online>(view,std::move(o));
  auto clock=std::make_shared<FixtureClock>();auto formatter=std::make_shared<FixtureFormatter>();anadius::information_effects::Options i;i.stream=stream;i.clock=clock;i.formatter=formatter;information=std::make_shared<anadius::information_effects::Information>(i);
  anadius::content_effects::Options c;c.stream=stream;c.dialog=dialog;c.files=std::make_shared<b8_test::MemoryFiles>(files);c.clock=clock;c.formatter=formatter;content=std::make_shared<anadius::content_effects::Content>(view,paths,*online,c);
  anadius::licensing_effects::Options l;l.stream=stream;l.dialog=dialog;l.files=std::make_shared<b8_test::MemoryFiles>(files);l.clock=clock;l.formatter=formatter;licensing=std::make_shared<anadius::licensing_effects::Handlers>(view,*online,std::make_shared<anadius::licensing_effects::AttemptState>(anadius::licensing_effects::AttemptState{0,0}),l);licensing->bind_authentication_code(authentication);
 }
 ~ConcreteProviders(){licensing.reset();content.reset();information.reset();if(!online->close_after_cleanup())std::terminate();online.reset();state.provider_retired_thread=::GetCurrentThreadId();++state.worker_provider_retirements;}
};
struct NativeHooks {
 State& state;Dispatcher& dispatcher;cp::Policy* policy=nullptr;s::Api* errors=nullptr;
 s::IocpSchedulerEntry* completion_owner(sp::Application& application,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t){
  state.application=&application;auto ports=application.ports();if(!state.view_value){state.view_value.emplace(ports.view);state.view=&*state.view_value;}
  if(ports.accept.contains(&op)){++state.accept_completions;if(state.shutdown_posted&&&op.overlapped==state.accept_packets.back()){require(error.value==995&&!application.snapshot().live_sessions&&!application.snapshot().live_jobs,"Native fixture selects terminal pending accept only after source ownership drain");state.terminal_selected=true;return nullptr;}}
  else{const auto handle=ports.view.io.handle_for(&op);const auto kind=ports.view.io.kind(handle);if(kind==s::IoMemoryKind::Posted)++state.posted_packets;
   if(kind==s::IoMemoryKind::Send){++state.send_packets;auto& c=state.connection(201);require(c.send_waiting&&c.send_posted&&c.send_ov==&op.overlapped,"Prepared response completes its exact actual SDK send loan");c.send_waiting=c.send_posted=false;++c.completed_sends;}
   if(kind==s::IoMemoryKind::Receive)++state.receive_packets;
  }return owner;
 }
 void after_completion(sp::Application& application){
  auto ports=application.ports();if(state.terminal_selected){state.terminal_selected=false;state.shutdown_done=true;state.accept_pending=false;}
  if(state.native_failure.starts_with("session-"))return; // selected Stop owns terminal IO; do not feed request2
  auto& trace=*state.native_trace;
  if(trace.delay_first_exit&&!state.delayed_acknowledged&&state.native_jobs->snapshot().completed==1){
   if(::WaitForSingleObject(trace.exit_entered,10000)!=WAIT_OBJECT_0)::ExitProcess(245);
   HANDLE proof=nullptr;{std::lock_guard lock(trace.gate);require(trace.launches.size()==1&&trace.receipts.size()==1&&!trace.exit_released,"First real child reaches exit gate after one ready publication");proof=trace.launches.front().proof;}
   DWORD code=0;require(::WaitForSingleObject(proof,0)==WAIT_TIMEOUT&&::GetExitCodeThread(proof,&code)&&code==STILL_ACTIVE,"Acknowledged child is still physically live before actual ExitThread/FLS");
   require(state.native_notices==1&&!application.snapshot().live_jobs&&state.native_jobs->snapshot().pending==1&&state.processed==1,"Source handoff retires once while its physical NativeJobs row remains occupied");state.delayed_before_decrement_work=ports.view.scheduler.work_count_30.load();state.delayed_acknowledged=true;
  }
  auto& c=state.connection(201);
  if(c.completed_sends&&c.completed_sends==state.native_delivered&&c.completed_sends==c.processed&&c.receive_waiting&&c.cursor==c.receive_wire.size()&&!c.eof_posted){
   if(trace.grow_pending_rows&&c.completed_sends==1&&!trace.exit_released){
    const auto jobs=state.native_jobs->snapshot();
    require(state.delayed_acknowledged&&state.delayed_retention_checked&&jobs.pending==1&&jobs.completed==1&&!trace.pending_growth_seen,
      "Only the separate growth scenario feeds request2 while the first consumed physical row remains occupied");
   }else require(state.native_jobs->snapshot().pending==0,"Known physical/FLS reaping makes capacity1 reusable before the next native message");
   if(c.completed_sends<70){c.receive_wire+=c.requests[c.completed_sends];c.receive_wire.push_back(0);++state.native_delivered;}deliver_receive(state,c);
  }
  if(!application.snapshot().live_sessions&&state.processed==70&&!state.shutdown_posted){require(clear(application.registry())&&!application.snapshot().live_jobs&&state.native_jobs->snapshot().completed==70,"Seventy real child jobs drain all source registries before fixture stop");state.shutdown_posted=true;const auto h=ports.accept.handle_for(state.accept_packets.back());ports.accept.note_cancel_requested(h);for(auto& binding:ports.accept.dispatcher_bindings())if(binding.overlapped_identity==state.accept_packets.back())s::publish_send_status_028b50(ports.view.scheduler,*binding.operation,binding,995,0,ports.completion,*errors);}
 }
 void returned(sp::Application& application,std::uint64_t,const s::WorkerRunResult& result){
  auto ports=application.ports();const auto jobs=state.native_jobs->snapshot();auto& c=state.connection(201);
  if(state.native_failure.starts_with("session-")){
   const auto held=state.native_trace->hold_exit_until_close;
   if(held){if(::WaitForSingleObject(state.native_trace->exit_entered,10000)!=WAIT_OBJECT_0)::ExitProcess(245);}
   const auto stop=state.session_stop->snapshot();require(result.error&&!result.error->value&&state.native_notices==1&&state.processed==1&&jobs.started==1&&jobs.completed==1&&jobs.pending==(held?1U:0U)&&!application.snapshot().live_jobs&&!application.snapshot().live_sessions&&!ports.view.io.snapshot().allocations&&ports.view.memory.snapshot().controls_destroyed==1&&!ports.view.scheduler.work_count_30.load(),"Combined stop drains source job/IO while keeping physically live child row distinct");
   require(stop.phase==ast::Phase::Draining&&stop.kernel_packets>=2&&stop.acknowledged==stop.kernel_packets+stop.software_packets&&!stop.receipts,"Every selected late IO route is observed and acknowledged once before worker return");
   if(policy)require(policy->snapshot().exit_cleanups==2&&policy->snapshot().worker_initializations==1&&policy->snapshot().worker_finishes==1,"Direct Graph fixture observes exact Policy cleanup/init/finish counts");
   state.worker_done=true;state.worker_source_io_drained=true;return;
  }
  if(state.native_failure=="early-drain"){
   require(result.error&&!result.error->value&&jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.started==1&&jobs.detached==1&&jobs.completed==1&&!jobs.pending&&state.native_notices==1&&state.processed==1&&state.drain_counter_checked,"Selected early stop consumes exact ready receipt before original worker finish without body replay");
   require(!application.snapshot().live_jobs&&application.snapshot().live_sessions==1&&ports.view.memory.snapshot().controls==1&&ports.view.scheduler.work_count_30.load()&&state.native_trace->private_closes==1&&state.native_trace->barrier_closes==1&&policy->snapshot().exit_cleanups==2&&policy->snapshot().worker_initializations==1&&policy->snapshot().worker_finishes==1,"Receipt drain completes child/original source cleanup and physical reap while unrelated session IO remains explicitly owned");state.worker_done=true;return;
  }
  require(result.error&&!result.error->value&&state.shutdown_done&&state.native_notices==70&&c.received_logs==70&&c.response_logs==70&&c.completed_sends==70&&jobs.started==70&&jobs.detached==70&&jobs.completed==70&&!jobs.pending,"Prepared NativeJobs executes and acknowledges70 child bodies exactly once, with capacity1 physical reuse");
  require(!application.snapshot().live_jobs&&!application.snapshot().live_sessions&&clear(application.registry())&&!ports.view.io.snapshot().allocations&&ports.view.memory.snapshot().controls_destroyed==1&&!ports.view.scheduler.work_count_30.load(),"Real021770/response/control/IO/work cleanup completes before native worker return");
  std::size_t cursor=0;for(unsigned i=0;i<70;++i){const auto end=c.response_wire.find('\0',cursor);require(end!=std::string::npos,"Every original-worker SDK response has a native frame terminator");const auto frame=std::string_view(c.response_wire).substr(cursor,end-cursor);require(frame.find(native_response(i))!=std::string_view::npos&&frame.find("id=\"native-"+std::to_string(i)+"\"")!=std::string_view::npos,"Original continuation preserves exact native response FIFO ids without reprocessing");cursor=end+1;}require(cursor==c.response_wire.size()&&state.configuration.achievements.at("one").field08==14,"No duplicate responses or repeated Content achievement mutations");
  require(policy->snapshot().exit_cleanups==140&&policy->snapshot().worker_initializations==1&&policy->snapshot().worker_finishes==1,"Each current child04a47c and original02177004a47c runs once for distinct actor scopes");
  if(state.native_failure=="late-close"){state.close_wait=std::make_shared<anadius::services_completion::WaitTarget>(2,0);state.close_wait->lock();policy->exit_waits().register_current_thread(state.close_wait);require(policy->exit_waits().active()==1,"Owned original-thread registration deliberately defers final Policy close after NativeJobs closes");}state.worker_done=true;
 }
 bool external(o::ServerView view,sp::Application& application){
  require(!view.memory.operation_on_this_thread()&&state.worker_done&&!application.snapshot().live_jobs&&!state.native_jobs->snapshot().pending,"External attestation follows independently observed child FLS exit and source drain outside Parent");
  ++state.worker_external_proofs;
  if(state.native_failure=="early-drain"){++state.native_external_calls;return false;} // unrelated owned IO has no external drain proof
  ++state.native_external_calls;if(state.native_failure=="unknown-external")throw o::QuiescenceDeferred(); // an opaque exception is never a known false return
  unsigned relays=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* op=nullptr;const auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,op,0);const auto error=dispatcher.get_last_error();if(!got){require(error==WAIT_TIMEOUT&&!op,"Own native IOCP drains to actual timeout");break;}require(!bytes&&!key&&!op,"Only source stop relay remains after native receipt/IO drain");++relays;}require(relays==(state.queue_drained?0U:1U),"Native fixture consumes stop relay once and rechecks actual timeout on retry");state.queue_drained=true;return true;
 }
};
BOOL WINAPI own_native_module(DWORD flags,LPCWSTR address,HMODULE* module){if(flags!=4||!address||!module)::ExitProcess(249);*module=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
void run_prepared_native(std::shared_ptr<anadius::thread_domain::DomainStorage> domain,anadius::worker_handoff::PreparedMemory& prepared,std::shared_ptr<anadius::bootstrap_providers::NativeThreads> threads,std::shared_ptr<NativeTrace> trace,std::string failure,std::shared_ptr<ast::Stop> session_stop={}){
 auto borrow=prepared.acquire();auto state=std::make_shared<State>();state->native_mode=true;state->original_thread=::GetCurrentThreadId();state->native_trace=std::move(trace);state->native_failure=std::move(failure);running=state.get();
 state->session_stop=std::move(session_stop);
 state->configuration.entitlements.push_back({"synthetic-offer","synthetic-group","synthetic-version","synthetic-type","synthetic-tag"});
 std::uint32_t index=::TlsAlloc();require(index!=TLS_OUT_OF_INDEXES,"Prepared native fixture owns one original IOCP TLS selector");
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{},achievement_mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);anadius::execution_recovered::initialize_mutex_04a698(achievement_mutex,2);
 std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};state->ready=&ready;
 auto providers=std::make_unique<ConcreteProviders>(*state,execution);
 {
  auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto worker=std::make_shared<WorkerApi>(index,*dispatcher);auto hooks=std::make_shared<NativeHooks>(NativeHooks{*state,*dispatcher});
  anadius::owned_jobs::NativeJobApi api;api.create_event=native_event;api.duplicate=native_duplicate;api.close=native_close;api.wait=native_wait;api.post=native_post;state->native_jobs=std::make_shared<anadius::owned_jobs::NativeJobs>(threads,1,api);
  auto log_buffer=std::make_shared<std::stringbuf>();ag::Options options;options.state={state,&state->configuration,&state->online,&state->saved,&state->achievement_file};options.policy.stream=anadius::shared_log::make_stream(log_buffer);options.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,sync_send_sdk,select_sdk};options.policy.lifecycle.lifetime=hooks;
  options.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& app,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){return h->completion_owner(app,owner,op,error,bytes);};
  options.policy.lifecycle.after=[h=hooks.get()](sp::Application& app){h->after_completion(app);};options.policy.lifecycle.returned=[h=hooks.get()](sp::Application& app,std::uint64_t n,const s::WorkerRunResult& result){h->returned(app,n,result);};options.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView view,sp::Application& app){return h->external(view,app);};
  options.information=providers->information;options.content=providers->content;options.licensing=providers->licensing;options.time=std::make_shared<Time>();options.version="prepared-native";options.show_lsx=1;options.event_capacity=8;options.profile.sessions=1;options.profile.jobs_capacity=1;options.profile.headers=128;options.profile.initial_output_slots=8;options.profile.jobs=state->native_jobs;options.profile.receive={receive_sdk,receive_error_sdk};options.profile.send={send_sdk,send_error_sdk};
  options.prepare_session=[state](sp::SessionView view){prepare_peer(*state,view);};options.observe_log=[state](sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){observe(*state,view,kind,text,error);};
  auto graph=ag::Graph::create(std::move(options));graph->bind_achievement_mutex(achievement_mutex,platform);state->graph=graph;
  o::Dependencies server_deps;server_deps.state_lifetime=state;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(*state);server_deps.executor=std::make_shared<Executor>(*state);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
  rt::Dependencies runtime_deps;runtime_deps.state_lifetime=state;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.worker=worker;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.tls_index=&index;runtime_deps.executor={state.get(),move_executor};runtime_deps.capacity=32;
  if(state->native_failure.starts_with("early-drain"))runtime_deps.prepared_stop_timeout_ms=state->native_failure=="early-drain-timeout"?200U:5000U;
  if(state->session_stop){runtime_deps.accept_stop=state->session_stop;runtime_deps.prepared_stop_timeout_ms=state->native_failure=="session-missing-io"?200U:5000U;}
  anadius::bootstrap_backend::Resources resources{};graph->install_server_prepared(resources,domain,std::move(borrow),std::move(server_deps),std::move(runtime_deps));hooks->policy=graph->policy().get();hooks->errors=graph->policy()->errors().get();
  require(state->native_jobs->snapshot().phase==anadius::owned_jobs::Phase::Fresh&&state->native_trace->launches.empty(),"Prepared Graph installation publishes no native job/event before source start");
  std::uint16_t port=0;
  try{server::server_start_029ea0(*resources.server,port,ready,execution);}
  catch(const o::QuiescenceDeferred&){
   const auto backend=graph->backend().snapshot();
   require(backend.phase==o::Phase::Quiescing&&!backend.quiescence_attested&&backend.acceptor_initialized&&backend.pending_initialized&&!backend.active_calls&&!state->ports_closed&&!state->cleanups,"Known false drain retains live sockets/context and clears Graph/server call admission for retry");
   if(state->native_failure=="early-drain"){
    const auto jobs=state->native_jobs->snapshot();require(graph->snapshot().phase==ag::Phase::Bound&&graph->profile()->snapshot().phase==sp::Phase::Closing&&jobs.phase==anadius::owned_jobs::Phase::Bound&&!jobs.pending&&jobs.completed==1&&state->native_external_calls==1&&state->application->snapshot().live_sessions==1&&!state->application->snapshot().live_jobs&&state->drain_counter_checked,"Prepared receipt drain permits known later IO deferral; it cannot attest remaining session/SDK ownership");std::_Exit(197);
   }
   require(state->native_failure=="late-close"&&graph->snapshot().phase==ag::Phase::Drained&&graph->profile()->snapshot().phase==sp::Phase::Closing&&graph->policy()->snapshot().phase==cp::Phase::Bound&&state->native_jobs->snapshot().phase==anadius::owned_jobs::Phase::Closed&&state->native_external_calls==1&&state->close_wait&&state->close_wait->registered(),"Late Policy deferral preserves already-closed NativeJobs and Drained Graph for explicit pending-destruction retry");
   const auto before=state->native_jobs->snapshot();const auto private_closes=state->native_trace->private_closes,barrier_closes=state->native_trace->barrier_closes;
   // Resolve only our deliberately added registration on its actual owner.
   // Do not replay worker start, child processing, source04a47c or server_start.
   graph->policy()->exit_waits().cleanup_current_thread();require(!state->close_wait->registered()&&!state->close_wait->is_locked()&&state->close_wait->ready_value_quiescent()==1,"Actual original owner resolves only its deliberate close blocker");state->close_wait.reset();
   resources.server->destroy_pending_socket_024db0();resources.server->destroy_acceptor_024db0();resources.server->destroy_context();state->pending_close_retried=true;
   const auto after=state->native_jobs->snapshot();require(after.started==before.started&&after.completed==before.completed&&after.detached==before.detached&&state->native_trace->private_closes==private_closes&&state->native_trace->barrier_closes==barrier_closes&&state->native_external_calls==2&&graph->policy()->snapshot().exit_cleanups==140,"Close retry rechecks external proof without replaying jobs/04a47c or closing native handles twice");
  }catch(const std::exception& error){
   const auto jobs=state->native_jobs->snapshot();
   if(state->native_failure=="session-cancel-fail"||state->native_failure=="session-missing-io"){
    const bool missing=state->native_failure=="session-missing-io";const auto stop=state->session_stop->snapshot();
    require(graph->snapshot().phase==ag::Phase::Poisoned&&graph->backend().snapshot().phase==o::Phase::Poisoned&&!state->worker_done&&!state->ports_closed&&!state->cleanups&&!graph->policy()->snapshot().worker_finishes&&state->view->io.snapshot().pending&&state->view->memory.snapshot().controls==1,"Combined stop SDK/deadline failure retains source session/IO/backing without invented worker finish or port close");
    if(missing)require(stop.phase==ast::Phase::Draining&&stop.acknowledged>=1&&jobs.completed==1&&!state->application->snapshot().live_jobs&&state->native_notices==1,"Missing scripted IO completion remains pending even after prepared receipt and other observed packets retire");
    else require(stop.phase==ast::Phase::Poisoned&&stop.cancel_calls==1&&!stop.acknowledged&&!jobs.completed&&state->application->snapshot().live_jobs==1,"Cancel denial preserves every unobserved IO loan and prepared source claim");std::_Exit(missing?202:201);
   }
   if(state->native_failure=="early-stop"){
    require(graph->snapshot().phase==ag::Phase::Poisoned&&graph->backend().snapshot().phase==o::Phase::Poisoned&&graph->profile()->snapshot().phase==sp::Phase::Poisoned&&jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.pending==1&&!jobs.completed&&state->application->snapshot().live_jobs==1&&!state->worker_done&&!state->native_external_calls&&!state->native_trace->private_closes&&!state->native_trace->barrier_closes,"Default early stop is rejected by Profile before Policy worker_returned; unconsumed job ownership remains terminally retained");std::_Exit(195);
   }
   if(state->native_failure=="early-drain-timeout"||state->native_failure=="early-drain-resume-fail"){
    const bool timeout=state->native_failure=="early-drain-timeout";
    require(graph->snapshot().phase==ag::Phase::Poisoned&&graph->backend().snapshot().phase==o::Phase::Poisoned&&graph->profile()->snapshot().phase==sp::Phase::Open&&jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.pending==1&&jobs.started==1&&!state->worker_done&&!state->native_external_calls&&!state->native_trace->private_closes&&!state->native_trace->barrier_closes&&graph->policy()->snapshot().worker_active&&!graph->policy()->snapshot().worker_finishes,"Missing receipt/deferred-stop wake failure aborts current run without invented finish, replay, physical cleanup or quiescence");
    if(timeout)require(!jobs.completed&&!state->native_notices&&state->application->snapshot().live_jobs==1&&!state->drain_resume_posted,"Finite prepared stop deadline retains unacknowledged source work when controlled proxy withholds ready packet");
    else require(jobs.completed==1&&state->native_notices==1&&!state->application->snapshot().live_jobs&&state->resume_denied&&state->view->scheduler.work_count_30.load()==state->resume_failed_work,"Failed resumed-stop post retains dispatcher work pin after one consumed response; no second continuation/decrement");std::_Exit(timeout?198:199);
   }
   if(state->native_failure=="unknown-external"){
    require(graph->snapshot().phase==ag::Phase::Poisoned&&graph->backend().snapshot().phase==o::Phase::Poisoned&&graph->profile()->snapshot().phase==sp::Phase::Poisoned&&graph->policy()->snapshot().phase==cp::Phase::Poisoned&&jobs.phase==anadius::owned_jobs::Phase::Bound&&!jobs.pending&&jobs.completed==70&&state->native_external_calls==1&&!state->ports_closed&&!state->cleanups,"Opaque lower exception with deferral type still poisons every active layer and never permits cleanup retry");std::_Exit(196);
   }
   if(state->native_failure=="duplicate"||state->native_failure=="wait"||state->native_failure=="close"){
    require(jobs.phase==anadius::owned_jobs::Phase::Poisoned&&jobs.started==1&&jobs.detached==1&&jobs.completed==1&&graph->profile()->snapshot().phase==sp::Phase::Poisoned&&!state->application->snapshot().live_jobs&&state->view->scheduler.work_count_30.load()==state->native_before_wait_work,"Duplicate receipt or failed physical poll poisons without second body/work decrement");
    if(state->native_failure=="duplicate")require(state->native_notices==2&&state->native_trace->post_calls==1&&state->processed==1,"Duplicate is one child publication delivered twice, with no message replay");
    else require((state->native_failure=="wait"?state->native_trace->wait_denied:state->native_trace->close_denied)&&jobs.pending==1&&!state->native_trace->private_closes&&!state->native_trace->barrier_closes,"Failed physical wait/close retains private handle/barrier without speculative close or replay");
    std::_Exit(state->native_failure=="duplicate"?192:state->native_failure=="wait"?193:194);
   }
   std::cerr<<"Prepared Graph/native source failure: "<<error.what()<<'\n'<<std::flush;::ExitProcess(250);
  }
  if(state->native_failure=="session-drain"){
   const auto jobs=state->native_jobs->snapshot();require(jobs.phase==anadius::owned_jobs::Phase::Closed&&!jobs.pending&&jobs.started==1&&jobs.completed==1&&jobs.detached==1&&graph->snapshot().phase==ag::Phase::Closed&&graph->profile()->snapshot().phase==sp::Phase::Closed&&state->session_stop->snapshot().phase==ast::Phase::Closed&&state->queue_drained,"Selected prepared/SessionIo stop reaches closed Graph/Profile/NativeJobs/Stop after observed packets and independent external proof");
   require(state->session_cancel_packets.empty()&&state->session_cancel_calls>=2&&state->closed.size()==3&&state->closed.back()==200&&state->ports_closed==1&&state->cleanups==1&&state->native_trace->launches.size()==1&&state->native_trace->private_closes==1&&state->native_trace->barrier_closes==1,"Synthetic cancel receipt queue and all session/native/kernel owners retire once without replay");
  }else{
  require(state->native_failure.empty()||(state->native_failure=="late-close"&&state->pending_close_retried),"Injected failure must reach its retained prefix or explicit known close retry");const auto jobs=state->native_jobs->snapshot();
  require(port==12345&&ready==1&&!other&&!last&&state->starts==1&&state->cleanups==1&&state->ports_closed==1&&state->closed.size()==3&&state->closed.back()==200&&state->sdk_accepts==2&&state->address_reads==1,"Prepared source initializes, accepts one synthetic peer, drains terminal pending202 and retires owned kernel context once");
  require(jobs.phase==anadius::owned_jobs::Phase::Closed&&!jobs.pending&&jobs.started==70&&jobs.completed==70&&jobs.detached==70&&graph->snapshot().phase==ag::Phase::Closed&&graph->profile()->snapshot().phase==sp::Phase::Closed&&graph->policy()->snapshot().phase==cp::Phase::Closed&&state->queue_drained,"Actual selected native Jobs/Profile/Graph/Policy reach closed after physical and source drain");
  require(state->native_trace->valid&&state->native_trace->launches.size()==70&&state->native_trace->receipts.size()==70&&state->native_trace->private_closes==70&&state->native_trace->barrier_closes==70&&state->connections[0]->greeting_gone,"Seventy actual native starts/receipts and private handle/barrier retires reuse capacity1 beyond64 total jobs");
  require(!state->native_trace->delay_first_exit||(state->delayed_acknowledged&&state->delayed_retention_checked&&state->native_trace->exit_released),"Delayed first child completes receipt-before-exit retention then physical reaping before capacity reuse");
  require(!state->native_trace->grow_pending_rows||state->native_trace->pending_growth_seen,"Separate initial1 NativeJobs growth scenario observed two retained rows before releasing the first physical exit");
  for(const auto& row:state->native_trace->launches)require(row.original_closed&&row.joined_closed&&row.barrier_closed&&row.id!=state->original_thread,"Detach and physical close each consume exact native ownership on distinct child actors");
  require(providers->files->reads.empty()&&providers->files->writes.empty()&&providers->files->outputs.empty()&&providers->buffer->str().find("prepared-user")!=std::string::npos,"Concrete child provider path logs own profile and performs no real/synthetic file I/O in empty-path achievement scenario");
  }state->application=nullptr;
 }
 providers.reset();state->native_jobs.reset();require(::TlsFree(index)!=FALSE,"Original fixture TLS remains live until all concrete graph providers retire");running=nullptr;
}
void prepared_native_profile(std::string failure={},bool delay_exit=false,bool grow_pending_rows=false){
 namespace d=anadius::thread_domain;namespace wh=anadius::worker_handoff;
 auto identity=std::make_shared<std::array<std::byte,8>>();auto domain=d::DomainStorage::create({0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity});require(domain->initialize(),"Prepared native controller initializes real heap/FLS/PTD Domain");
 auto trace=std::make_shared<NativeTrace>();trace->launches.reserve(70);trace->receipts.reserve(70);trace->delay_first_exit=delay_exit;trace->grow_pending_rows=grow_pending_rows;
 std::shared_ptr<ast::Stop> session_stop;
 if(failure.starts_with("session-")){
  trace->session_entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr);trace->session_release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!trace->session_entered||!trace->session_release)::ExitProcess(244);
  ast::Options options;options.lifetime=trace;options.scope=ast::Scope::SessionIoPrepared;options.capacity=128;options.imports.cancel=session_cancel;
  options.other_work_drained=[]{return true;}; // explicit synthetic fixture assumption: no unmanaged raw/foreign work or new external admission
  session_stop=ast::Stop::create(std::move(options));
 }
 if(delay_exit){trace->exit_entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr);trace->exit_release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!trace->exit_entered||!trace->exit_release)::ExitProcess(245);exit_trace=trace.get();}
 auto api=anadius::thread_native::native_api();api.get_module=own_native_module;if(delay_exit)api.exit=delayed_native_exit;std::shared_ptr<anadius::bootstrap_providers::NativeThreads> threads=domain->make_threads(api);auto prepared=wh::PreparedMemory::prepare(domain);
 struct Original {
  std::function<void()> action;HANDLE handle=nullptr;std::uint32_t id=0;
  static unsigned __stdcall entry(void* p){auto& self=*static_cast<Original*>(p);try{self.action();}catch(const std::exception& error){std::cerr<<error.what()<<'\n'<<std::flush;::ExitProcess(251);}catch(...){::ExitProcess(251);}return 67;}
 } original;
 original.action=[&]{run_prepared_native(domain,*prepared,threads,trace,failure,session_stop);};original.handle=reinterpret_cast<HANDLE>(anadius::threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&Original::entry),reinterpret_cast<std::uintptr_t>(&original),0,&original.id,*threads));
 if(session_stop){if(::WaitForSingleObject(trace->session_entered,10000)!=WAIT_OBJECT_0||!session_stop->request()||!::SetEvent(trace->session_release))::ExitProcess(244);}
 if(!original.handle||!original.id||::WaitForSingleObject(original.handle,120000)!=WAIT_OBJECT_0)::ExitProcess(252);DWORD code=STILL_ACTIVE;require(::GetExitCodeThread(original.handle,&code)&&code==67,"Actual original wrapper returns after concrete server and FLS teardown");require(::CloseHandle(original.handle)!=FALSE,"Controller closes exact joined original handle");original.handle=nullptr;
 require(prepared->close(10000)&&prepared->snapshot().worker_exit_verified,"Prepared controller independently observes original and all child physical exits before backing release");
 for(const auto& row:trace->launches)require(::CloseHandle(row.proof)!=FALSE,"Independent test proof handle closes only after physical original/child drain");
 if(delay_exit){require(::CloseHandle(trace->exit_entered)&&::CloseHandle(trace->exit_release),"Controller closes owned exit gates after actual original/child termination");trace->exit_entered=trace->exit_release=nullptr;}exit_trace=nullptr;
 if(session_stop){require(session_stop->snapshot().phase==ast::Phase::Closed&&::CloseHandle(trace->session_entered)&&::CloseHandle(trace->session_release),"Controller retains Stop and receipt handshake until actual native/FLS join");session_stop.reset();trace->session_entered=trace->session_release=nullptr;}
 original.action={};prepared.reset();threads.reset();require(domain->provider_leases()==0&&domain->close(),"Controller retires final Domain only after borrowed backing and all thread providers are gone");domain.reset();
}

// Actual concrete Worker owns Graph/PreparedMemory/TLS and all source teardown.
// Provider aliases keep their owning Online/views alive until original-worker
// retirement; the controller deliberately owns NO ConcreteProviders instance.
void configure_delayed_worker(const std::shared_ptr<State>& state,const std::shared_ptr<anadius::bootstrap_providers::NativeThreads>& threads,anadius::bootstrap_backend::CallbackStateView view,aw::Inputs& input){
 state->original_thread=::GetCurrentThreadId();running=state.get();
 auto providers=std::make_shared<ConcreteProviders>(*state,view.execution,&view);
 auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto hooks=std::make_shared<NativeHooks>(NativeHooks{*state,*dispatcher});
 anadius::owned_jobs::NativeJobApi api;api.create_event=native_event;api.duplicate=native_duplicate;api.close=native_close;api.wait=native_wait;api.post=native_post;state->native_jobs=std::make_shared<anadius::owned_jobs::NativeJobs>(threads,1,api);
 auto& options=input.graph;options.policy.stream=anadius::shared_log::make_stream(std::make_shared<std::stringbuf>());options.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,sync_send_sdk,select_sdk};options.policy.lifecycle.lifetime=hooks;
 options.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& app,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){return h->completion_owner(app,owner,op,error,bytes);};
 options.policy.lifecycle.after=[h=hooks.get()](sp::Application& app){h->after_completion(app);};options.policy.lifecycle.returned=[h=hooks.get()](sp::Application& app,std::uint64_t count,const s::WorkerRunResult& result){h->returned(app,count,result);};options.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView view,sp::Application& app){return h->external(view,app);};
 options.information=std::shared_ptr<anadius::handlers_recovered::Effects>(providers,providers->information.get());
 options.content=std::shared_ptr<anadius::content_handlers_recovered::Effects>(providers,providers->content.get());
 options.licensing=std::shared_ptr<anadius::dispatch_recovered::LicenseHandlers>(providers,providers->licensing.get());
 options.time=std::make_shared<Time>();options.version="prepared-worker-delay";options.show_lsx=1;options.event_capacity=8;options.profile.sessions=1;options.profile.jobs_capacity=1;options.profile.headers=128;options.profile.initial_output_slots=8;options.profile.jobs=state->native_jobs;options.profile.receive={receive_sdk,receive_error_sdk};options.profile.send={send_sdk,send_error_sdk};
 options.prepare_session=[state](sp::SessionView session){prepare_peer(*state,session);};options.observe_log=[state](sp::SessionView session,ag::Log kind,std::string_view text,std::string_view error){observe(*state,session,kind,text,error);};
 auto& server=input.server;server.state_lifetime=state;server.sockets=sockets;server.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server.cancellation=std::make_shared<Cancel>(*state);server.executor=std::make_shared<Executor>(*state);server.kernel={startup,cleanup,local_name,environment,gate_delete};
 auto& runtime=input.runtime;runtime.state_lifetime=state;runtime.sockets=sockets;runtime.dispatcher=dispatcher;runtime.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime.executor={state.get(),move_executor};runtime.capacity=32;runtime.prepared_stop_timeout_ms=5000;
 // Protected graph.state and runtime.tls_index stay empty: Worker installs them.
}
void prepared_worker_session_delay(bool timeout=false){
 namespace td=anadius::thread_domain;namespace bb=anadius::bootstrap_backend;
 auto identity=std::make_shared<std::array<std::byte,8>>();auto domain=td::DomainStorage::create({0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity});require(domain->initialize(),"Controller initializes own Domain for concrete Worker/native child close continuation");
 auto state=std::make_shared<State>();state->native_mode=true;state->native_failure=timeout?"session-worker-timeout":"session-worker-delay";
 auto trace=std::make_shared<NativeTrace>();trace->launches.reserve(1);trace->receipts.reserve(1);trace->delay_first_exit=true;trace->hold_exit_until_close=true;state->native_trace=trace;
 trace->exit_entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr);trace->exit_release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);trace->session_entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr);trace->session_release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!trace->exit_entered||!trace->exit_release||!trace->session_entered||!trace->session_release)::ExitProcess(244);exit_trace=trace.get();
 auto thread_api=anadius::thread_native::native_api();thread_api.get_module=own_native_module;thread_api.exit=delayed_native_exit;std::shared_ptr<anadius::bootstrap_providers::NativeThreads> threads=domain->make_threads(thread_api);
 ast::Options stop_options;stop_options.lifetime=trace;stop_options.scope=ast::Scope::SessionIoPrepared;stop_options.capacity=128;stop_options.imports.cancel=session_cancel;stop_options.other_work_drained=[]{return true;}; // closed fixture has no unmanaged work
 auto stop=ast::Stop::create(std::move(stop_options));state->session_stop=stop;
 bb::RuntimeSeed seed{};ExecutionPlatform platform;anadius::execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);anadius::execution_recovered::Execution execution{seed.execution_mutex,{seed.server_ready,seed.online_ready,seed.libraries_ready},seed.frequency_cache,platform};state->ready=&seed.server_ready;
 const bb::CallbackStateView view{state->configuration,seed,execution};aw::Settings settings;settings.lifetime=state;settings.state=aw::Settings::StateSeed{0,{}};settings.accept_stop=stop;settings.close_retry_timeout_ms=timeout?2000U:5000U;settings.configure=[state,threads,view](aw::Inputs& input){configure_delayed_worker(state,threads,view,input);};
 auto worker=aw::Worker::create(domain,view,aw::take_settings(settings));anadius::execution_recovered::MutexState achievement_mutex{};anadius::execution_recovered::initialize_mutex_04a698(achievement_mutex,2);worker->bind_achievement_mutex(achievement_mutex,platform);
 struct Original {std::function<void()> action;HANDLE handle=nullptr;std::uint32_t id=0;static unsigned __stdcall entry(void* raw){auto& self=*static_cast<Original*>(raw);self.action();return 67;}} original;
 original.action=[&]{worker->run_server(state->configuration.server_port,seed.server_ready,execution);};original.handle=reinterpret_cast<HANDLE>(anadius::threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&Original::entry),reinterpret_cast<std::uintptr_t>(&original),0,&original.id,*threads));if(!original.handle||!original.id)::ExitProcess(244);
 auto wait_phase=[&](aw::Phase phase,std::uint64_t epoch=0){for(unsigned i=0;i<5000;++i){const auto snap=worker->snapshot();if(snap.phase==phase&&(!epoch||snap.close_epoch==epoch))return snap;if(snap.phase==aw::Phase::Poisoned)break;::Sleep(1);}::ExitProcess(243);};
 (void)wait_phase(aw::Phase::WaitingStartup);worker->startup_complete(anadius::execution_recovered::ReadyStage::Online,&execution);worker->startup_complete(anadius::execution_recovered::ReadyStage::Host,&execution);
 if(::WaitForSingleObject(trace->session_entered,10000)!=WAIT_OBJECT_0||!stop->request()||!::SetEvent(trace->session_release))::ExitProcess(244);
 const auto first=wait_phase(aw::Phase::WaitingClose,1);HANDLE proof=nullptr;{std::lock_guard lock(trace->gate);require(trace->launches.size()==1&&!trace->private_closes&&!trace->barrier_closes,"Physical native ownership remains retained after source and IO drain");proof=trace->launches.front().proof;}
 const auto jobs=state->native_jobs->snapshot();DWORD code=0;
 require(first.pending_retained&&first.memory.phase==anadius::worker_handoff::Phase::Borrowed&&state->worker_source_io_drained&&jobs.phase==anadius::owned_jobs::Phase::Bound&&jobs.pending==1&&jobs.completed==1&&::WaitForSingleObject(proof,0)==WAIT_TIMEOUT&&::GetExitCodeThread(proof,&code)&&code==STILL_ACTIVE&&!state->worker_external_proofs&&!state->worker_provider_retirements,"Controller sees logical job/IO completion with real live child; no external proof/provider retirement can bypass physical drain");
 require(!worker->finish_after_join()&&!worker->cancel_unpublished(),"Controller cannot close prepared backing while child and original close waiter are live");
 if(!timeout){
  bb::ApplicationJoinContinuation& continuation=*worker;require(!continuation.prepare_join(),"Production controller seam only signals current close epoch without claiming native join");
  const auto second=wait_phase(aw::Phase::WaitingClose,2);require(second.close_retries==1&&state->native_jobs->snapshot().pending==1&&!state->worker_external_proofs&&!state->worker_provider_retirements&&::WaitForSingleObject(original.handle,0)==WAIT_TIMEOUT,"A retry before physical exit preserves all owners and publishes a new wait epoch");
  if(!::SetEvent(trace->exit_release))::ExitProcess(245);{std::lock_guard lock(trace->gate);trace->exit_released=true;}
  if(::WaitForSingleObject(proof,10000)!=WAIT_OBJECT_0||!::GetExitCodeThread(proof,&code)||code)::ExitProcess(247);
  require(!continuation.prepare_join(),"After independent child/FLS join, controller requests original-thread physical reap and close continuation");
 }
 if(::WaitForSingleObject(original.handle,10000)!=WAIT_OBJECT_0||!::GetExitCodeThread(original.handle,&code)||code!=67)::ExitProcess(252);require(::CloseHandle(original.handle)!=FALSE,"Actual original wrapper/FLS join precedes controller finish");original.handle=nullptr;
 const auto final=worker->snapshot();
 if(timeout){require(final.phase==aw::Phase::Poisoned&&final.pending_retained&&final.memory.phase==anadius::worker_handoff::Phase::Borrowed&&state->native_jobs->snapshot().pending==1&&!state->worker_external_proofs&&!state->worker_provider_retirements&&!worker->prepare_join()&&!worker->finish_after_join()&&::WaitForSingleObject(proof,0)==WAIT_TIMEOUT,"Close-wait deadline retains providers/borrow/physical child even after original exits; no controller destruction fallback");std::_Exit(203);}
 require(final.phase==aw::Phase::Returned&&!final.pending_retained&&final.close_epoch==2&&final.close_retries==2&&state->native_jobs->snapshot().phase==anadius::owned_jobs::Phase::Closed&&state->worker_external_proofs==1&&state->worker_provider_retirements==1&&state->provider_retired_thread==original.id&&stop->snapshot().phase==ast::Phase::Closed,"Original actor reaps child, rechecks external proof and retires providers/Graph/TLS once before returning");
 require(worker->finish_after_join()&&worker->snapshot().memory.worker_exit_verified,"Controller releases prepared backing only after actual original and child termination");
 original.action={};worker.reset();state->application=nullptr;running=nullptr;exit_trace=nullptr;state->native_jobs.reset();state->session_stop.reset();stop.reset();
 require(::CloseHandle(proof)&&::CloseHandle(trace->exit_entered)&&::CloseHandle(trace->exit_release)&&::CloseHandle(trace->session_entered)&&::CloseHandle(trace->session_release),"Controller closes owned independent proof/events after complete worker/child retirement");state.reset();threads.reset();require(domain->provider_leases()==0&&domain->close(),"Last native provider/Domain retires only after all concrete Worker lifetimes drain");domain.reset();
}
struct SelectedJobCounts {unsigned starts=0,detaches=0;};
// Explicit synthetic same-worker tickets for binding coverage; no native jobs.
struct SelectedJobs final:sp::Jobs {
 std::shared_ptr<SelectedJobCounts> counts;
 explicit SelectedJobs(std::shared_ptr<SelectedJobCounts> p):counts(std::move(p)){}
 s::FrameThreadHandle start(sp::JobToken,std::function<void()> runner)override{
  const auto id=++counts->starts;runner();runner={};return {0x90000+id,id};
 }
 std::int32_t detach(const s::FrameThreadHandle& handle)override{
  require(handle.id&&handle.handle==0x90000+handle.id,"Explicit selected Jobs receives its matching source ticket at detach");++counts->detaches;return 0;
 }
};
void concrete_profile(bool use_selected_jobs=false){
 auto selected_counts=std::make_shared<SelectedJobCounts>();std::weak_ptr<SelectedJobs> selected_lifetime;
 NativeDomain native;auto state=std::make_shared<State>();running=state.get();auto* observed=state.get();std::weak_ptr<State> lifetime=state;state->configuration.entitlements.push_back({"synthetic-offer","synthetic-group","synthetic-version","synthetic-type","synthetic-tag"});
 std::uint32_t index=::TlsAlloc();if(index==TLS_OUT_OF_INDEXES)unused();std::weak_ptr<ag::Graph> graph_lifetime;std::weak_ptr<Hooks> weak_hooks;
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto worker=std::make_shared<WorkerApi>(index,*dispatcher);auto hooks=std::make_shared<Hooks>(*state,*dispatcher);weak_hooks=hooks;
  auto log_buffer=std::make_shared<std::stringbuf>();auto log_stream=anadius::shared_log::make_stream(log_buffer);ag::Options options;options.state={state,&state->configuration,&state->online,&state->saved,&state->achievement_file};options.policy.stream=log_stream;options.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,sync_send_sdk,select_sdk};options.policy.lifecycle.lifetime=hooks;
  options.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& a,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){return h->completion_owner(a,owner,op,error,bytes);};
  options.policy.lifecycle.after=[h=hooks.get()](sp::Application& a){h->after_completion(a);};options.policy.lifecycle.returned=[h=hooks.get()](sp::Application& a,std::uint64_t n,const s::WorkerRunResult& r){h->worker_returned(a,n,r);};options.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView v,sp::Application& a){return h->external_quiescent(v,a);};
  options.information=std::make_shared<Information>();options.content=std::make_shared<ContentEffects>();options.licensing=std::make_shared<LicenseBoundary>();options.time=std::make_shared<Time>();options.version=std::string(70000,'a');options.show_lsx=1;options.event_capacity=8;
   options.profile.receive={receive_sdk,receive_error_sdk};options.profile.send={send_sdk,send_error_sdk};options.profile.sessions=1;options.profile.jobs_capacity=4;options.profile.headers=128;options.profile.initial_output_slots=8;
  if(use_selected_jobs){auto jobs=std::make_shared<SelectedJobs>(selected_counts);selected_lifetime=jobs;options.profile.jobs=std::move(jobs);}
  options.prepare_session=[keep=state](sp::SessionView view){prepare_peer(*keep,view);};options.observe_log=[keep=state](sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){observe(*keep,view,kind,text,error);};
  auto graph=ag::Graph::create(std::move(options));graph_lifetime=graph;state->graph=graph;
  o::Dependencies server_deps;server_deps.state_lifetime=state;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(*state);server_deps.executor=std::make_shared<Executor>(*state);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
  rt::Dependencies runtime_deps;runtime_deps.state_lifetime=state;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.worker=worker;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.tls_index=&index;runtime_deps.executor={observed,move_executor};runtime_deps.capacity=1;
  anadius::bootstrap_backend::Resources resources{};graph->install_server(resources,native.root,std::move(server_deps),std::move(runtime_deps));hooks->policy=graph->policy().get();hooks->errors=graph->policy()->errors().get();
  require(resources.server&&graph->profile()->snapshot().phase==sp::Phase::Fresh&&!observed->starts&&!observed->factory_calls&&graph->policy()->snapshot().worker_initializations==0&&graph->policy()->snapshot().exit_cleanups==0,"Concrete Graph installs Resources.server passively with NOcaller XML/Logic/Router/Random/event/context/Policy graph");
  sockets.reset();dispatcher.reset();worker.reset();hooks.reset();graph.reset();state.reset();require(!lifetime.expired()&&!graph_lifetime.expired(),"Server wrapper retains concrete Graph and all required SDK/state/lifecycle lifetimes after external owners drop");
  require(!selected_counts->starts&&!selected_counts->detaches&&(!use_selected_jobs||!selected_lifetime.expired()),"Explicit Jobs is retained by Graph/Profile without executing work during installation");
  std::uint16_t port=0;
  try{server::server_start_029ea0(*resources.server,port,ready,execution);}catch(const std::exception& e){std::cerr<<"SOURCE EXCEPTION: "<<e.what()<<"\n"<<std::flush;::ExitProcess(111);}
  for(const auto& c:observed->connections)require(c->greeting_gone&&c->sync_calls==3&&c->max_sync==65536&&c->greeting_wire.find(std::string(70000,'a'))!=std::string::npos&&c->greeting_wire.find("<Challenge")!=std::string::npos&&c->greeting_wire.back()==0&&c->greeting_wire.size()>65536,"Concrete Profile builds/owns large actualMessageHandler greeting and native025720 frees it before per-session receive");
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1&&observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Actual SourceBackend server component publishes ready/port through existing native source path");
  require(observed->closed.size()==4&&observed->closed.back()==200&&std::count(observed->closed.begin(),observed->closed.end(),201)==1&&std::count(observed->closed.begin(),observed->closed.end(),202)==1&&std::count(observed->closed.begin(),observed->closed.end(),203)==1&&observed->moves==4&&observed->ports_closed==1&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1,"Two actual023000 factories and both native disposals close201/202 plus terminal203/listener exactly once before own port/gate/WSA teardown");
  const auto final=graph_lifetime.lock()->backend().snapshot();require(final.phase==o::Phase::Closed&&final.quiescence_attested&&observed->queue_drained&&!native.root->provider_leases()&&graph_lifetime.lock()->profile()->snapshot().phase==sp::Phase::Closed&&graph_lifetime.lock()->snapshot().phase==ag::Phase::Closed,"Runtime and concrete Profile retire owned graphs only after explicit external drain plus actual source counts");const auto policy_final=graph_lifetime.lock()->policy()->snapshot();require(policy_final.phase==cp::Phase::Closed&&!policy_final.active&&!policy_final.worker_active&&policy_final.worker_initializations==1&&policy_final.worker_finishes==1&&policy_final.exit_cleanups==18,"Concrete policy closes dedicated exit registry after one real native worker initialize/finish and18 inline job cleanups");require(graph_lifetime.lock()->policy()->exit_waits().active()==0&&graph_lifetime.lock()->policy()->exit_waits().blocks()==1&&!weak_hooks.expired(),"Dedicated selected exit registry has zero registrations and only its fixed inline head while lifecycle hooks remain retained through closed graph");observed->application=nullptr;
  require(use_selected_jobs?(selected_counts->starts==18&&selected_counts->detaches==18):(!selected_counts->starts&&!selected_counts->detaches),"Explicit Jobs handles all18 real source message starts/detaches; empty selection preserves default backend");
 }
 require(selected_lifetime.expired(),"Explicit Jobs backing retires with closed Graph/Profile owners");
 require(lifetime.expired()&&graph_lifetime.expired()&&weak_hooks.expired(),"Closed server/Runtime/Profile/Policy destruction releases all captured per-session logic/SDK/lifecycle hooks without ownership cycle");running=nullptr;require(::TlsFree(index)!=FALSE,"Borrowed TLS selector remains valid until complete concrete graph destruction");require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Real process heap/PTD/FLS domain drains every Profile frame/string/map/block/control/IO allocation");
}
}
int main(int argc,char** argv){try{
 if(argc==2&&std::string_view(argv[1])=="--prepared-native-growth"){prepared_native_profile({},true,true);return 0;}
 if(argc==2){const std::string_view mode=argv[1];if(mode=="--prepared-worker-session-delay"){prepared_worker_session_delay();return 0;}if(mode=="--prepared-worker-session-timeout"){prepared_worker_session_delay(true);return 1;}if(mode=="--prepared-native"){prepared_native_profile();return 0;}if(mode=="--prepared-native-delayed-exit"){prepared_native_profile({},true);return 0;}if(mode=="--prepared-native-late-close"){prepared_native_profile("late-close");return 0;}if(mode=="--prepared-native-session-drain"){prepared_native_profile("session-drain");return 0;}if(mode=="--prepared-native-session-cancel-fail"){prepared_native_profile("session-cancel-fail");return 1;}if(mode=="--prepared-native-session-missing-io"){prepared_native_profile("session-missing-io");return 1;}if(mode=="--prepared-native-early-stop"){prepared_native_profile("early-stop");return 1;}if(mode=="--prepared-native-early-drain"){prepared_native_profile("early-drain");return 1;}if(mode=="--prepared-native-early-drain-timeout"){prepared_native_profile("early-drain-timeout");return 1;}if(mode=="--prepared-native-early-drain-resume-fail"){prepared_native_profile("early-drain-resume-fail");return 1;}if(mode=="--prepared-native-unknown-external"){prepared_native_profile("unknown-external");return 1;}if(mode=="--prepared-native-post"){prepared_native_profile("post");return 1;}if(mode=="--prepared-native-duplicate"){prepared_native_profile("duplicate");return 1;}if(mode=="--prepared-native-wait"){prepared_native_profile("wait");return 1;}if(mode=="--prepared-native-close"){prepared_native_profile("close");return 1;}return 2;}
 concrete_profile();concrete_profile(true);
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native",static_cast<const char*>(nullptr))==0,"Own isolated source scenario covers prepared Graph/Profile/NativeJobs70messages with actual child/FLS exit and capacity1 reuse");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-delayed-exit",static_cast<const char*>(nullptr))==0,"Own isolated delayed-exit source scenario retains physical ownership after receipt consumption before actual ExitThread/FLS");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-growth",static_cast<const char*>(nullptr))==0,"Own isolated initial1 NativeJobs scenario grows while first consumed child is physically held, then drains the same seventy native receipts/FLS exits exactly once");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-late-close",static_cast<const char*>(nullptr))==0,"Own isolated late Policy-close deferral retries pending teardown after NativeJobs closed without replay");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-early-stop",static_cast<const char*>(nullptr))==195,"Own isolated explicit early stop retains unconsumed ready receipt and source/native ownership");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-session-drain",static_cast<const char*>(nullptr))==0,"Own combined prepared/SessionIo source scenario closes all tracked job/IO owners after observed packets and actual thread joins");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-worker-session-delay",static_cast<const char*>(nullptr))==0,"Own concrete Worker/native child source scenario spans IO drain, live physical child, two controller close epochs and original provider retirement");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-worker-session-timeout",static_cast<const char*>(nullptr))==203,"Own concrete Worker close deadline retains providers/borrow and live child without controller cleanup fallback");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-session-cancel-fail",static_cast<const char*>(nullptr))==201,"Own combined cancel-denial prefix retains every unobserved loan without cleanup");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-session-missing-io",static_cast<const char*>(nullptr))==202,"Own combined missing-IO receipt deadline retains source/kernel ownership after prepared body completes");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-early-drain",static_cast<const char*>(nullptr))==197,"Own selected early-stop receipt drain consumes job once, resumes stop and retains unrelated IO without attestation");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-early-drain-timeout",static_cast<const char*>(nullptr))==198,"Own selected finite receipt deadline retains work if controlled proxy withholds ready packet");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-early-drain-resume-fail",static_cast<const char*>(nullptr))==199,"Own selected resumed-stop post failure retains untransferred dispatcher work pin without replay");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-unknown-external",static_cast<const char*>(nullptr))==196,"Own isolated opaque exception using deferral type remains poisoned without cleanup retry");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-post",static_cast<const char*>(nullptr))==191,"Own isolated post-denial source prefix retains job/control/work/handles without retry");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-duplicate",static_cast<const char*>(nullptr))==192,"Own isolated duplicate-notice source prefix never repeats body/work decrement");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-wait",static_cast<const char*>(nullptr))==193,"Own isolated failed-physical-wait source prefix retains joined handle/barrier");
 require(_spawnl(_P_WAIT,argv[0],argv[0],"--prepared-native-close",static_cast<const char*>(nullptr))==194,"Own isolated failed-physical-close source prefix retains joined handle/barrier without replay");
 std::cout<<"PASS: "<<checks<<" application-graph integration source checks; prepared native normal/late-close, early-stop receipt drain and retained SDK/deadline prefixes; no original nativeABI claim\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected graph integration exception\n";return 1;}}
