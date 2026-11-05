#include "native.hpp"
#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>
#include <atomic>
#include <exception>
#include <system_error>
#include <thread>

namespace sock=anadius::socket_recovered;
namespace srv=anadius::server_recovered;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Fixture;
Fixture* active=nullptr;
struct Fixture final:sock::CategoryProvider {
    std::vector<std::string> trace;
    int last=0,convert_result=4,parse_result=0,parse_size=16,send_result=0,select_result=0;
    std::wstring converted=L"abc";bool terminate=true;
    int family=AF_INET,expected_capacity=4;
    const char* expected_source="abc";
    sock::Socket expected_socket=0x123456789abcdef0ULL;
    sock::Buffer* expected_buffers=nullptr;DWORD expected_count=0,expected_flags=0,bytes=0x80000001U;
    int expected_nfds=0;
    std::array<std::uint8_t,16> address{};std::uint32_t scope=0x87654321;
    std::optional<int> category_last;
    Fixture(){require(active==nullptr,"fixtures are sequential");active=this;for(unsigned i=0;i<16;++i)address[i]=static_cast<std::uint8_t>(i+1);}
    ~Fixture(){active=nullptr;}
    sock::Category get_028620()override{trace.push_back("category");if(category_last)last=*category_last;return {0xaabbccdd,srv::ErrorCategory::other};}
};
void WSAAPI fake_set(int value){active->trace.push_back("set");active->last=value;}
int WSAAPI fake_get(){active->trace.push_back("get");return active->last;}
int WINAPI fake_convert(UINT cp,DWORD flags,LPCCH source,int count,LPWSTR out,int capacity){
    auto& f=*active;f.trace.push_back("convert");
    require(cp==0 && flags==0 && count==-1,"native ACP conversion uses codepage0 flags0 NUL-terminated input");
    require(source==f.expected_source || std::strcmp(source,f.expected_source)==0,"native conversion forwards source contents");
    require(capacity==f.expected_capacity && out!=nullptr,"native conversion forwards exact capacity and owned output");
    if(f.convert_result>0 && f.convert_result<=capacity){
        require(f.converted.size()+1==static_cast<std::size_t>(f.convert_result),"fixture initializes full reported conversion range");
        std::memcpy(out,f.converted.data(),f.converted.size()*sizeof(wchar_t));
        out[f.convert_result-1]=f.terminate?L'\0':L'x';
    }
    return f.convert_result;
}
INT WSAAPI fake_parse(LPWSTR text,INT family,LPWSAPROTOCOL_INFOW protocol,LPSOCKADDR out,LPINT size){
    auto& f=*active;f.trace.push_back("parse");
    require(std::wstring(text)==f.converted && family==f.family,"typed WSA parser receives converted text and family");
    require(protocol==nullptr && out!=nullptr && size!=nullptr && *size==128,"typed WSA parser has null protocol and SDK128byte storage");
    if(family==AF_INET){sockaddr_in value{};value.sin_family=AF_INET;std::memcpy(&value.sin_addr,f.address.data(),4);std::memcpy(out,&value,sizeof(value));}
    else{sockaddr_in6 value{};value.sin6_family=AF_INET6;std::memcpy(&value.sin6_addr,f.address.data(),16);value.sin6_scope_id=f.scope;std::memcpy(out,&value,sizeof(value));}
    *size=f.parse_size;f.last=55;return f.parse_result;
}
int WSAAPI fake_send(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD transferred,DWORD flags,LPWSAOVERLAPPED overlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE completion){
    auto& f=*active;f.trace.push_back("send");
    require(socket==f.expected_socket && buffers==f.expected_buffers && count==f.expected_count && flags==f.expected_flags,"native WSASend forwards full socket and actual SDK WSABUF table without copying");
    require(transferred!=nullptr && *transferred==0 && overlapped==nullptr && completion==nullptr,"native WSASend starts DWORD0 and uses synchronous null callbacks");
    *transferred=f.bytes;f.last=10035;return f.send_result;
}
int WSAAPI fake_select(int nfds,fd_set* read,fd_set* write,fd_set* except,const timeval* timeout){
    auto& f=*active;f.trace.push_back("select");
    require(nfds==f.expected_nfds && read==nullptr && except==nullptr && timeout==nullptr,"native select exact nfds and null read/except/timeout");
    require(write!=nullptr && write->fd_count==1 && write->fd_array[0]==f.expected_socket,"native select contains one full-width socket in write set");
    f.last=1234;return f.select_result;
}
sock::Imports imports(){sock::Imports value;value.set_last_error=fake_set;value.get_last_error=fake_get;value.convert=fake_convert;value.parse=fake_parse;value.send=fake_send;value.select_write=fake_select;return value;}
template<class F>void unsupported(F&& call,const char* why){bool caught=false;try{call();}catch(const sock::UnsupportedInput&){caught=true;}require(caught,why);}
void conversion(){
    {Fixture f;sock::NativeApi default_imports(f);require(f.trace.empty(),"default NativeApi construction invokes no operating-system import");}
    {Fixture f;sock::NativeApi api(f,imports());api.set_last_error(77);require(api.get_last_error()==77,"typed injected last-error imports");const auto cat=api.system_category_028620();require(cat.name_identity==0xaabbccdd && cat.logical==srv::ErrorCategory::other,"mandatory provider preserves opaque name identity and logical category");}
    {Fixture f;sock::NativeApi api(f,imports());const auto converted=api.convert_acp("abc",4);require(converted.result==4 && converted.text==std::optional<std::wstring>(L"abc"),"successful conversion owns initialized text without trailing NUL");require(f.trace==std::vector<std::string>{"convert"},"conversion does not call last-error/category APIs");}
    for(const int raw:{0,-1,5}){Fixture f;f.convert_result=raw;sock::NativeApi api(f,imports());auto converted=api.convert_acp("abc",4);require(converted.result==raw && !converted.text,"failed or oversized conversion exposes no initialized wide text");unsupported([&]{(void)api.string_to_address(converted,2,128);},"undefined converted storage is explicit unsupported-input domain");require(f.trace==std::vector<std::string>{"convert"},"undefined conversion does not call WSA parser or invent WSA errors");}
    {Fixture f;f.terminate=false;sock::NativeApi api(f,imports());auto converted=api.convert_acp("abc",4);require(!converted.text,"reported converted range without NUL is unsupported storage");}
    {Fixture f;sock::NativeApi api(f,imports());for(int capacity:{0,-1})unsupported([&]{(void)api.convert_acp("abc",capacity);},"nonpositive capacity rejected before native import");require(f.trace.empty(),"invalid capacity invokes no import");}
}
void addresses(){
    for(const int family:{AF_INET,AF_INET6})for(const int status:{-2,0,3,-1}){
        Fixture f;f.family=family;f.parse_size=family==AF_INET?16:28;f.parse_result=status;sock::NativeApi api(f,imports());sock::Conversion text{4,std::wstring(L"abc")};const auto result=api.string_to_address(text,family,128);
        require(result.result==status,"raw WSAStringToAddressW status preserved");
        for(unsigned i=0;i<16;++i)require(result.bytes[i]==(status==-1 || (family==AF_INET && i>=4)?0:f.address[i]),"native adapter extracts only SDK address bytes on nonminus1 success");
        require(result.scope==(status!=-1 && family==AF_INET6?f.scope:0),"native adapter extracts only IPv6 SDK scope");
        require(f.trace==std::vector<std::string>{"parse"},"native address import does not read last error prematurely");
    }
    for(const int size:{0,15,129}){Fixture f;f.parse_size=size;sock::NativeApi api(f,imports());sock::Conversion text{4,std::wstring(L"abc")};unsupported([&]{(void)api.string_to_address(text,AF_INET,128);},"successful parser output must cover SDK sockaddr and stay within128bytes");}
    {Fixture f;f.family=AF_INET6;f.parse_size=27;sock::NativeApi api(f,imports());sock::Conversion text{4,std::wstring(L"abc")};unsupported([&]{(void)api.string_to_address(text,AF_INET6,128);},"short IPv6 sockaddr success rejected");}
    {Fixture f;sock::NativeApi api(f,imports());sock::Conversion text{4,std::wstring(L"abc")};unsupported([&]{(void)api.string_to_address(text,1,128);},"unmodeled native family rejected");unsupported([&]{(void)api.string_to_address(text,2,127);},"wrong initial sockaddr size rejected");require(f.trace.empty(),"invalid native address preconditions invoke no imports");}
    {Fixture f;f.parse_result=-1;f.category_last=222;sock::NativeApi api(f,imports());std::array<std::uint8_t,16> out;out.fill(0x66);sock::Error error;require(sock::address_028690(AF_INET,"abc",out,nullptr,error,api)==-1 && error.value==222,"actual core captures mandatory provider side effect before native last-error read");require(f.trace==std::vector<std::string>{"set","convert","parse","category","get"},"composed address import order");require(out[0]==0x66 && out[15]==0x66,"composed failure leaves caller bytes untouched");}
}
void sending(){
    {Fixture f;char first[]="ab",second[]="c";sock::Buffer buffers[]={{2,first},{1,second}};f.expected_buffers=buffers;f.expected_count=2;f.expected_flags=0x80000001U;f.send_result=-1;sock::NativeApi api(f,imports());const auto result=api.wsa_send(f.expected_socket,buffers,f.expected_flags);require(result.result==-1 && result.transferred==0x80000001U,"native send preserves raw status and all DWORD byte bits");require(f.trace==std::vector<std::string>{"send"} && f.last==10035,"native send performs no cleanup or last-error reads after import");}
    {Fixture f;f.expected_buffers=nullptr;f.expected_count=0;f.bytes=0;sock::NativeApi api(f,imports());const auto result=api.wsa_send(f.expected_socket,{},0);require(result.result==0 && result.transferred==0,"native send forwards empty actual SDK table");}
    for(int nfds:{0,-1,2147483647})for(int status:{-1,0,1}){Fixture f;f.expected_nfds=nfds;f.select_result=status;sock::NativeApi api(f,imports());require(api.select_write(f.expected_socket,nfds)==status,"native select preserves raw negative/zero/positive status");require(f.trace==std::vector<std::string>{"select"},"native select does not read last error or call category");}
    {Fixture f;char byte='x';sock::Buffer buffer{1,&byte};f.expected_buffers=&buffer;f.expected_count=1;f.bytes=1;f.category_last=64;sock::NativeApi api(f,imports());sock::Error error;require(sock::send_02a200(f.expected_socket,0,{&buffer,1},0,false,error,api)==1 && error.value==0,"actual core composes injected native send, remap and success clear");require(f.trace==std::vector<std::string>{"send","category","get","category"},"composed send captures category before last error even after success");}
}
struct MessageFixture;
MessageFixture* message_active=nullptr;
struct MessageFixture {
 std::vector<char> bytes,expected_bytes;std::vector<std::string> trace;
 DWORD code=0,count=0,format_error=0x2313,free_error=0x3456;bool supplies_buffer=true,throw_format=false,throw_free=false;HLOCAL free_result=nullptr;unsigned formats=0,frees=0;
 MessageFixture(std::string input,DWORD returned,std::string trimmed,bool supply=true):bytes(input.begin(),input.end()),expected_bytes(trimmed.begin(),trimmed.end()),count(returned),supplies_buffer(supply){require(!message_active,"system message fixtures are sequential");bytes.push_back(0);expected_bytes.push_back(0);require(bytes.size()==expected_bytes.size()&&(!count||count<bytes.size()),"injected message initializes the complete counted trim range and C terminator");message_active=this;}
 ~MessageFixture(){message_active=nullptr;}
};
DWORD WINAPI message_format(DWORD flags,LPCVOID source,DWORD code,DWORD language,LPSTR output,DWORD size,va_list* args){
 auto& f=*message_active;f.trace.push_back("format");++f.formats;
 require(flags==0x1300&&!source&&code==f.code&&language==0x400&&output&&!size&&!args,"Actual020950 forwards exact FormatMessageA flags/source/DWORDcode/language/size/arguments");
 auto** buffer=reinterpret_cast<char**>(output);require(!*buffer,"Actual020950 supplies an initially NULL allocated-buffer pointer");*buffer=f.supplies_buffer?f.bytes.data():nullptr;::SetLastError(f.format_error);if(f.throw_format)throw std::runtime_error("Controlled unknown FormatMessage effect");return f.count;
}
HLOCAL WINAPI message_free(HLOCAL raw){
 auto& f=*message_active;f.trace.push_back("free");++f.frees;
 require(raw==(f.supplies_buffer?static_cast<HLOCAL>(f.bytes.data()):nullptr),"Actual020950 LocalFree receives the exact saved SDK pointer, including NULL failure");
 require(f.bytes==f.expected_bytes,"Counted trim mutates exactly the native C buffer positions before LocalFree");
 ::SetLastError(f.free_error);if(f.throw_free)throw std::runtime_error("Controlled unknown LocalFree effect");std::fill(f.bytes.begin(),f.bytes.end(),'#');return f.free_result;
}
void system_messages(){
 const sock::SystemMessageImports passive;require(passive.format_message==&::FormatMessageA&&passive.local_free==&::LocalFree,"SystemMessageImports default to the actual SDK entries without invoking them");
 struct Case {std::string input,mutated,expected;DWORD count;bool supplies=true;};
 const std::vector<Case> cases{
  {"plain","plain","plain",5},
  {"ok\r\n",std::string("ok\0\0",4),"ok",4},
  {"ok\n\r",std::string("ok\n\0",4),"ok\n",4},
  {"ok\n\n",std::string("ok\n\0",4),"ok\n",4},
  {"ok\r\r",std::string("ok\r\0",4),"ok\r",4},
  {"\n",std::string(1,'\0'),"asio.system error",1},
  {"\r",std::string(1,'\0'),"asio.system error",1},
  {"\r\n",std::string(2,'\0'),"asio.system error",2},
  {"untouched","untouched","asio.system error",0},
  {"unused","unused","asio.system error",0,false},
  {"abTAIL","abTAIL","abTAIL",2},
  {"A\nTAIL",std::string("A\0TAIL",6),"A",2},
  {std::string("A\0B\n",4),std::string("A\0B\0",4),"A",4},
  {std::string("\x80\xff\r\n",4),std::string("\x80\xff\0\0",4),std::string("\x80\xff",2),4},
  {std::string("\0tail",5),std::string("\0tail",5),"",5},
  {std::string(5000,'L')+"\r\n",std::string(5000,'L')+std::string(2,'\0'),std::string(5000,'L'),5002}
 };
 for(std::size_t i=0;i<cases.size();++i){const auto& c=cases[i];MessageFixture f(c.input,c.count,c.mutated,c.supplies);const auto code=i&1?std::numeric_limits<std::int32_t>::min():-1;f.code=static_cast<DWORD>(code);if(i==0)f.free_result=reinterpret_cast<HLOCAL>(std::uintptr_t{0x1234});
  const sock::SystemMessageImports injected{message_format,message_free};sock::SystemMessageState state;require(!f.formats&&!f.frees&&!state.buffer&&!state.entered&&!state.release_entered,"Injected imports and persistent message state construct passively");
  auto result=sock::system_message_020950(code,state,injected);
  require(result==c.expected&&f.formats==1&&f.frees==1&&f.trace==std::vector<std::string>{"format","free"},"Actual020950 preserves native trim/fallback/Cstrlen semantics and constructs text before one LocalFree");
  require(!state.buffer&&!state.entered&&!state.release_entered,"Normal free return, including nonNULL return value, clears the source-owned message state");
  require(result==c.expected,"Returned owned message remains intact after LocalFree callback overwrites all SDK bytes");
 }
 {MessageFixture f("state",5,"state");f.code=77;sock::SystemMessageState state;unsigned probes=0;
  const std::function<void()> checkpoint=[&]{++probes;::SetLastError(0x9999);};
  const auto value=sock::system_message_020950(77,state,{message_format,message_free},checkpoint);const auto sdk_error=::GetLastError();
  require(value=="state"&&probes&&sdk_error==f.free_error,"Nonmutating metadata checkpoints preserve the live LocalFree SDK LastError even when the test checkpoint clobbers it");
 }
 for(const bool free_phase:{false,true}){MessageFixture f("hold",4,"hold");f.code=77;f.throw_format=!free_phase;f.throw_free=free_phase;sock::SystemMessageState state;bool thrown=false;
  try{(void)sock::system_message_020950(77,state,{message_format,message_free});}catch(const std::runtime_error&){thrown=true;}
  require(thrown&&state.entered&&state.buffer==f.bytes.data()&&state.release_entered==free_phase&&f.formats==1&&f.frees==unsigned(free_phase),"Unknown injected Format/Free effect preserves exact entered pointer/release phase without automatic cleanup");
  bool rejected=false;try{(void)sock::system_message_020950(77,state,{message_format,message_free});}catch(const std::logic_error&){rejected=true;}
  require(rejected&&f.formats==1&&f.frees==unsigned(free_phase)&&state.buffer==f.bytes.data(),"Retained unknown SDK prefix rejects reuse without replaying FormatMessage or LocalFree");
 }
 {MessageFixture f("done",4,"done");f.code=77;sock::SystemMessageState state;unsigned probes=0;bool thrown=false;
  const std::function<void()> checkpoint=[&]{if(++probes==4)throw std::runtime_error("Controlled post-LocalFree checkpoint failure");};
  try{(void)sock::system_message_020950(77,state,{message_format,message_free},checkpoint);}catch(const std::runtime_error&){thrown=true;}
  require(thrown&&probes==4&&f.formats==1&&f.frees==1&&state.entered&&state.release_entered&&state.buffer==f.bytes.data(),"Failed final checkpoint retains the known-entered LocalFree phase and its passive pointer instead of clearing retry protection");
  bool rejected=false;try{(void)sock::system_message_020950(77,state,{message_format,message_free},checkpoint);}catch(const std::logic_error&){rejected=true;}
  require(rejected&&probes==4&&f.formats==1&&f.frees==1,"Post-free checkpoint failure rejects reuse without dereferencing or freeing the saved pointer again");
 }
}
HLOCAL WINAPI alternate_message_free(HLOCAL raw){return message_free(raw);}
void native_system_error_values(){
 const sock::SystemMessageImports imports{message_format,message_free};
 const auto& category=[&]() -> const std::error_category& {MessageFixture f("passive",7,"passive");const auto& value=sock::system_error_category_028620(imports);require(!f.formats&&!f.frees,"First native category factory lookup invokes neither configured SDK method");return value;}();
 require(&category==&sock::system_error_category_028620(imports)&&std::string_view(category.name())=="asio.system","Immutable typed import pair selects one module-lifetime native system category");
 require(&category!=&sock::system_error_category_028620({message_format,alternate_message_free})&&&category!=&std::system_category(),"Different typed import pair has distinct stable category and does not reuse compiler system category");
 const auto& actual=sock::system_error_category_028620();require(&actual==&sock::system_error_category_028620(),"Default SDK pair has a stable passive category identity");
 for(const sock::SystemMessageImports invalid:std::array<sock::SystemMessageImports,2>{{{nullptr,message_free},{message_format,nullptr}}}){bool rejected=false;try{(void)sock::system_error_category_028620(invalid);}catch(const std::invalid_argument&){rejected=true;}require(rejected,"Native category factory rejects incomplete immutable SDK pairs");}
 for(const int value:{2,-1,std::numeric_limits<int>::min()}){const auto condition=category.default_error_condition(value);const std::error_code code(value,category);
  require(condition.value()==value&&&condition.category()==&category&&category.equivalent(value,condition)&&category.equivalent(code,value),"Native default condition preserves raw code and its own category identity in both equivalence directions");
  require(!category.equivalent(value,std::error_condition(value,std::generic_category()))&&!category.equivalent(std::error_code(value,std::system_category()),value)&&!category.equivalent(value,std::error_condition(value==2?3:2,category)),"Native conditions do not normalize Win32 values into compiler generic/system categories or unequal codes");
 }
 struct Case {std::string input,mutated,text;DWORD count;bool supplied=true;};
 const std::vector<Case> cases{{"native \r\n",std::string("native \0\0",9),"native ",9},{"unused","unused","asio.system error",0,false},{std::string("A\0B\n",4),std::string("A\0B\0",4),"A",4},{"tail\n\r",std::string("tail\n\0",6),"tail\n",6}};
 for(const auto& c:cases){MessageFixture f(c.input,c.count,c.mutated,c.supplied);f.code=77;
  const auto& same=sock::system_error_category_028620(imports);require(!f.formats&&!f.frees&&&same==&category,"Category lookup invokes neither configured SDK import");
  const std::error_code code(77,category);std::system_error original(code,"pqcs");const auto expected="pqcs: "+c.text;
  require(std::string(original.what())==expected&&original.code()==code&&f.formats==1&&f.frees==1,"Plain system_error invokes native020950 exactly once and owns exact pqcs text including trailing space/fallback/Cstrlen");
  const std::system_error copied=original;const std::runtime_error sliced=original;const std::error_code retained=original.code();
  require(std::string(copied.what())==expected&&std::string(sliced.what())==expected&&retained.value()==77&&&retained.category()==&category&&f.formats==1&&f.frees==1,"Exception copy, runtime_error value slicing and error_code reference preserve native text/category without a second SDK formatting call");
 }
 {MessageFixture f("error",5,"error");f.code=77;const auto message=category.message(77);const auto error=::GetLastError();require(message=="error"&&error==f.free_error&&f.formats==1&&f.frees==1,"Category attempt allocation/publication/retirement preserves the actual LocalFree LastError after success");}
}
struct ConcurrentMessage {
 std::array<char,8> bytes{};unsigned formats=0,frees=0;bool fail=false;
};
// Static callback state remains valid for passive failed attempts retained by
// the module category. Each concurrent call owns one distinct record.
std::array<ConcurrentMessage,3> concurrent_messages;
std::atomic<unsigned> message_arrivals{0};bool message_rendezvous=false;
DWORD WINAPI concurrent_format(DWORD flags,LPCVOID source,DWORD code,DWORD language,LPSTR output,DWORD size,va_list* args){
 if(flags!=0x1300||source||language!=0x400||!output||size||args||code<710||code>712)throw std::logic_error("Concurrent SDK signature mismatch");
 auto& f=concurrent_messages[code-710];auto** pointer=reinterpret_cast<char**>(output);if(*pointer)throw std::logic_error("Concurrent attempt pointer was shared");++f.formats;*pointer=f.bytes.data();
 if(message_rendezvous){message_arrivals.fetch_add(1);const auto deadline=::GetTickCount64()+5000;while(message_arrivals.load()!=2){if(::GetTickCount64()>deadline)throw std::runtime_error("Concurrent formatter entry timed out");std::this_thread::yield();}}
 if(f.fail)throw std::runtime_error("Controlled independent FormatMessage failure");return 5;
}
HLOCAL WINAPI concurrent_free(HLOCAL pointer){
 for(auto& f:concurrent_messages)if(pointer==f.bytes.data()){++f.frees;if(f.bytes[3]||f.bytes[4])throw std::logic_error("Concurrent native trim missing");std::fill(f.bytes.begin(),f.bytes.end(),'#');return nullptr;}
 throw std::logic_error("Concurrent attempt freed another SDK pointer");
}
void independent_system_error_attempts(){
 for(std::size_t i=0;i<concurrent_messages.size();++i){auto& f=concurrent_messages[i];f={};f.bytes={'o','k',static_cast<char>('0'+i),'\r','\n',0,0,0};}
 const auto& category=sock::system_error_category_028620({concurrent_format,concurrent_free});
 require(!concurrent_messages[0].formats&&!concurrent_messages[1].formats,"Concurrent category construction is passive");
 message_arrivals=0;message_rendezvous=true;std::array<std::string,2> results;std::array<std::exception_ptr,2> failures;
 std::thread first([&]{try{results[0]=category.message(710);}catch(...){failures[0]=std::current_exception();}});
 std::thread second([&]{try{results[1]=category.message(711);}catch(...){failures[1]=std::current_exception();}});
 first.join();second.join();message_rendezvous=false;
 require(!failures[0]&&!failures[1]&&results[0]=="ok0"&&results[1]=="ok1"&&concurrent_messages[0].formats==1&&concurrent_messages[1].formats==1&&concurrent_messages[0].frees==1&&concurrent_messages[1].frees==1,"Simultaneous message calls reach both SDK bodies with independent pointers and each copy/free exactly once");
 concurrent_messages[2].fail=true;bool thrown=false;try{(void)category.message(712);}catch(const std::runtime_error&){thrown=true;}
 require(thrown&&concurrent_messages[2].formats==1&&!concurrent_messages[2].frees,"Failed independent attempt retains its unknown prefix without automatic SDK cleanup");
 auto& next=concurrent_messages[0];next.bytes={'o','k','0','\r','\n',0,0,0};
 require(category.message(710)=="ok0"&&next.formats==2&&next.frees==2&&concurrent_messages[2].formats==1&&!concurrent_messages[2].frees,"A failed retained attempt neither poisons a different message nor retries its own imported effect");
}
}
int main(){try{conversion();addresses();sending();system_messages();native_system_error_values();independent_system_error_attempts();std::cout<<"PASS: "<<checks<<" socket native checks; native020950 text and stable independent native system-error category values\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
