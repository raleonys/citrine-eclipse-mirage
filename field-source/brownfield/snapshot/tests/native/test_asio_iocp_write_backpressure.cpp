// IDS07 bounded 32 MiB real Asio async_write against a non-reading TCP peer.
// Genuine Windows x64 IOCP is mandatory for native acceptance. A separately
// marked Linux portable replay exercises algorithm only, never IOCP acceptance.
#include <asio.hpp>
#include "asio_session_timeout_policy.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#if !defined(IDS07_PORTABLE_PROBE)
# if !defined(_WIN32) || !defined(_WIN64) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
#  error IDS07 native write-stall acceptance requires genuine Windows x64 Asio IOCP.
# endif
#endif
using namespace std::chrono_literals;
using asio::ip::tcp;
using Policy=preloader_l::runtime::AsioSessionTimeoutPolicy;
struct Sender : std::enable_shared_from_this<Sender> {
    tcp::socket socket;
    asio::steady_timer timer;
    std::vector<uint8_t> payload;
    Policy deadline;
    size_t observed{0};
    size_t final_bytes{0};
    bool started{false};
    bool timed_out{false};
    bool completed{false};
    bool unexpected_success{false};
    Policy::Time start_at{};
    Policy::Time deadline_at{};
    Policy::Time finished_at{};
    explicit Sender(tcp::socket&& sock)
        :socket(std::move(sock)),timer(socket.get_executor()),payload(32u*1024u*1024u,0x47u){}
    void arm() {
        Policy::Time at{};
        if(!deadline.next(&at)) {unexpected_success=true;return;}
        timer.expires_at(at);
        auto self=shared_from_this();
        timer.async_wait([self](const asio::error_code& e) {
            if(e || self->completed)return;
            const auto now=Policy::Clock::now();
            if(self->deadline.expired(now)==Policy::Expired::send) {
                self->timed_out=true;
                self->deadline_at=now;
                asio::error_code ignored;
                self->socket.cancel(ignored);
                self->socket.close(ignored);
            } else self->arm();
        });
    }
    void run() {
        started=true;
        socket.set_option(asio::socket_base::send_buffer_size(4096));
        socket.set_option(tcp::no_delay(true));
        start_at=Policy::Clock::now();
        deadline.begin_send(start_at);
        arm();
        auto self=shared_from_this();
        asio::async_write(socket,asio::buffer(payload),
            [self](const asio::error_code& ec,size_t transferred)->size_t {
                if(transferred>self->observed) {
                    self->observed=transferred;
                    self->deadline.send_progress(Policy::Clock::now());
                }
                return asio::transfer_all()(ec,transferred);
            },
            [self](const asio::error_code& ec,size_t bytes) {
                self->completed=true;
                self->finished_at=Policy::Clock::now();
                self->final_bytes=bytes;
                if(!self->timed_out || !ec || bytes>=self->payload.size())
                    self->unexpected_success=true;
                self->deadline.end_send();
                try { (void)self->timer.cancel(); } catch (...) { self->unexpected_success=true; }
            });
    }
};
int main() {
    try {
        asio::io_context io;
        tcp::acceptor listener(io,tcp::endpoint(asio::ip::address_v4::loopback(),0));
        tcp::socket peer(io);
        peer.open(tcp::v4());
        peer.set_option(asio::socket_base::receive_buffer_size(1024));
        std::shared_ptr<Sender> sender;
        asio::error_code accept_error;
        bool accepted=false;
        listener.async_accept([&](const asio::error_code& ec,tcp::socket s) {
            accept_error=ec;
            if(ec)return;
            accepted=true;
            sender=std::make_shared<Sender>(std::move(s));
            sender->run();
        });
        peer.connect(listener.local_endpoint());
        io.run_for(11s);
        if(!sender || !accepted || accept_error || !sender->timed_out ||
            !sender->completed || sender->unexpected_success ||
            sender->final_bytes>=sender->payload.size()) {
            std::fprintf(stderr,"IDS07_IOCP_WRITE_BACKPRESSURE FAIL accepted=%d timed_out=%d completed=%d abnormal=%d bytes=%zu\n",
                accepted?1:0,sender&&sender->timed_out?1:0,
                sender&&sender->completed?1:0,sender&&sender->unexpected_success?1:0,
                sender?sender->final_bytes:0u);
            return 3;
        }
        const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(
            sender->deadline_at-sender->start_at).count();
        if(elapsed<4500 || elapsed>10500) {
            std::fprintf(stderr,"IDS07_IOCP_WRITE_BACKPRESSURE FAIL elapsed=%lld\n",
                static_cast<long long>(elapsed));return 4;
        }
        asio::error_code ignored;
        listener.close(ignored);
        peer.close(ignored);
        io.restart();
        io.run_for(150ms);
#if defined(IDS07_PORTABLE_PROBE)
        std::printf("IDS07_PORTABLE_ASYNC_WRITE_MODEL: PASS elapsed_ms=%lld bytes=%zu (NOT_IOCP)\n",
                static_cast<long long>(elapsed),sender->final_bytes);
#else
        std::printf("IDS07_REAL_IOCP_WRITE_BACKPRESSURE: PASS elapsed_ms=%lld bytes=%zu\n",
                static_cast<long long>(elapsed),sender->final_bytes);
#endif
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr,"IDS07_IOCP_WRITE_BACKPRESSURE EXCEPTION %s\n",e.what());return 5;
    }
}
