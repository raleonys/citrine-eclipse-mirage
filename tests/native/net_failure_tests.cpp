#include <atomic>
#include <cstddef>
#include <cstdlib>
#include <malloc.h>
#include <memory>
#include <new>
#include <asio/detail/config.hpp>
#include <asio/detail/cstdint.hpp>
#include <asio/detail/throw_exception.hpp>

namespace fault {
enum class Route {none,scalar_new,aligned_crt};
thread_local bool next=false;
thread_local Route last=Route::none;
std::atomic<unsigned> injected{0};
bool consume(Route route) noexcept {
    if(!next)return false;
    next=false;last=route;++injected;return true;
}
void* aligned_malloc(std::size_t size,std::size_t alignment) noexcept {
    if(consume(Route::aligned_crt))return nullptr;
    return ::_aligned_malloc(size,alignment);
}
}
// These pinned Windows branches must remain visible to the injection fixture.
#if defined(ASIO_HAS_STD_ALIGNED_ALLOC) || defined(ASIO_HAS_BOOST_ALIGN)
#error Unsupported Asio allocator branch: extend the fault fixture explicitly
#endif
#if defined(ASIO_DETAIL_MEMORY_HPP) || defined(_aligned_malloc)
#error Allocation helper or CRT macro was already bound before the fault fixture
#endif
// Cross replay changes only the allocation helper header, not compiler/platform
// detection for the rest of Asio. Native MSVC already selects this CRT branch.
#if defined(PRELOADER_L_TEST_MSVC_MEMORY_REPLAY) && !defined(ASIO_MSVC)
#define ASIO_MSVC 1
#define FIXTURE_RESTORE_ASIO_MSVC
#endif
#if defined(ASIO_MSVC)
#define FIXTURE_ALIGNED_CRT_PATH 1
#endif
#define _aligned_malloc fault::aligned_malloc
#include <asio/detail/memory.hpp>
#undef _aligned_malloc
#ifdef FIXTURE_RESTORE_ASIO_MSVC
#undef ASIO_MSVC
#undef FIXTURE_RESTORE_ASIO_MSVC
#endif
#include <asio.hpp>
#include "preloader_l/net/server.hpp"
#include "preloader_l/net/session_policy.hpp"
#include "preloader_l/platform/execution_windows.hpp"
#include <memory>
#include <exception>
// White-box access is confined to this source-included test executable.
#define private public
#include "preloader_l/net/asio_service.hpp"
#undef private
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>

void* operator new(std::size_t n) {
    if(fault::consume(fault::Route::scalar_new))throw std::bad_alloc();
    if(auto p=std::malloc(n?n:1))return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}
#include "src/net/asio_service.cpp"

