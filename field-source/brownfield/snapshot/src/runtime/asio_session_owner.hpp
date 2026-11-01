#pragma once
// Production Asio IOCP LSX session; transaction and frame authority shared
// with the shipping protocol engine. Deadline ownership stays on the strand.
#include "../protocol/socket_exchange_internal.hpp"
#include "../protocol/socket_exchange_async_transaction.hpp"
#include "asio_session_timeout_policy.hpp"
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>
#include <asio/steady_timer.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace preloader_l { namespace runtime {
struct AsioSessionObserver final {
    void* context{};
    void (*finished)(void*, preloader_l::protocol::ExchangeStatus) noexcept{};
};

class AsioSessionOwner final : public std::enable_shared_from_this<AsioSessionOwner> {
public:
    using Socket = asio::ip::tcp::socket;
    explicit AsioSessionOwner(Socket&& socket);
    AsioSessionOwner(const AsioSessionOwner&)=delete;
    AsioSessionOwner& operator=(const AsioSessionOwner&)=delete;
    ~AsioSessionOwner() noexcept;
    // All operations are serialized by the socket's executor strand.
    bool start(std::string remote, AsioSessionObserver observer) noexcept;
    bool request_stop() noexcept;
private:
    using Status = preloader_l::protocol::ExchangeStatus;
    using Frame = preloader_l::protocol::detail::WireFrameView;
    using TxResult = preloader_l::protocol::detail::DispatchTransactionResult;
    using TxDriver = preloader_l::protocol::detail::PreparedAsyncTransaction;
    using Strand = asio::strand<asio::any_io_executor>;
    void send_greeting() noexcept;
    void proceed() noexcept;
    void read_more() noexcept;
    void handle_frame(Frame frame) noexcept;
    void stop(Status status) noexcept;
    void maybe_finish() noexcept;
    bool refresh_deadline() noexcept;
    void cancel_deadline() noexcept;
    void write_progress(size_t transferred) noexcept;
    bool begin_write_deadline() noexcept;
    void end_write_deadline() noexcept;
    bool send_prepared(const uint8_t*,size_t) noexcept;
    bool launch_prepared_write(const uint8_t*,size_t) noexcept;
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    bool schedule_stalled_prepared(const uint8_t*,size_t) noexcept;
#endif
    void prepared_finished(const TxResult&) noexcept;
    void resume_transaction() noexcept;
    static bool begin_prepared_write(void*,const uint8_t*,size_t) noexcept;
    static void completed_prepared(void*,const TxResult&) noexcept;

    Socket socket_;
    Strand strand_;
    asio::steady_timer deadline_timer_;
    AsioSessionTimeoutPolicy timeout_policy_{};
    size_t deadline_callbacks_{0u};
    size_t write_observed_{0u};
    uint64_t deadline_generation_{0u};
    preloader_l::protocol::ExchangeState exchange_{};
    AsioSessionObserver observer_{};
    std::string remote_{};
    std::unique_ptr<TxDriver> transaction_{};
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
    // Never compiled into the shipping DLL. Owns only a native F02 test timer.
    std::unique_ptr<asio::steady_timer> f02_stall_timer_{};
    bool acceptance_socket_close_retained_{false};
#endif
    Frame pending_frame_{};
    TxResult pending_result_{};
    char greeting_[256]{};
    size_t greeting_size_{0u};
    uint32_t malformed_streak_{0u};
    bool started_{false};
    bool stopping_{false};
    bool read_pending_{false};
    bool write_pending_{false};
    bool transaction_finished_{false};
    bool completion_notified_{false};
    Status terminal_status_{Status::ok};
};
} }
