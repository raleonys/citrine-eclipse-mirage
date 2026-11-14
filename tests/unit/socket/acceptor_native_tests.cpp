// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_acceptor_native.hpp"
#include <bit>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

namespace sock=laureondo::socket;
namespace srv=laureondo::server;
namespace {
unsigned checks=0;
void require(bool condition,const char* why){++checks;if(!condition)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected native acceptor dependency");}
struct Fixture;Fixture* active=nullptr;
struct Fixture {
    std::vector<std::string> trace;
    int family=2,status=0,level=65535,option=4,value=1,size=16,backlog=std::numeric_limits<int>::max();
    sock::Socket socket=0xabcdef0123456789ULL,created=0xfedcba9876543210ULL;
    std::uintptr_t port=0x123456789abcdef0ULL,attached=0x9988776655443322ULL;
    DWORD last=0x80000001U;std::uint32_t ioctl_command=0x8004667eU,ioctl_input=0,ioctl_output=0xfedcba98U;
    const std::byte* address=nullptr;
    Fixture(){require(!active,"native fixtures are sequential");active=this;}
    ~Fixture(){active=nullptr;}
};
SOCKET WSAAPI create(int family,int type,int protocol,LPWSAPROTOCOL_INFOW info,GROUP group,DWORD flags){
    auto& f=*active;f.trace.push_back("create");require(family==f.family && type==1 && protocol==6,"WSASocketW protocol constants and raw selected family");require(info==nullptr && group==0 && flags==1,"WSASocketW null protocol/group0/overlapped1");return static_cast<SOCKET>(f.created);
}
HANDLE WINAPI attach(HANDLE file,HANDLE port,ULONG_PTR key,DWORD threads){auto& f=*active;f.trace.push_back("attach");require(reinterpret_cast<std::uintptr_t>(file)==f.socket && reinterpret_cast<std::uintptr_t>(port)==f.port,"IOCP preserves full-width socket and completion port");require(key==0 && threads==0,"IOCP association uses zero completion key and concurrency");return reinterpret_cast<HANDLE>(f.attached);}
DWORD WINAPI last(){active->trace.push_back("win_error");return active->last;}
int WSAAPI option(SOCKET socket,int level,int name,const char* value,int size){auto& f=*active;f.trace.push_back("option");require(socket==f.socket && level==f.level && name==f.option,"setsockopt exact full socket, level and option");require(value!=nullptr && size==4,"setsockopt uses initialized signed DWORD value storage");int copied=0;std::memcpy(&copied,value,4);require(copied==f.value,"setsockopt value bits preserved");return f.status;}
int WSAAPI bind(SOCKET socket,const sockaddr* address,int size){auto& f=*active;f.trace.push_back("bind");require(socket==f.socket && reinterpret_cast<const std::byte*>(address)==f.address && size==f.size,"bind forwards original bounded SDK address pointer and length");return f.status;}
int WSAAPI listen(SOCKET socket,int backlog){auto& f=*active;f.trace.push_back("listen");require(socket==f.socket && backlog==f.backlog,"listen forwards full socket and backlog unchanged");return f.status;}
int WSAAPI close(SOCKET socket){auto& f=*active;f.trace.push_back("close");require(socket==f.socket,"closesocket preserves complete SOCKET bits");return f.status;}
int WSAAPI ioctl(SOCKET socket,long command,u_long* value){auto& f=*active;f.trace.push_back("ioctl");require(socket==f.socket && std::bit_cast<std::uint32_t>(command)==f.ioctl_command,"ioctlsocket preserves signed command bit pattern and socket");require(value!=nullptr && *value==f.ioctl_input,"ioctlsocket receives initialized SDK u_long");*value=f.ioctl_output;return f.status;}
sock::AcceptorImports imports(){sock::AcceptorImports value;value.create=create;value.attach=attach;value.last=last;value.option=option;value.bind=bind;value.listen=listen;value.close=close;value.ioctl=ioctl;return value;}
template<class F>void unsupported(F&& call,const char* why){bool caught=false;try{call();}catch(const sock::UnsupportedInput&){caught=true;}require(caught,why);}
void typed_calls(){
    {Fixture f;sock::NativeAcceptorApi api;require(f.trace.empty(),"default import construction makes no actual operating-system call");}
    for(int family:{0,2,23,65535})for(auto result:{sock::Socket{0},sock::Socket{0xfedcba9876543210ULL},sock::invalid_socket}){Fixture f;f.family=family;f.created=result;sock::NativeAcceptorApi api(imports());require(api.create_socket(family)==result,"socket creation preserves full return including INVALID_SOCKET");require(f.trace==std::vector<std::string>{"create"},"socket creation reads no errors and performs no cleanup");}
    for(auto result:{std::uintptr_t{0},std::uintptr_t{0x9988776655443322ULL}}){Fixture f;f.attached=result;sock::NativeAcceptorApi api(imports());require(api.attach_completion_port(f.socket,f.port)==result,"IOCP preserves raw null/full-width return");require(f.trace==std::vector<std::string>{"attach"},"IOCP adapter does not get last error or call category");}
    {Fixture f;sock::NativeAcceptorApi api(imports());require(api.get_win32_error()==f.last && f.trace==std::vector<std::string>{"win_error"},"GetLastError preserves all DWORD bits");}
    for(int status:{-1,0,7})for(const auto values:std::vector<std::array<int,3>>{{41,27,0},{65535,4,1},{-1,-2,std::numeric_limits<int>::min()}}){Fixture f;f.status=status;f.level=values[0];f.option=values[1];f.value=values[2];sock::NativeAcceptorApi api(imports());require(api.set_option(f.socket,f.level,f.option,f.value)==status,"setsockopt raw result unchanged");require(f.trace==std::vector<std::string>{"option"},"option adapter adds no error getter");}
    alignas(sockaddr_storage) std::array<std::byte,128> bytes{};
    for(int size:{0,1,16,28,128})for(int status:{-1,0,3}){Fixture f;f.address=bytes.data();f.size=size;f.status=status;sock::NativeAcceptorApi api(imports());require(api.bind_socket(f.socket,bytes,size)==status,"bind raw status and all bounded lengths preserved");require(f.trace==std::vector<std::string>{"bind"},"bind adapter does not get error or copy address");}
    {Fixture f;sock::NativeAcceptorApi api(imports());unsupported([&]{(void)api.bind_socket(f.socket,bytes,-1);},"negative bind length excluded from bounded domain");unsupported([&]{(void)api.bind_socket(f.socket,bytes,129);},"oversized bind length excluded from bounded domain");require(f.trace.empty(),"invalid bounded bind domain invokes no SDK import");}
    for(int status:{-1,0,5})for(int backlog:{0,-1,std::numeric_limits<int>::max()}){Fixture f;f.status=status;f.backlog=backlog;sock::NativeAcceptorApi api(imports());require(api.listen_socket(f.socket,backlog)==status,"listen raw status preserved");require(f.trace==std::vector<std::string>{"listen"},"listen wrapper samples no errors");}
    for(int status:{-1,0,5}){Fixture f;f.status=status;sock::NativeAcceptorApi api(imports());require(api.close_socket(f.socket)==status && f.trace==std::vector<std::string>{"close"},"close raw return with no ownership mutation or error query");}
    for(int status:{-1,0,5})for(auto command:{0U,0x8004667eU,0xffffffffU}){Fixture f;f.status=status;f.ioctl_command=command;f.ioctl_input=0x98765432U;std::uint32_t value=f.ioctl_input;sock::NativeAcceptorApi api(imports());require(api.set_nonblocking(f.socket,command,value)==status && value==f.ioctl_output,"ioctl copies SDK mutable DWORD result and preserves raw status");require(f.trace==std::vector<std::string>{"ioctl"},"ioctl adapter samples no error/category");}
}
struct ErrorApi final:sock::Api {
    int error=0;
    void set_last_error(std::int32_t)override{unused();}
    sock::Category system_category_028620()override{active->trace.push_back("category");return {77,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{active->trace.push_back("wsa_error");return error;}
    sock::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    sock::AddressCall string_to_address(sock::Conversion&,std::int32_t,std::int32_t)override{unused();}
    sock::SendCall wsa_send(sock::Socket,std::span<sock::Buffer>,std::uint32_t)override{unused();}
    std::int32_t select_write(sock::Socket,std::int32_t)override{unused();}
};
struct Runtime final:sock::AcceptorRuntime {
    sock::AcceptorService service;
    explicit Runtime(std::uintptr_t port):service{port}{}
    void initialize_0211b0(sock::AcceptorState& state)override{active->trace.push_back("initialize");state.socket=sock::invalid_socket;state.service=&service;}
    void reset_cancellation_0237f0(sock::AcceptorState& state)override{active->trace.push_back("reset");require(state.socket==active->created && state.socket_state==0x10,"composed native path transfers socket/state before ownership reset");}
    sock::Category miscellaneous_category_0285b0()override{unused();}
    [[noreturn]]void raise_028240(sock::Error,std::string_view)override{unused();}
};
void composition(){
    for(const int family:{2,23}){Fixture f;ErrorApi errors;Runtime runtime(f.port);f.family=family;f.socket=f.created;f.level=41;f.option=27;f.value=0;f.size=family==2?16:28;alignas(sockaddr_storage) std::array<std::byte,128> address{};const auto word=static_cast<std::uint16_t>(family);std::memcpy(address.data(),&word,2);f.address=address.data();sock::NativeAcceptorApi api(imports());sock::AcceptorState state;auto& result=sock::acceptor_020e20(state,address,false,errors,api,runtime);require(&result==&state && state.socket==f.created && state.protocol_40==family,"real recovered constructor composes typed SDK imports");std::vector<std::string> expected{"initialize","create"};if(family!=2)expected.push_back("option");expected.insert(expected.end(),{"attach","reset","bind","listen"});require(f.trace==expected,"composed native constructor exact import/provider order");}
    {Fixture f;ErrorApi errors;f.status=0;sock::NativeAcceptorApi api(imports());sock::Socket slot=f.socket;sock::close_025310(slot,errors,api);require(slot==f.socket && f.trace==std::vector<std::string>{"close"},"real close helper composes SDK wrapper without invalidating slot");}
}
}
int main(){try{typed_calls();composition();std::cout<<"PASS: "<<checks<<" acceptor native checks; injected SDK imports only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
