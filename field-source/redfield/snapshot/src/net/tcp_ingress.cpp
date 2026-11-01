// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/tcp_ingress.hpp"
#include "preloader_l/net/frame_ingress.hpp"
#include "asio_listener_owner.hpp"
#include "asio_session_timeout_policy.hpp"
#include <asio/io_context.hpp>
#include <array>
#include <atomic>
#include <exception>
#include <optional>

namespace preloader_l {
using Listener=preloader_l::runtime::AsioListenerOwner;
using Policy=preloader_l::runtime::AsioSessionTimeoutPolicy;
using asio::ip::tcp;
struct TcpIngress::State {
    struct Peer {
        State* owner;
        tcp::socket socket;
        net::IngressState ingress;
        std::array<std::uint8_t,4096> read_buffer;
        Policy deadline;
        std::array<char,TcpIngress::response_capacity> output;
        std::size_t output_size=0,write_offset=0;
        std::unique_ptr<FrameDelivery> delivery;
        bool sending_event=false;
        bool pending=false,closing=false,terminal_read=false;
        Peer(State* s,tcp::socket&& value) noexcept : owner(s),socket(std::move(value)) {
            net::reset_ingress(&ingress);
            deadline.begin_receive(Policy::Clock::now(),false);
        }
    };
    asio::io_context io;
    std::shared_ptr<Listener> listener=std::make_shared<Listener>(io);
    // Listener admission is 128. One spare slot retains a socket if that
    // invariant ever changes, rather than discarding unknown ownership.
    std::array<std::optional<Peer>,129> peers;
    FrameObserver observer;
    std::atomic_flag driver=ATOMIC_FLAG_INIT;
    std::atomic<bool> drained{true};
    std::atomic<std::uint16_t> port{0};
    std::atomic<std::uint64_t> accepted{0},active{0},completed{0},frames{0};
    std::atomic<std::uint64_t> responses{0},bytes_written{0};
    std::atomic<std::uint64_t> events{0},committed_deliveries{0};
    std::atomic<std::uint64_t> malformed{0},capacity{0},truncated{0},timeouts{0},failures{0};
    bool attempted=false,started=false,stopping=false,listener_drained=false,stop_posted=false;

