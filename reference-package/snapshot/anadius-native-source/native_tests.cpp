#include "native_callbacks.hpp"
#include <algorithm>
#include <atomic>
#include <bit>
#include <cstring>
#include <iostream>
#include <thread>
#include <string>
#include <cstdlib>
#include <condition_variable>
using namespace anadius::native_recovered;
namespace {
int checks;
void require(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
struct Life{LifecycleFault fault;};
Callback expected_failure=Callback::connect_00fd00;
bool boundary_mode{};
struct Faults final:Errors {
    [[noreturn]]void lifecycle(LifecycleFault f)override{throw Life{f};}
    [[noreturn]]void callback_exception(Callback c,std::exception_ptr error)noexcept override{
        std::_Exit(boundary_mode&&c==expected_failure&&error?86:99);
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
    SOCKET socket{};const sockaddr* address{};int length{};
    std::atomic<int> open_a{},open_w{},query_a{},query_w{},close{},process_a{},process_w{},folder_a{},folder_w{},connect{};
    std::mutex mutex;std::condition_variable cv;bool block{},entered{},release{};bool throw_process{};
} seen;
LSTATUS WINAPI original_open_a(HKEY k,LPCSTR s,DWORD o,REGSAM a,PHKEY p){++seen.open_a;seen.key=k;seen.name=s;seen.options=o;seen.access=a;seen.result=p;if(p)*p=reinterpret_cast<HKEY>(0x4567);SetLastError(321);return -5;}
LSTATUS WINAPI original_open_w(HKEY k,LPCWSTR s,DWORD o,REGSAM a,PHKEY p){++seen.open_w;seen.key=k;seen.name=s;seen.options=o;seen.access=a;seen.result=p;if(p)*p=reinterpret_cast<HKEY>(0x4568);SetLastError(322);return 0;}
template<class Char>LSTATUS query(HKEY k,const Char* s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){seen.key=k;seen.name=s;seen.reserved=r;seen.type=t;seen.data=d;seen.size=n;if(n)*n=1;if(t)*t=0;if(d)*d=0x7f;SetLastError(323);return -23;}
LSTATUS WINAPI original_query_a(HKEY k,LPCSTR s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){++seen.query_a;return query(k,s,r,t,d,n);}
LSTATUS WINAPI original_query_w(HKEY k,LPCWSTR s,LPDWORD r,LPDWORD t,LPBYTE d,LPDWORD n){++seen.query_w;return query(k,s,r,t,d,n);}
LSTATUS WINAPI original_close(HKEY k){++seen.close;seen.key=k;SetLastError(324);return 0;}
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
int WSAAPI original_connect(SOCKET s,const sockaddr* p,int n){++seen.connect;seen.socket=s;seen.address=p;seen.length=n;SetLastError(327);return -10035;}
struct Effects final:OriginalEffects {
    int waits{},allocs{},frees{};std::vector<std::string> events;
    Effects(const std::array<std::uintptr_t,10>& s,Wide& w):OriginalEffects(s,w){}
    std::string filesystem_narrow(std::u16string_view s)override{return {s.begin(),s.end()};}
    int crt_stricmp(const char* a,const char* b)override{std::string x(a),y(b);for(auto& c:x)if(c>='A'&&c<='Z')c+=32;for(auto& c:y)if(c>='A'&&c<='Z')c+=32;return x.compare(y);}
    int crt_strcpy_s(char* d,std::size_t n,const char* s)override{const auto len=std::strlen(s)+1;if(!d||len>n)return 34;std::memcpy(d,s,len);return 0;}
    anadius::registry_recovered::Handle allocate_4_04b0bc()override{++allocs;return 0x9876;}
    void free_04b0f8(anadius::registry_recovered::Handle p)override{require(p==0x9876,"fabricated handle identity freed");++frees;}
    void wait_036880(std::int32_t n)override{require(n==-1,"registry startup count");++waits;}
    void wait_steps_036880(std::int32_t n)override{require(n==-1,"network startup count");++waits;}
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
    anadius::host_init_recovered::Values values{{"ClientKey",u"Client\\Key"},{"ClientKey2",u"Client\\Key2"},{"ClientValue",u"ClientPath"},{"ClientPath",u"C:\\game.exe"},{"Documents",u"D:\\docs"}};
    anadius::registry_recovered::Handles clients,languages,fabricated;std::uint8_t language=0;
    std::int32_t v4=16,v6=28;std::uint16_t port=6099;std::uint32_t attempts=0;
    std::array<std::uintptr_t,10> originals{};Wide wide;Effects effects;Context context;Binding binding;
    Fixture():originals{reinterpret_cast<std::uintptr_t>(&original_connect),reinterpret_cast<std::uintptr_t>(&original_open_a),reinterpret_cast<std::uintptr_t>(&original_open_w),reinterpret_cast<std::uintptr_t>(&original_close),reinterpret_cast<std::uintptr_t>(&original_query_a),reinterpret_cast<std::uintptr_t>(&original_query_w),reinterpret_cast<std::uintptr_t>(&original_process_a),reinterpret_cast<std::uintptr_t>(&original_process_w),reinterpret_cast<std::uintptr_t>(&original_folder_a),reinterpret_cast<std::uintptr_t>(&original_folder_w)},
      effects(originals,wide),context{{values,clients,languages,fabricated,language},{v4,v6,port,attempts},effects,effects,effects},binding(context,faults){}
};
void test_resolver_and_registry(){
    Fixture f;Resolver resolver;
    const std::array<std::uintptr_t,10> expected{reinterpret_cast<std::uintptr_t>(&connect_entry),reinterpret_cast<std::uintptr_t>(&reg_open_a_entry),reinterpret_cast<std::uintptr_t>(&reg_open_w_entry),reinterpret_cast<std::uintptr_t>(&reg_close_entry),reinterpret_cast<std::uintptr_t>(&reg_query_a_entry),reinterpret_cast<std::uintptr_t>(&reg_query_w_entry),reinterpret_cast<std::uintptr_t>(&process_a_entry),reinterpret_cast<std::uintptr_t>(&process_w_entry),reinterpret_cast<std::uintptr_t>(&folder_a_entry),reinterpret_cast<std::uintptr_t>(&folder_w_entry)};
    for(std::size_t i=0;i<10;++i)require(resolver.rebuilt_address(static_cast<Callback>(i))==expected[i],"every callback resolver result is rebuilt SDK shim");
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
    decltype(&::CreateProcessA) pa=&process_a_entry;
    require(pa("different",cmd,&p,&t,-7,0xdeadbeef,reinterpret_cast<void*>(0x123),"cwd",&sa,&info)==7,"native BOOL result preserves nonboolean value");
    require(seen.process==std::array<std::uintptr_t,10>{reinterpret_cast<std::uintptr_t>("different"),reinterpret_cast<std::uintptr_t>(cmd),reinterpret_cast<std::uintptr_t>(&p),reinterpret_cast<std::uintptr_t>(&t),static_cast<unsigned>(-7),0xdeadbeef,0x123,reinterpret_cast<std::uintptr_t>("cwd"),reinterpret_cast<std::uintptr_t>(&sa),reinterpret_cast<std::uintptr_t>(&info)},"all ten SDK process arguments forwarded");
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
    sockaddr_in socket_address{};socket_address.sin_family=AF_INET;auto* raw=reinterpret_cast<unsigned char*>(&socket_address);raw[2]=0x0c;raw[3]=0x90;
    const auto old=socket_address;decltype(&::connect) connect=&connect_entry;
    require(connect(0x123,reinterpret_cast<const sockaddr*>(&socket_address),sizeof socket_address)==-10035,"native socket callback forwards original result");
    require(raw[2]==0x17&&raw[3]==0xd3&&f.attempts==1&&seen.address==reinterpret_cast<const sockaddr*>(&socket_address)&&seen.length==16&&GetLastError()==327,"native connect rewrites port in caller memory, retains pointer/error");
    require(std::memcmp(reinterpret_cast<const unsigned char*>(&old)+4,raw+4,sizeof(old)-4)==0,"connect preserves native IP and remaining structure bytes");
    require(connect(9,nullptr,-1)==-10035&&seen.address==nullptr&&seen.length==-1,"length bypass forwards null native socket pointer without access");
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
struct InitRemaining final:anadius::host_init_recovered::Effects {
    std::array<std::uintptr_t,10>& targets;std::size_t index{};
    explicit InitRemaining(std::array<std::uintptr_t,10>& t):targets(t){}
    std::u16string filesystem_widen_006d70(std::string_view s)override{return {s.begin(),s.end()};}
    std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t t,std::uint8_t)override{return m+t;}
    bool verify_version(const anadius::host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t)override{return true;}
    std::int32_t initialize_engine_046a20()override{throw std::logic_error("unrouted engine init");}
    std::uintptr_t get_module_handle(std::u16string_view)override{return 1;}
    std::uintptr_t get_proc_address(std::uintptr_t,std::string_view)override{return targets.at(index++);}
    std::int32_t create_hook_0466e0(std::uintptr_t,Callback,std::uintptr_t&)override{throw std::logic_error("unrouted create");}
    std::int32_t enable_hook_046a10(std::uintptr_t)override{throw std::logic_error("unrouted enable");}
    void log_text_006880(std::string_view)override{}
    void log_integer_00af90(std::int32_t)override{}
    void finish_log_0088e0()override{}
};
struct InitApi final:recovered_hooks::Api {
    std::array<std::uintptr_t,10>& targets;std::array<std::uintptr_t,10> detours{};std::size_t enabled{};
    explicit InitApi(std::array<std::uintptr_t,10>& t):targets(t){}
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
        }
        require(active_calls()==0,"SDK entry released its lease before synthetic enable returns");return 0;
    }
    int disable(void*)override{throw std::logic_error("unexpected native install disable");}
    int uninitialize()override{throw std::logic_error("unexpected native install uninit");}
};
void test_actual_host_install_binding(){
    Fixture f;auto targets=f.originals;f.originals.fill(0);InitRemaining remaining(targets);InitApi api(targets);Resolver resolver;
    recovered_hooks::HostInitEffects effects(remaining,api,resolver);anadius::configuration::State config{};
    config.executable_path=u"C:\\synthetic\\game.exe";config.block_reruns=1;config.documents=u"D:\\synthetic-docs";
    std::vector<std::uintptr_t> enabled;
    anadius::host_init_recovered::initialize_host_00e310({config,f.values,enabled,f.originals},effects);
    require(api.enabled==10&&enabled==std::vector<std::uintptr_t>(targets.begin(),targets.end()),"actual host-init/native resolver/callback/original-effect graph installs ten synthetic callbacks");
    require(f.originals==targets,"all original slots populated before callback can run");
}
}
int main(int argc,char** argv){try{
    if(argc>1){boundary_mode=true;Fixture f;
        if(std::strcmp(argv[1],"--closed")==0){expected_failure=Callback::reg_close_00f5b0;f.binding.close();reg_close_entry(nullptr);}
        else if(std::strcmp(argv[1],"--missing")==0){expected_failure=Callback::reg_close_00f5b0;f.originals[3]=0;reg_close_entry(nullptr);}
        else if(std::strcmp(argv[1],"--throw-original")==0){expected_failure=Callback::create_process_a_00f3f0;seen.throw_process=true;process_a_entry("other",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr);}
        return 98;
    }
    test_resolver_and_registry();test_process_folder_connect();test_binding_lifecycle();test_actual_host_install_binding();
    std::cout<<"PASS: "<<checks<<" native callback/trampoline checks; synthetic originals, real LastError metadata only\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected native test exception\n";return 1;}}
