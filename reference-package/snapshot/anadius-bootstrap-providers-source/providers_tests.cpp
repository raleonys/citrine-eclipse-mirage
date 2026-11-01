#include "providers_test_fixture.hpp"
#include <climits>
#include <csignal>

namespace {
using namespace b5_test;
void configuration_routes(){
    PlatformState state;platform=&state;b2_test::Events events;
    auto policy=std::make_unique<Policies>(events);auto* raw=policy.get();auto encoding=std::make_unique<Encoding>();auto* converted=encoding.get();
    p::Configuration config(std::move(policy),std::move(encoding),std::make_unique<encoding_recovered::PortableErrors>(),config_api());
    require(config.parent_path_019820(u"C:\\a\\b.exe")==u"C:\\a","configuration uses recovered parent algorithm");
    require(config.join_016b00(u"C:\\a",u"\\b")==u"C:\\b","configuration uses recovered rooted path replacement");
    require(config.filename(u"C:\\a\\name_AVATAR_x.png")==u"name_AVATAR_x.png","configuration uses recovered filename algorithm");
    require(config.filesystem_widen_010540(std::string_view("a\0b",3))==configuration::Path(u"a\0b",3)&&converted->conversions==2,"existing encoding source preserves explicit length and embedded NUL");
    require(config.crt_is_space(255)==false&&state.space_operand==255,"space receives unsigned byte without narrowing");
    require(config.crt_is_space(' ')&&config.crt_tolower('A')=='a'&&state.lower_operand=='A',"selected compiler CRT predicates preserve result semantics");
    require(config.crt_tolower(EOF)==EOF,"EOF remains a valid compiler-CRT operand");
    rejects<std::domain_error>([&]{config.crt_tolower(-23);},"signed negative non-EOF domain rejected before C tolower UB");
    rejects<std::domain_error>([&]{config.crt_tolower(UCHAR_MAX+1);},"outside unsigned-char domain rejected");
    require(!config.set_environment_variable("B5_TEST","value")&&state.environment==std::vector<std::pair<std::string,std::string>>{{"B5_TEST","value"}},"SDK environment operands and FALSE result preserved");
    converted->fail=true;bool correct=false;try{config.filesystem_widen_010540("bad");}catch(const std::system_error& e){correct=e.code().value()==1113;}
    require(correct,"existing encoding error route preserves exact failure code");
    require(config.executable_path_03c6d0()==u"C:\\fixture\\host.exe"&&config.local_app_data_03c6f0()==u"LOCAL","unbound application path policies stay explicit");
    config.log_config_path(u"anadius.cfg");require(raw->logs==1,"remaining log policy stays owned and invoked");
    auto bad=config_api();bad.lower=nullptr;
    rejects<std::invalid_argument>([&]{p::Configuration c(std::make_unique<Policies>(events),std::make_unique<Encoding>(),std::make_unique<encoding_recovered::PortableErrors>(),bad);},"incomplete compiler CRT table rejected");
    rejects<std::invalid_argument>([&]{p::Configuration c({});},"absent remaining configuration policy rejected");
}
void console_source_routes(){
    PlatformState state;platform=&state;configuration::State config{};std::uint8_t lsx=0,show=1,no=1;
    const auto refs=[&]()->startup_recovered::StateReferences{return {config.language_from_arguments,config.registry_spoof,lsx,show,no,config.game_attributes};};
    p::CompilerConsole console(console_api(),streams());
    startup_recovered::setup_console_038760(refs(),console);require(!state.attaches&&state.opens.empty(),"no-console source branch has no OS or stream effects");
    no=0;show=0;startup_recovered::setup_console_038760(refs(),console);require(state.attaches==1&&!state.allocations&&state.opens.empty(),"failed attach without show-console returns before reopening");
    show=1;startup_recovered::setup_console_038760(refs(),console);
    require(state.allocations==1&&state.moves==1&&state.opens==std::vector<std::string>({"conin$:r","conout$:w","conout$:w"}),"failed allocation return does not gate remaining observed console sequence");
    require(state.original_streams==std::vector<FILE*>({streams()[0],streams()[1],streams()[2]}),"distinct input/output/error compiler streams reach CRT API");
    state.attach_ok=true;state.opens.clear();startup_recovered::setup_console_038760(refs(),console);
    require(state.allocations==1&&state.opens==std::vector<std::string>{"conout$:w"},"successful parent attach only reopens stderr");
    const auto a=console.stream_052238(0),b=console.stream_052238(1);
    require(a==console.stream_052238(0)&&a!=b,"opaque stream slot tokens remain stable and distinct");
    rejects<std::out_of_range>([&]{console.stream_052238(3);},"unbound compiler stream index rejected");
    rejects<std::invalid_argument>([&]{console.reopen_05f024(a,"x","r",b);},"unobserved mismatched output/source identity rejected");
    rejects<std::invalid_argument>([&]{console.reopen_05f024(1,"x","r",1);},"foreign token never becomes a FILE pointer");
    state.reopen_result=13;require(console.reopen_05f024(a,"x","r",a)==13,"CRT reopen failure returned unchanged");
    rejects<std::logic_error>([&]{console.reopen_05f024(a,"x","r",a);},"closed stream is not reused after failed compiler CRT reopen");
    auto bad=console_api();bad.reopen=nullptr;rejects<std::invalid_argument>([&]{p::CompilerConsole invalid(bad,streams());},"missing CRT reopen provider rejected");
    const auto native=p::native_console_api();require(native.attach==::AttachConsole&&native.reopen==::freopen_s,"native adapter table binds real imports without opening console");
}
unsigned __stdcall entry(void* value){return static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(value));}
struct Captured {void* security=nullptr;SIZE_T stack=0;LPTHREAD_START_ROUTINE start=nullptr;LPVOID argument=nullptr;DWORD flags=0;bool fail=false;};
Captured* captured=nullptr;
HANDLE WINAPI create(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,LPVOID arg,DWORD flags,LPDWORD id){
    require(::GetLastError()==55,"launch allocation bookkeeping preserves incoming LastError");
    *captured={security,stack,start,arg,flags,captured->fail};::SetLastError(56);if(captured->fail)return nullptr;if(id)*id=456;return reinterpret_cast<HANDLE>(0x1001);
}
void thread_launch_contract(){
    auto lower=std::make_unique<ThreadLower>();auto* raw=lower.get();auto api=thread_native::native_api();api.create=create;Captured call;captured=&call;
    p::NativeThreads provider(std::move(lower),api);auto* context=provider.calloc_context_066848();context->entry=reinterpret_cast<std::uintptr_t>(&entry);context->argument=73;
    std::uint32_t id=0;::SetLastError(55);const auto handle=provider.create_thread(reinterpret_cast<void*>(0x99),0x100000001ULL,context,4,&id);
    require(handle==0x1001&&id==456&&::GetLastError()==56,"SDK handle/id/error preserved by launch wrapper");
    require(call.security==reinterpret_cast<void*>(0x99)&&call.stack==0x100000001ULL&&call.flags==4,"full-width stack size and create operands preserved");
    require(call.argument!=context&&call.start&&reinterpret_cast<std::uintptr_t>(call.start)!=context->entry,"SDK receives owned envelope plus rebuilt CRT wrapper, not application callback");
    HANDLE worker=::CreateThread(nullptr,0,call.start,call.argument,0,nullptr);require(worker!=nullptr,"own synthetic wrapper worker created");
    require(::WaitForSingleObject(worker,10000)==WAIT_OBJECT_0,"own wrapper worker joined");DWORD code=0;require(::GetExitCodeThread(worker,&code)&&code==73,"recovered thread entry reaches supplied rebuilt callback and source end_thread");::CloseHandle(worker);
    require(raw->invocations==1&&raw->contexts.empty(),"envelope and synthetic Context ownership do not escape successful worker completion");
    context=provider.calloc_context_066848();call.fail=true;id=999;::SetLastError(55);
    require(provider.create_thread(nullptr,0,context,0,&id)==0&&id==999&&::GetLastError()==56,"failed SDK create leaves output ID untouched and preserves failure LastError");provider.free_0668c0(context);
    require(raw->contexts.empty()&&raw->creations==0,"outer concrete creation bypasses lower create_thread without bypassing source thread body");
}
int terminal(){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::set_terminate([]{std::_Exit(85);});
    auto lower=std::make_unique<ThreadLower>();lower->throw_entry=true;auto api=thread_native::native_api();p::NativeThreads provider(std::move(lower),api);
    auto* context=provider.calloc_context_066848();context->entry=reinterpret_cast<std::uintptr_t>(&entry);context->argument=1;std::uint32_t id;
    const auto h=provider.create_thread(nullptr,0,context,0,&id);::WaitForSingleObject(reinterpret_cast<HANDLE>(h),10000);std::_Exit(84);
}
}
int main(int argc,char** argv){if(argc==2)return terminal();try{
    configuration_routes();console_source_routes();thread_launch_contract();
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--worker-escape",static_cast<const char*>(nullptr))==85,"own worker exception contained at Win32 ABI boundary");
    std::cout<<"PASS: "<<b2_test::checks+b3_tracking_test::checks<<" startup-provider core checks; recovered config/path/encoding, compiler-console stream tokens and per-launch CRT thread start; only own worker and child executed\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