    static void accepted_socket(void* context,tcp::socket&& socket,const tcp::endpoint&) noexcept {
        auto& self=*static_cast<State*>(context);
        for(auto& slot:self.peers) if(!slot) {
            slot.emplace(&self,std::move(socket));
            ++self.accepted;++self.active;
            if(self.stopping) self.close_peer(*slot);
            else self.arm(*slot);
            return;
        }
        // Cannot occur under the pinned listener cap: fail closed on broken
        // ownership rather than acknowledge quiescence with an unowned socket.
        std::terminate();
    }
    static void listener_finished(void* context) noexcept {
        static_cast<State*>(context)->listener_drained=true;
    }
    static void listener_failed(void* context,const asio::error_code&) noexcept {
        auto& self=*static_cast<State*>(context);
        ++self.failures;self.stopping=true;self.stop_posted=false;
    }
    bool close_peer(Peer& peer) noexcept {
        peer.closing=true;
        asio::error_code ignored;
        peer.socket.cancel(ignored);
        asio::error_code error;
        if(peer.socket.is_open())peer.socket.close(error);
        // Retain the slot for retry on an OS close failure.
        return !error && !peer.socket.is_open();
    }
    void reap() noexcept {
        for(auto& slot:peers) if(slot && slot->closing) {
            if(!close_peer(*slot))continue;
            if(slot->pending || slot->socket.is_open())continue;
            if(!listener->session_finished()) {++failures;continue;}
            slot.reset();--active;++completed;
        }
    }
    void arm_write(Peer& peer) noexcept {
        if(peer.closing || stopping)return;
        peer.pending=true;
        try {
            const auto remaining=peer.output_size-peer.write_offset;
            const auto amount=remaining<4096?remaining:4096;
            const auto view=peer.delivery
                ? (peer.sending_event?peer.delivery->completion_event():peer.delivery->response())
                : std::string_view(peer.output.data(),peer.output_size);
            peer.socket.async_write_some(asio::buffer(view.data()+peer.write_offset,amount),
                [&peer](const asio::error_code& error,std::size_t length) noexcept {
                    auto& self=*peer.owner;peer.pending=false;
                    if(self.stopping || peer.closing){self.close_peer(peer);return;}
                    const auto now=Policy::Clock::now();
                    if(peer.deadline.expired(now)!=Policy::Expired::none){++self.timeouts;self.close_peer(peer);return;}
                    if(length>peer.output_size-peer.write_offset){++self.failures;self.close_peer(peer);return;}
                    if(length){peer.write_offset+=length;self.bytes_written.fetch_add(length);peer.deadline.send_progress(now);}
                    if(error || !length){++self.failures;self.close_peer(peer);return;}
                    if(peer.write_offset!=peer.output_size){self.arm_write(peer);return;}
                    if(peer.sending_event)++self.events;
                    else {
                        ++self.responses;
                        if(peer.delivery){
                            if(!peer.delivery->commit()){++self.failures;self.close_peer(peer);return;}
                            ++self.committed_deliveries;
                            const auto event=peer.delivery->completion_event();
                            if(!event.empty()){
                                peer.sending_event=true;peer.output_size=event.size();peer.write_offset=0;
                                peer.deadline.end_send();peer.deadline.begin_send(now);
                                self.arm_write(peer);return;
                            }
                        }
                    }
                    peer.delivery.reset();peer.sending_event=false;
                    peer.output_size=0;peer.write_offset=0;
                    peer.deadline.end_send();peer.deadline.begin_receive(now,peer.ingress.receive.size!=0);
                    self.process(peer);
                });
        }catch(...){peer.pending=false;++failures;close_peer(peer);}
    }
    void process(Peer& peer) noexcept {
        preloader_l::xml::TextSpan frame;
        auto result=net::peek_ingress_frame(&peer.ingress,&frame);
        while(result==net::IngressStatus::frame_ready) {
            std::size_t written=0;
            const bool accepted_frame=observer.prepare
                ? observer.prepare(observer.context,{frame.data,frame.size},peer.output.data(),peer.output.size(),&written,peer.delivery)
                : observer.respond
                    ? observer.respond(observer.context,{frame.data,frame.size},peer.output.data(),peer.output.size(),&written)
                    : observer.received(observer.context,{frame.data,frame.size});
            if(!accepted_frame || written>peer.output.size()){++malformed;close_peer(peer);return;}
            if(peer.delivery){
                const auto response=peer.delivery->response(),event=peer.delivery->completion_event();
                // Fixed peer admission: at most one response and one event,
                // each strictly below the 16 KiB NUL-inclusive frame capacity.
                if(written || response.empty() || response.size()>=response_capacity || event.size()>=response_capacity){++malformed;close_peer(peer);return;}
                written=response.size();
            }
            ++frames;
            if(net::consume_ingress_frame(&peer.ingress,frame.size)!=net::IngressStatus::ok){++malformed;close_peer(peer);return;}
            const auto now=Policy::Clock::now();
            peer.deadline.end_receive();
            if(written) {
                peer.output_size=written;peer.write_offset=0;peer.deadline.begin_send(now);
                arm_write(peer);return; // bounded backpressure: do not read while sending
            }
            peer.deadline.begin_receive(now,peer.ingress.receive.size!=0);
            result=net::peek_ingress_frame(&peer.ingress,&frame);
        }
        if(result!=net::IngressStatus::need_more) {
            if(result==net::IngressStatus::capacity)++capacity;else ++malformed;
            close_peer(peer);return;
        }
        if(peer.terminal_read) {
            if(peer.ingress.receive.size)++truncated;
            close_peer(peer);return;
        }
        arm(peer);
    }
    void arm(Peer& peer) noexcept {
        if(peer.closing || stopping)return;
        const auto available=net::receive_buffer_available(peer.ingress.receive);
        if(!available){++capacity;close_peer(peer);return;}
        const auto amount=available<peer.read_buffer.size()?available:peer.read_buffer.size();
        peer.pending=true;
        try {
            peer.socket.async_read_some(asio::buffer(peer.read_buffer.data(),amount),
                [&peer](const asio::error_code& error,std::size_t length) noexcept {
                    auto& self=*peer.owner;peer.pending=false;
                    if(self.stopping || peer.closing){self.close_peer(peer);return;}
                    const auto now=Policy::Clock::now();
                    if(peer.deadline.expired(now)!=Policy::Expired::none){++self.timeouts;self.close_peer(peer);return;}
                    if(length) {
                        preloader_l::xml::TextSpan frame;
                        const auto result=net::feed_ingress(&peer.ingress,peer.read_buffer.data(),length,&frame);
                        if(result!=net::IngressStatus::frame_ready && result!=net::IngressStatus::need_more) {
                            if(result==net::IngressStatus::capacity)++self.capacity;else ++self.malformed;
                            self.close_peer(peer);return;
                        }
                        peer.deadline.receive_progress(now,peer.ingress.receive.size!=0);
                    }
                    // Complete frames are delivered and replied to before EOF retirement.
                    peer.terminal_read=error || !length;
                    self.process(peer);
                });
        }catch(...){peer.pending=false;++failures;close_peer(peer);}
    }
    void tick() noexcept {
        const auto now=Policy::Clock::now();
        for(auto& slot:peers) if(slot && !slot->closing && slot->deadline.expired(now)!=Policy::Expired::none) {
            ++timeouts;close_peer(*slot);
        }
        if(stopping) {
            if(started && !listener_drained && !stop_posted)stop_posted=listener->request_stop();
            if(!started && !listener_drained) {
                asio::error_code error;
                listener_drained=listener->abort_before_start(error);
            }
            for(auto& slot:peers)if(slot)close_peer(*slot);
        }
        reap();
    }
    void drive(std::chrono::milliseconds budget) noexcept {
        tick();
        try {
            if(io.stopped())io.restart();
            if(budget.count()>0)io.run_for(budget);else io.poll_one();
        }catch(...){++failures;stopping=true;}
        tick();
        if(stopping && listener_drained && active.load()==0) {
            // All network completions have drained. Clear any finite trailing
            // listener bookkeeping posts before allowing io_context retirement.
            try {if(io.stopped())io.restart();io.poll();}
            catch(...){++failures;return;}
            if(io.stopped()){port.store(0);drained.store(true,std::memory_order_release);}
        }
    }
};
namespace {struct ReleaseDriver {std::atomic_flag& flag;~ReleaseDriver(){flag.clear(std::memory_order_release);}};}
TcpIngress::TcpIngress():state_(std::make_unique<State>()){}
TcpIngress::~TcpIngress(){if(!quiescent())std::terminate();}
TcpStartResult TcpIngress::start(FrameObserver observer,std::uint16_t port) noexcept {
    auto& s=*state_;
    if(s.driver.test_and_set(std::memory_order_acquire))return TcpStartResult::busy;
    ReleaseDriver release{s.driver};
    if(s.attempted)return TcpStartResult::invalid_state;
    if(unsigned(bool(observer.received))+unsigned(bool(observer.respond))+unsigned(bool(observer.prepare))!=1)return TcpStartResult::invalid_observer;
    s.attempted=true;s.observer=observer;s.drained.store(false,std::memory_order_release);
    asio::error_code error;
    if(!s.listener->bind_loopback(port,error)) {
        s.stopping=true;s.drive(std::chrono::milliseconds(0));return TcpStartResult::failed;
    }
    const auto bound=s.listener->bound_port(error);
    if(error || !bound || !s.listener->start({&s,State::accepted_socket,State::listener_finished,State::listener_failed})) {
        s.stopping=true;s.drive(std::chrono::milliseconds(0));return TcpStartResult::failed;
    }
    s.started=true;s.port.store(bound,std::memory_order_release);return TcpStartResult::started;
}
bool TcpIngress::poll_for(std::chrono::milliseconds budget) noexcept {
    auto& s=*state_;
    if(s.driver.test_and_set(std::memory_order_acquire))return false;
    ReleaseDriver release{s.driver};
    if(!s.drained.load(std::memory_order_acquire))s.drive(budget);
    return true;
}
bool TcpIngress::stop_for(std::chrono::milliseconds budget) noexcept {
    auto& s=*state_;
    if(s.driver.test_and_set(std::memory_order_acquire))return false;
    ReleaseDriver release{s.driver};
    if(s.drained.load(std::memory_order_acquire))return true;
    s.stopping=true;s.drive(budget);return s.drained.load(std::memory_order_acquire);
}
bool TcpIngress::quiescent() const noexcept{return state_->drained.load(std::memory_order_acquire);}
TcpSnapshot TcpIngress::snapshot() const noexcept {
    const auto& s=*state_;
    return {s.port.load(),s.accepted.load(),s.active.load(),s.completed.load(),s.frames.load(),
        s.malformed.load(),s.capacity.load(),s.truncated.load(),s.timeouts.load(),s.failures.load(),s.responses.load(),s.bytes_written.load(),s.events.load(),s.committed_deliveries.load()};
}
}
