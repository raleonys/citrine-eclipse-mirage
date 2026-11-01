#include <winsock2.h>

#include "factory.hpp"
#include "../anadius-services-factory-source/factory.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"

#include "../anadius-shared-log-source/stream.hpp"

#include "../anadius-accept-stop-source/stop.hpp"

#include "../anadius-application-worker-source/worker.hpp"

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

namespace of=anadius::online_factory;namespace bn=anadius::bootstrap_native_recovered;namespace sf=anadius::services_factory;namespace sc=anadius::services_completion;

namespace s=anadius::socket_recovered;namespace o=anadius::owned_server;namespace server=anadius::server_recovered;

namespace m=anadius::messages_recovered;namespace rt=anadius::server_runtime;namespace sp=anadius::session_profile;namespace np=anadius::native_profile;namespace cp=anadius::session_policy;namespace ag=anadius::application_graph;namespace wh=anadius::worker_handoff;namespace td=anadius::thread_domain;namespace t=anadius::threads_recovered;namespace aw=anadius::application_worker;namespace bb=anadius::bootstrap_backend;namespace br=anadius::bootstrap_runtime;namespace ast=anadius::accept_stop;

std::atomic<unsigned> checks=0;

void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}

[[noreturn]]void unused(){throw std::logic_error("Unselected owned-server integration dependency");}

// Own SDK observation is synchronized across controller, worker and its actual

// FLS callback; no legacy fixture globals/maps are accessed across threads.

struct HeapObservation {

 mutable std::mutex gate;std::map<void*,SIZE_T> blocks;unsigned allocations=0,frees=0;std::atomic<unsigned> slot_allocations=0,slot_releases=0;bool valid=true;std::vector<DWORD> allocation_threads,free_threads;struct Payload{void* raw;SIZE_T bytes;bool freed=false;DWORD freed_by=0;};bool watch_payloads=false;DWORD payload_controller=0;std::vector<Payload> payloads;

 std::size_t count()const{std::lock_guard lock(gate);return blocks.size();}

 bool balanced()const{std::lock_guard lock(gate);return valid&&blocks.empty()&&allocations==frees;}

 bool used_worker(DWORD id)const{std::lock_guard lock(gate);return std::find(allocation_threads.begin(),allocation_threads.end(),id)!=allocation_threads.end()&&std::find(free_threads.begin(),free_threads.end(),id)!=free_threads.end();}

};

HeapObservation* heap_observation=nullptr;

void* WINAPI heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){

 auto& state=*heap_observation;std::lock_guard lock(state.gate);if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))state.valid=false;

 auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw){if(!state.blocks.emplace(raw,bytes).second)state.valid=false;++state.allocations;state.allocation_threads.push_back(::GetCurrentThreadId());if(state.watch_payloads&&::GetCurrentThreadId()==state.payload_controller&&(bytes==8||bytes==16)&&flags==0)state.payloads.push_back({raw,bytes,false,0});}::SetLastError(error);return raw;

}

BOOL WINAPI heap_free(HANDLE heap,DWORD flags,LPVOID value){

 auto& state=*heap_observation;std::lock_guard lock(state.gate);if(heap!=::GetProcessHeap()||flags||!state.blocks.contains(value))state.valid=false;

 const auto result=::HeapFree(heap,flags,value);const auto error=::GetLastError();if(result){if(state.blocks.erase(value)!=1)state.valid=false;++state.frees;state.free_threads.push_back(::GetCurrentThreadId());for(auto& p:state.payloads)if(p.raw==value&&!p.freed){p.freed=true;p.freed_by=::GetCurrentThreadId();break;}}::SetLastError(error);return result;

}

DWORD WINAPI allocate_fls(PFLS_CALLBACK_FUNCTION callback){++heap_observation->slot_allocations;return ::FlsAlloc(callback);}

BOOL WINAPI release_fls(DWORD index){++heap_observation->slot_releases;return ::FlsFree(index);}

HMODULE WINAPI unused_load(LPCWSTR,HANDLE,DWORD){unused();}FARPROC WINAPI unused_procedure(HMODULE,LPCSTR){unused();}BOOL WINAPI unused_release(HMODULE){unused();}std::uint32_t unused_flags(){unused();}

struct NativeDomain {

 HeapObservation memory;std::shared_ptr<td::DomainStorage> root;

 NativeDomain(){if(heap_observation)unused();heap_observation=&memory;auto identity=std::make_shared<std::array<std::byte,64>>();td::Seed seed{0x5a1b2c3d4e5f6071ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity.get()),identity};td::Apis apis;apis.heap.heap_alloc=heap_allocate;apis.heap.heap_free=heap_free;apis.fls.allocate=allocate_fls;apis.fls.release=release_fls;apis.runtime={unused_load,unused_procedure,unused_release,::GetLastError,unused_flags};root=td::DomainStorage::create(seed,apis);if(!root->initialize())unused();}

 ~NativeDomain(){if(root){try{if(!root->close())std::terminate();}catch(...){std::terminate();}root.reset();}heap_observation=nullptr;}

};

enum class OnlineMode {Success,VersionMissing,VersionMismatch,FactoryMissing,Offline};
struct PrivateObject {std::uintptr_t table=0;bool destroyed=false;};
struct State {

 OnlineMode online_mode=OnlineMode::Success;of::Online* online_component=nullptr;const anadius::execution_recovered::Execution* online_execution=nullptr;std::array<std::uintptr_t,5> online_table{};PrivateObject online_object{};std::byte module_token{};std::vector<char> export_events;std::vector<std::string> export_names;unsigned online_loads=0,version_calls=0,factory_calls=0,status_calls=0,getter_calls=0,code_acquired=0,code_released=0,dialogs=0,quiescence_calls=0;bool code_valid=true,online_quiescent=false;std::array<std::string,3> profile{std::string(1024,'U'),std::string(1536,'P'),std::string(2048,'I')};std::shared_ptr<b8_test::MemoryFilesState> online_files=std::make_shared<b8_test::MemoryFilesState>();std::shared_ptr<std::stringbuf> online_log=std::make_shared<std::stringbuf>();
 const td::DomainStorage* expected_domain=nullptr;DWORD controller=0,worker=0;anadius::configuration::State* config=nullptr;bb::RuntimeSeed* runtime=nullptr;std::atomic<aw::Worker*> component=nullptr;std::weak_ptr<ast::Stop> stop;

 HANDLE reader=nullptr,writer=nullptr,read_done=nullptr,allow_get=nullptr,entered_get=nullptr,armed=nullptr,accepted=nullptr;bool success_race=false,reader_closed=false,accepted_closed=false,first_get=true;

 std::atomic<bool> other_admission_closed=false,host_barrier=false,online_barrier=false,drained=false;std::atomic<unsigned> physical_gets=0;unsigned submissions=0,kernel_packets=0,zero_packets=0,source_packets=0,after_calls=0,decode_calls=0,update_calls=0,sessions=0,io_unselected=0,starts=0,cleanups=0,ports_created=0,ports_closed=0,gate_inits=0,gate_deletes=0;DWORD kernel_status=0;std::uint32_t kernel_bytes=0;OVERLAPPED* ov=nullptr;std::uintptr_t address=0,port=0;sp::Application* application=nullptr;

};

State* running=nullptr;

bool live_range(HeapObservation& observation,std::uintptr_t address,std::size_t bytes){std::lock_guard lock(observation.gate);for(const auto& [p,n]:observation.blocks){auto base=reinterpret_cast<std::uintptr_t>(p);if(address>=base&&address-base<=n&&bytes<=n-(address-base))return true;}return false;}

