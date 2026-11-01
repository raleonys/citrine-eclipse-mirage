// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_native.hpp"
#include <cstring>
#include <iostream>
#include <vector>

namespace sock=preloader_l::socket;
namespace srv=preloader_l::server;
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
}
int main(){try{conversion();addresses();sending();std::cout<<"PASS: "<<checks<<" socket native checks; injected SDK imports only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
