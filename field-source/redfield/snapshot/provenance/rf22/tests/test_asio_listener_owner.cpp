// SPDX-License-Identifier: GPL-3.0-only
// HF13 Stage01: portable source-linked actual Asio TCP accept/close + serialized
// owner lifetime; NOT Windows IOCP native acceptance or shipping cutover.
#include "asio_listener_owner.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

using namespace redfield::runtime;
using asio::ip::tcp;
struct Probe {
    std::shared_ptr<AsioListenerOwner> owner;
    std::vector<tcp::socket> sockets;
    std::size_t accepted{0u}, quiesced{0u}, failed{0u};
    bool finish_immediately{false};
    static void on_accept(void* ctx, tcp::socket&& socket,
                          const tcp::endpoint& remote) noexcept {
        auto* p=static_cast<Probe*>(ctx);
        assert(remote.address().is_loopback());
        ++p->accepted;
        if (p->finish_immediately) {
            asio::error_code ec;
            socket.close(ec);
            assert(!ec);
            assert(p->owner->session_finished());
        } else {
            p->sockets.emplace_back(std::move(socket));
        }
    }
    static void on_quiesced(void* ctx) noexcept {
        ++static_cast<Probe*>(ctx)->quiesced;
    }
    static void on_failed(void* ctx,const asio::error_code&) noexcept {
        ++static_cast<Probe*>(ctx)->failed;
    }
    AsioListenerObserver observer() noexcept {
        return {this,&on_accept,&on_quiesced,&on_failed};
    }
};

static void connect_n(asio::io_context& io, uint16_t port,
                      std::vector<tcp::socket>& clients, unsigned count) {
    for (unsigned i=0u;i<count;++i) {
        tcp::socket client(io);
        asio::error_code ec;
        client.connect(tcp::endpoint(asio::ip::address_v4::loopback(),port),ec);
        assert(!ec);
        clients.emplace_back(std::move(client));
    }
}

static void single_owner_lifecycle(bool immediately) {
    asio::io_context io;
    Probe p;
    p.owner=std::make_shared<AsioListenerOwner>(io);
    p.finish_immediately=immediately;
    asio::error_code ec;
    assert(p.owner->bind_loopback(0,ec) && !ec);
    assert(!p.owner->bind_loopback(0,ec) && ec);
    ec.clear();
    auto port=p.owner->bound_port(ec);
    assert(port!=0u && !ec);
    assert(p.owner->start(p.observer()));
    std::vector<tcp::socket> clients;
    connect_n(io,port,clients,3u);
    io.run_for(std::chrono::milliseconds(500));
    assert(p.accepted==3u && p.failed==0u);
    assert(p.owner->request_stop());
    io.restart();io.run_for(std::chrono::milliseconds(500));
    assert(p.quiesced==(immediately?1u:0u));
    if (!immediately) {
        for (auto& socket:p.sockets) {
            socket.close(ec); assert(!ec);
            assert(p.owner->session_finished());
        }
        p.sockets.clear();
        io.restart();io.run_for(std::chrono::milliseconds(500));
        assert(p.quiesced==1u);
    }
    // Duplicate per-session completion cannot underflow and signal twice.
    assert(p.owner->session_finished());
    io.restart();io.poll();
    assert(p.quiesced==1u && p.failed==0u);
    auto weak=std::weak_ptr<AsioListenerOwner>(p.owner);
    p.owner.reset();
    io.restart();io.poll();
    assert(weak.expired());
}

static void cancelled_accept() {
    asio::io_context io;
    Probe p;
    p.owner=std::make_shared<AsioListenerOwner>(io);
    asio::error_code ec;
    assert(p.owner->bind_loopback(0,ec));
    assert(p.owner->start(p.observer()));
    assert(p.owner->request_stop());
    io.run_for(std::chrono::milliseconds(500));
    assert(p.accepted==0u && p.failed==0u && p.quiesced==1u);
    auto weak=std::weak_ptr<AsioListenerOwner>(p.owner);
    p.owner.reset();
    io.restart();io.poll();
    assert(weak.expired());
}

static void cap_gate() {
    asio::io_context io;
    Probe p;
    p.owner=std::make_shared<AsioListenerOwner>(io);
    asio::error_code ec;
    assert(p.owner->bind_loopback(0,ec));
    const auto port=p.owner->bound_port(ec);
    assert(p.owner->start(p.observer()));
    std::vector<tcp::socket> clients;
    // Prevent a 129th accepted/session-owned socket from being published.
    connect_n(io,port,clients,129u);
    io.run_for(std::chrono::milliseconds(1200));
    assert(p.accepted==128u && p.sockets.size()==128u && p.failed==0u);
    assert(p.owner->request_stop());
    io.restart();io.run_for(std::chrono::milliseconds(250));
    assert(p.quiesced==0u);
    for (auto& s:p.sockets) {
        s.close(ec); assert(!ec);
        assert(p.owner->session_finished());
    }
    p.sockets.clear();
    io.restart();io.run_for(std::chrono::milliseconds(500));
    assert(p.quiesced==1u && p.accepted==128u);
}
int main() {
    single_owner_lifecycle(true);
    single_owner_lifecycle(false);
    cancelled_accept();
    cap_gate();
    std::cout << "HF13_ASIO_LISTENER_OWNER_PORTABLE: PASS\n";
}
