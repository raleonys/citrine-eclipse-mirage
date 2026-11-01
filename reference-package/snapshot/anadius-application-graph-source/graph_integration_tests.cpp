#include "graph.hpp"
#include <sstream>
#include "../anadius-native-profile-source/native.hpp"
#include "../anadius-server-runtime-source/resources.hpp"
#include <optional>
#include "../anadius-socket-source/response_containers.hpp"
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
namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;namespace sp=anadius::session_profile;namespace np=anadius::native_profile;namespace cp=anadius::session_policy;namespace ag=anadius::application_graph;
unsigned checks=0;
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
struct State {
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
  ++state.gets;DWORD transferred=0;ULONG_PTR token=0;LPOVERLAPPED raw=nullptr;
  auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&transferred,&token,&raw,timeout);
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
   const auto build=[&](unsigned index){return "<LSX><Request "+std::string()+"id=\"conn"+std::to_string(socket-200)+(c.encrypted?"":"-"+std::to_string(index))+"\" recipient=\"EbisuSDK\">"+std::string(c.encrypted?"<GetConfig/>":"<QueryEntitlements Group=\"synthetic-group\"><FilterOffers>synthetic-offer</FilterOffers></QueryEntitlements>")+"</Request></LSX>";};
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

void concrete_profile(){
 NativeDomain native;auto state=std::make_shared<State>();running=state.get();auto* observed=state.get();std::weak_ptr<State> lifetime=state;state->configuration.entitlements.push_back({"synthetic-offer","synthetic-group","synthetic-version","synthetic-type","synthetic-tag"});
 std::uint32_t index=::TlsAlloc();if(index==TLS_OUT_OF_INDEXES)unused();std::weak_ptr<ag::Graph> graph_lifetime;std::weak_ptr<Hooks> weak_hooks;
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto worker=std::make_shared<WorkerApi>(index,*dispatcher);auto hooks=std::make_shared<Hooks>(*state,*dispatcher);weak_hooks=hooks;
  auto log_buffer=std::make_shared<std::stringbuf>();auto log_stream=anadius::shared_log::make_stream(log_buffer);ag::Options options;options.state={state,&state->configuration,&state->online,&state->saved,&state->achievement_file};options.policy.stream=log_stream;options.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,sync_send_sdk,select_sdk};options.policy.lifecycle.lifetime=hooks;
  options.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& a,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){return h->completion_owner(a,owner,op,error,bytes);};
  options.policy.lifecycle.after=[h=hooks.get()](sp::Application& a){h->after_completion(a);};options.policy.lifecycle.returned=[h=hooks.get()](sp::Application& a,std::uint64_t n,const s::WorkerRunResult& r){h->worker_returned(a,n,r);};options.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView v,sp::Application& a){return h->external_quiescent(v,a);};
  options.information=std::make_shared<Information>();options.content=std::make_shared<ContentEffects>();options.licensing=std::make_shared<LicenseBoundary>();options.time=std::make_shared<Time>();options.version=std::string(70000,'a');options.show_lsx=1;options.event_capacity=8;
  options.profile.receive={receive_sdk,receive_error_sdk};options.profile.send={send_sdk,send_error_sdk};options.profile.sessions=2;options.profile.jobs_capacity=4;options.profile.headers=128;options.profile.initial_output_slots=8;
  options.prepare_session=[keep=state](sp::SessionView view){prepare_peer(*keep,view);};options.observe_log=[keep=state](sp::SessionView view,ag::Log kind,std::string_view text,std::string_view error){observe(*keep,view,kind,text,error);};
  auto graph=ag::Graph::create(std::move(options));graph_lifetime=graph;state->graph=graph;
  o::Dependencies server_deps;server_deps.state_lifetime=state;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(*state);server_deps.executor=std::make_shared<Executor>(*state);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
  rt::Dependencies runtime_deps;runtime_deps.state_lifetime=state;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.worker=worker;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.tls_index=&index;runtime_deps.executor={observed,move_executor};runtime_deps.capacity=32;
  anadius::bootstrap_backend::Resources resources{};graph->install_server(resources,native.root,std::move(server_deps),std::move(runtime_deps));hooks->policy=graph->policy().get();hooks->errors=graph->policy()->errors().get();
  require(resources.server&&graph->profile()->snapshot().phase==sp::Phase::Fresh&&!observed->starts&&!observed->factory_calls&&graph->policy()->snapshot().worker_initializations==0&&graph->policy()->snapshot().exit_cleanups==0,"Concrete Graph installs Resources.server passively with NOcaller XML/Logic/Router/Random/event/context/Policy graph");
  sockets.reset();dispatcher.reset();worker.reset();hooks.reset();graph.reset();state.reset();require(!lifetime.expired()&&!graph_lifetime.expired(),"Server wrapper retains concrete Graph and all required SDK/state/lifecycle lifetimes after external owners drop");
  std::uint16_t port=0;
  try{server::server_start_029ea0(*resources.server,port,ready,execution);}catch(const std::exception& e){std::cerr<<"SOURCE EXCEPTION: "<<e.what()<<"\n"<<std::flush;::ExitProcess(111);}
  for(const auto& c:observed->connections)require(c->greeting_gone&&c->sync_calls==3&&c->max_sync==65536&&c->greeting_wire.find(std::string(70000,'a'))!=std::string::npos&&c->greeting_wire.find("<Challenge")!=std::string::npos&&c->greeting_wire.back()==0&&c->greeting_wire.size()>65536,"Concrete Profile builds/owns large actualMessageHandler greeting and native025720 frees it before per-session receive");
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1&&observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Actual SourceBackend server component publishes ready/port through existing native source path");
  require(observed->closed.size()==4&&observed->closed.back()==200&&std::count(observed->closed.begin(),observed->closed.end(),201)==1&&std::count(observed->closed.begin(),observed->closed.end(),202)==1&&std::count(observed->closed.begin(),observed->closed.end(),203)==1&&observed->moves==4&&observed->ports_closed==1&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1,"Two actual023000 factories and both native disposals close201/202 plus terminal203/listener exactly once before own port/gate/WSA teardown");
  const auto final=graph_lifetime.lock()->backend().snapshot();require(final.phase==o::Phase::Closed&&final.quiescence_attested&&observed->queue_drained&&!native.root->provider_leases()&&graph_lifetime.lock()->profile()->snapshot().phase==sp::Phase::Closed&&graph_lifetime.lock()->snapshot().phase==ag::Phase::Closed,"Runtime and concrete Profile retire owned graphs only after explicit external drain plus actual source counts");const auto policy_final=graph_lifetime.lock()->policy()->snapshot();require(policy_final.phase==cp::Phase::Closed&&!policy_final.active&&!policy_final.worker_active&&policy_final.worker_initializations==1&&policy_final.worker_finishes==1&&policy_final.exit_cleanups==18,"Concrete policy closes dedicated exit registry after one real native worker initialize/finish and18 inline job cleanups");require(graph_lifetime.lock()->policy()->exit_waits().active()==0&&graph_lifetime.lock()->policy()->exit_waits().blocks()==1&&!weak_hooks.expired(),"Dedicated selected exit registry has zero registrations and only its fixed inline head while lifecycle hooks remain retained through closed graph");observed->application=nullptr;
 }
 require(lifetime.expired()&&graph_lifetime.expired()&&weak_hooks.expired(),"Closed server/Runtime/Profile/Policy destruction releases all captured per-session logic/SDK/lifecycle hooks without ownership cycle");running=nullptr;require(::TlsFree(index)!=FALSE,"Borrowed TLS selector remains valid until complete concrete graph destruction");require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Real process heap/PTD/FLS domain drains every Profile frame/string/map/block/control/IO allocation");
}
}
int main(){try{concrete_profile();std::cout<<"PASS: "<<checks<<" application-graph integration checks; concrete Graph owns NativeLogic/XML/Router/Random/eventqueue/Policy/Profile/defaultjobs/environment/transports, real ownheap/FLS/IOCP and nativeplain+AES/wrapped8to16/FIFO17; only SDK/packetdriver/externaldrain decisions remain selected; no nativeABI/kernelquiescence claim\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected deque integration exception\n";return 1;}}
