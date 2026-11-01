#pragma once
// HF13 internal listener/session lifecycle bridge. Shipping-compiled, but the
// ReleaseServer cutover is NOT complete. The caller must request_stop() and
// observe quiescence before retiring the containing io_context.
#include "asio_listener_owner.hpp"
#include "asio_session_owner.hpp"
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace preloader_l { namespace runtime {
struct AsioServiceObserver final {
    void* context{};
    void (*quiesced)(void*) noexcept{};
    void (*failed)(void*, const asio::error_code&) noexcept{};
    void (*session_finished)(void*, preloader_l::protocol::ExchangeStatus) noexcept{};
    // Native bootstrap admission accounting before any session start can send.
    // False triggers fail-closed session retirement and service stop.
    bool (*session_accepted)(void*) noexcept{};
};

class AsioServiceOwner final : public std::enable_shared_from_this<AsioServiceOwner> {
public:
    explicit AsioServiceOwner(asio::io_context& io);
    AsioServiceOwner(const AsioServiceOwner&) = delete;
    AsioServiceOwner& operator=(const AsioServiceOwner&) = delete;
    ~AsioServiceOwner() noexcept;

    // Bind/start while io_context is stopped or not running; remaining changes
    // are serialized on service_strand_. Ports are loopback only.
    bool bind_loopback(uint16_t port, asio::error_code& ec) noexcept;
    uint16_t bound_port(asio::error_code& ec) const noexcept;
    bool abort_before_start(asio::error_code& ec) noexcept;
    bool start(AsioServiceObserver observer) noexcept;
    bool request_stop() noexcept;

private:
    struct SessionEntry final {
        AsioServiceOwner* owner{};
        std::shared_ptr<AsioSessionOwner> session{};
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
        bool started{false};
#endif
    };
    using Strand = asio::strand<asio::io_context::executor_type>;
    static void accepted(void*, asio::ip::tcp::socket&&,
                         const asio::ip::tcp::endpoint&) noexcept;
    static void listener_quiesced(void*) noexcept;
    static void listener_failed(void*,const asio::error_code&) noexcept;
    static void session_finished(void*,preloader_l::protocol::ExchangeStatus) noexcept;
    void add_session(asio::ip::tcp::socket socket, std::string remote) noexcept;
    void retire_session(SessionEntry*,preloader_l::protocol::ExchangeStatus) noexcept;
    void apply_stop() noexcept;
    void fail(const asio::error_code& ec) noexcept;
    void finish() noexcept;

    Strand service_strand_;
    std::shared_ptr<AsioListenerOwner> listener_;
    std::vector<std::unique_ptr<SessionEntry>> sessions_{};
    AsioServiceObserver observer_{};
    std::shared_ptr<AsioServiceOwner> self_hold_{}; // released only at quiescence
    bool started_{false};
    bool stopping_{false};
    bool listener_drained_{false};
    bool completed_{false};
    bool failure_notified_{false};
};
} }
