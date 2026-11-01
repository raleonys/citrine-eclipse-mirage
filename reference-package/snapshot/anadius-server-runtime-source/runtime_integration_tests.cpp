#include "resources.hpp"
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
namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected owned-server integration dependency");}
void weak_release(s::IoReference* c){if(c&&c->weak_0c.fetch_sub(1)==1)c->delete_slot_08();}
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
 bool first_error_delivered=false,terminal_selected=false,application_destroyed=false;unsigned factory_calls=0,environment_calls=0,profile_calls=0,worker_init=0,worker_finish=0;
 s::DispatcherApi* completion_api=nullptr;
 bool encrypted=false,greeting_gone=false,shutdown_posted=false,shutdown_done=false;
 std::uintptr_t greeting_data=0;unsigned sync_calls=0,receives=0,sends=0,processed=0;std::uint32_t max_sync=0;std::string greeting_wire,expected_greeting,response_wire;
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
 require(st.accept_memory&&st.parent_memory&&st.accept_memory->snapshot().pending==1&&st.parent_memory->snapshot().controls==(st.sdk_accepts>=2?1U:0U)&&st.parent_memory->snapshot().buffer_borrows>=1,"Accept SDK borrower pins raw storage without fabricating a SessionControl for its pending accept");
 auto bindings=st.accept_memory->dispatcher_bindings();auto storage=st.accept_memory->storage_bindings();std::size_t slot=bindings.size();
 for(std::size_t i=0;i<bindings.size();++i)if(bindings[i].overlapped_identity==overlapped)slot=i;
 require(slot<bindings.size()&&storage[slot].storage&&storage[slot].storage->bytes.size()==473,"Distinct typed accept OVERLAPPED has fixed base binding and exact473-byte native payload+marker allocation");
 const auto raw=storage[slot].storage->identity;
 require(reinterpret_cast<std::uintptr_t>(buffer)==raw+0x68&&std::to_integer<unsigned char>(storage[slot].storage->bytes[472])==118&&reinterpret_cast<std::uintptr_t>(overlapped)!=raw,"Accept address buffer is owned raw+68/288; marker118 is separate from actual typed OVERLAPPED");
 if(st.sdk_accepts<2&&!st.completion_api->post(st.port,0,0,overlapped))unused();
 st.accept_packets.push_back(overlapped);st.accept_raw.push_back(raw);st.accept_extents.push_back(473);++st.sdk_accepts;
 *bytes=0x4567;::WSASetLastError(st.encrypted&&st.sdk_accepts==2?1234:997);return st.encrypted&&st.sdk_accepts==2?TRUE:FALSE;
}
VOID PASCAL accept_addresses(PVOID buffer,DWORD received,DWORD local_size,DWORD remote_size,sockaddr** local,int* local_bytes,sockaddr** remote,int* remote_bytes){
 auto& st=*running;require(received==0&&local_size==0x90&&remote_size==0x90&&st.accept_memory->snapshot().pending==0,"Observed completion releases accept loan before native endpoint address extraction");
 auto* bytes=static_cast<std::byte*>(buffer);std::fill_n(bytes,288,std::byte{});auto* address=bytes+144;
 address[0]=static_cast<std::byte>(st.ipv6?23:2);address[2]=std::byte{0x12};address[3]=std::byte{0x34};address[st.ipv6?23:7]=std::byte{0x7f};
 *local=reinterpret_cast<sockaddr*>(bytes);*local_bytes=st.ipv6?28:16;*remote=reinterpret_cast<sockaddr*>(address);*remote_bytes=*local_bytes;++st.address_reads;
}
int WSAAPI update_context(SOCKET socket,int level,int option,const char* value,int length){
 auto& st=*running;s::Socket listener=0;std::memcpy(&listener,value,sizeof(listener));
 require(socket==202&&level==0xffff&&option==0x700b&&length==8&&listener==200,"Native0271a0 updates accepted context with captured listener pointer value and exact option700b/size8");++st.context_updates;return 0;
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
  require(socket==202&&buffers.size()==1&&!flags,"Actual02a200 uses live accepted socket and one synchronous greeting buffer");auto& b=buffers[0];state.max_sync=std::max(state.max_sync,static_cast<std::uint32_t>(b.len));if(!state.sync_calls)state.greeting_data=reinterpret_cast<std::uintptr_t>(b.buf);
  const auto count=state.sync_calls==0?std::min(static_cast<std::uint32_t>(b.len),30000U):state.sync_calls==1?0U:b.len;++state.sync_calls;state.greeting_wire.append(b.buf,count);::SetLastError(0);return {0,count};
 }std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Sockets:s::AcceptorApi {
 State& state;explicit Sockets(State& s):state(s){}
 s::Socket create_socket(std::int32_t family)override{require(family==(state.ipv6?23:2),"Actual020e20 opens selected family through synthetic socket SDK boundary");state.order.push_back("socket");return 200;}
 std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require((socket==200||socket==202)&&port==state.port,"Actual acceptor/accepted transfer attaches selected socket to same owned kernel completion port");return port;}
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
  auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&transferred,&token,&raw,timeout);
  if(raw&&!state.first_error_delivered&&raw==state.accept_packets.front()){state.first_error_delivered=true;::SetLastError(64);result=FALSE;}
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
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{return 0xabc;}
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
 State& state;AppData& data;MessageEffects(State& s,AppData& a):state(s),data(a){}
 int atoi_05ee08(std::string_view)override{unused();}bool registered_handler(std::uint32_t r,const m::XmlNode& n,m::XmlPrinter& p)override{++state.processed;return data.router.registered_handler(r,n,p);}
 [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
 void log_received(std::string_view)override{}void log_response(std::string_view)override{}void log_event(std::string_view)override{unused();}void log_bad_xml(std::string_view,std::string_view)override{unused();}void log_decryption_failure(std::string_view)override{unused();}
};
struct Harness;Harness* active=nullptr;void register_header(const s::ReceiveBufferString&);
void prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&);void construct(s::IoHandlerEnvironment&,std::uintptr_t,s::IoEnvironmentConstructorScratch&);
void response20(s::IoHandlerEnvironment&,std::uintptr_t,std::uintptr_t);void response88(s::IoHandlerEnvironment&,std::uintptr_t,s::ResponseExecutorScratch&);
void schedule_handler(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&);void schedule_response(s::IoHandlerEnvironment&,s::PostedFunctor*&);std::uintptr_t type_query();
struct Types:s::IoEnvironmentTypeApi {std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{if(a!=0x1008||b!=0x1800a25c8ULL)unused();return -1;}};
struct ProfileEnvironment {
 Table table;Types types;s::IoHandlerExecutor executor{prepare};s::IoHandlerSentinels sentinels{&table,0,&executor,table.destroy_identity};s::IoHandlerEnvironment source{{1,2},&table,1,0x456,&executor};
 std::array<s::IoEnvironmentTypeBinding,1> queries{{{0x456,type_query}}};std::array<s::IoEnvironmentFactoryBinding,1> factories{{{&executor,construct}}};s::IoEnvironmentConstructorContext constructor{types,queries,factories};std::array<s::IoEnvironmentSourceBinding,1> sources{};
};
struct WorkerControl:rt::WorkerControl {State& state;explicit WorkerControl(State& s):state(s){}
 std::uint32_t exchange_stop(s::IocpSchedulerEntry& e,std::uint32_t n)override{return std::atomic_ref(e.high_34).exchange(n);}
 std::uint32_t exchange_wake(s::IocpSchedulerEntry& e,std::uint32_t n)override{const auto old=static_cast<std::uint32_t>(e.word_38);e.word_38=(e.word_38&0xffffffff00000000ULL)|n;return old;}
 void initialize(std::array<std::uintptr_t,2>& v)override{v.fill(0);++state.worker_init;}void finish(std::array<std::uintptr_t,2>& v)override{if(v!=std::array<std::uintptr_t,2>{})unused();++state.worker_finish;}[[noreturn]]void raise(s::Error,std::uintptr_t)override{unused();}
};
struct Terminal:s::FrameReadTerminal,s::QueuedWriteTerminal {[[noreturn]]void bad_weak_ptr_026860()override{unused();}};
struct Category:s::WriteFinishCategory {void message_slot_10(s::WriteFinishString& value,std::int32_t)override{value={};value.size_10=3;value.storage_00[0]='e';}};
struct Logging:s::RecoveredQueuedWriteApi {
 s::SessionMemoryDomain& memory;s::WriteFinishStream stream{0x9000,-8};Logging(s::SessionMemoryDomain& m,s::QueuedWriteContext& q):RecoveredQueuedWriteApi(q),memory(m){}
 void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{memory.free_sized_04b0f8(p,n);}[[noreturn]]void invalid_parameter_05c974()override{unused();}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{return stream;}s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{return stream;}
 std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t n)override{return n;}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{}void flush_00e1e0(s::WriteFinishStream&)override{}
};
struct FrameApi:s::FrameReadApi {
 Harness& h;std::unique_ptr<s::FrameMessageClosure> closure;unsigned detached=0,freed=0;explicit FrameApi(Harness& x):h(x){}
 void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override;
 s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override{if(n!=48||closure)unused();closure=std::make_unique<s::FrameMessageClosure>();return closure.get();}
 std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::FrameMessageClosure*,std::uint32_t,std::uint32_t&)override;
 std::int32_t detach_04a458(const s::FrameThreadHandle& t)override{if(t.handle!=99||t.id!=57)unused();++detached;return 0;}
 [[noreturn]]void thread_error_04a648(int)override{unused();}void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;[[noreturn]]void invalid_parameter_05c974()override{unused();}
 void next_write_02a500(std::uintptr_t)override{unused();}s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}
};
struct ReadApi:s::RecoveredFrameReadApi {using RecoveredFrameReadApi::RecoveredFrameReadApi;void construct_length_error_024390(s::ReadLengthError&,std::uintptr_t)override{unused();}[[noreturn]]void throw_length_error_023930(s::ReadLengthError&)override{unused();}};
struct Receive:s::AsyncReceiveApi {Harness& h;explicit Receive(Harness& x):h(x){}s::AsyncReceiveResult receive(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override;};
struct Send:s::AsyncSendApi {Harness& h;explicit Send(Harness& x):h(x){}s::AsyncSendResult send(s::Socket,s::Buffer&,std::uint32_t,OVERLAPPED&)override;};
struct Effects:s::RecoveredComposedReadEffects {
 s::CoherentIoMemory& memory;Effects(s::IoCompletionEnvironment e,s::WorkerRunApi& a,s::CoherentIoMemory& m,std::span<s::HandlerScheduleBinding> schedule,s::AsyncSendApi& send,s::DispatcherApi& completion,Errors& errors,std::span<s::SendStreamBinding> streams,s::IoEnvironmentConstructorContext& constructor,std::span<s::IoEnvironmentSourceBinding> sources,Logging& log,std::span<s::WriteFinishOwnerBinding> owners,std::span<s::WriteFinishCategoryBinding> categories,std::span<s::WriteFinishAllocationHeader> headers)
 :RecoveredComposedReadEffects(e,a,m,schedule,send,completion,errors,streams,constructor,sources,log,owners,categories,headers),memory(m){}
 s::AsyncSendOperation& project_async_send(s::OperationStorage& storage)override{return memory.project_async_send(storage);}s::Category misc_category_0285b0()override{return {0x3333,server::ErrorCategory::asio_misc};}
};
struct Strings:s::BoundedMessageCoreStrings {s::ReceiveBufferContext& memory;explicit Strings(s::ReceiveBufferContext& c):BoundedMessageCoreStrings(c),memory(c){}void construct_response(s::ReceiveBufferString& out,std::string_view text)override;};
struct MessageApi:s::CompatibleMessageCoreApi {
 Logging& log;unsigned exits=0;MessageApi(s::ResponseDispatchContext& r,s::MessageCoreContext& c,Logging& l):CompatibleMessageCoreApi(r,c),log(l){}
 void destroy_empty_message_00aa50(s::ReceiveBufferString& v)override{if(v.capacity_18>15)log.free_sized_04b0f8(s::receive_buffer_address(v),v.capacity_18+1);v={};}void cleanup_exit_waits_04a47c()override{++exits;}
};
struct ResponseApi:s::RecoveredOwnedResponseApi {s::ReceiveBufferContext& strings;ResponseApi(s::OwnedResponseContext& c,s::ReceiveBufferContext& s):RecoveredOwnedResponseApi(c),strings(s){}s::ReceiveBufferString& copy_response_009eb0(s::ReceiveBufferString& out,s::ReceiveBufferString& source)override{auto& result=s::copy_response_string_009eb0(out,source,strings);register_header(out);return result;}};
struct QueueApi:s::ResponseQueueApi {s::SessionMemoryDomain& memory;explicit QueueApi(s::SessionMemoryDomain& m):memory(m){}void grow_deque_026670(s::WriteFinishOwner&,std::uint64_t)override{unused();}s::WriteFinishString* allocate_string_04b0bc(std::uint64_t n)override{if(n!=32)unused();return memory.allocate_output_block();}};
struct Harness:rt::Application {
 State& state;o::ServerView view;s::OwnedAcceptMemory& accept;s::WorkerRunApi& worker;s::DispatcherApi& completion;Errors& errors;ProfileEnvironment& env;std::uintptr_t server_identity;
 Table& table=env.table;s::IoHandlerExecutor& executor=env.executor;s::IoHandlerSentinels sentinels=env.sentinels;s::IoHandlerEnvironment& source=env.source;s::IoEnvironmentConstructorContext& constructor=env.constructor;std::span<s::IoEnvironmentSourceBinding> sources{env.sources};
 s::CoherentIoMemory& memory;s::IoCompletionEnvironment environment;
 std::array<s::WriteFinishAllocationHeader,64> headers{};Terminal terminal;Category misc;std::array<s::WriteFinishCategoryBinding,1> categories{{{{0x3333,server::ErrorCategory::asio_misc},&misc}}};
 s::HandlerSchedule handler_schedule{schedule_handler,nullptr};std::array<s::HandlerScheduleBinding,1> schedules{{{0x456,&handler_schedule}}};
 std::array<s::WriteFinishOwnerBinding,1> finish_owners{};s::QueuedWriteOwner queued_owner;std::array<s::QueuedWriteOwnerBinding,1> queued_owners{};s::QueuedWriteContext queued{queued_owners,terminal,nullptr};Logging log{view.memory,queued};
 Receive receive{*this};Send send{*this};s::PinnedAsyncReceiveApi pinned_receive{view.io,receive};s::PinnedAsyncSendApi pinned_send{view.io,send};
 s::AsyncSendService send_service{&view.scheduler};s::SendSocketImplementation send_implementation;std::array<s::SendStreamBinding,1> send_streams{};
 Effects effects{environment,worker,memory,schedules,pinned_send,completion,errors,send_streams,constructor,sources,log,finish_owners,categories,headers};
 FrameApi frame_api{*this};std::array<s::FrameReadOwnerBinding,1> owners{};s::FrameReadContext frame{view.memory.buffer_context(),effects,frame_api,owners,categories,headers,terminal,{0x1111,server::ErrorCategory::system},nullptr};ReadApi read_api{frame};
 s::AsyncReceiveService read_service{&view.scheduler};s::ReceiveSocketImplementation read_implementation;std::array<s::ReceiveStringBinding,1> strings{};std::array<s::ReceiveStreamBinding,1> read_streams{};
 s::AsyncReceiveContext receive_context{environment,worker,memory.posted(),pinned_receive,completion,errors,memory};s::ComposedReadContext read{view.memory.buffer_context(),receive_context,effects,read_api,strings,read_streams};
 s::OwnedResponseContext owned_response{environment.storage,worker,memory,frame,nullptr};ResponseApi response_api{owned_response,view.memory.buffer_context()};QueueApi queue_api{view.memory};s::ResponseQueueContext queue{frame,queue_api,finish_owners,&queued};
 std::array<s::ResponseExecutorBinding,1> response_executors{{{&executor,response20,response88}}};s::ResponseSchedule response_schedule{schedule_response,nullptr};std::array<s::ResponseScheduleBinding,1> response_schedules{{{0x456,&response_schedule}}};s::ResponseDispatchContext responses{frame,sentinels,response_executors,response_schedules,response_api,&queue};
 AppData app;Events events;Random random;s::SessionCipherFactory crypto;s::SessionCipher client;MessageEffects message_effects{state,app};std::uint8_t show=0;std::string version=std::string(70000,'v');
 std::unique_ptr<anadius::session_bridge::MessageHandler> handler;std::unique_ptr<s::MessageCoreSession> core_session;std::array<s::MessageCoreBinding,1> core_bindings{};Strings core_strings{view.memory.buffer_context()};s::MessageCoreContext core{core_bindings,core_strings};MessageApi message_api{responses,core,log};
 s::MessageDispatchOwner message_owner;std::array<s::MessageDispatchOwnerBinding,1> message_owners{};std::array<s::MessageEnvironmentCopyBinding,1> copies{{{&table,copy_env}}};s::MessageDispatchContext message{frame,message_api,message_owners,copies,{}};
 s::SessionOwner* owner=nullptr;s::IoReference* observer=nullptr;s::OwnedNoopControlFactory control_factory;
 DispatchEffects guard{view.io,environment,worker,errors,effects};WorkerControl worker_control{state};
 static void copy_env(s::IoHandlerEnvironment& d,s::IoHandlerEnvironment& source){d.payload_00=source.payload_00;d.state_18=source.state_18;}
 Harness(State& s,rt::Ports ports,Errors& e,ProfileEnvironment& profile):state(s),view(ports.view),accept(ports.accept),worker(ports.worker),completion(ports.completion),errors(e),env(profile),server_identity(ports.server_identity),memory(ports.memory),environment(ports.environment){active=this;misc.identity_08=0x3333;frame.read=&read;effects.bind_read_context(read);queued.write=&effects.write_context();owned_response.response=&responses;state.view=&view;state.accept_memory=&accept;state.parent_memory=&view.memory;state.completion_api=&completion;}
 ~Harness(){active=nullptr;state.application_destroyed=true;state.view=nullptr;state.accept_memory=nullptr;state.parent_memory=nullptr;state.completion_api=nullptr;}
 rt::Bindings bindings()override{return {frame,log,effects,guard,worker_control,categories,{},&effects.write_context()};}
 void construct_accept_environment(s::IoHandlerEnvironment& out,std::uintptr_t allocator)override{require(allocator==server_identity+0x68,"Concrete Runtime/native270e0 supplies stable server allocator ADDRESS+68");out={{1,2},&table,0,0,nullptr};state.accept_pending=true;}
 void reset_pending_owner(s::AcceptPending& pending)override{s::replace_owner_0237f0(pending.object.cancellation,0,control_factory);++state.pending_resets;}
 s::IocpSchedulerEntry* completion_owner(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override;
 void after_completion()override;void worker_returned(std::uint64_t,const s::WorkerRunResult&)override;
 std::string text(const s::ReceiveBufferString& value){if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};auto span=view.memory.buffer_context().memory.view(s::receive_buffer_address(value),value.size_10);return {reinterpret_cast<const char*>(span.data()),span.size()};}
 void copy(s::ReceiveBufferString& out,std::string_view value){core_strings.construct_response(out,value);}
 s::ReceiveBufferString& greeting(s::SessionOwner& object,s::ReceiveBufferString& output)override{
  owner=&object;observer=object.weak_self.control_08;observer->weak_0c.fetch_add(1);const auto id=object.identity();
  require(observer->strong_08==1&&observer->weak_0c==3&&object.cipher.encrypted_c4==0&&object.cipher.unknown_c5==0&&object.cipher.native().mode_00==0,"Actual023000 output reaches025720 second-reset defined-field transfer before application greeting");
  owners[0]={id,&object.weak_self};strings[0]={id+0xa8,&object.input};sources[0]={id+0x78,&source};
  read_implementation={object.socket_state.socket,object.socket_state.socket_state,id,observer};read_streams[0]={id+0x10,&read_service,&read_implementation};
  send_implementation={object.socket_state.socket,object.socket_state.socket_state,id,observer};send_streams[0]={id+0x10,&send_service,&send_implementation,id+0x78};
  auto map=view.memory.allocate_output_map(8);object.output.fields={map,8,0,0};finish_owners[0]={id,&object.output.fields};queued_owner={id,observer,&object.output.fields,&send_streams[0]};queued_owners[0]={id,&queued_owner};
  message_owner={&object.weak_self,&source};message_owners[0]={id,&message_owner};
  handler=std::make_unique<anadius::session_bridge::MessageHandler>(anadius::session_bridge::StateReferences{object.cipher.encrypted_c4,object.cipher.unknown_c5,show,version,object.cipher,events},app.xml,crypto,random,message_effects);
  state.expected_greeting=handler->initial_message_02d540();copy(output,state.expected_greeting);
  if(state.encrypted){object.cipher.set_fixed_key_084c98();object.cipher.encrypted_c4=1;client.set_fixed_key_084c98();}
  core_session=std::make_unique<s::MessageCoreSession>(s::MessageCoreSession{{object.cipher.encrypted_c4,show,object.cipher,events},app.xml,crypto,random,message_effects});core_bindings[0]={id+0xf0,core_session.get()};return output;
 }
 [[noreturn]]void raise_write(s::Error&,std::uintptr_t)override{unused();}
};
void Strings::construct_response(s::ReceiveBufferString& out,std::string_view value){out={};if(value.size()<=15){out.size_10=value.size();std::copy(value.begin(),value.end(),out.storage_00.begin());return;}s::resize_receive_buffer_010040(out,value.size(),0,memory);auto bytes=memory.memory.view(s::receive_buffer_address(out),value.size());std::copy(value.begin(),value.end(),bytes.begin());register_header(out);}
void register_header(const s::ReceiveBufferString& value){if(value.capacity_18+1<4096||value.capacity_18<=15)return;auto& h=*active;const auto address=s::receive_buffer_address(value)-8;auto data=h.view.memory.buffer_context().memory.view(address,8);std::uintptr_t base=0;for(unsigned i=0;i<8;++i)base|=std::uintptr_t(data[i])<<(8*i);for(auto& entry:h.headers)if(!entry.address||entry.address==address){entry={address,base};return;}unused();}
void make_environment(s::IoHandlerEnvironment& out){auto& h=*active;out={{1,2},&h.table,1,0x456,&h.executor};}
std::uintptr_t type_query(){return 0x1000;}
void prepare(s::IoHandlerEnvironment& out,std::uintptr_t q,s::IoPrepareScratch&){if(q!=0xabc)unused();make_environment(out);}
void construct(s::IoHandlerEnvironment& out,std::uintptr_t q,s::IoEnvironmentConstructorScratch&){if(q!=0xabc)unused();make_environment(out);}
void response20(s::IoHandlerEnvironment& out,std::uintptr_t q,std::uintptr_t tag){if(q!=0xabc||tag!=0x180084425ULL)unused();make_environment(out);}
void response88(s::IoHandlerEnvironment& out,std::uintptr_t q,s::ResponseExecutorScratch&){if(q!=0xabc)unused();make_environment(out);}
void schedule_handler(s::IoHandlerEnvironment&,s::OwnedHandlerTask*& slot){auto* task=slot;if(!task)unused();slot=nullptr;task->call_slot_00(1);} // selected serialized scheduler, not native concurrency
void schedule_response(s::IoHandlerEnvironment&,s::PostedFunctor*& slot){auto& h=*active;s::PostedExecutor executor{h.view.identity.value,&h.view.scheduler,1};s::dispatch_posted_022e40(executor,slot,h.environment.storage,h.worker,h.completion,h.memory.posted());}
void FrameApi::construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t p,std::uint64_t n){auto bytes=h.view.memory.buffer_context().memory.view(p,n);h.copy(out,{reinterpret_cast<const char*>(bytes.data()),bytes.size()});}
std::uintptr_t FrameApi::begin_frame_thread_05e8e0(std::uintptr_t a,std::uint32_t b,std::uintptr_t entry,s::FrameMessageClosure* p,std::uint32_t flags,std::uint32_t& id){if(a||b||flags||entry!=0x180021770ULL||p!=closure.get())unused();id=57;require(h.observer->strong_08>0,"Raw frame job is executed while current native receive handler explicitly holds owner alive");if(s::message_thread_entry_021770(*p,h.message))unused();return 99;}
void FrameApi::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){if(closure&&p==reinterpret_cast<std::uintptr_t>(closure.get())){if(n!=48||closure->message_00.size_10)unused();closure.reset();++freed;return;}h.view.memory.free_sized_04b0f8(p,n);}
s::AsyncReceiveResult Receive::receive(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov){
 if(socket!=202||flags)unused();auto& st=h.state;++st.receives;
 if(st.receives==1){
  const auto handle=h.view.io.handle_for(&ov);auto& binding=h.accept.storage_bindings()[handle.slot];const auto packet=h.accept.dispatcher_bindings()[handle.slot];bool rejected=false;
  try{(void)h.memory.project_async_send(*binding.storage);}catch(const std::logic_error&){rejected=true;}
  require(rejected&&h.view.io.kind(handle)==s::IoMemoryKind::Receive&&h.accept.dispatcher_bindings()[handle.slot].operation==packet.operation&&h.accept.dispatcher_bindings()[handle.slot].overlapped_identity==&ov&&h.view.io.snapshot().pending==1,"Coherent send reproject cannot retire/change an actual pinned receive packet or generation");
  require(binding.identity==st.accept_raw[0]&&binding.storage->bytes.size()==473&&!h.accept.contains(packet.operation),"Actual cached accept473 becomes receive225 without losing original extent or retaining ended accept projection");
  try{(void)h.view.memory.buffer_context().memory.view(st.greeting_data,1);}catch(const s::UnsupportedInput&){st.greeting_gone=true;}
  std::string request="<LSX><Request id=\"flow\" recipient=\"EbisuSDK\"><GetConfig/></Request></LSX>";if(st.encrypted){if(!h.client.encrypt_042610(request))unused();m::hex_encode_03d190(request);}request.push_back(0);if(request.size()>buffer.len)unused();std::memcpy(buffer.buf,request.data(),request.size());if(!h.completion.post(h.view.scheduler.completion.port,static_cast<std::uint32_t>(request.size()),0,&ov))unused();
 }else{if(st.receives!=2)unused();if(!h.completion.post(h.view.scheduler.completion.port,0,0,&ov))unused();}return {-1,997,0};
}
s::AsyncSendResult Send::send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov){if(socket!=202||flags)unused();++h.state.sends;
 const auto handle=h.view.io.handle_for(&ov);auto& binding=h.accept.storage_bindings()[handle.slot];const auto packet=h.accept.dispatcher_bindings()[handle.slot];bool rejected=false;
 try{(void)h.memory.project_async_receive(*binding.storage);}catch(const std::logic_error&){rejected=true;}
 require(rejected&&h.view.io.kind(handle)==s::IoMemoryKind::Send&&h.accept.dispatcher_bindings()[handle.slot].operation==packet.operation&&h.accept.dispatcher_bindings()[handle.slot].overlapped_identity==&ov,"Coherent receive reproject cannot retire/change an actual pinned send packet or generation");
 require(binding.identity==h.state.accept_raw[0]&&binding.storage->bytes.size()==473,"Actual native posted-response cleanup reuses original accept473 allocation for send209 through coherent cache0");
 h.state.response_wire.append(buffer.buf,buffer.len);if(!h.completion.post(h.view.scheduler.completion.port,buffer.len,0,&ov))unused();return {-1,997,0};}


