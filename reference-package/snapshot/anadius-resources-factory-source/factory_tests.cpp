#include <winsock2.h>
#include "factory.hpp"
#include "configuration.hpp"
#include "../anadius-module-reference-source/reference.hpp"
#include "../anadius-permanent-errors-source/errors.hpp"
#include "../anadius-thread-domain-source/factory.hpp"
#include "../anadius-pre-unload-source/shutdown.hpp"
#include "../anadius-bootstrap-providers-source/providers_test_fixture.hpp"
#include "../anadius-host-platform-source/assembly.hpp"
#include "../anadius-host-platform-source/engine.hpp"
#include "../anadius-initial-cells-source/defaults.hpp"
#include "../anadius-initial-cells-source/runtime.hpp"
#include "../anadius-initial-cells-source/worker.hpp"
#include "../anadius-native-context-source/context_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-socket-source/worker_run_native.hpp"
#include <atomic>
#include <bit>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream>

// Actual installer/Runtime/three Domain bootstrap workers and concrete Graph
// effects. Configuration/host/console are explicit synthetic test boundaries;
// no whole-configuration, original image, network or user-file closure claim.
namespace {
using namespace anadius;namespace rf=resources_factory;namespace aw=application_worker;namespace s=socket_recovered;namespace sp=session_profile;namespace os=owned_server;namespace br=bootstrap_runtime;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
[[noreturn]]void unused(){throw std::logic_error("unselected resources fixture boundary");}
struct HookEngine final:recovered_hooks::Api {
    b14_test::Engine calls;native_recovered::Resolver resolver;
    int initialize()override{return calls.initialize();}
    int create(void* target,void* detour,void** out)override{
        std::size_t slot=0;while(slot<10&&resolver.rebuilt_address(static_cast<host_init_recovered::Callback>(slot))!=reinterpret_cast<std::uintptr_t>(detour))++slot;
        require(slot<10,"Installed host uses actual rebuilt SDK callback entry addresses");const auto result=calls.create(target,detour,out);*out=reinterpret_cast<void*>(b15_test::originals()[slot]);return result;
    }
    int enable(void* t)override{return calls.enable(t);}int disable(void* t)override{return calls.disable(t);}int uninitialize()override{return calls.uninitialize();}
};
struct State;State* current=nullptr;
struct State {
    b2_test::Events events;b4_test::HostState host;std::optional<bootstrap_backend::CallbackStateView> bound;
    std::shared_ptr<thread_domain::DomainStorage> domain;const achievement_storage::Paths* shared_paths=nullptr;
    std::array<std::uintptr_t,7> table{};std::array<std::byte,17> object{};
    std::shared_ptr<std::stringbuf> buffer=std::make_shared<std::stringbuf>();std::shared_ptr<shared_log::Stream> stream=shared_log::make_stream(buffer);
    std::shared_ptr<library_effects::Dialog> dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());b12_test::Ui ui;
    std::shared_ptr<b8_test::MemoryFilesState> files=std::make_shared<b8_test::MemoryFilesState>();
    HANDLE armed=::CreateEventW(nullptr,TRUE,FALSE,nullptr),done=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
    std::uintptr_t port=0;OVERLAPPED* terminal=nullptr;std::vector<OVERLAPPED*> accepts;sp::Application* app=nullptr;
    b14_test::Sdk host_sdk;HookEngine* hooks=nullptr;bool provider_ready=true,external=true,stop_allowed=true;unsigned stops=0;std::weak_ptr<foreign_callback::Domain> callback_admission;
    bool shutdown=false,worker_returned=false,drained=false,slot_valid=true,path_retirement_valid=false;unsigned configured=0,startups=0,cleanups=0,loads=0,avatars=0,auths=0,destroys=0,pins=0,frees=0,socket_count=0,responses=0,config_retired=0,legacy_calls=0,legacy_retired=0,module_paths=0,environment_paths=0;
    b8_test::FsSdk filesystem_sdk;b5_test::PlatformState console_sdk;
    DWORD graph_thread=0,online_thread=0;std::string wire;std::size_t cursor=0;std::vector<std::string> response_text;configuration::Path avatar_path;std::string avatar_owner;std::int32_t avatar_size=0;std::vector<std::pair<std::string,std::int32_t>> avatar_requests;std::vector<configuration::Path> avatar_copy_from,avatar_copy_to;
    State(){require(armed&&done,"Own bounded driver events created");current=this;b4_test::host_state=&host;b14_test::sdk=&host_sdk;b8_test::fs_sdk=&filesystem_sdk;b5_test::platform=&console_sdk;filesystem_sdk.names.clear();}
    ~State(){::CloseHandle(armed);::CloseHandle(done);b4_test::host_state=nullptr;b14_test::sdk=nullptr;b8_test::fs_sdk=nullptr;b5_test::platform=nullptr;current=nullptr;}
    online_recovered::Object identity(){return reinterpret_cast<online_recovered::Object>(object.data()+1);}
};
void observe_online(void* object){auto& st=*current;st.slot_valid=st.slot_valid&&object==reinterpret_cast<void*>(st.identity())&&::FlsGetValue(st.domain->native_fls_index())!=nullptr;}
std::int32_t version(){return 2;}void destroy(void* object){observe_online(object);require(current->drained,"Foreign users already drained before online object cleanup");++current->destroys;}
std::int32_t status(void* object){observe_online(object);current->online_thread=::GetCurrentThreadId();return 0;}
const char* username(void* object){observe_online(object);return "resource-user";}const char* persona(void* object){observe_online(object);return "resource-persona";}const char* userid(void* object){observe_online(object);return "owner";}
std::uint8_t avatar(void* object,const char16_t* path,const char* owner,std::int32_t size){observe_online(object);++current->avatars;current->avatar_path=path;current->avatar_owner=owner;current->avatar_size=size;current->avatar_requests.emplace_back(owner,size);return 0x80;}
const char* auth(void* object,const char* client){observe_online(object);++current->auths;require(std::string_view(client)=="client","Actual owned auth receives client");return "";}
online_recovered::Object factory(const char16_t*){auto& st=*current;st.table={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&userid),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};const auto table=reinterpret_cast<std::uintptr_t>(st.table.data());std::memcpy(st.object.data()+1,&table,sizeof table);return st.identity();}
FARPROC WINAPI lookup(HMODULE module,LPCSTR name){require(reinterpret_cast<std::uintptr_t>(module)==0x80,"Only own synthetic module lookup");if(std::string_view(name)=="version_check")return std::bit_cast<FARPROC>(&version);require(std::string_view(name)=="create_online","Only selected own factory export");return std::bit_cast<FARPROC>(&factory);}
BOOL WINAPI retain_online(DWORD flags,LPCWSTR address,HMODULE* module){require(flags==GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS&&reinterpret_cast<std::uintptr_t>(address)==0x80,"Selected reference provider passes exact own synthetic base and refcounting flags");*module=reinterpret_cast<HMODULE>(0x80);return TRUE;}
BOOL WINAPI release_online(HMODULE module){require(reinterpret_cast<std::uintptr_t>(module)==0x80&&current->drained&&current->destroys==1&&current->frees==1,"Extra module references retire only after drained object and source module cleanup");++current->pins;return TRUE;}
configuration::Path read_copy_path(LPCWSTR p){configuration::Path result;for(std::size_t i=0;;++i){char16_t unit;std::memcpy(&unit,reinterpret_cast<const std::byte*>(p)+i*2,2);if(!unit)return result;result+=unit;}}
BOOL WINAPI copy_avatar(LPCWSTR from,LPCWSTR to,BOOL fail_if_exists){
    auto& st=*current;require(!fail_if_exists&&::GetCurrentThreadId()==st.graph_thread&&::FlsGetValue(st.domain->native_fls_index()),"Selected avatar copy preserves overwrite flag and exact Graph worker/PTD");
    st.avatar_copy_from.push_back(read_copy_path(from));st.avatar_copy_to.push_back(read_copy_path(to));return st.avatar_copy_from.size()==1?FALSE:TRUE;
}
struct OnlineOs final:recovered::libraries::SystemCalls {State& st;b12_test::Os inner;explicit OnlineOs(State& st):st(st){}std::uint32_t get_error_mode()override{return inner.get_error_mode();}std::uint32_t set_error_mode(std::uint32_t n)override{return inner.set_error_mode(n);}std::uintptr_t load_library(const wchar_t*)override{++st.loads;return 0x80;}bool free_library(std::uintptr_t n)override{require(n==0x80,"Only fake module freed");++st.frees;return false;}int wide_to_narrow(std::uint32_t c,std::uint32_t f,const wchar_t* p,int n,char* out,int z,bool* u)override{return inner.wide_to_narrow(c,f,p,n,out,z,u);}std::uint32_t last_error()override{return inner.last_error();}};
BOOL WINAPI no_module(DWORD,LPCWSTR,HMODULE* result){*result=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
HMODULE WINAPI no_load(LPCWSTR,HANDLE,DWORD){unused();}FARPROC WINAPI no_runtime_procedure(HMODULE,LPCSTR){unused();}BOOL WINAPI no_release(HMODULE){unused();}std::uint32_t no_flags(){unused();}
struct Domain {
    std::shared_ptr<thread_domain::DomainStorage> root;
    Domain(){thread_domain::InitialInputs in({0x15263748596a7b8cULL,0,nullptr,nullptr,1});in.apis.runtime={no_load,no_runtime_procedure,no_release,::GetLastError,no_flags};root=thread_domain::make_initial_domain(in);require(root->phase()==thread_domain::Phase::Constructed&&root->initialize(),"Owning initial-locale Domain factory returns passive root then explicitly initializes");}
    ~Domain(){if(!root->close())std::terminate();}
};
// Only SDK/legacy-document boundaries are controlled; real Configuration owns
// all active path/encoding/CRT/key/filesystem/persistence routes.
struct ConfigLifetime {
    std::shared_ptr<State> state;explicit ConfigLifetime(std::shared_ptr<State> s):state(std::move(s)){}
    ~ConfigLifetime(){auto& st=*state;++st.config_retired;st.path_retirement_valid=st.legacy_retired==1&&st.shared_paths&&st.shared_paths->xml_path==uR"(C:\MEM\local\anadius\LSX emu\achievement-selected.xml)"&&st.shared_paths->ini_path==uR"(C:\MEM\local\anadius\LSX emu\achievement_watcher\42\achievements.ini)"&&st.bound&&st.bound->configuration.username=="resource-user";}
};
struct Documents final:configuration_factory::LegacyDocumentReader {
    std::shared_ptr<State> state;explicit Documents(std::shared_ptr<State> s):state(std::move(s)){}~Documents()override{++state->legacy_retired;}
    configuration::Node read(const configuration::Path&)override{++state->legacy_calls;configuration::Node root;root.children["own-legacy"].value="owned-memory";return root;}
};
DWORD copy_configuration_path(std::u16string_view value,LPWSTR out,DWORD capacity){require(value.size()<capacity,"Own configuration SDK path fits selected source buffer");for(std::size_t i=0;i<value.size();++i)out[i]=static_cast<wchar_t>(value[i]);out[value.size()]=0;return static_cast<DWORD>(value.size());}
DWORD WINAPI configuration_module(HMODULE module,LPWSTR out,DWORD capacity){require(!module,"Actual configuration asks current process module");++current->module_paths;return copy_configuration_path(uR"(C:\MEM\host.exe)",out,capacity);}
DWORD WINAPI configuration_environment(LPCWSTR name,LPWSTR out,DWORD capacity){++current->environment_paths;const auto value=std::wstring_view(name);require(value==L"ProgramData"||value==L"LocalAppData","Actual configuration selects retained environment path names");return copy_configuration_path(value==L"ProgramData"?uR"(C:\MEM\program)":uR"(C:\MEM\local)",out,capacity);}

int WSAAPI startup(WORD,LPWSADATA){++current->startups;return 0;}int WSAAPI cleanup(){++current->cleanups;return 0;}
int WSAAPI local_name(SOCKET,sockaddr* out,int* size){std::memset(out,0,*size);out->sa_family=2;reinterpret_cast<unsigned char*>(out)[2]=0x30;reinterpret_cast<unsigned char*>(out)[3]=0x39;*size=16;return 0;}
BOOL WINAPI environment(LPCSTR key,LPCSTR value){require(std::string_view(key)=="EALsxPort"&&std::string_view(value)=="12345","Actual server publishes selected port through injected environment boundary");return FALSE;}
HANDLE WINAPI create_port(HANDLE file,HANDLE prior,ULONG_PTR key,DWORD count){auto handle=::CreateIoCompletionPort(file,prior,key,count);current->port=reinterpret_cast<std::uintptr_t>(handle);return handle;}BOOL WINAPI close_port(HANDLE handle){return ::CloseHandle(handle);}
BOOL PASCAL accept_sdk(SOCKET listener,SOCKET accepted,PVOID data,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED ov){require(listener==200&&accepted==201+current->accepts.size()&&data&&!receive&&local==0x90&&remote==0x90,"Actual accept uses bounded synthetic SDK endpoints and owned source buffer");current->accepts.push_back(ov);if(current->accepts.size()==1){require(::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(current->port),0,0,ov)!=0,"Own IOCP receives selected first accept packet");::SetEvent(current->armed);}*bytes=0;::WSASetLastError(997);return FALSE;}
VOID PASCAL addresses(PVOID data,DWORD,DWORD,DWORD,sockaddr** local,int* local_size,sockaddr** remote,int* remote_size){auto* bytes=static_cast<std::byte*>(data);std::fill_n(bytes,288,std::byte{});bytes[0]=bytes[144]=std::byte{2};*local=reinterpret_cast<sockaddr*>(bytes);*remote=reinterpret_cast<sockaddr*>(bytes+144);*local_size=*remote_size=16;}
int WSAAPI context(SOCKET,int,int,const char*,int){return 0;}SOCKET WSAAPI socket_factory(int,int,int,LPWSAPROTOCOL_INFOW,GROUP,DWORD){return 201+current->socket_count++;}BOOL WINAPI cancel_sdk(HANDLE,LPOVERLAPPED){unused();}
int WINAPI convert(UINT,DWORD,LPCCH,int,LPWSTR out,int size){std::memcpy(out,L"127.0.0.1",10*sizeof(wchar_t));return size;}
INT WSAAPI parse(LPWSTR,INT family,LPWSAPROTOCOL_INFOW,LPSOCKADDR out,LPINT size){if(family==23){::WSASetLastError(10022);return SOCKET_ERROR;}std::memset(out,0,*size);out->sa_family=2;*size=16;return 0;}
int WSAAPI sync_send(SOCKET,LPWSABUF b,DWORD,LPDWORD bytes,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE){*bytes=b->len;return 0;}int WSAAPI select_write(int,fd_set*,fd_set*,fd_set*,const timeval*){unused();}
int WSAAPI receive(SOCKET socket,LPWSABUF b,DWORD count,LPDWORD bytes,LPDWORD flags,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE cb){auto& st=*current;require(socket==201&&count==1&&!*bytes&&!*flags&&!cb,"Actual native receive forwards exact one-buffer SDK shape");const auto n=std::min<std::size_t>(b->len,st.wire.size()-st.cursor);if(n)std::memcpy(b->buf,st.wire.data()+st.cursor,n);st.cursor+=n;require(st.app->ports().completion.post(st.port,static_cast<std::uint32_t>(n),0,ov)!=0,"Own receive publishes bytes/EOF to actual IOCP");::WSASetLastError(997);return SOCKET_ERROR;}
int WSAAPI send(SOCKET,LPWSABUF b,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED ov,LPWSAOVERLAPPED_COMPLETION_ROUTINE){require(current->app->ports().completion.post(current->port,b->len,0,ov)!=0,"Own async send completion posted");::WSASetLastError(997);return SOCKET_ERROR;}
struct Sockets final:s::AcceptorApi {s::Socket create_socket(std::int32_t)override{return 200;}std::uintptr_t attach_completion_port(s::Socket,std::uintptr_t p)override{return p;}std::uint32_t get_win32_error()override{return ::GetLastError();}std::int32_t set_option(s::Socket,std::int32_t,std::int32_t,std::int32_t)override{return 0;}std::int32_t bind_socket(s::Socket,std::span<const std::byte>,std::int32_t)override{return 0;}std::int32_t listen_socket(s::Socket,std::int32_t)override{return 0;}std::int32_t close_socket(s::Socket)override{return 0;}std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}};
struct Dispatcher final:s::DispatcherApi {
    void set_last_error(std::uint32_t n)override{::SetLastError(n);}std::uint32_t get_last_error()override{return ::GetLastError();}
    std::int32_t get(std::uintptr_t p,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& ov,std::uint32_t timeout)override{DWORD n=0;ULONG_PTR k=0;auto result=::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(p),&n,&k,&ov,timeout);bytes=n;key=k;if(ov&&ov==current->terminal){::SetLastError(995);return 0;}return result;}
    std::int32_t post(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* ov)override{return ::PostQueuedCompletionStatus(reinterpret_cast<HANDLE>(p),n,k,ov);}std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
