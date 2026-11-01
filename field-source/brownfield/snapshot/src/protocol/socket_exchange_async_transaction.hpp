#pragma once
// Internal Stage04B transport-neutral asynchronous Response/Event write driver.
// The caller MUST hold the driver, its transport and session alive until the
// outstanding I/O completion callback is delivered. IOCP callbacks MUST be
// serialized on one Asio strand. This is not a shipping server/session owner.
#include "socket_exchange_internal.hpp"
namespace preloader_l { namespace protocol { namespace detail {

using StartPreparedAsyncWrite = bool (*)(void* context,
    const uint8_t* bytes,size_t length) noexcept;
using PreparedAsyncFinished = void (*)(void* context,
    const DispatchTransactionResult& outcome) noexcept;

class PreparedAsyncTransaction final {
public:
    PreparedAsyncTransaction(PreparedResponseTransaction* prepared,void* context,
        StartPreparedAsyncWrite start,PreparedAsyncFinished finished) noexcept;
    PreparedAsyncTransaction(const PreparedAsyncTransaction&)=delete;
    PreparedAsyncTransaction& operator=(const PreparedAsyncTransaction&)=delete;
    ~PreparedAsyncTransaction() noexcept;

    // A true return only means the first async write was successfully queued.
    bool begin() noexcept;
    // Exactly one callback per successfully queued async write. A failed full
    // byte count is NOT a successful write. Duplicate callbacks are ignored.
    void completed(bool success,size_t bytes) noexcept;
    // Marks an intent to cancel: caller must ALSO cancel socket operation.
    // Owner remains held until its in-flight completion is observed. A full
    // successful Response completion can still commit Grant before cancellation
    // stops the optional Event. No Event is launched after stop_requested().
    void request_stop() noexcept;
    bool has_pending_write() const noexcept {return pending_;}
    bool is_finished() const noexcept {return finished_;}
    bool stop_requested() const noexcept {return stopping_;}
    DispatchTransactionResult outcome() const noexcept {return outcome_;}
private:
    bool queue(const PreparedWireFrame& frame) noexcept;
    void finish() noexcept;
    PreparedResponseTransaction* transaction_;
    void* context_;
    StartPreparedAsyncWrite start_;
    PreparedAsyncFinished finished_cb_;
    DispatchTransactionResult outcome_{};
    bool pending_{false};
    bool started_{false};
    bool stopping_{false};
    bool finished_{false};
};

} } }
