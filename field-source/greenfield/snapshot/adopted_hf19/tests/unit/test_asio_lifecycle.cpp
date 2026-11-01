// HF12 P2 research-only Asio P3 feasibility; NOT a shipping socket backend.
// This uses localhost sockets, no external network or emulator protocol.
#define ASIO_STANDALONE 1
#define ASIO_NO_TYPEID 1
#include <asio.hpp>
#include <cassert>
#include <array>
#include <iostream>
#include <string>
#include <utility>

static void exercise(uint32_t sequence) {
    asio::io_context io;
    using tcp=asio::ip::tcp;
    tcp::acceptor server(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
    tcp::socket sender(io),receiver(io);
    const auto address=server.local_endpoint();
    assert(address.address().is_loopback());
    std::array<char,12> message{{'H','F','1','2','-','A','S','I','O','-',char('0'+(sequence%10)),0}};
    std::array<char,12> got{};
    bool accepted=false,connected=false,received=false,sent=false;
    server.async_accept(receiver,[&](const asio::error_code& ec) {
        assert(!ec);accepted=true;
        asio::async_read(receiver,asio::buffer(got),[&](const asio::error_code& e,std::size_t n){
            assert(!e && n==got.size());received=true;
            assert(got==message);
            asio::error_code closed;
            receiver.close(closed);assert(!closed);
        });
    });
    sender.async_connect(address,[&](const asio::error_code& ec){
        assert(!ec);connected=true;
        asio::async_write(sender,asio::buffer(message),[&](const asio::error_code& e,std::size_t n){
            assert(!e && n==message.size());sent=true;
            asio::error_code closed;
            sender.close(closed);assert(!closed);
        });
    });
    io.run();
    assert(accepted&&connected&&received&&sent);
    // A canceled wait callback is delivered exactly once with operation_aborted.
    io.restart();
    bool called=false;
    asio::steady_timer timer(io);
    timer.expires_after(std::chrono::hours(1));
    timer.async_wait([&](const asio::error_code& ec){
        assert(ec==asio::error::operation_aborted);called=true;
    });
    asio::post(io,[&]{assert(timer.cancel()==1);});
    io.run();
    assert(called);
}
int main(){
    for(uint32_t i=0;i<32;++i)exercise(i);
    std::cout<<"HF12_ASIO_LOCALHOST_ASYNC_CANCEL: PASS 32/32\n";
}
