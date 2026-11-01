#include "effects.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include <bit>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <process.h>
#include <sstream>
#include <thread>

// Only own strings, memory files, injected directory/CopyFile/dialog SDKs and
// callable slots in this test EXE. No original object, module or network runs.
namespace {
using namespace anadius;namespace ce=content_effects;namespace ch=content_handlers_recovered;namespace of=online_factory;
using Path=configuration::Path;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Buffer final:std::stringbuf {
    std::function<void()> hook;unsigned flushes=0;
    std::streamsize xsputn(const char* p,std::streamsize n)override{const auto out=std::stringbuf::xsputn(p,n);if(hook)hook();return out;}
    int sync()override{++flushes;return std::stringbuf::sync();}
};
struct Trace {
    of::Online* online=nullptr;std::array<std::uintptr_t,7> table{};std::array<std::byte,17> object{};
    unsigned loads=0,avatars=0,destroyed=0,code_drops=0,atoi_calls=0,copies=0,checks=0,time=0,gmtime=0,formatted=0,initial=0,output_destroyed=0,output_closed=0;
    bool valid=true,copy_success=false,deny=false,deny_after_open=false;BOOL copy_fail_if_exists=TRUE;
    Path avatar_path,copy_source,copy_destination;std::string owner,atoi_text,pattern,dialog_output;std::int32_t size=0;
    std::function<void()> time_hook,initial_hook,directory_hook;Buffer* buffer=nullptr;
    online_recovered::Object identity(){return reinterpret_cast<online_recovered::Object>(object.data()+1);}
    static void checkpoint(void* raw){auto& t=*static_cast<Trace*>(raw);++t.checks;if(t.deny)throw std::logic_error("selected content admission denied");}
};
Trace* current=nullptr;
struct Pin {Trace& trace;~Pin(){++trace.code_drops;}};
void destroy(void* object){current->valid=current->valid&&object==reinterpret_cast<void*>(current->identity());++current->destroyed;}
std::int32_t status(void*){throw std::logic_error("unselected status");}
const char* getter(void*){throw std::logic_error("unselected getter");}
std::uint8_t avatar(void* object,const char16_t* path,const char* owner,std::int32_t size){auto& t=*current;++t.avatars;const auto snap=t.online->snapshot().binding;t.valid=t.valid&&object==reinterpret_cast<void*>(t.identity())&&snap.active&&snap.object==t.identity()&&!snap.object_unknown;t.avatar_path=path;t.owner=owner;t.size=size;return 0x80;}
const char* auth(void*,const char*){throw std::logic_error("unselected auth");}
online_recovered::Object factory(const char16_t*){auto& t=*current;t.table={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&getter),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};const auto table=reinterpret_cast<std::uintptr_t>(t.table.data());std::memcpy(t.object.data()+1,&table,sizeof table);return t.identity();}
FARPROC WINAPI lookup(HMODULE module,LPCSTR name){require(reinterpret_cast<std::uintptr_t>(module)==0x80&&std::string_view(name)=="create_online","Only own factory export is resolved");return std::bit_cast<FARPROC>(&factory);}
int atoi_call(const char* text){++current->atoi_calls;current->atoi_text=text;return std::atoi(text);}
BOOL WINAPI copy_call(LPCWSTR source,LPCWSTR destination,BOOL fail_if_exists){auto& t=*current;++t.copies;t.copy_source.assign(source,source+std::wcslen(source));t.copy_destination.assign(destination,destination+std::wcslen(destination));t.copy_fail_if_exists=fail_if_exists;return t.copy_success?TRUE:FALSE;}
BOOL WINAPI directory_call(LPCWSTR path,LPSECURITY_ATTRIBUTES security){const auto result=b8_test::fs_create(path,security);if(current->directory_hook)current->directory_hook();return result;}
int WINAPI message_call(HWND window,LPCWSTR text,LPCWSTR title,UINT flags){current->dialog_output=current->buffer->str();return b12_test::message(window,text,title,flags);}
struct Encoding final:encoding_recovered::Calls {
    encoding_recovered::NativeCalls actual;unsigned calls=0;
    std::uint32_t locale_codepage_0629ec()override{++calls;return 65001;}bool are_file_apis_ansi()override{++calls;return true;}
    int multi_byte_to_wide(std::uint32_t c,std::uint32_t f,const char* s,int n,char16_t* p,int z)override{++calls;return actual.multi_byte_to_wide(c,f,s,n,p,z);}
    int wide_to_multi_byte(std::uint32_t c,std::uint32_t f,const char16_t* s,int n,char* p,int z,bool* u)override{++calls;return actual.wide_to_multi_byte(c,f,s,n,p,z,u);}
    std::uint32_t last_error()override{return actual.last_error();}
};
struct Clock final:achievement_runtime::Clock {
    Trace& t;explicit Clock(Trace& t):t(t){}
    std::int64_t time_05eeb8()override{++t.time;if(t.time_hook)t.time_hook();return 1577836800;}
    std::optional<achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t value)override{++t.gmtime;require(value==1577836800,"UTC passes actual supplied time to gmtime");return achievement_runtime::BrokenDown{0,0,0,1,0,120,3,0,0};}
};
struct Formatter final:achievement_runtime::Formatter {
    Trace& t;explicit Formatter(Trace& t):t(t){}
    std::string put_time_03d610(const achievement_runtime::BrokenDown& value,std::string_view pattern)override{++t.formatted;t.pattern=pattern;require(value.year==120&&value.day==1,"Formatter receives copied controlled UTC fields");return "2020-01-01T00:00:00";}
};
struct Files final:file_recovered::Factory {
    Trace& t;b8_test::MemoryFiles inner;Files(Trace& t,std::shared_ptr<b8_test::MemoryFilesState> memory):t(t),inner(std::move(memory)){}
    struct Output final:file_recovered::Output {Trace& t;std::unique_ptr<file_recovered::Output> inner;Output(Trace& t,std::unique_ptr<file_recovered::Output> p):t(t),inner(std::move(p)){}
        ~Output()override{++t.output_destroyed;}
        std::uint32_t initial_state()const override{++t.initial;if(t.initial_hook)t.initial_hook();return inner->initial_state();}void write_03be80(std::string_view bytes)override{inner->write_03be80(bytes);}void close_03ae70()override{++t.output_closed;inner->close_03ae70();}};
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& p,std::uint32_t m,std::uint32_t s)override{return inner.open_input_039f70(p,m,s);}
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t m,std::uint32_t s)override{auto result=std::make_unique<Output>(t,inner.open_output_03a1a0(p,m,s));if(t.deny_after_open)t.deny=true;return result;}
};
struct Fixture {
    configuration::State config{};bootstrap_backend::RuntimeSeed runtime{};b2_test::Events events;b4_test::Execution platform{events};execution_recovered::Execution execution;
    achievement_storage::Paths paths{u"MEM\\achievements.xml",u"MEM\\nested\\achievements.ini"};Trace trace;b8_test::FsSdk sdk;b12_test::Ui ui;
    std::shared_ptr<b8_test::MemoryFilesState> memory=std::make_shared<b8_test::MemoryFilesState>();
    std::shared_ptr<Buffer> buffer=std::make_shared<Buffer>();std::shared_ptr<std::ostream> stream=std::make_shared<std::ostream>(buffer.get());
    std::shared_ptr<library_effects::Dialog> dialog;std::shared_ptr<Encoding> encoding=std::make_shared<Encoding>();std::shared_ptr<config_storage::Filesystem> filesystem;
    std::unique_ptr<of::Online> online;std::unique_ptr<ce::Content> content;
    bootstrap_backend::CallbackStateView view(){return {config,runtime,execution};}
    ce::Options options(){ce::Options o;o.lifetime=buffer;o.stream=stream;o.dialog=dialog;o.filesystem=filesystem;o.files=std::make_shared<Files>(trace,memory);o.clock=std::make_shared<Clock>(trace);o.formatter=std::make_shared<Formatter>(trace);o.encoding=encoding;o.atoi=atoi_call;o.copy_file=copy_call;return o;}
    Fixture():execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform){
        current=&trace;trace.buffer=buffer.get();b8_test::fs_sdk=&sdk;b12_test::ui=&ui;auto fs=b8_test::fs_api();fs.create_directory=directory_call;filesystem=std::make_shared<config_storage::Filesystem>(fs);dialog=std::make_shared<library_effects::Dialog>(library_effects::DialogApi{b12_test::convert,message_call});execution_recovered::initialize_mutex_04a698(runtime.execution_mutex,2);runtime.frequency_cache=10000000;
        config.user_id="owner";config.user_directory=u"MEM";config.avatar=u"MEM\\avatar.png";config.default_avatar=u"MEM\\default.png";config.last_avatar_path=u"MEM\\last.png";
        of::Options o;o.stream=stream;o.dialog=dialog;auto os=std::make_unique<b12_test::Os>();os->load_hook=[&](const wchar_t*){++trace.loads;return std::uintptr_t{0x80};};o.os=std::move(os);o.get_proc=lookup;o.code_lease=[&](online_recovered::Module){return std::shared_ptr<void>(new Pin{trace});};o.foreign_quiescent=[]{return true;};o.files=std::make_unique<b8_test::MemoryFiles>(memory);
        online.reset(static_cast<of::Online*>(of::make_online(view(),o).release()));trace.online=online.get();content=std::make_unique<ce::Content>(view(),paths,*online,options());
    }
    void adopt(){runtime.online_module=online->load_copy_0450d0(u"OWN-EXE-SYNTHETIC");const auto create=online->get_proc_address(runtime.online_module,"create_online");runtime.online_object=online->call_create_online(create,u"own-cookie");runtime.online=1;}
    void checkpoint(){content->bind_content_checkpoint({&trace,Trace::checkpoint});}
    ~Fixture(){if(content){content->clear_content_checkpoint();content.reset();}if(online){if(runtime.online_object){online->destroy_virtual_00(runtime.online_object);runtime.online_object=0;}if(runtime.online_module){online->free_library(runtime.online_module);runtime.online_module=0;}if(!online->close_after_cleanup())std::terminate();online.reset();}b8_test::fs_sdk=nullptr;b12_test::ui=nullptr;current=nullptr;}
};
struct UnusedMutex final:ch::Mutex {int lock_04a6ac()override{throw std::logic_error("QueryImage must not lock achievements");}std::int32_t& count_4c()override{throw std::logic_error("unused count");}void unlock_04a6b4()override{throw std::logic_error("unused unlock");}[[noreturn]]void thread_error_04a648(int)override{throw std::logic_error("unused terminal");}};
struct UnusedEvents final:ch::EventQueue {ch::Mutex& mutex_02c620()override{throw std::logic_error("unused events");}bool initialized_locked()const override{throw std::logic_error("unused initialized");}void initialize_locked(std::string)override{throw std::logic_error("unused initialize");}void push_locked_034660(std::string)override{throw std::logic_error("unused push");}};
void query_image_and_routes(){
    Fixture f;require(!f.trace.loads&&!f.trace.atoi_calls&&!f.trace.time&&!f.encoding->calls&&f.memory->reads.empty()&&f.memory->writes.empty()&&f.sdk.creates.empty()&&f.ui.messages.empty()&&f.buffer->str().empty(),"Content/Online construction owns inputs passively without provider IO");f.adopt();
    UnusedMutex mutex;UnusedEvents events;std::int32_t saved=-3;auto refs=achievement_runtime::bind_state(f.config,f.paths,f.runtime.online,saved,mutex,events);
    auto document=f.content->printer_0019b0();ch::RequestTree request{{{"Width","-416"},{"Height","208"},{"ImageId","avatar:owner"}}, {}};
    require(ch::query_image_0314f0(request,document->xml(),refs,*f.content),"Actual QueryImage selects concrete AvatarRequest through Content");
    require(f.trace.avatars==1&&f.trace.avatar_path==u"MEM\\_AVATAR_owner_-416"&&f.trace.owner=="owner"&&f.trace.size==-2&&f.trace.valid,"Actual Content→Online→own slot28 receives cache path, owner suffix and signed atoi/208 size");
    require(f.trace.copies==1&&f.trace.copy_fail_if_exists==FALSE&&f.trace.copy_source==f.trace.avatar_path&&f.trace.copy_destination==f.config.last_avatar_path&&saved==-3&&f.buffer->str().empty(),"CopyFileW FALSE return and failIfExists FALSE are preserved without saved-size update/log");
    require(document->c_string_without_nul().find("_AVATAR_owner_-416")!=std::string::npos,"Actual memory XML response publishes received cache path even when optional copy fails");
    f.trace.copy_success=true;request.attributes[0].second="416";auto second=f.content->printer_0019b0();require(ch::query_image_0314f0(request,second->xml(),refs,*f.content)&&saved==2&&f.buffer->str()=="saved size: 2\n","Successful copy updates live saved-size cell then writes exact prefix/value/newline");
    const std::string diagnostic("left\0right",10);f.content->unknown_entitlement_field(diagnostic);require(f.buffer->str()==std::string("saved size: 2\n")+diagnostic+"\n"&&f.trace.dialog_output==f.buffer->str()&&f.ui.messages.size()==1&&f.ui.messages[0].title==L"UNKNOWN MESSAGE"&&f.ui.messages[0].flags==0x10,"Counted diagnostic plus newline/flush precedes actual selected dialog flags10");
    require(f.ui.conversions.size()==4&&f.ui.conversions[0].input=="UNKNOWN MESSAGE"&&f.ui.conversions[2].input==diagnostic,"Dialog converts title then full counted NUL diagnostic before its final C-string boundary");
    require(f.content->atoi_05ee08(std::string_view("416\0tail",8))==416&&f.trace.atoi_text=="416","Atoi receives an owned native C-string prefix");
    require(f.content->filesystem_widen_006d70("utf8-\xc3\xa9")==u"utf8-\u00e9"&&f.content->path_to_utf8_031ca0(u"wide-\u00e9")=="wide-\xc3\xa9"&&f.content->path_append_016b50(u"MEM",u"child")==u"MEM\\child","Selected filesystem conversion, explicit UTF8 and recovered path append run on owned text");
}
void persistence_and_utc(){
    Fixture f;std::uint64_t epoch=9;require(f.content->utc_time_041b60(epoch)=="2020-01-01T00:00:00"&&epoch==1577836800&&f.trace.time==1&&f.trace.gmtime==1&&f.trace.formatted==1&&f.trace.pattern=="%Y-%m-%dT%H:%M:%S","Actual UTC helper sequences controlled clock, gmtime, calendar and formatter");
    auto xml=f.content->printer_0019b0();xml->xml().open_004080("Own",false);xml->xml().attribute_005560("text","a<&");xml->xml().close_virtual_48(false);const auto bytes=xml->c_string_without_nul();require(bytes.find("a&lt;&amp;")!=std::string::npos,"Concrete document printer performs actual XML escaping");f.content->write_xml_03c1d0(f.paths.xml_path,bytes);require(f.memory->outputs.at(f.paths.xml_path)==bytes,"Actual file helper writes printer bytes through owned memory files");
    const std::string counted("x\0y",3);f.content->write_xml_03c1d0(u"MEM\\counted.xml",counted);require(f.memory->outputs.at(u"MEM\\counted.xml")==counted,"Direct XML persistence preserves explicit byte count including NUL");
    f.config.achievements.emplace("one",configuration::Achievement{5,1,3,"2020-01-01T00:00:00","name","steam-1",17});f.paths.ini_path=u"MEM\\updated\\achievements.ini";f.content->save_steam_01f9a0(f.config.achievements);
    require(f.memory->outputs.at(f.paths.ini_path)=="[steam-1]\r\nAchieved=1\r\nCurProgress=3\r\nMaxProgress=5\r\nUnlockTime=17\r\n\r\n"&&!f.sdk.creates.empty(),"Actual INI serializer/directories use live shared Paths and State map");
}
void ownership_identity_and_affinity(){
    Fixture f;auto lifetime=std::make_shared<int>(9);std::weak_ptr<int> weak=lifetime;auto options=f.options();options.lifetime=lifetime;auto owned=std::make_unique<ce::Content>(f.view(),f.paths,*f.online,std::move(options));lifetime.reset();require(!weak.expired(),"Content owns explicit backing lifetime");owned.reset();require(weak.expired(),"Unused concrete provider releases only its owned backing normally");
    configuration::State foreign{};rejects<std::invalid_argument>([&]{ce::Content wrong({foreign,f.runtime,f.execution},f.paths,*f.online,f.options());},"Content refuses Online bound to a different actual configuration view");auto invalid=f.options();invalid.copy_file=nullptr;rejects<std::invalid_argument>([&]{ce::Content wrong(f.view(),f.paths,*f.online,std::move(invalid));},"Incomplete CopyFile SDK dependency rejects passively");
    bool rejected=false;std::thread other([&]{try{(void)f.content->atoi_05ee08("5");}catch(const std::logic_error&){rejected=true;}});other.join();require(rejected&&!f.trace.atoi_calls&&f.content->atoi_05ee08("7")==7,"Foreign worker rejects before CRT effects without poisoning original worker use");
    rejects<online_recovered::OutOfEvidenceInput>([&]{(void)f.content->online_avatar_virtual_28(u"MEM\\missing-args");},"Legacy path-only avatar call cannot invent recovered owner/size arguments");
}
void primitive_checkpoints(){
    {Fixture f;f.checkpoint();f.buffer->hook=[&]{f.trace.deny=true;};rejects<std::logic_error>([&]{f.content->unknown_entitlement_field("prefix");},"Stream primitive postcheck rejects swallowed admission loss");require(f.buffer->str()=="prefix"&&!f.buffer->flushes&&f.ui.messages.empty(),"Stream denial stops before newline/flush/dialog");}
    {Fixture f;f.checkpoint();f.trace.time_hook=[&]{f.trace.deny=true;};std::uint64_t epoch=99;rejects<std::logic_error>([&]{(void)f.content->utc_time_041b60(epoch);},"Clock primitive postcheck stops UTC sequence");require(f.trace.time==1&&!f.trace.gmtime&&!f.trace.formatted&&epoch==99,"Denied time callback cannot reach gmtime/format or assign epoch");}
    {Fixture f;f.checkpoint();f.trace.directory_hook=[&]{f.trace.deny=true;};rejects<std::logic_error>([&]{f.content->save_steam_01f9a0(f.config.achievements);},"Directory SDK postcheck stops source prefix traversal");require(f.sdk.creates.size()==1&&f.memory->writes.empty(),"First directory callback denial prevents next CreateDirectory prefix and file open");}
}
[[noreturn]]void file_initial_denial_child(){
    // Unknown file ownership is deliberately retained. Do not unwind the
    // fail-closed provider or claim a successful cleanup of this prefix.
    auto* f=new Fixture;f->checkpoint();f->trace.initial_hook=[f]{f->trace.deny=true;};bool caught=false;
    try{f->content->write_xml_03c1d0(u"MEM\\prefix.xml","payload");}catch(const std::logic_error&){caught=true;}
    const auto& t=f->trace;
    if(caught&&t.initial==1&&!t.output_destroyed&&!t.output_closed&&f->memory->events==std::vector<std::string>{"open"}&&f->memory->outputs.empty())std::_Exit(171);
    std::_Exit(239);
}
[[noreturn]]void file_open_denial_child(){
    // The nonnull lower result must already have a retained owner when its
    // post-return checkpoint fails, before any initial_state query is allowed.
    auto* f=new Fixture;f->checkpoint();f->trace.deny_after_open=true;bool caught=false;
    try{f->content->write_xml_03c1d0(u"MEM\\open-prefix.xml","payload");}catch(const std::logic_error&){caught=true;}
    const auto& t=f->trace;
    if(caught&&!t.initial&&!t.output_destroyed&&!t.output_closed&&f->memory->events==std::vector<std::string>{"open"}&&f->memory->outputs.empty())std::_Exit(172);
    std::_Exit(239);
}
}
int main(int argc,char** argv){try{if(argc==2&&std::string_view(argv[1])=="--file-initial-denial")file_initial_denial_child();if(argc==2&&std::string_view(argv[1])=="--file-open-denial")file_open_denial_child();query_image_and_routes();persistence_and_utc();ownership_identity_and_affinity();primitive_checkpoints();require(_spawnl(_P_WAIT,argv[0],argv[0],"--file-initial-denial",static_cast<const char*>(nullptr))==171,"Own child proves initial-state denial retains raw output without write/close/destructor");require(_spawnl(_P_WAIT,argv[0],argv[0],"--file-open-denial",static_cast<const char*>(nullptr))==172,"Own child proves open-return denial retains adopted raw output before initial_state/write/close/destructor");std::cout<<"PASS: "<<checks<<" content effects checks; own slot28/memory persistence and 2 retained-prefix children\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
