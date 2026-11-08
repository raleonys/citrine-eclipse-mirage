// SPDX-License-Identifier: GPL-3.0-only

#include "../../support/fixture_ascii.hpp"
#include <ws2tcpip.h>
#include "preloader_l/platform/hook_native_callbacks.hpp"
#include "preloader_l/platform/callback_lock.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <barrier>
#include <memory>
#include <cstring>
#include <iostream>
#include <thread>
#include <string>
#include <cstdlib>
#include <exception>
#include <new>
#include <condition_variable>
#include <chrono>
#include <functional>
using namespace preloader_l::native;
using preloader_l::host_init::callback_count;
namespace {
int checks;
void require(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
struct Life{LifecycleFault fault;};
Callback expected_failure=Callback::connect_00fd00;
bool boundary_mode{};
struct Faults final:Errors {
    Binding* lease_owner=nullptr;
    [[noreturn]]void lifecycle(LifecycleFault f)override{throw Life{f};}
    [[noreturn]]void callback_exception(Callback c,std::exception_ptr error)noexcept override{
        if(!boundary_mode||c!=expected_failure||!error)std::_Exit(99);
        if(lease_owner){
            if(active_calls()!=1)std::_Exit(90);
            bool busy=false;
            try{
                std::thread closer([&]{
                    try{lease_owner->close();}
                    catch(const Life& failure){busy=failure.fault==LifecycleFault::Busy;}
                    catch(...){busy=false;}
                });
                closer.join();
            }catch(...){std::_Exit(91);}
            if(!busy||!is_bound()||active_calls()!=1)std::_Exit(92);
        }
        std::_Exit(86);
    }
} faults;
std::u16string read_utf16(const void* p){
    if(!p)throw std::invalid_argument("synthetic null UTF16");std::u16string s;
    for(std::size_t i=0;;++i){char16_t c;std::memcpy(&c,static_cast<const std::byte*>(p)+i*2,2);if(!c)return s;s+=c;}
}
struct Wide final:WideOperations {
    int compare_utf16(const void* a,const void* b)override{
        auto x=read_utf16(a),y=read_utf16(b);for(auto& c:x)if(c>=u'A'&&c<=u'Z')c+=32;for(auto& c:y)if(c>=u'A'&&c<=u'Z')c+=32;return x.compare(y);
    }
    int copy_utf16(void* d,std::size_t n,const void* s)override{
        const auto x=read_utf16(s);if(!d||x.size()+1>n)return 34;std::memcpy(d,x.c_str(),(x.size()+1)*2);return 0;
    }
};
struct Seen {
    const void* name{};void* reserved{};void* type{};void* data{};void* size{};void* result{};HKEY key{};DWORD options{},access{};
    std::array<std::uintptr_t,10> process{};std::array<std::uintptr_t,5> folder{};
    SOCKET socket{};const sockaddr* address{};int length{};unsigned short connect_port{};DWORD connect_incoming{};
    std::atomic<int> open_a{},open_w{},query_a{},query_w{},close{},process_a{},process_w{},folder_a{},folder_w{},connect{};
    std::mutex mutex;std::condition_variable cv;bool block{},entered{},release{};bool throw_process{};
    std::function<void(SOCKET)> connect_wait;
    std::array<std::uintptr_t,11> crypt{};
    std::wstring crypt_object;
    DWORD crypt_incoming{};BOOL crypt_result=7;int crypt_calls{};
} seen;
BOOL WINAPI original_crypt(DWORD type,const void* object,DWORD content,DWORD format,DWORD flags,
    DWORD* encoding,DWORD* content_type,DWORD* format_type,HCERTSTORE* store,HCRYPTMSG* message,const void** context){
    ++seen.crypt_calls;seen.crypt_incoming=GetLastError();
    seen.crypt={type,reinterpret_cast<std::uintptr_t>(object),content,format,flags,
        reinterpret_cast<std::uintptr_t>(encoding),reinterpret_cast<std::uintptr_t>(content_type),
        reinterpret_cast<std::uintptr_t>(format_type),reinterpret_cast<std::uintptr_t>(store),
        reinterpret_cast<std::uintptr_t>(message),reinterpret_cast<std::uintptr_t>(context)};
    seen.crypt_object=type==1&&object?static_cast<const wchar_t*>(object):L"";
    {std::unique_lock lock(seen.mutex);if(seen.block){seen.entered=true;seen.cv.notify_all();seen.cv.wait(lock,[]{return seen.release;});}}
    if(encoding)*encoding=0x111;if(content_type)*content_type=0x222;if(format_type)*format_type=0x333;
    if(store)*store=reinterpret_cast<HCERTSTORE>(0x444);if(message)*message=reinterpret_cast<HCRYPTMSG>(0x555);
    if(context)*context=reinterpret_cast<const void*>(0x666);
    SetLastError(0x1234abcd);return seen.crypt_result;
}
struct CryptHost final:preloader_l::platform::CryptQueryHost {
    preloader_l::platform::WindowsCryptQueryHost native;
    bool readable=true;
    preloader_l::hooks::PathProbe inspect_path(const void* p)noexcept override{
        const auto result=readable?native.inspect_path(p):preloader_l::hooks::PathProbe{};SetLastError(991);return result;
    }
};
LSTATUS WINAPI original_open_a(HKEY k,LPCSTR s,DWORD o,REGSAM a,PHKEY p){++seen.open_a;seen.key=k;seen.name=s;seen.options=o;seen.access=a;seen.result=p;if(p)*p=reinterpret_cast<HKEY>(0x4567);SetLastError(321);return -5;}
LSTATUS WINAPI original_open_w(HKEY k,LPCWSTR s,DWORD o,REGSAM a,PHKEY p){++seen.open_w;seen.key=k;seen.name=s;seen.options=o;seen.access=a;seen.result=p;if(p)*p=reinterpret_cast<HKEY>(0x4568);SetLastError(322);return 0;}
template<class Char>LSTATUS query(HKEY k,const Char* s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){seen.key=k;seen.name=s;seen.reserved=r;seen.type=t;seen.data=d;seen.size=n;if(n)*n=1;if(t)*t=0;if(d)*d=0x7f;SetLastError(323);return -23;}
LSTATUS WINAPI original_query_a(HKEY k,LPCSTR s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){++seen.query_a;return query(k,s,r,t,d,n);}
LSTATUS WINAPI original_query_w(HKEY k,LPCWSTR s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){++seen.query_w;return query(k,s,r,t,d,n);}
LSTATUS WINAPI original_close(HKEY k){++seen.close;seen.key=k;SetLastError(324);return 0;}
LSTATUS WINAPI replacement_close(HKEY k){seen.key=k;SetLastError(0x4a31);return -77;}
template<class Char,class Start>BOOL process(const Char* a,Char* b,LPSECURITY_ATTRIBUTES p,LPSECURITY_ATTRIBUTES t,BOOL h,DWORD f,LPVOID e,const Char* c,Start* s,LPPROCESS_INFORMATION i){
    seen.process={reinterpret_cast<std::uintptr_t>(a),reinterpret_cast<std::uintptr_t>(b),reinterpret_cast<std::uintptr_t>(p),reinterpret_cast<std::uintptr_t>(t),static_cast<std::uintptr_t>(static_cast<unsigned>(h)),f,reinterpret_cast<std::uintptr_t>(e),reinterpret_cast<std::uintptr_t>(c),reinterpret_cast<std::uintptr_t>(s),reinterpret_cast<std::uintptr_t>(i)};
    if(seen.throw_process)throw std::runtime_error("synthetic original exception");
    {std::unique_lock lock(seen.mutex);if(seen.block){seen.entered=true;seen.cv.notify_all();seen.cv.wait(lock,[]{return seen.release;});}}
    if(i)i->dwProcessId=0xfedcba98;SetLastError(325);return 7;
}
BOOL WINAPI original_process_a(LPCSTR a,LPSTR b,LPSECURITY_ATTRIBUTES p,LPSECURITY_ATTRIBUTES t,BOOL h,DWORD f,LPVOID e,LPCSTR c,LPSTARTUPINFOA s,LPPROCESS_INFORMATION i){++seen.process_a;return process(a,b,p,t,h,f,e,c,s,i);}
BOOL WINAPI original_process_w(LPCWSTR a,LPWSTR b,LPSECURITY_ATTRIBUTES p,LPSECURITY_ATTRIBUTES t,BOOL h,DWORD f,LPVOID e,LPCWSTR c,LPSTARTUPINFOW s,LPPROCESS_INFORMATION i){++seen.process_w;return process(a,b,p,t,h,f,e,c,s,i);}
template<class Char>HRESULT folder(HWND w,int id,HANDLE token,DWORD flags,Char* p){seen.folder={reinterpret_cast<std::uintptr_t>(w),static_cast<std::uintptr_t>(static_cast<unsigned>(id)),reinterpret_cast<std::uintptr_t>(token),flags,reinterpret_cast<std::uintptr_t>(p)};if(p)p[0]=Char{'X'};SetLastError(326);return std::bit_cast<HRESULT>(0x87654321u);}
HRESULT WINAPI original_folder_a(HWND w,int id,HANDLE t,DWORD f,LPSTR p){++seen.folder_a;return folder(w,id,t,f,p);}
HRESULT WINAPI original_folder_w(HWND w,int id,HANDLE t,DWORD f,LPWSTR p){++seen.folder_w;return folder(w,id,t,f,p);}
int WSAAPI original_connect(SOCKET s,const sockaddr* p,int n){
    const auto incoming=GetLastError();
    if(seen.connect_wait)seen.connect_wait(s);
    {std::lock_guard lock(seen.mutex);
     ++seen.connect;seen.socket=s;seen.address=p;seen.length=n;seen.connect_incoming=incoming;
     if(p&&n>=4){
         std::array<std::byte,4> prefix{};SIZE_T copied=0;
         const bool readable=ReadProcessMemory(GetCurrentProcess(),p,prefix.data(),prefix.size(),&copied)&&copied==prefix.size();
         SetLastError(incoming);
         if(readable)seen.connect_port=static_cast<unsigned short>((std::to_integer<unsigned>(prefix[2])<<8)|std::to_integer<unsigned>(prefix[3]));
     }}
    SetLastError(327);return -10035;
}
struct Effects final:OriginalEffects {
    int waits{},allocs{},frees{},crypt_observations{};std::vector<std::string> events;
    std::function<void()> registry_wait;
    std::function<void()> network_wait;
    void crypt_observed(preloader_l::hooks::Selection) noexcept override{++crypt_observations;SetLastError(993);}
    Effects(const std::array<std::uintptr_t,callback_count>& s,Wide& w,CryptHost& h):OriginalEffects(s,w,h){}
    std::string filesystem_narrow(std::u16string_view s)override{return preloader_l::test_support::fixture_ascii(s);}
    int crt_stricmp(const char* a,const char* b)override{std::string x(a),y(b);for(auto& c:x)if(c>='A'&&c<='Z')c+=32;for(auto& c:y)if(c>='A'&&c<='Z')c+=32;return x.compare(y);}
    int crt_strcpy_s(char* d,std::size_t n,const char* s)override{const auto len=std::strlen(s)+1;if(!d||len>n)return 34;std::memcpy(d,s,len);return 0;}
    preloader_l::registry::Handle allocate_4_04b0bc()override{++allocs;return 0x9876;}
    void free_04b0f8(preloader_l::registry::Handle p)override{require(p==0x9876,"fabricated handle identity freed");++frees;}
    void wait_036880(std::int32_t n)override{
        require(n==-1,"registry startup count");++waits;
        const auto incoming=GetLastError();
        {preloader_l::callback_lock::Pause pause;if(registry_wait)registry_wait();}
        SetLastError(incoming);
    }
    void wait_steps_036880(std::int32_t n)override{require(n==-1,"network startup count");++waits;if(network_wait)network_wait();}
    void log_literal_006880(std::uint64_t)override{events.push_back("reg-log");}
    void log_text_007990(std::string_view)override{}
    void finish_log_0088e0()override{}
    void log_literal_006880(std::string_view)override{events.push_back("connect-log");}
    void log_port_00ad70(std::uint16_t)override{}
    void log_finish_0088e0()override{}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{throw std::runtime_error("unexpected UI");}
    [[noreturn]]void exit_05cdc0(std::uint32_t)override{throw std::runtime_error("unexpected exit");}
};
struct Fixture {
    preloader_l::host_init::Values values{{"ClientKey",u"Client\\Key"},{"ClientKey2",u"Client\\Key2"},{"ClientValue",u"ClientPath"},{"ClientPath",u"C:\\game.exe"},{"Documents",u"D:\\docs"}};
    preloader_l::registry::Handles clients,languages,fabricated;std::uint8_t language=0;
    std::int32_t v4=16,v6=28;std::uint16_t port=6099;std::uint32_t attempts=0;
    std::array<std::uintptr_t,callback_count> originals{};Wide wide;CryptHost crypt_host;Effects effects;Context context;Binding binding;
    explicit Fixture(Errors& policy=faults):originals{reinterpret_cast<std::uintptr_t>(&original_connect),reinterpret_cast<std::uintptr_t>(&original_open_a),reinterpret_cast<std::uintptr_t>(&original_open_w),reinterpret_cast<std::uintptr_t>(&original_close),reinterpret_cast<std::uintptr_t>(&original_query_a),reinterpret_cast<std::uintptr_t>(&original_query_w),reinterpret_cast<std::uintptr_t>(&original_process_a),reinterpret_cast<std::uintptr_t>(&original_process_w),reinterpret_cast<std::uintptr_t>(&original_folder_a),reinterpret_cast<std::uintptr_t>(&original_folder_w),reinterpret_cast<std::uintptr_t>(&original_crypt)},
      effects(originals,wide,crypt_host),context{{values,clients,languages,fabricated,language},{v4,v6,port,attempts},effects,effects,effects,effects},binding(context,policy){}
};
void test_network_wait_allows_peer_callbacks(bool startup_wait){
    Fixture f;
    std::mutex mutex;std::condition_variable cv;
    bool entered=false,release=false,peer_completed=false;
    auto block=[&]{std::unique_lock lock(mutex);entered=true;cv.notify_all();cv.wait(lock,[&]{return release;});};
    if(startup_wait)f.effects.network_wait=block;
    else seen.connect_wait=[&](SOCKET socket){if(socket==111)block();};
    sockaddr_in first{};first.sin_family=AF_INET;first.sin_addr.s_addr=htonl(INADDR_LOOPBACK);first.sin_port=htons(startup_wait?3216:443);
    sockaddr_in peer{};peer.sin_family=AF_INET;peer.sin_port=htons(80);
    int first_result=0,peer_result=0;DWORD first_error=0,peer_error=0;
    std::thread blocked([&]{first_result=connect_entry(111,reinterpret_cast<sockaddr*>(&first),sizeof(first));first_error=GetLastError();});
    bool reached;
    {std::unique_lock lock(mutex);reached=cv.wait_for(lock,std::chrono::seconds(5),[&]{return entered;});}
    std::thread other([&]{peer_result=connect_entry(222,reinterpret_cast<sockaddr*>(&peer),sizeof(peer));peer_error=GetLastError();
        std::lock_guard lock(mutex);peer_completed=true;cv.notify_all();});
    bool progressed;
    {std::unique_lock lock(mutex);progressed=cv.wait_for(lock,std::chrono::seconds(5),[&]{return peer_completed;});}
    const auto outstanding=active_calls();
    bool rejected=false;
    try{f.binding.close();}catch(const Life& e){rejected=e.fault==LifecycleFault::Busy;}
    {std::lock_guard lock(mutex);release=true;cv.notify_all();}
    blocked.join();other.join();seen.connect_wait={};
    require(reached,"blocking network fixture reached the selected wait");
    require(progressed,"startup wait or original connect must not block unrelated network callbacks");
    require(outstanding==1&&rejected,"blocked connect retains lease and rejects premature unbinding");
    require(first_result==-10035&&peer_result==-10035&&first_error==327&&peer_error==327,"concurrent connect results and LastError retained");
    require(ntohs(first.sin_port)==(startup_wait?3216:443)&&ntohs(peer.sin_port)==80,"caller addresses preserved across concurrent callbacks");
    require(f.attempts==static_cast<unsigned>(startup_wait)&&active_calls()==0,"attempt count and callback drain retained");
}
void test_concurrent_attempts(){
    Fixture f;f.attempts=0xffffff00u;
    std::atomic<unsigned> fifteenth{};std::atomic<bool> errors{false};
    std::array<std::thread,8> workers;
    for(auto& worker:workers)worker=std::thread([&]{for(unsigned i=0;i<1000;++i){
        SetLastError(765);const auto attempt=f.effects.increment_attempts(f.attempts);
        if(attempt==15)++fifteenth;
        if(GetLastError()!=765)errors=true;
    }});
    for(auto& worker:workers)worker.join();
    require(f.attempts==0xffffff00u+8000u,"concurrent counter increments wrap without lost updates");
    require(fifteenth==1&&!errors,"each increment has its own result and preserves LastError");
}
void test_hook_reference_common_guard_scope(){
    Fixture f;
    {std::lock_guard lock(seen.mutex);seen.block=true;seen.entered=false;seen.release=false;}
    std::thread process([]{process_a_entry("other",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);});
    {std::unique_lock lock(seen.mutex);seen.cv.wait(lock,[]{return seen.entered;});seen.block=false;}
    std::atomic<bool> peer_completed{false};
    std::thread registry([&]{HKEY key{};(void)reg_open_a_entry(nullptr,"unmatched",0,0,&key);peer_completed=true;});
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(active_calls()<2&&!peer_completed&&std::chrono::steady_clock::now()<until)std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const bool serialized=!peer_completed&&active_calls()==2;
    // Both exclusions must still progress while a process callback holds the
    // common guard. Use passthrough arguments so no startup dependency applies.
    const auto connected=connect_entry(333,nullptr,-1);
    const auto crypt=crypt_query_entry(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
    {std::lock_guard lock(seen.mutex);seen.release=true;seen.cv.notify_all();}
    process.join();registry.join();seen.block=false;
    require(serialized,"Hook reference process callback shares guard with registry callbacks");
    require(connected==-10035&&crypt==7&&peer_completed&&active_calls()==0,"connect/crypt bypass common callback guard and all leases drain");
}
void test_registry_startup_wait_allows_peer_callbacks(){
    Fixture f;f.clients.insert(0x4568);
    std::mutex mutex;std::condition_variable cv;
    bool waiting=false,release=false,peer_completed=false;
    f.effects.registry_wait=[&]{
        std::unique_lock lock(mutex);waiting=true;cv.notify_all();
        cv.wait(lock,[&]{return release;});SetLastError(998);
    };
    LSTATUS queried=-1,opened=0;DWORD query_error=0;
    std::thread query_thread([&]{DWORD type=0,size=64;BYTE bytes[64]{};
        queried=reg_query_a_entry(reinterpret_cast<HKEY>(0x4568),"ClientPath",nullptr,&type,bytes,&size);
        query_error=GetLastError();
    });
    {std::unique_lock lock(mutex);cv.wait(lock,[&]{return waiting;});}
    std::thread startup_peer([&]{HKEY key{};opened=reg_open_a_entry(HKEY_CURRENT_USER,"unmatched",0,0,&key);
        std::lock_guard lock(mutex);peer_completed=true;cv.notify_all();
    });
    bool progressed;
    {std::unique_lock lock(mutex);progressed=cv.wait_for(lock,std::chrono::seconds(5),[&]{return peer_completed;});
        release=true;cv.notify_all();}
    query_thread.join();startup_peer.join();
    require(progressed,"startup registry wait blocked peer callback needed to finish startup");
    require(queried==0&&opened==-5&&query_error==323,"paused registry callback preserves original results and LastError");
    require(active_calls()==0,"paused registry callback keeps and then releases its binding lease");
}
void test_resolver_and_registry(){
    Fixture f;Resolver resolver;
    const std::array<std::uintptr_t,callback_count> expected{reinterpret_cast<std::uintptr_t>(&connect_entry),reinterpret_cast<std::uintptr_t>(&reg_open_a_entry),reinterpret_cast<std::uintptr_t>(&reg_open_w_entry),reinterpret_cast<std::uintptr_t>(&reg_close_entry),reinterpret_cast<std::uintptr_t>(&reg_query_a_entry),reinterpret_cast<std::uintptr_t>(&reg_query_w_entry),reinterpret_cast<std::uintptr_t>(&process_a_entry),reinterpret_cast<std::uintptr_t>(&process_w_entry),reinterpret_cast<std::uintptr_t>(&folder_a_entry),reinterpret_cast<std::uintptr_t>(&folder_w_entry),reinterpret_cast<std::uintptr_t>(&crypt_query_entry)};
    for(std::size_t i=0;i<callback_count;++i)require(resolver.rebuilt_address(static_cast<Callback>(i))==expected[i],"every callback resolver result is rebuilt SDK shim");
    HKEY out{};const char key[]="unmatched";decltype(&::RegOpenKeyExA) oa=&reg_open_a_entry;
    require(oa(HKEY_CURRENT_USER,key,0x1234,0xfedcba98,&out)==-5,"native LSTATUS bits retained");
    require(out==reinterpret_cast<HKEY>(0x4567)&&seen.name==key&&seen.result==&out&&seen.key==HKEY_CURRENT_USER&&seen.options==0x1234&&seen.access==0xfedcba98,"native handle pointer and all original open arguments preserve identity");
    require(GetLastError()==321,"last-error from original survives callback lease release");
    const wchar_t wkey[]=L"CLIENT\\KEY2";decltype(&::RegOpenKeyExW) ow=&reg_open_w_entry;
    require(ow(HKEY_LOCAL_MACHINE,wkey,7,8,&out)==0&&f.clients.contains(0x4568),"native wchar input matches bytewise policy after original call");
    require(seen.name==wkey&&seen.result==&out,"wide original receives caller wchar pointer unchanged");
    require(oa(HKEY_CURRENT_USER,"client\\key",1,2,&out)==0&&out==reinterpret_cast<HKEY>(0x9876)&&f.fabricated.contains(0x9876),"failed matched open immediately fabricates native HKEY result");
    DWORD reserved=77,type=9,size=64;std::array<BYTE,64> bytes{};const char name[]="ClientPath";
    decltype(&::RegQueryValueExA) qa=&reg_query_a_entry;
    require(qa(out,name,&reserved,&type,bytes.data(),&size)==0,"native query override succeeds using pre-original capacity");
    require(size==12&&type==1&&std::strcmp(reinterpret_cast<char*>(bytes.data()),"C:\\game.exe")==0,"DWORD cells update real native objects and byte buffer");
    require(seen.reserved==&reserved&&seen.type==&type&&seen.size==&size&&seen.data==bytes.data()&&seen.name==name,"all query pointer identities retained through original trampoline");
    require(GetLastError()==323&&f.effects.waits==1,"original error state and startup wait retained");
    size=64;type=9;std::array<wchar_t,32> wide{};const wchar_t wname[]=L"CLIENTPATH";decltype(&::RegQueryValueExW) qw=&reg_query_w_entry;
    require(qw(out,wname,&reserved,&type,reinterpret_cast<BYTE*>(wide.data()),&size)==0,"wide SDK query override");
    require(size==24&&type==1&&std::wstring(wide.data())==L"C:\\game.exe"&&seen.name==wname,"UTF16 wide copy writes wchar-backed memory through byte policy");
    require(reg_close_entry(out)==0&&!f.fabricated.contains(0x9876)&&f.effects.frees==1,"fabricated native close bypasses original and erases tracking");
    require(reg_close_entry(reinterpret_cast<HKEY>(0x4568))==0&&!f.clients.contains(0x4568)&&GetLastError()==324,"real native close forwards then erases");
}
void test_process_folder_connect(){
    Fixture f;STARTUPINFOA sa{};STARTUPINFOW sw{};PROCESS_INFORMATION info{};SECURITY_ATTRIBUTES p{},t{};char cmd[]="arg";wchar_t wcmd[]=L"arg";
    // Pointer identity must use the same caller objects, independent of /GF pooling.
    const char app_a[]="different",cwd_a[]="cwd";
    decltype(&::CreateProcessA) pa=&process_a_entry;
    require(pa(app_a,cmd,&p,&t,-7,0xdeadbeef,reinterpret_cast<void*>(0x123),cwd_a,&sa,&info)==7,"native BOOL result preserves nonboolean value");
    require(seen.process==std::array<std::uintptr_t,10>{reinterpret_cast<std::uintptr_t>(app_a),reinterpret_cast<std::uintptr_t>(cmd),reinterpret_cast<std::uintptr_t>(&p),reinterpret_cast<std::uintptr_t>(&t),static_cast<unsigned>(-7),0xdeadbeef,0x123,reinterpret_cast<std::uintptr_t>(cwd_a),reinterpret_cast<std::uintptr_t>(&sa),reinterpret_cast<std::uintptr_t>(&info)},"all ten SDK process arguments forwarded");
    require(info.dwProcessId==0xfedcba98&&GetLastError()==325,"native process output and error state retained");
    info.dwProcessId=42;SetLastError(567);const auto before=seen.process_a.load();
    require(pa("c:\\GAME.EXE",cmd,&p,&t,0,0,nullptr,nullptr,&sa,&info)==0&&seen.process_a==before&&info.dwProcessId==42&&GetLastError()==567,"blocked native process preserves untouched output and LastError");
    const wchar_t app[]=L"different";const wchar_t cwd[]=L"cwd";decltype(&::CreateProcessW) pw=&process_w_entry;
    require(pw(app,wcmd,&p,&t,9,12,nullptr,cwd,&sw,&info)==7&&seen.process[0]==reinterpret_cast<std::uintptr_t>(app)&&seen.process[1]==reinterpret_cast<std::uintptr_t>(wcmd)&&seen.process[7]==reinterpret_cast<std::uintptr_t>(cwd)&&seen.process[8]==reinterpret_cast<std::uintptr_t>(&sw),"wide native original sees original wchar pointers");
    std::array<char,260> path{};std::array<wchar_t,260> wpath{};decltype(&::SHGetFolderPathA) fa=&folder_a_entry;decltype(&::SHGetFolderPathW) fw=&folder_w_entry;
    require(fa(reinterpret_cast<HWND>(1),5,reinterpret_cast<HANDLE>(2),3,path.data())==0&&std::strcmp(path.data(),"D:\\docs")==0,"native narrow documents override");
    require(fw(reinterpret_cast<HWND>(1),5,reinterpret_cast<HANDLE>(2),3,wpath.data())==0&&std::wstring(wpath.data())==L"D:\\docs","native wide documents override with wchar storage");
    require(std::bit_cast<std::uint32_t>(fa(reinterpret_cast<HWND>(1),0x8005,reinterpret_cast<HANDLE>(2),3,path.data()))==0x87654321u&&seen.folder[4]==reinterpret_cast<std::uintptr_t>(path.data())&&GetLastError()==326,"nonexact folder id forwards all native outputs/HRESULT bits");
    require(std::bit_cast<std::uint32_t>(fw(reinterpret_cast<HWND>(1),6,reinterpret_cast<HANDLE>(2),3,wpath.data()))==0x87654321u&&seen.folder[4]==reinterpret_cast<std::uintptr_t>(wpath.data()),"wide folder fallback pointer and result");
    sockaddr_in socket_address{};socket_address.sin_family=AF_INET;socket_address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);auto* raw=reinterpret_cast<unsigned char*>(&socket_address);raw[2]=0x0c;raw[3]=0x90;
    const auto old=socket_address;decltype(&::connect) connect=&connect_entry;
    require(connect(0x123,reinterpret_cast<const sockaddr*>(&socket_address),sizeof socket_address)==-10035,"native socket callback forwards original result");
    require(raw[2]==0x0c&&raw[3]==0x90&&f.attempts==1&&seen.address!=reinterpret_cast<const sockaddr*>(&socket_address)&&seen.connect_port==6099&&seen.length==16&&GetLastError()==327,"native connect redirects a complete copy, retains const caller bytes/error");
    require(std::memcmp(reinterpret_cast<const unsigned char*>(&old)+4,raw+4,sizeof(old)-4)==0,"connect preserves native IP and remaining structure bytes");
    SetLastError(777);
    require(connect(9,nullptr,-1)==-10035&&seen.address==nullptr&&seen.length==-1&&
            seen.connect_incoming==777&&GetLastError()==327,"length bypass forwards null native socket pointer and error state without access");
    const auto connect_before=seen.connect.load();SetLastError(778);
    require(connect(9,nullptr,16)==-10035&&seen.connect==connect_before+1&&seen.address==nullptr&&
            seen.length==16&&seen.connect_incoming==778&&GetLastError()==327&&f.attempts==1,
            "recognized-length null pointer reaches original Winsock path without fail-fast");
}
void test_read_only_connect_address(){
    Fixture f;
    for(bool ipv6:{false,true}) {
        auto* page=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
        require(page!=nullptr,"allocate protected caller address");
        const int length=ipv6?sizeof(sockaddr_in6):sizeof(sockaddr_in);
        if(ipv6){auto& a=*static_cast<sockaddr_in6*>(page);a.sin6_family=AF_INET6;a.sin6_port=htons(3216);a.sin6_addr.u.Byte[15]=1;}
        else{auto& a=*static_cast<sockaddr_in*>(page);a.sin_family=AF_INET;a.sin_port=htons(3216);a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);}
        std::array<std::byte,sizeof(sockaddr_in6)> before{};
        std::memcpy(before.data(),page,static_cast<std::size_t>(length));
        DWORD previous=0;require(VirtualProtect(page,4096,PAGE_READONLY,&previous)!=0,"caller storage is read-only");
        const auto result=connect_entry(123,static_cast<const sockaddr*>(page),length);
        const auto error=GetLastError();
        const auto original_port=ntohs(static_cast<const sockaddr_in*>(page)->sin_port);
        const bool unchanged=std::memcmp(before.data(),page,static_cast<std::size_t>(length))==0;
        const bool released=VirtualFree(page,0,MEM_RELEASE)!=0;
        require(result==-10035&&error==327&&original_port==3216&&seen.connect_port==6099&&unchanged&&released,"IPv4/IPv6 read-only storage redirects safely, preserves all caller bytes and Winsock error");
    }
}
void test_unreadable_connect_address(){
    Fixture f;
    constexpr SIZE_T page_size=4096;
    auto* area=static_cast<std::byte*>(VirtualAlloc(nullptr,page_size*2,
        MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    require(area!=nullptr,"allocate guarded sockaddr pages");
    DWORD previous=0;
    require(VirtualProtect(area+page_size,page_size,PAGE_NOACCESS,&previous)!=0,
            "guard upper page from callback inspection");
    for(int length:{16,28})for(auto* invalid:{area+page_size,area+page_size-8}){
        const auto before=seen.connect.load();const auto errors=active_calls();
        SetLastError(761);
        const auto result=connect_entry(123,reinterpret_cast<const sockaddr*>(invalid),length);
        require(result==-10035&&seen.connect==before+1&&seen.address==reinterpret_cast<const sockaddr*>(invalid)&&
                seen.length==length&&seen.connect_incoming==761&&GetLastError()==327&&active_calls()==errors&&
                f.attempts==0,
                "unreadable and boundary-spanning pointers forward intact without hook dereference");
    }
    require(VirtualFree(area,0,MEM_RELEASE)!=0,"release guarded sockaddr pages");
}
void test_readable_connect_passthrough_error_state(){
    Fixture f;
    for(bool ipv6:{false,true}){
        sockaddr_in6 address{};
        address.sin6_family=ipv6?AF_INET6:AF_INET;
        address.sin6_port=htons(443);
        const auto* pointer=reinterpret_cast<const sockaddr*>(&address);
        const auto before=seen.connect.load();
        SetLastError(782);
        const auto result=connect_entry(456,pointer,ipv6?28:16);
        require(result==-10035&&seen.connect==before+1&&seen.address==pointer&&
                seen.length==(ipv6?28:16)&&seen.connect_incoming==782&&GetLastError()==327&&
                f.attempts==0&&active_calls()==0,
                "readable nontarget preserves pointer/length and both incoming and outgoing error state");
    }
}
void test_real_winsock_invalid_address_equivalence(){
    // Genuine Windows-only comparison: the hook must delegate malformed caller
    // addresses to the SDK trampoline without inventing a WSA error or terminating.
    WSADATA startup{};
    require(WSAStartup(MAKEWORD(2,2),&startup)==0,"initialize real Winsock error oracle");
    SOCKET socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    require(socket!=INVALID_SOCKET,"create real socket for native fault comparison");
    Fixture f;
    f.originals[0]=reinterpret_cast<std::uintptr_t>(&::connect);
    auto* area=static_cast<std::byte*>(VirtualAlloc(nullptr,8192,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    require(area!=nullptr,"allocate real Winsock pointer-fault pages");
    DWORD old=0;
    require(VirtualProtect(area+4096,4096,PAGE_NOACCESS,&old)!=0,"protect real Winsock fault page");
    for(int length:{-1,0,16,28,29})for(auto* pointer:{static_cast<const sockaddr*>(nullptr),
                       reinterpret_cast<const sockaddr*>(area+4096),
                       reinterpret_cast<const sockaddr*>(area+4096-8)}){
        WSASetLastError(2713);
        const int expected=::connect(socket,pointer,length);
        const int expected_error=WSAGetLastError();
        WSASetLastError(2714);
        const int observed=connect_entry(socket,pointer,length);
        const int observed_error=WSAGetLastError();
        require(expected==observed&&expected_error==observed_error&&
                expected==SOCKET_ERROR&&f.attempts==0&&active_calls()==0,
                "real SDK and hook must agree on invalid pointer/length errors");
    }
    require(VirtualFree(area,0,MEM_RELEASE)!=0,"free protected Winsock comparison pages");
    require(closesocket(socket)==0,"release real Winsock comparison socket");
    require(WSACleanup()==0,"leave real Winsock fixture balanced");
    std::cout<<"PASS: real Winsock invalid-address return/error equivalence across pointer and length cases\n";
}
void test_binding_lifecycle(){
    Fixture f;try{Binding second(f.context,faults);require(false,"duplicate binding rejected");}catch(const Life& x){require(x.fault==LifecycleFault::AlreadyBound&&is_bound(),"duplicate bind leaves active context");}
    {std::lock_guard lock(seen.mutex);seen.block=true;seen.entered=false;seen.release=false;}
    std::atomic<BOOL> result{};std::atomic<DWORD> error{};
    std::thread worker([&]{SetLastError(17);result=process_a_entry("other",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);error=GetLastError();});
    {std::unique_lock lock(seen.mutex);seen.cv.wait(lock,[]{return seen.entered;});}
    require(active_calls()==1,"process-global context visible on worker thread");
    try{f.binding.close();require(false,"inflight close rejected");}catch(const Life& x){require(x.fault==LifecycleFault::Busy&&is_bound(),"busy binding keeps live context");}
    {std::lock_guard lock(seen.mutex);seen.release=true;seen.cv.notify_all();}worker.join();seen.block=false;
    require(result==7&&error==325&&active_calls()==0,"callback releases lease and preserves worker-local LastError");
    f.binding.close();require(!is_bound(),"explicit close succeeds after callback quiescence");f.binding.close();require(!is_bound(),"close idempotent");
    {Binding replacement(f.context,faults);require(is_bound(),"context may be bound again explicitly after quiescence");}
    require(!is_bound(),"binding destructor closes idle context");
}
void test_concurrent_binding_constructors(){
    Fixture first;first.binding.close();
    Fixture second;second.binding.close();
    first.values["Documents"]=u"C:\\first";
    second.values["Documents"]=u"D:\\second";
    std::array<Context*,2> contexts{&first.context,&second.context};
    std::array<std::unique_ptr<Binding>,2> owners;
    std::array<bool,2> rejected{};
    std::array<std::exception_ptr,2> unexpected{};
    std::barrier start(3);
    auto construct=[&](std::size_t index){
        start.arrive_and_wait();
        try{owners[index]=std::make_unique<Binding>(*contexts[index],faults);}
        catch(const Life& error){rejected[index]=error.fault==LifecycleFault::AlreadyBound;}
        catch(...){unexpected[index]=std::current_exception();}
        // Hold the winner until both constructors finish, so the loser cannot
        // succeed merely because the first owner was already destroyed.
    };
    std::thread left(construct,0),right(construct,1);
    start.arrive_and_wait();left.join();right.join();
    require(!unexpected[0]&&!unexpected[1],"concurrent binding construction has no unexpected exception");
    require(static_cast<bool>(owners[0])!=static_cast<bool>(owners[1])&&
            rejected[0]!=rejected[1]&&is_bound(),
            "concurrent constructors admit exactly one binding and reject the other");
    const std::size_t winner=owners[0]?0:1;
    std::array<wchar_t,260> output{};
    require(folder_w_entry(nullptr,5,nullptr,0,output.data())==0&&
            std::wstring(output.data())==(winner==0?L"C:\\first":L"D:\\second")&&active_calls()==0,
            "native callback observes the winning constructor's context");
    owners[winner].reset();
    require(!is_bound(),"winning constructor retains sole cleanup ownership");
}
void test_closed_binding_destruction_with_live_replacement(){
    Fixture f;f.binding.close();
    auto old=std::make_unique<Binding>(f.context,faults);
    old->close();
    Binding replacement(f.context,faults);
    old.reset();
    require(is_bound(),"destroying a closed binding preserves its live replacement");
    std::array<wchar_t,260> output{};
    require(folder_w_entry(nullptr,5,nullptr,0,output.data())==0&&
            std::wstring(output.data())==L"D:\\docs"&&active_calls()==0,
            "replacement remains callable after closed owner destruction");
    replacement.close();
    require(!is_bound(),"replacement closes normally after old owner destruction");
}
void test_staged_cross_family_binding_drain(){
    // A11-04/B51 comparison: current product owns a *shared native hook*
    // lease counter, not a donor-only Application-entry admission gate.
    // Hold two different SDK callback families independently; a successful
    // first return must not release the second callback's Binding ownership.
    Fixture f;
    const auto before_process=seen.process_a.load();
    const auto before_connect=seen.connect.load();
    {std::lock_guard lock(seen.mutex);seen.block=true;seen.entered=false;seen.release=false;}
    std::mutex peer_mutex;std::condition_variable peer_cv;
    bool peer_entered=false,peer_release=false;
    seen.connect_wait=[&](SOCKET socket){
        if(socket!=202)return;
        std::unique_lock lock(peer_mutex);
        peer_entered=true;peer_cv.notify_all();
        peer_cv.wait(lock,[&]{return peer_release;});
    };
    BOOL process_result=0;DWORD process_error=0;
    int connect_result=0;DWORD connect_error=0;
    std::thread process_thread([&]{
        SetLastError(0xa110);
        process_result=process_a_entry("other",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);
        process_error=GetLastError();
    });
    bool process_entered=false;
    {std::unique_lock lock(seen.mutex);
     process_entered=seen.cv.wait_for(lock,std::chrono::seconds(5),[]{return seen.entered;});}
    std::thread connect_thread([&]{
        SetLastError(0xa111);
        connect_result=connect_entry(202,nullptr,-1);
        connect_error=GetLastError();
    });
    bool connect_entered=false;
    {std::unique_lock lock(peer_mutex);
     connect_entered=peer_cv.wait_for(lock,std::chrono::seconds(5),[&]{return peer_entered;});}
    const auto both_active=active_calls();
    bool both_busy=false;
    try{f.binding.close();}catch(const Life& error){both_busy=error.fault==LifecycleFault::Busy;}
    const bool both_bound=is_bound();
    {std::lock_guard lock(seen.mutex);seen.release=true;seen.cv.notify_all();}
    process_thread.join();
    const auto one_active=active_calls();
    bool one_busy=false;
    try{f.binding.close();}catch(const Life& error){one_busy=error.fault==LifecycleFault::Busy;}
    const bool one_bound=is_bound();
    {std::lock_guard lock(peer_mutex);peer_release=true;peer_cv.notify_all();}
    connect_thread.join();
    seen.connect_wait={};
    {std::lock_guard lock(seen.mutex);seen.block=false;}
    const auto drained=active_calls();
    const bool bound_before_final_close=is_bound();
    if(drained==0&&bound_before_final_close)f.binding.close();
    require(process_entered&&connect_entered&&both_active==2&&both_busy&&both_bound,
        "two independent OS callback families retain distinct native Binding leases");
    require(one_active==1&&one_busy&&one_bound,
        "Binding close must remain busy after the first callback returns");
    require(drained==0&&bound_before_final_close&&!is_bound(),
        "only final callback retirement permits a successful Binding close");
    require(process_result==7&&process_error==325&&connect_result==-10035&&connect_error==327,
        "each admitted native callback retains its exact return and thread-local error");
    require(seen.process_a==before_process+1&&seen.connect==before_connect+1,
        "exactly one original process and one original connect call complete");
}
void test_live_owner_state_identity(){
    Fixture f;
    // Mutations AFTER Effects/Context/Binding construction must reach callbacks.
    // Equal-valued snapshots would pass construction checks and fail these uses.
    f.originals[3]=reinterpret_cast<std::uintptr_t>(&replacement_close);
    const auto key=reinterpret_cast<HKEY>(0x112233);
    require(reg_close_entry(key)==-77&&seen.key==key&&GetLastError()==0x4a31,
        "callback reads the live owner's original slot, not a copied array");
    f.values["Documents"]=u"E:\\changed";
    std::array<wchar_t,260> output{};
    require(folder_w_entry(nullptr,5,nullptr,0,output.data())==0&&std::wstring(output.data())==L"E:\\changed",
        "native Context reads the owner's updated map");
    f.port=6137;
    sockaddr_in address{};address.sin_family=AF_INET;address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);address.sin_port=htons(3216);
    const auto before=address;
    require(connect_entry(123,reinterpret_cast<const sockaddr*>(&address),sizeof address)==-10035&&seen.connect_port==6137,
        "connect observes the live owner's selected port");
    require(std::memcmp(&before,&address,sizeof address)==0&&active_calls()==0,
        "identity checks preserve caller memory and release every callback lease");
}
void test_crypt_forwarding(){
    Fixture f;
    DWORD encoding=1,content=2,format=3;HCERTSTORE store{};HCRYPTMSG message{};const void* context{};
    std::array<std::byte,9> blob{};const auto before=blob;
    decltype(&::CryptQueryObject) query=&crypt_query_entry;
    SetLastError(71);seen.crypt_result=7;
    require(query(2,blob.data(),0xa1,0xb2,0xc3,&encoding,&content,&format,&store,&message,&context)==7,
        "CryptQueryObject preserves non-normalized BOOL result");
    require(seen.crypt==std::array<std::uintptr_t,11>{2,reinterpret_cast<std::uintptr_t>(blob.data()),0xa1,0xb2,0xc3,
        reinterpret_cast<std::uintptr_t>(&encoding),reinterpret_cast<std::uintptr_t>(&content),
        reinterpret_cast<std::uintptr_t>(&format),reinterpret_cast<std::uintptr_t>(&store),
        reinterpret_cast<std::uintptr_t>(&message),reinterpret_cast<std::uintptr_t>(&context)},
        "all eleven SDK arguments and every output pointer retain identity");
    require(blob==before&&encoding==0x111&&content==0x222&&format==0x333&&
        store==reinterpret_cast<HCERTSTORE>(0x444)&&message==reinterpret_cast<HCRYPTMSG>(0x555)&&
        context==reinterpret_cast<const void*>(0x666),"original output cells propagate without fabricated values");
    require(seen.crypt_incoming==71&&GetLastError()==0x1234abcd,"incoming/outgoing LastError preserved");
    const wchar_t* cases[]={nullptr,L"preloader_l.dll",L"C:\\synthetic\\preloader_l.dll",
        L"xpreloader_l.dll",L"\U0001f600preloader_l.dll",L"PRELOADER_L.DLL",L"preloader_l.dll.old",L""};
    for(std::size_t i=0;i<std::size(cases);++i){
        SetLastError(72);const int calls=seen.crypt_calls;
        require(query(1,cases[i],0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==7&&seen.crypt_calls==calls+1,
            "one original call for each recovered selector branch");
        if(i<5)require(seen.crypt_object==L"EAAntiCheat.GameServiceLauncher.dll","RE companion selection retained");
        else require(seen.crypt[1]==reinterpret_cast<std::uintptr_t>(cases[i]),"nonmatching/case-sensitive input remains original pointer");
        require(seen.crypt_incoming==72&&GetLastError()==0x1234abcd,
            "safe-read and diagnostics cannot corrupt original API LastError boundary");
    }
    const wchar_t nul[]={L'p',0,L'p',L'r',L'e'};SetLastError(73);
    require(query(1,nul,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==7&&seen.crypt[1]==reinterpret_cast<std::uintptr_t>(nul),
        "UTF16 NUL ends path before any later suffix bytes");
    const wchar_t high[]={0xd800,0},low[]={0xdc00,0},broken[]={0xd800,L'x',0};
    for(const auto* invalid:{high,low,broken}){
        require(query(1,invalid,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==7&&seen.crypt[1]==reinterpret_cast<std::uintptr_t>(invalid),"Hook reference invalid UTF16 retains original pointer without trap");
    }
    f.crypt_host.readable=false;
    const wchar_t matching[]=L"preloader_l.dll";
    require(query(1,matching,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==7&&seen.crypt[1]==reinterpret_cast<std::uintptr_t>(matching),"unreadable input bypasses substitution");
    f.crypt_host.readable=true;
    require(!f.crypt_host.native.inspect_path(reinterpret_cast<const void*>(1)).valid,"actual Windows reader rejects inaccessible address without dereference");
    SYSTEM_INFO system{};GetSystemInfo(&system);
    auto* pages=static_cast<std::byte*>(VirtualAlloc(nullptr,system.dwPageSize*2,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
    require(pages!=nullptr,"protected reader fixture pages allocated");
    DWORD old=0;require(VirtualProtect(pages+system.dwPageSize,system.dwPageSize,PAGE_NOACCESS,&old)!=FALSE,"protected fixture boundary set");
    const auto units=std::size(matching)-1;
    auto* truncated=pages+system.dwPageSize-units*sizeof(wchar_t);
    std::memcpy(truncated,matching,units*sizeof(wchar_t));
    const auto partial=f.crypt_host.native.inspect_path(truncated);
    require(!partial.valid,"matching suffix without readable terminator remains unreadable");
    require(preloader_l::hooks::select_object(1,false,partial)==preloader_l::hooks::Selection::original,"unreadable matching suffix does not substitute");
    auto* terminated=pages+system.dwPageSize-sizeof(matching);
    std::memcpy(terminated,matching,sizeof(matching));
    const auto complete=f.crypt_host.native.inspect_path(terminated);
    require(complete.valid&&complete.suffix_matches,"NUL at final readable code unit completes protected reader");
    require(VirtualFree(pages,0,MEM_RELEASE)!=FALSE,"protected reader fixture pages released");
    require(f.effects.crypt_observations==seen.crypt_calls,"one observation for each original call");
    seen.crypt_result=0;SetLastError(74);
    require(query(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==0&&GetLastError()==0x1234abcd,
        "real original failure is not converted to success");
    seen.crypt_result=7;
}
void test_crypt_lifetime(){
    Fixture f;
    {std::lock_guard lock(seen.mutex);seen.block=true;seen.entered=false;seen.release=false;}
    std::atomic<BOOL> result{};std::atomic<DWORD> error{};
    std::thread worker([&]{SetLastError(75);result=crypt_query_entry(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);error=GetLastError();});
    {std::unique_lock lock(seen.mutex);seen.cv.wait(lock,[]{return seen.entered;});}
    require(active_calls()==1,"crypt callback lease remains held while original trampoline is blocked");
    try{f.binding.close();require(false,"in-flight crypt close rejected");}
    catch(const Life& x){require(x.fault==LifecycleFault::Busy&&is_bound(),"busy crypt callback retains live owner");}
    {std::lock_guard lock(seen.mutex);seen.release=true;seen.cv.notify_all();}
    worker.join();seen.block=false;
    require(result==7&&error==0x1234abcd&&active_calls()==0,"crypt original finishes before lease releases");
    f.binding.close();require(!is_bound(),"crypt context closes after callback quiescence");
}
// Child processes isolate terminal paths. A closed binding has no owned
// error policy; a live exception retains its lease even while another thread
// attempts close. Guarded retired policy storage detects any stale dereference.
void test_terminal_policy_lifetime() {
    std::array<wchar_t,32768> executable{};
    const DWORD n=GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size()));
    require(n>0&&n<executable.size(),"native callback executable path");
    struct Scenario {const wchar_t* mode;DWORD expected;};
    for(const auto& scenario:std::array<Scenario,7>{{
            {L"--closed",87},{L"--crypt-closed",87},{L"--throw-original",86},
            {L"--missing",86},{L"--crypt-missing",86},
            {L"--retired-owner",87},{L"--crypt-retired-owner",87}}}) {
        auto command=L"\"" + std::wstring(executable.data(),n) + L"\" " + scenario.mode;
        STARTUPINFOW startup{};startup.cb=static_cast<DWORD>(sizeof(startup));
        PROCESS_INFORMATION process{};
        require(CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,
                               nullptr,nullptr,&startup,&process)!=FALSE,
                "spawn isolated terminal callback regression");
        const DWORD waited=WaitForSingleObject(process.hProcess,15000);
        DWORD code=0;
        const BOOL got=GetExitCodeProcess(process.hProcess,&code);
        if(waited!=WAIT_OBJECT_0){
            (void)TerminateProcess(process.hProcess,88);
            (void)WaitForSingleObject(process.hProcess,5000);
        }
        (void)CloseHandle(process.hThread);
        (void)CloseHandle(process.hProcess);
        if(waited!=WAIT_OBJECT_0||!got||code!=scenario.expected)
            std::wcerr<<L"terminal scenario "<<scenario.mode<<L": wait="<<waited
                      <<L" exit="<<code<<L" expected="<<scenario.expected<<L'\n';
        require(waited==WAIT_OBJECT_0&&got&&code==scenario.expected,
                "retired callback policy cannot run; live policy still handles exceptions");
    }
}
#if !defined(PRELOADER_L_PRODUCTION_CALLBACK_TESTS)
struct InitRemaining final:preloader_l::host_init::Effects {
    std::array<std::uintptr_t,callback_count>& targets;std::size_t index{};
    explicit InitRemaining(std::array<std::uintptr_t,callback_count>& t):targets(t){}
    std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t t,std::uint8_t)override{return m+t;}
    bool verify_version(const preloader_l::host_init::VersionRequest&,std::uint32_t,std::uint64_t)override{return true;}
    std::int32_t initialize_engine_046a20()override{throw std::logic_error("unrouted engine init");}
    std::uintptr_t get_module_handle(std::u16string_view)override{return 1;}
    std::uintptr_t get_proc_address(std::uintptr_t,std::string_view)override{return targets.at(index++);}
    std::int32_t create_hook_0466e0(std::uintptr_t,Callback,std::uintptr_t&)override{throw std::logic_error("unrouted create");}
    std::int32_t enable_hook_046a10(std::uintptr_t)override{throw std::logic_error("unrouted enable");}
    void log_text_006880(std::string_view)override{}
    void log_integer_00af90(std::int32_t)override{}
    void finish_log_0088e0()override{}
};
struct InitApi final:preloader_l::platform::hook_engine::Api {
    std::array<std::uintptr_t,callback_count>& targets;std::array<std::uintptr_t,callback_count> detours{};std::size_t enabled{};
    explicit InitApi(std::array<std::uintptr_t,callback_count>& t):targets(t){}
    int initialize()override{return 0;}
    int create(void* target,void* detour,void** out)override{
        const auto it=std::find(targets.begin(),targets.end(),reinterpret_cast<std::uintptr_t>(target));require(it!=targets.end(),"engine receives synthetic executable function target");
        detours[static_cast<std::size_t>(it-targets.begin())]=reinterpret_cast<std::uintptr_t>(detour);*out=target;return 0;
    }
    int enable(void* target)override{
        const auto it=std::find(targets.begin(),targets.end(),reinterpret_cast<std::uintptr_t>(target));require(it!=targets.end(),"enable receives created target");
        const auto index=static_cast<std::size_t>(it-targets.begin());require(index==enabled++,"host install and native resolver order");
        // Models a callback arriving before EnableHook returns. Each original
        // slot must already be published by the HostInitEffects create path.
        const auto entry=detours[index];HKEY key{};DWORD type{},size{};
        switch(index){
        case 0:reinterpret_cast<decltype(&::connect)>(entry)(0,nullptr,-1);break;
        case 1:reinterpret_cast<decltype(&::RegOpenKeyExA)>(entry)(nullptr,nullptr,0,0,&key);break;
        case 2:reinterpret_cast<decltype(&::RegOpenKeyExW)>(entry)(nullptr,nullptr,0,0,&key);break;
        case 3:reinterpret_cast<decltype(&::RegCloseKey)>(entry)(nullptr);break;
        case 4:reinterpret_cast<decltype(&::RegQueryValueExA)>(entry)(nullptr,nullptr,nullptr,&type,nullptr,&size);break;
        case 5:reinterpret_cast<decltype(&::RegQueryValueExW)>(entry)(nullptr,nullptr,nullptr,&type,nullptr,&size);break;
        case 6:reinterpret_cast<decltype(&::CreateProcessA)>(entry)(nullptr,nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);break;
        case 7:reinterpret_cast<decltype(&::CreateProcessW)>(entry)(nullptr,nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);break;
        case 8:reinterpret_cast<decltype(&::SHGetFolderPathA)>(entry)(nullptr,0,nullptr,0,nullptr);break;
        case 9:reinterpret_cast<decltype(&::SHGetFolderPathW)>(entry)(nullptr,0,nullptr,0,nullptr);break;
        case 10:reinterpret_cast<decltype(&::CryptQueryObject)>(entry)(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);break;
        }
        require(active_calls()==0,"SDK entry released its lease before synthetic enable returns");return 0;
    }
    int disable(void*)override{throw std::logic_error("unexpected native install disable");}
    int uninitialize()override{throw std::logic_error("unexpected native install uninit");}
};
void test_actual_host_install_binding(){
    Fixture f;auto targets=f.originals;f.originals.fill(0);InitRemaining remaining(targets);InitApi api(targets);Resolver resolver;
    preloader_l::platform::hook_engine::HostInitEffects effects(remaining,api,resolver);preloader_l::configuration::State config{};
    config.executable_path=u"C:\\synthetic\\game.exe";config.block_reruns=1;config.documents=u"D:\\synthetic-docs";
    std::vector<std::uintptr_t> enabled;
    preloader_l::host_init::initialize_host_00e310({config,f.values,enabled,f.originals},effects);
    require(api.enabled==callback_count&&enabled==std::vector<std::uintptr_t>(targets.begin(),targets.end()),"actual host-init/native resolver/callback/original-effect graph installs eleven synthetic callbacks");
    require(f.originals==targets,"all original slots populated before callback can run");
}
#endif

}
int main(int argc,char** argv){try{
    if(argc>1){
        boundary_mode=true;
        SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
        if(std::strcmp(argv[1],"--retired-owner")==0||std::strcmp(argv[1],"--crypt-retired-owner")==0){
            std::set_terminate([]{std::_Exit(87);});
            void* storage=VirtualAlloc(nullptr,sizeof(Faults),MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
            require(storage!=nullptr,"allocate isolated error policy page");
            auto* policy=::new(storage) Faults;
            {Fixture f(*policy);f.binding.close();}
            policy->~Faults();
            DWORD previous=0;
            require(VirtualProtect(storage,sizeof(Faults),PAGE_NOACCESS,&previous)!=FALSE,
                    "retired error policy page is inaccessible");
            // The terminal child owns this allocation until process exit.
            if(std::strcmp(argv[1],"--retired-owner")==0)reg_close_entry(nullptr);
            else crypt_query_entry(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);
            return 98;
        }
        Fixture f;faults.lease_owner=&f.binding;
        if(std::strcmp(argv[1],"--closed")==0){std::set_terminate([]{std::_Exit(87);});expected_failure=Callback::reg_close_00f5b0;f.binding.close();reg_close_entry(nullptr);}
        else if(std::strcmp(argv[1],"--missing")==0){expected_failure=Callback::reg_close_00f5b0;f.originals[3]=0;reg_close_entry(nullptr);}
        else if(std::strcmp(argv[1],"--throw-original")==0){expected_failure=Callback::create_process_a_00f3f0;seen.throw_process=true;process_a_entry("other",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);}
        else if(std::strcmp(argv[1],"--crypt-missing")==0){expected_failure=Callback::crypt_query_001264;f.originals[10]=0;crypt_query_entry(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);}
        else if(std::strcmp(argv[1],"--crypt-closed")==0){std::set_terminate([]{std::_Exit(87);});expected_failure=Callback::crypt_query_001264;f.binding.close();crypt_query_entry(2,nullptr,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr);}
        else if(std::strcmp(argv[1],"--crypt-invalid")==0){const wchar_t invalid[]={0xd800,0};require(crypt_query_entry(1,invalid,0,0,0,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr)==7,"invalid UTF16 forwards instead of trapping");return 0;}
        return 98;
    }
    test_resolver_and_registry();test_process_folder_connect();test_binding_lifecycle();test_concurrent_binding_constructors();test_closed_binding_destruction_with_live_replacement();test_staged_cross_family_binding_drain();test_live_owner_state_identity();test_crypt_forwarding();test_crypt_lifetime();
#if !defined(PRELOADER_L_PRODUCTION_CALLBACK_TESTS)
    test_actual_host_install_binding();
#endif
    test_registry_startup_wait_allows_peer_callbacks();
    test_read_only_connect_address();
    test_unreadable_connect_address();
    test_readable_connect_passthrough_error_state();
    test_real_winsock_invalid_address_equivalence();
    test_network_wait_allows_peer_callbacks(true);test_network_wait_allows_peer_callbacks(false);test_concurrent_attempts();
    test_hook_reference_common_guard_scope();
    test_terminal_policy_lifetime();
    std::cout<<"PASS: "<<checks<<" native callback/trampoline checks; synthetic originals plus real Winsock invalid-address equivalence\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected native test exception\n";return 1;}}