int WSAAPI startup(WORD version,LPWSADATA data){require(version==MAKEWORD(2,2),"Server requests ownWSA2.2 scope without creating a network socket");++running->starts;return ::WSAStartup(version,data);}int WSAAPI cleanup(){++running->cleanups;return ::WSACleanup();}

int WSAAPI local_name(SOCKET socket,sockaddr* out,int* bytes){require(socket==reinterpret_cast<SOCKET>(running->reader)&&*bytes>=28,"Synthetic acceptor localendpoint uses EXACTowned namedpipe listener handle");std::memset(out,0,*bytes);out->sa_family=2;reinterpret_cast<unsigned char*>(out)[2]=0x30;reinterpret_cast<unsigned char*>(out)[3]=0x39;*bytes=28;return 0;}

BOOL WINAPI environment(LPCSTR key,LPCSTR value){require(std::string_view(key)=="EALsxPort"&&std::string_view(value)=="12345","Actual source publishes synthetic selected boundport; no processenvironment write");return FALSE;}

BOOL WINAPI gate_initialize(LPCRITICAL_SECTION p,DWORD spin){++running->gate_inits;return ::InitializeCriticalSectionAndSpinCount(p,spin);}VOID WINAPI gate_delete(LPCRITICAL_SECTION p){++running->gate_deletes;::DeleteCriticalSection(p);}

HANDLE WINAPI create_port(HANDLE file,HANDLE old,ULONG_PTR key,DWORD threads){require(file==INVALID_HANDLE_VALUE&&!old&&!key&&threads==0xffffffffU,"Actual source024990 creates own real IOCP with observed operands");auto h=::CreateIoCompletionPort(file,old,key,threads);if(h){++running->ports_created;running->port=reinterpret_cast<std::uintptr_t>(h);}return h;}

BOOL WINAPI close_port(HANDLE h){require(reinterpret_cast<std::uintptr_t>(h)==running->port,"Source closes exact owned kernel IOCP after acceptdrain");++running->ports_closed;return ::CloseHandle(h);}

int WINAPI convert_sdk(UINT page,DWORD flags,LPCCH in,int count,LPWSTR out,int cap){require(!page&&!flags&&count==-1&&cap==10&&std::string_view(in)=="127.0.0.1","Address adapter uses explicit synthetic loopback conversion");std::memcpy(out,L"127.0.0.1",20);return 10;}

INT WSAAPI parse_sdk(LPWSTR text,INT family,LPWSAPROTOCOL_INFOW info,LPSOCKADDR address,LPINT size){require(std::wstring_view(text)==L"127.0.0.1"&&!info&&*size==128,"Address adapter uses exact typed parser signature without network");if(family==23){::WSASetLastError(10022);return SOCKET_ERROR;}require(family==2,"Synthetic selected listener family isIPv4");sockaddr_in out{};out.sin_family=2;reinterpret_cast<unsigned char*>(&out.sin_addr)[0]=127;reinterpret_cast<unsigned char*>(&out.sin_addr)[3]=1;std::memcpy(address,&out,sizeof(out));*size=sizeof(out);::WSASetLastError(0);return 0;}

int WSAAPI unused_send(SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){++running->io_unselected;unused();}int WSAAPI unused_receive(SOCKET,LPWSABUF,DWORD,LPDWORD,LPDWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){++running->io_unselected;unused();}int WSAAPI unused_error(){unused();}int WSAAPI unused_select(int,fd_set*,fd_set*,fd_set*,const timeval*){unused();}

BOOL PASCAL pipe_accept(SOCKET listener,SOCKET accepted,PVOID address,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED ov){auto& state=*running;require(listener==reinterpret_cast<SOCKET>(state.reader)&&accepted==reinterpret_cast<SOCKET>(state.accepted)&&address&&!receive&&local==0x90&&remote==0x90&&ov,"Selected AcceptSDK maps EXACTlistener/ownedOV andauthoritative288addressspan to own overlappedReadFile, NOTAcceptEx interoperability");++state.submissions;state.ov=ov;state.address=reinterpret_cast<std::uintptr_t>(address);ov->hEvent=state.read_done;DWORD transferred=0;const auto result=::ReadFile(state.reader,address,288,&transferred,ov);const auto error=::GetLastError();require(!result&&error==ERROR_IO_PENDING&&transferred==0,"Own connected namedpipe ReadFile is genuinely kernelpending before any stop request");*bytes=transferred;if(!::SetEvent(state.armed))unused();::SetLastError(error);return result;}

int WSAAPI pipe_error(){return static_cast<int>(::GetLastError());}

VOID PASCAL unused_addresses(PVOID,DWORD,DWORD,DWORD,sockaddr**,int*,sockaddr**,int*){++running->decode_calls;unused();}int WSAAPI unused_update(SOCKET,int,int,const char*,int){++running->update_calls;unused();}

struct Sockets:s::AcceptorApi {

 State& state;explicit Sockets(State& s):state(s){}s::Socket create_socket(std::int32_t family)override{require(family==2,"Source acceptor chooses syntheticIPv4 family");return reinterpret_cast<s::Socket>(state.reader);}std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{require(socket==reinterpret_cast<s::Socket>(state.reader)&&port==state.port,"Only actual namedpipe listener is attached to exactsourceIOCP");auto out=::CreateIoCompletionPort(state.reader,reinterpret_cast<HANDLE>(port),0,0);require(reinterpret_cast<std::uintptr_t>(out)==port,"Real namedpipe kernel completions target actualsourceIOCP");return reinterpret_cast<std::uintptr_t>(out);}std::uint32_t get_win32_error()override{return ::GetLastError();}std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t option,std::int32_t value)override{require(socket==reinterpret_cast<s::Socket>(state.reader)&&level==65535&&option==4&&value==1,"Source reuseaddress operands remain explicit nonnetwork lower");return 0;}std::int32_t bind_socket(s::Socket,std::span<const std::byte>,std::int32_t size)override{require(size==16,"SyntheticIPv4 bind prefix remains source-defined");return 0;}std::int32_t listen_socket(s::Socket,std::int32_t backlog)override{require(backlog==INT_MAX,"Source backlog is observedINT_MAX");return 0;}std::int32_t close_socket(s::Socket value)override{auto h=reinterpret_cast<HANDLE>(value);if(h==state.reader){require(!state.reader_closed,"Source closes own namedpipe listener once");state.reader_closed=true;}else if(h==state.accepted){require(!state.accepted_closed,"Source closes ownpending acceptedEvent token once");state.accepted_closed=true;}else unused();return ::CloseHandle(h)?0:SOCKET_ERROR;}std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}

};

struct Cancel:s::CancellationEffects {void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{unused();}};struct Executor:s::RegistryExecutorEffects {void executor_cleanup(s::SocketObject&,std::uintptr_t table)override{require(table!=0,"Actual024db0 retains explicit executor cleanup dependency");}};

struct Dispatcher:s::DispatcherApi {

 State& state;explicit Dispatcher(State& s):state(s){}void set_last_error(std::uint32_t n)override{::SetLastError(n);}std::uint32_t get_last_error()override{return ::GetLastError();}

 std::int32_t get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& op,std::uint32_t timeout)override{require(port==state.port,"Every physical get targets own real IOCP");if(state.first_get){state.first_get=false;if(!::SetEvent(state.entered_get)||::WaitForSingleObject(state.allow_get,30000)!=WAIT_OBJECT_0)unused();}++state.physical_gets;DWORD n=0;ULONG_PTR k=0;OVERLAPPED* p=nullptr;const auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),&n,&k,&p,timeout);const auto error=::GetLastError();bytes=n;key=k;op=p;if(p){require(p==state.ov,"Actual physical kernel packet retains EXACToverlapped identity submitted to ReadFile");++state.kernel_packets;state.kernel_status=result?0:error;state.kernel_bytes=n;}else if(result)++state.zero_packets;::SetLastError(error);return result;}

 std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* op)override{return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(port),bytes,key,op);}std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}

};

