#include "profile.hpp"
#include "../anadius-server-runtime-source/resources.hpp"
#include <optional>
#include "../anadius-socket-source/response_containers.hpp"
#include "../anadius-socket-source/message_core_bridge.hpp"
#include "../anadius-session-source/handler.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include "../anadius-dispatch-source/router.hpp"
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
namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;namespace sp=anadius::session_profile;
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
 s::Socket socket=0;sp::SessionToken token{};std::uintptr_t owner_id=0,greeting_data=0;bool encrypted=false,greeting_gone=false,eof_seen=false,logic_destroyed=false;unsigned receives=0,sends=0,sync_calls=0,processed=0;std::uint32_t max_sync=0;s::SessionCipher client;std::string greeting_wire,expected_greeting,response_wire;
};
struct State {
 bool ipv6=false,accept_pending=false,shutdown_posted=false,shutdown_done=false,terminal_selected=false,overlap_seen=false,deferred_hold_seen=false,cleared_seen=false,worker_done=false,queue_drained=false;
 std::uint8_t* ready=nullptr;o::ServerView* view=nullptr;std::optional<o::ServerView> view_value;sp::Application* application=nullptr;std::array<std::unique_ptr<Connection>,2> connections;
 unsigned sdk_accepts=0,address_reads=0,context_updates=0,starts=0,cleanups=0,gate_inits=0,gate_deletes=0,ports_created=0,ports_closed=0,factory_calls=0,moves=0,gets=0,posts=0,wait_scopes=0,initial_waits=0,executor_cleanups=0,cancellations=0,processed=0,worker_init=0,worker_finish=0,exits=0,job_starts=0,job_detaches=0;
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
 auto& st=*running;require(listener==200&&accepted==201+st.sdk_accepts&&buffer&&!receive&&local==0x90&&remote==0x90,"Profile integration uses exact injected AcceptEx boundary with distinct201/202/203 sockets and90+90 endpoints");
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
 s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffers,std::uint32_t flags)override{
  auto& c=state.connection(socket);if(buffers.size()!=1||flags)unused();auto& b=buffers[0];c.max_sync=std::max(c.max_sync,static_cast<std::uint32_t>(b.len));if(!c.sync_calls)c.greeting_data=reinterpret_cast<std::uintptr_t>(b.buf);
  const auto count=c.sync_calls==0?std::min(static_cast<std::uint32_t>(b.len),30000U):c.sync_calls==1?0U:b.len;++c.sync_calls;c.greeting_wire.append(b.buf,count);::SetLastError(0);return {0,count};
 }std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
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
struct Table:s::IoHandlerVtable {
 Table():IoHandlerVtable(0x180017a30ULL){}
 void destroy_slot_00(s::IoHandlerEnvironment&)override{}
 void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{to.payload_00=from.payload_00;to.state_18=from.state_18;}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment& e)override{return e.payload_00[0];}
};

