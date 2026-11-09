// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/asio_service.hpp"
#include "citrine_eclipse_mirage/platform/execution_windows.hpp"
#include <asio.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace citrine_eclipse_mirage;
using namespace std::chrono_literals;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>bool until(F fn,std::chrono::milliseconds timeout=3000ms){auto end=std::chrono::steady_clock::now()+timeout;do{if(fn())return true;Sleep(1);}while(std::chrono::steady_clock::now()<end);return false;}
struct Factory final:net::HandlerFactory {
    std::atomic<unsigned> created{0},timeouts{0},requests{0},next_requests{0},limits{0};
    std::atomic<bool> large_first_greeting{false};
    struct Handler final:server::Handler {
        Factory& f;unsigned id;
        Handler(Factory& factory,unsigned number):f(factory),id(number){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override {
            if(f.large_first_greeting&&id==1)return std::string(8*1024*1024,'X');
            return std::string("hello\0",6);
        }
        std::string process_message_033d70(std::string_view request)override {
            ++f.requests;if(request=="next")++f.next_requests;
            if(request=="huge")return std::string(8*1024*1024,'X');
            std::string result(request);result.push_back('\0');return result;
        }
    };
    std::unique_ptr<server::Handler> create()override{return std::make_unique<Handler>(*this,++created);}
    void error(std::string_view operation,std::exception_ptr error)noexcept override {
        if(operation=="session timeout")++timeouts;
        try{if(error)std::rethrow_exception(error);}catch(const std::length_error&){++limits;}catch(...){}
    }
};
struct Harness {
    Factory factory;execution::MutexState mutex{};
    std::uint8_t a=0,b=0,c=0;std::uint16_t port=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    net::AsioService service;
    explicit Harness(net::SessionPolicy policy={}):service(factory,execution,port,a,policy){execution::initialize_mutex_04a698(mutex,2);service.start();}
};
void policy_construction_validation() {
    Factory factory;execution::MutexState mutex{};
    std::uint8_t ready=0x6b,b=0,c=0;std::uint16_t port=0x5a5a;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{ready,b,c},frequency,platform};
    execution::initialize_mutex_04a698(mutex,2);
    const auto construct=[&](const net::SessionPolicy& policy) {
        net::AsioService service(factory,execution,port,ready,policy);
        require(port==0x5a5a&&ready==0x6b,"construction does not publish a listener or readiness");
        require(factory.created==0&&factory.requests==0&&factory.timeouts==0&&factory.limits==0,
                "construction does not dispatch handlers or report session errors");
    };
    const auto reject=[&](const net::SessionPolicy& policy) {
        bool refused=false;
        try{construct(policy);}catch(const std::invalid_argument&){refused=true;}
        require(refused,"actual AsioService constructor rejects invalid policy before start");
        require(port==0x5a5a&&ready==0x6b&&factory.created==0&&factory.requests==0&&
                factory.timeouts==0&&factory.limits==0,"invalid construction preserves caller state and handler counters");
    };
    struct Resource {std::size_t net::SessionPolicy::*field;std::size_t maximum;};
    const std::array resources{
        Resource{&net::SessionPolicy::max_sessions,65535},
        Resource{&net::SessionPolicy::max_frame_bytes,1024ull*1024*1024},
        Resource{&net::SessionPolicy::max_response_bytes,1024ull*1024*1024}};
    for(const auto& resource:resources)for(const auto value:{std::size_t{0},resource.maximum+1}) {
        net::SessionPolicy policy;policy.*resource.field=value;reject(policy);
    }
    for(const auto field:{&net::SessionPolicy::receive_idle,&net::SessionPolicy::receive_frame,
                          &net::SessionPolicy::send_idle})
        for(const auto value:{-1ms,0ms,24h+1ms}) {
            net::SessionPolicy policy;policy.*field=value;reject(policy);
        }
    construct(net::SessionPolicy{});
    for(bool maximum:{false,true}) {
        net::SessionPolicy policy;
        for(const auto& resource:resources)policy.*resource.field=maximum?resource.maximum:1;
        policy.receive_idle=policy.receive_frame=policy.send_idle=maximum?24h:1ms;
        construct(policy);
    }
}
struct Peer {
    asio::io_context io;asio::ip::tcp::socket socket{io};std::string input;
    explicit Peer(std::uint16_t port,bool small_window=false,bool ipv6=false){socket.open(ipv6?asio::ip::tcp::v6():asio::ip::tcp::v4());if(small_window)socket.set_option(asio::socket_base::receive_buffer_size(1024));const asio::ip::address address=ipv6?asio::ip::address(asio::ip::address_v6::loopback()):asio::ip::address(asio::ip::address_v4::loopback());socket.connect({address,port});socket.non_blocking(true);}
    void close(){asio::error_code e;socket.close(e);}
    void raw(std::string_view bytes){socket.non_blocking(false);asio::write(socket,asio::buffer(bytes));socket.non_blocking(true);}
    void send(std::string bytes){bytes.push_back('\0');raw(bytes);}
    std::string frame(){std::string result;const bool done=until([&]{
        auto end=input.find('\0');if(end!=std::string::npos){result=input.substr(0,end);input.erase(0,end+1);return true;}
        char bytes[4096];asio::error_code error;const auto size=socket.read_some(asio::buffer(bytes),error);
        if(!error)input.append(bytes,size);else if(error!=asio::error::would_block&&error!=asio::error::try_again)throw asio::system_error(error);
        return false;
    });require(done,"wire frame received before timeout");return result;}
    bool closed(){return until([&]{char bytes[4096];asio::error_code error;socket.read_some(asio::buffer(bytes),error);return error&&error!=asio::error::would_block&&error!=asio::error::try_again;});}
};
void session_cap() {
    net::SessionPolicy p;p.max_sessions=2;Harness h(p);
    Peer first(h.port),second(h.port);require(first.frame()=="hello"&&second.frame()=="hello","first two sessions greeted");
    Peer third(h.port);Sleep(80);
    require(h.factory.created==2&&third.socket.available()==0,"cap pauses application admission while listener backlog remains pending");
    first.close();require(third.frame()=="hello"&&h.factory.created==3,"retiring a session resumes accept exactly once");
    asio::io_context io;asio::ip::tcp::acceptor probe(io);asio::error_code error;
    probe.open(asio::ip::tcp::v6(),error);
    if(!error)probe.bind({asio::ip::address_v6::loopback(),0},error);
    if(error==asio::error::address_family_not_supported||
       (error.category()==asio::error::get_system_category()&&error.value()==WSAEADDRNOTAVAIL))
        std::cout<<"SKIP: cross-family cap assertion; host IPv6 loopback unavailable\n";
    else {
        if(error)throw asio::system_error(error,"IPv6 test capability probe");
        Peer excess(h.port,false,true);
        require(excess.closed()&&h.factory.created==3,"IPv6 and IPv4 pending accepts share one application cap");
    }
}
void same_family_accept_burst() {
    // B36/B37 accept-owner regression: one family must rearm while several
    // sessions are live, not only when a previous session is retired.
    net::SessionPolicy p;p.max_sessions=4;Harness h(p);
    std::vector<std::unique_ptr<Peer>> active;
    for(unsigned n=0;n<4;++n) {
        auto peer=std::make_unique<Peer>(h.port);
        require(peer->frame()=="hello","IPv4 accept rearms before any existing session retires");
        active.push_back(std::move(peer));
    }
    require(h.factory.created==4,"four simultaneously active IPv4 peers reached handler factories");
    for(auto& peer:active) {
        peer->send("keep");
        require(peer->frame()=="keep","each admitted session retains independent request/response state");
    }
    Peer waiting(h.port);
    Sleep(80);
    require(h.factory.created==4&&waiting.socket.available()==0,
            "session cap blocks application admission while four IPv4 peers remain live");
    active[2]->close();
    require(waiting.frame()=="hello"&&until([&]{return h.factory.created==5;}),
            "one retired IPv4 owner resumes exactly one backlog admission");
    waiting.send("resumed");
    require(waiting.frame()=="resumed","newly accepted client processes a request after cap retirement");
    active[0]->send("still");
    require(active[0]->frame()=="still","retiring a different peer preserves existing sessions");
}
void frame_and_response_limits() {
    net::SessionPolicy p;p.max_frame_bytes=8;p.max_response_bytes=16;Harness h(p);
    Peer exact(h.port);require(exact.frame()=="hello","bounded session greeting");exact.send("1234567");
    require(exact.frame()=="1234567","exact frame limit includes delimiter");
    exact.send(std::string("a\0b",3));require(exact.frame()=="a"&&exact.frame()=="b","bounded reads preserve multiple pipelined frames");
    exact.raw("12345678");require(exact.closed()&&h.factory.limits>=1,"unterminated full frame closes only offending session");
    Peer oversized(h.port);(void)oversized.frame();oversized.send("huge");
    // The IO thread may execute the queued close before the handler worker
    // reports its exception. Observe both outcomes without assuming that order.
    require(oversized.closed()&&until([&]{return h.factory.limits>=2;}),"oversize response is rejected before sending");
    Peer healthy(h.port);(void)healthy.frame();healthy.send("ok");require(healthy.frame()=="ok","listener remains usable after both resource violations");
}
void receive_deadlines() {
    {net::SessionPolicy p;p.receive_idle=350ms;p.receive_frame=1000ms;Harness h(p);Peer peer(h.port);(void)peer.frame();
     require(until([&]{return h.factory.timeouts==1;})&&peer.closed(),"receive-idle timer closes an inactive real socket");}
    {net::SessionPolicy p;p.receive_idle=2000ms;p.receive_frame=350ms;Harness h(p);Peer peer(h.port);(void)peer.frame();peer.raw("x");
     require(until([&]{return h.factory.timeouts==1;})&&peer.closed(),"partial-frame timer expires independently of long idle budget");}
    {net::SessionPolicy p;p.receive_idle=800ms;p.receive_frame=1000ms;Harness h(p);Peer peer(h.port);(void)peer.frame();
     Sleep(300);peer.send("ok");require(peer.frame()=="ok","activity completes before original idle deadline");
     Sleep(550);require(h.factory.timeouts==0,"cancelled earlier timer generation cannot close a renewed receive");
     require(until([&]{return h.factory.timeouts==1;})&&peer.closed(),"renewed idle timer eventually expires");}
}
void asynchronous_greeting_and_send_timeout() {
    net::SessionPolicy p;p.send_idle=500ms;Harness h(p);h.factory.large_first_greeting=true;
    Peer slow(h.port,true);require(until([&]{return slow.socket.available()!=0;}),"slow peer has an actual greeting prefix");
    Peer healthy(h.port);require(healthy.frame()=="hello","slow greeting does not occupy IO thread or prevent another greeting");
    require(until([&]{return h.factory.timeouts>=1;}),"stalled asynchronous greeting hits send-idle deadline");
    healthy.send("ok");require(healthy.frame()=="ok","another session remains live after greeting timeout");
}
void send_backpressure_and_stop() {
    net::SessionPolicy p;p.send_idle=500ms;Harness h(p);Peer slow(h.port,true);(void)slow.frame();
    slow.send(std::string("huge\0next",9));require(until([&]{return h.factory.requests==1;}),"large response handler entered");
    Sleep(100);require(h.factory.next_requests==0,"next pipelined request waits for complete response delivery");
    require(until([&]{return h.factory.timeouts>=1;})&&h.factory.next_requests==0,"stalled response times out without processing later frame");
    Peer active(h.port);(void)active.frame();const auto start=std::chrono::steady_clock::now();h.service.stop();
    require(std::chrono::steady_clock::now()-start<1000ms,"stop cancels armed timers and drains without waiting for idle expiry");
}
}
int main()try{policy_construction_validation();session_cap();same_family_accept_burst();frame_and_response_limits();receive_deadlines();asynchronous_greeting_and_send_timeout();send_backpressure_and_stop();std::cout<<"PASS: "<<checks<<" real-socket admission/limits/deadlines/backpressure/stop checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
