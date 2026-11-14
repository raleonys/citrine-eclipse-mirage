// SPDX-License-Identifier: GPL-3.0-only

#include <asio.hpp>
#include "laureondo/net/server.hpp"
#include "laureondo/net/session_policy.hpp"
#include "laureondo/platform/execution_windows.hpp"
#include <memory>
#include <new>
#define private public
#include "laureondo/net/asio_service.hpp"
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
using namespace laureondo;
using Tcp=asio::ip::tcp;
using namespace std::chrono_literals;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
struct Factory:net::HandlerFactory {
    unsigned errors=0,completion_errors=0,prepared=0,committed=0,cancelled=0,destroyed=0;
    std::string response;
    struct Receipt:server::DeliveryReceipt {
        Factory& owner;explicit Receipt(Factory& f):owner(f){}
        ~Receipt(){++owner.destroyed;}
        bool finish(bool success)noexcept override{if(success)++owner.committed;else ++owner.cancelled;return true;}
    };
    struct Handler:server::Handler {
        Factory& owner;explicit Handler(Factory& f):owner(f){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view)override{return owner.response;}
        server::PreparedResponse prepare_response(std::string_view request)override{
            ++owner.prepared;return {process_message_033d70(request),std::make_unique<Receipt>(owner)};
        }
    };
    std::unique_ptr<server::Handler> create()override{return std::make_unique<Handler>(*this);}
    void error(std::string_view operation,std::exception_ptr)noexcept override{
        ++errors;if(operation=="completion")++completion_errors;
    }
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
// Drive one real received frame to the existing owned worker queue. Execute
// its job here so the fault can be armed after response preparation but before
// the IO executor initiates the socket write; no worker timing race is assumed.
void queue_prepared_response(Harness& h){
    h.c.factory.response=std::string("reply\0",6);
    h.peer.non_blocking(false);asio::write(h.peer,asio::buffer("request\0",8));h.peer.non_blocking(true);
    const auto until=std::chrono::steady_clock::now()+1s;
    while(h.owner.jobs.empty()&&std::chrono::steady_clock::now()<until){
        h.owner.io.restart();h.owner.io.poll();if(h.owner.jobs.empty())Sleep(1);
    }
    require(h.owner.jobs.size()==1&&!h.client->armed_deadline&&h.client->pending_reads==0,
        "complete real frame pauses receive and admits exactly one handler job");
    auto job=std::move(h.owner.jobs.front());h.owner.jobs.pop_front();job();
    require(h.c.factory.prepared==1&&h.c.factory.committed==0&&h.c.factory.cancelled==0&&
            h.c.factory.destroyed==0&&h.client->pending_writes==0&&
            h.client->session.queued_messages()==0,
        "prepared response and receipt await IO dispatch without a socket write");
}
void send_timer_failure(fault::Operation operation){
    Harness h;queue_prepared_response(h);
    const auto prior=fault::injected;fault::next=operation;
    h.owner.io.restart();h.wait_closed();
    require(fault::injected==prior+1&&fault::next==fault::Operation::none&&
            h.c.factory.errors==1&&h.c.factory.completion_errors==1,
        "send timer fault is contained at the response IO completion boundary");
    require(h.c.factory.committed==0&&h.c.factory.cancelled==1&&h.c.factory.destroyed==1,
        "failed send initiation cancels and destroys its delivery receipt exactly once");
    require(h.client->pending_writes==0&&h.client->pending_reads==0&&h.client->pending_timers==0&&
            h.client->session.queued_messages()==0&&h.owner.jobs.empty(),
        "send timer failure leaves no pending socket timer output or handler owner");
    h.owner.stop();h.client->close();h.client->session.cancel_output();
    require(h.c.factory.errors==1&&h.c.factory.cancelled==1&&h.c.factory.destroyed==1,
        "repeated retirement cannot duplicate diagnostics or delivery cancellation");
}
void successful_prepared_response(){
    Harness h;queue_prepared_response(h);
    const auto prior=fault::injected;
    std::string received;const auto until=std::chrono::steady_clock::now()+1s;
    while((received.size()<6||h.c.factory.destroyed!=1)&&std::chrono::steady_clock::now()<until){
        h.owner.io.restart();h.owner.io.poll();
        char bytes[16];asio::error_code error;const auto n=h.peer.read_some(asio::buffer(bytes),error);
        if(!error)received.append(bytes,n);
        else if(error!=asio::error::would_block&&error!=asio::error::try_again)throw asio::system_error(error);
        if(received.size()<6||h.c.factory.destroyed!=1)Sleep(1);
    }
    require(received==h.c.factory.response&&h.c.factory.committed==1&&
            h.c.factory.cancelled==0&&h.c.factory.destroyed==1&&h.c.factory.errors==0&&fault::injected==prior,
        "unarmed control delivers exact bytes and commits its receipt once after fault cases");
    require(!h.client->closed&&h.client->pending_reads==1&&h.client->pending_writes==0&&
            h.client->armed_deadline.has_value()&&h.client->session.queued_messages()==0,
        "successful response restores receive timing on the still-open real socket");
}
void callback_rearm_failure(){
    Harness h;const auto prior=fault::injected;
    // Preserve the already queued timer, while a later logical receive
    // deadline makes its callback take the real reschedule branch.
    h.client->deadlines.receive_progress(net::SessionDeadlines::Clock::now()+1s,1);
    fault::next=fault::Operation::wait;h.wait_closed();
    require(fault::injected==prior+1,"timer callback reschedule fault reached");
}
void send_initiation_failure(fault::Operation operation){
    Harness h;
    // A completed frame leaves no receive deadline. A subsequent ordered
    // response must install its own send deadline through Client::async_send.
    // Exercise the real Session output queue on the live IO executor, rather
    // than calling the timer or the socket adapter in isolation.
    h.client->frame_received();
    require(!h.client->armed_deadline,"completed frame retires the receive deadline");
    const auto prior=fault::injected;
    fault::next=operation;
    const auto client=h.client;
    client->post_io([client]{client->session.do_write_028270("reply");});
    h.wait_closed();
    require(fault::injected==prior+1&&h.c.factory.errors==1,
        "send timer initiation failure is contained by the IO callback");
    require(client->pending_writes==0&&client->session.queued_messages()==0,
        "failed send timer leaves no posted socket write or queued response");
}

}
int main()try{
    receive_failure(fault::Operation::wait);receive_failure(fault::Operation::cancel);callback_rearm_failure();
    send_initiation_failure(fault::Operation::wait);send_initiation_failure(fault::Operation::cancel);
    send_timer_failure(fault::Operation::wait);send_timer_failure(fault::Operation::cancel);
    successful_prepared_response();
    std::cout<<"PASS: "<<checks<<" checks; seven fault runs across five timer boundaries; direct/prepared ownership and unarmed send control\n";
}catch(const std::exception& e){fault::next=fault::Operation::none;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
