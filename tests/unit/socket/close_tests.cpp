// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_close.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <vector>

namespace sock=citrine_eclipse_mirage::socket;
namespace srv=citrine_eclipse_mirage::server;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected close dependency");}
struct Fixture final:sock::Api,sock::AcceptorApi {
    std::vector<std::string> trace;std::function<void(std::string_view)> hook;
    std::deque<int> statuses,errors;std::deque<sock::Category> categories;
    std::vector<sock::Socket> sockets;unsigned category_calls=0;
    int option_status=0,ioctl_status=0,last=0;
    void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
    void set_last_error(std::int32_t)override{unused();}
    sock::Category system_category_028620()override{++category_calls;event("category");if(categories.empty())return {11,srv::ErrorCategory::system};auto c=categories.front();categories.pop_front();return c;}
    std::int32_t get_last_error()override{event("get");if(errors.empty())return last;int result=errors.front();errors.pop_front();return result;}
    sock::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    sock::AddressCall string_to_address(sock::Conversion&,std::int32_t,std::int32_t)override{unused();}
    sock::SendCall wsa_send(sock::Socket,std::span<sock::Buffer>,std::uint32_t)override{unused();}
    std::int32_t select_write(sock::Socket,std::int32_t)override{unused();}
    sock::Socket create_socket(std::int32_t)override{unused();}
    std::uintptr_t attach_completion_port(sock::Socket,std::uintptr_t)override{unused();}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(sock::Socket socket,int level,int option,int value)override{event("option");sockets.push_back(socket);require(level==65535 && option==128 && value==0,"destroying linger option exact level/name/int0");return option_status;}
    std::int32_t bind_socket(sock::Socket,std::span<const std::byte>,std::int32_t)override{unused();}
    std::int32_t listen_socket(sock::Socket,std::int32_t)override{unused();}
    std::int32_t close_socket(sock::Socket socket)override{event("close");sockets.push_back(socket);require(!statuses.empty(),"finite scripted close attempts");int result=statuses.front();statuses.pop_front();return result;}
    std::int32_t set_nonblocking(sock::Socket socket,std::uint32_t command,std::uint32_t& value)override{event("ioctl");sockets.push_back(socket);require(command==0x8004667eU && value==0,"close retry exact ioctl command and DWORD0");value=123;return ioctl_status;}
};
void invalid_and_success(){
    for(bool destroying:{false,true})for(std::uint8_t initial:{0,3,8,11,255}){
        Fixture f;auto state=initial;sock::Error error{777,{99,srv::ErrorCategory::other}};
        require(sock::close_026e50(sock::invalid_socket,state,destroying,error,f,f)==0,"invalid close returns0");
        require(f.trace.empty() && state==initial && error.value==777 && error.category.name_identity==99,"invalid close never touches state/error/APIs");
        f.statuses={0};
        require(sock::close_026e50(0xfedcba9876543210ULL,state,destroying,error,f,f)==0,"successful close returns raw0");
        const bool option=destroying && (initial&8);
        require(f.trace==(option?std::vector<std::string>{"option","close"}:std::vector<std::string>{"close"}),"only destroying with statebit8 invokes linger option");
        require(error.value==0 && error.category.name_identity==99 && error.category.logical==srv::ErrorCategory::other && state==initial,"success clears value only, retains category and state");
        for(auto socket:f.sockets)require(socket==0xfedcba9876543210ULL,"all close operations preserve full64bit socket");
    }
    for(int status:{-1,7}){Fixture f;f.option_status=status;f.statuses={0};f.errors={10035};std::uint8_t state=8;sock::Error error{777,{99,srv::ErrorCategory::other}};require(sock::close_026e50(9,state,true,error,f,f)==0,"failed linger option does not prevent close");require(f.trace==std::vector<std::string>{"option","category","get","close"} && error.value==0 && error.category.name_identity==99,"ignored option failure retains mandatory category/error calls without assigning caller category");}
}
void failures_and_retry(){
    for(int status:{-1,1,7})for(int code:{0,64,1234,10035,1237,88}){
        Fixture f;f.statuses={status,0};f.errors={code};std::uint8_t state=0xff;sock::Error error;
        const bool retry=code==10035 || code==1237;
        const auto result=sock::close_026e50(0xabcdef0123456789ULL,state,false,error,f,f);
        require(result==(retry?0:status),"close raw nonzero status preserved unless retry succeeds");
        require(error.value==(retry?0:code) && error.category.name_identity==11,"failed close captures raw WSA error without64/1234 remapping");
        require(state==(retry?0xfc:0xff),"only retry clears low2 state bits");
        std::vector<std::string> expected{"close","category","get","category"};if(code!=10035)expected.push_back("category");if(retry){expected.push_back("ioctl");expected.push_back("close");}
        require(f.trace==expected,"close failure exact comparison/get/ioctl order");
    }
    for(int status:{-1,3}){Fixture f;f.statuses={-1,status};f.errors={10035,64};f.ioctl_status=-1;f.categories={{11,srv::ErrorCategory::other},{11,srv::ErrorCategory::system},{33,srv::ErrorCategory::other}};std::uint8_t state=3;sock::Error error;require(sock::close_026e50(8,state,false,error,f,f)==status && error.value==64 && error.category.name_identity==33,"second close failure captures raw second code/category and raw status");require(state==0 && f.trace==std::vector<std::string>{"close","category","get","category","ioctl","close","category","get"},"failed ioctl still clears state and retries; second failure does not compare again");}
    {Fixture f;f.statuses={-1,0};f.errors={10035};f.categories={{11,srv::ErrorCategory::other},{11,srv::ErrorCategory::system}};std::uint8_t state=3;sock::Error e;require(sock::close_026e50(1,state,false,e,f,f)==0 && e.category.logical==srv::ErrorCategory::other,"retry uses opaque name identity, success preserves captured logical category");}
    {Fixture f;f.statuses={-1};f.errors={10035};f.categories={{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::system}};std::uint8_t state=3;sock::Error e;require(sock::close_026e50(1,state,false,e,f,f)==-1 && f.sockets.size()==1,"10035 cannot use only second category match");}
    {Fixture f;f.statuses={-1,0};f.errors={1237};f.categories={{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::other}};std::uint8_t state=3;sock::Error e;require(sock::close_026e50(1,state,false,e,f,f)==0 && state==0,"1237 may use second category match");}
    {Fixture f;f.statuses={-1};f.errors={1237};f.categories={{11,srv::ErrorCategory::system},{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system}};std::uint8_t state=3;sock::Error e;require(sock::close_026e50(1,state,false,e,f,f)==-1 && state==3,"1237 requires second identity even if first matched");}
}
void live_effects(){
    {Fixture f;f.statuses={-1};f.last=44;std::uint8_t state=0;sock::Error e;f.hook=[&](std::string_view event){if(event=="category")f.last=222;};(void)sock::close_026e50(1,state,false,e,f,f);require(e.value==222,"WSA last error sampled after mandatory category effects");}
    {Fixture f;f.statuses={-1,0};f.errors={10035};std::uint8_t state=3;sock::Error e;f.hook=[&](std::string_view event){if(event=="ioctl")state=0xab;if(event=="close" && f.trace.size()>1)require(state==0xa8,"state clears live post-ioctl lowbits before second close");};require(sock::close_026e50(1,state,false,e,f,f)==0 && state==0xa8,"ioctl state mutation is preserved except low2 bits");}
    {Fixture f;f.statuses={-1,0};f.errors={88};std::uint8_t state=3;sock::Error e;f.hook=[&](std::string_view event){if(event=="category" && f.category_calls==2)e.value=10035;};require(sock::close_026e50(1,state,false,e,f,f)==0 && state==0,"comparison reads live caller error after category hook");}
    {Fixture f;f.statuses={-1};f.errors={10035};std::uint8_t state=3;sock::Error e;f.hook=[&](std::string_view event){if(event=="category" && f.category_calls==2)e.category.name_identity=99;};require(sock::close_026e50(1,state,false,e,f,f)==-1 && state==3,"comparison reads live caller category after factory effects");}
}
}
int main(){try{invalid_and_success();failures_and_retry();live_effects();std::cout<<"PASS: "<<checks<<" socket close checks; injected socket/error APIs only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