// Only GetConfig is selected; all unrelated application effects fail closed.
struct Events:m::EventQueue {bool nonempty_relaxed()const override{return false;}int lock_04a6ac()override{unused();}std::int32_t& lock_count_4c()override{unused();}bool nonempty_locked()const override{unused();}std::string copy_front_locked()const override{unused();}void pop_front_locked()override{unused();}void unlock_04a6b4()override{unused();}};
struct Random:m::Random {m::Key challenge_bytes_034cf0()override{return {};}m::Key key_from_response_034c30(std::string_view)override{unused();}};
struct Information final:anadius::handlers_recovered::Effects {
    void log_profile_name_084f48(std::string_view)override{unused();}
    std::string current_time_041b60()override{return "2000-01-01T00:00:00";}
};
struct Mutex final:anadius::content_handlers_recovered::Mutex {
    std::int32_t count=0;unsigned locks=0,unlocks=0;
    int lock_04a6ac()override{++locks;return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{++unlocks;}
    [[noreturn]] void thread_error_04a648(int)override{unused();}
};
struct ContentEvents final:anadius::content_handlers_recovered::EventQueue {
    anadius::content_handlers_recovered::Mutex& mutex_02c620()override{unused();}
    bool initialized_locked()const override{unused();}
    void initialize_locked(std::string)override{unused();}
    void push_locked_034660(std::string)override{unused();}
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
struct Igo final:anadius::misc_handlers_recovered::IgoEffects {
    int lock_02c620_04a6ac()override{unused();}
    std::int32_t& lock_count_4c()override{unused();}
    void acquire_event_queue_02c400()override{unused();}
    void enqueue_034660(std::string&&)override{unused();}
    void unlock_04a6b4()override{unused();}
    [[noreturn]] void thread_error_04a648(int)override{unused();}
};
struct NoResponseBoundary final:anadius::misc_handlers_recovered::NoResponse {
    [[noreturn]] void throw_no_response_02bcd0_04d228()override{unused();}
};
struct LicenseBoundary final:anadius::dispatch_recovered::LicenseHandlers {
    unsigned calls=0;
    bool auth_code_02e3b0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool request_license_0320c0(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
    bool invalidate_license_030c50(const m::XmlNode&,anadius::handlers_recovered::Xml&)override{++calls;unused();}
};

struct AppData {
 anadius::handlers_recovered::Attributes game,settings;anadius::handlers_recovered::Profile profile{};std::uint8_t online=0,pretend=1;
 std::vector<anadius::configuration::Content> contents;std::vector<anadius::configuration::Entitlement> entitlements;
 anadius::content_handlers_recovered::AchievementMap achievements;std::string text;anadius::content_handlers_recovered::Path path;std::int32_t saved=0;
 Mutex mutex;ContentEvents content_events;Information info;Igo igo;NoResponseBoundary no_response;ContentEffects effects;LicenseBoundary licensing;
 anadius::xml_reader_bridge::Backend xml;
 anadius::dispatch_recovered::Router router{{game,settings,profile,online,pretend},{contents,entitlements,game,achievements,text,text,text,path,path,path,path,path,online,saved,mutex,content_events},{xml,info,igo,no_response,effects,licensing}};
};
struct MessageEffects:m::Effects {
 State& state;Connection& connection;AppData& data;MessageEffects(State& s,Connection& c,AppData& a):state(s),connection(c),data(a){}
 int atoi_05ee08(std::string_view)override{unused();}bool registered_handler(std::uint32_t r,const m::XmlNode& n,m::XmlPrinter& p)override{++state.processed;++connection.processed;return data.router.registered_handler(r,n,p);}
 [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
 void log_received(std::string_view)override{}void log_response(std::string_view)override{}void log_event(std::string_view)override{unused();}void log_bad_xml(std::string_view,std::string_view)override{unused();}void log_decryption_failure(std::string_view)override{unused();}
};

struct Guard:s::DispatcherEffects {
 std::uintptr_t capture_tls_slot()override{return 1;}std::int32_t thread_epoch(std::uintptr_t n)override{if(n!=1)unused();return 0;}void guard_header_04b838(std::int32_t&)override{unused();}void register_atexit_04b770(std::uintptr_t)override{unused();}void guard_footer_04b7cc(std::int32_t&)override{unused();}std::uint32_t read_stop_34(s::IocpSchedulerEntry& e)override{return e.high_34;}
 std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>&,const std::array<std::uintptr_t,2>&)override{unused();}[[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{unused();}void complete(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{unused();}
};
struct Logger:s::WriteFinishApi {
 s::WriteFinishStream stream{0x9000,-8};void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{unused();}[[noreturn]]void invalid_parameter_05c974()override{unused();}void next_write_02a500(std::uintptr_t)override{unused();}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{return stream;}s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{return stream;}std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t c)override{return c;}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{}void flush_00e1e0(s::WriteFinishStream&)override{}
};
struct Category:s::WriteFinishCategory {void message_slot_10(s::WriteFinishString& out,std::int32_t)override{out={};out.size_10=3;out.storage_00[0]='e';}};
struct Types:s::IoEnvironmentTypeApi {std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{if(a!=0x1008||b!=0x1800a25c8ULL)unused();return -1;}};
struct Environment {
 State& state;Table table;std::shared_ptr<Types> types=std::make_shared<Types>();s::IoHandlerExecutor executor{prepare};s::HandlerSchedule handlers{schedule_handler,nullptr};s::ResponseSchedule responses{schedule_response,nullptr};
 explicit Environment(State& s):state(s){}
 void make(s::IoHandlerEnvironment& out,std::uintptr_t owner=0){out={{reinterpret_cast<std::uintptr_t>(this),owner},&table,1,0x456,&executor};}
 static Environment& find(std::uintptr_t q){if(!q)unused();return *reinterpret_cast<Environment*>(q);}
 static std::uintptr_t type_query(){return 0x1000;}
 static void prepare(s::IoHandlerEnvironment& out,std::uintptr_t q,s::IoPrepareScratch&){find(q).make(out);}
 static void construct(s::IoHandlerEnvironment& out,std::uintptr_t q,s::IoEnvironmentConstructorScratch&){find(q).make(out);}
 static void response20(s::IoHandlerEnvironment& out,std::uintptr_t q,std::uintptr_t tag){if(tag!=0x180084425ULL)unused();find(q).make(out);}
 static void response88(s::IoHandlerEnvironment& out,std::uintptr_t q,s::ResponseExecutorScratch&){find(q).make(out);}
 static void copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& in){out.payload_00=in.payload_00;out.state_18=in.state_18;}
 static void schedule_handler(s::IoHandlerEnvironment&,s::OwnedHandlerTask*& slot){auto* task=slot;if(!task)unused();slot=nullptr;task->call_slot_00(1);}
 static void schedule_response(s::IoHandlerEnvironment& env,s::PostedFunctor*& slot){auto& self=find(env.payload_00[0]);auto ports=self.state.application->ports();s::PostedExecutor executor{ports.view.identity.value,&ports.view.scheduler,1};s::dispatch_posted_022e40(executor,slot,ports.environment.storage,ports.worker,ports.completion,ports.memory.posted());}
 sp::Environment configuration(){sp::Environment out;out.sentinels={&table,0,&executor,table.destroy_identity};out.types=types;out.queries={{0x456,type_query}};out.factories={{&executor,construct}};out.handlers={{0x456,&handlers}};out.response_executors={{&executor,response20,response88}};out.responses={{0x456,&responses}};out.copies={{&table,copy}};return out;}
};
struct Jobs:sp::Jobs {
 State& state;std::function<void()> deferred;sp::JobToken first{};explicit Jobs(State& s):state(s){}
 s::FrameThreadHandle start(sp::JobToken token,std::function<void()> runner)override{const auto n=++state.job_starts;if(n==1){first=token;deferred=std::move(runner);}else if(n==2)runner();else unused();return {1000+n,200+n};}
 std::int32_t detach(const s::FrameThreadHandle& handle)override{if(handle.handle!=handle.id+800)unused();++state.job_detaches;return 0;}
 void pump(){if(!deferred)unused();auto runner=std::move(deferred);deferred={};runner();}
};
struct Receive:s::AsyncReceiveApi {
 State& state;explicit Receive(State& s):state(s){}
 s::AsyncReceiveResult receive(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov)override{
  if(flags||!state.application)unused();auto& c=state.connection(socket);auto& app=*state.application;auto ports=app.ports();++c.receives;
  if(c.receives==1){
   auto session=app.session(c.token);require(session.owner.identity()==c.owner_id&&ports.view.memory.owns_control(session.owner.weak_self.control_08),"Profile session token resolves actual023000 control, not a caller-prepared owner");
   try{(void)ports.view.memory.buffer_context().memory.view(c.greeting_data+c.expected_greeting.size()-1,1);}catch(const s::UnsupportedInput&){c.greeting_gone=true;}
   if(socket==201)require(app.registry().frames.size()==1,"First SDK client uses the configured initial one-session registry before any growth");
   if(socket==202){auto registry=app.registry();const auto& first=state.connection(201);require(app.snapshot().live_sessions==2&&ports.view.memory.snapshot().controls==2&&registry.frames.size()==2&&registry.frames[0].owner&&registry.frames[1].owner&&registry.strings[0].string&&registry.strings[1].string&&registry.reads[0].implementation&&registry.reads[1].implementation&&registry.writes[0].implementation&&registry.writes[1].implementation&&registry.finish[0].owner&&registry.finish[1].owner&&registry.queued[0].owner&&registry.queued[1].owner&&registry.messages[0].owner&&registry.messages[1].owner&&registry.environments[0].source&&registry.environments[1].source&&app.session(first.token).owner.identity()==first.owner_id&&app.session(first.token).owner.weak_self.control_08->strong_08>0,"Two actual SDK sessions force growth from initial1 while the first deferred job retains its exact original owner and all routing contexts");state.overlap_seen=true;}
   std::string request="<LSX><Request id=\"conn"+std::to_string(socket-200)+"\" recipient=\"EbisuSDK\"><GetConfig/></Request></LSX>";if(c.encrypted){if(!c.client.encrypt_042610(request))unused();m::hex_encode_03d190(request);}request.push_back(0);if(request.size()>buffer.len)unused();std::memcpy(buffer.buf,request.data(),request.size());if(!ports.completion.post(state.port,static_cast<std::uint32_t>(request.size()),0,&ov))unused();
  }else{if(c.receives!=2)unused();if(!ports.completion.post(state.port,0,0,&ov))unused();}
  return {-1,997,0};
 }
};
struct Send:s::AsyncSendApi {State& state;explicit Send(State& s):state(s){}s::AsyncSendResult send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov)override{if(flags)unused();auto& c=state.connection(socket);++c.sends;c.response_wire.append(buffer.buf,buffer.len);auto ports=state.application->ports();if(!ports.completion.post(state.port,buffer.len,0,&ov))unused();return {-1,997,0};}};
struct Logic:sp::SessionLogic {
 State& state;Connection& connection;sp::SessionView session;AppData& data;Events events;Random random;s::SessionCipherFactory crypto;MessageEffects effects;std::uint8_t show=0;std::string version;anadius::session_bridge::MessageHandler handler;
 Logic(State& s,Connection& c,sp::SessionView view,AppData& app):state(s),connection(c),session(view),data(app),effects(s,c,app),version(70000,c.encrypted?'b':'a'),handler({view.owner.cipher.encrypted_c4,view.owner.cipher.unknown_c5,show,version,view.owner.cipher,events},app.xml,crypto,random,effects){require(view.owner.cipher.encrypted_c4==0&&view.owner.cipher.unknown_c5==0&&view.owner.cipher.native().mode_00==0,"Production Profile logic receives actual factory owner after native025720 second reset");if(c.encrypted){view.owner.cipher.set_fixed_key_084c98();view.owner.cipher.encrypted_c4=1;c.client.set_fixed_key_084c98();}}
 ~Logic(){connection.logic_destroyed=true;}
 std::string greeting()override{connection.expected_greeting=handler.initial_message_02d540();return connection.expected_greeting;}
 std::string process(std::string& request)override{return m::process_message_033d70({session.owner.cipher.encrypted_c4,show,session.owner.cipher,events},request,data.xml,crypto,random,effects);}
};
bool clear(const sp::RegistryView& registry){bool result=true;for(const auto& v:registry.frames)result&=!v.identity&&!v.owner;for(const auto& v:registry.strings)result&=!v.identity&&!v.string;for(const auto& v:registry.reads)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.writes)result&=!v.identity&&!v.service&&!v.implementation;for(const auto& v:registry.finish)result&=!v.identity&&!v.owner;for(const auto& v:registry.queued)result&=!v.identity&&!v.owner;for(const auto& v:registry.messages)result&=!v.identity&&!v.owner;for(const auto& v:registry.environments)result&=!v.allocator_identity&&!v.source;return result;}
struct Policy:sp::Policy {
 State& state;Dispatcher& dispatcher;std::shared_ptr<Jobs> jobs;Environment env;AppData app;Guard guard;Logger logger;explicit Policy(State& s,Dispatcher& d,std::shared_ptr<Jobs> j):state(s),dispatcher(d),jobs(std::move(j)),env(s){}
 s::DispatcherEffects& guards()override{return guard;}s::WriteFinishApi& logging()override{return logger;}
 s::IoHandlerEnvironment session_environment(sp::SessionView v)override{s::IoHandlerEnvironment out;env.make(out,v.owner.identity());return out;}
 void accept_environment(s::IoHandlerEnvironment& out,std::uintptr_t)override{env.make(out);out.state_18=0;state.accept_pending=true;}
 void grow_deque(s::WriteFinishOwner&,std::uint64_t)override{unused();}void construct_length_error(s::ReadLengthError&,std::uintptr_t)override{unused();}[[noreturn]]void throw_length_error(s::ReadLengthError&)override{unused();}[[noreturn]]void thread_error(int)override{unused();}void cleanup_exit_waits()override{++state.exits;}[[noreturn]]void raise_write(s::Error&,std::uintptr_t)override{unused();}[[noreturn]]void bad_weak_ptr_026860()override{unused();}
 std::uint32_t exchange_stop(s::IocpSchedulerEntry& e,std::uint32_t n)override{return std::atomic_ref(e.high_34).exchange(n);}std::uint32_t exchange_wake(s::IocpSchedulerEntry& e,std::uint32_t n)override{const auto old=static_cast<std::uint32_t>(e.word_38);e.word_38=(e.word_38&0xffffffff00000000ULL)|n;return old;}
 void initialize(std::array<std::uintptr_t,2>& v)override{v.fill(0);++state.worker_init;}void finish(std::array<std::uintptr_t,2>& v)override{if(v!=std::array<std::uintptr_t,2>{})unused();++state.worker_finish;}[[noreturn]]void raise(s::Error,std::uintptr_t)override{unused();}
 s::IocpSchedulerEntry* completion_owner(sp::Application& application,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes)override{
  state.application=&application;auto ports=application.ports();if(!state.view_value){state.view_value.emplace(ports.view);state.view=&*state.view_value;}
  if(state.shutdown_posted&&&op.overlapped==state.accept_packets.back()){require(error.value==995&&application.snapshot().live_sessions==0&&application.snapshot().live_jobs==0,"Selected final-owner0 policy requires observed203/995 after both Profile sessions/jobs already reaped");state.terminal_selected=true;return nullptr;}
  if(!ports.accept.contains(&op)){const auto handle=ports.view.io.handle_for(&op);if(ports.view.io.kind(handle)==s::IoMemoryKind::Receive&&bytes==0&&!error.value){const auto id=ports.view.io.receive_operation(handle).io.handler.words_88_98[2];for(auto& c:state.connections)if(c&&c->owner_id==id)c->eof_seen=true;}}
  return owner;
 }
 void after_completion(sp::Application& application)override{
  auto ports=application.ports();if(state.terminal_selected){state.terminal_selected=false;state.shutdown_done=true;state.accept_pending=false;}
  if(jobs->deferred&&state.connections[0]&&state.connections[0]->eof_seen){auto v=application.session(state.connections[0]->token);require(v.owner.weak_self.control_08->strong_08==1&&application.snapshot().live_jobs==1&&state.overlap_seen,"After observed EOF only Profile-owned extra job strong retains first session before deferred runner");state.deferred_hold_seen=true;jobs->pump();}
  if(application.snapshot().live_sessions==0&&state.processed==2&&!state.shutdown_posted){require(clear(application.registry())&&state.connections[0]->logic_destroyed&&state.connections[1]->logic_destroyed&&ports.view.memory.snapshot().controls_destroyed==2,"Profile clears every fixed binding and logic before final observer delete without fixture cleanup");state.cleared_seen=true;state.shutdown_posted=true;const auto h=ports.accept.handle_for(state.accept_packets.back());ports.accept.note_cancel_requested(h);for(auto& b:ports.accept.dispatcher_bindings())if(b.overlapped_identity==state.accept_packets.back())s::publish_send_status_028b50(ports.view.scheduler,*b.operation,b,995,0,ports.completion,*errors);}
 }
 s::Api* errors=nullptr;
 void worker_returned(sp::Application& application,std::uint64_t count,const s::WorkerRunResult& result)override{
  auto ports=application.ports();require(count==11&&result.error&&result.error->value==0&&state.worker_init==1&&state.worker_finish==1,"Concrete Runtime/Profile execute eleven native packets for two overlapping full session request/response/EOF flows and final accept");
  require(state.overlap_seen&&state.deferred_hold_seen&&state.cleared_seen&&state.shutdown_done&&!application.snapshot().live_sessions&&!application.snapshot().live_jobs&&clear(application.registry()),"Bounded Profile owns and retires overlapping session/job registries without caller-built contexts or observer cleanup");
  for(auto& c:state.connections){auto response=c->response_wire;require(c->processed==1&&c->sends==1&&c->receives==2&&!response.empty()&&response.back()==0,"Each Profile session emits actual N5/GetConfig response and native queued send with terminal NUL");if(c->encrypted){response.pop_back();m::hex_decode_03ce30(response);require(c->client.decrypt_042480(response),"Independent second client decrypts Profile-managed AES response with automatic large-string header tracking");response.push_back(0);}require(response.find("GetConfigResponse")!=std::string::npos&&response.find("id=\"conn"+std::to_string(c->socket-200)+"\"")!=std::string::npos,"Per-session actual Router/XML response stays correlated to its own plain or encrypted connection");}
  require(state.exits==2&&state.job_starts==2&&state.job_detaches==2&&!jobs->deferred&&ports.view.memory.snapshot().controls_destroyed==2&&!ports.view.memory.snapshot().allocations&&!ports.view.io.snapshot().allocations&&!ports.view.scheduler.work_count_30,"Profile concrete frame/string/output/header/job ownership and native worker caches drain to actual zero allocations");state.worker_done=true;
 }
 bool external_quiescent(o::ServerView view,sp::Application& application)override{
  bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}require(outside&&state.worker_done&&state.shutdown_done&&!jobs->deferred&&!application.snapshot().live_jobs,"External policy checks actual joined/deferred job drain outside Root; worker counts alone are not attestation");
  unsigned relay=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* op=nullptr;const auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,op,0);const auto error=dispatcher.get_last_error();if(!got){if(error!=WAIT_TIMEOUT||op)unused();break;}if(op||bytes||key)unused();++relay;}
  require(relay==1&&state.gets==state.wait_scopes+state.initial_waits&&state.initial_waits==1&&!view.memory.snapshot().allocations,"Real stop relay/timeout observed outside Root after Profile reaps all owners, without operation lookup");state.queue_drained=true;return true;
 }
};
SOCKET WSAAPI open_socket(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){if(family!=2||type!=1||protocol!=6||info||group||flags!=1)unused();return 200+ ++running->factory_calls;}
BOOL WINAPI cancel_sdk(HANDLE,LPOVERLAPPED){unused();}
void move_executor(void* p,s::ExecutorFields& out,s::ExecutorFields& source,std::uintptr_t table){if(table!=0x180084928ULL)unused();++static_cast<State*>(p)->moves;out.context=source.context;out.zero_74=source.zero_74;out.self_reference_80=true;}
struct ExecutionPlatform:anadius::execution_recovered::Platform {unsigned locks=0,unlocks=0;std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}void acquire_srw_exclusive(std::uintptr_t&)override{++locks;}void release_srw_exclusive(std::uintptr_t&)override{++unlocks;}std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}};

