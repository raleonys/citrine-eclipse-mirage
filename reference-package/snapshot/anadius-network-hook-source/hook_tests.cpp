#include "connect.hpp"
#include "hook_cleanup.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace anadius::network_hook_recovered;
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
    b[2]=static_cast<std::byte>(port>>8);b[3]=static_cast<std::byte>(port&255);return b;
}
std::uint16_t get_port(std::span<const std::byte> b){return static_cast<std::uint16_t>((std::to_integer<unsigned>(b[2])<<8)|std::to_integer<unsigned>(b[3]));}
struct Fake final : Effects {
    std::vector<std::string> events;std::vector<std::uint16_t> log_ports;
    Fixture& f;std::string throw_at;std::uint16_t after_wait{},after_flush{};bool change_wait{},change_flush{};
    std::int32_t result{-10035};int connects{};Socket seen_socket{};std::byte* seen_address{};std::int32_t seen_length{};
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
    std::int32_t original_connect_0a3838(Socket s,std::span<std::byte> a,std::int32_t n) override {
        ++connects;seen_socket=s;seen_address=a.data();seen_length=n;event("original");return result;
    }
    bool has(const char* s)const{return std::find(events.begin(),events.end(),s)!=events.end();}
};
void test_decisions(){
    struct Case{std::uint16_t requested,server;bool wait,redirect;};
    const Case cases[]={{80,6099,false,false},{443,6099,false,false},{80,80,false,true},
        {443,443,false,true},{80,443,false,false},{443,80,false,false},{3216,6099,true,true},
        {6099,6099,true,true},{6098,6099,true,false},{0,0,true,true},{0,6099,true,false},
        {3216,0,true,true},{65535,65535,true,true},{65535,6099,true,false},{1,1,true,true}};
    for(const auto& c:cases)for(auto family:{std::uint16_t{2},std::uint16_t{23}}){
        Fixture f;f.port=c.server;Fake e(f);auto b=make_address(family,c.requested);auto expected=b;
        if(c.redirect){expected[2]=static_cast<std::byte>(c.server>>8);expected[3]=static_cast<std::byte>(c.server&255);}
        const auto length=family==2?16:28;
        require(connect_00fd00(f.refs(),e,0x12345678,b,length)==e.result,"original connect int32 result forwarded");
        require(e.has("wait")==c.wait,"HTTP/HTTPS wait exception and all other ports wait");
        require(e.has("flush")==c.redirect && f.attempts==static_cast<unsigned>(c.redirect),"only redirected attempts counted/logged");
        require(b==expected,"only network-order port changes, all IP/scope/trailing bytes preserved");
        require(e.connects==1&&e.seen_socket==0x12345678&&e.seen_address==b.data()&&e.seen_length==length,"original socket/pointer/length forwarded exactly once");
        require(!e.has("dialog"),"normal attempts do not show UI");
        if(c.redirect)require(e.log_ports==std::vector<std::uint16_t>{c.requested,c.server},"log retains original port then observed current server port");
    }
    for(auto family:{std::uint16_t{0},std::uint16_t{2},std::uint16_t{23},std::uint16_t{65535}})
      for(auto length:{-1,0,15,16,27,28,29}) {
        if((family==2&&length==16)||(family==23&&length==28))continue;
        Fixture f;Fake e(f);auto b=make_address(family,3216);const auto old=b;
        connect_00fd00(f.refs(),e,9,b,length);
        require(b==old&&f.attempts==0&&e.events==std::vector<std::string>{"original"},"unrecognized family/length bypasses wait/mutation/log");
    }
}
void test_dynamic_state(){
    for(auto input:{std::uint16_t{3216},std::uint16_t{4000},std::uint16_t{6099}}){
        Fixture f;Fake e(f);e.change_wait=true;e.after_wait=4000;auto b=make_address(2,input);
        connect_00fd00(f.refs(),e,8,b,16);
        require(f.attempts==static_cast<unsigned>(input==3216||input==4000),"server port comparison happens after wait");
        require(get_port(b)==(input==3216||input==4000?4000:input),"port captured before wait; new server port used after wait");
    }
    {Fixture f;Fake e(f);e.change_flush=true;e.after_flush=5555;auto b=make_address(2,3216);
     connect_00fd00(f.refs(),e,1,b,16);
     require(e.log_ports.back()==6099&&get_port(b)==5555,"server port read again after logging before rewrite");}
    for(auto initial:{0u,13u,14u,15u,0xfffffffeu,0xffffffffu}){
        Fixture f;f.attempts=initial;Fake e(f);auto b=make_address(23,3216);const auto old=b;
        try{connect_00fd00(f.refs(),e,8,b,28);require(initial!=14,"15th attempt must terminate");}
        catch(const Exit& x){require(initial==14&&x.code==0xffffffffu,"15th attempt exit unsigned code");}
        require(f.attempts==initial+1u,"32bit attempt counter wraps");
        require(e.connects==(initial==14?0:1),"exit before port mutation/original connect");
        if(initial==14)require(b==old&&e.has("dialog")&&e.has("exit"),"terminal attempt preserves input address");
    }
    {Fixture f;f.v4=4;f.v6=4;Fake e(f);auto b=make_address(23,3216);connect_00fd00(f.refs(),e,8,b,4);
     require(f.attempts==1,"family tests preserve short-circuit alternative when configured sizes equal");}
}
void test_bounds_and_failures(){
    {Fixture f;Fake e(f);require(connect_00fd00(f.refs(),e,9,{},-1)==e.result,"mismatched size may forward null address without read");}
    for(std::size_t size:{0u,1u,2u,3u}){
        Fixture f;Fake e(f);auto b=make_address(2,3216);
        try{connect_00fd00(f.refs(),e,9,std::span{b}.first(size),16);require(false,"short inspected header rejected");}
        catch(const OutOfEvidenceInput&){require(e.events.empty()&&f.attempts==0,"bounds stop before external effects");}
    }
    {Fixture f;Fake e(f);auto b=make_address(99,3216);connect_00fd00(f.refs(),e,9,std::span{b}.first(2),16);
     require(e.connects==1,"invalid family only needs two readable bytes");}
    for(const char* failure:{"wait","literal:game connected on port ","port","flush"}){
        Fixture f;Fake e(f);e.throw_at=failure;auto b=make_address(2,3216);const auto old=b;
        try{connect_00fd00(f.refs(),e,9,b,16);require(false,"effect exception propagated");}
        catch(const Injected&){require(b==old&&f.attempts==0&&e.connects==0,"exception before counter leaves state/address; no original call");}
    }
    {Fixture f;f.attempts=14;Fake e(f);e.throw_at="dialog";auto b=make_address(2,3216);const auto old=b;
     try{connect_00fd00(f.refs(),e,9,b,16);}catch(const Injected&){require(f.attempts==15&&b==old&&!e.has("exit"),"dialog exception keeps increment but skips exit/write/callback");}}
}
void test_prefix_classification(){
    require(original_only_00fd00(16,28,{},-1),"unknown length classification does not access null backing");
    for(auto family:{std::uint16_t{0},std::uint16_t{2},std::uint16_t{23},std::uint16_t{99}}){
        auto address=make_address(family,3216);const auto old=address;
        for(int length:{-1,16,28})require(original_only_00fd00(16,28,std::span{address}.first(2),length)==
            !((length==16&&family==2)||(length==28&&family==23)),"family-only classification needs no port bytes");
        require(address==old,"classification never mutates caller backing");
    }
    auto equal=make_address(23,80);
    require(!original_only_00fd00(4,4,std::span{equal}.first(2),4),"equal lengths preserve second family alternative");
    require(!original_only_00fd00(16,28,make_address(2,80),16)&&!original_only_00fd00(16,28,make_address(2,443),16),
        "HTTP/HTTPS are shared-state paths even when they skip wait");
    for(std::uint16_t port:{0,80,443,3216,65535})require(requested_port_00fd00(make_address(2,port))==port,"pure extraction retains requested network-order port");
    try{requested_port_00fd00(std::span{equal}.first(3));require(false,"short port prefix rejected");}
    catch(const OutOfEvidenceInput&){}
}
void test_redirect_body_split(){
    Fixture f;Fake e(f);auto address=make_address(2,3216);e.change_flush=true;e.after_flush=5555;
    redirect_body_00fd00(e,address,3216,f.port,f.attempts);
    require(f.attempts==1&&get_port(address)==5555&&e.connects==0&&!e.has("wait"),"extracted body keeps post-log live port reread and has no wait/original call");
    Fixture terminal;terminal.attempts=14;Fake end(terminal);auto unchanged=make_address(2,3216);const auto old=unchanged;
    try{redirect_body_00fd00(end,unchanged,3216,terminal.port,terminal.attempts);require(false,"terminal body must not return");}
    catch(const Exit& value){require(value.code==0xffffffffu&&terminal.attempts==15&&unchanged==old&&end.connects==0,"terminal15 leaves sockaddr/original untouched after increment");}
    Fixture short_input;Fake bound(short_input);auto prefix=make_address(2,3216);
    try{redirect_body_00fd00(bound,std::span{prefix}.first(3),3216,short_input.port,short_input.attempts);require(false,"body requires full writable prefix");}
    catch(const OutOfEvidenceInput&){require(bound.events.empty()&&short_input.attempts==0,"short body rejects before any effect");}
}
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
struct ExecutionPlatform final : anadius::execution_recovered::Platform {
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
    using namespace anadius::execution_recovered;
    Fixture f;Fake remaining(f);MutexState mutex{};initialize_mutex_04a698(mutex,1);
    std::uint8_t a=0,b=0,c=0;std::int64_t frequency=0;ExecutionPlatform platform(a,b,c,f.port);
    Execution execution(mutex,{a,b,c},frequency,platform);ExecutionEffects effects(remaining,execution);
    auto address=make_address(23,3216);
    require(connect_00fd00(f.refs(),effects,99,address,28)==remaining.result,"connect composes actual recovered synchronization core");
    require(platform.sleeps>0&&platform.acquires>=2&&platform.acquires==platform.releases,"wait loops under recovered mutex and observes synthetic ready transition");
    require(!remaining.has("wait")&&get_port(address)==4000&&f.attempts==1,"wait core replaces external wait and published port reaches callback");
}
}
int main(){try{test_decisions();test_dynamic_state();test_bounds_and_failures();test_prefix_classification();test_redirect_body_split();test_cleanup();test_execution_composition();
 std::cout<<"PASS: "<<checks<<" connect/cleanup checks; synthetic effects, no sockets/hooks/process exit\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected test exception\n";return 1;}}
