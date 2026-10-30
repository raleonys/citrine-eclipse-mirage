#include "preloader_l/net/asio_service.hpp"
#include "preloader_l/net/handler_policy.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#include <asio.hpp>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>

using namespace preloader_l;
using namespace std::chrono_literals;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>bool until(F fn,std::chrono::milliseconds timeout=5000ms){auto end=std::chrono::steady_clock::now()+timeout;do{if(fn())return true;Sleep(1);}while(std::chrono::steady_clock::now()<end);return false;}
struct Factory final:net::HandlerFactory {
    std::mutex mutex;std::condition_variable cv;bool released=false;
    std::set<std::thread::id> threads;std::atomic<unsigned> active{0},peak{0},calls{0},overloads{0};
    void release(){std::lock_guard lock(mutex);released=true;cv.notify_all();}
    struct Handler final:server_recovered::Handler {
        Factory& owner;unsigned expected=0;
        explicit Handler(Factory& f):owner(f){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view request)override {
            ++owner.calls;const auto count=++owner.active;
            auto previous=owner.peak.load();while(count>previous&&!owner.peak.compare_exchange_weak(previous,count)){}
            struct Finish{Factory& f;~Finish(){--f.active;}} finish{owner};
            {std::unique_lock lock(owner.mutex);owner.threads.insert(std::this_thread::get_id());
             if(request=="hold")owner.cv.wait(lock,[&]{return owner.released;});}
            if(request=="throw")throw std::runtime_error("isolated handler failure");
            if(request!="hold"&&request!=std::to_string(expected++))throw std::runtime_error("out-of-order session request");
            std::string response(request);response.push_back('\0');return response;
        }
    };
    std::unique_ptr<server_recovered::Handler> create()override{return std::make_unique<Handler>(*this);}
    void error(std::string_view,std::exception_ptr failure)noexcept override {
        try{if(failure)std::rethrow_exception(failure);}catch(const std::exception& e){if(std::string_view(e.what()).find("Handler queue is full")!=std::string_view::npos)++overloads;}catch(...){}
    }
};
struct Peer {
    asio::io_context io;asio::ip::tcp::socket socket{io};std::string input;
    explicit Peer(std::uint16_t port){socket.connect({asio::ip::address_v4::loopback(),port});socket.non_blocking(true);require(frame()=="hello","IO greeting remains live");}
    void send(std::string bytes){bytes.push_back('\0');socket.non_blocking(false);asio::write(socket,asio::buffer(bytes));socket.non_blocking(true);}
    std::string frame(){std::string result;const bool received=until([&]{
        const auto pos=input.find('\0');if(pos!=std::string::npos){result=input.substr(0,pos);input.erase(0,pos+1);return true;}
        char bytes[4096];asio::error_code error;const auto size=socket.read_some(asio::buffer(bytes),error);
        if(!error)input.append(bytes,size);else if(error!=asio::error::would_block&&error!=asio::error::try_again)throw asio::system_error(error);
        return false;
    });require(received,"response timeout");return result;}
    bool closed(){return until([&]{char data[64];asio::error_code error;socket.read_some(asio::buffer(data),error);return error&&error!=asio::error::would_block&&error!=asio::error::try_again;},2000ms);}
};
void run() {
    Factory factory;
    execution_recovered::MutexState mutex{};execution_recovered::initialize_mutex_04a698(mutex,2);
    std::uint8_t a=0,b=0,c=0;std::uint16_t port=0;
    execution_recovered::WindowsPlatform platform(+[](int){throw std::runtime_error("thread error");});
    auto frequency=platform.query_performance_frequency();execution_recovered::Execution execution(mutex,{a,b,c},frequency,platform);
    net::SessionPolicy policy;policy.max_sessions=256;
    net::AsioService service(factory,execution,port,a,policy);
    struct Unblock{Factory& f;~Unblock(){f.release();}} unblock{factory};
    service.start();
    Peer ordered(port);std::string batch;
    for(unsigned i=0;i<100;++i){if(i)batch.push_back('\0');batch+=std::to_string(i);}ordered.send(batch);
    for(unsigned i=0;i<100;++i)require(ordered.frame()==std::to_string(i),"pipelined request/response order preserved");
    Peer failure(port);failure.send("throw");require(failure.closed(),"handler exception closes only its own session");
    Peer healthy(port);healthy.send("0");require(healthy.frame()=="0","pool remains usable after handler exception");
    Peer first(port),second(port);first.send("hold");second.send("hold");
    require(until([&]{return factory.active==net::handler_policy::workers;}),"exactly two blocked worker jobs entered");
    const auto before=factory.calls.load();
    std::vector<std::unique_ptr<Peer>> queued;
    for(std::size_t i=0;i<net::handler_policy::queued_jobs+4;++i){auto peer=std::make_unique<Peer>(port);peer->send("hold");queued.push_back(std::move(peer));}
    require(until([&]{return factory.overloads>0;}),"full admission queue rejects excess work without blocking IO");
    require(factory.peak==2&&factory.calls==before,"queued work never creates extra handler threads");
    {std::lock_guard lock(factory.mutex);require(factory.threads.size()==2,"worker identities remain bounded across all sessions");}
    auto stopped=std::async(std::launch::async,[&]{service.stop();});
    const bool waiting=stopped.wait_for(50ms)==std::future_status::timeout;
    bool sockets_closed=false;try{sockets_closed=first.closed();}catch(...){}
    factory.release(); // Always release active handlers before asserting stop behavior.
    const bool drained=stopped.wait_for(5000ms)==std::future_status::ready;
    require(drained,"shutdown joins running workers");stopped.get();
    require(waiting&&sockets_closed,"stop cancels sockets while waiting for active handlers");
    require(factory.active==0&&factory.calls==before,"stop discards queued jobs and drains only running work");
    service.stop();
}
}
int main()try{run();std::cout<<"PASS: "<<checks<<" real-socket bounded pool/FIFO/overload/shutdown checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