void concrete_profile(){
 NativeDomain native;auto state=std::make_shared<State>();running=state.get();auto* observed=state.get();std::weak_ptr<State> lifetime=state;
 std::uint32_t index=::TlsAlloc();if(index==TLS_OUT_OF_INDEXES)unused();std::int32_t epoch=0;
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto errors=std::make_shared<Errors>(*state);auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto worker=std::make_shared<WorkerApi>(index,*dispatcher);auto jobs=std::make_shared<Jobs>(*state);auto policy=std::make_shared<Policy>(*state,*dispatcher,jobs);policy->errors=errors.get();
  sp::Dependencies profile_deps;profile_deps.state_lifetime=state;profile_deps.errors=errors;profile_deps.receive=std::make_shared<Receive>(*state);profile_deps.send=std::make_shared<Send>(*state);profile_deps.policy=policy;profile_deps.jobs=jobs;profile_deps.environment=policy->env.configuration();profile_deps.success={0x1111,server::ErrorCategory::system};profile_deps.misc={0x3333,server::ErrorCategory::asio_misc};profile_deps.sessions=1;profile_deps.jobs_capacity=4;profile_deps.headers=128;profile_deps.initial_output_slots=8;
  for(const auto key:{s::Category{0x1111,server::ErrorCategory::system},s::Category{0x2222,server::ErrorCategory::system},s::Category{0x3333,server::ErrorCategory::asio_misc}}){auto object=std::make_shared<Category>();object->identity_08=key.name_identity;profile_deps.categories.push_back({key,object});}
  profile_deps.logic=[s=state.get(),p=policy.get()](sp::SessionView view)->std::unique_ptr<sp::SessionLogic>{const auto socket=view.owner.socket_state.socket;if(socket<201||socket>202||s->connections[socket-201])unused();auto c=std::make_unique<Connection>();c->socket=socket;c->token=view.token;c->owner_id=view.owner.identity();c->encrypted=socket==202;auto* own=c.get();s->connections[socket-201]=std::move(c);return std::make_unique<Logic>(*s,*own,view,p->app);};
  auto profile=std::make_shared<sp::Profile>(std::move(profile_deps));std::weak_ptr<sp::Profile> profile_lifetime=profile;
  o::Dependencies server_deps;server_deps.state_lifetime=state;server_deps.errors=errors;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(*state);server_deps.executor=std::make_shared<Executor>(*state);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
  rt::Dependencies runtime_deps;runtime_deps.state_lifetime=state;runtime_deps.errors=errors;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.worker=worker;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.profile=profile;runtime_deps.tls_index=&index;runtime_deps.guard_epoch=&epoch;runtime_deps.generic_category={0x1111,server::ErrorCategory::system};runtime_deps.system_category={0x2222,server::ErrorCategory::system};runtime_deps.misc_category={0x3333,server::ErrorCategory::asio_misc};runtime_deps.executor={observed,move_executor};runtime_deps.capacity=32;
  anadius::bootstrap_backend::Resources resources{};rt::install_server(resources,native.root,std::move(server_deps),std::move(runtime_deps),32);auto* backend=dynamic_cast<o::OwnedServerBackend*>(resources.server.get());
  require(backend&&profile->snapshot().phase==sp::Phase::Fresh&&!observed->starts&&!observed->factory_calls&&!observed->job_starts,"Concrete Profile and installed Resources.server factories assemble passively before native startup");
  errors.reset();sockets.reset();dispatcher.reset();worker.reset();jobs.reset();policy.reset();profile.reset();state.reset();require(!lifetime.expired()&&!profile_lifetime.expired(),"Server/Runtime/Profile own all remaining SDK/message/environment/job policy lifetimes after external drops");
  std::uint16_t port=0;server::server_start_029ea0(*resources.server,port,ready,execution);
  for(const auto& c:observed->connections)require(c->greeting_gone&&c->sync_calls==3&&c->max_sync==65536&&c->greeting_wire==c->expected_greeting&&c->greeting_wire.size()>65536,"Concrete Profile builds/owns large actualMessageHandler greeting and native025720 frees it before per-session receive");
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1&&observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Actual SourceBackend server component publishes ready/port through existing native source path");
  require(observed->closed.size()==4&&observed->closed.back()==200&&std::count(observed->closed.begin(),observed->closed.end(),201)==1&&std::count(observed->closed.begin(),observed->closed.end(),202)==1&&std::count(observed->closed.begin(),observed->closed.end(),203)==1&&observed->moves==4&&observed->ports_closed==1&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1,"Two actual023000 factories and both native disposals close201/202 plus terminal203/listener exactly once before own port/gate/WSA teardown");
  const auto final=backend->snapshot();require(final.phase==o::Phase::Closed&&final.quiescence_attested&&observed->queue_drained&&!native.root->provider_leases()&&profile_lifetime.lock()->snapshot().phase==sp::Phase::Closed,"Runtime and concrete Profile retire owned graphs only after explicit external drain plus actual source counts");observed->application=nullptr;
 }
 require(lifetime.expired(),"Closed server/Runtime/Profile destruction releases all captured per-session logic/SDK/policy state without ownership cycle");running=nullptr;require(::TlsFree(index)!=FALSE,"Borrowed TLS selector remains valid until complete concrete graph destruction");require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Real process heap/PTD/FLS domain drains every Profile frame/string/map/block/control/IO allocation");
}
}
int main(){try{concrete_profile();std::cout<<"PASS: "<<checks<<" session-profile integration checks; actual concrete Profile owns all frame/read/write/response/string/registry contexts for two overlapping plain/AES sessions, deferred same-worker job retained across EOF and automatic observer cleanup; SDK/message/env/log/scheduling/drain policies explicit, no native detached-concurrency or fullResourcesFactory claim\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected profile integration exception\n";return 1;}}
