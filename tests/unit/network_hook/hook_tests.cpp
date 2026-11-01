// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/platform/hook_connect.hpp"
#include "preloader_l/platform/hook_cleanup.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace preloader_l::network_hook;
namespace {
int checks;
void require(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
struct Injected{};
struct Exit{std::uint32_t code;};
struct Fixture {
    std::int32_t v4{16},v6{28};std::uint16_t port{6099};std::uint32_t attempts{};
    StateRefs refs(){return {v4,v6,port,attempts};}
};
std::array<std::byte,32> make_address(std::uint16_t family,std::uint16_t port){
    std::array<std::byte,32> b;for(std::size_t i=0;i<b.size();++i)b[i]=static_cast<std::byte>(i+30);
    b[0]=static_cast<std::byte>(family&255);b[1]=static_cast<std::byte>(family>>8);
    b[2]=static_cast<std::byte>(port>>8);b[3]=static_cast<std::byte>(port&255);
    if(family==2){b[4]=std::byte{127};b[5]=b[6]=std::byte{0};b[7]=std::byte{1};}
    if(family==23){std::fill(b.begin()+8,b.begin()+23,std::byte{0});b[23]=std::byte{1};}
    return b;
}
std::uint16_t get_port(std::span<const std::byte> b){return static_cast<std::uint16_t>((std::to_integer<unsigned>(b[2])<<8)|std::to_integer<unsigned>(b[3]));}
struct Fake final : Effects {
    std::vector<std::string> events;std::vector<std::uint16_t> log_ports;
    Fixture& f;std::string throw_at;std::uint16_t after_wait{},after_flush{};bool change_wait{},change_flush{};
    std::int32_t result{-10035};int connects{};Socket seen_socket{};const std::byte* seen_address{};std::vector<std::byte> seen_bytes;std::int32_t seen_length{};
    explicit Fake(Fixture& x):f(x){}
    void event(std::string s){events.push_back(s);if(s==throw_at)throw Injected{};}
    void wait_steps_036880(std::int32_t count) override {require(count==-1,"connect waits for all startup steps");event("wait");if(change_wait)f.port=after_wait;}
    void log_literal_006880(std::string_view s) override {event("literal:"+std::string(s));}
    void log_port_00ad70(std::uint16_t p) override {log_ports.push_back(p);event("port");}
    void log_finish_0088e0() override {event("flush");if(change_flush)f.port=after_flush;}
    void dialog_038bc0(std::string_view title,std::string_view text,std::uint32_t flags) override {
        require(title=="error"&&flags==0x10,"attempt15 dialog title/flags");
        require(text=="The game tried to connect to the crack 15 times already. Most probably your firewall or anti-virus blocks it. Disable them and try again.","target UI literal preserved");event("dialog");
    }
    [[noreturn]] void exit_05cdc0(std::uint32_t c) override {event("exit");throw Exit{c};}
    std::int32_t original_connect_0a3838(Socket s,std::span<const std::byte> a,std::int32_t n) override {
        ++connects;seen_socket=s;seen_address=a.data();seen_length=n;seen_bytes.assign(a.begin(),a.end());event("original");return result;
    }
    bool has(const char* s)const{return std::find(events.begin(),events.end(),s)!=events.end();}
};
void test_decisions(){
    for(auto family:{std::uint16_t{2},std::uint16_t{23}})
      for(auto port:{std::uint16_t{0},std::uint16_t{80},std::uint16_t{443},std::uint16_t{3216},std::uint16_t{6099}})
       for(bool local:{false,true}) {
        Fixture f;Fake e(f);auto b=make_address(family,port);
        if(!local)b[family==2?4:8]=std::byte{10};
        const auto old=b;const int length=family==2?16:28;
        const bool redirect=local&&port==3216;
        require(connect_00fd00(f.refs(),e,77,b,length)==e.result,"Winsock return retained");
        require(b==old,"caller storage stays byte-identical");
        require(e.has("wait")==redirect&&e.has("flush")==redirect,"only exact loopback:3216 waits/logs");
        require(f.attempts==unsigned(redirect),"only selected local destination counts");
        require(e.connects==1&&e.seen_socket==77&&e.seen_length==length,"original called once with socket/length");
        require((e.seen_address!=b.data())==redirect,"only redirect gets stack copy");
        require(get_port(e.seen_bytes)==(redirect?f.port:port),"replacement is network-order listener port");
        auto expected=old;if(redirect){expected[2]=std::byte(f.port>>8);expected[3]=std::byte(f.port&255);}
        require(std::equal(e.seen_bytes.begin(),e.seen_bytes.end(),expected.begin()),"IP/flow/scope and other address bytes retained");
      }
    for(auto family:{std::uint16_t{0},std::uint16_t{2},std::uint16_t{23},std::uint16_t{65535}})
      for(int length:{-1,0,15,16,27,28,29}) {
        if((family==2&&length==16)||(family==23&&length==28))continue;
        Fixture f;Fake e(f);auto b=make_address(family,3216);auto old=b;
        connect_00fd00(f.refs(),e,9,b,length);
        require(b==old&&e.events==std::vector<std::string>{"original"},"unrecognized address untouched");
      }
    // Exact IPv4 address, not every member of 127/8; mapped IPv6 is not ::1.
    {Fixture f;Fake e(f);auto b=make_address(2,3216);b[7]=std::byte{2};connect_00fd00(f.refs(),e,1,b,16);require(!e.has("wait"),"127.0.0.2 is not selected");}
    {Fixture f;Fake e(f);auto b=make_address(23,3216);b[18]=b[19]=std::byte{255};connect_00fd00(f.refs(),e,1,b,28);require(!e.has("wait"),"mapped/other IPv6 address not selected");}
}
void test_protected_observation_and_original_pointer(){
    // The inspected snapshot is not the original pointer. Only redirected
    // calls may forward a stack copy instead of the original opaque address.
    for(auto family:{std::uint16_t{2},std::uint16_t{23}}){
        Fixture f;Fake e(f);
        const int len=family==2?16:28;
        auto caller=make_address(family,443);
        auto observed=make_address(family,3216);
        const auto original=caller;
        const std::span<const std::byte> opaque{caller.data(),std::size_t{0}};
        const auto result=connect_00fd00_inspected(f.refs(),e,77,opaque,std::span{observed}.first(len),len);
        require(result==e.result&&e.connects==1&&e.seen_address!=caller.data()&&
                get_port(e.seen_bytes)==f.port&&f.attempts==1,
                "native decision uses observed snapshot and redirects through a full stack copy");
        require(caller==original,"snapshot classification never mutates caller storage");
    }
    for(auto family:{std::uint16_t{2},std::uint16_t{23}}){
        Fixture f;Fake e(f);const int len=family==2?16:28;
        auto caller=make_address(family,3216),observed=make_address(family,443);
        const std::span<const std::byte> opaque{caller.data(),std::size_t{0}};
        connect_00fd00_inspected(f.refs(),e,8,opaque,std::span{observed}.first(len),len);
        require(e.connects==1&&e.seen_address==caller.data()&&e.seen_bytes.empty()&&
                f.attempts==0&&!e.has("wait"),
                "non-target observation forwards exact original pointer/length without reading it");
    }
    for(auto family:{std::uint16_t{2},std::uint16_t{23}}){
        Fixture f;f.port=0;Fake e(f);const int len=family==2?16:28;
        auto caller=make_address(family,443),observed=make_address(family,3216);
        const auto original=caller;
        const std::span<const std::byte> opaque{caller.data(),std::size_t{0}};
        connect_00fd00_inspected(f.refs(),e,8,opaque,std::span{observed}.first(len),len);
        require(e.connects==1&&e.seen_address==caller.data()&&e.seen_bytes.empty()&&
                f.attempts==0&&e.events==std::vector<std::string>{"wait","original"}&&caller==original,
                "absent listener preserves opaque caller after wait without redirect effects");
    }
}
void test_dynamic_state(){
    {Fixture f;Fake e(f);e.change_wait=true;e.after_wait=4000;const auto b=make_address(2,3216);
     connect_00fd00(f.refs(),e,8,b,16);require(get_port(e.seen_bytes)==4000&&get_port(b)==3216,"wait publishes port into copy only");}
    {Fixture f;f.port=0;Fake e(f);auto b=make_address(2,3216);connect_00fd00(f.refs(),e,8,b,16);
     require(f.attempts==0&&get_port(e.seen_bytes)==3216&&e.seen_address==b.data(),"no invalid port-zero rewrite");}
    {Fixture f;Fake e(f);e.change_flush=true;e.after_flush=5555;auto b=make_address(2,3216);
     connect_00fd00(f.refs(),e,1,b,16);require(e.log_ports.back()==6099&&get_port(e.seen_bytes)==5555,"published port reread after logging");}
    for(auto initial:{0u,13u,14u,15u,0xfffffffeu,0xffffffffu}) {
        Fixture f;f.attempts=initial;Fake e(f);const auto b=make_address(23,3216);
        try{connect_00fd00(f.refs(),e,8,b,28);require(initial!=14,"15th attempt exits");}
        catch(const Exit& x){require(initial==14&&x.code==0xffffffffu,"15th attempt code retained");}
        require(f.attempts==initial+1u&&e.connects==(initial==14?0:1),"attempt synchronization/termination retained");
        require(get_port(b)==3216,"all attempts preserve const input");
    }
}
void test_bounds_and_failures(){
    {Fixture f;Fake e(f);require(connect_00fd00(f.refs(),e,9,{},-1)==e.result,"null invalid-length passthrough");}
    for(std::size_t size:{0u,1u,2u,3u,4u,7u,15u}) {
        Fixture f;Fake e(f);auto b=make_address(2,3216);
        try{connect_00fd00(f.refs(),e,9,std::span{b}.first(size),16);require(false,"partial sockaddr rejected");}
        catch(const OutOfEvidenceInput&){require(e.events.empty(),"bounds validated before effects");}
    }
    for(const char* failure:{"wait","literal:game connected on port ","port","flush"}) {
        Fixture f;Fake e(f);e.throw_at=failure;const auto b=make_address(2,3216);
        try{connect_00fd00(f.refs(),e,9,b,16);require(false,"effects failure propagates");}
        catch(const Injected&){require(get_port(b)==3216&&f.attempts==0&&e.connects==0,"failure never mutates caller memory");}
    }
}
#if !defined(PRELOADER_L_PRODUCTION_NETWORK_TESTS)
struct CleanupFake final : CleanupEffects{
    std::vector<std::string> events;std::vector<std::uintptr_t> seen;std::vector<std::int32_t> statuses;
    std::string throw_at;std::int32_t engine_status{};std::uintptr_t* mutate{};
    void event(std::string s){events.push_back(s);if(throw_at==s)throw Injected{};}
    std::int32_t disable_hook_046a00(std::uintptr_t a)override{seen.push_back(a);event("disable");if(mutate)*mutate=55;return statuses.empty()?0:statuses.at(seen.size()-1);}
    std::int32_t uninitialize_engine_046ac0()override{event("uninitialize");return engine_status;}
    void log_literal_006880(std::string_view s)override{event("literal:"+std::string(s));}
    void log_finish_0088e0()override{event("flush");}
};
void test_cleanup(){
    const std::vector<std::uintptr_t> targets{0,12,12,~std::uintptr_t{0}};
    for(auto engine:{0,-1,1,11}){
        CleanupFake e;e.engine_status=engine;e.statuses={1,0,-7,0};cleanup_hooks_0102b0(targets,e);
        require(e.seen==targets,"cleanup forward order, zero/invalid/duplicate targets retained");
        require(std::count(e.events.begin(),e.events.end(),"literal:disabling hook failed")==2,"each nonzero disable status logs but does not abort");
        require(e.events[e.events.size()-4]=="literal:MinHook uninitialization "&&e.events[e.events.size()-3]=="uninitialize","uninitialize prefix logged before actual engine call");
        require(e.events[e.events.size()-2]==(engine==0?"literal:succeeded":"literal:failed")&&e.events.back()=="flush","engine result logs/flushes");
    }
    {CleanupFake e;cleanup_hooks_0102b0({},e);require(e.seen.empty()&&e.events.size()==4,"empty target list still uninitializes engine");}
    {std::array<std::uintptr_t,4> data{1,2,3,4};CleanupFake e;e.mutate=&data[1];
     cleanup_hooks_0102b0(std::span{data}.first(2),e);
     require(e.seen==std::vector<std::uintptr_t>{1,55}&&data[2]==3,"stable span observes future element mutation and excludes appended region");}
    for(const char* failure:{"disable","literal:MinHook uninitialization ","uninitialize","literal:succeeded","flush"}){
        CleanupFake e;e.throw_at=failure;
        try{cleanup_hooks_0102b0(targets,e);require(false,"cleanup effect failure propagates");}
        catch(const Injected&){require(e.events.back()==failure,"cleanup has no invented exception rollback");}
    }
}
#endif
struct ExecutionPlatform final : preloader_l::execution::Platform {
    std::uint8_t& a;std::uint8_t& b;std::uint8_t& c;std::uint16_t& port;
    int acquires{},releases{},sleeps{};std::int64_t counter{};
    ExecutionPlatform(std::uint8_t& x,std::uint8_t& y,std::uint8_t& z,std::uint16_t& p):a(x),b(y),c(z),port(p){}
    std::uint32_t current_thread_id()override{return 77;}
    void acquire_srw_exclusive(std::uintptr_t&)override{++acquires;}
    void release_srw_exclusive(std::uintptr_t&)override{++releases;}
    std::int64_t query_performance_frequency()override{return 10'000'000;}
    std::int64_t query_performance_counter()override{return counter+=10000;}
    void sleep_milliseconds(std::uint32_t n)override{++sleeps;counter+=std::int64_t(n)*10000;a=b=c=1;port=4000;}
    [[noreturn]]void raise_thread_error_04a648(int)override{throw Injected{};}
};
void test_execution_composition(){
    using namespace preloader_l::execution;
    Fixture f;Fake remaining(f);MutexState mutex{};initialize_mutex_04a698(mutex,1);
    std::uint8_t a=0,b=0,c=0;std::int64_t frequency=0;ExecutionPlatform platform(a,b,c,f.port);
    Execution execution(mutex,{a,b,c},frequency,platform);ExecutionEffects effects(remaining,execution);
    auto address=make_address(23,3216);
    require(connect_00fd00(f.refs(),effects,99,address,28)==remaining.result,"connect composes actual recovered synchronization core");
    require(platform.sleeps>0&&platform.acquires>=2&&platform.acquires==platform.releases,"wait loops under recovered mutex and observes synthetic ready transition");
    require(!remaining.has("wait")&&get_port(remaining.seen_bytes)==4000&&get_port(address)==3216&&f.attempts==1,"wait core replaces external wait and published port reaches callback");
}
}
int main(){try{test_decisions();test_protected_observation_and_original_pointer();test_dynamic_state();test_bounds_and_failures();
#if !defined(PRELOADER_L_PRODUCTION_NETWORK_TESTS)
    test_cleanup();
#endif
    test_execution_composition();
 std::cout<<"PASS: "<<checks<<" connect fixture checks; synthetic effects, no sockets/hooks/process exit\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected test exception\n";return 1;}}