SOCKET WSAAPI open_socket(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){require(family==2&&type==1&&protocol==6&&!info&&!group&&flags==1&&!running->accepted,"Native029df0 calls explicitfactory; selected result is OWNkernelEvent token, not a network socket");running->accepted=::CreateEventW(nullptr,TRUE,FALSE,nullptr);if(!running->accepted)unused();return reinterpret_cast<SOCKET>(running->accepted);}BOOL WINAPI unused_cancel(HANDLE,LPOVERLAPPED){unused();}void move_executor(void*,s::ExecutorFields& out,s::ExecutorFields& in,std::uintptr_t table){require(table==0x180084928ULL,"Source executor move uses exact selected identity");out.context=in.context;out.zero_74=in.zero_74;out.self_reference_80=true;}

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



struct Hooks {

 State& state;Dispatcher& dispatcher;explicit Hooks(State& s,Dispatcher& d):state(s),dispatcher(d){}

 s::IocpSchedulerEntry* select(sp::Application& app,s::IocpSchedulerEntry* owner,s::CancelOperation&,const s::Error&,std::uint32_t){state.application=&app;++state.source_packets;return owner;}

 void after(sp::Application& app){++state.after_calls;auto p=app.ports();require(app.snapshot().live_sessions==0&&app.snapshot().live_jobs==0&&p.view.memory.snapshot().controls==0,"Actual source owner0 acceptcompletion never constructs a session/job/SDKreceive/send");}

 void returned(sp::Application& app,std::uint64_t count,const s::WorkerRunResult& result){auto p=app.ports();require(count==1&&result.error&&result.error->value==0&&state.kernel_packets==1&&state.after_calls==1&&state.sessions==0&&!state.decode_calls&&!state.update_calls&&!state.io_unselected&&app.snapshot().live_sessions==0&&app.snapshot().live_jobs==0&&!p.view.scheduler.work_count_30&&p.view.memory.snapshot().allocations==0&&p.view.io.snapshot().allocations==0,"Actualphysical acceptpacket→native0271a0owner0→cachefree drives sourcework naturallyto0 without manualdecrement/reprojection/session");}

 bool drained(o::ServerView view,sp::Application& app){bool outside=false;try{(void)view.memory.buffer_context();}catch(const std::logic_error&){outside=true;}require(outside&&state.kernel_packets==1&&app.snapshot().live_sessions==0&&app.snapshot().live_jobs==0,"Independent externaldrain observes source return outside Parent after actual kernelpacket and no other admitted work");unsigned relays=0;for(;;){std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* op=nullptr;auto got=dispatcher.get(view.scheduler.completion.port,bytes,key,op,0);const auto error=dispatcher.get_last_error();if(!got){require(error==WAIT_TIMEOUT&&!op,"Actual ownIOCP emptytimeout after source stoprelay, not fabricated drain");break;}require(!bytes&&!key&&!op,"Only real native NULLstoprelay remains after observed terminalpacket");++relays;}require(relays==1,"Nativeworker stoprelay is physically drained after actualsourcecounter0");state.drained=true;return true;}

};

void register_source_wait();

void configure_inputs(State& state,std::shared_ptr<ast::Stop> stop,aw::Inputs& inputs){require(!inputs.graph.state.lifetime&&!inputs.graph.state.configuration&&!inputs.runtime.tls_index,"ApplicationWorker retains exclusive exactStateView/Graph/TLS factory responsibilities");state.worker=::GetCurrentThreadId();running=&state;register_source_wait();auto sockets=std::make_shared<Sockets>(state);auto dispatcher=std::make_shared<Dispatcher>(state);auto hooks=std::make_shared<Hooks>(state,*dispatcher);auto& g=inputs.graph;g.policy.stream=anadius::shared_log::make_stream(std::make_shared<std::stringbuf>());g.policy.imports={::WSASetLastError,::WSAGetLastError,convert_sdk,parse_sdk,unused_send,unused_select};g.policy.lifecycle.lifetime=hooks;g.policy.lifecycle.select_owner=[h=hooks.get()](sp::Application& a,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& e,std::uint32_t n){return h->select(a,owner,op,e,n);};g.policy.lifecycle.after=[h=hooks.get()](sp::Application& a){h->after(a);};g.policy.lifecycle.returned=[h=hooks.get()](sp::Application& a,std::uint64_t n,const s::WorkerRunResult& r){h->returned(a,n,r);};g.policy.lifecycle.external_drained=[h=hooks.get()](o::ServerView v,sp::Application& a){return h->drained(v,a);};g.information=std::make_shared<Information>();g.content=std::make_shared<ContentEffects>();g.licensing=std::make_shared<LicenseBoundary>();g.profile.receive={unused_receive,unused_error};g.profile.send={unused_send,unused_error};g.profile.sessions=1;g.profile.jobs_capacity=4;g.profile.headers=16;g.event_capacity=2;g.version="synthetic";g.prepare_session=[&state](sp::SessionView){++state.sessions;unused();};

 auto& sd=inputs.server;sd.sockets=sockets;sd.dispatcher=dispatcher;s::IocpConstructorImports imports;imports.create_completion_port=create_port;imports.close_handle=close_port;sd.iocp=std::make_shared<s::NativeIocpConstructorApi>(imports);sd.gates=std::make_shared<s::NativeServiceGateApi>(s::ServiceGateImports{gate_initialize,::GetLastError});sd.cancellation=std::make_shared<Cancel>();sd.executor=std::make_shared<Executor>();sd.kernel={startup,cleanup,local_name,environment,gate_delete};auto& rd=inputs.runtime;rd.sockets=sockets;rd.dispatcher=dispatcher;rd.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{pipe_accept,unused_addresses,unused_update,pipe_error});rd.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{open_socket});rd.cancel=std::make_shared<s::NativeRetryCancelApi>(unused_cancel);rd.executor={&state,move_executor};rd.capacity=16;rd.accept_stop=std::move(stop);

}

struct BootstrapControl {

 State& state;std::mutex gate;std::atomic<DWORD> application_thread=0;std::atomic<unsigned> online_destroyed=0,online_freed=0,hook_disabled=0,hook_uninitialized=0;std::shared_ptr<sc::ExitWaitRegistry> waits;std::vector<std::pair<DWORD,std::shared_ptr<sc::WaitTarget>>> wait_targets;unsigned stops=0,configured=0;std::vector<DWORD> worker_ids;std::terminate_handler handler=nullptr;std::vector<anadius::bootstrap_native_recovered::ExitCallback> callbacks;

 explicit BootstrapControl(State& s):state(s){}

};

BootstrapControl* bootstrap_control=nullptr;

void register_source_wait(){auto& c=*bootstrap_control;if(::GetCurrentThreadId()==c.state.controller||::GetCurrentThreadId()==c.application_thread.load())return;std::lock_guard lock(c.gate);for(const auto& row:c.wait_targets)if(row.first==::GetCurrentThreadId())return;auto target=std::make_shared<sc::WaitTarget>(1,0);target->lock();c.waits->register_current_thread(target);c.wait_targets.emplace_back(::GetCurrentThreadId(),std::move(target));}

void wait_phase(State& state,aw::Phase expected){const auto start=::GetTickCount64();for(;;){auto* component=state.component.load();if(component){const auto snapshot=component->snapshot();if(snapshot.phase==expected)return;if(snapshot.phase==aw::Phase::Poisoned)throw std::logic_error("application worker poisoned");}if(::GetTickCount64()-start>=30000)throw std::logic_error("application worker phase timeout");::Sleep(1);}}

