#include "preloader_l/net/server.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cstdlib>
#include <new>
namespace greeting_lifetime {
const void* tracked=nullptr;
void* candidate=nullptr;
std::size_t candidate_bytes=0,minimum_bytes=0;
bool capturing=false;
unsigned released=0;
void release(void* pointer) noexcept {
    if(pointer&&pointer==tracked){tracked=nullptr;++released;}
    std::free(pointer);
}
}
void* operator new(std::size_t size){
    if(auto* p=std::malloc(size?size:1)){
        if(greeting_lifetime::capturing&&size>=greeting_lifetime::minimum_bytes){
            greeting_lifetime::candidate=p;greeting_lifetime::candidate_bytes=size;
        }
        return p;
    }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size){return ::operator new(size);}
void operator delete(void* p) noexcept {greeting_lifetime::release(p);}
void operator delete[](void* p) noexcept {greeting_lifetime::release(p);}
void operator delete(void* p,std::size_t) noexcept {greeting_lifetime::release(p);}
void operator delete[](void* p,std::size_t) noexcept {greeting_lifetime::release(p);}
using namespace preloader_l::server_recovered;
namespace {
int checks=0;
void check(bool value,const char* text) {++checks;if(!value)throw std::runtime_error(text);}
struct Raised {Error error; std::string context;};
struct FakeExecutionPlatform final : preloader_l::execution_recovered::Platform {
    int acquired=0,released=0;
    std::uint32_t current_thread_id() override {return 7;}
    void acquire_srw_exclusive(std::uintptr_t&) override {++acquired;}
    void release_srw_exclusive(std::uintptr_t&) override {++released;}
    std::int64_t query_performance_frequency() override {throw std::runtime_error("unexpected QPF");}
    std::int64_t query_performance_counter() override {throw std::runtime_error("unexpected QPC");}
    void sleep_milliseconds(std::uint32_t) override {throw std::runtime_error("unexpected sleep");}
    [[noreturn]] void raise_thread_error_04a648(int) override {throw std::runtime_error("unexpected mutex error");}
};
struct FakeServer final : ServerBackend {
    std::vector<std::string> events;
    ParsedAddress ipv6{0,{10022,ErrorCategory::system},{},0};
    ParsedAddress ipv4{1,{},{127,0,0,1},0};
    LocalEndpoint local{{},0x3930,28}; // network byte representation of12345 on x64
    Endpoint observed;
    Error run_error;
    bool env_result=true;
    std::uint8_t* ready=nullptr;
    void initialize_context_0241b0() override {events.emplace_back("context");}
    ParsedAddress parse_address_028690(std::uint16_t family,std::string_view literal) override {
        check(literal=="127.0.0.1","exact loopback literal");
        events.emplace_back(family==23?"parse6":"parse4");return family==23?ipv6:ipv4;
    }
    void construct_acceptor_020e20(const Endpoint& e,bool reuse,int backlog) override {
        observed=e;events.emplace_back("acceptor");
        check(reuse&&backlog==0x7fffffff,"reuse-address and observed listen backlog");
    }
    void initialize_pending_socket_0211b0() override {events.emplace_back("pending");}
    LocalEndpoint local_endpoint() override {events.emplace_back("local");return local;}
    bool set_environment_variable(std::string_view key,std::string_view value) override {
        check(key=="EALsxPort"&&value=="12345","bound port published as decimal");
        check(!ready||*ready==0,"environment before ready flag");events.emplace_back("environment");return env_result;
    }
    void async_accept_021b00() override {
        check(!ready||*ready==1,"readiness before initial accept");events.emplace_back("accept");
    }
    Error run_context_0298b0() override {events.emplace_back("run");return run_error;}
    void destroy_pending_socket_024db0() override {events.emplace_back("destroy_pending");}
    void destroy_acceptor_024db0() override {events.emplace_back("destroy_acceptor");}
    void destroy_context() override {events.emplace_back("destroy_context");}
    [[noreturn]] void raise_error(Error error,std::string_view context) override {throw Raised{error,std::string(context)};}
};
struct FakeScheduling final : Scheduling {
    std::deque<std::function<void()>> io,workers,write_dispatch;
    std::vector<std::string> errors;
    bool launch_fails=false;
    int launches=0;
    void post_io(std::function<void()> f) override {io.push_back(std::move(f));}
    void launch_detached(std::function<void()> f) override {
        ++launches;if(launch_fails)throw Raised{{22,ErrorCategory::other},"detach"};
        workers.push_back(std::move(f));
    }
    void dispatch_write(std::function<void()> f) override {write_dispatch.push_back(std::move(f));}
    void log_error(std::string_view operation,Error) override {errors.emplace_back(operation);}
    [[noreturn]] void raise_write_error(Error e) override {throw Raised{e,"write"};}
    void one(std::deque<std::function<void()>>& queue) {
        check(!queue.empty(),"scheduled job exists");auto f=std::move(queue.front());queue.pop_front();f();
    }
};
struct FakeHandler final : Handler {
    int resets=0;
    std::string initial;
    std::vector<std::string> requests;
    std::string reply="response";
    void reset_02bc70() override {++resets;}
    std::string initial_message_02d540() override {return initial;}
    std::string process_message_033d70(std::string_view s) override {requests.emplace_back(s);return reply;}
};
struct FakeTransport final : Transport {
    std::function<void(Error,std::string)> receive;
    Completion send;
    std::size_t requested=0;
    std::vector<std::string> send_requests;
    std::string greeting;
    std::string wire;
    std::size_t sync_limit=0x10000;
    bool sync_zero_once=false;
    int sync_calls=0;
    Error sync_error;
    std::function<void(std::span<const char>)> observe_greeting;
    std::function<void()> observe_read;
    WriteResult synchronous_send(std::span<const char> bytes) override {
        ++sync_calls;
        if(observe_greeting)observe_greeting(bytes);
        check(bytes.size()<=0x10000,"greeting chunks capped64KiB");
        if(sync_zero_once){sync_zero_once=false;return {};}
        const auto length=std::min(bytes.size(),sync_limit);
        greeting.append(bytes.data(),length);return {sync_error,length};
    }
    void async_receive(std::size_t size,std::function<void(Error,std::string)> callback) override {
        if(observe_read)observe_read();
        check(!receive,"no concurrent read operation");requested=size;receive=std::move(callback);
    }
    void async_send(std::span<const char> bytes,Completion callback) override {
        check(!send,"no concurrent send operation");check(bytes.size()<=0x10000,"send chunks capped64KiB");
        send_requests.emplace_back(bytes.data(),bytes.size());send=std::move(callback);
    }
    void feed(std::string bytes,Error error={}) {
        check(static_cast<bool>(receive),"receive callback armed");
        check(bytes.size()<=requested,"test respects receive capacity");auto f=std::move(receive);receive={};f(error,std::move(bytes));
    }
    void complete_send(std::size_t length,Error error={}) {
        check(static_cast<bool>(send),"send callback armed");
        check(length<=send_requests.back().size(),"completion count bounded by submitted send");
        wire.append(send_requests.back().data(),length);
        auto f=std::move(send);send={};f(error,length);
    }
};
struct Fixture {
    FakeTransport transport;
    FakeScheduling scheduling;
    FakeHandler handler;
    Session session{transport,scheduling,handler};
};
void server_tests() {
    FakeExecutionPlatform platform;
    preloader_l::execution_recovered::MutexState mutex{};
    preloader_l::execution_recovered::initialize_mutex_04a698(mutex,2);
    std::uint8_t first=0,second=0,third=0;
    std::int64_t frequency=0;
    preloader_l::execution_recovered::Execution execution{mutex,{first,second,third},frequency,platform};
    FakeServer server;server.ready=&first;server.env_result=false;
    std::uint16_t port=0;
    server_start_029ea0(server,port,first,execution);
    check(port==12345&&first==1&&second==0&&third==0,"actual port and single readiness flag");
    check(platform.acquired==1&&platform.released==1,"readiness uses recovered mutex path");
    check(server.observed.family==2&&server.observed.address[0]==127&&server.observed.address[3]==1,"IPv4 loopback fallback");
    check(server.observed.port_host_order==0,"caller supplied ephemeral port reaches bind");
    check(server.events==std::vector<std::string>({"context","parse6","parse4","acceptor","pending","local","environment","accept","run","destroy_pending","destroy_acceptor","destroy_context"}),"normal startup/cleanup order");
    FakeServer six;six.ipv6={1,{},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},9};
    first=0;port=7777;construct_server_024480(six,port,first,execution);
    check(six.observed.family==23&&six.observed.scope_id==9&&six.observed.port_host_order==7777,"IPv6 parse success uses observed fields");
    check(std::find(six.events.begin(),six.events.end(),"parse4")==six.events.end(),"no IPv4 fallback on successful parse6");
    for(auto mode:{0,1,2}) {
        FakeServer broken;first=0;port=11;
        if(mode==0)broken.ipv4.error={7,ErrorCategory::other};
        if(mode==1)broken.local.error={0x2719,ErrorCategory::system};
        if(mode==2)broken.local.returned_size=129;
        try {construct_server_024480(broken,port,first,execution);check(false,"expected startup failure");}
        catch(const Raised& e){check(e.error.value!=0,"startup preserves nonzero error");}
        check(first==0&&port==11,"failed startup does not publish readiness/port");
        check(std::find(broken.events.begin(),broken.events.end(),"accept")==broken.events.end(),"failed startup does not accept");
    }
}
void read_tests() {
    Fixture f;check(f.handler.resets==1,"session constructor initializes Handler");
    f.handler.initial=std::string("hello\0",6);f.transport.sync_limit=2;
    f.session.start_025720();
    check(f.handler.resets==2&&f.transport.greeting==f.handler.initial,"accepted session resets Handler and sends exact greeting including nul");
    f.transport.feed("par");check(f.session.buffered_input()=="par"&&f.scheduling.workers.empty(),"partial message retained without dispatch");
    f.transport.feed(std::string("tial\0second\0tail",16));
    check(f.scheduling.io.size()==1,"coalesced frames complete through queued IO");
    f.scheduling.one(f.scheduling.io);
    check(f.scheduling.workers.size()==1&&f.session.buffered_input()==std::string_view("second\0tail",11),"consume first delimiter and preserve trailing frame/partial bytes");
    f.scheduling.one(f.scheduling.io);
    check(f.scheduling.workers.size()==2&&f.session.buffered_input()=="tail"&&static_cast<bool>(f.transport.receive),"second complete frame dispatched and read rearmed for suffix");
    // Detached worker order can differ from arrival order. Queue follows actual
    // processing/dispatch order, not a fabricated total order across workers.
    auto second=std::move(f.scheduling.workers.back());f.scheduling.workers.pop_back();second();
    f.scheduling.one(f.scheduling.workers);
    check(f.handler.requests==std::vector<std::string>({"second","partial"}),"independent workers may process out of order");
    while(!f.scheduling.write_dispatch.empty())f.scheduling.one(f.scheduling.write_dispatch);
    check(f.session.queued_messages()==2&&f.transport.send_requests.size()==1,"one send starts for two queued replies");
    Fixture empty;empty.session.do_read_0280d0();empty.transport.feed(std::string(1,'\0'));
    empty.scheduling.one(empty.scheduling.io);empty.scheduling.one(empty.scheduling.workers);
    check(empty.handler.requests.empty()&&empty.scheduling.write_dispatch.empty(),"empty frame still launches worker but produces no Handler call/reply");
    Fixture fail;fail.session.do_read_0280d0();fail.transport.feed(std::string("x\0",2));fail.scheduling.launch_fails=true;
    try {fail.scheduling.one(fail.scheduling.io);check(false,"expected detach failure");}catch(const Raised&){}
    check(fail.session.buffered_input()==std::string_view("x\0",2)&&!fail.transport.receive,"launch failure preserves frame and stops rearm");
    for(auto category:{ErrorCategory::asio_misc,ErrorCategory::system}) {
        Fixture error;error.session.do_read_0280d0();error.transport.feed(std::string("partial\0",8),{2,category});
        check(error.scheduling.workers.empty()&&!error.transport.receive,"read error leaves partial input unprocessed and stops rearm");
        check(error.session.buffered_input()==std::string_view("partial\0",8)&&error.scheduling.io.empty(),"read error wins over delimiter and preserves received bytes");
        check(error.scheduling.errors.size()==(category==ErrorCategory::asio_misc?0u:1u),"only misc EOF2 suppresses read error log");
    }
    const std::string framed("a\0\0bcd\0",7);
    for(std::size_t split=1;split<framed.size();++split) {
        Fixture fragment;fragment.session.do_read_0280d0();fragment.transport.feed(framed.substr(0,split));
        while(!fragment.scheduling.io.empty())fragment.scheduling.one(fragment.scheduling.io);
        fragment.transport.feed(framed.substr(split));
        while(!fragment.scheduling.io.empty())fragment.scheduling.one(fragment.scheduling.io);
        while(!fragment.scheduling.workers.empty())fragment.scheduling.one(fragment.scheduling.workers);
        check(fragment.handler.requests==std::vector<std::string>({"a","bcd"}),"all transport split positions yield identical nonempty requests");
        check(fragment.session.buffered_input().empty(),"all frames consumed at each split position");
    }
}
void write_tests() {
    Fixture f;std::string big(140000,'z');
    for(std::size_t index=0;index<big.size();++index)big[index]=static_cast<char>(index%251);
    f.session.do_write_028270(big);f.session.do_write_028270("next");
    check(f.transport.send_requests.back().size()==65536&&f.session.queued_messages()==2,"first capped write and FIFO append");
    f.transport.complete_send(10);check(f.session.written_offset()==10&&f.transport.send_requests.back().size()==65536,"partial completion advances offset and retries remaining capped span");
    f.transport.complete_send(65536);check(f.transport.send_requests.back().size()==65536,"second capped span");
    f.transport.complete_send(65536);check(f.transport.send_requests.back().size()==8918,"tail span length");
    f.transport.complete_send(8918);check(f.session.queued_messages()==1&&f.transport.send_requests.back()=="next","message completion pops then starts next FIFO item");
    f.transport.complete_send(4);check(f.session.queued_messages()==0&&!f.transport.send,"final completion drains queue");
    check(f.transport.wire==big+"next","partial/capped writes preserve exact binary bytes and FIFO order");
    Fixture zero;zero.session.do_write_028270("not all sent");zero.transport.complete_send(0);
    check(zero.session.queued_messages()==0,"observed successful zero-progress completion pops message");
    Fixture empty;empty.session.do_write_028270("");check(empty.transport.send_requests.back().empty(),"empty reply still initiates zero-size send");empty.transport.complete_send(0);
    check(empty.session.queued_messages()==0,"empty reply callback drains item");
    for(auto category:{ErrorCategory::asio_misc,ErrorCategory::system}) {
        Fixture error;error.session.do_write_028270("retained");error.transport.complete_send(2,{2,category});
        check(error.session.queued_messages()==1&&!error.transport.send,"write error retains queue without retry");
        error.session.do_write_028270("new");check(error.session.queued_messages()==2&&!error.transport.send,"later enqueue does not restart failed nonempty queue");
        check(error.scheduling.errors.size()==(category==ErrorCategory::asio_misc?0u:1u),"only misc EOF2 suppresses write error log");
    }
}
void sizing_and_accept_tests() {
    check(receive_request_size(0,15,100000)==512,"initial dynamic-buffer receive request");
    check(receive_request_size(1,100000,200000)==65536,"receive request max64KiB");
    check(receive_request_size(99,100,100)==1,"receive request bounded by remaining maximum");
    check(receive_request_size(100,100,100)==0,"full dynamic buffer yields not-found boundary");
    for(const std::size_t maximum:{std::size_t{1},std::size_t{7},std::size_t{512},std::size_t{65536},std::size_t{65537},std::numeric_limits<std::size_t>::max()}) {
        for(const auto size:{std::size_t{0},maximum/2,maximum-1}) {
            for(const auto capacity:{std::size_t{0},size,maximum,std::numeric_limits<std::size_t>::max()}) {
                const auto count=receive_request_size(size,capacity,maximum);
                check(count>0&&count<=maximum-size&&count<=65536,"remaining input capacity always yields a positive bounded receive request");
            }
        }
    }
    FakeServer server;Fixture session;
    accept_completion_025720({9,ErrorCategory::other},nullptr,server,session.scheduling);
    check(session.scheduling.errors==std::vector<std::string>({"accept"})&&server.events.back()=="accept","accept failure logs then immediately rearms");
    accept_completion_025720({},&session.session,server,session.scheduling);
    check(session.handler.resets==2&&session.transport.receive&&server.events.size()==2,"accept success starts supplied constructed session then rearms");
    Fixture greeting;greeting.handler.initial=std::string(150001,'g');greeting.handler.initial.back()='\0';
    greeting.session.start_025720();
    check(greeting.transport.greeting==greeting.handler.initial,"large initial message fully sent across64KiB chunks");
    Fixture retry;retry.handler.initial="abc";retry.transport.sync_zero_once=true;retry.session.start_025720();
    check(retry.transport.sync_calls==2&&retry.transport.greeting=="abc","greeting zero-success does not complete operation; next call continues");
    Fixture fail;fail.handler.initial="greeting";fail.transport.sync_error={54,ErrorCategory::system};
    try{accept_completion_025720({},&fail.session,server,fail.scheduling);check(false,"expected greeting error");}
    catch(const Raised& e){check(e.context=="write"&&e.error.value==54,"blocking greeting send error is raised");}
    check(!fail.transport.receive&&server.events.size()==2,"greeting failure prevents session read and accept rearm");
}
}
void greeting_release_before_read() {
    for(bool fail:{false,true}) {
        Fixture f;f.handler.initial.assign(150001,'g');
        struct Reset {~Reset(){greeting_lifetime::tracked=nullptr;greeting_lifetime::capturing=false;}} reset;
        greeting_lifetime::released=0;bool armed=false,read=false;
        greeting_lifetime::candidate=nullptr;greeting_lifetime::capturing=true;
        greeting_lifetime::minimum_bytes=f.handler.initial.size();
        f.transport.observe_greeting=[&](std::span<const char> bytes){
            if(!armed){
                // A standard library may align the character buffer inside a
                // larger allocation. Observe that containing new/delete pair,
                // rather than assuming data() is the operator-new pointer.
                const auto base=reinterpret_cast<std::uintptr_t>(greeting_lifetime::candidate);
                const auto data=reinterpret_cast<std::uintptr_t>(bytes.data());
                check(base&&data>=base&&data-base<greeting_lifetime::candidate_bytes,
                    "greeting buffer belongs to the observed temporary allocation");
                greeting_lifetime::tracked=greeting_lifetime::candidate;
                greeting_lifetime::capturing=false;armed=true;
            }
            check(greeting_lifetime::released==0,"greeting lives through every synchronous send");
        };
        f.transport.observe_read=[&]{read=true;check(greeting_lifetime::released==1,"greeting storage released before first read initiation");};
        if(fail){
            f.transport.sync_error={54,ErrorCategory::system};bool raised=false;
            try{f.session.start_025720();}catch(const Raised& e){raised=e.error.value==54;}
            check(raised&&!read,"send failure retains original error and does not start reading");
        }else{
            f.session.start_025720();check(read&&f.transport.greeting==f.handler.initial,"full greeting precedes first read");
        }
        check(greeting_lifetime::released==1,"greeting allocation released once on normal and throwing paths");
    }
}
int main() {
    try{server_tests();read_tests();write_tests();sizing_and_accept_tests();greeting_release_before_read();std::cout<<"PASS "<<checks<<" synthetic checks; no socket/network/original target execution\n";return 0;}
    catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}
    catch(const Raised& e){std::cerr<<"FAIL unexpected Raised "<<e.context<<'\n';return 1;}
}
