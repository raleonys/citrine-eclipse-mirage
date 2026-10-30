#include "preloader_l/net/transport_operations.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace sock=preloader_l::socket_recovered;
namespace srv=preloader_l::server_recovered;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected socket test dependency");}
struct SendStep {int result;std::uint32_t bytes;int error;};
struct SelectStep {int result,error;};
struct Api final:sock::Api {
    std::vector<std::string> trace;int last=0,conversion_result=1;
    sock::AddressCall parsed;std::string text;int capacity=0,family=0,initial_size=0;
    std::deque<SendStep> sends;std::deque<SelectStep> selects;std::deque<sock::Category> categories;
    sock::Category category{11,srv::ErrorCategory::system};std::optional<int> category_last;
    std::vector<sock::Socket> sockets;std::vector<int> nfds;std::vector<std::uint32_t> flags;
    sock::Buffer* expected_buffers=nullptr;std::size_t expected_count=0;std::string written;
    void set_last_error(std::int32_t value)override{trace.push_back("set");require(value==0,"address begins by clearing WSA error");last=value;}
    sock::Category system_category_028620()override{trace.push_back("category");if(category_last)last=*category_last;if(categories.empty())return category;const auto c=categories.front();categories.pop_front();return c;}
    std::int32_t get_last_error()override{trace.push_back("get");return last;}
    sock::Conversion convert_acp(const char* raw,std::int32_t count)override{trace.push_back("convert");text=raw;capacity=count;return {conversion_result,std::nullopt};}
    sock::AddressCall string_to_address(sock::Conversion& wide,std::int32_t f,std::int32_t size)override{trace.push_back("parse");require(wide.result==conversion_result,"conversion raw result reaches parse even zero");family=f;initial_size=size;last=address_error;return parsed;}
    int address_error=0;
    sock::SendCall wsa_send(sock::Socket socket,std::span<sock::Buffer> buffers,std::uint32_t value)override{
        trace.push_back("send");require(!sends.empty(),"finite scripted send attempts");
        if(expected_buffers)require(buffers.data()==expected_buffers && buffers.size()==expected_count,"WSABUF table identity/count unchanged through retries");
        sockets.push_back(socket);flags.push_back(value);const auto step=sends.front();sends.pop_front();last=step.error;
        if(step.result==0 && step.bytes<0x80000000U && !buffers.empty() && step.bytes<=buffers[0].len)
            written.append(buffers[0].buf,step.bytes);
        return {step.result,step.bytes};
    }
    std::int32_t select_write(sock::Socket socket,std::int32_t count)override{
        trace.push_back("select");require(!selects.empty(),"finite scripted select attempts");sockets.push_back(socket);nfds.push_back(count);const auto step=selects.front();selects.pop_front();last=step.error;return step.result;
    }
};
void addresses(){
    for(int family:{0,1,3,65535}){
        Api api;sock::Error error{17,{99,srv::ErrorCategory::other}};std::array<std::uint8_t,16> out;out.fill(0xa5);const auto before=out;std::uint32_t scope=88;
        require(sock::address_028690(family,"ignored",out,&scope,error,api)==-1,"unsupported family returns signed logical minus1");
        require(error.value==10047 && error.category.name_identity==11 && out==before && scope==88,"unsupported family error and untouched outputs");
        require(api.trace==std::vector<std::string>{"set","category"},"unsupported family does not convert or parse");
    }
    for(const int result:{-2,0,1,7})for(const int family:{2,23}){
        Api api;api.conversion_result=0;api.parsed.result=result;api.parsed.scope=0x12345678;api.address_error=777;
        for(unsigned i=0;i<16;++i)api.parsed.bytes[i]=static_cast<std::uint8_t>(i+1);
        sock::Error error;std::array<std::uint8_t,16> out;out.fill(0xa5);std::uint32_t scope=99;
        const std::string input("127.0.0.1\0tail",14);
        require(sock::address_028690(family,input,out,&scope,error,api)==1,"any nonminus1 WSA parse result is logical success");
        require(api.text=="127.0.0.1" && api.capacity==10 && api.family==family && api.initial_size==128,"CStr input, strlen+1 capacity and128byte sockaddr");
        require(api.trace==std::vector<std::string>{"set","convert","parse","category","get"},"conversion return ignored; category occurs before last-error read");
        require(error.value==0 && error.category.name_identity==11,"parse success clears stale WSA error");
        for(unsigned i=0;i<16;++i)require(out[i]==(family==2 && i>=4?0xa5:i+1),"IPv4 writes only4 bytes, IPv6 writes16");
        require(scope==(family==23?0x12345678U:99U),"only IPv6 writes scope");
    }
    {Api api;api.parsed.result=0;api.parsed.scope=9;std::array<std::uint8_t,16> out{};sock::Error e;require(sock::address_028690(23,"::1",out,nullptr,e,api)==1,"IPv6 optional null scope accepted");}
    for(int family:{2,23})for(int last:{0,5,10049}){
        Api api;api.address_error=last;std::array<std::uint8_t,16> out;out.fill(0xc3);const auto old=out;std::uint32_t scope=77;sock::Error error;
        require(sock::address_028690(family,"bad",out,&scope,error,api)==-1,"parse failure returned");
        require(out==old && scope==77 && error.value==(last?last:10022),"ordinary failure keeps outputs and maps zero error to10022");
        require(api.trace.size()==static_cast<std::size_t>(last?5:6),"only zero-error failure calls category again");
    }
    for(int last:{0,5,10049}){
        Api api;api.address_error=last;std::array<std::uint8_t,16> out;out.fill(0x55);std::uint32_t scope=7;sock::Error error;
        require(sock::address_028690(2,"255.255.255.255",out,&scope,error,api)==-1 && error.value==10022,"broadcast failure preserves native forced10022 fallthrough");
        for(unsigned i=0;i<16;++i)require(out[i]==(i<4?0xff:0x55),"broadcast failure writes4FF only");
        require(scope==7 && api.trace==std::vector<std::string>{"set","convert","parse","category","get","category"},"broadcast always repeats category after error reset");
    }
    {Api api;api.address_error=44;api.category_last=222;std::array<std::uint8_t,16> out{};sock::Error error;(void)sock::address_028690(23,"bad",out,nullptr,error,api);require(error.value==222,"category factory LastError side effects captured after call");}
}
void sends(){
    char data[]="binary";sock::Buffer buffer{6,data};auto span=std::span<sock::Buffer>(&buffer,1);
    {Api api;sock::Error e{77,{999,srv::ErrorCategory::other}};require(sock::send_02a200(std::numeric_limits<sock::Socket>::max(),0,span,9,false,e,api)==0 && e.value==10009,"invalid socket produces10009");require(api.trace==std::vector<std::string>{"category"},"invalid socket skips send/select/getLastError");}
    {Api api;sock::Error e{77,{999,srv::ErrorCategory::other}};require(sock::send_02a200(4,0x10,span,9,true,e,api)==0 && e.value==0 && e.category.name_identity==999 && e.category.logical==srv::ErrorCategory::other,"all-empty fast path only clears value");require(api.trace.empty(),"all-empty state0x10 performs no API calls");}
    for(const int stale:{0,64,1234,10035}){
        Api api;api.sends={{0,3,stale}};api.expected_buffers=&buffer;api.expected_count=1;sock::Error e;
        require(sock::send_02a200(0x1234567800000004ULL,0x10,span,0x80000001U,false,e,api)==3 && e.value==0,"successful send count and cleared stale error");
        require(api.sockets.front()==0x1234567800000004ULL && api.flags.front()==0x80000001U,"full socket and raw send flags preserved");
        const std::vector<std::string> expected=stale==64 || stale==1234?std::vector<std::string>{"send","category","get","category"}:std::vector<std::string>{"send","category","get"};
        require(api.trace==expected,"send captures category/last even success, remaps before success clear");
    }
    for(const auto pair:std::vector<std::pair<int,int>>{{64,10054},{1234,10061},{77,77},{10035,10035}}){
        Api api;api.sends={{-1,91,pair.first}};sock::Error e;require(sock::send_02a200(4,1,span,0,false,e,api)==0 && e.value==pair.second,"nonblocking failure remap and zero bytes");require(api.nfds.empty(),"statebit1 prevents wait/retry");
    }
    for(const auto bytes:{0x80000000U,0xffffffffU}){
        Api api;api.sends={{0,bytes,5}};sock::Error e;require(sock::send_02a200(4,0,span,0,false,e,api)==0 && e.value==0,"transferred DWORD is signed before positive return");
        require(api.trace==std::vector<std::string>{"send","category","get","category","category"},"negative signed byte count falls through both category checks");
    }
    {Api api;api.sends={{-1,0,10035},{0,2,888}};api.selects={{0,999}};api.expected_buffers=&buffer;api.expected_count=1;sock::Error e;
        require(sock::send_02a200(0x123456787fffffffULL,0,span,3,false,e,api)==2 && e.value==0,"select0 retries without synthetic timeout");
        require(api.nfds==std::vector<int>{std::numeric_limits<int>::min()},"select nfds is wrapped low32socket+1");
        require(api.trace==std::vector<std::string>{"send","category","get","category","select","send","category","get"},"10035 composite match uses first category check only");
    }
    {Api api;api.sends={{-1,0,10035},{0,1,0}};api.selects={{1,0}};sock::Error e;
        require(sock::send_02a200(0x12345678ffffffffULL,0,span,0,false,e,api)==1 && api.nfds==std::vector<int>{0},"low32 allones socket plus1 wraps select nfds to0 without truncating socket");
        require(api.sockets==std::vector<sock::Socket>(3,0x12345678ffffffffULL),"send/select/retry preserve full socket despite wrapped nfds");
    }
    {Api api;api.sends={{-1,0,1237}};api.categories={{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::other},{33,srv::ErrorCategory::other}};api.selects={{-1,64}};sock::Error e;
        require(sock::send_02a200(4,0,span,0,false,e,api)==0 && e.value==64 && e.category.name_identity==33,"select failure reads raw error without WSASend remap");
        require(api.trace==std::vector<std::string>{"send","category","get","category","category","select","category","get"},"1237 uses second category name comparison and category-before-error after select");
    }
    {Api api;api.sends={{-1,0,10035}};api.categories={{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::system}};sock::Error e;
        require(sock::send_02a200(4,0,span,0,false,e,api)==0 && api.nfds.empty(),"10035 cannot match only second category branch");
    }
    {Api api;api.sends={{-1,0,10035},{0,1,0}};api.categories={{11,srv::ErrorCategory::other},{11,srv::ErrorCategory::system},{11,srv::ErrorCategory::system}};api.selects={{1,0}};sock::Error e;
        require(sock::send_02a200(4,0,span,0,false,e,api)==1,"retry compares name token rather than logical category enum");
    }
    {Api api;api.sends={{0,0,0}};sock::Error e;require(sock::send_02a200(4,0,{},8,true,e,api)==0 && api.trace.front()=="send","all-empty without statebit0x10 still sends zero buffers");}
}
struct Backend final:sock::AddressBackend {
    using AddressBackend::AddressBackend;
    void initialize_context_0241b0()override{unused();}
    void construct_acceptor_020e20(const srv::Endpoint&,bool,int)override{unused();}
    void initialize_pending_socket_0211b0()override{unused();}
    srv::LocalEndpoint local_endpoint()override{unused();}
    bool set_environment_variable(std::string_view,std::string_view)override{unused();}
    void async_accept_021b00()override{unused();}
    srv::Error run_context_0298b0()override{unused();}
    void destroy_pending_socket_024db0()override{unused();}
    void destroy_acceptor_024db0()override{unused();}
    void destroy_context()override{unused();}
    [[noreturn]]void raise_error(srv::Error,std::string_view)override{unused();}
};
struct Transport final:sock::SynchronousTransport {
    using SynchronousTransport::SynchronousTransport;bool receive_armed=false;
    void async_receive(std::size_t requested,std::function<void(srv::Error,std::string)>)override{require(requested>=512,"Session arms read after greeting");receive_armed=true;}
    void async_send(std::span<const char>,srv::Completion)override{unused();}
};
struct Scheduling final:srv::Scheduling {
    void post_io(std::function<void()>)override{unused();}
    void launch_detached(std::function<void()>)override{unused();}
    void dispatch_write(std::function<void()>)override{unused();}
    void log_error(std::string_view,srv::Error)override{unused();}
    [[noreturn]]void raise_write_error(srv::Error)override{unused();}
};
struct Handler final:srv::Handler {unsigned resets=0;void reset_02bc70()override{++resets;}std::string initial_message_02d540()override{return std::string("hello\0",6);}std::string process_message_033d70(std::string_view)override{unused();}};
void composition(){
    {Api api;api.parsed.result=0;api.parsed.bytes[0]=127;api.parsed.bytes[3]=1;api.category.logical=srv::ErrorCategory::other;Backend backend(api);const auto result=backend.parse_address_028690(2,"127.0.0.1");require(result.conversion_result==1 && result.error.value==0 && result.error.category==srv::ErrorCategory::other && result.bytes[0]==127 && result.bytes[3]==1,"partial backend forwards logical parsed outputs");}
    Api api;api.sends={{-1,0,10035},{0,3,0},{0,2,0},{0,1,0}};api.selects={{0,0}};sock::Socket socket=9;std::uint8_t state=0;
    Transport transport(socket,state,api);Scheduling scheduling;Handler handler;srv::Session session(transport,scheduling,handler);session.start_025720();
    require(handler.resets==2 && transport.receive_armed && api.written==std::string("hello\0",6),"actual Session greeting uses recovered send retry and partial write-all, preserving NUL");
    require(api.flags==std::vector<std::uint32_t>(4,0),"transport bridge uses synchronous send flags0");
    socket=17;state=1;api.sends={{-1,0,77}};const auto result=transport.synchronous_send({"x",1});require(result.error.value==77 && result.transferred==0 && api.sockets.back()==17,"transport observes live socket/state references");
}
}
int main(){try{addresses();sends();composition();std::cout<<"PASS: "<<checks<<" socket operation checks; injected APIs and actual Session greeting only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