std::atomic<State*> current_online=nullptr;
State& online_state(){auto* value=current_online.load();if(!value)unused();return *value;}
void live_export(void* object,bool destroying=false){auto& state=online_state();require(object==&state.online_object&&!state.online_object.destroyed,"Typedforeignthis isEXACTadoptedOWNobject, noarbitrarytoken dereference");const auto snapshot=state.online_component->snapshot();require(snapshot.active_calls==1&&snapshot.binding.active&&snapshot.binding.object==reinterpret_cast<std::uintptr_t>(object)&&snapshot.binding.code_retained&&snapshot.binding.factory_code_retained&&snapshot.binding.object_unknown==destroying,"ActualComponent+Binding reservations/codeleases protect eachtypedvtablecall");}
std::int32_t private_version(){auto& state=online_state();++state.version_calls;state.export_events.push_back('V');::SetLastError(0x5401);return state.online_mode==OnlineMode::VersionMismatch?3:2;}
std::uintptr_t private_factory(const char16_t* cookie){auto& state=online_state();++state.factory_calls;state.export_events.push_back('F');require(std::u16string_view(cookie)==state.config->cookie_path,"Actual035bd0 borrowscopiedactualconfigurationcookieonlyduringtypedfactorycall");::SetLastError(0x5402);return reinterpret_cast<std::uintptr_t>(&state.online_object);}
std::int32_t private_status(void* object){live_export(object);auto& state=online_state();++state.status_calls;state.export_events.push_back('S');const auto worker=state.component.load()->snapshot();require(worker.host_complete&&!worker.online_complete&&worker.phase==aw::Phase::WaitingStartup&&state.physical_gets==0&&state.runtime->online_ready==1,"Typedrealstatus seesactualHostcomplete butOnlineNOTcomplete: preseededactualreadybyte cannot releaseSourceBackend/ApplicationWorkerbarrier");::SetLastError(0x5403);return state.online_mode==OnlineMode::Offline?1:0;}
const char* private_getter(void* object,unsigned index,char event){live_export(object);auto& state=online_state();++state.getter_calls;state.export_events.push_back(event);require(state.runtime->online==1,"Actualsourceonlinebyte iscommitted BEFOREtypedprofilegetterandcopy");::SetLastError(0x5410+index);return state.profile[index].c_str();}
const char* private_username(void* object){return private_getter(object,0,'U');}const char* private_persona(void* object){return private_getter(object,1,'P');}const char* private_userid(void* object){return private_getter(object,2,'I');}
void private_destroy(void* object){live_export(object,true);auto& state=online_state();state.export_events.push_back('D');require(state.runtime->online_object==reinterpret_cast<std::uintptr_t>(object),"Source0366f0 hasNOTclearedactualobjectcellbeforetypedDestroyreturns");state.online_object.destroyed=true;++bootstrap_control->online_destroyed;::SetLastError(0x5420);}
FARPROC WINAPI private_lookup(HMODULE module,LPCSTR name){auto& state=online_state();require(reinterpret_cast<void*>(module)==&state.module_token,"TypedGetProcAddress spyusesexactprivateModuleidentity, NOoriginalDLLlookup");state.export_names.emplace_back(name);::SetLastError(0x5430);if(std::string_view(name)=="version_check")return state.online_mode==OnlineMode::VersionMissing?nullptr:std::bit_cast<FARPROC>(&private_version);if(std::string_view(name)=="create_online")return state.online_mode==OnlineMode::FactoryMissing?nullptr:std::bit_cast<FARPROC>(&private_factory);unused();}
struct PrivateCode {std::shared_ptr<State> keep;explicit PrivateCode(std::shared_ptr<State> state):keep(std::move(state)){}~PrivateCode(){++keep->code_released;keep->code_valid&=keep->online_object.destroyed||keep->factory_calls==0;::SetLastError(0xdead);}};
// Private exports execute ONLY synchronously on the owned source worker and
// controller EXIT cleanup. This fixture NEVER admits asynchronous foreign
// object users. Existing stop_sources excludes/drains workers BEFORE destroy;
// the LATE foreign predicate gates provider retirement, not retroactive safety.
struct OnlineOs final:anadius::recovered::libraries::SystemCalls {
 State& state;std::uint32_t mode=0x22;explicit OnlineOs(State& value):state(value){}
 std::uint32_t get_error_mode()override{register_source_wait();return mode;}std::uint32_t set_error_mode(std::uint32_t value)override{const auto old=mode;mode=value;return old;}
 std::uintptr_t load_library(const wchar_t* path)override{++state.online_loads;require(std::wstring_view(path)==L"C:/fixture\\anadius64online.dll","Actualpaths/recovered0450d0loadpassprivateexpectedfilename toSDKspywithoutopeningANYDLL");const auto worker=state.component.load()->snapshot();require(worker.host_complete&&!worker.online_complete&&state.physical_gets==0,"Actualonlineworker waitsrealExecution/HostmilestonebeforeModuleAPIload");state.online_barrier=true;state.online_files->inputs[state.config->last_profile_path]="fallback-persona;fallback-id;fallback-user";::SetLastError(0x5440);return reinterpret_cast<std::uintptr_t>(&state.module_token);}
 bool free_library(std::uintptr_t module)override{require(module==reinterpret_cast<std::uintptr_t>(&state.module_token)&&state.runtime->online_object==0,"Sourceactualobjectcellcleared BEFOREmodulefreelower; noimplicitobjectdestroy/free");const auto snapshot=state.online_component->snapshot();require(snapshot.binding.active&&!snapshot.binding.object&&!snapshot.binding.procedure&&!snapshot.binding.factory,"Bindinginvalidatesbothcallableaddresses BEFOREcontrolledFreeFALSE");state.export_events.push_back('X');++bootstrap_control->online_freed;::SetLastError(0x5450);return false;}
 int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* source,int count,char* out,int capacity,bool* used)override{BOOL native_used=FALSE;const auto result=::WideCharToMultiByte(cp,flags,source,count,out,capacity,nullptr,used?&native_used:nullptr);if(used)*used=native_used!=FALSE;return result;}std::uint32_t last_error()override{return ::GetLastError();}
};
struct OnlineStats final:anadius::status_recovered::Stats {State& state;explicit OnlineStats(State& value):state(value){}anadius::status_recovered::StatsResult query(const anadius::configuration::Path& path,std::uint32_t flags)override{require(flags==3&&(path==state.config->cookie_path||path==state.config->last_avatar_path),"Actualprofilefallback/statususesliveRuntimeconfigpaths ONLY, syntheticmetadata nofilesystem");return {0,0,{}};}};
int WINAPI private_dialog(HWND owner,LPCWSTR body,LPCWSTR title,UINT flags){auto& state=online_state();++state.dialogs;require(!owner&&std::wstring_view(title)==L"crack error"&&std::wstring_view(body)==L"Online library version mismatch!"&&flags==0x10,"ActualDialog ownsconversion/calls typedUIspy withexactmismatchprefix, NOwindow");return IDOK;}
void check_online_result(State& state,const BootstrapControl& control){
 const bool created=state.online_mode==OnlineMode::Success||state.online_mode==OnlineMode::Offline;
 const auto expected_leases=state.online_mode==OnlineMode::VersionMissing?0U:(created?2U:1U);
 const std::string actual(state.export_events.begin(),state.export_events.end());const std::string expected=state.online_mode==OnlineMode::VersionMissing?"X":(created?(state.online_mode==OnlineMode::Offline?"VFSDX":"VFSUPIDX"):"VX");
 require(actual==expected&&state.online_loads==1&&control.online_freed==1&&control.online_destroyed==(created?1U:0U),"Actualtypedversion/factory/status/getter/Destroy/FreenormalprefixORDERmatchesselectedmodeandsourcecommits");
 require(state.code_valid&&state.code_acquired==expected_leases&&state.code_released==expected_leases,"ExactprivateModuleversion/factoryCodeLeaseownersremainthroughtypedcalls thenretireafteractualsourcecleanup");
 require(state.dialogs==(state.online_mode==OnlineMode::VersionMismatch?1U:0U)&&state.export_names==((state.online_mode==OnlineMode::VersionMissing||state.online_mode==OnlineMode::VersionMismatch)?std::vector<std::string>{"version_check"}:std::vector<std::string>{"version_check","create_online"}),"Missing/mismatchedexportprefixes nevercalllaterfactory/status/getters; mismatchusesactualtypedDialogonly");
 const auto& config=*state.config;
 if(state.online_mode==OnlineMode::Success){require(state.runtime->online==1&&config.username==state.profile[0]&&config.persona_id==state.profile[1]&&config.user_id==state.profile[2]&&config.avatar==config.last_avatar_path,"ActualSource035bd0typedOWNgetters copylongstringsintoRuntimeOwnerconfigurationinusername/persona/idorder");require(state.online_files->outputs.at(config.last_profile_path)==state.profile[1]+';'+state.profile[2]+';'+state.profile[0]&&state.online_files->writes==std::vector<anadius::configuration::Path>{config.last_profile_path}&&state.online_files->reads.empty(),"Recoveredprofilepersistence writesEXACTcountedownedstrings toMEMORYfileonly, no userprofile/cfg read");}
 else if(state.online_mode==OnlineMode::Offline){require(state.runtime->online==0&&config.username=="fallback-user"&&config.persona_id=="fallback-persona"&&config.user_id=="fallback-id"&&state.online_files->reads==std::vector<anadius::configuration::Path>{config.last_profile_path}&&state.online_files->writes.empty(),"Status1 actualsourcefallback3fieldparser readsONLYsyntheticMemoryprofileaftertypedstatus, preservesofflineflag");}
 else require(state.runtime->online==0&&config.username=="pre-online-user"&&state.online_files->reads.empty()&&state.online_files->writes.empty()&&state.factory_calls==0&&state.status_calls==0&&state.getter_calls==0,"Missingversion/mismatch/missingfactory preservepre-onlineprofileandperformNOobject/getter/fileprefix");
 const auto log=state.online_log->str();require(log.starts_with("online dll found!\n")&&log.find(state.online_mode==OnlineMode::Success?"authentication ok\n":state.online_mode==OnlineMode::Offline?"authentication failed, starting offline\n":state.online_mode==OnlineMode::VersionMissing?"version function not found\n":state.online_mode==OnlineMode::VersionMismatch?"online library version mismatch!\n":"factory function not found\n")!=std::string::npos,"NewactualOnline logleaves retainexactsourceprefixforeachnormalornegativebranch");
}
struct LibraryOS:anadius::recovered::libraries::SystemCalls {