struct Cancel final:s::CancellationEffects {void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{unused();}};
struct Executor final:s::RegistryExecutorEffects {void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}};
void move_executor(void*,s::ExecutorFields& out,s::ExecutorFields& from,std::uintptr_t){out.context=from.context;out.zero_74=from.zero_74;out.self_reference_80=true;}
void configure(State& st,aw::Inputs& input){
    ++st.configured;st.graph_thread=::GetCurrentThreadId();b12_test::ui=&st.ui;auto& graph=input.graph;graph.policy.stream=st.stream;graph.policy.imports={::WSASetLastError,::WSAGetLastError,convert,parse,sync_send,select_write};graph.profile.sessions=1;graph.profile.jobs_capacity=2;graph.profile.headers=64;graph.profile.environment_capacity=256;graph.profile.receive={receive,::WSAGetLastError};graph.profile.send={send,::WSAGetLastError};graph.show_lsx=1;
    graph.prepare_session=[&st](sp::SessionView view){view.owner.cipher.set_fixed_key_084c98();view.owner.cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();for(const auto* body:{"<GetProfile index=\"0\"/>","<QueryImage Width=\"416\" Height=\"208\" ImageId=\"avatar:remote\"/>","<GetAuthCode ClientId=\"client\"/>","<QueryImage Width=\"128\" Height=\"64\" ImageId=\"avatar:owner\"/>","<QueryImage Width=\"128\" Height=\"64\" ImageId=\"avatar:owner\"/>","<QueryImage Width=\"128\" Height=\"64\" ImageId=\"avatar:owner\"/>","<QueryImage Width=\"416\" Height=\"208\" ImageId=\"avatar:owner\"/>","<QueryImage Width=\"416\" Height=\"208\" ImageId=\"avatar:owner\"/>"}){std::string request="<LSX><Request id=\"own\" recipient=\"EbisuSDK\">";request+=body;request+="</Request></LSX>";require(client.encrypt_042610(request),"Own independent fixed-key client encrypts selected request");messages_recovered::hex_encode_03d190(request);st.wire+=request;st.wire.push_back(0);}};
    graph.observe_log=[&st](sp::SessionView,application_graph::Log kind,std::string_view text,std::string_view){if(kind==application_graph::Log::Response){++st.responses;st.response_text.emplace_back(text);}};
    graph.policy.lifecycle.lifetime=st.stream;graph.policy.lifecycle.select_owner=[&st](sp::Application& app,s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t){st.app=&app;if(st.shutdown&&app.ports().accept.contains(&op)){require(error.value==995,"Controlled terminal pending-accept packet uses selected error995");return static_cast<s::IocpSchedulerEntry*>(nullptr);}return owner;};
    graph.policy.lifecycle.after=[&st](sp::Application& app){if(!st.shutdown&&st.responses==8&&!app.snapshot().live_sessions){st.shutdown=true;auto ports=app.ports();st.terminal=st.accepts.back();ports.accept.note_cancel_requested(ports.accept.handle_for(st.terminal));require(ports.completion.post(st.port,0,0,st.terminal)!=0,"Selected fake SDK cancellation publishes its pending operation to own IOCP");}};
    graph.policy.lifecycle.returned=[&st](sp::Application& app,std::uint64_t,const s::WorkerRunResult& result){require(result.error&&result.error->value==0&&!app.snapshot().live_sessions&&!app.snapshot().live_jobs,"Actual native worker source returns with all synthetic session/jobs retired");st.worker_returned=true;};
    graph.policy.lifecycle.external_drained=[&st](os::ServerView,sp::Application& app){require(st.worker_returned&&st.shutdown&&!app.snapshot().live_sessions&&!app.snapshot().live_jobs,"Selected synthetic SDK driver attests only after actual session/job source return");for(;;){DWORD n=0;ULONG_PTR key=0;OVERLAPPED* ov=nullptr;if(!::GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(st.port),&n,&key,&ov,0)){require(::GetLastError()==WAIT_TIMEOUT&&!ov,"Own IOCP proves no leftover operation packet");break;}require(!n&&!key&&!ov,"Only source-posted empty stop relay may remain");}st.drained=true;::SetEvent(st.done);return true;};
    auto sockets=std::make_shared<Sockets>();auto dispatcher=std::make_shared<Dispatcher>();input.server.sockets=sockets;input.server.dispatcher=dispatcher;s::IocpConstructorImports iocp;iocp.create_completion_port=create_port;iocp.close_handle=close_port;input.server.iocp=std::make_shared<s::NativeIocpConstructorApi>(iocp);input.server.gates=std::make_shared<s::NativeServiceGateApi>();input.server.cancellation=std::make_shared<Cancel>();input.server.executor=std::make_shared<Executor>();input.server.kernel={startup,cleanup,local_name,environment,::DeleteCriticalSection};input.runtime.sockets=sockets;input.runtime.dispatcher=dispatcher;input.runtime.accept=std::make_shared<s::NativeAsyncAcceptApi>(s::AsyncAcceptImports{accept_sdk,addresses,context,::WSAGetLastError});input.runtime.factory=std::make_shared<s::NativeSocketFactoryApi>(s::SocketFactoryImports{socket_factory});input.runtime.cancel=std::make_shared<s::NativeRetryCancelApi>(cancel_sdk);input.runtime.executor={nullptr,move_executor};input.runtime.capacity=16;
}

