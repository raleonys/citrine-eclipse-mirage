// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_acceptor.hpp"
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace sock=citrine_eclipse_mirage::socket;
namespace srv=citrine_eclipse_mirage::server;
namespace {
unsigned checks=0;
void require(bool condition,const char* why){++checks;if(!condition)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected acceptor test dependency");}
struct Raised {sock::Error error;std::string context;};
struct Option {sock::Socket socket;int level,option,value;};
struct Fixture final:sock::Api,sock::AcceptorApi,sock::AcceptorRuntime {
    std::vector<std::string> trace;
    std::function<void(std::string_view)> hook;
    sock::Socket created=0x123456789abcdef0ULL,initial=sock::invalid_socket;
    std::uintptr_t port=0xabcdef0123456789ULL,attached=0x9988776655443322ULL;
    sock::AcceptorService service{port},replacement{42};
    int last=0,bind_status=0,listen_status=0,ioctl_status=0;
    std::uint32_t win_error=0;
    std::deque<int> errors,option_statuses,close_statuses;
    std::deque<sock::Category> categories;
    std::vector<int> created_families,bind_sizes,backlogs;
    std::vector<sock::Socket> close_sockets,bind_sockets,listen_sockets;
    std::vector<std::pair<sock::Socket,std::uintptr_t>> attachments;
    std::vector<Option> options;
    const std::byte* expected_address=nullptr;
    std::vector<std::byte> bound_bytes;
    unsigned resets=0;
    void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
    void set_last_error(std::int32_t)override{unused();}
    sock::Category system_category_028620()override{event("category");if(categories.empty())return {11,srv::ErrorCategory::system};auto value=categories.front();categories.pop_front();return value;}
    std::int32_t get_last_error()override{event("wsa_error");if(errors.empty())return last;int value=errors.front();errors.pop_front();return value;}
    sock::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    sock::AddressCall string_to_address(sock::Conversion&,std::int32_t,std::int32_t)override{unused();}
    sock::SendCall wsa_send(sock::Socket,std::span<sock::Buffer>,std::uint32_t)override{unused();}
    std::int32_t select_write(sock::Socket,std::int32_t)override{unused();}
    sock::Socket create_socket(std::int32_t family)override{event("create");created_families.push_back(family);return created;}
    std::uintptr_t attach_completion_port(sock::Socket socket,std::uintptr_t value)override{event("attach");attachments.emplace_back(socket,value);return attached;}
    std::uint32_t get_win32_error()override{event("win_error");return win_error;}
    std::int32_t set_option(sock::Socket socket,std::int32_t level,std::int32_t option,std::int32_t value)override{event("option");options.push_back({socket,level,option,value});if(option_statuses.empty())return 0;int result=option_statuses.front();option_statuses.pop_front();return result;}
    std::int32_t bind_socket(sock::Socket socket,std::span<const std::byte> bytes,std::int32_t size)override{event("bind");require(!expected_address || bytes.data()==expected_address,"bind receives original bounded address storage");bind_sockets.push_back(socket);bind_sizes.push_back(size);bound_bytes.assign(bytes.begin(),bytes.begin()+size);return bind_status;}
    std::int32_t listen_socket(sock::Socket socket,std::int32_t backlog)override{event("listen");listen_sockets.push_back(socket);backlogs.push_back(backlog);return listen_status;}
    std::int32_t close_socket(sock::Socket socket)override{event("close");close_sockets.push_back(socket);require(!close_statuses.empty(),"finite scripted close attempts");int result=close_statuses.front();close_statuses.pop_front();return result;}
    std::int32_t set_nonblocking(sock::Socket socket,std::uint32_t command,std::uint32_t& value)override{event("ioctl");require(command==0x8004667eU && value==0,"close retry disables nonblocking with exact command and DWORD0");require(!close_sockets.empty() && socket==close_sockets.front(),"ioctl uses cached original socket");value=99;return ioctl_status;}
    void initialize_0211b0(sock::AcceptorState& state)override{state.socket=initial;state.socket_state=0xa5;state.protocol_40=99;state.byte_44=88;state.slot_48=77;state.slot_50=66;state.slot_58=55;state.slot_60=44;state.service=&service;event("initialize");}
    void reset_cancellation_0237f0(sock::AcceptorState& state)override{++resets;require(state.socket==created && state.socket_state==0x10,"socket/state transfer precedes cancellation reset");require(state.protocol_40==99 && state.slot_48==77,"protocol fields reset after cancellation reset");event("reset");}
    sock::Category miscellaneous_category_0285b0()override{event("misc");return {33,srv::ErrorCategory::asio_misc};}
    [[noreturn]]void raise_028240(sock::Error error,std::string_view context)override{event("raise");throw Raised{error,std::string(context)};}
};
struct Address {
    alignas(8) std::array<std::byte,128> bytes{};
    explicit Address(std::uint16_t family){set(family);}
    void set(std::uint16_t family){std::memcpy(bytes.data(),&family,sizeof(family));}
    operator std::span<const std::byte>()const{return bytes;}
};
void reset_fields(const sock::AcceptorState& state,int family){
    require(state.protocol_40==family && state.byte_44==0,"protocol family and marker reset");
    require(state.slot_48==2 && state.slot_50==0 && state.slot_58==0 && state.slot_60==0,"protocol seed fixed2 and remaining fields zero");
}
template<class F>Raised raises(F&& call){try{call();}catch(const Raised& error){++checks;return error;}throw std::runtime_error("Expected native raise boundary");}
void close_helper(){
    {Fixture f;sock::Socket socket=sock::invalid_socket;sock::close_025310(socket,f,f);require(f.trace.empty() && socket==sock::invalid_socket,"invalid socket close is complete no-op");}
    {Fixture f;sock::Socket socket=0xfedcba9876543210ULL;f.close_statuses={0};sock::close_025310(socket,f,f);require(f.trace==std::vector<std::string>{"close"} && f.close_sockets.front()==socket,"successful close does not sample errors or invalidate slot");}
    for(int error:{0,64,1234,1237,10035,77}){
        Fixture f;sock::Socket socket=0xfedcba9876543210ULL;f.close_statuses={-1,0};f.errors={error};f.ioctl_status=-1;sock::close_025310(socket,f,f);
        const bool retry=error==10035 || error==1237;
        require(f.close_sockets.size()==static_cast<std::size_t>(retry?2:1),"only matching would-block or retry errors trigger second close");
        require(socket==0xfedcba9876543210ULL,"close helper never invalidates caller slot");
        std::vector<std::string> expected{"close","category","wsa_error","category"};
        if(error!=10035)expected.push_back("category");
        if(retry){expected.push_back("ioctl");expected.push_back("close");}
        require(f.trace==expected,"close helper exact category/get/ioctl call order");
    }
    {Fixture f;sock::Socket socket=9;f.close_statuses={-1,-1};f.errors={10035,222};sock::close_025310(socket,f,f);require(f.trace==std::vector<std::string>{"close","category","wsa_error","category","ioctl","close","category","wsa_error"},"second close failure captures and discards error after category");}
    {Fixture f;sock::Socket socket=9;f.close_statuses={-1,0};f.errors={10035};f.categories={{11,srv::ErrorCategory::other},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::system}};sock::close_025310(socket,f,f);require(f.close_sockets.size()==1,"10035 cannot match only second name branch");}
    {Fixture f;sock::Socket socket=9;f.close_statuses={-1,0};f.errors={1237};f.categories={{11,srv::ErrorCategory::other},{22,srv::ErrorCategory::system},{11,srv::ErrorCategory::system}};sock::close_025310(socket,f,f);require(f.close_sockets.size()==2,"1237 may match second opaque name identity despite different logical enum");}
    {Fixture f;sock::Socket socket=9;f.close_statuses={-1,0};f.errors={1237};f.categories={{11,srv::ErrorCategory::system},{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::system}};sock::close_025310(socket,f,f);require(f.close_sockets.size()==1,"1237 requires second category identity, not first");}
    {Fixture f;sock::Socket socket=9;f.close_statuses={-1,0};f.errors={10035};f.hook=[&](std::string_view event){if(event=="category")socket=17;};sock::close_025310(socket,f,f);require(f.close_sockets==std::vector<sock::Socket>{9,9} && socket==17,"close helper caches input before provider changes caller slot");}
}
void success(){
    for(const std::uint16_t family:{2,23,0,65535})for(bool reuse:{false,true}){
        Fixture f;Address address(family);f.expected_address=address.bytes.data();sock::AcceptorState state;
        auto& result=sock::acceptor_020e20(state,address,reuse,f,f,f);
        const int selected=family==2?2:23;
        require(&result==&state && state.socket==f.created && state.socket_state==0x10,"successful constructor returns same logical state with transferred socket");
        require(f.created_families==std::vector<int>{selected} && f.attachments==std::vector<std::pair<sock::Socket,std::uintptr_t>>{{f.created,f.port}},"socket protocol selected and service port passed full-width");
        reset_fields(state,selected);
        require(f.bind_sizes==std::vector<int>{family==2?16:28} && f.backlogs==std::vector<int>{std::numeric_limits<int>::max()},"bind family selects16or28, listen usesINTMAX");
        require(f.resets==1 && f.close_sockets.empty(),"success resets cancellation once and keeps owned socket");
        std::vector<std::string> expected{"initialize","create"};if(family!=2)expected.push_back("option");expected.push_back("attach");expected.push_back("reset");if(reuse)expected.push_back("option");expected.push_back("bind");expected.push_back("listen");require(f.trace==expected,"successful constructor exact operation order without stale-error reads");
        std::size_t index=0;if(family!=2){const auto option=f.options[index++];require(option.level==41 && option.option==27 && option.value==0 && option.socket==f.created,"all nonIPv4 families request dual-stack zero option");}if(reuse){const auto option=f.options[index++];require(option.level==65535 && option.option==4 && option.value==1 && option.socket==f.created,"reuse option exact level/name/value");}require(index==f.options.size(),"no extra option operations");
    }
    {Fixture f;Address address(23);f.option_statuses={-1};sock::AcceptorState state;sock::acceptor_020e20(state,address,false,f,f,f);require(f.trace==std::vector<std::string>{"initialize","create","option","attach","reset","bind","listen"},"failed dual-stack option completely ignored without error getter");}
    {Fixture f;Address address(2);sock::AcceptorState state;f.hook=[&](std::string_view event){if(event=="initialize")address.set(23);if(event=="create"){state.service=&f.replacement;f.service.completion_port=99;}if(event=="option")f.service.completion_port=100;if(event=="reset")address.set(2);};sock::acceptor_020e20(state,address,false,f,f,f);require(f.created_families==std::vector<int>{23} && f.bind_sizes==std::vector<int>{16},"family read after initializer and reread before bind");require(f.attachments.front().second==100,"service identity cached but original service port read live after dual-stack option");reset_fields(state,23);}
    {Fixture f;Address address(2);sock::AcceptorState state;f.hook=[&](std::string_view event){if(event=="reset")state.socket=17;if(event=="option")state.socket=18;if(event=="bind")state.socket=19;};sock::acceptor_020e20(state,address,true,f,f,f);require(f.options.back().socket==17 && f.bind_sockets==std::vector<sock::Socket>{18} && f.listen_sockets==std::vector<sock::Socket>{19},"option/bind/listen reload live socket before each call");}
}
void failures(){
    {Fixture f;Address address(2);sock::AcceptorState state;f.initial=7;const auto raised=raises([&]{sock::acceptor_020e20(state,address,true,f,f,f);});require(raised.error.value==1 && raised.error.category.logical==srv::ErrorCategory::asio_misc && raised.context=="open","already-open socket raises misc1/open");require(f.trace==std::vector<std::string>{"initialize","misc","raise"} && state.socket==7 && f.resets==0,"already-open path skips APIs and protocol reset");}
    for(bool reuse:{false,true})for(int error:{0,64,1234,10035}){
        Fixture f;Address address(2);sock::AcceptorState state;f.created=sock::invalid_socket;f.errors={error};const auto raised=raises([&]{sock::acceptor_020e20(state,address,reuse,f,f,f);});
        require(raised.error.value==(error?error:10009),"failed create preserves captured nonzero error or later invalid-socket error");require(f.close_sockets.empty() && f.resets==0 && f.attachments.empty(),"invalid temporary close no-op; no ownership transfer");
        require(raised.context==(error?"open":reuse?"set_option":"bind"),"failed create context follows captured error or subsequent invalid guard");
        std::vector<std::string> expected{"initialize","create","category","wsa_error"};if(!error)expected.push_back("category");expected.push_back("raise");require(f.trace==expected,"failed create exact category/error/raise ordering");
        if(error)require(state.protocol_40==99 && state.slot_48==77,"nonzero create error skips protocol reset");else reset_fields(state,2);
    }
    for(std::uint32_t error:{0U,5U,0x80000001U}){
        Fixture f;Address address(2);sock::AcceptorState state;f.attached=0;f.win_error=error;f.close_statuses={0};f.hook=[&](std::string_view event){if(event=="category")f.win_error=777;};
        if(error){const auto raised=raises([&]{sock::acceptor_020e20(state,address,false,f,f,f);});require(static_cast<std::uint32_t>(raised.error.value)==error && raised.context=="open","IOCP error captured before category side effects with open context");require(f.trace==std::vector<std::string>{"initialize","create","attach","win_error","category","close","raise"},"IOCP failure closes temporary before raising saved error");require(state.socket==sock::invalid_socket && f.resets==0 && state.protocol_40==99,"IOCP nonzero error does not transfer or reset protocol");}
        else{sock::acceptor_020e20(state,address,false,f,f,f);require(f.trace==std::vector<std::string>{"initialize","create","attach","win_error","category","reset","bind","listen"},"NULL IOCP with captured error0 still transfers and continues");require(state.socket==f.created && f.close_sockets.empty(),"IOCP error0 socket is retained");}
    }
    {Fixture f;Address address(2);sock::AcceptorState state;f.attached=0;f.win_error=5;f.close_statuses={-1,0};f.errors={10035};const auto raised=raises([&]{sock::acceptor_020e20(state,address,false,f,f,f);});require(raised.error.value==5 && f.close_sockets==std::vector<sock::Socket>{f.created,f.created},"IOCP cleanup uses recovered close retry without replacing saved initial error");}
    for(const std::string stage:{"reuse","bind","listen"})for(int error:{0,10048}){
        Fixture f;Address address(2);sock::AcceptorState state;f.errors={error};if(stage=="reuse")f.option_statuses={-1};else if(stage=="bind")f.bind_status=-1;else f.listen_status=-1;
        if(error){const auto raised=raises([&]{sock::acceptor_020e20(state,address,stage=="reuse",f,f,f);});require(raised.error.value==error && raised.context==(stage=="reuse"?"set_option":stage),"post-transfer failure propagates captured WSA code and context");require(f.trace.back()=="raise" && f.close_sockets.empty(),"late raise has no explicit normal-flow socket close");}
        else{sock::acceptor_020e20(state,address,stage=="reuse",f,f,f);require(f.trace.back()=="wsa_error" || f.trace.back()=="listen","nonzero API status and captured zero error continues or returns");require(f.listen_sockets.size()==1,"zero error preserves continuation through listen");}
        require(state.socket==f.created && f.resets==1,"post-transfer error retains logical socket; native EH cleanup outside this flow");reset_fields(state,2);
    }
    for(const std::string stage:{"reuse","bind","listen"}){
        Fixture f;Address address(2);sock::AcceptorState state;
        f.hook=[&](std::string_view event){if((stage=="reuse" || stage=="bind") && event=="reset")state.socket=sock::invalid_socket;if(stage=="listen" && event=="bind")state.socket=sock::invalid_socket;};
        const auto raised=raises([&]{sock::acceptor_020e20(state,address,stage=="reuse",f,f,f);});require(raised.error.value==10009 && raised.context==(stage=="reuse"?"set_option":stage) && f.trace[f.trace.size()-2]=="category","invalid live socket guard raises10009/context without WSA read");require(f.listen_sockets.empty(),"invalid live socket guard prevents listen call");
    }
    {Fixture f;Address address(2);sock::AcceptorState state;f.created=sock::invalid_socket;f.last=5;f.hook=[&](std::string_view event){if(event=="category")f.last=222;};const auto raised=raises([&]{sock::acceptor_020e20(state,address,false,f,f,f);});require(raised.error.value==222,"WSA path samples error after mandatory category effects");}
    {Fixture f;Address address(2);sock::AcceptorState state;f.attached=0;f.win_error=5;f.close_statuses={-1,0};f.errors={10035};f.categories={{11,srv::ErrorCategory::system},{22,srv::ErrorCategory::other},{22,srv::ErrorCategory::system}};const auto raised=raises([&]{sock::acceptor_020e20(state,address,false,f,f,f);});require(raised.error.category.name_identity==11 && raised.error.value==5,"IOCP saved error category survives cleanup category changes");}
    {Fixture f;Address address(2);sock::AcceptorState state;f.option_statuses={7};f.bind_status=7;f.listen_status=7;f.errors={0,0,0};sock::acceptor_020e20(state,address,true,f,f,f);require(f.trace==std::vector<std::string>{"initialize","create","attach","reset","option","category","wsa_error","bind","category","wsa_error","listen","category","wsa_error"},"positive nonzero API statuses also use error0 continuation");}
}
void bounded_domain(){
    for(std::size_t size:{std::size_t{0},std::size_t{1},std::size_t{15}}){Fixture f;Address address(2);sock::AcceptorState state;bool caught=false;try{sock::acceptor_020e20(state,{address.bytes.data(),size},false,f,f,f);}catch(const sock::UnsupportedInput&){caught=true;}require(caught,"short initialized address span explicitly unsupported");require(!f.trace.empty() && f.trace.front()=="initialize","constructor initialization precedes first family read even invalid bounded domain");}
    {Fixture f;Address address(23);sock::AcceptorState state;bool caught=false;try{sock::acceptor_020e20(state,{address.bytes.data(),27},false,f,f,f);}catch(const sock::UnsupportedInput&){caught=true;}require(caught,"short IPv6 bind storage explicitly unsupported");}
}
struct Backend final:sock::AcceptorBackend {
    using AcceptorBackend::AcceptorBackend;
    void initialize_context_0241b0()override{unused();}void initialize_pending_socket_0211b0()override{unused();}
    srv::LocalEndpoint local_endpoint()override{unused();}bool set_environment_variable(std::string_view,std::string_view)override{unused();}
    void async_accept_021b00()override{unused();}srv::Error run_context_0298b0()override{unused();}
    void destroy_pending_socket_024db0()override{unused();}void destroy_acceptor_024db0()override{unused();}void destroy_context()override{unused();}
    [[noreturn]]void raise_error(srv::Error,std::string_view)override{unused();}
};
void bridge(){
    for(std::uint16_t family:{2,23}){Fixture f;sock::AcceptorState state;Backend backend(state,f,f,f);srv::Endpoint endpoint;endpoint.family=family;endpoint.port_host_order=0x1234;for(unsigned i=0;i<16;++i)endpoint.address[i]=static_cast<std::uint8_t>(i+1);endpoint.scope_id=0x12345678;backend.construct_acceptor_020e20(endpoint,true,std::numeric_limits<int>::max());require(state.protocol_40==family && f.bind_sizes.front()==(family==2?16:28),"partial ServerBackend bridge selects original constructor flow");std::uint16_t encoded=0;std::memcpy(&encoded,f.bound_bytes.data(),2);require(encoded==family && f.bound_bytes[2]==std::byte{0x12} && f.bound_bytes[3]==std::byte{0x34},"bridge encodes family and host-order port as network bytes");const unsigned offset=family==2?4:8;const unsigned size=family==2?4:16;for(unsigned i=0;i<size;++i)require(f.bound_bytes[offset+i]==static_cast<std::byte>(endpoint.address[i]),"bridge serializes exact IPv4 or IPv6 address bytes");if(family==23){std::uint32_t scope=0;std::memcpy(&scope,f.bound_bytes.data()+24,4);require(scope==endpoint.scope_id,"bridge serializes IPv6 scope without swapping");for(unsigned i=4;i<8;++i)require(f.bound_bytes[i]==std::byte{0},"bridge initializes IPv6 flowinfo zero");}}
    {Fixture f;sock::AcceptorState state;Backend backend(state,f,f,f);srv::Endpoint endpoint;bool caught=false;try{backend.construct_acceptor_020e20(endpoint,true,1);}catch(const sock::UnsupportedInput&){caught=true;}require(caught && f.trace.empty(),"bridge rejects unmodeled backlog before runtime calls");}
}
}
int main(){try{close_helper();success();failures();bounded_domain();bridge();std::cout<<"PASS: "<<checks<<" acceptor operation checks; controlled runtime and socket APIs only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}catch(const Raised& error){std::cerr<<"FAIL: unexpected raise "<<error.error.value<<' '<<error.context<<'\n';return 1;}}