 std::atomic<std::uint32_t> mode=0;std::uint32_t get_error_mode()override{register_source_wait();return mode.load();}std::uint32_t set_error_mode(std::uint32_t value)override{return mode.exchange(value);}std::uintptr_t load_library(const wchar_t*)override{unused();}bool free_library(std::uintptr_t)override{unused();}int wide_to_narrow(std::uint32_t,std::uint32_t,const wchar_t*,int,char*,int,bool*)override{unused();}std::uint32_t last_error()override{return 0;}

};

struct LibraryEffects:anadius::recovered::libraries::Effects {

 void begin_load_log_0863e0()override{unused();}void append_log_path_and_flush(std::string_view)override{unused();}void message_038bc0(std::string,std::string,std::uint32_t)override{unused();}[[noreturn]]void throw_runtime_00cd70(int)override{unused();}[[noreturn]]void throw_filesystem_00cdb0(std::uint32_t)override{unused();}void wait_steps_036880(int)override{unused();}void finish_flag_036820()override{unused();}

};

struct Host:anadius::host_init_recovered::Effects {

 BootstrapControl& c;std::uintptr_t next=0x1000;explicit Host(BootstrapControl& v):c(v){}

 std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t,std::uint8_t)override{return m+1;}bool verify_version(const anadius::host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t)override{return true;}

 std::int32_t initialize_engine_046a20()override{if(::WaitForSingleObject(c.state.armed,30000)!=WAIT_OBJECT_0)unused();wait_phase(c.state,aw::Phase::WaitingStartup);const auto snapshot=c.state.component.load()->snapshot();require(!snapshot.host_complete&&!snapshot.online_complete&&c.state.physical_gets==0&&c.state.runtime->server_ready==1&&c.state.runtime->online_ready==1&&c.state.runtime->libraries_ready==1,"Kernelpending ownpipe request cannot dispatch while actual Host/Online callbacks incomplete despite ALLpreseeded readybytes");c.state.config->game_attributes["host-ready"]="set-by-source-host-lower";c.state.host_barrier=true;return 0;}

 std::uintptr_t get_module_handle(std::u16string_view)override{return 1;}std::uintptr_t get_proc_address(std::uintptr_t,std::string_view)override{return ++next;}std::int32_t create_hook_0466e0(std::uintptr_t target,anadius::host_init_recovered::Callback,std::uintptr_t& original)override{original=target+0x100000;return 0;}std::int32_t enable_hook_046a10(std::uintptr_t)override{return 0;}void log_text_006880(std::string_view)override{}void log_integer_00af90(std::int32_t)override{}void finish_log_0088e0()override{}

};

struct Cleanup:anadius::network_hook_recovered::CleanupEffects {

 BootstrapControl& c;explicit Cleanup(BootstrapControl& v):c(v){}std::int32_t disable_hook_046a00(std::uintptr_t)override{++c.hook_disabled;return 0;}std::int32_t uninitialize_engine_046ac0()override{++c.hook_uninitialized;return 0;}void log_literal_006880(std::string_view)override{}void log_finish_0088e0()override{}

};

int at_exit(anadius::bootstrap_native_recovered::ExitCallback callback)noexcept{bootstrap_control->callbacks.push_back(callback);return 0;}std::terminate_handler set_terminate(std::terminate_handler handler)noexcept{const auto old=bootstrap_control->handler;bootstrap_control->handler=handler;return old;}std::terminate_handler get_terminate()noexcept{return bootstrap_control->handler;}void previous_terminate(){std::abort();}

struct PermanentErrors:anadius::bootstrap_native_recovered::Errors {[[noreturn]]void lifecycle(anadius::bootstrap_native_recovered::LifecycleFault)override{unused();}[[noreturn]]void callback_exception(anadius::bootstrap_native_recovered::Entry,std::exception_ptr error)noexcept override{try{std::rethrow_exception(error);}catch(const std::exception& e){std::cerr<<"SOURCE CALLBACK EXCEPTION: "<<e.what()<<'\n'<<std::flush;}catch(...){}::ExitProcess(115);}} permanent_errors;

BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* output){if(flags!=4||!address||!output)unused();*output=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}

HANDLE WINAPI create_worker(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE entry,LPVOID argument,DWORD flags,LPDWORD id){require(!security&&!stack&&entry&&argument&&!flags&&id,"Actual bootstrap uses original CreateThread wrapper operands");auto result=::CreateThread(security,stack,entry,argument,flags,id);if(result){std::lock_guard lock(bootstrap_control->gate);bootstrap_control->worker_ids.push_back(*id);}return result;}

