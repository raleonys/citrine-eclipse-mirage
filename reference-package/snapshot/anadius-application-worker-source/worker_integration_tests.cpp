#include <winsock2.h>
#include "worker.hpp"
#include "../anadius-bootstrap-runtime-source/factory.hpp"
#include "../anadius-bootstrap-owner-source/owner_test_fixture.hpp"
#include "../anadius-application-graph-source/graph.hpp"
#include <atomic>
#include <mutex>
#include <map>
#include <sstream>
#include "../anadius-native-profile-source/native.hpp"
#include "../anadius-server-runtime-source/resources.hpp"
#include <optional>
#include "../anadius-socket-source/response_containers.hpp"
#include "../anadius-owned-server-source/server.hpp"
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
namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;namespace sp=anadius::session_profile;namespace np=anadius::native_profile;namespace cp=anadius::session_policy;namespace ag=anadius::application_graph;namespace wh=anadius::worker_handoff;namespace td=anadius::thread_domain;namespace t=anadius::threads_recovered;namespace aw=anadius::application_worker;namespace bb=anadius::bootstrap_backend;namespace br=anadius::bootstrap_runtime;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected owned-server integration dependency");}
// Own SDK observation is synchronized across controller, worker and its actual
// FLS callback; no legacy fixture globals/maps are accessed across threads.
struct HeapObservation {
 mutable std::mutex gate;std::map<void*,SIZE_T> blocks;unsigned allocations=0,frees=0;std::atomic<unsigned> slot_allocations=0,slot_releases=0;bool valid=true;std::vector<DWORD> allocation_threads,free_threads;
 std::size_t count()const{std::lock_guard lock(gate);return blocks.size();}
 bool balanced()const{std::lock_guard lock(gate);return valid&&blocks.empty()&&allocations==frees;}
 bool used_worker(DWORD id)const{std::lock_guard lock(gate);return std::find(allocation_threads.begin(),allocation_threads.end(),id)!=allocation_threads.end()&&std::find(free_threads.begin(),free_threads.end(),id)!=free_threads.end();}
};
HeapObservation* heap_observation=nullptr;
void* WINAPI heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){
 auto& state=*heap_observation;std::lock_guard lock(state.gate);if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))state.valid=false;
 auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw){if(!state.blocks.emplace(raw,bytes).second)state.valid=false;++state.allocations;state.allocation_threads.push_back(::GetCurrentThreadId());}::SetLastError(error);return raw;
}
BOOL WINAPI heap_free(HANDLE heap,DWORD flags,LPVOID value){
 auto& state=*heap_observation;std::lock_guard lock(state.gate);if(heap!=::GetProcessHeap()||flags||!state.blocks.contains(value))state.valid=false;
 const auto result=::HeapFree(heap,flags,value);const auto error=::GetLastError();if(result){if(state.blocks.erase(value)!=1)state.valid=false;++state.frees;state.free_threads.push_back(::GetCurrentThreadId());}::SetLastError(error);return result;
}
DWORD WINAPI allocate_fls(PFLS_CALLBACK_FUNCTION callback){++heap_observation->slot_allocations;return ::FlsAlloc(callback);}
BOOL WINAPI release_fls(DWORD index){++heap_observation->slot_releases;return ::FlsFree(index);}
HMODULE WINAPI unused_load(LPCWSTR,HANDLE,DWORD){unused();}FARPROC WINAPI unused_procedure(HMODULE,LPCSTR){unused();}BOOL WINAPI unused_release(HMODULE){unused();}std::uint32_t unused_flags(){unused();}
struct NativeDomain {
 HeapObservation memory;std::shared_ptr<td::DomainStorage> root;
 NativeDomain(){if(heap_observation)unused();heap_observation=&memory;auto identity=std::make_shared<std::array<std::byte,64>>();td::Seed seed{0x5a1b2c3d4e5f6071ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity.get()),identity};td::Apis apis;apis.heap.heap_alloc=heap_allocate;apis.heap.heap_free=heap_free;apis.fls.allocate=allocate_fls;apis.fls.release=release_fls;apis.runtime={unused_load,unused_procedure,unused_release,::GetLastError,unused_flags};root=td::DomainStorage::create(seed,apis);if(!root->initialize())unused();}
 ~NativeDomain(){if(root){try{if(!root->close())std::terminate();}catch(...){std::terminate();}root.reset();}heap_observation=nullptr;}
};
struct Connection {
 unsigned received_logs=0,response_logs=0,event_logs=0;bool live_request_seen=false,auto_decrypted=false;std::string expected_response;std::string expected_request;std::vector<std::string> requests,expected_requests;std::string receive_wire;std::size_t cursor=0;bool receive_waiting=false,eof_posted=false;s::Buffer receive_buffer{};OVERLAPPED* receive_ov=nullptr;bool send_waiting=false,send_posted=false;OVERLAPPED* send_ov=nullptr;std::uint32_t send_bytes=0;unsigned completed_sends=0;
 s::Socket socket=0;sp::SessionToken token{};std::uintptr_t owner_id=0,greeting_data=0,greeting_tail=0;bool encrypted=false,greeting_gone=false,eof_seen=false,logic_destroyed=false;unsigned receives=0,sends=0,sync_calls=0,processed=0;std::uint32_t max_sync=0;s::SessionCipher client;std::string greeting_wire,response_wire,event_text;
};
struct State {
 const td::DomainStorage* expected_domain=nullptr;DWORD expected_worker=0;
 anadius::configuration::State* bound_config=nullptr;bb::RuntimeSeed* bound_runtime=nullptr;anadius::execution_recovered::Execution* bound_execution=nullptr;std::atomic<aw::Worker*> component=nullptr;HANDLE armed=nullptr,drain_ready=nullptr;std::atomic<bool> shutdown_posted=false,stop_ready=false;std::atomic<bool> host_barrier=false,online_barrier=false,source_drained=false;std::atomic<unsigned> physical_gets=0;unsigned profile_logs=0;
 unsigned sdk_pending=0,sdk_expected_error=0,receive_error_calls=0,send_error_calls=0,receive_success=0,send_success=0;std::vector<char> sdk_trace;
 unsigned receive_packets=0,zero_receive_packets=0,send_packets=0,posted_packets=0,accept_completions=0;
 unsigned plain_stage=0;bool flush_started=false,wrap_seen=false,growth_seen=false,backlog_seen=false;std::uintptr_t old_map=0,new_map=0,current_owner=0;
 bool ipv6=false,accept_pending=false,shutdown_done=false,terminal_selected=false,overlap_seen=false,cleared_seen=false,worker_done=false,queue_drained=false;
 std::uint8_t* ready=nullptr;o::ServerView* view=nullptr;std::optional<o::ServerView> view_value;sp::Application* application=nullptr;std::array<std::unique_ptr<Connection>,2> connections;
 unsigned sdk_accepts=0,address_reads=0,context_updates=0,starts=0,cleanups=0,gate_inits=0,gate_deletes=0,ports_created=0,ports_closed=0,factory_calls=0,moves=0,gets=0,posts=0,wait_scopes=0,initial_waits=0,executor_cleanups=0,cancellations=0,processed=0;
 std::vector<OVERLAPPED*> accept_packets;std::vector<s::Socket> closed;std::vector<int> families;std::vector<std::string> order;std::string environment_key,environment_value;std::vector<std::byte> bound;std::uintptr_t port=0;
 Connection& connection(s::Socket socket){if(socket<201||socket>202||!connections[socket-201])unused();return *connections[socket-201];}
};
State* running=nullptr;
int WSAAPI startup(WORD version,LPWSADATA data){auto& st=*running;require(version==MAKEWORD(2,2),"Owned context requests WSA2.2");++st.starts;st.order.push_back("startup");return ::WSAStartup(version,data);}
int WSAAPI cleanup(){auto& st=*running;++st.cleanups;st.order.push_back("cleanup");return ::WSACleanup();}
int WSAAPI local_name(SOCKET socket,sockaddr* output,int* bytes){
 require(socket==200&&*bytes>=28,"Local endpoint uses actual owned acceptor token and initialized SDK buffer");auto& st=*running;std::memset(output,0,*bytes);
 output->sa_family=st.ipv6?23:2;reinterpret_cast<unsigned char*>(output)[2]=0x30;reinterpret_cast<unsigned char*>(output)[3]=0x39;*bytes=28;st.order.push_back("local");return 0;
}
BOOL WINAPI environment(LPCSTR key,LPCSTR value){auto& st=*running;require(st.ready&&*st.ready==1,"Actual server source publishes port even with preseeded ready byte; this is NOT startup milestone completion");st.environment_key=key;st.environment_value=value;st.order.push_back("environment");return FALSE;}
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
 if(st.sdk_accepts==1&&!::SetEvent(st.armed))unused();
 if(st.sdk_accepts<=2){if(st.application){auto ports=st.application->ports();require(ports.accept.snapshot().pending==1,"Concrete Profile/native flow retains next accept SDK loan alongside active session");if(!ports.completion.post(st.port,0,0,ov))unused();}else if(!::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(st.port),0,0,ov))unused();}
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
  ++state.gets;++state.physical_gets;DWORD transferred=0;ULONG_PTR token=0;LPOVERLAPPED raw=nullptr;
  auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&transferred,&token,&raw,timeout);
  bytes=transferred;key=token;operation=raw;if(result&&raw&&state.shutdown_posted&&raw==state.accept_packets.back()){::SetLastError(995);return FALSE;}return result;
 }
 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation)override{++state.posts;return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),bytes,key,operation);}
 std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct Information final:anadius::handlers_recovered::Effects {
    State& state;explicit Information(State& s):state(s){}
    void log_profile_name_084f48(std::string_view name)override{require(name=="bound-online-user","Actual Graph GetProfile handler reads username committed by actual Online source worker");++state.profile_logs;}
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
 const auto socket=view.owner.socket_state.socket;if(socket<201||socket>202||state.connections[socket-201])unused();auto c=std::make_unique<Connection>();c->socket=socket;c->token=view.token;c->owner_id=view.owner.identity();c->encrypted=socket==202;c->expected_response=c->encrypted?"GetProfileResponse":"QueryEntitlementsResponse";
 require(state.host_barrier&&state.online_barrier&&state.bound_config&&state.bound_runtime&&state.bound_config->username=="bound-online-user"&&state.bound_config->persona_id=="bound-online-persona"&&state.bound_config->user_id=="bound-online-id"&&state.bound_runtime->online==1&&state.bound_config->game_attributes.at("host-ready")=="set-by-source-host-lower","First real session sees SAME RuntimeOwner configuration and Online+Host writes only AFTER both actual startup calls return");
 require(view.owner.cipher.encrypted_c4==0&&view.owner.cipher.unknown_c5==0&&view.owner.cipher.native().mode_00==0,"Graph prepare callback receives actual023000 owner after second native reset and supplies NOlogic/context graph");if(c->encrypted){view.owner.cipher.set_fixed_key_084c98();view.owner.cipher.encrypted_c4=1;c->client.set_fixed_key_084c98();}state.connections[socket-201]=std::move(c);
}
void observe(State& state,sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){
 auto& c=state.connection(view.owner.socket_state.socket);if(view.token!=c.token||view.owner.identity()!=c.owner_id)unused();
 require(error.empty(),"Worker-owned Graph observer runs on exact typed native invocation without caller Graph/MessageEffects/Logic");
 if(kind==ag::Log::Received){
  ++c.received_logs;bool mapped=false;auto ports=state.application->ports();for(const auto& region:ports.view.memory.buffer_context().memory.regions)if(region.bytes.size()>=text.size())for(std::size_t i=0;i<=region.bytes.size()-text.size();++i)if(std::equal(text.begin(),text.end(),region.bytes.begin()+i)){mapped=true;break;}
  require(mapped&&text==c.expected_requests.at(c.processed),"ConcreteGraph realXML/native033d70 received observer sees exact live Parent-backed request BEFOREdispatch");c.live_request_seen=true;
  if(c.encrypted){require(view.owner.cipher.encrypted_c4==1&&view.owner.cipher.native().mode_00==2,"Declared synthetic encrypted peer uses actual native prefix AES decrypt with SAMEsession cipher before observer");c.auto_decrypted=true;}
 }else if(kind==ag::Log::Response){++c.response_logs;++c.processed;++state.processed;require(text.find(c.expected_response)!=std::string_view::npos,"Concrete owned Router produces real response BEFORE native encryption/hex/NUL");}
 else if(kind==ag::Log::Event){++c.event_logs;c.event_text.assign(text);require(text.find("DOWN")!=std::string_view::npos,"Worker-owned Graph supplies actual lazy DOWNevent without caller queue/context implementation");}
 else unused();
}

