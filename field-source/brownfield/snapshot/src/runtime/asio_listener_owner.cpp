#include "asio_listener_owner.hpp"
#include <asio/post.hpp>
#include <asio/bind_executor.hpp>
#include <utility>
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
#include "asio_acceptance_resource_fault.hpp"
#endif
#if defined(_WIN32) && (!defined(ASIO_HAS_IOCP) || defined(ASIO_DISABLE_IOCP))
#error HF13 Windows listener must use the real Asio IOCP backend.
#endif

namespace preloader_l { namespace runtime {
AsioListenerOwner::AsioListenerOwner(asio::io_context& io) noexcept
    : strand_(asio::make_strand(io)), acceptor_(strand_) {}

AsioListenerOwner::~AsioListenerOwner() noexcept {
    // Callers may drop their final reference only after IOCP handlers drain;
    // asynchronous callbacks each retain shared ownership of this object.
    asio::error_code ignored;
    acceptor_.close(ignored);
}

bool AsioListenerOwner::bind_loopback(uint16_t port, asio::error_code& ec) noexcept {
    if (started_ || stopping_ || acceptor_.is_open()) {
        ec = asio::error::already_open;
        return false;
    }
    const asio::ip::tcp::endpoint address(asio::ip::address_v4::loopback(), port);
    acceptor_.open(address.protocol(), ec);
    if (ec) return false;
    acceptor_.bind(address, ec);
    if (!ec) acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
        asio::error_code ignored;
        acceptor_.close(ignored);
        // Failed close must not be silently forgotten or considered retryable.
        if (ignored) ec = ignored;
        return false;
    }
    return true;
}

uint16_t AsioListenerOwner::bound_port(asio::error_code& ec) const noexcept {
    auto address = acceptor_.local_endpoint(ec);
    return ec ? 0u : address.port();
}

bool AsioListenerOwner::abort_before_start(asio::error_code& ec) noexcept {
    if (started_ || accept_pending_) { ec=asio::error::already_started; return false; }
    ec.clear();
    if (acceptor_.is_open()) acceptor_.close(ec);
    return !ec && !acceptor_.is_open();
}

bool AsioListenerOwner::start(AsioListenerObserver observer) noexcept {
    try {
        auto self = shared_from_this();
        asio::post(strand_, [self, observer]() noexcept {
            if (self->started_ || self->stopping_ || !self->acceptor_.is_open() ||
                !observer.accepted) return;
            self->observer_ = observer;
            self->started_ = true;
            self->arm_accept();
        });
        return true;
    } catch (...) { return false; }
}

bool AsioListenerOwner::request_stop() noexcept {
    try {
        auto self = shared_from_this();
        asio::post(strand_, [self]() noexcept { self->apply_stop(); });
        return true;
    } catch (...) { return false; }
}

bool AsioListenerOwner::session_finished() noexcept {
    try {
        auto self = shared_from_this();
        asio::post(strand_, [self]() noexcept {
            if (self->sessions_ == 0u) {
                // A duplicate completion cannot underflow the owner count.
                return;
            }
            --self->sessions_;
            if (self->stopping_) self->notify_quiesced();
            else self->arm_accept();
        });
        return true;
    } catch (...) { return false; }
}

void AsioListenerOwner::arm_accept() noexcept {
    if (!started_ || stopping_ || accept_pending_ || sessions_ >= kSessionCap)
        return;
    accept_pending_ = true;
    try {
        auto self = shared_from_this();
        acceptor_.async_accept(asio::bind_executor(strand_,
            [self](const asio::error_code& ec, asio::ip::tcp::socket socket) mutable noexcept {
                self->completed_accept(ec, std::move(socket));
            }));
    } catch (...) {
        accept_pending_ = false;
        const asio::error_code resource_failure = asio::error::no_memory;
        if (observer_.failed) observer_.failed(observer_.context, resource_failure);
        apply_stop();
    }
}

void AsioListenerOwner::completed_accept(
    const asio::error_code& ec, asio::ip::tcp::socket socket) noexcept {
    accept_pending_ = false;
    if (stopping_) {
        asio::error_code ignored;
        socket.close(ignored);
        notify_quiesced();
        return;
    }
    if (ec) {
        if (observer_.failed) observer_.failed(observer_.context, ec);
        apply_stop();
        return;
    }
    if (sessions_ >= kSessionCap) { // admission fail-closed, never leak socket
        asio::error_code ignored;
        socket.close(ignored);
        apply_stop();
        return;
    }
    asio::error_code remote_error;
    auto remote = socket.remote_endpoint(remote_error);
    if (remote_error) {
        asio::error_code ignored;
        socket.close(ignored);
        if (observer_.failed) observer_.failed(observer_.context, remote_error);
        apply_stop();
        return;
    }
    ++sessions_; // publish ownership before callback can synchronously complete
    observer_.accepted(observer_.context, std::move(socket), remote);
    arm_accept();
}

void AsioListenerOwner::apply_stop() noexcept {
    if (!stopping_) {
        stopping_ = true;
        asio::error_code cancel_error;
        acceptor_.cancel(cancel_error);
        // Cancel failures do not mean the listener was closed. Exact socket
        // ownership remains with acceptor_ until a successful close.
    }
    if (acceptor_.is_open()) {
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        // The injected failure is a withheld close, NOT fabricated success.
        // The acceptor, io_context and pending callbacks remain owned.
        if (consume_asio_acceptance_listener_close_failure()) {
            if (!acceptance_listener_close_retained_) {
                acceptance_listener_close_retained_=true;
                note_asio_acceptance_listener_close_retained();
            }
            return;
        }
#endif
        asio::error_code close_error;
        acceptor_.close(close_error);
        if (close_error || acceptor_.is_open()) {
            if (observer_.failed) observer_.failed(observer_.context, close_error);
            // A subsequent request_stop() retries the same owned acceptor.
            return;
        }
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        if (acceptance_listener_close_retained_) {
            acceptance_listener_close_retained_=false;
            note_asio_acceptance_listener_close_retired();
        }
#endif
    }
    notify_quiesced();
}

void AsioListenerOwner::notify_quiesced() noexcept {
    if (quiescence_notified_ || !stopping_ || accept_pending_ || sessions_ != 0u ||
        acceptor_.is_open()) return;
    quiescence_notified_ = true;
    if (observer_.quiesced) observer_.quiesced(observer_.context);
}
} }
