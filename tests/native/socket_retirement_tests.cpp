// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <windows.h>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <new>
#include <memory>
#include <span>
namespace receive_buffer_probe {
struct Allocation{void* base;std::size_t bytes;};
std::atomic<bool> recording{false};
std::atomic<void*> tracked{nullptr};
std::atomic<unsigned> released{0};
std::mutex gate;std::array<Allocation,64> allocations{};std::size_t count=0;
SOCKET socket=INVALID_SOCKET;
void observe_allocation(void* memory,std::size_t bytes){
    if(recording.load()&&bytes>=4096){
        std::lock_guard lock(gate);
        if(count<allocations.size())allocations[count++]={memory,bytes};
    }
}
void observe_buffer(SOCKET value,LPWSABUF buffers,DWORD n){
    if(!recording.load()||value!=socket||n!=1||buffers[0].len!=4096)return;
    const auto data=reinterpret_cast<std::uintptr_t>(buffers[0].buf);
    std::lock_guard lock(gate);
    for(std::size_t i=0;i<count;++i){
        const auto base=reinterpret_cast<std::uintptr_t>(allocations[i].base);
        if(data>=base&&data-base<=allocations[i].bytes&&buffers[0].len<=allocations[i].bytes-(data-base)){
            tracked=allocations[i].base;return;
        }
    }
}
}
void* operator new(std::size_t bytes){
    if(auto* memory=std::malloc(bytes?bytes:1)){receive_buffer_probe::observe_allocation(memory,bytes);return memory;}
    throw std::bad_alloc();
}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete(void* memory)noexcept{
    auto* expected=memory;
    if(memory&&receive_buffer_probe::tracked.compare_exchange_strong(expected,nullptr))++receive_buffer_probe::released;
    std::free(memory);
}
void operator delete[](void* memory)noexcept{::operator delete(memory);}
void operator delete(void* memory,std::size_t)noexcept{::operator delete(memory);}
void operator delete[](void* memory,std::size_t)noexcept{::operator delete(memory);}
namespace late_completion {
// This fixture dispatches IO only on its controller (no service runner). Keep
// one actual dequeued OS packet intact until the controller releases delivery.
struct Packet {HANDLE port{};BOOL ok{};DWORD error{},bytes{};ULONG_PTR key{};OVERLAPPED* operation{};};
bool armed=false,holding=false,cached=false;
bool capture_send=false;
SOCKET send_socket=INVALID_SOCKET;
OVERLAPPED* target=nullptr;
char* buffer=nullptr;
int submitted=0;DWORD submit_error=0;
unsigned captures=0,deliveries=0,timeouts=0;
Packet packet;
BOOL timeout(LPDWORD bytes,PULONG_PTR key,LPOVERLAPPED* operation,DWORD milliseconds){
    // Honor the real caller's finite wait rather than simulating an immediate
    // timeout and accidentally bypassing the production 100 ms drain budget.
    if(milliseconds&&milliseconds!=INFINITE)Sleep(milliseconds);
    *bytes=0;*key=0;*operation=nullptr;++timeouts;
    SetLastError(WAIT_TIMEOUT);return FALSE;
}
}
BOOL WINAPI TestGetQueuedCompletionStatus(HANDLE port,LPDWORD bytes,PULONG_PTR key,LPOVERLAPPED* operation,DWORD milliseconds){
    using namespace late_completion;
    if(cached&&packet.port==port){
        if(holding)return timeout(bytes,key,operation,milliseconds);
        *bytes=packet.bytes;*key=packet.key;*operation=packet.operation;
        cached=false;++deliveries;SetLastError(packet.error);return packet.ok;
    }
    const auto ok=::GetQueuedCompletionStatus(port,bytes,key,operation,milliseconds);
    const auto error=GetLastError();
    if(armed&&target&&*operation==target){
        packet={port,ok,error,*bytes,*key,*operation};cached=true;++captures;
        return timeout(bytes,key,operation,0);
    }
    SetLastError(error);return ok;
}
int WSAAPI TestReceive(SOCKET value,LPWSABUF buffers,DWORD n,LPDWORD bytes,LPDWORD flags,LPWSAOVERLAPPED operation,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
    receive_buffer_probe::observe_buffer(value,buffers,n);
    const bool selected=late_completion::armed&&value==receive_buffer_probe::socket&&n==1&&buffers[0].len==4096;
    if(selected){late_completion::target=operation;late_completion::buffer=buffers[0].buf;}
    const auto result=::WSARecv(value,buffers,n,bytes,flags,operation,callback);
    const auto error=WSAGetLastError();
    if(selected){late_completion::submitted=result;late_completion::submit_error=error;}
    WSASetLastError(error);return result;
}
int WSAAPI TestSend(SOCKET value,LPWSABUF buffers,DWORD n,LPDWORD bytes,DWORD flags,
    LPWSAOVERLAPPED operation,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){
    const bool selected=late_completion::armed&&late_completion::capture_send&&
        value==late_completion::send_socket&&n==1&&buffers[0].len==4;
    if(selected){late_completion::target=operation;late_completion::buffer=buffers[0].buf;}
    const auto result=::WSASend(value,buffers,n,bytes,flags,operation,callback);
    const auto error=WSAGetLastError();
    if(selected){late_completion::submitted=result;late_completion::submit_error=error;}
    WSASetLastError(error);return result;
}
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
#define WSARecv TestReceive
#define WSASend TestSend
#define GetQueuedCompletionStatus TestGetQueuedCompletionStatus
#include <asio.hpp>
#include "citrine_eclipse_mirage/net/server.hpp"
#include "citrine_eclipse_mirage/net/session_policy.hpp"
#include "citrine_eclipse_mirage/platform/execution_windows.hpp"
#define private public
#include "citrine_eclipse_mirage/net/asio_service.hpp"
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
#undef WSARecv
#undef WSASend
#undef GetQueuedCompletionStatus