void deliver_receive(State& state,Connection& c){
 if(!c.receive_waiting)unused();auto ports=state.application->ports();const auto left=c.receive_wire.size()-c.cursor;
 if(!left&&c.socket==201&&state.plain_stage<2)return;
 const auto count=std::min<std::size_t>(left,c.receive_buffer.len);if(count)std::memcpy(c.receive_buffer.buf,c.receive_wire.data()+c.cursor,count);else{if(c.eof_posted)unused();c.eof_posted=true;}
 c.cursor+=count;c.receive_waiting=false;if(!ports.completion.post(state.port,static_cast<std::uint32_t>(count),0,c.receive_ov))unused();
}
void post_send(State& state,Connection& c){if(!c.send_waiting||c.send_posted)unused();auto ports=state.application->ports();c.send_posted=true;if(!ports.completion.post(state.port,c.send_bytes,0,c.send_ov))unused();}
int WSAAPI receive_sdk(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,LPDWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
 auto& state=*running;require(!state.sdk_pending&&buffers&&count==1&&transferred&&!*transferred&&flags&&!*flags&&ov&&!callback,"Default NativeAsyncReceiveApi forwards exact WSARecv one-buffer/privateflags/zeroDWORD/realOV/nullcallback signature");auto& buffer=buffers[0];
  if(!state.application)unused();auto& c=state.connection(socket);auto& app=*state.application;auto ports=app.ports();++c.receives;
  if(c.receives==1){
   auto session=app.session(c.token);require(session.owner.identity()==c.owner_id&&ports.view.memory.owns_control(session.owner.weak_self.control_08),"Profile session token resolves actual023000 control, not a caller-prepared owner");
   try{(void)ports.view.memory.buffer_context().memory.view(c.greeting_tail,1);}catch(const s::UnsupportedInput&){c.greeting_gone=true;}
   if(socket==202){auto registry=app.registry();const auto& first=state.connection(201);require(app.snapshot().live_sessions==2&&ports.view.memory.snapshot().controls==2&&registry.frames.size()==2&&registry.frames[0].owner&&registry.frames[1].owner&&registry.strings[0].string&&registry.strings[1].string&&registry.reads[0].implementation&&registry.reads[1].implementation&&registry.writes[0].implementation&&registry.writes[1].implementation&&registry.finish[0].owner&&registry.finish[1].owner&&registry.queued[0].owner&&registry.queued[1].owner&&registry.messages[0].owner&&registry.messages[1].owner&&registry.environments[0].source&&registry.environments[1].source&&app.session(first.token).owner.weak_self.control_08->strong_08>0,"Two actual sessions overlap with all Profile-owned fixed registries live while plain read/send retains its real owner");state.overlap_seen=true;}
   const auto build=[&](unsigned index){return "<LSX><Request "+std::string()+"id=\"conn"+std::to_string(socket-200)+(c.encrypted?"":"-"+std::to_string(index))+"\" recipient=\"EbisuSDK\">"+std::string(c.encrypted?"<GetProfile index=\"0\"/>":"<QueryEntitlements Group=\"synthetic-group\"><FilterOffers>synthetic-offer</FilterOffers></QueryEntitlements>")+"</Request></LSX>";};
   for(unsigned i=0;i<(c.encrypted?1U:17U);++i){auto request=build(i);auto expected=request;c.requests.push_back(request);c.expected_requests.push_back(expected);}
   c.expected_request=c.expected_requests[0];for(unsigned i=0;i<(c.encrypted?1U:7U);++i){auto request=c.requests[i];if(c.encrypted){if(!c.client.encrypt_042610(request))unused();m::hex_encode_03d190(request);}request.push_back(0);c.receive_wire+=request;}
  }
  if(c.receive_waiting)unused();c.receive_waiting=true;c.receive_buffer=buffer;c.receive_ov=ov;deliver_receive(state,c);

 const bool success=socket==202&&c.receives==1;if(success){++state.receive_success;*transferred=static_cast<DWORD>(c.cursor);}*flags=0xface;
 state.sdk_pending=1;state.sdk_expected_error=success?1234U:997U;state.sdk_trace.push_back('R');::WSASetLastError(state.sdk_expected_error);return success?0:SOCKET_ERROR;
}
int WSAAPI send_sdk(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,DWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
 auto& state=*running;require(!state.sdk_pending&&buffers&&count==1&&transferred&&!*transferred&&!flags&&ov&&!callback,"Default NativeAsyncSendApi forwards exact WSASend one-buffer/zeroDWORD/flags0/realOV/nullcallback signature");auto& buffer=buffers[0];auto& c=state.connection(socket);if(c.send_waiting)unused();++c.sends;c.response_wire.append(buffer.buf,buffer.len);c.send_waiting=true;c.send_posted=false;c.send_ov=ov;c.send_bytes=buffer.len;if(c.encrypted)post_send(state,c);
 const bool success=socket==202&&c.sends==1;if(success){++state.send_success;*transferred=buffer.len;}state.sdk_pending=2;state.sdk_expected_error=success?1234U:997U;state.sdk_trace.push_back('S');::WSASetLastError(state.sdk_expected_error);return success?0:SOCKET_ERROR;
}
int error_sdk(unsigned kind){auto& state=*running;const auto error=::WSAGetLastError();require(state.sdk_pending==kind&&static_cast<unsigned>(error)==state.sdk_expected_error,"Native async import reads WSAGetLastError immediately and unconditionally, including successful non997 result");state.sdk_pending=0;state.sdk_trace.push_back('E');if(kind==1)++state.receive_error_calls;else ++state.send_error_calls;return error;}
int WSAAPI receive_error_sdk(){return error_sdk(1);}int WSAAPI send_error_sdk(){return error_sdk(2);}
bool clear(const sp::RegistryView& registry){bool result=true;for(const auto& v:registry.frames)result&=!v.identity&&!v.owner;for(const auto& v:registry.strings)result&=!v.identity&&!v.string;for(const auto& v:registry.reads)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.writes)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.finish)result&=!v.identity&&!v.owner;for(const auto& v:registry.queued)result&=!v.identity&&!v.owner;for(const auto& v:registry.messages)result&=!v.identity&&!v.owner;for(const auto& v:registry.environments)result&=!v.allocator_identity&&!v.source;return result;}
// Explicit selected lifecycle driver. It observes packets and external drain;
// those decisions are not supplied by concrete Policy itself.
struct Hooks {
 State& state;Dispatcher& dispatcher;explicit Hooks(State& s,Dispatcher& d):state(s),dispatcher(d){}
 s::IocpSchedulerEntry* completion_owner(sp::Application& application,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){
  state.application=&application;auto ports=application.ports();if(!state.view_value){state.view_value.emplace(ports.view);state.view=&*state.view_value;require(ports.view.memory.memory_domain_identity()==state.expected_domain&&::GetCurrentThreadId()==state.expected_worker,"Prepared Parent belongs to controller domain and executes actual Graph source chain on distinct admitted native worker");}
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
  if(application.snapshot().live_sessions==0&&state.processed==18&&!state.stop_ready){require(clear(application.registry())&&state.connections[0]->logic_destroyed&&state.connections[1]->logic_destroyed&&ports.view.memory.snapshot().controls_destroyed==2,"Profile clears every fixed binding and logic before final observer delete without fixture cleanup");state.cleared_seen=true;state.stop_ready=true;const auto h=ports.accept.handle_for(state.accept_packets.back());ports.accept.note_cancel_requested(h);if(!::SetEvent(state.drain_ready))unused();}
 }
 