bootstrap_backend::Resources resources(const std::shared_ptr<State>& st){
    bootstrap_backend::Resources r{};r.state_lifetime=st;
    auto files=std::make_unique<b2_test::Files>(st->events);files->base=b2_test::base_text();const auto pos=files->base.find("\"SDK\" \"1.2.3\"");require(pos!=std::string::npos,"Own config insertion location");files->base.insert(pos," \"Documents\" \"C:/MEM/docs\" ");const auto end=files->base.rfind('}');files->base.insert(end," \"Achievements\" { \"AchievementsSet\" \"selected\" \"AchievementIDs\" \"own\" \"SteamAppId\" \"42\" \"SteamIdPrefix\" \"steam_\" \"AchievementTotals\" { \"own\" \"5\" } } ");files->over.clear();r.files=std::move(files);r.input=std::make_unique<b2_test::InputPlatform>(st->events);r.library_os=std::make_unique<b4_test::LibraryOS>(st->events);r.join_timeout_ms=10000;
    
    r.stop_sources=[st]{++st->stops;auto callbacks=st->callback_admission.lock();require(callbacks&&!callbacks->ready(),"Host callback admission is sealed BEFORE opaque source stopper");return st->stop_allowed&&::WaitForSingleObject(st->done,10000)==WAIT_OBJECT_0;};return r;
}
void inputs(rf::Inputs& in,const std::shared_ptr<State>& st,Domain& domain){
    st->domain=domain.root;in.domain=domain.root;in.stream=st->stream;in.dialog=st->dialog;in.waits=std::make_shared<services_completion::ExitWaitRegistry>(services_completion::InitialRegistry::empty_owned);in.threads=thread_native::native_api();in.threads.get_module=no_module;
    in.effects.content.files=std::make_shared<b8_test::MemoryFiles>(st->files);in.effects.licensing.files=std::make_shared<b8_test::MemoryFiles>(st->files);
    in.online.os=std::make_unique<OnlineOs>(*st);in.online.get_proc=lookup;in.online.code_lease=module_reference::make_code_lease({retain_online,release_online},st);in.online.foreign_quiescent=[st]{return st->drained;};in.online.files=std::make_unique<b8_test::MemoryFiles>(st->files);
    initial_cells::select_worker_defaults(in);require(in.worker.state&&in.worker.state->saved_avatar_size==-1&&in.worker.state->achievement_file.empty(),"Actual Worker uses observed -1 avatar seed and live Runtime path binding rather than manual zero seed");in.effects.content.copy_file=copy_avatar;in.worker.lifetime=st;in.worker.configure=[st](aw::Inputs& value){configure(*st,value);};
}
void preflight_and_actual_runtime(){
    Domain domain;auto st=std::make_shared<State>();auto r=resources(st);
    st->files->inputs[uR"(C:\MEM\local\anadius\LSX emu\achievement-selected.xml)"]="<root><row Id=\"own\" Count=\"1\" Progress=\"2\" Grant=\"2020-01-01T00:00:00\"/></root>";
    rf::ConfigurationInputs base;base.stream=st->stream;base.configuration.lifetime=std::make_shared<ConfigLifetime>(st);base.configuration.filesystem=std::make_shared<config_storage::Filesystem>(b8_test::fs_api());base.configuration.platform={configuration_module,configuration_environment,::GetLastError,::SetLastError};base.configuration.crt=b5_test::config_api();base.configuration.persistence.files=std::make_unique<b8_test::MemoryFiles>(st->files);base.configuration.persistence.stats=std::make_unique<b8_test::ProbeStats>(std::make_shared<b8_test::StatsState>());base.documents=std::make_unique<Documents>(st);base.console=b5_test::console_api();base.streams=b5_test::streams();
    r.console=std::make_unique<b2_test::Console>(st->events);auto* prior_console=r.console.get();rejects<std::invalid_argument>([&]{rf::install_configuration_console(r,base);},"Occupied base-provider slot rejects before input consumption");require(r.console.get()==prior_console&&base.documents&&!r.configuration,"Rejected base install preserves target and reader");r.console.reset();
    const auto reopen=base.console.reopen;base.console.reopen=nullptr;rejects<std::invalid_argument>([&]{rf::install_configuration_console(r,base);},"Incomplete console API rejects before either provider is selected");require(base.documents&&!r.configuration&&!r.console&&!st->module_paths,"Incomplete API starts no provider/SDK work");base.console.reopen=reopen;
    base.configuration.stream=shared_log::make_stream(std::make_shared<std::stringbuf>());rejects<std::invalid_argument>([&]{rf::install_configuration_console(r,base);},"Configuration logger must share exact managed root stream");base.configuration.stream.reset();
    auto documents=std::move(base.documents);rejects<std::invalid_argument>([&]{rf::install_configuration_console(r,base);},"Narrow legacy reader remains explicit mandatory input");base.documents=std::move(documents);
    rf::install_configuration_console(r,base);auto* selected_configuration=dynamic_cast<configuration_factory::Configuration*>(r.configuration.get());require(selected_configuration&&dynamic_cast<bootstrap_providers::CompilerConsole*>(r.console.get())&&selected_configuration->accepts_observed_empty_paths(),"Actual native-key Configuration and CompilerConsole are selected without exposing empty Runtime path cells");
    require(!base.stream&&!base.documents&&!base.configuration.lifetime&&!base.configuration.stream&&!st->module_paths&&!st->environment_paths&&!st->console_sdk.attaches&&st->filesystem_sdk.creates.empty()&&st->files->reads.empty()&&st->files->writes.empty()&&!st->legacy_calls,"Concrete base install is passive and consumes owning inputs");rejects<std::logic_error>([&]{rf::install_configuration_console(r,base);},"Base-provider inputs are single-use");
    rf::Inputs in;inputs(in,st,domain);
    const auto errors=permanent_errors::compiler_abort_policy();
    std::unique_ptr<recovered_hooks::Api> lower_engine=std::make_unique<HookEngine>();st->hooks=static_cast<HookEngine*>(lower_engine.get());std::shared_ptr<void> engine_code=st;auto owned_engine=recovered_hooks::make_owned_engine(lower_engine,engine_code);
    host_platform::AssemblyInputs host;host.domain=domain.root;host.stream=st->stream;host.callbacks.memory=domain.root->make_memory();host.permanent_errors=&errors.host;host.host.api=b14_test::api();
    initial_cells::select_defaults(host,in);require(host.seed&&host.seed->client.empty()&&host.seed->language.empty()&&host.seed->fabricated.empty()&&host.seed->ipv4_length==16&&host.seed->ipv6_length==28&&!host.seed->attempts&&in.effects.attempts&&!in.effects.attempts->attempted_files&&!in.effects.attempts->blocked,"Actual host/resources assembly selects evidence-backed native callback and license initial cells instead of fixture seed values");
    host.providers_ready=[st]{return st->provider_ready&&st->hooks->calls.init==0&&st->hooks->calls.targets.size()==st->hooks->calls.enabled.size();};
    host.external_quiescence=[st]{return st->external&&st->drained;};host_platform::select_owned_engine(host,owned_engine);
    r.host=std::make_unique<b4_test::Host>(st->events);auto* occupied_host=r.host.get();rejects<std::invalid_argument>([&]{(void)host_platform::install_host(r,host);},"Occupied host output rejects without consuming host inputs");
    require(r.host.get()==occupied_host&&host.engine==owned_engine&&host.seed&&host.providers_ready&&!r.callback_attachment,"Rejected host install preserves owners, seed and callable donors");r.host.reset();
    host.callbacks.stream=shared_log::make_stream(std::make_shared<std::stringbuf>());rejects<std::invalid_argument>([&]{(void)host_platform::install_host(r,host);},"Host callback stream must match exact managed root");host.callbacks.stream.reset();
    const auto callback_seed=host.seed;host.seed.reset();rejects<std::invalid_argument>([&]{(void)host_platform::install_host(r,host);},"Host callback seed cannot be silently fabricated");host.seed=callback_seed;
    auto source_stop=std::move(r.stop_sources);r.stop_sources={};rejects<std::invalid_argument>([&]{(void)host_platform::install_host(r,host);},"Host assembly requires explicit source stopper before consuming host settings");require(host.seed&&host.engine&&!r.callback_attachment,"Missing source stopper leaves host settings unconsumed");r.stop_sources=std::move(source_stop);source_stop={};
    auto installed_host=host_platform::install_host(r,host);st->callback_admission=installed_host.callbacks;
    require(dynamic_cast<host_platform::HostPlatform*>(r.host.get())&&dynamic_cast<host_platform::HookCleanup*>(r.hook_cleanup.get())&&r.callback_attachment&&r.startup_publication==installed_host.milestones,"Host installer commits concrete engine-backed host/cleanup and matched native callback/publication pair");
    require(!host.domain&&!host.stream&&!host.engine&&!host.code_lifetime&&!host.seed&&!host.providers_ready&&!host.external_quiescence&&!host.callbacks.memory&&!host.callbacks.stream&&!host.callbacks.originals&&!native_recovered::is_bound()&&!st->hooks->calls.init_calls,"Passive host install consumes donors without binding native entries or calling engine");
    rejects<std::logic_error>([&]{(void)host_platform::install_host(r,host);},"Host inputs are single-use");
    auto attachment=std::move(r.callback_attachment);r.callback_attachment=[st,attachment=std::move(attachment)](bootstrap_backend::CallbackStateView view){require(!st->bound,"Runtime view observed once");st->bound.emplace(view);return attachment(view);};attachment={};
    require(!r.threads&&!r.remaining&&!r.remaining_factory&&!r.online&&!r.online_factory&&!r.server&&!r.application_worker&&!r.library_effects&&!st->loads&&!st->startups&&st->files->reads.empty(),"Selected target slots and owned installer settings start passive");
    auto* configuration=r.configuration.get();auto* online_os=in.online.os.get();const auto attempts=in.effects.attempts;auto* domain_identity=in.domain.get();
    r.server=std::make_unique<b4_test::Server>(st->events);auto* occupied=r.server.get();rejects<std::invalid_argument>([&]{rf::install(r,in);},"Occupied selected output slot rejects before consuming inputs");
    require(r.server.get()==occupied&&r.configuration.get()==configuration&&in.online.os.get()==online_os&&in.domain.get()==domain_identity&&in.effects.attempts==attempts&&in.worker.configure&&in.online.code_lease&&!r.threads&&!r.remaining_factory,"Rejected occupied target preserves owners, callable donors and target exactly");r.server.reset();
    in.effects.information.stream=shared_log::make_stream(std::make_shared<std::stringbuf>());auto wrong_stream=in.effects.information.stream;rejects<std::invalid_argument>([&]{rf::install(r,in);},"Nonnull stream differing from root rejects before consumption");
    require(in.effects.information.stream==wrong_stream&&in.online.os.get()==online_os&&in.worker.configure&&!r.application_worker&&!r.library_effects&&!r.threads,"Stream mismatch retains donors and publishes no selected provider");in.effects.information.stream.reset();
    in.effects.attempts.reset();rejects<std::invalid_argument>([&]{rf::install(r,in);},"Attempt/block root remains mandatory");require(!r.threads&&in.online.os.get()==online_os&&in.worker.configure,"Missing attempt root does not consume or partially install");in.effects.attempts=attempts;
    rf::install(r,in);
    require(dynamic_cast<bootstrap_providers::NativeThreads*>(r.threads.get())&&r.remaining_factory&&r.online_factory&&r.application_worker&&dynamic_cast<library_effects::BoundLibrary*>(r.library_effects.get())&&!r.remaining&&!r.online&&!r.server,"Installer adopts actual same-Domain NativeThreads, narrow factories and BoundLibrary without legacy substitutes");
    require(!in.worker.configure&&!in.worker.effects&&!in.online.code_lease&&!in.online.os&&!in.effects.attempts&&!st->loads&&!st->startups&&!st->configured&&st->files->reads.empty(),"Successful passive install consumes owning callable/provider donors before publication");
    rejects<std::logic_error>([&]{rf::install(r,in);},"Consumed Inputs is single-use");
    auto runtime=initial_cells::make_fresh_runtime(r,errors.bootstrap,b4_test::host_api());st->shared_paths=&selected_configuration->paths();
    require(st->bound&&(&st->bound->configuration==&runtime->configuration())&&st->shared_paths&&runtime->phase()==bootstrap_owner::Phase::Constructed&&!st->configured&&!st->loads&&!st->startups&&st->host.registrations==0,"Actual Runtime binds installed Online/Worker/phase/path roots passively before publication");
    require(!st->bound->runtime.frequency_cache&&st->bound->runtime.registry_values.empty()&&st->bound->runtime.enabled_targets.empty()&&std::all_of(st->bound->runtime.originals.begin(),st->bound->runtime.originals.end(),[](auto value){return value==0;})&&runtime->configuration().username=="anadius"&&runtime->configuration().content_id=="0"&&st->bound->runtime.execution_mutex.type==2,"Actual installed Resources now uses combined observed constructor and outside-image defaults before host setup fills original slots");
    const auto selected=runtime->selected_defaults();require(selected.execution&&selected.thread_join&&!selected.files&&!selected.input&&!selected.library_os,"Runtime keeps injected memory/config boundaries and selects existing concrete execution/native join defaults");
    runtime->publish();const auto cached_application=runtime->application_entry();require(cached_application(nullptr,1,nullptr)==TRUE,"Actual source application entry starts installed three Domain workers");
    require(::WaitForSingleObject(st->done,10000)==WAIT_OBJECT_0,"Own controlled SDK driver reaches source drain before controller activation");
    for(unsigned n=0;!installed_host.milestones->observed_all()&&n<2000;++n)::Sleep(1);
    require(installed_host.milestones->observed_all()&&installed_host.milestones->matches_execution(&st->bound->execution),"Installed host and three actual workers publish same-Execution milestones");
    require(st->hooks->calls.targets.size()==8&&st->bound->runtime.enabled_targets==st->hooks->calls.enabled,"Actual host source selects six base and two document hooks through same selected engine");
    for(std::size_t i=0;i<st->hooks->calls.detours.size();++i){std::size_t slot=0;while(slot<10&&st->hooks->resolver.rebuilt_address(static_cast<host_init_recovered::Callback>(slot))!=st->hooks->calls.detours[i])++slot;require(slot<10&&st->bound->runtime.originals[slot]==b15_test::originals()[slot],"Engine trampoline result reaches exact stable Backend original slot");}
    st->provider_ready=false;require(!installed_host.callbacks->activate_foreign(),"Controller activation cannot override missing provider proof");st->provider_ready=true;require(installed_host.callbacks->activate_foreign(),"Controller activates completed same-Execution source publication");
    b15_test::Probe connection_probe;b15_test::active_probe=&connection_probe;bool foreign=false,redirect4=false,redirect6=false;std::thread callback([&]{std::array<wchar_t,260> path{};::SetLastError(0x4512);foreign=native_recovered::folder_w_entry(nullptr,5,nullptr,0,path.data())==0&&std::wstring(path.data())==L"C:/MEM/docs"&&::GetLastError()==0x4512&&::FlsGetValue(domain.root->native_fls_index())==nullptr;
        sockaddr_in v4{};v4.sin_family=AF_INET;v4.sin_port=htons(3216);redirect4=native_recovered::connect_entry(1,reinterpret_cast<const sockaddr*>(&v4),16)==-10035&&ntohs(v4.sin_port)==12345;
        sockaddr_in6 v6{};v6.sin6_family=AF_INET6;v6.sin6_port=htons(3216);redirect6=native_recovered::connect_entry(1,reinterpret_cast<const sockaddr*>(&v6),28)==-10035&&ntohs(v6.sin6_port)==12345;
    });callback.join();b15_test::active_probe=nullptr;
    require(redirect4&&redirect6&&connection_probe.connects==2&&st->ui.messages.empty()&&!attempts->attempted_files&&!attempts->blocked,"Actual foreign native IPv4/IPv6 hooks use selected lengths, preserve own original result and leave independent license attempt cells unchanged");
    require(foreign&&!installed_host.callbacks->active(),"Real rebuilt foreign callback uses shared host map, counted wide copy and transient same-Domain PTD/FLS");
    st->stop_allowed=false;require(!runtime->close()&&st->stops==1&&!installed_host.callbacks->ready()&&st->hooks->calls.disabled.empty()&&!st->destroys,"Source stop false seals callback admission without joins/provider/hook cleanup");
    require(cached_application(nullptr,1,nullptr)==FALSE&&st->configured==1&&st->startups==1&&st->hooks->calls.init_calls==1,"Cached Application entry is already sealed while external stopper requests retry");
    st->stop_allowed=true;st->external=true;st->hooks->calls.uninit=MH_ERROR_MEMORY_PROTECT;
    require(!runtime->close()&&native_recovered::is_bound()&&!installed_host.callbacks->ready()&&st->stops==2&&!owned_engine->empty(),"Actual failed engine cleanup retains native binding even when external proof says true");
    require(st->hooks->calls.disabled==st->hooks->calls.enabled&&st->hooks->calls.uninit_calls==1,"Runtime hook cleanup preserves same engine/targets and failed uninitialize result");
    require(!runtime->close()&&st->hooks->calls.uninit_calls==1&&st->stops==2,"Close retry does not replay consumed source cleanup after failed engine release");
    st->hooks->calls.uninit=MH_OK;require(owned_engine->uninitialize()==MH_OK&&owned_engine->empty()&&st->drained,"Explicit controller repair after real foreign drain establishes engine empty state");
    st->external=false;require(!runtime->close()&&native_recovered::is_bound()&&st->hooks->calls.uninit_calls==2,"Engine empty state alone does not replace external quiescence proof");
    st->external=true;if(!runtime->close()){std::cerr<<"Runtime retained uncompleted installed driver prefix\n";std::_Exit(1);}
    require(!native_recovered::is_bound()&&st->hooks->calls.uninit_calls==2&&st->stops==2,"Explicit external proof closes native binding without repeating source stop or engine cleanup");
    require(runtime->phase()==bootstrap_owner::Phase::Closed&&!bootstrap_native_recovered::is_bound()&&st->configured==1&&st->startups==1&&st->cleanups==1&&st->drained&&st->responses==8,"Actual SourceBackend joins installed workers and retires concrete Worker/Graph after observed synthetic driver drain");
    require(st->module_paths==1&&st->environment_paths>0&&!st->legacy_calls&&st->console_sdk.attaches==1&&st->console_sdk.allocations==1&&st->console_sdk.moves==1&&st->console_sdk.opens==std::vector<std::string>{"conin$:r","conout$:w","conout$:w"},"Actual configuration FilePass bypasses legacy reader; real compiler-console preserves attach/allocate/move/three reopen source calls at controlled SDK boundaries");
    require(st->files->outputs.at(runtime->configuration().avatar).size()==config_storage::embedded_png(configuration::EmbeddedPng::default_user_1800809d0).size()&&st->files->outputs.at(runtime->configuration().default_avatar).size()==config_storage::embedded_png(configuration::EmbeddedPng::default_avatar_180082710).size()&&st->files->outputs.at(st->shared_paths->ini_path).find("[steam_own]\r\nAchieved=1\r\nCurProgress=2\r\nMaxProgress=5\r\n")!=std::string::npos&&runtime->configuration().achievements.at("own").field04==1,"Real configuration Persistence writes retained PNGs and restored XML/INI through owned memory IO and actual live Runtime paths");
    require(st->response_text[0].find("resource-user")!=std::string::npos&&st->response_text[1].find("_AVATAR_remote_416")!=std::string::npos&&st->response_text[2].find(license_recovered::AuthenticationCode{}.value())!=std::string::npos,"Installed actual Info/Content/Licensing handlers execute profile/image/auth using automatically bound roots");
    require(st->avatars==6&&st->auths==1&&st->avatar_owner=="owner"&&st->avatar_size==2&&st->slot_valid&&st->graph_thread!=::GetCurrentThreadId()&&st->online_thread!=::GetCurrentThreadId()&&st->graph_thread!=st->online_thread,"Two distinct admitted Domain workers use actual own typed Online queries and selected Graph SDK driver");
    require(st->bound->runtime.server_ready&&st->bound->runtime.online_ready&&st->bound->runtime.libraries_ready&&!st->bound->runtime.online_object&&!st->bound->runtime.online_module&&st->destroys==1&&st->frees==1&&st->pins==2,"Real phase binding completes all source milestones and exit cleanup destroys/frees own object/code prefixes once");
    require(st->avatar_requests==std::vector<std::pair<std::string,std::int32_t>>{{"remote",2},{"owner",0},{"owner",0},{"owner",0},{"owner",2},{"owner",2}},"Actual native requests preserve owner and scaled size through all avatar cache cases");
    require(st->avatar_copy_from.size()==3&&st->avatar_copy_from[0].ends_with(u"_AVATAR_owner_128")&&st->avatar_copy_from[1]==st->avatar_copy_from[0]&&st->avatar_copy_from[2].ends_with(u"_AVATAR_owner_416")&&std::all_of(st->avatar_copy_to.begin(),st->avatar_copy_to.end(),[&](const auto& path){return path==runtime->configuration().last_avatar_path;}),"Native -1 permits first small avatar copy, failed copy retries, equal successful size skips and larger size upgrades");
    const auto log=st->buffer->str();const auto zero=log.find("saved size: 0\n"),two=log.find("saved size: 2\n");require(zero!=std::string::npos&&two!=std::string::npos&&zero<two&&log.find("saved size: ",zero+1)==two&&log.find("saved size: ",two+1)==std::string::npos,"Only successful avatar copies publish saved size/log; fail and equal-size hits do not");
    require(in.waits->active()==0&&in.waits->close(),"Actual selected exit registry is quiescent/closed after source cleanup");auto external_memory=domain.root->make_memory();auto domain_donor=domain.root;auto unload=pre_unload::Controller::take(runtime,domain_donor);const auto close_entry=pre_unload::Controller::entry();
    require(!close_entry(unload->context())&&!runtime&&!domain_donor&&unload->snapshot().phase==pre_unload::Phase::ClosingDomain,"Actual pre-unload entry retires closed Runtime but retains Domain while explicit external provider lease remains");
    require(st->config_retired==1&&st->path_retirement_valid,"Opaque configuration retirement sees live actual state/path roots before aggregate release");
    installed_host.callbacks.reset();installed_host.milestones.reset();external_memory.reset();require(domain.root->provider_leases()==0,"All worker and retained host control handles release Domain leases before Domain close");st->bound.reset();owned_engine.reset();st.reset();require(close_entry(unload->context())&&unload->closed()&&domain.root->phase()==thread_domain::Phase::Closed,"Actual pre-unload entry closes whole own Domain after real workers/FLS and all retained host/provider controls retire");unload.reset();
}
}
int main(){try{preflight_and_actual_runtime();std::cout<<"PASS: "<<checks<<" Resources installer checks; actual Runtime/three Domain workers and assembled native host callbacks; actual Configuration/CompilerConsole; controlled path/FS/CRT/engine/socket SDK boundaries\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';std::_Exit(1);}}
