// HF12 IOCP migration Stage01: real Windows-owned Asio IOCP accept/read/cancel/close.
// This is a bounded native ingress proof, NOT the shipping LSX dispatcher.
// The exact production frame scanner is compiled and linked from socket_exchange_wire.cpp.
#include <asio.hpp>
#if !defined(_WIN32) || !defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP)
# error HF12 acceptance must use genuine Windows Asio IOCP; reactor fallback is forbidden.
#endif
#if !defined(_WIN64)
#error HF12 native acceptance requires MSVC x64 (the shipping target); x86 results are not authoritative.
#endif
#include "socket_exchange_internal.hpp"
#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using asio::ip::tcp;
namespace p = preloader_l::protocol;
namespace w = preloader_l::protocol::detail;
using namespace std::chrono_literals;

struct Ingress final : std::enable_shared_from_this<Ingress> {
    tcp::socket socket;
    p::ExchangeState state{};
    std::vector<std::string> frames;
    size_t read_completions = 0;
    size_t aborted_callbacks = 0;
    size_t close_count = 0;
    asio::error_code final_close_error;
    bool unexpected_error = false;

    explicit Ingress(tcp::socket accepted) : socket(std::move(accepted)) {}

    void read() {
        if (state.wire_size >= sizeof(state.wire_response)) {
            unexpected_error = true;
            return;
        }
        auto self = shared_from_this();
        socket.async_read_some(
            asio::buffer(state.wire_response + state.wire_size,
                         sizeof(state.wire_response) - state.wire_size),
            [self](const asio::error_code& ec, std::size_t n) {
                ++self->read_completions;
                if (ec == asio::error::operation_aborted) {
                    ++self->aborted_callbacks;
                    return;
                }
                if (ec || n == 0) {
                    self->unexpected_error = true;
                    return;
                }
                self->state.wire_size += n;
                w::WireFrameView view{};
                while (w::scan_available_wire_frame(&self->state, &view)) {
                    if (!view.consume_length || view.consume_length > self->state.wire_size) {
                        self->unexpected_error = true;
                        return;
                    }
                    self->frames.emplace_back(self->state.wire_response, view.payload_length);
                    w::consume_wire_frame(&self->state, view.consume_length);
                }
                self->read();
            });
    }
    void cancel_pending_read() {
        asio::error_code ec;
        socket.cancel(ec);
        if (ec) unexpected_error = true;
    }
    void close_after_drain() {
        // Caller runs the io_context until pending cancellation handler completes.
        // Never invoke release() or manually close a native SOCKET.
        socket.close(final_close_error);
        ++close_count;
        if (final_close_error) unexpected_error = true;
    }
};

static unsigned checks = 0;
static void verify(bool success, const char* message) {
    ++checks;
    if (!success) throw std::runtime_error(message);
}

static void exercise(unsigned round) {
    asio::io_context io;
    tcp::acceptor listener(io, tcp::endpoint(asio::ip::address_v4::loopback(), 0));
    std::shared_ptr<Ingress> server;
    bool accepted = false;
    asio::error_code accept_error;
    listener.async_accept([&](const asio::error_code& ec, tcp::socket socket) {
        accept_error = ec;
        accepted = true;
        if (!ec) {
            server = std::make_shared<Ingress>(std::move(socket));
            server->read();
        }
    });
    tcp::socket client(io);
    asio::error_code connect_error;
    client.connect(listener.local_endpoint(), connect_error);
    verify(!connect_error, "loopback connection");
    const std::string a = "<LSX><Request id=\"a\"><GetConfig/></Request></LSX>";
    const std::string b = "<LSX><Request id=\"b\"><GetConfig/></Request></LSX>";
    const std::string c = "<LSX><Request id=\"c\"><GetConfig/></Request></LSX>";
    const size_t cut = 1u + round % (a.size() - 2u);
    asio::write(client, asio::buffer(a.data(), cut));
    asio::steady_timer delayed(io);
    delayed.expires_after(2ms);
    delayed.async_wait([&](const asio::error_code& ec) {
        if (ec) return;
        const std::string tail = a.substr(cut) + std::string(1,'\0') +
                                 b + std::string(1,'\0') + c + std::string(1,'\0');
        asio::error_code send_error;
        asio::write(client, asio::buffer(tail), send_error);
        if (send_error && server) server->unexpected_error = true;
    });
    asio::steady_timer cancel_timer(io);
    cancel_timer.expires_after(30ms);
    cancel_timer.async_wait([&](const asio::error_code& ec) {
        if (!ec && server) server->cancel_pending_read();
    });
    io.run_for(200ms);
    verify(accepted && !accept_error, "IOCP async_accept callback");
    verify(server != nullptr, "Asio-owned accepted socket");
    verify(server->frames.size() == 3u, "fragmented/coalesced frame count");
    verify(server->frames[0] == a && server->frames[1] == b && server->frames[2] == c,
           "source-linked frame bytes and order");
    verify(server->aborted_callbacks == 1u, "pending async_read cancellation completion");
    verify(!server->unexpected_error, "no IOCP read/cancel errors");
    verify(server->socket.is_open(), "socket still Asio-owned before close");
    server->close_after_drain();
    verify(!server->socket.is_open() && server->close_count == 1u && !server->final_close_error,
           "Asio native close without release");
    listener.close();
    client.close();
    io.restart();
    io.poll();
    verify(server.use_count() == 1u, "handler owners drained before session destruction");
    server.reset();
}
int main() {
    try {
        for (unsigned round = 0; round < 24u; ++round) exercise(round);
        std::cout << "HF12_STAGE01_REAL_WINDOWS_ASIO_IOCP PASS checks=" << checks
                  << " rounds=24\n";
    } catch(const std::exception& ex) {
        std::cerr << "FAIL after " << checks << " checks: " << ex.what() << "\n";
        return 1;
    }
}