void connect_own_pipe(State& state){

 const auto name=L"\\\\.\\pipe\\codex-b54-"+std::to_wstring(::GetCurrentProcessId())+L"-"+std::to_wstring(state.success_race);

 state.reader=::CreateNamedPipeW(name.c_str(),PIPE_ACCESS_INBOUND|FILE_FLAG_OVERLAPPED,PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,1,512,512,0,nullptr);require(state.reader!=INVALID_HANDLE_VALUE,"Own test creates local overlapped inbound namedpipe; NO network socket");HANDLE event=::CreateEventW(nullptr,TRUE,FALSE,nullptr);require(event,"Own pipe connection has separate completion event before IOCP attachment");OVERLAPPED connect{};connect.hEvent=event;const auto connecting=::ConnectNamedPipe(state.reader,&connect);const auto error=::GetLastError();require(!connecting&&error==ERROR_IO_PENDING,"Own namedpipe connect begins through actual overlapped kernelAPI");state.writer=::CreateFileW(name.c_str(),GENERIC_WRITE,0,nullptr,OPEN_EXISTING,0,nullptr);require(state.writer!=INVALID_HANDLE_VALUE&&::WaitForSingleObject(event,10000)==WAIT_OBJECT_0,"Own local writer connects actual test pipe before source attachesIOCP");DWORD n=0;require(::GetOverlappedResult(state.reader,&connect,&n,FALSE)&&::CloseHandle(event),"Connection OVERLAPPED is genuinely retired before its own stack ends/IOCPassociation");

 state.read_done=::CreateEventW(nullptr,TRUE,FALSE,nullptr);state.allow_get=::CreateEventW(nullptr,TRUE,FALSE,nullptr);state.entered_get=::CreateEventW(nullptr,TRUE,FALSE,nullptr);state.armed=::CreateEventW(nullptr,TRUE,FALSE,nullptr);require(state.read_done&&state.allow_get&&state.entered_get&&state.armed&&!(reinterpret_cast<std::uintptr_t>(state.read_done)&1),"ReadFile uses own stable hEvent with LOWBIT0, preserving actual IOCP notification alongside independent kernelcompletion witness");

}

struct BlockingBuffer final:std::stringbuf {

 HANDLE entered=::CreateEventW(nullptr,TRUE,FALSE,nullptr),release=::CreateEventW(nullptr,TRUE,FALSE,nullptr);std::atomic<bool> blocked=false;

 ~BlockingBuffer()override{if(entered)::CloseHandle(entered);if(release)::CloseHandle(release);}

 std::streamsize xsputn(const char* bytes,std::streamsize count)override{bool expected=false;if(std::string_view(bytes,static_cast<std::size_t>(count))=="LSX emu"&&blocked.compare_exchange_strong(expected,true)){if(!::SetEvent(entered)||::WaitForSingleObject(release,30000)!=WAIT_OBJECT_0){std::cerr<<"Actual BootstrapLogger firstliteral barrier timeout\n"<<std::flush;::ExitProcess(121);}}return std::stringbuf::xsputn(bytes,count);}

};

struct AppPlan {bn::ApplicationEntry cached;HeapObservation* memory;std::atomic<BOOL> result=FALSE;};

unsigned __stdcall old_application(void* opaque){auto& p=*static_cast<AppPlan*>(opaque);bootstrap_control->application_thread=::GetCurrentThreadId();{std::lock_guard lock(p.memory->gate);p.memory->payload_controller=::GetCurrentThreadId();p.memory->watch_payloads=true;}p.result=p.cached(nullptr,1,nullptr);return 63;}

