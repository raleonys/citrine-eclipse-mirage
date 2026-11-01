// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_closing.hpp"
#include "preloader_l/net/transport_acceptor_native.hpp"
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <vector>

namespace sock=preloader_l::socket;
namespace srv=preloader_l::server;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected closing dependency");}
struct Errors final:sock::Api {
    std::vector<std::string>& trace;std::deque<int> codes;std::function<void()> category_hook;
    explicit Errors(std::vector<std::string>& trace):trace(trace){}
    void set_last_error(std::int32_t)override{unused();}
    sock::Category system_category_028620()override{trace.push_back("category");if(category_hook)category_hook();return {11,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{trace.push_back("get");require(!codes.empty(),"finite WSA error script");int code=codes.front();codes.pop_front();return code;}
    sock::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    sock::AddressCall string_to_address(sock::Conversion&,std::int32_t,std::int32_t)override{unused();}
    sock::SendCall wsa_send(sock::Socket,std::span<sock::Buffer>,std::uint32_t)override{unused();}
    std::int32_t select_write(sock::Socket,std::int32_t)override{unused();}
};
struct Api final:sock::AcceptorApi {
    std::vector<std::string>& trace;std::function<void(std::string_view)> hook;
    std::deque<int> closes;std::vector<sock::Socket> sockets;
    explicit Api(std::vector<std::string>& trace):trace(trace){}
    void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
    sock::Socket create_socket(std::int32_t family)override{require(family==2,"integration constructor selects IPv4");event("create");return 13;}
    std::uintptr_t attach_completion_port(sock::Socket socket,std::uintptr_t port)override{require(socket==13 && port==99,"integration attaches actual service completion port");event("attach");return 33;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(sock::Socket socket,int level,int option,int value)override{require(level==65535 && option==128 && value==0,"closing requests exact linger option");sockets.push_back(socket);event("option");return 0;}
    std::int32_t bind_socket(sock::Socket socket,std::span<const std::byte>,std::int32_t length)override{require(socket==13 && length==16,"integration bind follows recovered constructor");event("bind");return 0;}
    std::int32_t listen_socket(sock::Socket socket,std::int32_t backlog)override{require(socket==13 && backlog==2147483647,"integration listens with observed backlog");event("listen");return 0;}
    std::int32_t close_socket(sock::Socket socket)override{sockets.push_back(socket);event("close");require(!closes.empty(),"finite close script");int value=closes.front();closes.pop_front();return value;}
    std::int32_t set_nonblocking(sock::Socket socket,std::uint32_t command,std::uint32_t& value)override{sockets.push_back(socket);require(command==0x8004667eU && value==0,"integration retry ioctl exact arguments");event("ioctl");return -1;}
};
struct Gate final:sock::SocketGate {
    std::vector<std::string>& trace;std::string prefix;int depth=0;std::function<void(std::string_view)> hook;
    Gate(std::vector<std::string>& trace,std::string prefix):trace(trace),prefix(std::move(prefix)){}
    void enter()override{trace.push_back(prefix+"enter");require(depth==0,"gate enters once");++depth;if(hook)hook("enter");}
    void leave()override{trace.push_back(prefix+"leave");require(depth==1,"gate leaves after enter");if(hook)hook("leave");--depth;}
};
struct Cancel final:sock::CancellationEffects {
    std::vector<std::string>& trace;unsigned calls=0;std::function<void(sock::CancellationEngine&,sock::Socket,const sock::Error&)> hook;
    explicit Cancel(std::vector<std::string>& trace):trace(trace){}
    void cancel_026aa0(sock::CancellationEngine& engine,sock::Socket socket,const sock::Error& reason)override{trace.push_back("cancel");++calls;require(reason.value==995 && reason.category.name_identity==11,"cancellation forwards operation-aborted995/system category");if(hook)hook(engine,socket,reason);}
};
struct Owner final:sock::Control {
    unsigned destroys=0,deletes=0;std::function<void()> hook;
    Owner(int strong=1,int weak=1){this->strong=strong;this->weak=weak;}
    void destroy_resource()override{++destroys;if(hook)hook();}
    void delete_control()override{++deletes;}
};
void initial_and_reset(){
    for(bool valid:{false,true})for(int strong:{1,2,0})for(int weak:{1,3}){
        std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);Gate gate(trace,"engine_");sock::CancellationEngine engine(gate);sock::CloseContext context;context.engine=&engine;sock::AcceptorState state;state.socket=valid?7:sock::invalid_socket;state.socket_state=3;sock::SocketObject object(state);Owner owner(strong,weak);object.cancellation={99,&owner};api.closes={0};
        owner.hook=[&]{require(state.socket==sock::invalid_socket && state.socket_state==0 && object.cancellation.resource==0 && !object.cancellation.control,"reset stores precede old owner destruction callback");};
        sock::cancel_and_close_026fb0(context,object,errors,api,cancel);
        require(context.engine==&engine,"nonnull engine snapshot CAS does not clear field");
        require(state.socket==sock::invalid_socket && state.socket_state==0 && object.cancellation.resource==0 && !object.cancellation.control,"close resets socket/state/resource/control unconditionally");
        require(owner.strong==strong-1 && owner.destroys==static_cast<unsigned>(strong==1),"reset decrements strong once and destroys only previous1");
        require(owner.weak==(strong==1?weak-1:weak) && owner.deletes==static_cast<unsigned>(strong==1 && weak==1),"weak release follows last strong and deletes only previous1");
        const auto expected=valid?std::vector<std::string>{"engine_enter","category","cancel","engine_leave","close"}:std::vector<std::string>{};
        require(trace==expected && cancel.calls==static_cast<unsigned>(valid),"invalid initial socket skips engine/close but not reset/release");
    }
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);sock::CloseContext context;sock::AcceptorState state;state.socket=9;sock::SocketObject object(state);api.closes={7};errors.codes={0};sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(!context.engine && cancel.calls==0 && trace==std::vector<std::string>{"close","category","get","category","category"},"null engine skips gate/category/cancel; raw close error0 does not prevent reset");require(state.socket==sock::invalid_socket && !object.cancellation.control,"raw nonzero close result ignored by outer reset");}
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);sock::CloseContext context;sock::AcceptorState state;sock::SocketObject object(state);sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(trace.empty() && !context.engine,"invalid initial socket and null owner is complete API no-op");}
}
void live_effects(){
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);Gate first_gate(trace,"first_"),other_gate(trace,"other_");sock::CancellationEngine first(first_gate),other(other_gate);sock::CloseContext context;context.engine=&first;sock::AcceptorState state;state.socket=13;sock::SocketObject object(state);api.closes={0};first_gate.hook=[&](std::string_view phase){if(phase=="enter"){state.socket=17;context.engine=&other;}else state.socket=23;};cancel.hook=[&](sock::CancellationEngine& engine,sock::Socket socket,const sock::Error&){require(&engine==&first && socket==13 && first_gate.depth==1 && other_gate.depth==0,"cancel uses cached engine/socket captured before gate");state.socket=19;};sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(api.sockets==std::vector<sock::Socket>{23} && context.engine==&other,"close rereads live socket after gate leave without replacing changed engine slot");require(trace==std::vector<std::string>{"first_enter","category","cancel","first_leave","close"},"same cached engine gate left after cancellation");}
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);Gate gate(trace,"engine_");sock::CancellationEngine engine(gate);sock::CloseContext context;context.engine=&engine;sock::AcceptorState state;state.socket=7;sock::SocketObject object(state);cancel.hook=[&](sock::CancellationEngine&,sock::Socket,const sock::Error&){state.socket=sock::invalid_socket;};sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(api.sockets.empty() && cancel.calls==1,"cancel changing live socket invalid causes close helper no-op");}
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);sock::CloseContext context;sock::AcceptorState state;state.socket=7;sock::SocketObject object(state);Owner initial(5,7),late;object.cancellation={123,&initial};api.closes={0};api.hook=[&](std::string_view event){if(event=="close")object.cancellation.control=&late;};late.hook=[&]{require(!object.cancellation.control && object.cancellation.resource==0,"late old control cleared before release");late.weak=4;};sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(initial.strong==5 && !initial.destroys && late.destroys==1 && late.weak==3 && !late.deletes,"owner is read after close and weak after destruction callback");}
    {std::vector<std::string> trace;Errors errors(trace);Api api(trace);Cancel cancel(trace);sock::CloseContext context;sock::AcceptorState state;sock::SocketObject object(state);Owner old,installed(5,7);object.cancellation={123,&old};old.hook=[&]{state.socket=99;state.socket_state=8;object.cancellation={456,&installed};};sock::cancel_and_close_026fb0(context,object,errors,api,cancel);require(state.socket==99 && state.socket_state==8 && object.cancellation.resource==456 && object.cancellation.control==&installed,"release callback reentry is not overwritten by extra reset after release");require(old.destroys==1 && old.deletes==1 && installed.strong==5,"old release cached while callback installs another owner");}
}
struct Effects final:sock::ClosingLifecycleEffects {
    std::vector<std::string>& trace;sock::SocketService& service;unsigned cleaned=0;
    Effects(sock::SocketService& service,sock::CloseContext& context,Errors& errors,Api& api,Cancel& cancel,std::vector<std::string>& trace)
        :ClosingLifecycleEffects(service,context,errors,api,cancel),trace(trace),service(service){}
    sock::SocketService& resolve_service_0239c0(sock::ContextIdentity context)override{trace.push_back("resolve");require(context.value==42,"composed resolver supplied context");return service;}
    void executor_cleanup(sock::SocketObject& object,std::uintptr_t table)override{trace.push_back("executor");++cleaned;require(table==0x180084928ULL && !object.next && !object.previous,"actual destructor unlinks before cleanup");require(object.state.socket==sock::invalid_socket && !object.cancellation.control,"actual close body resets before executor and final owner read");}
};
struct Runtime final:sock::LifecycleRuntime {using LifecycleRuntime::LifecycleRuntime;sock::Category miscellaneous_category_0285b0()override{unused();}[[noreturn]]void raise_028240(sock::Error,std::string_view)override{unused();}};
struct Factory final:sock::ControlFactory {Owner owner;sock::Control* allocate_04b0bc(std::uint32_t request)override{require(request==24,"composed acceptor requests observed control allocation size");return &owner;}};
void composition(){
    std::vector<std::string> trace;Gate service_gate(trace,"service_"),engine_gate(trace,"engine_");sock::SocketService service(service_gate);service.completion_port=99;sock::CancellationEngine engine(engine_gate);sock::CloseContext context;context.engine=&engine;Errors errors(trace);Api api(trace);Cancel cancel(trace);Effects effects(service,context,errors,api,cancel,trace);sock::AcceptorState state;sock::SocketObject object(state);Factory factory;Runtime runtime(object,{42},effects,factory);alignas(8) std::array<std::byte,128> address{};address[0]=std::byte{2};
    sock::acceptor_020e20(state,address,false,errors,api,runtime);require(service.head==&object && object.cancellation.control==&factory.owner,"actual acceptor binds recovered initializer and shared-owner replacement");
    state.socket_state=0x1b;api.closes={-1,0};errors.codes={10035};trace.clear();cancel.hook=[&](sock::CancellationEngine& selected,sock::Socket socket,const sock::Error&){require(&selected==&engine && socket==13 && engine_gate.depth==1 && service_gate.depth==0,"real destruction cancels before service unlink gate");};
    runtime.destroy_socket_024db0();
    require(trace==std::vector<std::string>{"engine_enter","category","cancel","engine_leave","option","close","category","get","category","ioctl","close","service_enter","service_leave","executor"},"actual acceptor/lifecycle/closing/destructor chain exact normal order");
    require(!service.head && state.socket==sock::invalid_socket && state.socket_state==0 && !object.cancellation.control && factory.owner.destroys==1 && factory.owner.deletes==1,"normal close owner released exactly once; outer destructor final read seesNULL");
    require(context.engine==&engine && effects.cleaned==1 && engine_gate.depth==0 && service_gate.depth==0,"cached engine retained and both gates balanced");
    sock::SocketService wrong(service_gate);bool caught=false;try{effects.close_and_cancel_026fb0(wrong,object);}catch(const sock::UnsupportedInput&){caught=true;}require(caught,"service-context mapping mismatch explicitly outside adapter domain");
}
struct NativeFixture {std::vector<std::string> trace;std::deque<int> statuses{-1,0};sock::Socket socket=0xfedcba9876543210ULL;std::uint8_t* state=nullptr;};
NativeFixture* native=nullptr;
int WSAAPI native_option(SOCKET socket,int level,int name,const char* value,int size){native->trace.push_back("option");int integer=-1;if(value && size==4)std::memcpy(&integer,value,4);require(socket==native->socket && level==65535 && name==128 && size==4 && integer==0,"SDK adapter forwards recovered linger call typed4bytevalue");return 0;}
int WSAAPI native_close(SOCKET socket){native->trace.push_back("close");require(socket==native->socket && !native->statuses.empty(),"SDK close forwards full cached socket and finite script");if(native->statuses.size()==1)require(*native->state==0xfc,"second SDK close follows post-ioctl low-bit clear");int value=native->statuses.front();native->statuses.pop_front();return value;}
int WSAAPI native_ioctl(SOCKET socket,long command,u_long* value){native->trace.push_back("ioctl");require(socket==native->socket && static_cast<std::uint32_t>(command)==0x8004667eU && value && *value==0,"SDK ioctl forwards commandbits and DWORD0");*native->state=0xff;return -1;}
void typed_sdk(){
    NativeFixture fixture;native=&fixture;std::uint8_t state=11;fixture.state=&state;Errors errors(fixture.trace);errors.codes={10035};sock::AcceptorImports imports;imports.option=native_option;imports.close=native_close;imports.ioctl=native_ioctl;sock::NativeAcceptorApi api(imports);sock::Error error;
    require(sock::close_026e50(fixture.socket,state,true,error,errors,api)==0 && error.value==0 && state==0xfc,"real recovered close composes typed injected SDK adapter");
    require(fixture.trace==std::vector<std::string>{"option","close","category","get","category","ioctl","close"},"typed SDK composition adds no error reads or allocator cleanup");native=nullptr;
}
}
int main(){try{initial_and_reset();live_effects();composition();typed_sdk();std::cout<<"PASS: "<<checks<<" socket closing checks; controlled cancellation and injected APIs only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
