#include "socket_exchange_async_transaction.hpp"

namespace preloader_l { namespace protocol { namespace detail {

PreparedAsyncTransaction::PreparedAsyncTransaction(
    PreparedResponseTransaction* prepared,void* context,
    StartPreparedAsyncWrite start,PreparedAsyncFinished finished) noexcept
    : transaction_(prepared),context_(context),start_(start),finished_cb_(finished) {
    outcome_.status=ExchangeStatus::invalid_argument;
    outcome_.dispatch_status=preloader_l::core::DispatchStatus::invalid_argument;
}

PreparedAsyncTransaction::~PreparedAsyncTransaction() noexcept {
    // An in-flight callback must hold the owning session/driver alive; deleting
    // the driver with pending_ true is a caller contract violation, not a
    // supported cancellation mechanism. Stage04B never claims shutdown parity.
    if (transaction_) destroy_prepared_response_transaction(transaction_);
}

void PreparedAsyncTransaction::finish() noexcept {
    if (finished_) return;
    pending_=false;
    finished_=true;
    auto* tx=transaction_;
    transaction_=nullptr;
    if (tx) destroy_prepared_response_transaction(tx);
    if (finished_cb_) finished_cb_(context_,outcome_);
}

bool PreparedAsyncTransaction::queue(const PreparedWireFrame& frame) noexcept {
    if (!transaction_||finished_||pending_||stopping_||!start_||
        !frame.bytes||frame.length==0u) return false;
    // StartWrite must only schedule an operation; it cannot synchronously
    // deliver a completion, and its captured callback must retain this driver.
    pending_=true;
    if (start_(context_,frame.bytes,frame.length)) return true;
    pending_=false;
    PreparedWireFrame next{};
    (void)complete_prepared_transaction_write(transaction_,false,0u,&next,&outcome_);
    outcome_.status=ExchangeStatus::send_error;
    finish();
    return false;
}

bool PreparedAsyncTransaction::begin() noexcept {
    if (started_||stopping_||finished_||!transaction_) return false;
    started_=true;
    PreparedWireFrame frame{};
    if (!prepared_response_frame(transaction_,&frame)) {
        outcome_.status=ExchangeStatus::encode_error;
        finish();
        return false;
    }
    if (!start_) {
        outcome_.status=ExchangeStatus::send_error;
        finish();
        return false;
    }
    return queue(frame);
}

void PreparedAsyncTransaction::completed(bool successful,size_t bytes) noexcept {
    if (!pending_||finished_||!transaction_) return;
    pending_=false;
    PreparedWireFrame next{};
    const auto decision=complete_prepared_transaction_write(
        transaction_,successful,bytes,&next,&outcome_);
    if (decision==PreparedWriteDisposition::send_event) {
        if (stopping_) {
            // Full Response may have committed, but the queued Event must be
            // retained for retry and never launched on a closing session.
            outcome_.status=ExchangeStatus::send_error;
            finish();
        } else {
            (void)queue(next);
        }
        return;
    }
    if (decision==PreparedWriteDisposition::rejected)
        outcome_.status=ExchangeStatus::send_error;
    finish();
}

void PreparedAsyncTransaction::request_stop() noexcept {
    if (finished_||stopping_) return;
    stopping_=true;
    // Do not release an outstanding frame/transaction from under IOCP.
    // Caller cancels the socket and still delivers its completion callback.
    if (!pending_) {
        outcome_.status=ExchangeStatus::send_error;
        finish();
    }
}

} } }
