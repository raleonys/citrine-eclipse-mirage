#include "effects.hpp"
#include "../anadius-application-io-source/call.hpp"
#include "../anadius-initial-cells-source/defaults.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-token-source/constants.hpp"
#include "../anadius-crypto-source/crypto.hpp"
#include "../anadius-xml-source/memory_printer.hpp"
#include <bit>
#include <atomic>
#include <exception>
#include <cstring>
#include <iostream>
#include <process.h>
#include <sstream>
#include <thread>

// Synthetic memory DLL/token bytes and callable slots in this EXE only. Real
// files, configuration, token stores, original code and native exit never run.
namespace {
using namespace anadius;namespace le=licensing_effects;namespace of=online_factory;using Path=configuration::Path;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Exit {int code;};
struct Buffer final:std::stringbuf {unsigned flushes=0;std::function<void(unsigned)> hook;int sync()override{++flushes;if(hook)hook(flushes);return std::stringbuf::sync();}};
struct Trace {
    of::Online* online=nullptr;std::array<std::uintptr_t,7> table{};std::array<std::byte,17> object{};
    unsigned loads=0,auths=0,destroyed=0,pins=0,time=0,gmtime=0,formatted=0,module_calls=0,procedure_calls=0,wine_calls=0,initial=0,reads=0,closes=0,input_destroyed=0;
    bool valid=true,deny=false,module_present=true;std::string client,auth_value="online<&",pattern;std::function<void()> open_hook,read_hook;
    online_recovered::Object identity(){return reinterpret_cast<online_recovered::Object>(object.data()+1);}
    static void checkpoint(void* raw){if(static_cast<Trace*>(raw)->deny)throw std::logic_error("selected licensing admission denied");}
};
Trace* current=nullptr;
void destroy(void* object){current->valid=current->valid&&object==reinterpret_cast<void*>(current->identity());++current->destroyed;}
std::int32_t status(void*){throw std::logic_error("unselected status");}
const char* getter(void*){throw std::logic_error("unselected getter");}
std::uint8_t avatar(void*,const char16_t*,const char*,std::int32_t){throw std::logic_error("unselected avatar");}
const char* auth(void* object,const char* client){auto& t=*current;++t.auths;t.client=client;const auto s=t.online->snapshot().binding;t.valid=t.valid&&object==reinterpret_cast<void*>(t.identity())&&s.active&&s.object==t.identity()&&!s.object_unknown&&s.factory_code_retained;return t.auth_value.c_str();}
online_recovered::Object factory(const char16_t*){auto& t=*current;t.table={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};const auto table=reinterpret_cast<std::uintptr_t>(t.table.data());std::memcpy(t.object.data()+1,&table,sizeof table);return t.identity();}
FARPROC WINAPI online_lookup(HMODULE module,LPCSTR name){require(reinterpret_cast<std::uintptr_t>(module)==0x80&&std::string_view(name)=="create_online","Only own online factory is resolved");return std::bit_cast<FARPROC>(&factory);}
void wine_function(){++current->wine_calls;}
HMODULE WINAPI module_call(LPCSTR name){++current->module_calls;require(std::string_view(name)=="ntdll.dll","Wine helper uses exact module-name query");return current->module_present?reinterpret_cast<HMODULE>(0x200):nullptr;}
FARPROC WINAPI procedure_call(HMODULE module,LPCSTR name){++current->procedure_calls;require(module==reinterpret_cast<HMODULE>(0x200)&&std::string_view(name)=="wine_get_version","Wine helper resolves exact symbol on returned module");return std::bit_cast<FARPROC>(&wine_function);}
void exit_call(int code){throw Exit{code};}
HMODULE WINAPI unexpected_marker(LPCSTR){throw le::NoResponseRequiredError{};}
struct Pin {Trace& trace;~Pin(){++trace.pins;}};
struct Clock final:achievement_runtime::Clock {Trace& t;explicit Clock(Trace& t):t(t){}std::int64_t time_05eeb8()override{++t.time;return 1577836800;}std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t value)override{++t.gmtime;require(value==1577836800,"Filename UTC forwards controlled clock value");return achievement_runtime::BrokenDown{0,0,0,1,0,120,3,0,0};}};
struct Formatter final:achievement_runtime::Formatter {Trace& t;explicit Formatter(Trace& t):t(t){}std::string put_time_03d610(const achievement_runtime::BrokenDown&,std::string_view pattern)override{++t.formatted;t.pattern=pattern;return "2020-01-01_00-00-00";}};
struct Files final:file_recovered::Factory {
    Trace& t;std::shared_ptr<b8_test::MemoryFilesState> memory;b8_test::MemoryFiles outputs;
    Files(Trace& t,std::shared_ptr<b8_test::MemoryFilesState> memory):t(t),memory(std::move(memory)),outputs(this->memory){}
    struct Input final:file_recovered::Input {
        Trace& t;std::string bytes;bool exists;std::size_t position=0;
        Input(Trace& t,std::string bytes,bool exists):t(t),bytes(std::move(bytes)),exists(exists){}~Input()override{++t.input_destroyed;}
        std::uint32_t initial_state()const override{++t.initial;return exists?0:2;}
        void seek_03b610(std::int64_t offset,file_recovered::Seek mode)override{require(offset==0,"Memory binary reader seeks zero");position=mode==file_recovered::Seek::end?bytes.size():0;}
        std::uint64_t position_sum_03ba50()override{return position;}
        std::int32_t peek_043790()override{return position==bytes.size()?-1:static_cast<unsigned char>(bytes[position]);}
        std::size_t read_03b2a0(std::span<std::uint8_t> target)override{++t.reads;const auto count=std::min(target.size(),bytes.size()-position);if(count)std::memcpy(target.data(),bytes.data()+position,count);position+=count;if(t.read_hook)t.read_hook();return count;}
        void close_03ae70()override{++t.closes;}
    };
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& p,std::uint32_t mode,std::uint32_t share)override{require(mode==0x20&&share==0x40,"Actual hash/token helpers preserve binary/share flags");memory->reads.push_back(p);auto found=memory->inputs.find(p);auto raw=std::make_unique<Input>(t,found==memory->inputs.end()?std::string{}:found->second,found!=memory->inputs.end());if(t.open_hook)t.open_hook();return raw;}
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t mode,std::uint32_t share)override{return outputs.open_output_03a1a0(p,mode,share);}
};
struct Stats final:status_recovered::Stats {std::shared_ptr<b8_test::MemoryFilesState> memory;explicit Stats(std::shared_ptr<b8_test::MemoryFilesState> m):memory(std::move(m)){}status_recovered::StatsResult query(const Path& p,std::uint32_t flags)override{require(flags==3,"Licensing status uses exact flags3");return memory->inputs.contains(p)?status_recovered::StatsResult{0,FILE_ATTRIBUTE_NORMAL,{}}:status_recovered::StatsResult{ERROR_FILE_NOT_FOUND,0,{}};}};
struct Request final:messages_recovered::XmlNode {
    std::map<std::string,std::string,std::less<>> values;
    std::string_view name()const override{return "own";}const messages_recovered::XmlNode* first_element()const override{return nullptr;}const messages_recovered::XmlNode* next_element()const override{return nullptr;}
    std::optional<std::string_view> attribute(std::string_view key)const override{const auto found=values.find(key);if(found==values.end())return std::nullopt;return found->second;}
};
struct Reader final:xml_recovered::AttributeReader {const char* attribute(handlers_recovered::Request,const char*)override{throw std::logic_error("Output-only XML must not query request attributes");}};
struct Output {xml_recovered::MemoryPrinter printer;Reader reader;xml_recovered::HandlerWriter xml{printer,reader};std::string bytes()const{return std::string(printer.bytes_without_terminator());}};
struct Fixture {
    configuration::State config{};bootstrap_backend::RuntimeSeed runtime{};b2_test::Events events;b4_test::Execution platform{events};execution_recovered::Execution execution;
    license_recovered::AuthenticationCode authentication;std::shared_ptr<le::AttemptState> attempts=std::make_shared<le::AttemptState>(initial_cells::make_license_attempt_defaults());Trace trace;b12_test::Ui ui;
    std::shared_ptr<b8_test::MemoryFilesState> memory=std::make_shared<b8_test::MemoryFilesState>();std::shared_ptr<Buffer> buffer=std::make_shared<Buffer>();std::shared_ptr<std::ostream> stream=std::make_shared<std::ostream>(buffer.get());std::shared_ptr<library_effects::Dialog> dialog;
    std::unique_ptr<of::Online> online;std::unique_ptr<le::Core> core;std::unique_ptr<le::Handlers> handlers;
    bootstrap_backend::CallbackStateView view(){return {config,runtime,execution};}
    le::Options options(){le::Options o;o.lifetime=buffer;o.stream=stream;o.dialog=dialog;o.files=std::make_shared<Files>(trace,memory);o.stats=std::make_shared<Stats>(memory);o.clock=std::make_shared<Clock>(trace);o.formatter=std::make_shared<Formatter>(trace);o.module=module_call;o.procedure=procedure_call;o.exit_process=exit_call;return o;}
    Fixture():execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform){current=&trace;b12_test::ui=&ui;dialog=std::make_shared<library_effects::Dialog>(b12_test::dialog_api());execution_recovered::initialize_mutex_04a698(runtime.execution_mutex,2);runtime.frequency_cache=10000000;config.denuvo_dll=u"MEM\\own.dll";config.executable_path=u"MEM\\own.exe";config.license_directory=u"MEM\\license";config.content_id="own-content";config.dll_hash="A9993E364706816ABA3E25717850C26C9CD0D89D";memory->inputs[config.denuvo_dll]="abc";
        of::Options o;o.stream=stream;o.dialog=dialog;auto os=std::make_unique<b12_test::Os>();os->load_hook=[&](const wchar_t*){++trace.loads;return std::uintptr_t{0x80};};o.os=std::move(os);o.get_proc=online_lookup;o.code_lease=[&](online_recovered::Module){return std::shared_ptr<void>(new Pin{trace});};o.foreign_quiescent=[]{return true;};o.files=std::make_unique<b8_test::MemoryFiles>(memory);online.reset(static_cast<of::Online*>(of::make_online(view(),o).release()));trace.online=online.get();core=std::make_unique<le::Core>(view(),*online,options());handlers=std::make_unique<le::Handlers>(view(),*online,attempts,options());}
    void bind(){handlers->bind_authentication_code(authentication);}
    void adopt(){runtime.online_module=online->load_copy_0450d0(u"OWN-EXE");const auto create=online->get_proc_address(runtime.online_module,"create_online");runtime.online_object=online->call_create_online(create,u"own-cookie");runtime.online=1;}
    ~Fixture(){handlers->clear_content_checkpoint();handlers.reset();core->clear_content_checkpoint();core.reset();if(runtime.online_object){online->destroy_virtual_00(runtime.online_object);runtime.online_object=0;}if(runtime.online_module){online->free_library(runtime.online_module);runtime.online_module=0;}if(!online->close_after_cleanup())std::terminate();online.reset();b12_test::ui=nullptr;current=nullptr;}
};
std::string encrypted(std::string_view token){std::string result(65,'H');result+="<GameToken>";result+=token;result+="</GameToken>";crypto_recovered::Cipher cipher;cipher.set_key_and_iv_042760(token_recovered::constants::key_084a50,token_recovered::constants::iv_084a60);require(cipher.encrypt_with_offset(result,65),"Own synthetic token is encrypted with actual recovered AES helper");return result;}
void unknown_diagnostic(){
    Fixture f;
    struct Input final:messages_recovered::UnknownMessageInput {
        std::shared_ptr<Buffer> buffer;std::string value="before";mutable std::vector<unsigned> reads;
        explicit Input(std::shared_ptr<Buffer> b):buffer(std::move(b)){}
        std::string_view view()const override{reads.push_back(buffer->flushes);return value;}
    } input(f.buffer);
    f.buffer->hook=[&](unsigned n){input.value=n==1?std::string("L\0D",3):"dialog-new";};
    f.handlers->unknown_message_07bd70(input);
    f.buffer->hook={};
    require(input.reads==std::vector<unsigned>{1,2}&&f.buffer->flushes==2,"Diagnostic samples borrowed input after each of two flushes");
    require(f.buffer->str()==std::string("[UNKNOWN MESSAGE]\nL\0D\n",22),"Counted request log preserves embedded NUL and includes exact prefix/newlines");
    require(f.ui.messages.size()==1&&f.ui.messages[0].title==L"UNKNOWN MESSAGE"&&f.ui.messages[0].text==L"dialog-new"&&f.ui.messages[0].flags==0x10,"Dialog owns freshly resampled request after second flush and exact title/flags");
    require(f.ui.conversions.size()==4&&f.ui.conversions[0].input=="UNKNOWN MESSAGE"&&f.ui.conversions[2].input=="dialog-new","Dialog conversions receive full owned source strings");
    require(!f.trace.auths&&!f.trace.loads&&!f.trace.reads&&!f.attempts->attempted_files&&!f.attempts->blocked&&f.memory->reads.empty()&&f.memory->writes.empty(),"Independent diagnostic needs no authentication binding, online call or licensing/file mutation");
    f.bind();Request request;request.values.emplace("ClientId","own");Output output;require(f.handlers->auth_code_02e3b0(request,output.xml),"Normal admitted handler remains usable after diagnostic");
}
void actual_auth_license_and_cache(){
    Fixture f;require(f.memory->reads.empty()&&f.memory->writes.empty()&&!f.trace.loads&&!f.trace.auths&&!f.trace.time&&f.ui.messages.empty()&&f.buffer->str().empty(),"Concrete Core/Handlers construction performs no I/O, online call or publication");f.bind();f.adopt();Request auth_request;auth_request.values.emplace("ClientId",std::string("client\0tail",11));Output auth_xml;
    require(f.handlers->auth_code_02e3b0(auth_request,auth_xml.xml)&&auth_xml.bytes().find("online&lt;&amp;")!=std::string::npos&&f.trace.auths==1&&f.trace.client=="client"&&f.trace.valid,"Actual bound Handlers select Core owned-auth→Online.copy_auth with leased own EXE slot30 and client C-string boundary");
    f.trace.auth_value.clear();Output fallback;require(f.handlers->auth_code_02e3b0(auth_request,fallback.xml)&&fallback.bytes().find(f.authentication.value())!=std::string::npos,"Empty owned online auth preserves actual bound authentication root");
    const auto hash=f.config.dll_hash;f.memory->inputs[u"MEM\\license.dlf"]=encrypted("PRIMARY<&");Request request;Output primary;
    require(f.handlers->request_license_0320c0(request,primary.xml)&&f.config.denuvo_token=="PRIMARY<&"&&primary.bytes().find("PRIMARY&lt;&amp;")!=std::string::npos&&f.attempts->attempted_files==1&&!f.attempts->blocked&&f.config.dll_hash==hash,"Actual status/SHA-1/AES token route emits license and updates explicit attempts without mutating copied hash");
    f.config.denuvo_token="LIVE-TOKEN";Output live;require(f.handlers->request_license_0320c0(request,live.xml)&&live.bytes().find("LIVE-TOKEN")!=std::string::npos&&f.attempts->attempted_files==1,"Handler reads live State token instead of a constructor snapshot");
    Output invalidated;require(f.handlers->invalidate_license_030c50(request,invalidated.xml)&&f.config.denuvo_token.empty()&&f.attempts->attempted_files==1&&!f.attempts->blocked,"Invalidate clears only live token, preserving attempt/block roots");f.memory->inputs[u"MEM\\license_cached.dlf"]=encrypted("CACHED");Output cached;
    require(f.handlers->request_license_0320c0(request,cached.xml)&&f.config.denuvo_token=="CACHED"&&f.attempts->attempted_files==2&&std::count(f.memory->reads.begin(),f.memory->reads.end(),u"MEM\\license_cached.dlf")==1,"Next explicit attempt selects actual cached path and decrypts memory fallback token");
    Output again;f.handlers->invalidate_license_030c50(request,again.xml);Output exhausted;require(!f.handlers->request_license_0320c0(request,exhausted.xml)&&f.attempts->attempted_files==2&&f.attempts->blocked==1,"Exhausted explicit attempts with no request ticket block without an invented retry or native exit");rejects<le::NoResponseRequiredError>([&]{f.handlers->request_license_0320c0(request,exhausted.xml);},"Blocked request uses explicit HOST no-response marker");require(f.trace.closes>0&&f.trace.input_destroyed==f.trace.closes,"Normal hash/token streams close and retire actual raw input owners once");
}
void direct_core_routes_and_guards(){
    Fixture f;require(f.core->wine_detected_045010()&&f.trace.module_calls==1&&f.trace.procedure_calls==1&&!f.trace.wine_calls,"Wine route resolves exact SDK names without calling the returned function");f.trace.module_present=false;require(!f.core->wine_detected_045010()&&f.trace.module_calls==2&&f.trace.procedure_calls==1,"Missing module skips procedure lookup");
    require(f.core->current_time_041b60_null_true()=="2020-01-01_00-00-00"&&f.trace.pattern=="%Y-%m-%d_%H-%M-%S"&&f.trace.time==1&&f.trace.gmtime==1&&f.trace.formatted==1,"Filename UTC uses output-null mode1 through controlled clock/tm/formatter");
    f.core->log_literal_006880(std::string_view("L\0tail",6));f.core->log_string_006860(std::string_view("S\0T",3));require(f.buffer->str()==std::string("LS\0T",4)&&!f.buffer->flushes,"Literal C-string log and counted raw ticket log add no newline or flush");const std::string bytes("ticket\0raw",10);require(f.core->write_ticket_03c1d0(u"MEM\\ticket.txt",bytes)&&f.memory->outputs.at(u"MEM\\ticket.txt")==bytes,"Ticket output uses actual counted binary file helper");
    require(f.core->filename_00e120(u"MEM\\name.bin")==u"name.bin"&&f.core->parent_path_019820(u"MEM\\name.bin")==u"MEM"&&f.core->join_016b00(u"MEM",u"child")==u"MEM\\child"&&f.core->filesystem_widen_006d70("ascii")==u"ascii"&&f.core->path_utf8_01e3f0(u"ascii")=="ascii","Direct core paths/conversions use existing source helpers on owned inputs");
    Request request;Output xml;rejects<std::logic_error>([&]{f.handlers->auth_code_02e3b0(request,xml.xml);},"Handlers reject dispatch before authentication root binding");f.bind();rejects<std::logic_error>([&]{f.bind();},"Authentication root binds exactly once");
    configuration::State foreign{};rejects<std::invalid_argument>([&]{le::Core wrong({foreign,f.runtime,f.execution},*f.online,f.options());},"Core rejects foreign Online view");auto invalid=f.options();invalid.stream.reset();rejects<std::invalid_argument>([&]{le::Core wrong(f.view(),*f.online,std::move(invalid));},"Missing mandatory stream rejects before work");rejects<std::invalid_argument>([&]{le::Handlers wrong(f.view(),*f.online,{},f.options());},"Explicit owned attempt/block roots are mandatory");
    bool denied=false;std::thread worker([&]{try{(void)f.core->wine_detected_045010();}catch(const std::logic_error&){denied=true;}});worker.join();require(denied&&f.trace.module_calls==2,"Foreign worker rejects before SDK lookup");rejects<le::NoResponseRequiredError>([&]{f.core->no_response_02bcd0_04d228();},"Core no-response is explicit selected host control marker");require(f.core->filename_00e120(u"MEM\\still-open")==u"still-open","Known no-response marker does not poison unrelated subsequent source use");{auto options=f.options();options.module=unexpected_marker;le::Core other(f.view(),*f.online,std::move(options));rejects<le::NoResponseRequiredError>([&]{(void)other.wine_detected_045010();},"Unrelated SDK same-marker throw propagates as unknown");rejects<std::logic_error>([&]{(void)other.filename_00e120(u"MEM\\denied");},"Unrelated SDK marker poisons Core rather than admitting later source work");}bool exited=false;try{f.core->exit_05cdc0(-7);}catch(const Exit& e){exited=e.code==-7;}require(exited,"Exit route uses only injected throwing boundary, never native exit");
}
// Source scenarios only until an explicit rebuild/test instruction. Real
// prepared Parent/NativeThreads, synthetic data/own EXE SDK slots throughout.
struct ChildEvent {
    HANDLE handle=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
    ChildEvent(){require(handle!=nullptr,"Licensing test event created");}
    ~ChildEvent(){if(handle)::CloseHandle(handle);}
    void signal(){require(::SetEvent(handle)!=0,"Licensing event signaled");}
    void wait(){require(::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0,"Licensing event observed within finite bound");}
};
BOOL WINAPI child_no_module(DWORD flags,LPCWSTR address,HMODULE* module){if(flags!=4||!address||!module)std::terminate();*module=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
struct ChildTask {
    std::function<void()> action;std::exception_ptr error;HANDLE handle=nullptr;std::uint32_t id=0;
    static unsigned __stdcall run(void* p){auto& task=*static_cast<ChildTask*>(p);try{task.action();}catch(...){task.error=std::current_exception();}return 67;}
    void launch(bootstrap_providers::NativeThreads& threads,std::function<void()> f){action=std::move(f);handle=reinterpret_cast<HANDLE>(threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&run),reinterpret_cast<std::uintptr_t>(this),0,&id,threads));require(handle&&id,"Licensing test uses same-Domain NativeThreads");}
    void join(){if(!handle||::WaitForSingleObject(handle,10000)!=WAIT_OBJECT_0){std::cerr<<"Retain active Licensing wait/token owners\n"<<std::flush;::ExitProcess(1);}DWORD code=0;require(::GetExitCodeThread(handle,&code)&&code==67,"Licensing worker exits through actual wrapper/FLS path");require(::CloseHandle(handle)!=0,"Joined Licensing handle closed");handle=nullptr;if(error)std::rethrow_exception(error);}
};
struct ChildLife final:socket_recovered::LifecycleEffects {
    socket_recovered::SocketService& resolve_service_0239c0(socket_recovered::ContextIdentity)override{throw std::logic_error("No Licensing fixture socket selected");}
    void close_and_cancel_026fb0(socket_recovered::SocketService&,socket_recovered::SocketObject&)override{throw std::logic_error("No Licensing fixture socket close");}
    void executor_cleanup(socket_recovered::SocketObject&,std::uintptr_t)override{throw std::logic_error("No Licensing fixture executor cleanup");}
};
struct ChildAdmission {
    Trace& trace;unsigned calls=0;std::function<void(unsigned)> hook{};
    static void check(void* p){auto& c=*static_cast<ChildAdmission*>(p);++c.calls;Trace::checkpoint(&c.trace);if(c.hook)c.hook(c.calls);}
    content_handlers_recovered::CallCheckpoint callback(){return {this,check};}
};
void prepared_licensing_child(std::string_view mode={}){
    namespace sm=socket_recovered;namespace wh=worker_handoff;
    auto identity=std::make_shared<std::array<std::byte,8>>();
    auto domain=thread_domain::DomainStorage::create({0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity});
    require(domain->initialize(),"Own Licensing Domain initializes real heap/FLS/PTD");auto api=thread_native::native_api();api.get_module=child_no_module;auto threads=domain->make_threads(api);
    auto prepared=wh::PreparedMemory::prepare(domain),other=wh::PreparedMemory::prepare(domain);
    std::shared_ptr<sm::SessionMemoryDomain> parent,foreign;std::optional<wh::PreparedMemory::JobMemory> offer;std::optional<sm::SessionMemoryDomain::JobPermit> retained;
    std::optional<application_io::CallContext> retained_view_context;
    std::vector<std::shared_ptr<le::Core::CallView>> closed_core_views;
    std::vector<std::shared_ptr<le::Handlers::CallView>> closed_handler_views;
    std::unique_ptr<Fixture> fixture;std::vector<std::unique_ptr<le::Core>> probes;
    std::vector<std::unique_ptr<le::Handlers>> handler_probes;std::unique_ptr<le::Core> recursive_core,bad_sdk;std::unique_ptr<le::Handlers> recursive_handler,diagnostic,bad_node;
    ChildEvent ready,returned,exit;ChildTask worker,child;
    try{
        worker.launch(*threads,[&]{
            auto borrow=prepared->acquire();parent=std::make_shared<sm::SessionMemoryDomain>(std::move(borrow),std::make_shared<ChildLife>());
            auto wrong=other->acquire();foreign=std::make_shared<sm::SessionMemoryDomain>(std::move(wrong),std::make_shared<ChildLife>());
            {auto operation=parent->enter();fixture=std::make_unique<Fixture>();auto& f=*fixture;f.bind();f.adopt();
                f.memory->inputs[u"MEM\\token.dlf"]=encrypted("OWN");f.memory->inputs[u"MEM\\license.dlf"]=encrypted("PRIMARY<&");f.memory->inputs[u"MEM\\license_cached.dlf"]=encrypted("CACHED");
                recursive_core=std::make_unique<le::Core>(f.view(),*f.online,f.options());auto sdk=f.options();sdk.module=unexpected_marker;bad_sdk=std::make_unique<le::Core>(f.view(),*f.online,std::move(sdk));
                for(unsigned i=0;i<3;++i)probes.push_back(std::make_unique<le::Core>(f.view(),*f.online,f.options()));
                for(unsigned i=0;i<5;++i)handler_probes.push_back(std::make_unique<le::Handlers>(f.view(),*f.online,f.attempts,f.options()));
                recursive_handler=std::make_unique<le::Handlers>(f.view(),*f.online,f.attempts,f.options());diagnostic=std::make_unique<le::Handlers>(f.view(),*f.online,f.attempts,f.options());bad_node=std::make_unique<le::Handlers>(f.view(),*f.online,f.attempts,f.options());bad_node->bind_authentication_code(f.authentication);offer.emplace(parent->prepare_job());}
            ready.signal();returned.wait();
            {auto operation=parent->enter();require(fixture->core->filename_00e120(u"MEM\\resumed")==u"resumed","Original Core route resumes after child authority ends");Request request;request.values.emplace("ClientId","resumed");Output output;require(fixture->handlers->auth_code_02e3b0(request,output.xml),"Original Handler route resumes after child control return");
                rejects<std::logic_error>([&]{fixture->handlers->bind_authentication_code(fixture->authentication);},"Closing child views cannot reset the owner's irreversible authentication binding");
                diagnostic->bind_authentication_code(fixture->authentication); // first binding remains legal after unbound diagnostic views close
                bad_node.reset();diagnostic.reset();recursive_handler.reset();handler_probes.clear();probes.clear();bad_sdk.reset();recursive_core.reset();fixture.reset();}
            require(parent->close()&&foreign->close(),"Licensing child retains no Parent operation/lease");parent.reset();foreign.reset();
        });
        ready.wait();child.launch(*threads,[&]{
            offer->acquire();auto& f=*fixture;ChildAdmission view_admission{f.trace};
            {
                auto operation=parent->enter(*offer);auto permit=operation.job_permit();retained.emplace(permit);ChildAdmission admission{f.trace};
                if(!mode.empty()){
                    if(mode=="read")f.trace.read_hook=[&]{f.trace.deny=true;};else f.trace.open_hook=[&]{f.trace.deny=true;};
                    bool caught=false;try{f.core->with_job(*parent,permit,admission.callback(),[&]{(void)f.core->load_token_02afa0(u"MEM\\token.dlf");});}catch(const std::logic_error&){caught=true;}
                    if(caught&&f.memory->reads.size()==1&&f.trace.initial==(mode=="read"?1U:0U)&&f.trace.reads==(mode=="read"?1U:0U)&&!f.trace.closes&&!f.trace.input_destroyed)std::_Exit(mode=="read"?184:183);
                    std::_Exit(239);
                }
                rejects<std::logic_error>([&]{(void)f.core->filename_00e120(u"unreached");},"Core default rejects child without explicit scope");Request request;Output denied;
                rejects<std::logic_error>([&]{f.handlers->auth_code_02e3b0(request,denied.xml);},"Handlers default reject child without scope");
                rejects<std::invalid_argument>([&]{f.handlers->with_job(*foreign,permit,admission.callback(),[]{throw std::runtime_error("wrong-root action ran");});},"Licensing rejects different Parent before any callback");
                ChildTask outsider;outsider.launch(*threads,[&]{rejects<std::logic_error>([&]{f.core->with_job(*parent,permit,admission.callback(),[]{throw std::runtime_error("wrong-actor action ran");});},"Licensing wrong actor rejects before held Parent lock");});outsider.join();
                require(!admission.calls&&!f.trace.auths&&!f.trace.reads&&!f.trace.module_calls,"Denied Licensing proof reaches no probe/SDK/file");
                ::SetLastError(0x47f2);const auto result=f.core->with_job(*parent,permit,admission.callback(),[&]{
                    require(f.core->wine_detected_045010()&&!f.trace.wine_calls,"Child Wine helper resolves only injected symbol");
                    require(f.core->current_time_041b60_null_true()=="2020-01-01_00-00-00","Child Core uses existing mode1 UTC");
                    require(f.core->load_token_02afa0(u"MEM\\token.dlf")=="OWN"&&f.core->file_hash_matches_042d20(f.config.denuvo_dll,f.config.dll_hash),"Child Core keeps actual SHA1/AES over synthetic memory only");
                    const std::string bytes("ticket\0raw",10);require(f.core->write_ticket_03c1d0(u"MEM\\child.txt",bytes)&&f.memory->outputs.at(u"MEM\\child.txt")==bytes,"Child ticket writer preserves counted NUL bytes");return f.core->join_016b00(u"MEM",u"child");
                });
                require(result==u"MEM\\child"&&::GetLastError()==0x47f2&&f.trace.closes==f.trace.input_destroyed,"Owned Path/LastError return after raw input retirement");
                rejects<le::NoResponseRequiredError>([&]{f.core->with_job(*parent,permit,admission.callback(),[&]{f.core->no_response_02bcd0_04d228();});},"Core source control passes after successful scope retirement");
                f.core->with_job(*parent,permit,admission.callback(),[&]{f.core->log_string_006860("child");});
                request.values.emplace("ClientId","child");Output auth_output;
                require(f.handlers->with_job(*parent,permit,admission.callback(),[&]{return f.handlers->auth_code_02e3b0(request,auth_output.xml);})&&f.trace.auths==1&&f.trace.client=="child"&&auth_output.bytes().find("online&lt;&amp;")!=std::string::npos,"Child dual scope reaches actual bound auth/owned Online slot30");
                Output primary;require(f.handlers->with_job(*parent,permit,admission.callback(),[&]{return f.handlers->request_license_0320c0(request,primary.xml);})&&f.config.denuvo_token=="PRIMARY<&"&&f.attempts->attempted_files==1,"Child request uses actual token route/live attempts");
                Output invalidated;f.handlers->with_job(*parent,permit,admission.callback(),[&]{f.handlers->invalidate_license_030c50(request,invalidated.xml);});Output cached;
                require(f.handlers->with_job(*parent,permit,admission.callback(),[&]{return f.handlers->request_license_0320c0(request,cached.xml);})&&f.config.denuvo_token=="CACHED"&&f.attempts->attempted_files==2,"Child invalidation preserves attempts and cached AES branch");
                Output again;f.handlers->with_job(*parent,permit,admission.callback(),[&]{f.handlers->invalidate_license_030c50(request,again.xml);});Output exhausted;
                require(!f.handlers->with_job(*parent,permit,admission.callback(),[&]{return f.handlers->request_license_0320c0(request,exhausted.xml);})&&f.attempts->blocked,"Missing ticket with exhausted attempts sets recovered block flag");
                rejects<le::NoResponseRequiredError>([&]{f.handlers->with_job(*parent,permit,admission.callback(),[&]{f.handlers->request_license_0320c0(request,exhausted.xml);});},"Blocked request completes both scopes before known no-response");
                Output after;require(f.handlers->with_job(*parent,permit,admission.callback(),[&]{return f.handlers->auth_code_02e3b0(request,after.xml);}),"Known control leaves dual scope healthy for later authentication");
                struct Diagnostic final:messages_recovered::UnknownMessageInput {std::string_view view()const override{return "own child diagnostic";}} input;
                {
                    struct UiScope {
                        b12_test::Ui* prior=b12_test::ui;
                        explicit UiScope(b12_test::Ui& value){b12_test::ui=&value;}
                        ~UiScope(){b12_test::ui=prior;}
                    } ui_scope(f.ui);
                    diagnostic->with_job(*parent,permit,admission.callback(),[&]{diagnostic->unknown_message_07bd70(input);});
                    application_io::CallContext context(*parent,permit,view_admission.callback());
                    auto unbound=diagnostic->make_call_view(context);Request no_auth;Output no_auth_output;
                    rejects<std::logic_error>([&]{unbound->auth_code_02e3b0(no_auth,no_auth_output.xml);},"Unbound diagnostic view cannot invent an authentication fallback or semantic executor");
                    unbound->unknown_message_07bd70(input);unbound->close();closed_handler_views.push_back(std::move(unbound));
                }
                require(f.ui.messages.back().text==L"own child diagnostic","Unbound child diagnostic uses same owned UI/stream without auth");
                {
                    application_io::CallContext context(*parent,permit,view_admission.callback());retained_view_context.emplace(context);
                    auto core=f.core->make_call_view(context);auto handlers=f.handlers->make_call_view(context);
                    const auto before=view_admission.calls;
                    ChildTask foreign_actor;foreign_actor.launch(*threads,[&]{
                        rejects<std::logic_error>([&]{(void)f.core->make_call_view(context);},"Core call-view factory rejects another actor's context before callbacks");
                        rejects<std::logic_error>([&]{(void)f.handlers->make_call_view(context);},"Handler call-view factory rejects another actor's context before callbacks");
                    });foreign_actor.join();
                    require(view_admission.calls==before,"Foreign call-view denial reaches neither live borrowed checkpoint");
                    rejects<le::NoResponseRequiredError>([&]{core->no_response_02bcd0_04d228();},"Actual Core call-view known control leaves its registered execution healthy");
                    require(core->filename_00e120(u"MEM\\call-view")==u"call-view"&&core->current_time_041b60_null_true()=="2020-01-01_00-00-00","Core call-view keeps actual path and mode1 UTC bodies after source control");
                    const std::string counted("view\0ticket",11);require(core->write_ticket_03c1d0(u"MEM\\view-ticket.txt",counted)&&f.memory->outputs.at(u"MEM\\view-ticket.txt")==counted,"Core call-view normally retires its actual counted-NUL ticket output");
                    Request view_request;view_request.values.emplace("ClientId","call-view");Output suppressed;
                    require(f.attempts->blocked!=0,"New Handler view observes the same already-blocked actual AttemptState");
                    rejects<le::NoResponseRequiredError>([&]{handlers->request_license_0320c0(view_request,suppressed.xml);},"Nested Handler/Core call-view source control does not poison either execution");
                    const auto auths=f.trace.auths;Output copied;
                    require(handlers->auth_code_02e3b0(view_request,copied.xml)&&f.trace.auths==auths+1&&f.trace.client=="call-view"&&f.trace.valid&&copied.bytes().find("online&lt;&amp;")!=std::string::npos,"Bound Handler view uses its nested Core owned Online result and real own-EXE slot30 reservation");
                    handlers->close();core->close();
                    rejects<std::logic_error>([&]{(void)core->filename_00e120(u"MEM\\closed");},"Closed Core view cannot regain source execution");
                    Output closed;rejects<std::logic_error>([&]{handlers->auth_code_02e3b0(view_request,closed.xml);},"Closed Handler view cannot regain source execution");
                    closed_core_views.push_back(std::move(core));closed_handler_views.push_back(std::move(handlers));
                }
                rejects<std::logic_error>([&]{f.core->log_string_006860("unreached");},"Core child authority ends after synchronous return");
                rejects<std::logic_error>([&]{f.handlers->auth_code_02e3b0(request,after.xml);},"Handler child authority ends after synchronous return");
                for(unsigned i=0;i<probes.size();++i){ChildAdmission probe{f.trace};probe.hook=[i](unsigned n){if(n==i+1)throw le::NoResponseRequiredError{};};bool ran=false;
                    rejects<std::logic_error>([&]{probes[i]->with_job(*parent,permit,probe.callback(),[&]{ran=true;});},"Core admission/post/final same-marker denial is never source control");require(ran==(i!=0)&&probe.calls==i+1,"Core failed probe is not retried");
                    rejects<std::logic_error>([&]{probes[i]->with_job(*parent,permit,admission.callback(),[]{});},"Core probe denial remains poisoned");}
                const std::array<unsigned,5> selected{1,2,4,5,6};
                for(unsigned i=0;i<handler_probes.size();++i){ChildAdmission probe{f.trace};const auto stop=selected[i];probe.hook=[stop](unsigned n){if(n==stop)throw le::NoResponseRequiredError{};};bool ran=false;
                    rejects<std::logic_error>([&]{handler_probes[i]->with_job(*parent,permit,probe.callback(),[&]{ran=true;});},"Dual-scope initial/Core-final/Handler-final marker denial is not valid control");require(ran==(stop>2)&&probe.calls==stop,"Dual-scope probe failure has no retry");
                    rejects<std::logic_error>([&]{handler_probes[i]->with_job(*parent,permit,admission.callback(),[]{});},"Handler scope denial remains poisoned");}
                rejects<std::logic_error>([&]{bad_sdk->with_job(*parent,permit,admission.callback(),[&]{(void)bad_sdk->wine_detected_045010();});},"Unrelated SDK same-marker poisons child Core");
                struct BadNode final:messages_recovered::XmlNode {
                    std::string_view name()const override{return "own";}const messages_recovered::XmlNode* first_element()const override{return nullptr;}const messages_recovered::XmlNode* next_element()const override{return nullptr;}
                    std::optional<std::string_view> attribute(std::string_view)const override{throw le::NoResponseRequiredError{};}
                } node;
                Output opaque;rejects<std::logic_error>([&]{bad_node->with_job(*parent,permit,admission.callback(),[&]{bad_node->auth_code_02e3b0(node,opaque.xml);});},"Raw XML same-marker is unknown and poisons Handler rather than source control");
                bool ran=false;unsigned swallowed=0;ChildAdmission recursive{f.trace};recursive.hook=[&](unsigned){try{recursive_core->with_job(*parent,permit,recursive.callback(),[&]{ran=true;});}catch(const std::logic_error&){++swallowed;}};
                rejects<std::logic_error>([&]{recursive_core->with_job(*parent,permit,recursive.callback(),[&]{ran=true;});},"Swallowed Core scope reentry remains poisoned");require(swallowed==1&&!ran,"Recursive Core admission reaches neither action");
                ran=false;swallowed=0;ChildAdmission recursive_h{f.trace};recursive_h.hook=[&](unsigned){try{recursive_handler->with_job(*parent,permit,recursive_h.callback(),[&]{ran=true;});}catch(const std::logic_error&){++swallowed;}};
                rejects<std::logic_error>([&]{recursive_handler->with_job(*parent,permit,recursive_h.callback(),[&]{ran=true;});},"Swallowed Handler scope reentry remains poisoned");require(swallowed==1&&!ran,"Recursive Handler admission reaches neither action");
            }
            ChildAdmission expired{f.trace};rejects<std::logic_error>([&]{f.handlers->with_job(*parent,*retained,expired.callback(),[]{throw std::runtime_error("expired action ran");});},"Licensing copied permit expires with exact Operation");require(!expired.calls,"Expired Licensing permit reaches no checkpoint");
            const auto callbacks=view_admission.calls;
            rejects<std::logic_error>([&]{(void)f.core->make_call_view(*retained_view_context);},"Expired same-actor context cannot mint a Core call view");
            rejects<std::logic_error>([&]{(void)f.handlers->make_call_view(*retained_view_context);},"Expired same-actor context cannot mint a Handler call view");
            require(view_admission.calls==callbacks,"Expired factory admission reaches no retained borrowed checkpoint");
            {auto operation=parent->enter(*offer);auto fresh=operation.job_permit();ChildAdmission admission{f.trace};rejects<std::logic_error>([&]{f.core->with_job(*parent,*retained,admission.callback(),[]{throw std::runtime_error("stale action ran");});},"Old Core seal remains invalid in next Operation");require(f.core->with_job(*parent,fresh,admission.callback(),[&]{return f.core->filename_00e120(u"MEM\\fresh");})==u"fresh","Fresh permit admits existing Core source");}
            require(offer->retire(),"Licensing descriptors end before logical child return");returned.signal();exit.wait();
        });
        worker.join();require(!prepared->close(0)&&other->close(10000),"Licensing scope completion does not imply physical child exit");exit.signal();child.join();require(prepared->close(10000),"Physical NativeThreads/FLS exit releases Licensing backing");closed_handler_views.clear();closed_core_views.clear();retained_view_context.reset();retained.reset();offer.reset();other.reset();prepared.reset();threads.reset();require(domain->close(),"Licensing Domain closes after leases and threads");domain.reset();
    }catch(const std::exception& e){std::cerr<<"Prepared Licensing source scenario: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
[[noreturn]]void input_denial_child(bool after_read){
    auto* f=new Fixture;f->memory->inputs[u"MEM\\token.dlf"]=encrypted("OWN");f->core->bind_content_checkpoint({&f->trace,Trace::checkpoint});if(after_read)f->trace.read_hook=[f]{f->trace.deny=true;};else f->trace.open_hook=[f]{f->trace.deny=true;};bool caught=false;
    try{(void)f->core->load_token_02afa0(u"MEM\\token.dlf");}catch(const std::logic_error&){caught=true;}
    const auto& t=f->trace;if(caught&&f->memory->reads.size()==1&&t.initial==(after_read?1U:0U)&&t.reads==(after_read?1U:0U)&&!t.closes&&!t.input_destroyed)std::_Exit(after_read?182:181);std::_Exit(239);
}
}
int main(int argc,char** argv){try{if(argc==2&&std::string_view(argv[1])=="--input-open-denial")input_denial_child(false);if(argc==2&&std::string_view(argv[1])=="--input-read-denial")input_denial_child(true);if(argc==2&&std::string_view(argv[1])=="--job-input-open-denial")prepared_licensing_child("open");if(argc==2&&std::string_view(argv[1])=="--job-input-read-denial")prepared_licensing_child("read");unknown_diagnostic();actual_auth_license_and_cache();direct_core_routes_and_guards();prepared_licensing_child();require(_spawnl(_P_WAIT,argv[0],argv[0],"--input-open-denial",static_cast<const char*>(nullptr))==181,"Own child proves nonnull input open-return is adopted before denied postcheck");require(_spawnl(_P_WAIT,argv[0],argv[0],"--input-read-denial",static_cast<const char*>(nullptr))==182,"Own child proves read denial retains raw input without close/destructor");require(_spawnl(_P_WAIT,argv[0],argv[0],"--job-input-open-denial",static_cast<const char*>(nullptr))==183,"Prepared Licensing child source retains unknown open-return input");require(_spawnl(_P_WAIT,argv[0],argv[0],"--job-input-read-denial",static_cast<const char*>(nullptr))==184,"Prepared Licensing child source retains unknown read input");std::cout<<"PASS: "<<checks<<" licensing effects checks; prepared child source, synthetic auth/hash/AES and4retained-input children\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