namespace {
using namespace citrine_eclipse_mirage;using Tcp=asio::ip::tcp;using namespace std::chrono_literals;
void require(bool b,const char* text){if(!b)throw std::runtime_error(text);}
struct Factory:net::HandlerFactory {
    bool throw_create=false;
    unsigned tickets=0,positive=0,negative=0;
    struct Ticket:server::DeliveryReceipt {
        Factory& f;explicit Ticket(Factory& x):f(x){++f.tickets;}
        ~Ticket(){--f.tickets;}
        bool finish(bool success)noexcept override{if(success)++f.positive;else ++f.negative;return true;}
    };
    struct Handler:server::Handler {
        Factory& f;explicit Handler(Factory& x):f(x){}
        void reset_02bc70()override{}
        std::string initial_message_02d540()override{return std::string("hello\0",6);}
        std::string process_message_033d70(std::string_view)override{return {};}
        server::PreparedResponse prepare_response(std::string_view)override{return {std::string(128*1024,'X'),std::make_unique<Ticket>(f)};}
    };
    std::unique_ptr<server::Handler> create()override{if(throw_create)throw std::runtime_error("fixture handler failure");return std::make_unique<Handler>(*this);}
    void error(std::string_view,std::exception_ptr)noexcept override{}
};
struct Context {
    Factory factory;execution::MutexState mutex{};
    std::uint8_t a=0,b=0,c=0;std::uint16_t port=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    Context(){execution::initialize_mutex_04a698(mutex,2);}
};
struct Harness {
    Context c;net::AsioService::Impl owner{c.factory,c.execution,c.port,c.a,{}};
    asio::io_context peer_io;Tcp::socket peer{peer_io};
    std::shared_ptr<net::AsioService::Impl::Client> client;
    explicit Harness(bool start_session=true){
        fault::reset();Tcp::acceptor listener(owner.io,{asio::ip::address_v4::loopback(),0});
        peer.connect(listener.local_endpoint());peer.non_blocking(true);
        client=std::make_shared<net::AsioService::Impl::Client>(owner,listener.accept());
        owner.clients.push_back(client);client->counted=true;++owner.active_sessions;owner.used=true;
        if(!start_session)return;
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
void receive_cancellation_lifetime(){
    Harness h(false);
    require(h.client->pending_reads==0,"own connected client starts without an outstanding read");
    unsigned callbacks=0;
    // Supply only the business observer: keep the production receive buffer,
    // socket operation and CompletionRetirement path in the actual Client.
    struct Reset{~Reset(){receive_buffer_probe::recording=false;receive_buffer_probe::tracked=nullptr;}} reset;
    receive_buffer_probe::count=0;receive_buffer_probe::released=0;
    receive_buffer_probe::socket=h.client->socket.native_handle();receive_buffer_probe::recording=true;
    h.client->async_receive(4096,[&](server::Error,std::string){++callbacks;});
    receive_buffer_probe::recording=false;
    require(receive_buffer_probe::tracked!=nullptr&&receive_buffer_probe::released==0,
        "actual WSARecv borrows the observed live containing buffer allocation");
    require(h.client->pending_reads==1,"actual receive initiation owns one pending buffer operation");
    std::weak_ptr<net::AsioService::Impl::Client> lifetime=h.client;
    h.client->close();
    require(receive_buffer_probe::released==0,"cancel/close cannot free the exact kernel-borrowed receive buffer");
    require(h.client->pending_reads==1&&h.owner.active_sessions==1&&!h.client->retired(),
        "cancel/close cannot retire receive storage or session capacity before terminal completions");
    h.client.reset();
    require(!lifetime.expired(),"pending receives keep their client and owned buffers alive after caller release");
    require(receive_buffer_probe::released==0,"caller release retains the kernel-borrowed buffer until completion");
    h.owner.stop();h.owner.stop();
    require(callbacks==0,"a cancelled receive never invokes its stale business callback");
    require(h.owner.active_sessions==0&&h.owner.clients.empty()&&lifetime.expired(),
        "terminal completion drains the receive loan and releases the client exactly once");
    require(receive_buffer_probe::released==1,"terminal completion frees the exact observed receive allocation once");
}
void delayed_receive_completion(bool success){
    Harness h(false);
    require(!h.owner.runner.joinable(),"completion gate has a single controller and no service runner");
    struct DeliveryGate {
        DeliveryGate(){
            require(!late_completion::armed&&!late_completion::cached,"previous completion gate must be drained");
            late_completion::target=nullptr;late_completion::buffer=nullptr;
            late_completion::captures=late_completion::deliveries=late_completion::timeouts=0;
            late_completion::packet={};late_completion::armed=late_completion::holding=true;
        }
        void release(){late_completion::armed=late_completion::holding=false;}
        ~DeliveryGate(){release();} // Harness cleanup can consume a held packet after an assertion failure.
    } delivery;
    struct ResetProbe {
        ~ResetProbe(){receive_buffer_probe::recording=false;receive_buffer_probe::tracked=nullptr;}
    } reset;
    unsigned callbacks=0;
    receive_buffer_probe::count=0;receive_buffer_probe::released=0;
    receive_buffer_probe::socket=h.client->socket.native_handle();
    receive_buffer_probe::recording=true;
    h.client->async_receive(4096,[&](server::Error,std::string){++callbacks;});
    receive_buffer_probe::recording=false;
    require(late_completion::target&&late_completion::buffer&&receive_buffer_probe::tracked,
        "actual WSARecv operation and containing buffer allocation are observed");
    require(late_completion::submitted==SOCKET_ERROR&&late_completion::submit_error==WSA_IO_PENDING,
        "real connected receive is kernel-pending before peer data or cancellation");
    fault::target=receive_buffer_probe::socket;
    if(success){
        const char payload='Q';
        require(::send(h.peer.native_handle(),&payload,1,0)==1,"real peer supplies the successful receive byte");
    }else h.client->close();
    const auto until=std::chrono::steady_clock::now()+2s;
    while(!late_completion::cached&&std::chrono::steady_clock::now()<until){
        h.owner.io.restart();h.owner.io.poll_one();
        if(!late_completion::cached)Sleep(1);
    }
    const auto packet=late_completion::packet;
    require(late_completion::cached&&late_completion::captures==1&&packet.operation==late_completion::target,
        "one actual GetQueuedCompletionStatus packet matches the submitted OVERLAPPED");
    require(packet.port&&packet.key==0,"packet comes from the real socket completion port, not a synthetic result key");
    require(success?(packet.ok&&packet.error==ERROR_SUCCESS&&packet.bytes==1&&*late_completion::buffer=='Q'):
        (!packet.ok&&packet.error!=ERROR_SUCCESS&&packet.error!=WAIT_TIMEOUT&&packet.bytes==0),
        "actual native packet records the selected successful or cancelled receive result");
    require(!callbacks&&!receive_buffer_probe::released,"held native completion cannot dispatch or release its buffer");
    std::weak_ptr<net::AsioService::Impl::Client> lifetime=h.client;
    h.client.reset();
    const auto first=std::chrono::steady_clock::now();
    for(unsigned attempt=0;attempt<2;++attempt){
        const auto waits=late_completion::timeouts;
        require(stop_failed(h.owner),"held socket completion must reject each bounded stop attempt");
        require(late_completion::timeouts>waits,"each stop attempt reaches the controlled IOCP wait");
        auto kept=lifetime.lock();
        require(kept&&kept->closed&&kept->pending_reads==1&&!kept->retired(),
            "incomplete stop retains the exact client and pending receive");
        require(h.owner.clients.size()==1&&h.owner.active_sessions==1&&!h.owner.io.stopped(),
            "service retains client capacity and outstanding IO across stop retries");
        require(late_completion::cached&&late_completion::captures==1&&!late_completion::deliveries,
            "retry cannot consume or duplicate the held native packet");
        require(!callbacks&&!receive_buffer_probe::released&&receive_buffer_probe::tracked,
            "business callbacks stay suppressed and exact buffer ownership survives both drain attempts");
        require(fault::closes==1,"completed native socket close is not repeated while IO retirement waits");
    }
    const auto elapsed=std::chrono::steady_clock::now()-first;
    require(elapsed>=100ms&&late_completion::timeouts>=2,
        "real completion delivery remains held beyond the unchanged first 100 ms drain budget");
    delivery.release();h.owner.stop();
    require(late_completion::captures==1&&late_completion::deliveries==1&&!late_completion::cached,
        "retry delivers the original native completion exactly once");
    require(!callbacks&&receive_buffer_probe::released==1&&!receive_buffer_probe::tracked,
        "terminal retirement frees the exact buffer once without a stale business callback");
    require(!h.owner.active_sessions&&h.owner.clients.empty()&&lifetime.expired(),
        "successful retry releases the retained client and session capacity");
    h.owner.stop();
    require(fault::closes==1&&late_completion::deliveries==1&&receive_buffer_probe::released==1,
        "completed cleanup is idempotent without native-close or completion replay");
    std::cout<<"native completion: success="<<success<<" error="<<packet.error<<" bytes="<<packet.bytes
        <<" held_stops=2 elapsed_ms="<<std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
        <<" captures="<<late_completion::captures<<" deliveries="<<late_completion::deliveries<<'\n';
}
void delayed_send_completion(){
    // B50: exercise the actual WSASend -> IOCP callback path with its packet
    // held after kernel dequeue. This is distinct from WSARecv in PR #107.
    Harness h(false);
    require(!h.owner.runner.joinable(),"send completion fixture uses a single IO controller");
    struct SendScope {
        SendScope(SOCKET native){
            require(!late_completion::armed&&!late_completion::cached,
                "previous delayed completion must already be retired");
            late_completion::target=nullptr;late_completion::buffer=nullptr;
            late_completion::packet={};
            late_completion::captures=late_completion::deliveries=late_completion::timeouts=0;
            late_completion::submitted=0;late_completion::submit_error=0;
            late_completion::send_socket=native;
            late_completion::capture_send=true;
            late_completion::holding=late_completion::armed=true;
        }
        void release(){late_completion::holding=late_completion::armed=false;}
        ~SendScope(){
            release();late_completion::capture_send=false;
            late_completion::send_socket=INVALID_SOCKET;
        }
    } gate(h.client->socket.native_handle());
    auto output=std::make_shared<std::string>("PING");
    std::weak_ptr<std::string> borrowed=output;
    std::weak_ptr<net::AsioService::Impl::Client> lifetime=h.client;
    unsigned stale_callbacks=0;
    h.client->async_send(std::span<const char>(output->data(),output->size()),
        [output,&stale_callbacks](server::Error,std::size_t){++stale_callbacks;});
    output.reset();
    require(late_completion::target!=nullptr&&late_completion::buffer!=nullptr&&
            late_completion::submitted!=SOCKET_ERROR ||
            (late_completion::target!=nullptr&&late_completion::buffer!=nullptr&&
             late_completion::submit_error==WSA_IO_PENDING),
        "actual overlapped WSASend was submitted through the native adapter");
    require(h.client->pending_writes==1&&!borrowed.expired(),
        "completion closure owns the borrowed WSASend bytes until dispatch");
    const auto end=std::chrono::steady_clock::now()+2s;
    while(!late_completion::cached&&std::chrono::steady_clock::now()<end){
        h.owner.io.restart();h.owner.io.poll_one();
        if(!late_completion::cached)Sleep(1);
    }
    const auto packet=late_completion::packet;
    require(late_completion::cached&&late_completion::captures==1&&
            packet.operation==late_completion::target&&packet.port&&packet.key==0,
        "the real IOCP send completion was captured exactly once");
    require(packet.ok&&packet.error==ERROR_SUCCESS&&packet.bytes==4,
        "captured send packet reports the actual successful four-byte write");
    require(!borrowed.expired()&&!stale_callbacks,
        "holding the send completion retains output and suppresses callbacks");
    fault::target=late_completion::send_socket;
    h.client->close();
    h.client.reset();
    require(!borrowed.expired()&&!lifetime.expired()&&h.owner.active_sessions==1,
        "cancel/close and caller release cannot retire the undelivered send");
    const auto started=std::chrono::steady_clock::now();
    for(unsigned i=0;i<2;++i){
        const auto waits=late_completion::timeouts;
        require(stop_failed(h.owner),"held WSASend completion must reject bounded shutdown");
        require(late_completion::timeouts>waits&&late_completion::cached&&
                late_completion::captures==1&&!late_completion::deliveries,
            "every failed shutdown preserves the original native completion packet");
        auto kept=lifetime.lock();
        require(kept&&kept->closed&&kept->pending_writes==1&&
                !kept->retired()&&h.owner.active_sessions==1&&
                h.owner.clients.size()==1,
            "held send completion retains the Client and session capacity");
        require(!borrowed.expired()&&!stale_callbacks,
            "pending write bytes remain owned without a business callback");
        require(fault::closes==1,"completed socket close must not be repeated during retry");
    }
    require(std::chrono::steady_clock::now()-started>=100ms,
        "held send delivery crossed the unchanged first 100 ms drain interval");
    gate.release();
    h.owner.stop();
    require(late_completion::captures==1&&late_completion::deliveries==1&&
            !late_completion::cached&&!stale_callbacks,
        "the exact send completion is delivered once without a stale callback");
    require(borrowed.expired()&&lifetime.expired()&&
            h.owner.active_sessions==0&&h.owner.clients.empty(),
        "terminal send completion retires output and Client exactly once");
    h.owner.stop();
    require(late_completion::deliveries==1&&fault::closes==1,
        "later idempotent shutdown cannot repeat native send or close");
    std::cout<<"native delayed WSASend: packet_bytes="<<packet.bytes
        <<" captures="<<late_completion::captures
        <<" deliveries="<<late_completion::deliveries<<'\n';
}
void combined_read_write_close_retry(){
    // A source-linked B50 regression: real WSARecv and WSASend borrow two
    // distinct live allocations while the same native socket close is retained.
    // The retry is caused by an injected WSAEWOULDBLOCK close, not a fabricated
    // Windows completion or a claim that the kernel took over 100 ms.
    Harness h(false);
    struct Reset {
        ~Reset(){receive_buffer_probe::recording=false;receive_buffer_probe::tracked=nullptr;fault::error=0;}
    } reset;
    receive_buffer_probe::count=0;
    receive_buffer_probe::released=0;
    const auto native=h.client->socket.native_handle();
    receive_buffer_probe::socket=native;
    receive_buffer_probe::recording=true;
    unsigned receive_callbacks=0,send_callbacks=0;
    h.client->async_receive(4096,[&](server::Error,std::string){++receive_callbacks;});
    receive_buffer_probe::recording=false;
    require(receive_buffer_probe::tracked!=nullptr&&receive_buffer_probe::released==0,
        "real WSARecv must own the observed receive buffer until completion");
    auto payload=std::make_shared<std::string>(128*1024,'X');
    std::weak_ptr<std::string> borrowed_write=payload;
    h.client->async_send(std::span<const char>(payload->data(),payload->size()),
        [payload,&send_callbacks](server::Error,std::size_t){++send_callbacks;});
    payload.reset();
    require(h.client->pending_reads==1&&h.client->pending_writes==1&&
            !borrowed_write.expired(),
        "both kernel-submitted operations retain independent read/write storage");
    fault::target=native;
    fault::error=WSAEWOULDBLOCK;
    h.client->close();
    require(h.client->pending_reads==1&&h.client->pending_writes==1&&
            receive_buffer_probe::released==0&&!borrowed_write.expired()&&
            h.owner.active_sessions==1&&!h.client->retired(),
        "failed native close must retain both pending operation loans");
    std::weak_ptr<net::AsioService::Impl::Client> lifetime=h.client;
    h.client.reset();
    require(!lifetime.expired(),"failed close may not retire the client at caller release");
    const bool pending=stop_failed(h.owner);
    require(pending&&h.owner.active_sessions==1&&!h.owner.clients.empty()&&
            !lifetime.expired(),
        "first shutdown must fail closed while the native close lease is retained");
    require(receive_callbacks==0&&send_callbacks==0,
        "cancelled operations must never dispatch stale business callbacks");
    // The first run_for may already consume the Windows completions; either
    // timing is valid, but the failing native close still owns the session.
    fault::error=0;
    h.owner.stop();
    require(h.owner.active_sessions==0&&h.owner.clients.empty()&&
            lifetime.expired()&&borrowed_write.expired()&&
            receive_buffer_probe::released==1,
        "successful retry must release both real I/O loans and the client once");
    require(receive_callbacks==0&&send_callbacks==0,
        "retry must not resurrect cancelled read/write business callbacks");
    const auto close_count=fault::closes.load();
    h.owner.stop();
    require(fault::closes==close_count&&receive_buffer_probe::released==1,
        "later shutdown cannot repeat an already-completed native close or buffer free");
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
    server::Endpoint endpoint;endpoint.family=2;endpoint.address[0]=127;endpoint.address[3]=1;
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
        {"receive-completion",receive_cancellation_lifetime},
        {"read-write-retry",combined_read_write_close_retry},
        {"late-receive-cancel",+[]{delayed_receive_completion(false);}},
        {"late-receive-success",+[]{delayed_receive_completion(true);}},
        {"late-send-success",delayed_send_completion},
        {"timer",timer_retirement},{"constructor",constructor_rollback},{"bind",bind_rollback},{"listener",listener_retry}})
        if(selected=="all"||selected==name){fn();ran=true;std::cout<<"PASS: "<<name<<'\n';}
    require(ran,"unknown case");
}catch(const std::exception& e){fault::error=0;fault::hold_timer=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