 void worker_returned(sp::Application& application,std::uint64_t count,const s::WorkerRunResult& result){
  auto ports=application.ports();require(count==59&&state.accept_completions==3&&state.receive_packets==state.connections[0]->receives+state.connections[1]->receives+state.zero_receive_packets&&state.zero_receive_packets==11&&state.posted_packets==18&&state.send_packets==18&&result.error&&result.error->value==0,"Actual59 native packets include11 immediate zero-sized receive completions for buffered frames plus all SDK/posted response/send/accept completions");
  require(state.overlap_seen&&state.cleared_seen&&state.shutdown_done&&!application.snapshot().live_sessions&&!application.snapshot().live_jobs&&clear(application.registry()),"Bounded Profile owns and retires overlapping session/job registries without caller-built contexts or observer cleanup");
  for(auto& c:state.connections){
   std::vector<std::string> frames;std::size_t cursor=0;while(cursor<c->response_wire.size()){const auto end=c->response_wire.find('\0',cursor);if(end==std::string::npos)unused();frames.push_back(c->response_wire.substr(cursor,end-cursor));cursor=end+1;}
   const auto messages=c->encrypted?1U:17U;require(c->processed==messages&&c->sends==messages&&c->completed_sends==messages&&c->received_logs==messages&&c->response_logs==messages&&c->live_request_seen&&c->eof_posted,"Every real native input generates one observed FIFO queued response/send and preserves typed live-prefix callbacks");
   if(c->encrypted){require(frames.size()==2,"Encrypted peer retains separate response/event NUL frames during other session backlog");auto response=frames[0];m::hex_decode_03ce30(response);require(c->client.decrypt_042480(response)&&response.find("GetProfileResponse")!=std::string::npos&&response.find("Persona=\"bound-online-user\"")!=std::string::npos&&response.find("PersonaId=\"bound-online-persona\"")!=std::string::npos&&response.find("UserId=\"bound-online-id\"")!=std::string::npos&&response.find("id=\"conn2\"")!=std::string::npos,"Independent client decrypts actual native prefix response while plain queue grows");auto event=frames[1];m::hex_decode_03ce30(event);require(c->client.decrypt_042480(event)&&event==c->event_text&&event.find("DOWN")!=std::string::npos&&c->event_logs==1&&c->auto_decrypted,"Typed prefix AES event chaining/pop remains intact alongside owned deque growth");}
   else {require(frames.size()==17&&c->event_logs==0&&state.flush_started&&state.wrap_seen&&state.growth_seen&&state.backlog_seen,"Actual17-response deque spans initial drain, head wrap and owned8to16 growth without legacy policy ");bool ordered=true;for(unsigned i=0;i<17;++i)ordered&=frames[i].find("QueryEntitlementsResponse")!=std::string::npos&&frames[i].find("id=\"conn1-"+std::to_string(i)+"\"")!=std::string::npos;require(ordered,"All17 physical SDK responses preserve exact FIFO IDs through worker-created concrete Graph Router after wrap/growth");}
  }
  bool ordered=state.sdk_trace.size()==2*(state.receive_error_calls+state.send_error_calls);for(std::size_t i=0;i+1<state.sdk_trace.size();i+=2)ordered&=(state.sdk_trace[i]=='R'||state.sdk_trace[i]=='S')&&state.sdk_trace[i+1]=='E';
  require(ordered&&!state.sdk_pending&&state.receive_error_calls==state.connections[0]->receives+state.connections[1]->receives&&state.send_error_calls==18&&state.receive_success==1&&state.send_success==1,"All27 actual WinSock async imports have matching immediate error reads, including BOOLsuccess and pending997 paths");
  require(state.profile_logs==1,"Worker-created owned Router dispatches GetProfile through explicit narrow Information lower exactly once");
  require(ports.view.memory.snapshot().controls_destroyed==2&&ports.view.memory.output_map_bindings().empty()&&!ports.view.io.snapshot().allocations&&!ports.view.scheduler.work_count_30,"Profile maps/controls and worker IO drain while legitimate Graph-owned event buffers remain until external attestation");state.worker_done=true;
 }
 bool external_quiescent(o::ServerView view,sp::Application& application){
  bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}require(outside&&state.worker_done&&state.shutdown_done&&!application.snapshot().live_jobs,"External hook checks observed inline job/session drain outside Root; worker counts alone are not attestation");
  unsigned relay=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* op=nullptr;const auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,op,0);const auto error=dispatcher.get_last_error();if(!got){if(error!=WAIT_TIMEOUT||op)unused();break;}if(op||bytes||key)unused();++relay;}
  require(relay==1&&state.gets==state.wait_scopes+state.initial_waits&&state.initial_waits==1&&!view.memory.snapshot().controls&&!view.io.snapshot().allocations,"Real stop relay/timeout observed outside Root after Profile reaps all owners; Graph event buffers are retained until this external proof, without operation lookup");state.queue_drained=true;state.source_drained=true;return true;
 }
};
SOCKET WSAAPI open_socket(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){if(family!=2||type!=1||protocol!=6||info||group||flags!=1)unused();return 200+ ++running->factory_calls;}
BOOL WINAPI cancel_sdk(HANDLE,LPOVERLAPPED){unused();}
void move_executor(void* p,s::ExecutorFields& out,s::ExecutorFields& source,std::uintptr_t table){if(table!=0x180084928ULL)unused();++static_cast<State*>(p)->moves;out.context=source.context;out.zero_74=source.zero_74;out.self_reference_80=true;}
// Inputs contain only lower providers. Worker creates Graph, its exactStateView,
// Policy/Profile/epoch/TLS and server source itself; no caller Graph is built.
void configure_inputs(State& observed,aw::Inputs& inputs){
 require(!inputs.graph.state.configuration&&!inputs.graph.state.lifetime&&!inputs.runtime.tls_index&&!inputs.runtime.guard_epoch&&!inputs.runtime.profile&&!inputs.runtime.errors&&!inputs.server.errors,"Concrete application Worker configure supplies only lowers; StateView/TLS/Graph/Profile/errors remain module-owned");
 observed.expected_worker=::GetCurrentThreadId();running=&observed;
 auto sockets=std::make_shared<Sockets>(observed);auto dispatcher=std::make_shared<Dispatcher>(observed);auto hooks=std::make_shared<Hooks>(observed,*dispatcher);
 auto& options=inputs.graph;auto log_buffer=std::make_shared<std::stringbuf>();options.policy.stream=anadius::shared_log::make_stream(log_buffer);options.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,sync_send_sdk,select_sdk};options.policy.lifecycle.lifetime=hooks;
 options.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& a,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){return h->completion_owner(a,owner,op,error,bytes);};options.policy.lifecycle.after=[h=hooks.get()](sp::Application& a){h->after_completion(a);};options.policy.lifecycle.returned=[h=hooks.get()](sp::Application& a,std::uint64_t n,const s::WorkerRunResult& r){h->worker_returned(a,n,r);};options.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView v,sp::Application& a){return h->external_quiescent(v,a);};
 options.information=std::make_shared<Information>(observed);options.content=std::make_shared<ContentEffects>();options.licensing=std::make_shared<LicenseBoundary>();options.time=std::make_shared<Time>();options.version=std::string(70000,'a');options.show_lsx=1;options.event_capacity=8;
 options.profile.receive={receive_sdk,receive_error_sdk};options.profile.send={send_sdk,send_error_sdk};options.profile.sessions=2;options.profile.jobs_capacity=4;options.profile.headers=128;options.profile.initial_output_slots=8;
 options.prepare_session=[&observed](sp::SessionView view){prepare_peer(observed,view);};options.observe_log=[&observed](sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){observe(observed,view,kind,text,error);};
 auto& server_deps=inputs.server;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(observed);server_deps.executor=std::make_shared<Executor>(observed);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
 auto& runtime_deps=inputs.runtime;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.executor={&observed,move_executor};runtime_deps.capacity=32;
}