s::IocpSchedulerEntry* Harness::completion_owner(s::IocpSchedulerEntry* source,s::CancelOperation& op,const s::Error& error,std::uint32_t){
 if(state.shutdown_posted&&&op.overlapped==state.accept_packets.back()){
  require(error.value==995&&observer&&observer->strong_08==0&&state.processed==1&&!frame_api.closure,"Runtime completion policy selects null only for observed final203/995 after session/raw-job drain");state.terminal_selected=true;return nullptr;
 }
 return source;
}
void Harness::after_completion(){
 if(state.terminal_selected){require(!accept.snapshot().pending&&!accept.snapshot().projections,"Actual runtime router completes final owner0 accept cleanup before application drain flag");state.shutdown_done=true;state.accept_pending=false;state.terminal_selected=false;}
 if(observer&&observer->strong_08==0&&!state.shutdown_posted){
  require(state.accept_packets.size()==3&&state.sends==1&&message_api.exits==1&&state.factory_calls==3,"Actual retry/fullflow response and EOF precede final203 pending-accept shutdown intent");state.shutdown_posted=true;
  const auto handle=accept.handle_for(state.accept_packets.back());accept.note_cancel_requested(handle);
  for(auto& binding:accept.dispatcher_bindings())if(binding.overlapped_identity==state.accept_packets.back())s::publish_send_status_028b50(view.scheduler,*binding.operation,binding,995,0,completion,errors);
 }
}
void Harness::worker_returned(std::uint64_t count,const s::WorkerRunResult& result){
 require(count==7&&result.error&&result.error->value==0&&state.worker_init==1&&state.worker_finish==1,"Concrete production Runtime executes actual0298b0/027c10 across10053retry/success/read/ownedresponse/EOF/send/finalaccept");
 require(state.first_error_delivered&&state.sdk_accepts==3&&state.factory_calls==3&&state.moves==2&&state.shutdown_done&&!state.accept_pending,"Concrete runtime binds recovered29660 retry before full021a10/025720 factory/270e0 rearm and selected terminal cleanup");
 require(state.processed==1&&frame_api.detached==1&&frame_api.freed==1&&message_api.exits==1&&!frame_api.closure,"Profile's selected serialized raw job runs actual021770/028d30/N5core/Router/GetConfig and releases exact frame closure");
 auto response=state.response_wire;require(!response.empty()&&response.back()==0,"Production runtime retains response terminal NUL through actual owned response/queued native send");
 if(state.encrypted){response.pop_back();m::hex_decode_03ce30(response);require(client.decrypt_042480(response),"Independent client decrypts actual runtime-owned session cipher response");response.push_back(0);}
 require(response.find("GetConfigResponse")!=std::string::npos&&response.find("id=\"flow\"")!=std::string::npos&&state.receives==2&&state.sends==1,"Native response021420/022280/028270/02a500/021fd0/0261d0/025a70 consumes real XMLReader/Router result");
 require(!view.io.snapshot().allocations&&!accept.snapshot().pending&&!accept.snapshot().projections&&!view.scheduler.work_count_30&&worker.tls_get(environment.storage.tls_index)==nullptr,"Concrete runtime restores TLS and frees coherent0/1/4/5 native worker caches after all observed packets");
 require(observer&&observer->strong_08==0&&observer->weak_0c==1&&view.memory.snapshot().allocations==1&&view.memory.snapshot().controls_destroyed==0,"Actual026570 frees session resources before deliberate observer weak retains only exact factory control");
 weak_release(observer);observer=nullptr;owner=nullptr;
 require(view.memory.snapshot().controls_destroyed==1&&!view.memory.snapshot().allocations&&view.memory.snapshot().successful_allocations==view.memory.snapshot().successful_frees,"Final observer017420/026290 runs under existing whole-worker RootScope and drains all owned native allocations");
 state.session_deleted=true;state.worker_done=true;state.job_done=true;state.accept_packets_drained=true;
}
struct Profile:rt::Profile {
 State& state;Errors& errors;Dispatcher& dispatcher;ProfileEnvironment env;explicit Profile(State& s,Errors& e,Dispatcher& d):state(s),errors(e),dispatcher(d){}
 rt::Environment environment(o::ServerView,std::uintptr_t)override{++state.environment_calls;return {env.sentinels,env.constructor,env.sources};}
 std::unique_ptr<rt::Application> create(rt::Ports ports)override{++state.profile_calls;return std::make_unique<Harness>(state,ports,errors,env);}
 bool external_quiescent(o::ServerView view,rt::Application& application)override{
  auto& graph=dynamic_cast<Harness&>(application);bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}
  require(outside&&state.worker_done&&state.job_done&&state.session_deleted&&state.accept_packets_drained&&state.shutdown_done&&!graph.observer,"External transport attestation runs outside Root only after actual worker/session/raw-job completion");
  unsigned relays=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;const auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,raw,0);const auto error=dispatcher.get_last_error();if(!got){if(error!=WAIT_TIMEOUT||raw)unused();break;}if(raw||key||bytes)unused();++relays;}
  require(relays==1&&state.gets==state.wait_scopes&&!view.memory.snapshot().allocations&&view.memory.snapshot().provider_leases==2,"Selected lower observes real stop-relay/timeout outside Root before runtime retires accept provider; no operation lookup outside admission");state.queue_drained=true;return true;
 }
};
SOCKET WSAAPI open_socket(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){
 require(family==2&&type==1&&protocol==6&&!info&&!group&&flags==1,"Concrete runtime native029df0/029660 uses injected exact WSASocketW boundary only");return 200+ ++running->factory_calls;
}
BOOL WINAPI cancel_sdk(HANDLE,LPOVERLAPPED){unused();}
void move_executor(void* p,s::ExecutorFields& d,s::ExecutorFields& source,std::uintptr_t table){auto& state=*static_cast<State*>(p);if(table!=0x180084928ULL)unused();++state.moves;d.context=source.context;d.zero_74=source.zero_74;d.self_reference_80=true;}
struct ExecutionPlatform:anadius::execution_recovered::Platform {
 unsigned locks=0,unlocks=0;std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}void acquire_srw_exclusive(std::uintptr_t&)override{++locks;}void release_srw_exclusive(std::uintptr_t&)override{++unlocks;}
 std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}
};
void production_runtime(bool encrypted){
 NativeDomain native;auto state=std::make_shared<State>();state->encrypted=encrypted;running=state.get();auto* observed=state.get();std::weak_ptr<State> lifetime=state;
 const auto index=::TlsAlloc();if(index==TLS_OUT_OF_INDEXES)unused();std::uint32_t tls_index=index;std::int32_t epoch=0;
 ExecutionPlatform platform;anadius::execution_recovered::MutexState mutex{};anadius::execution_recovered::initialize_mutex_04a698(mutex,2);std::uint8_t ready=0,other=0,last=0;std::int64_t frequency=0;anadius::execution_recovered::Execution execution{mutex,{ready,other,last},frequency,platform};observed->ready=&ready;
 {
  auto errors=std::make_shared<Errors>(*state);auto sockets=std::make_shared<Sockets>(*state);auto dispatcher=std::make_shared<Dispatcher>(*state);auto worker=std::make_shared<WorkerApi>(tls_index,*dispatcher);auto profile=std::make_shared<Profile>(*state,*errors,*dispatcher);std::weak_ptr<Profile> profile_lifetime=profile;
  o::Dependencies server_deps;server_deps.state_lifetime=state;server_deps.errors=errors;server_deps.sockets=sockets;server_deps.dispatcher=dispatcher;
  s::IocpConstructorImports port_imports;port_imports.create_completion_port=create_port;port_imports.close_handle=close_port;server_deps.iocp=std::make_shared<s::NativeIocpConstructorApi>(port_imports);server_deps.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});server_deps.cancellation=std::make_shared<Cancel>(*state);server_deps.executor=std::make_shared<Executor>(*state);server_deps.kernel={startup,cleanup,local_name,environment,gate_delete};
  rt::Dependencies runtime_deps;runtime_deps.state_lifetime=state;runtime_deps.errors=errors;runtime_deps.sockets=sockets;runtime_deps.dispatcher=dispatcher;runtime_deps.worker=worker;runtime_deps.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,accept_addresses,update_context,::WSAGetLastError});runtime_deps.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});runtime_deps.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);runtime_deps.profile=profile;runtime_deps.tls_index=&tls_index;runtime_deps.guard_epoch=&epoch;runtime_deps.generic_category={0x1111,server::ErrorCategory::system};runtime_deps.system_category={0x2222,server::ErrorCategory::system};runtime_deps.misc_category={0x3333,server::ErrorCategory::asio_misc};runtime_deps.executor={observed,move_executor};runtime_deps.capacity=16;
  anadius::bootstrap_backend::Resources resources{};rt::install_server(resources,native.root,std::move(server_deps),std::move(runtime_deps),16);auto* backend=dynamic_cast<o::OwnedServerBackend*>(resources.server.get());
  require(backend&&observed->starts==0&&observed->factory_calls==0&&observed->profile_calls==0&&observed->environment_calls==0,"Actual Resources.server factory installs owned concrete Runtime without starting sockets/workers/application graph");
  errors.reset();sockets.reset();dispatcher.reset();worker.reset();profile.reset();state.reset();
  require(!lifetime.expired()&&!profile_lifetime.expired(),"Factory-owned server/runtime dependencies retain profile and shared SDK/application state after external strong drops");
  std::uint16_t port=0;server::server_start_029ea0(*resources.server,port,ready,execution);
  require(observed->greeting_gone&&observed->sync_calls==3&&observed->max_sync==65536&&observed->greeting_wire==observed->expected_greeting&&observed->greeting_wire.size()>65536,"Native025720 greeting lifetime and short/zero-progress/64KiB chunk behavior survive concrete Runtime assembly");
  require(port==12345&&ready==1&&!other&&!last&&platform.locks==1&&platform.unlocks==1&&observed->environment_key=="EALsxPort"&&observed->environment_value=="12345","Installed concrete server executes actual029ea0 readiness/environment/Execution publication");
  require(observed->closed==std::vector<s::Socket>({201,202,203,200})&&observed->moves==2&&observed->factory_calls==3&&observed->ports_closed==1&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1,"Actual retry201close, session202dispose, terminal203close and listener teardown release own port/gates/WSA once");
  const auto final=backend->snapshot();require(final.phase==o::Phase::Closed&&final.quiescence_attested&&observed->application_destroyed&&observed->profile_calls==1&&observed->environment_calls==1&&!native.root->provider_leases(),"Explicit external drain plus actual source counts permit Runtime graph retirement and break Core ownership cycle before Closed");
 }
 require(lifetime.expired(),"Closed Resources.server destruction releases complete retained source profile/SDK state graph");running=nullptr;require(::TlsFree(tls_index)!=FALSE,"Caller-owned TLS selector remains alive through final graph destruction");
 require(native.root->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Real process heap/FLS backing closes with all observed source allocations/frees balanced");
}
}
int main(){try{production_runtime(false);production_runtime(true);std::cout<<"PASS: "<<checks<<" server-runtime integration checks; actual installed Resources.server concreteRuntime, native10053retry and full factory/frame/N5message/ownedresponse/send/disposal through realownIOCP/heap/FLS; injected networkSDKs and explicit serialized profile/terminalowner0 policy, no nativeABI/quiescence claim\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected runtime integration exception\n";return 1;}}
