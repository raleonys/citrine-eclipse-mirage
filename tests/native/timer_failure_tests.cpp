// SPDX-License-Identifier: GPL-3.0-only

#include <asio.hpp>
#include "preloader_l/net/server.hpp"
#include "preloader_l/net/session_policy.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#include <memory>
#include <new>
#define private public
#include "preloader_l/net/asio_service.hpp"
#undef private
#include <iostream>
#include <stdexcept>

namespace fault {
enum class Operation {none,wait,cancel};
thread_local Operation next=Operation::none;
unsigned injected=0;
void check(Operation operation){if(next==operation){next=Operation::none;++injected;throw std::bad_alloc();}}
}
namespace asio {
// Test-only initiation/cancel boundary. Successful calls use the real IOCP
// timer, socket and callbacks; injected exceptions precede the operation.
class FixtureTimer:public steady_timer {
public:
    using steady_timer::steady_timer;
    std::size_t cancel(){fault::check(fault::Operation::cancel);return steady_timer::cancel();}
    template<class Handler>auto async_wait(Handler&& handler){fault::check(fault::Operation::wait);return steady_timer::async_wait(std::forward<Handler>(handler));}
};
}
#define steady_timer FixtureTimer
#include "src/net/asio_service.cpp"
#undef steady_timer
namespace {
using namespace preloader_l;
using Tcp=asio::ip::tcp;
using namespace std::chrono_literals;
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
struct Factory:net::HandlerFactory {
    unsigned errors=0;
    struct Handler:server::Handler {
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view)override{return {};}
    };
    std::unique_ptr<server::Handler> create()override{return std::make_unique<Handler>();}
    void error(std::string_view,std::exception_ptr)noexcept override{++errors;}
};
struct Context {
    Factory factory;execution::MutexState mutex{};
    std::uint8_t a=0,b=0,c=0;std::uint16_t port=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    net::SessionPolicy policy;
    Context(){execution::initialize_mutex_04a698(mutex,2);policy.receive_idle=200ms;}
};
struct Harness {
    Context c;
    net::AsioService::Impl owner{c.factory,c.execution,c.port,c.a,c.policy};
    asio::io_context peer_io;Tcp::socket peer{peer_io};
    std::shared_ptr<net::AsioService::Impl::Client> client;
    Harness(){
        Tcp::acceptor listener(owner.io,{asio::ip::address_v4::loopback(),0});
        peer.connect(listener.local_endpoint());peer.non_blocking(true);
        client=std::make_shared<net::AsioService::Impl::Client>(owner,listener.accept());
        owner.clients.push_back(client);owner.used=true;
        client->session.start_025720();
        const auto until=std::chrono::steady_clock::now()+1s;
        std::string greeting;
        while(greeting.size()<6&&std::chrono::steady_clock::now()<until){
            owner.io.poll();char buffer[16];asio::error_code ec;const auto n=peer.read_some(asio::buffer(buffer),ec);
            if(!ec)greeting.append(buffer,n);if(greeting.size()<6)Sleep(1);
        }
        require(greeting==std::string("hello\0",6)&&client->armed_deadline.has_value(),"real async greeting and receive deadline armed");
    }
    ~Harness(){fault::next=fault::Operation::none;owner.stop();}
    void wait_closed(){
        const auto until=std::chrono::steady_clock::now()+2s;
        while(!client->closed&&std::chrono::steady_clock::now()<until){owner.io.poll();Sleep(1);}
        require(client->closed&&!client->socket.is_open(),"timer failure must close its actual session");
        require(!client->armed_deadline,"closed session must clear cached deadline");
        owner.stop();
        char bytes[16];asio::error_code error;peer.read_some(asio::buffer(bytes),error);
        require(error==asio::error::eof||error==asio::error::connection_reset,"peer observes complete retirement");
    }
};
void receive_failure(fault::Operation operation){
    Harness h;const auto prior=fault::injected;fault::next=operation;
    h.peer.non_blocking(false);asio::write(h.peer,asio::buffer("partial",7));h.peer.non_blocking(true);
    h.wait_closed();require(fault::injected==prior+1&&h.c.factory.errors==1,"receive callback contains the injected timer exception");
}
void callback_rearm_failure(){
    Harness h;const auto prior=fault::injected;
    // Preserve the already queued timer, while a later logical receive
    // deadline makes its callback take the real reschedule branch.
    h.client->deadlines.receive_progress(net::SessionDeadlines::Clock::now()+1s,1);
    fault::next=fault::Operation::wait;h.wait_closed();
    require(fault::injected==prior+1,"timer callback reschedule fault reached");
}
}
int main()try{
    receive_failure(fault::Operation::wait);receive_failure(fault::Operation::cancel);callback_rearm_failure();
    std::cout<<"PASS: three timer exception paths close real sessions; internal cache hypothesis does not bypass caller containment\n";
}catch(const std::exception& e){fault::next=fault::Operation::none;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
