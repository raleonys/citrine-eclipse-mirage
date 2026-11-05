// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <windows.h>
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
// A11-05: capture an actual Windows IOCP accept packet immediately after it
// leaves the kernel, before the source-included Asio dispatcher observes it.
// Only the single-owner accept fixture arms this hook; normal tests pass through.
namespace held_accept {
struct Packet {HANDLE port{};BOOL success{};DWORD error{},bytes{};ULONG_PTR key{};OVERLAPPED* operation{};};
bool armed=false,holding=false,cached=false;
unsigned captures=0,deliveries=0,waits=0;
Packet packet;
BOOL timeout(LPDWORD bytes,PULONG_PTR key,LPOVERLAPPED* operation,DWORD milliseconds){
    if(milliseconds&&milliseconds!=INFINITE)Sleep(milliseconds);
    *bytes=0;*key=0;*operation=nullptr;++waits;
    SetLastError(WAIT_TIMEOUT);return FALSE;
}
}
BOOL WINAPI TestAcceptCompletion(HANDLE port,LPDWORD bytes,PULONG_PTR key,
    LPOVERLAPPED* operation,DWORD milliseconds){
    using namespace held_accept;
    if(cached&&packet.port==port){
        if(holding)return timeout(bytes,key,operation,milliseconds);
        *bytes=packet.bytes;*key=packet.key;*operation=packet.operation;
        cached=false;++deliveries;SetLastError(packet.error);
        return packet.success;
    }
    const auto ok=::GetQueuedCompletionStatus(port,bytes,key,operation,milliseconds);
    const auto error=GetLastError();
    if(armed&&!cached&&ok&&*operation){
        packet={port,ok,error,*bytes,*key,*operation};cached=true;++captures;
        return timeout(bytes,key,operation,0);
    }
    SetLastError(error);return ok;
}
#define GetQueuedCompletionStatus TestAcceptCompletion
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
#undef GetQueuedCompletionStatus

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
    struct Handler final:server::Handler {
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view input)override {
            if(input=="fail")throw HandlerFailure{};
            if(input=="fail-post"){fault::next=true;throw HandlerFailure{};}
            std::string response(input);response.push_back('\0');return response;
        }
    };
    std::unique_ptr<server::Handler> create()override {
        ++creates;return null_handler.load()?nullptr:std::make_unique<Handler>();
    }
    void error(std::string_view op,std::exception_ptr)noexcept override {
        ++errors;if(op=="cleanup dispatch")++dispatch_errors;
    }
};
struct Context {
    Factory factory;
    execution::MutexState mutex{};
    std::uint8_t ready=0,second=0,third=0;
    std::uint16_t port=0;
    execution::WindowsPlatform platform{+[](int){throw HandlerFailure{};}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{ready,second,third},frequency,platform};
    Context(){execution::initialize_mutex_04a698(mutex,2);}
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
void accept_cancellation_finality() {
    Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.ready,{});
    owner.used=true;
    owner.acceptor.open(Tcp::v4());owner.acceptor.bind({asio::ip::address_v4::loopback(),0});owner.acceptor.listen();
    owner.queue_accept(owner.acceptor,0);
    require(owner.accept_pending[0],"accept submission records a pending operation");
    owner.stopping=true;asio::error_code error;owner.acceptor.cancel(error);
    require(!error&&owner.accept_pending[0],"cancel intent must not retire an accept before its completion");
    owner.acceptor.close(error);
    require(!error&&owner.accept_pending[0]&&c.factory.creates==0,"closing the listener retains its unconsumed completion without constructing a handler");
    owner.stop();owner.stop();
    require(!owner.accept_pending[0]&&!owner.accept_pending[1]&&owner.active_sessions==0,
        "stop drains the exact pending accept before acknowledging complete cleanup");
    require(c.factory.creates==0&&c.factory.errors==0,"cancelled accept completion cannot create a session or rearm a closed listener");
}
void dual_stack_accept_cancellation_finality() {
    // Both Asio accept owners may have a pending kernel operation when the
    // last admission is sealed. Cancellation intent is not completion.
    for(bool connected_before_stop:{false,true}) {
        Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.ready,{});
        owner.used=true;
        owner.acceptor.open(Tcp::v4());
        owner.acceptor.bind({asio::ip::address_v4::loopback(),0});
        owner.acceptor.listen();
        owner.acceptor_v6=std::make_unique<decltype(owner.acceptor)>(
            owner.io,&owner,&net::AsioService::Impl::retain_native);
        owner.acceptor_v6->open(Tcp::v6());
        owner.acceptor_v6->set_option(asio::ip::v6_only(true));
        owner.acceptor_v6->bind({asio::ip::address_v6::loopback(),0});
        owner.acceptor_v6->listen();
        owner.queue_accept(owner.acceptor,0);
        owner.queue_accept(*owner.acceptor_v6,1);
        require(owner.accept_pending[0]&&owner.accept_pending[1],
                "both listening families must own one pending accept before stop");
        asio::io_context peer_io;
        Tcp::socket peer4(peer_io),peer6(peer_io);
        if(connected_before_stop) {
            // Synchronous TCP connections make success possible while neither
            // queued completion has yet executed on the owner's IO context.
            peer4.connect(owner.acceptor.local_endpoint());
            peer6.connect(owner.acceptor_v6->local_endpoint());
        }
        owner.stopping.store(true,std::memory_order_release);
        asio::error_code error;
        owner.acceptor.cancel(error);
        require(!error&&owner.accept_pending[0]&&owner.accept_pending[1],
                "cancel of first family cannot retire either pending completion");
        owner.acceptor_v6->cancel(error);
        require(!error&&owner.accept_pending[0]&&owner.accept_pending[1],
                "cancel of second family cannot publish completion or a session");
        owner.stop();owner.stop();
        require(!owner.accept_pending[0]&&!owner.accept_pending[1]&&
                owner.active_sessions==0&&owner.clients.empty(),
                "both family completions drain and sessions remain absent after repeated stop");
        require(c.factory.creates==0&&c.factory.errors==0,
                "late accepted or cancelled sockets cannot create handlers after stop");
        owner.io.restart();
        require(owner.io.poll()==0&&!owner.accept_pending[0]&&!owner.accept_pending[1],
                "post-stop polling must not rearm either listener");
    }
}
void held_successful_accept_retirement() {
    // Single actual pending AcceptEx on a loopback listener. With only one
    // submitted operation, the captured successful IOCP packet belongs to it.
    // A kernel dequeue is not Asio callback completion or buffer retirement.
    Context c;net::AsioService::Impl owner(c.factory,c.execution,c.port,c.ready,{});
    owner.used=true;
    owner.acceptor.open(Tcp::v4());
    owner.acceptor.bind({asio::ip::address_v4::loopback(),0});
    owner.acceptor.listen();
    owner.queue_accept(owner.acceptor,0);
    require(owner.accept_pending[0]&&!owner.accept_pending[1]&&owner.active_sessions==0,
        "one pending native accept owns admission before the peer connects");
    struct Gate {
        Gate(){
            require(!held_accept::armed&&!held_accept::cached,
                "previous held accept must be fully retired");
            held_accept::packet={};
            held_accept::captures=held_accept::deliveries=held_accept::waits=0;
            held_accept::armed=held_accept::holding=true;
        }
        void release(){held_accept::armed=held_accept::holding=false;}
        ~Gate(){release();}
    } gate;
    asio::io_context peer_io;
    Tcp::socket peer(peer_io);
    peer.connect(owner.acceptor.local_endpoint());
    const auto deadline=std::chrono::steady_clock::now()+3s;
    while(!held_accept::cached&&std::chrono::steady_clock::now()<deadline){
        owner.io.restart();owner.io.poll_one();
        if(!held_accept::cached)Sleep(1);
    }
    const auto packet=held_accept::packet;
    require(held_accept::cached&&held_accept::captures==1&&
            packet.success&&packet.port&&packet.operation&&packet.key==0,
        "one real successful native accept completion is held after IOCP dequeue");
    require(owner.accept_pending[0]&&owner.active_sessions==0&&c.factory.creates==0,
        "dequeued but unconsumed accept cannot create a handler or release its owner");
    owner.stopping.store(true,std::memory_order_release);
    const auto begin=std::chrono::steady_clock::now();
    for(unsigned i=0;i<2;++i){
        const auto previous_waits=held_accept::waits;
        bool incomplete=false;
        try{owner.stop();}catch(const std::runtime_error&){incomplete=true;}
        require(incomplete&&held_accept::waits>previous_waits,
            "held accept completion must force each bounded stop to remain incomplete");
        require(owner.accept_pending[0]&&owner.active_sessions==0&&
                held_accept::cached&&held_accept::captures==1&&!held_accept::deliveries,
            "stop cannot retire the pending accept or replay the original IOCP packet");
        require(c.factory.creates==0&&owner.clients.empty(),
            "a stopped listener cannot construct a session before accept completion");
    }
    require(std::chrono::steady_clock::now()-begin>=100ms,
        "native accept packet must survive beyond the first 100 ms drain budget");
    gate.release();
    owner.stop();owner.stop();
    require(!held_accept::cached&&held_accept::captures==1&&held_accept::deliveries==1,
        "the original successful accept packet must be delivered exactly once");
    require(!owner.accept_pending[0]&&!owner.accept_pending[1]&&
            owner.active_sessions==0&&owner.clients.empty()&&c.factory.creates==0,
        "post-stop successful accept never rearms or publishes a new handler");
    owner.io.restart();
    require(owner.io.poll()==0&&!owner.accept_pending[0],
        "subsequent executor polling cannot resurrect a stopped accept");
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
    for(auto [name,fn]:{std::pair{"accept",accept_failure},{"accept-cancel",accept_cancellation_finality},{"dual-accept-stop",dual_stack_accept_cancellation_finality},{"held-accept-stop",held_successful_accept_retirement},{"post",post_failure},{"stop-oom",stop_without_allocation},{"io-exception",io_failure},{"drain-retry",drain_retry},{"factory-rearm",factory_failure_rearms}})
        if(selected=="all"||selected==name){fn();ran=true;std::cout<<"PASS: "<<name<<'\n';}
    require(ran,"unknown case");return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
catch(...){fault::next=false;std::cerr<<"FAIL: unexpected exception\n";return 1;}
