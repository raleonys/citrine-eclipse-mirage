#include <winsock2.h>
#include <windows.h>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>
namespace fault {
std::atomic<SOCKET> target{INVALID_SOCKET};
std::atomic<int> error{0};
std::atomic<unsigned> closes{0},option_failures{0};
bool select_connected=false,select_next=false,close_then_error=false,hold_timer=false;
void reset(){target=INVALID_SOCKET;error=0;closes=0;option_failures=0;select_connected=select_next=close_then_error=hold_timer=false;}
}
int WSAAPI TestCloseSocket(SOCKET value){
    if(fault::target==INVALID_SOCKET&&(fault::select_next||fault::select_connected)){
        sockaddr_storage address{};int size=sizeof address;
        if(fault::select_next||::getpeername(value,reinterpret_cast<sockaddr*>(&address),&size)==0)fault::target=value;
    }
    if(value==fault::target)++fault::closes;
    if(value==fault::target&&fault::error){
        if(fault::close_then_error){fault::close_then_error=false;::closesocket(value);}
        WSASetLastError(fault::error);return SOCKET_ERROR;
    }
    return ::closesocket(value);
}
int WSAAPI TestSetSockOpt(SOCKET value,int level,int name,const char* data,int size){
    if(value==fault::target&&name==SO_LINGER&&fault::option_failures){
        --fault::option_failures;WSASetLastError(WSAENETDOWN);return SOCKET_ERROR;
    }
    return ::setsockopt(value,level,name,data,size);
}
#define closesocket TestCloseSocket
#define setsockopt TestSetSockOpt
#include <asio.hpp>
#include "preloader_l/net/server.hpp"
#include "preloader_l/net/session_policy.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#define private public
#include "preloader_l/net/asio_service.hpp"
#undef private
namespace asio {
class FixtureTimer:public steady_timer {
public:
    using steady_timer::steady_timer;
    std::size_t cancel(){if(fault::hold_timer)throw std::bad_alloc();return steady_timer::cancel();}
};
}
#define steady_timer FixtureTimer
#include "src/net/asio_service.cpp"
#undef steady_timer
#undef setsockopt
#undef closesocket

