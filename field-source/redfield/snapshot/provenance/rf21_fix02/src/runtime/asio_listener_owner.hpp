#pragma once
// HF13 internal Asio listener ownership boundary. This is not wired into the
// production service's accept/worker dispatch yet. Keep the public 548-byte
// service ABI completely unchanged. The callback owns each accepted socket.
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace preloader_l { namespace runtime {
struct AsioListenerObserver final {
    void* context{nullptr};
    // Called on the listener strand; the callback takes ownership of the socket.
    // Every successful accept must be paired with one session_finished().
    void (*accepted)(void*, asio::ip::tcp::socket&&,
                     const asio::ip::tcp::endpoint&) noexcept {nullptr};
    // Fired exactly once when the stop request, pending accept completion, and
    // externally owned accepted sessions have all drained.
    void (*quiesced)(void*) noexcept {nullptr};
    // A non-cancellation accept failure is terminal, never a hot retry loop.
    void (*failed)(void*, const asio::error_code&) noexcept {nullptr};
};

class AsioListenerOwner final : public std::enable_shared_from_this<AsioListenerOwner> {
public:
    explicit AsioListenerOwner(asio::io_context& io) noexcept;
    AsioListenerOwner(const AsioListenerOwner&) = delete;
    AsioListenerOwner& operator=(const AsioListenerOwner&) = delete;
    ~AsioListenerOwner() noexcept;

    // Configure while io_context is NOT being run, before start().
    // Caller checks error_code; no exception-based socket setup API.
    bool bind_loopback(uint16_t port, asio::error_code& ec) noexcept;
    uint16_t bound_port(asio::error_code& ec) const noexcept;
    // The owning shared_ptr must remain valid across start() publication.
    // start(), request_stop(), and session_finished() may be called from any
    // thread. All mutable ownership state is serialized by the Asio strand.
    // false means the post could not be scheduled (including allocation
    // failure); the owner/caller must retain state and retry, never promote
    // failure to successful retirement.
    // Only before start: close an uncommitted bind without transferring the
    // native socket to any other owner. A close error leaves this owner live.
    bool abort_before_start(asio::error_code& ec) noexcept;
    bool start(AsioListenerObserver observer) noexcept;
    bool request_stop() noexcept;
    bool session_finished() noexcept;
private:
    using Strand = asio::strand<asio::io_context::executor_type>;
    void arm_accept() noexcept;
    void completed_accept(const asio::error_code& ec,
                          asio::ip::tcp::socket socket) noexcept;
    void apply_stop() noexcept;
    void notify_quiesced() noexcept;

    Strand strand_;
    asio::ip::tcp::acceptor acceptor_;
    AsioListenerObserver observer_{};
    std::size_t sessions_{0u};
    bool started_{false};
    bool stopping_{false};
    bool accept_pending_{false};
    bool quiescence_notified_{false};
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    bool acceptance_listener_close_retained_{false};
#endif
    static constexpr std::size_t kSessionCap = 128u;
};
} }