struct BootstrapControl {
 State& state;std::mutex gate;std::map<void*,std::size_t> payloads;unsigned allocated=0,deleted=0;std::atomic<unsigned> cleaned=0,online_destroyed=0,online_freed=0,hook_disabled=0,hook_uninitialized=0;unsigned stops=0,configured=0;std::vector<DWORD> worker_ids;std::terminate_handler handler=nullptr;std::vector<anadius::bootstrap_native_recovered::ExitCallback> callbacks;
 explicit BootstrapControl(State& s):state(s){}
};
BootstrapControl* bootstrap_control=nullptr;
void wait_phase(State& state,aw::Phase expected){const auto start=::GetTickCount64();for(;;){auto* component=state.component.load();if(component){const auto snapshot=component->snapshot();if(snapshot.phase==expected)return;if(snapshot.phase==aw::Phase::Poisoned)throw std::logic_error("application worker poisoned");}if(::GetTickCount64()-start>=30000)throw std::logic_error("application worker phase timeout");::Sleep(1);}}
struct Remaining:anadius::bootstrap_owner::Services {
 BootstrapControl& c;explicit Remaining(BootstrapControl& v):c(v){}
 int register_exit(anadius::bootstrap_native_recovered::ExitCallback)override{unused();}void install_terminate(std::terminate_handler)override{unused();}void* log_insert(void* p,std::uintptr_t)override{return p;}void log_finish(void*)override{}
 void* allocate(std::size_t n)override{auto* p=::operator new(n);std::lock_guard lock(c.gate);if(!c.payloads.emplace(p,n).second)unused();++c.allocated;return p;}
 void deallocate(void* p,std::size_t n)override{{std::lock_guard lock(c.gate);auto it=c.payloads.find(p);require(it!=c.payloads.end()&&it->second==n,"Actual bootstrap worker frees its exact source payload through synchronized provider");c.payloads.erase(it);++c.deleted;}::operator delete(p);}
 anadius::recovered::NoArgumentFunction worker(anadius::recovered::Worker)override{unused();}anadius::recovered::BooleanFunction boolean_worker()override{unused();}[[noreturn]]void thread_error(int)override{unused();}void between_workers()override{unused();}void guard_return()override{}void cleanup_threads()override{++c.cleaned;}void cleanup_first()override{unused();}void cleanup_second()override{unused();}void cleanup_network()override{unused();}void terminate_log()override{unused();}[[noreturn]]void unexpected_terminate_return()override{unused();}
};
struct Online:anadius::online_recovered::Effects {
 BootstrapControl& c;explicit Online(BootstrapControl& v):c(v){}
 std::uint32_t suppress_errors_0450a0()override{return 77;}void restore_error_mode(std::uint32_t mode)override{require(mode==77,"Actual Online source restores injected error mode");}
 anadius::configuration::Path filesystem_widen_006d70(std::string_view t)override{return {t.begin(),t.end()};}anadius::configuration::Path parent_path_019820(const anadius::configuration::Path&)override{return u"synthetic";}anadius::configuration::Path join_016b00(const anadius::configuration::Path& a,const anadius::configuration::Path& b)override{return a+u"/"+b;}
 anadius::online_recovered::Module load_copy_0450d0(const anadius::configuration::Path&)override{return 0x80;}anadius::online_recovered::Procedure get_proc_address(anadius::online_recovered::Module m,std::string_view name)override{require(m==0x80,"Actual Online source uses owned synthetic module token without any DLLload");return name=="version_check"?0x81:0x82;}
 std::int32_t call_version_check(anadius::online_recovered::Procedure p)override{require(p==0x81,"Online version source invokes explicit lower token");return 2;}anadius::online_recovered::Object call_create_online(anadius::online_recovered::Procedure p,const char16_t*)override{require(p==0x82,"Online source creates explicit synthetic object");return 0x90;}
 std::int32_t status_virtual_08(anadius::online_recovered::Object object)override{require(object==0x90,"Online source status uses exact synthetic object");const auto snapshot=c.state.component.load()->snapshot();require(snapshot.host_complete&&!snapshot.online_complete&&snapshot.phase==aw::Phase::WaitingStartup&&c.state.physical_gets==0&&c.state.bound_runtime->online_ready==1,"Actual Host returned milestone is visible but preseeded Online readybyte does NOT release queued AcceptEx IO before Online source returns");c.state.online_barrier=true;return 0;}
 const char* username_virtual_10(anadius::online_recovered::Object)override{return "bound-online-user";}const char* persona_virtual_18(anadius::online_recovered::Object)override{return "bound-online-persona";}const char* user_id_virtual_20(anadius::online_recovered::Object)override{return "bound-online-id";}
 void destroy_virtual_00(anadius::online_recovered::Object o)override{require(o==0x90,"Joined cleanup destroys actual Online source state object once");++c.online_destroyed;}bool free_library(anadius::online_recovered::Module m)override{require(m==0x80,"Joined cleanup releases actual Online source module state once");++c.online_freed;return false;}
 anadius::status_recovered::Status status_01ea20(const anadius::configuration::Path&,std::uint32_t flags)override{require(flags==3,"Online source uses original status flags without reading actual files");return {0,0,0};}[[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const anadius::configuration::Path&)override{unused();}std::string read_binary_03b3f0(const anadius::configuration::Path&)override{unused();}bool write_binary_03c1d0(const anadius::configuration::Path&,std::string_view)override{unused();}void log_literal_006880(std::string_view)override{}void log_finish_0088e0()override{}void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{unused();}void wait_steps_036880(std::int32_t)override{unused();}void finish_flag_036820()override{unused();}
};
struct LibraryOS:anadius::recovered::libraries::SystemCalls {
 std::atomic<std::uint32_t> mode=0;std::uint32_t get_error_mode()override{return mode.load();}std::uint32_t set_error_mode(std::uint32_t value)override{return mode.exchange(value);}std::uintptr_t load_library(const wchar_t*)override{unused();}bool free_library(std::uintptr_t)override{unused();}int wide_to_narrow(std::uint32_t,std::uint32_t,const wchar_t*,int,char*,int,bool*)override{unused();}std::uint32_t last_error()override{return 0;}
};
struct LibraryEffects:anadius::recovered::libraries::Effects {
 void begin_load_log_0863e0()override{unused();}void append_log_path_and_flush(std::string_view)override{unused();}void message_038bc0(std::string,std::string,std::uint32_t)override{unused();}[[noreturn]]void throw_runtime_00cd70(int)override{unused();}[[noreturn]]void throw_filesystem_00cdb0(std::uint32_t)override{unused();}void wait_steps_036880(int)override{unused();}void finish_flag_036820()override{unused();}
};
struct Host:anadius::host_init_recovered::Effects {
 BootstrapControl& c;std::uintptr_t next=0x1000;explicit Host(BootstrapControl& v):c(v){}
 std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t,std::uint8_t)override{return m+1;}bool verify_version(const anadius::host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t)override{return true;}
 std::int32_t initialize_engine_046a20()override{if(::WaitForSingleObject(c.state.armed,30000)!=WAIT_OBJECT_0)unused();wait_phase(c.state,aw::Phase::WaitingStartup);const auto snapshot=c.state.component.load()->snapshot();require(!snapshot.host_complete&&!snapshot.online_complete&&c.state.physical_gets==0&&c.state.bound_runtime->server_ready==1&&c.state.bound_runtime->online_ready==1&&c.state.bound_runtime->libraries_ready==1,"Queued real ownIOCP AcceptEx packet cannot dispatch while actual Host/Online callbacks incomplete despite ALLpreseeded readybytes");c.state.bound_config->game_attributes["host-ready"]="set-by-source-host-lower";c.state.host_barrier=true;return 0;}
 std::uintptr_t get_module_handle(std::u16string_view)override{return 1;}std::uintptr_t get_proc_address(std::uintptr_t,std::string_view)override{return ++next;}std::int32_t create_hook_0466e0(std::uintptr_t target,anadius::host_init_recovered::Callback,std::uintptr_t& original)override{original=target+0x100000;return 0;}std::int32_t enable_hook_046a10(std::uintptr_t)override{return 0;}void log_text_006880(std::string_view)override{}void log_integer_00af90(std::int32_t)override{}void finish_log_0088e0()override{}
};
struct Cleanup:anadius::network_hook_recovered::CleanupEffects {
 BootstrapControl& c;explicit Cleanup(BootstrapControl& v):c(v){}std::int32_t disable_hook_046a00(std::uintptr_t)override{++c.hook_disabled;return 0;}std::int32_t uninitialize_engine_046ac0()override{++c.hook_uninitialized;return 0;}void log_literal_006880(std::string_view)override{}void log_finish_0088e0()override{}
};
int at_exit(anadius::bootstrap_native_recovered::ExitCallback callback)noexcept{bootstrap_control->callbacks.push_back(callback);return 0;}std::terminate_handler set_terminate(std::terminate_handler handler)noexcept{const auto old=bootstrap_control->handler;bootstrap_control->handler=handler;return old;}std::terminate_handler get_terminate()noexcept{return bootstrap_control->handler;}void previous_terminate(){std::abort();}
struct PermanentErrors:anadius::bootstrap_native_recovered::Errors {[[noreturn]]void lifecycle(anadius::bootstrap_native_recovered::LifecycleFault)override{unused();}[[noreturn]]void callback_exception(anadius::bootstrap_native_recovered::Entry,std::exception_ptr error)noexcept override{try{std::rethrow_exception(error);}catch(const std::exception& e){std::cerr<<"SOURCE CALLBACK EXCEPTION: "<<e.what()<<'\n'<<std::flush;}catch(...){}::ExitProcess(115);}} permanent_errors;
BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* output){if(flags!=4||!address||!output)unused();*output=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
HANDLE WINAPI create_worker(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE entry,LPVOID argument,DWORD flags,LPDWORD id){require(!security&&!stack&&entry&&argument&&!flags&&id,"Actual bootstrap uses original CreateThread wrapper operands");auto result=::CreateThread(security,stack,entry,argument,flags,id);if(result){std::lock_guard lock(bootstrap_control->gate);bootstrap_control->worker_ids.push_back(*id);}return result;}
void actual_bootstrap_worker(){
 NativeDomain native;auto state=std::make_shared<State>();auto* observed=state.get();std::weak_ptr<State> state_lifetime=state;state->expected_domain=native.root.get();BootstrapControl control(*state);control.handler=previous_terminate;bootstrap_control=&control;auto* main_ptd=native.root->current_ptd();const auto controller=::GetCurrentThreadId();
 HANDLE armed=::CreateEventW(nullptr,TRUE,FALSE,nullptr),drain=::CreateEventW(nullptr,TRUE,FALSE,nullptr);require(armed&&drain,"Owned startup/drain observations use real kernel events");state->armed=armed;state->drain_ready=drain;
 b2_test::Events controller_events;bb::Resources resources{};resources.state_lifetime=state;resources.configuration=std::make_unique<b2_test::Effects>(controller_events);auto files=std::make_unique<b2_test::Files>(controller_events);files->base=b2_test::base_text();files->over=R"("Config2" {"User" {"Username" "pre-online-user"}})";resources.files=std::move(files);resources.input=std::make_unique<b2_test::InputPlatform>(controller_events);resources.console=std::make_unique<b2_test::Console>(controller_events);auto thread_api=anadius::thread_native::native_api();thread_api.get_module=no_module;thread_api.create=create_worker;resources.threads=native.root->make_threads(thread_api);resources.remaining=std::make_unique<Remaining>(control);resources.online=std::make_unique<Online>(control);resources.library_os=std::make_unique<LibraryOS>();resources.library_effects=std::make_unique<LibraryEffects>();resources.host=std::make_unique<Host>(control);resources.hook_cleanup=std::make_unique<Cleanup>(control);resources.join_timeout_ms=30000;
 resources.application_worker=[keep=state,domain=native.root,controller,&control](bb::CallbackStateView view)->std::unique_ptr<bb::ApplicationWorker>{require(::GetCurrentThreadId()==controller&&!keep->component&&view.runtime.server_ready==1&&view.runtime.online_ready==1&&view.runtime.libraries_ready==1,"Actual bind_state constructs application Worker on controller with stable RuntimeOwner state and deliberately preseeded readybytes");keep->bound_config=&view.configuration;keep->bound_runtime=&view.runtime;keep->bound_execution=&view.execution;keep->ready=&view.runtime.server_ready;aw::Settings settings;settings.lifetime=keep;settings.state=aw::Settings::StateSeed{0,{}};settings.configure=[keep,&control](aw::Inputs& input){++control.configured;configure_inputs(*keep,input);};settings.startup_timeout_ms=30000;auto worker=aw::Worker::create(domain,view,std::move(settings));keep->component=worker.get();return worker;};
 resources.stop_sources=[observed,&control]{++control.stops;require(observed->stop_ready&&observed->cleared_seen&&observed->component.load()->snapshot().phase==aw::Phase::Serving,"Controller externalstop receives actual drained sessions while pending accept SDK loan is STILLretained");observed->shutdown_posted=true;if(!::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(observed->port),0,0,observed->accept_packets.back()))unused();wait_phase(*observed,aw::Phase::Returned);require(observed->source_drained&&observed->component.load()->snapshot().memory.phase==wh::Phase::Returned,"Controller stop lower waits actual59packet terminal completion/externalproof/workerGraphdestruction before permitting SourceBackend request_stop and join");return true;};
 bb::RuntimeSeed seed{};anadius::execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.server_ready=seed.online_ready=seed.libraries_ready=1;seed.frequency_cache=0;
 auto runtime=std::make_unique<br::Runtime>(b2_test::seed(),std::move(seed),std::move(resources),permanent_errors,br::HostApi{at_exit,set_terminate,get_terminate});require(native.root->provider_leases()==2&&observed->component&&observed->component.load()->snapshot().phase==aw::Phase::Prepared&&!observed->starts&&!control.configured,"RuntimeOwner binds concrete Worker passively on controller; no caller Graph/Execution/TLS/StateView assembly or startupSDK before applicationentry");runtime->publish();state.reset();require(!state_lifetime.expired(),"Actual RuntimeOwner/SourceBackend/ApplicationWorker retain shared SDK/lower state after external owner drops");
 require(runtime->application_entry()(nullptr,1,nullptr)==TRUE,"Actual B4 RuntimeOwner native application_entry launches recovered source bootstrap and three real Domain threads");
 if(::WaitForSingleObject(drain,30000)!=WAIT_OBJECT_0){std::cerr<<"terminal drain ready timeout\n"<<std::flush;::ExitProcess(116);}
 require(runtime->configuration().username=="bound-online-user"&&runtime->configuration().persona_id=="bound-online-persona"&&runtime->configuration().game_attributes.at("host-ready")=="set-by-source-host-lower"&&observed->bound_config==&runtime->configuration()&&observed->host_barrier&&observed->online_barrier&&control.configured==1,"Actual Online source and Host setup modify EXACT RuntimeOwner configuration seen by worker-created Graph before firstdispatch");
 require(runtime->close(),"Actual SourceBackend externalstop→request_stop→TrackingThreads joins all3→routeleasesdrain→Worker.finish closes preparedmemory→applicationcleanup completes");const auto final=observed->component.load()->snapshot();
 require(final.phase==aw::Phase::Closed&&final.stop_requested&&final.online_complete&&final.host_complete&&final.memory.phase==wh::Phase::Closed&&final.memory.worker_exit_verified&&!final.memory.worker_handle_retained&&final.controller_thread==controller&&final.worker_thread==observed->expected_worker&&native.root->provider_leases()==1,"Closed concrete worker independently verifies actual native serverthread/FLS exit after all Graph owners retired onworker; NativeThreads provider remains controller-owned");
 require(control.worker_ids.size()==3&&control.worker_ids[0]==final.worker_thread&&control.worker_ids[0]!=control.worker_ids[1]&&control.worker_ids[1]!=control.worker_ids[2]&&control.worker_ids[0]!=control.worker_ids[2]&&std::none_of(control.worker_ids.begin(),control.worker_ids.end(),[&](DWORD id){return id==controller;}),"Actual recovered Factory created three distinct real server/online/library worker threads rather than manual one-worker execution");
 require(control.cleaned==3&&control.allocated==3&&control.deleted==3&&control.payloads.empty()&&control.online_destroyed==1&&control.online_freed==1&&control.hook_disabled>0&&control.hook_uninitialized==1&&control.stops==1,"Synchronized actual bootstrap payload/source cleanup and joined library/online/hook cleanup run exactly once with no concurrent fixture vector races");
 for(const auto& c:observed->connections)require(c->greeting_gone&&c->sync_calls==3&&c->max_sync==65536&&c->greeting_wire.find(std::string(70000,'a'))!=std::string::npos&&c->greeting_wire.find("<Challenge")!=std::string::npos&&c->greeting_wire.back()==0,"Worker itself creates actual large Graph greeting and frees Parent-backed bytes before receive");
 require(observed->worker_done&&observed->queue_drained&&observed->closed.size()==4&&observed->ports_closed==1&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1&&observed->bound_config->server_port==12345&&native.memory.count()==1&&native.root->current_ptd()==main_ptd&&native.root->locale_references()==2&&native.root->multibyte_references()==1,"Full Graph59packet/AES/DOWN/wrap/FIFO17 source chain and real3thread FLS teardown complete before controller retains only its originalPTD");
 runtime.reset();require(state_lifetime.expired()&&native.root->provider_leases()==0,"Controller destruction releases closed ApplicationWorker/NativeThreads/RuntimeOwner and every shared lower lifetime without a Graph alias oncontroller");running=nullptr;bootstrap_control=nullptr;require(::CloseHandle(armed)&&::CloseHandle(drain),"Owned observation handles close after complete native joins and provider destruction");require(native.root->close()&&native.memory.balanced()&&native.memory.slot_allocations==1&&native.memory.slot_releases==1&&native.root->locale_references()==1&&native.root->multibyte_references()==0,"Real processheap/FLS Domain closes oncontroller only after actual3workers/FLS cleanup and all provider leases drain");
}
}
int main(){try{actual_bootstrap_worker();std::cout<<"PASS: "<<checks<<" application-worker integration checks; actual RuntimeOwner/SourceBackend launches3DomainNativeThreads and concreteWorker-owned Graph NativeLogic/Router59packets/plainAES/DOWN/wrapped8to16/FIFO17, actualHost+Online startupbarriers despitepreseededready, controlledSDKterminaldrain thenrealjoin/FLS/controllerfinish; noOriginalDLL/game/network/nativeABI/fullResourcesLoaderclaim\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