namespace {
using namespace preloader_l;using Tcp=asio::ip::tcp;using namespace std::chrono_literals;
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
struct Factory:net::HandlerFactory {
    bool throw_create=false;
    unsigned tickets=0,positive=0,negative=0;
    struct Ticket:server_recovered::DeliveryReceipt {
        Factory& f;explicit Ticket(Factory& x):f(x){++f.tickets;}
        ~Ticket(){--f.tickets;}
        bool finish(bool success)noexcept override{if(success)++f.positive;else ++f.negative;return true;}
    };
    struct Handler:server_recovered::Handler {
        Factory& f;explicit Handler(Factory& x):f(x){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view)override{return {};}
        server_recovered::PreparedResponse prepare_response(std::string_view)override{return {std::string(128*1024,'X'),std::make_unique<Ticket>(f)};}
    };
    std::unique_ptr<server_recovered::Handler> create()override{if(throw_create)throw std::runtime_error("fixture handler failure");return std::make_unique<Handler>(*this);}
    void error(std::string_view,std::exception_ptr)noexcept override{}
};
struct Context {
    Factory factory;execution_recovered::MutexState mutex{};
    std::uint8_t a=0,b=0,c=0;std::uint16_t port=0;
    execution_recovered::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution_recovered::Execution execution{mutex,{a,b,c},frequency,platform};
    Context(){execution_recovered::initialize_mutex_04a698(mutex,2);}
};
struct Harness {
    Context c;net::AsioService::Impl owner{c.factory,c.execution,c.port,c.a,{}};
    asio::io_context peer_io;Tcp::socket peer{peer_io};
    std::shared_ptr<net::AsioService::Impl::Client> client;
    Harness(){
        fault::reset();Tcp::acceptor listener(owner.io,{asio::ip::address_v4::loopback(),0});
        peer.connect(listener.local_endpoint());peer.non_blocking(true);
        client=std::make_shared<net::AsioService::Impl::Client>(owner,listener.accept());
        owner.clients.push_back(client);client->counted=true;++owner.active_sessions;owner.used=true;
        client->session.start_025720();std::string greeting;const auto end=std::chrono::steady_clock::now()+2s;
        while(greeting.size()<6&&std::chrono::steady_clock::now()<end){owner.io.poll();char b[32];asio::error_code ec;const auto n=peer.read_some(asio::buffer(b),ec);if(!ec)greeting.append(b,n);if(greeting.size()<6)Sleep(1);}
        require(greeting==std::string("hello\0",6),"real greeting and queued receive");
        while(client->session.queued_messages()&&std::chrono::steady_clock::now()<end){owner.io.poll();Sleep(1);}
        require(!client->session.queued_messages(),"greeting completion dispatched before the next test phase");
    }
    ~Harness(){fault::error=0;fault::hold_timer=false;owner.stop();}
};
bool stop_failed(net::AsioService::Impl& owner){try{owner.stop();return false;}catch(const std::runtime_error&){return true;}}
void retained_retry(bool option_failure){
    Harness h;const auto raw=h.client->socket.native_handle();fault::target=raw;fault::error=WSAEWOULDBLOCK;
    const bool retained=stop_failed(h.owner);fault::error=0;
    bool option_retained=true;unsigned prior=fault::closes;
    if(retained&&option_failure){fault::option_failures=1;option_retained=stop_failed(h.owner)&&fault::closes==prior;}
    if(retained)h.owner.stop();else ::closesocket(raw); // Baseline test cleanup only.
    require(retained,"failed native close incorrectly acknowledges cleanup");
    require(option_retained,"retry after linger-option failure must keep ownership without closing");
    require(h.owner.active_sessions==0,"retry retires the original session once");
    const auto completed=fault::closes.load();
    h.owner.stop();require(fault::closes==completed,"completed retirement must not retry a numeric handle");
}
void unknown_quarantine(bool native_closed){
    // Ambiguous native results cannot be cleared by product policy. Keep the
    // complete test owner alive until process exit, as the DLL must do.
    auto* h=new Harness;fault::target=h->client->socket.native_handle();fault::error=WSAENETDOWN;fault::close_then_error=native_closed;
    const bool first=stop_failed(h->owner);const auto attempts=fault::closes.load();fault::error=0;
    Tcp::socket unrelated(h->peer_io);unrelated.open(Tcp::v4());const auto reused=unrelated.native_handle()==fault::target;
    const bool second=stop_failed(h->owner);
    int type=0,size=sizeof type;const bool other_live=::getsockopt(unrelated.native_handle(),SOL_SOCKET,SO_TYPE,reinterpret_cast<char*>(&type),&size)==0;
    require(first&&second,"ambiguous close must keep cleanup blocked across requests");
    require(fault::closes==attempts&&other_live,"ambiguous numeric handle must never be retried");
    std::cout<<"quarantine: native_closed="<<native_closed<<" reuse_observed="<<reused<<" retained_until_process_exit\n";
}
void completion_lifetime(){
    Harness h;h.client->session.process_message_028d30("receipt");const auto end=std::chrono::steady_clock::now()+2s;
    while(!h.client->session.queued_messages()&&std::chrono::steady_clock::now()<end)h.owner.io.poll_one();
    require(h.client->session.queued_messages()==1&&h.c.factory.tickets==1,"write operation owns prepared output");
    h.client->close();const auto active_before_callbacks=h.owner.active_sessions;
    require(h.c.factory.tickets==1&&h.c.factory.negative==1&&!h.c.factory.positive,"cancelled write retains its buffer and negative receipt until callback");
    h.owner.stop();
    require(active_before_callbacks==1,"session capacity retired before IO cancellation completions");
    require(h.owner.active_sessions==0&&h.c.factory.tickets==0&&h.c.factory.negative==1&&!h.c.factory.positive,"completion releases output exactly once without acknowledging cancelled delivery");
}
void timer_retirement(){
    Harness h;fault::hold_timer=true;const bool pending=stop_failed(h.owner);fault::hold_timer=false;
    h.client->timer.cancel();h.owner.stop();
    require(pending,"pending timer incorrectly acknowledges complete cleanup");
    require(h.owner.active_sessions==0,"timer completion releases session ownership");
}
void constructor_rollback(){
    fault::reset();Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.a,{});owner.used=true;
    owner.acceptor.open(Tcp::v4());owner.acceptor.bind({asio::ip::address_v4::loopback(),0});owner.acceptor.listen();
    c.factory.throw_create=true;owner.queue_accept(owner.acceptor,0);Tcp::socket peer(owner.io);peer.connect(owner.acceptor.local_endpoint());
    fault::select_connected=true;fault::error=WSAEWOULDBLOCK;owner.io.run_for(20ms);
    const auto raw=fault::target.load();const bool blocked=stop_failed(owner);fault::error=0;
    if(blocked)owner.stop();else if(raw!=INVALID_SOCKET)::closesocket(raw);
    require(raw!=INVALID_SOCKET&&blocked,"failed Client construction loses native rollback ownership");
}
void bind_rollback(){
    fault::reset();Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.a,{});owner.used=true;
    Tcp::acceptor blocker(owner.io,{asio::ip::address_v4::loopback(),net::listener_policy::first_port});
    server_recovered::Endpoint endpoint;endpoint.family=2;endpoint.address[0]=127;endpoint.address[3]=1;
    fault::select_next=true;fault::error=WSAEWOULDBLOCK;
    try{owner.construct_acceptor_020e20(endpoint,true,128);}catch(const std::exception&){}
    const auto raw=fault::target.load();const bool blocked=stop_failed(owner);fault::error=0;
    if(blocked)owner.stop();else if(raw!=INVALID_SOCKET)::closesocket(raw);
    require(raw!=INVALID_SOCKET&&blocked,"failed bind rollback loses native socket ownership");
}
void listener_retry(){
    fault::reset();Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.a,{});owner.used=true;
    owner.acceptor.open(Tcp::v4());owner.acceptor.bind({asio::ip::address_v4::loopback(),0});owner.acceptor.listen();
    const auto raw=owner.acceptor.native_handle();owner.queue_accept(owner.acceptor,0);
    fault::target=raw;fault::error=WSAEWOULDBLOCK;const bool blocked=stop_failed(owner);fault::error=0;
    if(blocked)owner.stop();else ::closesocket(raw);
    require(blocked,"failed listener close incorrectly acknowledges cleanup");
    const auto completed=fault::closes.load();owner.stop();
    require(!owner.accept_pending[0]&&fault::closes==completed,"listener retry drains accept and never repeats completed native close");
}
}
int main(int argc,char** argv)try{
    const std::string_view selected=argc==2?argv[1]:"all";bool ran=false;
    for(auto [name,fn]:{std::pair{"retry",+[]{retained_retry(false);}},
        {"retry-option",+[]{retained_retry(true);}},{"uncertain-open",+[]{unknown_quarantine(false);}},
        {"uncertain-closed",+[]{unknown_quarantine(true);}},{"completion",completion_lifetime},
        {"timer",timer_retirement},{"constructor",constructor_rollback},{"bind",bind_rollback},{"listener",listener_retry}})
        if(selected=="all"||selected==name){fn();ran=true;std::cout<<"PASS: "<<name<<'\n';}
    require(ran,"unknown case");
}catch(const std::exception& e){fault::error=0;fault::hold_timer=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
