#include "checkpoint.hpp"
#include "../anadius-callback-policy-source/policy_test_fixture.hpp"
#include <bit>

namespace {
using namespace anadius;namespace n=native_recovered;namespace cp=callback_policy;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Fault {};
void boundary_contract(){
    rejects<std::invalid_argument>([]{original_checkpoint::Boundary b({});},"missing permission rejected");
    bool allowed=true;unsigned probes=0,lower=0;DWORD incoming=0;char output='!';
    original_checkpoint::Boundary b([&]{++probes;::SetLastError(0xdead);return allowed;});
    ::SetLastError(0x1111);require(b.invoke([&]{++lower;incoming=::GetLastError();output='X';::SetLastError(0x2222);return 7;})==7&&lower==1&&probes==2&&incoming==0x1111&&::GetLastError()==0x2222,"boundary exact result and incoming/outgoing LastError");
    allowed=false;rejects<std::logic_error>([&]{b.invoke([&]{++lower;return 0;});},"precheck rejects");
    require(lower==1&&output=='X',"precheck never calls lower or rolls back previous prefix");allowed=true;
    auto expected=std::make_exception_ptr(Fault{});bool same=false;
    try{b.invoke([&]()->int{::SetLastError(0x3333);std::rethrow_exception(expected);});}catch(...){same=std::current_exception()==expected;}
    require(same&&::GetLastError()==0x3333,"valid postcheck retains identical lower exception and error");
    rejects<std::logic_error>([&]{b.invoke([&]()->int{output='Y';allowed=false;::SetLastError(0x4444);std::rethrow_exception(expected);});},"invalid postcondition wins over lower exception");
    require(output=='Y'&&::GetLastError()==0x4444,"invalid postcondition keeps prefix and outgoing error");
    original_checkpoint::Boundary throws([&]()->bool{::SetLastError(0xbeef);std::rethrow_exception(expected);});
    ::SetLastError(0x5555);same=false;try{throws.verify();}catch(...){same=std::current_exception()==expected;}
    require(same&&::GetLastError()==0x5555,"permission throw preserves exception and bookkeeping error");
    auto token=std::make_shared<int>(1);std::weak_ptr<int> weak=token;
    auto owner=std::make_unique<original_checkpoint::Boundary>([token]{return *token==1;});token.reset();
    require(!weak.expired(),"boundary retains permission owner");owner.reset();require(weak.expired(),"last boundary owner releases permission capture");
}
struct Args {
    HKEY root=reinterpret_cast<HKEY>(0x1234),opened=nullptr;char a[20]="own-A",command[20]="cmd-A",output[20]{};wchar_t w[20]=L"own-W",command_w[20]=L"cmd-W",output_w[20]{};
    DWORD reserved=0,type=0,size=8;BYTE data[8]{};SECURITY_ATTRIBUTES process{},thread{};char environment[4]{};
    STARTUPINFOA startup{};STARTUPINFOW startup_w{};PROCESS_INFORMATION info{};sockaddr_in address{};
    unsigned sdk=0;std::array<unsigned,10> routes{};
};
Args* args=nullptr;
int record(unsigned route,bool exact){require(exact,"exact typed SDK arguments");require(::GetLastError()==0x1111,"SDK sees incoming error after both prechecks");++args->sdk;++args->routes[route];::SetLastError(0x2222);return std::bit_cast<std::int32_t>(std::uint32_t{0x87654321});}
LSTATUS WINAPI open_a(HKEY h,LPCSTR key,DWORD options,REGSAM access,PHKEY out){const auto result=record(1,h==args->root&&key==args->a&&options==0x11&&access==0x22&&out==&args->opened);*out=reinterpret_cast<HKEY>(0xbeef);return result;}
LSTATUS WINAPI open_w(HKEY h,LPCWSTR key,DWORD options,REGSAM access,PHKEY out){const auto result=record(2,h==args->root&&key==args->w&&options==0x11&&access==0x22&&out==&args->opened);*out=reinterpret_cast<HKEY>(0xbeef);return result;}
LSTATUS WINAPI close(HKEY h){return record(3,h==args->root);}
LSTATUS WINAPI query_a(HKEY h,LPCSTR name,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD size){const auto result=record(4,h==args->root&&name==args->a&&reserved==&args->reserved&&type==&args->type&&data==args->data&&size==&args->size);*type=7;*size=1;*data='Q';return result;}
LSTATUS WINAPI query_w(HKEY h,LPCWSTR name,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD size){const auto result=record(5,h==args->root&&name==args->w&&reserved==&args->reserved&&type==&args->type&&data==args->data&&size==&args->size);*type=7;*size=1;*data='W';return result;}
BOOL WINAPI process_a(LPCSTR app,LPSTR command,LPSECURITY_ATTRIBUTES pa,LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID env,LPCSTR cwd,LPSTARTUPINFOA startup,LPPROCESS_INFORMATION info){const auto result=record(6,app==args->a&&command==args->command&&pa==&args->process&&ta==&args->thread&&inherit==TRUE&&flags==0x135&&env==args->environment&&cwd==args->a&&startup==&args->startup&&info==&args->info);*command='X';info->dwProcessId=42;return result;}
BOOL WINAPI process_w(LPCWSTR app,LPWSTR command,LPSECURITY_ATTRIBUTES pa,LPSECURITY_ATTRIBUTES ta,BOOL inherit,DWORD flags,LPVOID env,LPCWSTR cwd,LPSTARTUPINFOW startup,LPPROCESS_INFORMATION info){const auto result=record(7,app==args->w&&command==args->command_w&&pa==&args->process&&ta==&args->thread&&inherit==TRUE&&flags==0x135&&env==args->environment&&cwd==args->w&&startup==&args->startup_w&&info==&args->info);*command=L'X';info->dwProcessId=43;return result;}
HRESULT WINAPI folder_a(HWND hwnd,int id,HANDLE token,DWORD flags,LPSTR out){const auto result=record(8,hwnd==reinterpret_cast<HWND>(0x11)&&id==7&&token==reinterpret_cast<HANDLE>(0x22)&&flags==0x33&&out==args->output);out[0]='F';out[1]=0;return result;}
HRESULT WINAPI folder_w(HWND hwnd,int id,HANDLE token,DWORD flags,LPWSTR out){const auto result=record(9,hwnd==reinterpret_cast<HWND>(0x11)&&id==7&&token==reinterpret_cast<HANDLE>(0x22)&&flags==0x33&&out==args->output_w);out[0]=L'W';out[1]=0;return result;}
int WSAAPI connect(SOCKET socket,const sockaddr* address,int size){return record(0,socket==0x9876&&address==reinterpret_cast<const sockaddr*>(&args->address)&&size==16);}
template<class T>std::uintptr_t ptr(T value){return reinterpret_cast<std::uintptr_t>(value);}
void ten_typed_routes(){
    b16_test::Fixture f;Args state;args=&state;unsigned probes=0;
    f.driver.runtime.originals={ptr(&connect),ptr(&open_a),ptr(&open_w),ptr(&close),ptr(&query_a),ptr(&query_w),ptr(&process_a),ptr(&process_w),ptr(&folder_a),ptr(&folder_w)};
    auto resources=f.resources();resources.callback_scope=[&]{++probes;::SetLastError(0xdead);return true;};cp::Policy policy(f.view(),std::move(resources));
    auto run=[&](auto&& fn){probes=0;::SetLastError(0x1111);require(static_cast<std::uint32_t>(fn())==0x87654321&&probes==3&&::GetLastError()==0x2222,"all typed SDK routes preserve32-bit result and three boundary probes");};
    const auto root=ptr(state.root);namespace r=registry_recovered;
    run([&]{return policy.original_open_a({root,state.a,0x11,0x22,r::HandleCell::from_raw(&state.opened)});});require(state.opened==reinterpret_cast<HKEY>(0xbeef),"openA output prefix retained");
    run([&]{return policy.original_open_w({root,reinterpret_cast<const char16_t*>(state.w),0x11,0x22,r::HandleCell::from_raw(&state.opened)});});
    run([&]{return policy.original_close(root);});
    run([&]{return policy.original_query_a({root,state.a,&state.reserved,r::DwordCell::from_raw(&state.type),state.data,r::DwordCell::from_raw(&state.size)});});require(state.type==7&&state.size==1&&state.data[0]=='Q',"queryA exact prefix retained");
    run([&]{return policy.original_query_w({root,reinterpret_cast<const char16_t*>(state.w),&state.reserved,r::DwordCell::from_raw(&state.type),state.data,r::DwordCell::from_raw(&state.size)});});require(state.data[0]=='W',"queryW exact prefix retained");
    run([&]{return policy.original_create_process_a({state.a,state.command,ptr(&state.process),ptr(&state.thread),TRUE,0x135,ptr(state.environment),state.a,ptr(&state.startup),ptr(&state.info)});});require(state.command[0]=='X'&&state.info.dwProcessId==42,"processA in-out buffer prefix retained");
    run([&]{return policy.original_create_process_w({reinterpret_cast<const char16_t*>(state.w),reinterpret_cast<char16_t*>(state.command_w),ptr(&state.process),ptr(&state.thread),TRUE,0x135,ptr(state.environment),reinterpret_cast<const char16_t*>(state.w),ptr(&state.startup_w),ptr(&state.info)});});require(state.command_w[0]==L'X'&&state.info.dwProcessId==43,"processW in-out buffer prefix retained");
    run([&]{return policy.original_folder_a({0x11,7,0x22,0x33,state.output});});require(std::string(state.output)=="F","folderA original output retained");
    run([&]{return policy.original_folder_w({0x11,7,0x22,0x33,reinterpret_cast<char16_t*>(state.output_w)});});require(std::wstring(state.output_w)==L"W","folderW original output retained");
    run([&]{return policy.original_connect_0a3838(0x9876,{reinterpret_cast<std::byte*>(&state.address),sizeof(state.address)},16);});
    require(state.sdk==10&&std::all_of(state.routes.begin(),state.routes.end(),[](auto value){return value==1;}),"exactly one SDK invocation per each of ten routes");args=nullptr;
}
class Resolver final:public n::OriginalProvider {
public:
    const n::OriginalSlots& slots;unsigned loads=0;std::function<void()> action;
    explicit Resolver(const n::OriginalSlots& value):slots(value){}
    const n::OriginalSlots* slots_identity()const noexcept override{return &slots;}
    std::uintptr_t load(n::Callback c)override{++loads;if(action)action();return slots.at(host_init_recovered::slot(c));}
    void setup_begin()override{}void setup_complete()override{}void stop()noexcept override{}
};
void resolution_guards(){
    b16_test::Fixture f;auto resolver=std::make_shared<Resolver>(f.driver.runtime.originals);bool allowed=false;unsigned probes=0;
    auto resources=f.resources();resources.callback_scope=[&]{++probes;return allowed;};resources.originals=[resolver](const auto&){return resolver;};cp::Policy policy(f.view(),std::move(resources));
    rejects<std::logic_error>([&]{policy.original_close(0x11);},"first check guards resolution");
    require(resolver->loads==0&&f.driver.probe->closes==0&&probes==1,"denied first check touches neither provider nor SDK");
    allowed=true;probes=0;resolver->action=[&]{allowed=false;};
    rejects<std::logic_error>([&]{policy.original_close(0x11);},"postresolver preSDK check rejects changed permission");
    require(resolver->loads==1&&f.driver.probe->closes==0&&probes==2,"normal-return resolver mutation never reaches SDK");
    auto expected=std::make_exception_ptr(Fault{});allowed=true;probes=0;
    resolver->action=[&]{::SetLastError(0x6666);std::rethrow_exception(expected);};bool same=false;
    try{policy.original_close(0x11);}catch(...){same=std::current_exception()==expected;}
    require(same&&probes==2&&f.driver.probe->closes==0&&::GetLastError()==0x6666,"valid resolver throw retains exact exception/error without SDK");
    resolver->action=[&]{allowed=false;::SetLastError(0x7777);std::rethrow_exception(expected);};allowed=true;probes=0;
    rejects<std::logic_error>([&]{policy.original_close(0x11);},"invalid resolver postcondition wins over resolver exception");
    require(probes==2&&f.driver.probe->closes==0&&::GetLastError()==0x7777,"failed resolver prefix retained with no SDK call");
    auto legacy=f.resources();cp::Policy direct(f.view(),std::move(legacy));require(direct.original_close(0x11)==0&&f.driver.probe->closes==1,"direct no-scope legacy original route unchanged");
}
}
int main(){try{boundary_contract();ten_typed_routes();resolution_guards();std::cout<<"PASS: "<<checks<<" original SDK checkpoint core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected own fault\n";return 2;}}