namespace {
using namespace preloader_l;
using Tcp=asio::ip::tcp;
using namespace std::chrono_literals;
void require(bool value,const char* text){if(!value)throw std::runtime_error(text);}
template<class F> bool until(F f) {
    const auto end=std::chrono::steady_clock::now()+3s;
    do{if(f())return true;Sleep(1);}while(std::chrono::steady_clock::now()<end);
    return false;
}
struct HandlerFailure{};
struct Factory final:net::HandlerFactory {
    std::atomic<unsigned> creates{0},errors{0},dispatch_errors{0};
    std::atomic<bool> null_handler{false};
    struct Handler final:server_recovered::Handler {
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view input)override {
            if(input=="fail")throw HandlerFailure{};
            if(input=="fail-post"){fault::next=true;throw HandlerFailure{};}
            std::string response(input);response.push_back('\0');return response;
        }
    };
    std::unique_ptr<server_recovered::Handler> create()override {
        ++creates;return null_handler.load()?nullptr:std::make_unique<Handler>();
    }
    void error(std::string_view op,std::exception_ptr)noexcept override {
        ++errors;if(op=="cleanup dispatch")++dispatch_errors;
    }
};
struct Context {
    Factory factory;
    execution_recovered::MutexState mutex{};
    std::uint8_t ready=0,second=0,third=0;
    std::uint16_t port=0;
    execution_recovered::WindowsPlatform platform{+[](int){throw HandlerFailure{};}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution_recovered::Execution execution{mutex,{ready,second,third},frequency,platform};
    Context(){execution_recovered::initialize_mutex_04a698(mutex,2);}
};
struct Peer {
    asio::io_context io;Tcp::socket socket{io};std::string pending;
    explicit Peer(std::uint16_t port){socket.connect({asio::ip::address_v4::loopback(),port});socket.non_blocking(true);}
    void send(std::string input){input.push_back('\0');socket.non_blocking(false);asio::write(socket,asio::buffer(input));socket.non_blocking(true);}
    std::string frame() {
        std::string value;
        require(until([&] {
            const auto at=pending.find('\0');
            if(at!=std::string::npos){value=pending.substr(0,at);pending.erase(0,at+1);return true;}
            char data[256];asio::error_code ec;const auto n=socket.read_some(asio::buffer(data),ec);
            if(!ec)pending.append(data,n);
            else if(ec!=asio::error::would_block&&ec!=asio::error::try_again)throw asio::system_error(ec);
            return false;
        }),"frame timeout");return value;
    }
    bool closed(){return until([&]{char b[64];asio::error_code ec;socket.read_some(asio::buffer(b),ec);return ec&&ec!=asio::error::would_block&&ec!=asio::error::try_again;});}
};
void accept_failure() {
    Context c;c.factory.null_handler=true;
    net::AsioService::Impl owner(c.factory,c.execution,c.port,c.ready,{});
    owner.acceptor.open(Tcp::v4());owner.acceptor.bind({asio::ip::address_v4::loopback(),0});owner.acceptor.listen();
    const auto injected_before=fault::injected.load();
    bool threw=false;fault::next=true;
    try{owner.queue_accept(owner.acceptor,0);}catch(const std::bad_alloc&){threw=true;}
    fault::next=false;
    require(threw,"accept initiation allocation failure was not injected");
    require(fault::injected.load()==injected_before+1,"accept failure must consume exactly one injection");
#ifdef FIXTURE_ALIGNED_CRT_PATH
    require(fault::last==fault::Route::aligned_crt,"accept fixture missed the real aligned CRT allocation");
    std::cout<<"accept allocation backend: aligned-crt\n";
#else
    require(fault::last==fault::Route::scalar_new,"accept fixture missed the real scalar allocation");
    std::cout<<"accept allocation backend: scalar-new\n";
#endif
    require(!owner.accept_pending[0],"failed accept leaves a stale pending flag");
    owner.queue_accept(owner.acceptor,0);
    asio::io_context io;Tcp::socket peer(io);peer.connect(owner.acceptor.local_endpoint());
    require(until([&]{owner.io.restart();owner.io.poll();return c.factory.creates.load()==1;}),"accept retry does not reach the queued peer");
    owner.stopping=true;asio::error_code ec;owner.acceptor.close(ec);peer.close(ec);owner.io.restart();while(owner.io.poll()){}
}
void factory_failure_rearms() {
    // A failed handler factory must retire its accepted socket without
    // poisoning listener admission or the next independent handler owner.
    Context c;c.factory.null_handler.store(true);
    net::AsioService service(c.factory,c.execution,c.port,c.ready);
    service.start();
    Peer rejected(c.port);
    require(rejected.closed(),"rejected session keeps accepted socket alive");
    require(until([&]{return c.factory.creates.load()>=1&&c.factory.errors.load()>=1;}),
            "failed handler construction must be reported");
    c.factory.null_handler.store(false);
    Peer healthy(c.port);
    require(healthy.frame()=="hello","listener did not rearm after handler factory failure");
    healthy.send("recovered");
    require(healthy.frame()=="recovered","new handler cannot process traffic after factory recovery");
    service.stop();service.stop();
}
void post_failure() {
    Context c;net::AsioService service(c.factory,c.execution,c.port,c.ready);service.start();
    Peer control(c.port);require(control.frame()=="hello","control greeting");control.send("fail");
    require(control.closed(),"ordinary handler failure does not close its own session");
    Peer healthy(c.port);require(healthy.frame()=="hello","healthy greeting");healthy.send("ok");
    require(healthy.frame()=="ok","ordinary handler error stops healthy traffic");
    const auto injected_before=fault::injected.load();
    Peer broken(c.port);require(broken.frame()=="hello","fault greeting");broken.send("fail-post");
    require(until([&]{return c.factory.dispatch_errors.load()!=0;}),"cleanup-post resource failure not reported");
    require(fault::injected.load()==injected_before+1,"cleanup-post failure must consume exactly one injection");
    require(broken.closed()&&healthy.closed(),"failed cleanup dispatch leaves live sockets stranded");
    require(service.impl_->stopping.load(),"failed dispatch keeps accepting work");
    service.stop();service.stop();
}
void stop_without_allocation() {
    Context c;net::AsioService service(c.factory,c.execution,c.port,c.ready);service.start();
    Peer peer(c.port);require(peer.frame()=="hello","stop greeting");
    const auto injected_before=fault::injected.load();
    bool failed=false;fault::next=true;
    try{service.stop();}catch(...){failed=true;}
    fault::next=false;
    require(!failed&&fault::injected.load()==injected_before,"stop depends on a new heap allocation");require(peer.closed(),"stop did not close the socket");
    service.stop();
}
void io_failure() {
    Context c;net::AsioService service(c.factory,c.execution,c.port,c.ready);service.start();
    Peer peer(c.port);require(peer.frame()=="hello","IO failure greeting");
    asio::post(service.impl_->io,[]{throw HandlerFailure{};});
    require(peer.closed(),"unexpected IO exception leaves accepted socket open");
    require(service.impl_->stopping.load(),"unexpected IO exception leaves service admission enabled");service.stop();
}
void drain_retry() {
    Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.ready,{});owner.used=true;
    asio::post(owner.io,[]{throw HandlerFailure{};});
    bool threw=false;try{owner.stop();}catch(const HandlerFailure&){threw=true;}
    require(threw,"first cleanup did not execute the pending fault");
    unsigned completed=0;asio::post(owner.io,[&]{++completed;});owner.stop();owner.stop();
    require(completed==1,"post-join cleanup retry skipped or repeated remaining work");
}
}
int main(int argc,char** argv)try {
    const std::string_view selected=argc==2?argv[1]:"all";
    bool ran=false;
    for(auto [name,fn]:{std::pair{"accept",accept_failure},{"post",post_failure},{"stop-oom",stop_without_allocation},{"io-exception",io_failure},{"drain-retry",drain_retry},{"factory-rearm",factory_failure_rearms}})
        if(selected=="all"||selected==name){fn();ran=true;std::cout<<"PASS: "<<name<<'\n';}
    require(ran,"unknown case");return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
catch(...){fault::next=false;std::cerr<<"FAIL: unexpected exception\n";return 1;}