void actual_owned_online(bool success_race,OnlineMode mode=OnlineMode::Success){

 NativeDomain native;auto state=std::make_shared<State>();auto* observed=state.get();std::weak_ptr<State> state_lifetime=state;state->online_mode=mode;state->expected_domain=native.root.get();state->controller=::GetCurrentThreadId();state->success_race=success_race;connect_own_pipe(*state);BootstrapControl control(*state);control.handler=previous_terminate;bootstrap_control=&control;control.waits=std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned);auto buffer=std::make_shared<BlockingBuffer>();auto stream=anadius::shared_log::make_stream(buffer);auto logger=std::make_shared<sc::BootstrapLogger>(stream);std::weak_ptr<td::ApplicationMemory> remaining_memory;unsigned factory_calls=0;sf::Remaining* selected=nullptr;br::CallbackRegistry* bound_registry=nullptr;

 ast::Options stop_options;stop_options.lifetime=state;stop_options.capacity=16;stop_options.other_work_drained=[observed]{require(::GetCurrentThreadId()==observed->controller&&observed->other_admission_closed&&observed->submissions==1&&!observed->sessions&&!observed->io_unselected&&!observed->kernel_packets,"Independent controller proof closes OTHER admissions: no session/receive/send/raw-job/foreign callback was ever admitted in this accept-only graph; NOT inferred from accept counters");return true;};require(stop_options.imports.cancel==&::CancelIoEx&&stop_options.imports.post==&::PostQueuedCompletionStatus,"Concrete Stop uses DEFAULTrealCancelIoEx/realPostQCS imports, no cancellation result stub");auto stop=ast::Stop::create(std::move(stop_options));state->stop=stop;require(!stop->request(),"AcceptStop refuses controller stop before actualRuntime.run instead of fabricatingstarted completion");

 b2_test::Events controller_events;bb::Resources resources{};resources.state_lifetime=state;resources.configuration=std::make_unique<b2_test::Effects>(controller_events);auto files=std::make_unique<b2_test::Files>(controller_events);files->base=b2_test::base_text();files->over=R"("Config2" {"User" {"Username" "pre-online-user" "UseLastOnlineProfile" "true"}})";resources.files=std::move(files);resources.input=std::make_unique<b2_test::InputPlatform>(controller_events);resources.console=std::make_unique<b2_test::Console>(controller_events);auto thread_api=anadius::thread_native::native_api();thread_api.get_module=no_module;thread_api.create=create_worker;resources.threads=native.root->make_threads(thread_api);resources.remaining_factory=[&](br::CallbackRegistry& registry)->std::unique_ptr<bb::RemainingPolicies>{++factory_calls;bound_registry=&registry;sf::Options options;options.memory=native.root->make_memory();remaining_memory=options.memory;options.logger=logger;options.waits=control.waits;auto result=sf::make_remaining(registry,std::move(options));selected=dynamic_cast<sf::Remaining*>(result.get());return result;};resources.online_factory=[keep=state](bb::CallbackStateView view)->std::unique_ptr<bb::OnlineComponent>{keep->config=&view.configuration;keep->runtime=&view.runtime;keep->online_execution=&view.execution;current_online=keep.get();keep->online_table={reinterpret_cast<std::uintptr_t>(&private_destroy),reinterpret_cast<std::uintptr_t>(&private_status),reinterpret_cast<std::uintptr_t>(&private_username),reinterpret_cast<std::uintptr_t>(&private_persona),reinterpret_cast<std::uintptr_t>(&private_userid)};keep->online_object.table=reinterpret_cast<std::uintptr_t>(keep->online_table.data());of::Options options;options.lifetime=keep;options.stream=anadius::shared_log::make_stream(keep->online_log);options.dialog=std::make_shared<anadius::library_effects::Dialog>(anadius::library_effects::DialogApi{::MultiByteToWideChar,private_dialog});options.os=std::make_unique<OnlineOs>(*keep);options.stats=std::make_unique<OnlineStats>(*keep);options.files=std::make_unique<b8_test::MemoryFiles>(keep->online_files);options.get_proc=private_lookup;options.code_lease=[keep](std::uintptr_t module)->std::shared_ptr<void>{require(module==reinterpret_cast<std::uintptr_t>(&keep->module_token),"CodeLease isboundtoEXACTprivateModule andOWNEXEtypedexportbodies, notanarbitrarysuccessmarker");++keep->code_acquired;return std::make_shared<PrivateCode>(keep);};options.foreign_quiescent=[keep]{++keep->quiescence_calls;require(::GetCurrentThreadId()==keep->controller&&keep->drained&&keep->runtime->online_module==0&&keep->runtime->online_object==0,"Explicitforeignquiescence executesONLYafteractualsourceStop/join/EXITclearedactualmodule/object; noindependentseededstate");return keep->online_quiescent;};auto built=of::make_online(view,options);keep->online_component=dynamic_cast<of::Online*>(built.get());require(keep->online_component&&!options.code_lease&&!options.foreign_quiescent&&!options.lifetime,"NamedOptionssink consumescallabledonors while actualnewOnlinecomponentownsbacking");return built;};resources.library_os=std::make_unique<LibraryOS>();resources.library_effects=std::make_unique<LibraryEffects>();resources.host=std::make_unique<Host>(control);resources.hook_cleanup=std::make_unique<Cleanup>(control);resources.join_timeout_ms=30000;

 resources.application_worker=[keep=state,domain=native.root,stop,&control](bb::CallbackStateView view)->std::unique_ptr<bb::ApplicationWorker>{require(::GetCurrentThreadId()==keep->controller&&!keep->component,"Actual SourceBackend bind_state creates concrete ApplicationWorker on controller");keep->config=&view.configuration;keep->runtime=&view.runtime;aw::Settings settings;settings.lifetime=keep;settings.state=aw::Settings::StateSeed{0,{}};settings.startup_timeout_ms=30000;settings.configure=[keep,stop,&control](aw::Inputs& input){++control.configured;configure_inputs(*keep,stop,input);};auto worker=aw::Worker::create(domain,view,std::move(settings));keep->component=worker.get();return worker;};

 resources.stop_sources=[observed,stop,&control,&native]{++control.stops;require(observed->component.load()->snapshot().phase==aw::Phase::Serving&&observed->submissions==1&&!observed->kernel_packets&&!observed->sessions,"Controller stops only genuinely running graph with one kernelpending accept and ZERO other admittedIO/sessionwork");observed->other_admission_closed=true;require(stop->request(),"Controller Stop.request posts ONLYownNULLwake for originalworker cancellation");require(stop->snapshot().cancel_calls==0&&stop->snapshot().kernel_packets==0&&live_range(native.memory,observed->address,288),"Request/wake doesNOTissuecontrollerCancel or retire authoritative473allocation/kernelbuffer before originalworker observes anything");

  if(observed->success_race){const char byte='Z';DWORD written=0;require(::WriteFile(observed->writer,&byte,1,&written,nullptr)&&written==1&&::WaitForSingleObject(observed->read_done,10000)==WAIT_OBJECT_0,"Race writes ownpipe AFTERwake queued but BEFOREworker resumes, independently witnesses actual successful kernelcompletion");require(stop->snapshot().kernel_packets==0&&stop->snapshot().acknowledged==0&&live_range(native.memory,observed->address,288),"Even completed kernelIO retains source buffer/pin until actual physical IOCPpacket is observed and native source route retires it");}

  require(::SetEvent(observed->allow_get)!=FALSE,"Controller releases physicalGet gate; originalworker must issue REALCancelIoEx exactlistener/OV");wait_phase(*observed,aw::Phase::Returned);require(observed->drained&&stop->snapshot().phase==ast::Phase::Closed,"Controller stop lower waits actualkernelpacket/native0271a0/knownexternaldrain/Graphdestruction before SourceBackend request_stop/join");return true;};

 require(!resources.remaining&&resources.remaining_factory&&factory_calls==0&&buffer->str().empty()&&control.waits->active()==0,"PassiveResources selectsONLYnarrowfactory; no legacyServices/lower, sourceoutput or registrations beforeRuntime preparation");

 bb::RuntimeSeed seed{};anadius::execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.server_ready=seed.online_ready=seed.libraries_ready=1;auto runtime=br::make_runtime(b2_test::seed(),std::move(seed),resources,permanent_errors,br::HostApi{at_exit,set_terminate,get_terminate});require(!resources.remaining_factory&&!resources.stop_sources&&!resources.callback_attachment&&!resources.application_worker&&!resources.online_factory,"Explicitcaller take_resources clearsALLfunction donors whileactualreceivingprovider lives; noNRVO/SBOmove assumption");require(factory_calls==1&&!remaining_memory.expired()&&native.root->provider_leases()==3&&state->component.load()->snapshot().phase==aw::Phase::Prepared,"Actual RuntimeOwner passively binds preparedconcreteworker+NativeThreads before any SDKsubmission");runtime->publish();const auto cached=runtime->application_entry();auto outer_threads=native.root->make_threads(thread_api);AppPlan plan{cached,&native.memory};std::uint32_t outer_id=0;const auto outer_raw=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&old_application),reinterpret_cast<std::uintptr_t>(&plan),0,&outer_id,*outer_threads);require(outer_raw&&outer_id!=observed->controller,"ActualApplication runs onfourth sameDomainNativeThread withrealPTD, notmanualGraph/controller callback");auto outer_handle=reinterpret_cast<HANDLE>(outer_raw);state.reset();

 if(::WaitForSingleObject(buffer->entered,30000)!=WAIT_OBJECT_0){std::cerr<<"ApplicationLogger barrier timeout\n"<<std::flush;::ExitProcess(122);}require(selected&&selected->snapshot().phase==sf::Phase::Open&&selected->snapshot().active_calls==1&&bound_registry&&bound_registry->pending_exit_callbacks()==1,"ActualnewRemaining activelease/ownRuntimeCallbackRegistry visible whilefirstopaqueLogger insertion isblocked");require(buffer->blocked&&bn::active_calls()==1&&control.worker_ids.size()==1&&observed->submissions==0&&control.callbacks.size()==1,"PreadmittedApplication holdsactualCoreBootstrapLogger call AFTERregistryregistration BEFOREanybootstrap workerpayload");require(!runtime->close()&&runtime->phase()==anadius::bootstrap_owner::Phase::Stopping&&control.stops==0&&!observed->component.load()->snapshot().stop_requested,"Ownerseal waitsactualoldApp BEFOREcallingstop_sources orCoreclose; no fakeguard hook");for(const DWORD reason:{0U,1U,2U,3U}){::SetLastError(0x7b53);require(cached(nullptr,reason,nullptr)==FALSE&&::GetLastError()==0x7b53,"CachedApplication allreason rejection preservesLastError whilegenuineLogger call/oldApplication retained");}require(control.worker_ids.size()==1&&control.stops==0&&[&]{std::lock_guard lock(native.memory.gate);return native.memory.payloads.empty();}()&&control.callbacks.size()==1,"RejectedcachedApplication makesnoextraregistration/payload/thread/stop effect whilefirstliteral blocked");require(::SetEvent(buffer->release)!=FALSE,"ReleaseactualownedLogger opaque streambuf; preadmittedsource mayfinishdespiteapplicationgate seal");if(::WaitForSingleObject(outer_handle,30000)!=WAIT_OBJECT_0){std::cerr<<"ApplicationNativejoin timeout\n"<<std::flush;::ExitProcess(123);}DWORD outer_code=0;require(::GetExitCodeThread(outer_handle,&outer_code)&&outer_code==63&&plan.result==TRUE&&::CloseHandle(outer_handle),"PreadmittedApplication CONTINUESactualsource8/8/16+3bootstrapworkers afterseal andouterNativeThread/FLSreallyjoins");

 if(::WaitForSingleObject(observed->entered_get,30000)!=WAIT_OBJECT_0){std::cerr<<"accept firstGet timeout\n"<<std::flush;::ExitProcess(116);}require(observed->host_barrier&&observed->online_barrier&&observed->component.load()->snapshot().phase==aw::Phase::Serving&&observed->physical_gets==0,"Both actualHost+Online milestones precede realRuntimephysicalget, with ownedReadFile kernelpending and dispatch deliberatelygated");require(!runtime->close()&&runtime->phase()==anadius::bootstrap_owner::Phase::Stopping&&bound_registry->finished()&&observed->online_component->snapshot().phase==of::Phase::Open&&control.stops==1&&control.online_freed==1,"FALSEforeignattestation retainsOnlineowners AFTERactualStop/join/registeredEXIT+registryfinish; no guessedclose");const auto before_online=observed->online_component->snapshot();require(before_online.configuration==observed->config&&before_online.runtime==observed->runtime&&before_online.execution==observed->online_execution&&before_online.binding.module==0&&before_online.binding.object==0&&!before_online.binding.procedure&&!before_online.binding.factory&&!before_online.binding.active&&!before_online.binding.poisoned,"Onlinecomponent usesEXACTstableBackendconfiguration/runtime/Execution andactualemptyBinding, notsubstituteready/module/objectcells");observed->online_quiescent=true;require(runtime->close(),"Explicittrueforeignproof retriesterminalOnlineclose withoutrepeatingStop/join/EXIT");require(control.stops==1&&control.online_freed==1&&control.hook_uninitialized==1&&observed->online_component->snapshot().phase==of::Phase::Closed,"RetrydoesNOTrepeatexternalstop/modulefree/hookcleanup andsealsactualOnlinecomponent");const auto result=stop->snapshot();const auto worker=observed->component.load()->snapshot();require(selected->snapshot().phase==sf::Phase::Closed&&selected->snapshot().active_calls==0&&selected->snapshot().used&&bound_registry->finished()&&!bound_registry->pending_exit_callbacks(),"Narrowclose occursONLYAFTERactualstop/join, registeredNativeExitdrain andCallbackRegistryfinish");

 require(result.phase==ast::Phase::Closed&&result.cancel_calls==1&&result.kernel_packets==1&&result.software_packets==0&&result.acknowledged==1&&result.active_posts==0&&result.receipts==0&&observed->kernel_packets==1&&observed->source_packets==0&&observed->after_calls==1,"ExactlyONE REALkernelpacket is source-acknowledged after defaultCancelIoEx; no fabricatedterminal/softwarepost/workdecrement");

 if(success_race)require(result.not_found==1&&observed->kernel_status==0&&observed->kernel_bytes==1&&!observed->decode_calls&&!observed->update_calls&&!observed->sessions,"RealCancelIoExERROR_NOT_FOUND after successful ownpipecompletion still waits actualSUCCESSpacket and drops owner BEFOREendpointdecoder/session creation");else require(result.not_found==0&&observed->kernel_status==ERROR_OPERATION_ABORTED&&observed->kernel_bytes==0&&!observed->decode_calls&&!observed->sessions,"RealCancelIoEx causes actual995GQCScompletion; exactownedOV/source cleanup is observed rather than synthetic995publication");

 require(worker.phase==aw::Phase::Closed&&worker.memory.worker_exit_verified&&worker.memory.phase==wh::Phase::Closed&&worker.worker_thread==observed->worker&&control.worker_ids.size()==4&&control.worker_ids[0]==outer_id&&control.worker_ids[1]==worker.worker_thread&&!remaining_memory.expired()&&native.memory.count()==1&&native.root->provider_leases()==3,"Controllerfinish verifiesrealserver+FOURactualDomainthread/FLS exits; closedRemaining intentionallyretainsAppMemory lease untilproviderdestruction");

 require(observed->reader_closed&&observed->accepted_closed&&observed->ports_created==1&&observed->ports_closed==1&&observed->gate_inits==2&&observed->gate_deletes==2&&observed->starts==1&&observed->cleanups==1&&!observed->io_unselected,"Actual024db0/Graphcontext cleanup closes ownedpipe listener/eventtoken/IOCP/gates/ownWSAscope exactly once without any networkIO");

 check_online_result(*observed,control);require(buffer->str()=="LSX emu version 2.0.1.0\n"&&control.callbacks.size()==1,"ConcreteBootstrapLogger countedsourcechain+CallbackRegistry actualregistration used without giantRemaining lower");require(control.wait_targets.size()==3&&control.waits->active()==0,"AllTHREErealDomainworkers registeractualownedExitWaitTarget andsourcecleanupdrainsallrows");for(const auto& row:control.wait_targets)require(!row.second->registered()&&!row.second->is_locked()&&row.second->ready_value_quiescent()==1,"Actual04a47c cleanup publishesready/unlocks matching worker target BEFOREnativejoin/FLS");{std::lock_guard lock(native.memory.gate);unsigned eight=0,sixteen=0;bool valid=true;for(const auto& p:native.memory.payloads){eight+=p.bytes==8;sixteen+=p.bytes==16;valid&=p.freed&&p.freed_by!=outer_id&&std::find(control.worker_ids.begin(),control.worker_ids.end(),p.freed_by)!=control.worker_ids.end();}require(eight==2&&sixteen==1&&valid,"ConcreteApplicationMemory usesactualsameDomainHeap forSOURCE8/8/16payloads andallfreesoccuronactualbootstrapworkers");}auto writer=observed->writer,done=observed->read_done,allow=observed->allow_get,entered=observed->entered_get,armed=observed->armed;runtime.reset();require(remaining_memory.expired()&&native.root->provider_leases()==1&&!state_lifetime.expired(),"Remainingdestructor AFTERclosedResources releasesrealAppMemory; onlyjoinedouterNativeThreads provider andexternalclosedStop remain");outer_threads.reset();require(native.root->provider_leases()==0,"Controller retiresjoinedouterNativeThreads onlyafterRemaining/Runtime resources");stop.reset();require(state_lifetime.expired(),"ClosedStop controllerdestruction releases remaining lifecycle state without workerGraph/controller alias cycle");running=nullptr;current_online=nullptr;bootstrap_control=nullptr;require(::CloseHandle(writer)&&::CloseHandle(done)&&::CloseHandle(allow)&&::CloseHandle(entered)&&::CloseHandle(armed),"Ownwriter/readDone/gateevents remain alive through realterminal/source/join and close after all providers");require(native.root->close()&&native.memory.balanced()&&native.memory.slot_allocations==1&&native.memory.slot_releases==1,"Realprocessheap/FLS Domain closes only after actual4workerexit and all provider/controllerowners retire");

}

}

int main(){try{actual_owned_online(false);actual_owned_online(true);actual_owned_online(false,OnlineMode::VersionMissing);actual_owned_online(false,OnlineMode::VersionMismatch);actual_owned_online(false,OnlineMode::FactoryMissing);actual_owned_online(false,OnlineMode::Offline);std::cout<<"PASS: "<<checks<<" online-factory integration checks; actualRuntimeOwnerFOURNativeThreads+3bootstrapworkers/Graph usesownedoverlappedNAMEDPIPE ReadFile onauthoritativeacceptbuffer/OV+realIOCP, DEFAULTCancelIoEx995 andERROR_NOT_FOUND-successrace→observedkernelpacket→native0271a0owner0/cachefree→Graphclose→actualjoin/FLS/controllerfinish; OWNOnlineComponent/actualBinding/typedprivateversionfactoryvtable +sourceprofiles/copiedgetters/negativeprefixes/lateproviderretirement (noasyncforeignobjectusers admitted); no giantServiceslower/syntheticterminalpost/realnetwork/AcceptExinterop/nativeABI/fullIOdrainclaim\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

