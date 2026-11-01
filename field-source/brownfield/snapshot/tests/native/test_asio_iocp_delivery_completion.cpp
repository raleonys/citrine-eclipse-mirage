// Windows native IOCP acceptance for the exact production write completion
// discriminator. This tests real async_write completion, not dispatcher,
// commit-to-store, retry queue, close failure or application ACK.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#error HF12 Stage03 native requires Windows Asio IOCP, no reactor fallback.
#endif
#if !defined(_WIN64)
#error HF12 Stage03 native requires the x64 MSVC target.
#endif
#include "socket_exchange_write_completion.hpp"
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using asio::ip::tcp;
namespace w=preloader_l::protocol::detail;
using namespace std::chrono_literals;
static unsigned checks=0;
static void check(bool c,const char* message) {
    ++checks;if(!c)throw std::runtime_error(message);
}
struct Server:std::enable_shared_from_this<Server> {
    tcp::socket socket;
    w::DeliveryWriteCursor cursor{};
    const std::string response{"Response-transaction-frame\0",27};
    const std::string event{"Event-independent-frame\0",24};
    bool response_done=false;
    bool event_done=false;
    bool failed=false;
    explicit Server(tcp::socket socket_):socket(std::move(socket_)){}
    void start(bool with_event) {
        if(!w::begin_delivery_writes(&cursor,response.size(),with_event ? event.size():0)){
            failed=true;return;
        }
        auto self=shared_from_this();
        asio::async_write(socket,asio::buffer(response),[self,with_event](const asio::error_code& ec,size_t n){
            const auto d=w::complete_delivery_write(&self->cursor,!ec,n);
            if(ec || (d!=w::DeliveryWriteDecision::event_needed &&
                      d!=w::DeliveryWriteDecision::response_done)) {self->failed=true;return;}
            self->response_done=true;
            if(!with_event)return;
            // This second write is *strictly subsequent* to full Response
            // completion. It is never coalesced into one commit boundary.
            asio::async_write(self->socket,asio::buffer(self->event),[self](const asio::error_code& ec2,size_t n2){
                auto d2=w::complete_delivery_write(&self->cursor,!ec2,n2);
                if(ec2 || d2!=w::DeliveryWriteDecision::transaction_done){self->failed=true;return;}
                self->event_done=true;
            });
        });
    }
};
static void one_round(bool with_event) {
    asio::io_context io;
    tcp::acceptor listener(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
    std::shared_ptr<Server> server;
    tcp::socket client(io);
    bool accepted=false,received=false,io_failure=false;
    listener.async_accept([&](const asio::error_code& ec,tcp::socket sock){
        if(ec){io_failure=true;return;}
        accepted=true;server=std::make_shared<Server>(std::move(sock));
        server->start(with_event);
    });
    asio::error_code ec;
    client.connect(listener.local_endpoint(),ec);check(!ec,"connect loopback");
    const std::string expected=std::string("Response-transaction-frame\0",27)+
        (with_event?std::string("Event-independent-frame\0",24):std::string());
    auto bytes=std::make_shared<std::string>(expected.size(),'\0');
    asio::async_read(client,asio::buffer(&(*bytes)[0],bytes->size()),
        [&,bytes](const asio::error_code& e,size_t n){
            if(e||n!=expected.size()||*bytes!=expected)io_failure=true;
            else received=true;
        });
    io.run_for(1800ms);
    check(accepted && !io_failure && received,"async IOCP accept/write/read");
    check(server && !server->failed && server->response_done,"full Response completion");
    check(server->event_done==with_event,"event complete if present");
    check(server->cursor.phase==w::DeliveryWritePhase::complete,"cursor terminated exactly once");
    check(w::complete_delivery_write(&server->cursor,true,with_event?24u:27u)
           ==w::DeliveryWriteDecision::reject,"late/duplicate callback rejected");
    server->socket.cancel(ec);check(!ec,"owned socket cancel");
    server->socket.close(ec);check(!ec,"owned socket close without release");
    listener.close();client.close();io.restart();io.poll();
    check(server.use_count()==1u,"completion owners drained");
}
int main() {
    try {
        for(unsigned i=0;i<8;++i) {one_round(false);one_round(true);}
        std::cout<<"HF12_STAGE03_REAL_WINDOWS_IOCP_DELIVERY_COMPLETION PASS checks="<<checks<<"\n";
    }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<"\n";return 1;}
}
