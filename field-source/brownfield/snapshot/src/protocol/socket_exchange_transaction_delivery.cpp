#include "socket_exchange_transaction_owner_private.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
namespace preloader_l { namespace protocol { namespace detail {
bool prepared_response_frame(PreparedResponseTransaction* transaction,
                             PreparedWireFrame* output) noexcept {
    if (!transaction||!output||transaction->resolved||
        transaction->cursor.phase!=DeliveryWritePhase::response) return false;
    *output=PreparedWireFrame{reinterpret_cast<const uint8_t*>(transaction->wire.data),
                              transaction->response_required};
    return output->bytes && output->length!=0u;
}

// Called exactly once per *completed* Asio async_write or sync send_all.
// `successful` must mean !asio::error_code; short writes are rejected by
// the shared completion cursor. Never commit against a mere write initiation.
PreparedWriteDisposition complete_prepared_transaction_write(
    PreparedResponseTransaction* transaction,bool successful,size_t transferred,
    PreparedWireFrame* next,DispatchTransactionResult* outcome) noexcept {
    if (next) *next=PreparedWireFrame{nullptr,0u};
    if (!transaction||!outcome||!next||transaction->resolved)
        return PreparedWriteDisposition::rejected;
    const bool was_response=transaction->cursor.phase==DeliveryWritePhase::response;
    const auto decision=complete_delivery_write(&transaction->cursor,successful,transferred);
    if (decision==DeliveryWriteDecision::reject) {
        transaction->result.status=ExchangeStatus::send_error;
        if (!was_response && transaction->result.response_sent &&
            transferred <= transaction->event_required &&
            transferred <= static_cast<size_t>(-1)-transaction->result.bytes_sent)
            transaction->result.bytes_sent+=transferred;
        if (!was_response && transaction->result.response_sent) {
            transaction->grant_delivery.retry_event();
            transaction->show_igo_delivery.retry();
            transaction->generic_event_delivery.retry();
        }
        transaction->resolved=true;
        *outcome=transaction->result;
        return PreparedWriteDisposition::rejected;
    }
    transaction->result.bytes_sent+=transferred;
    if (was_response) {
        transaction->grant_delivery.commit_response();
        transaction->result.response_sent=true;
    }
    if (decision==DeliveryWriteDecision::event_needed) {
        *next=PreparedWireFrame{
            reinterpret_cast<const uint8_t*>(transaction->wire.data+transaction->response_required),
            transaction->event_required};
        *outcome=transaction->result;
        return PreparedWriteDisposition::send_event;
    }
    if (decision!=DeliveryWriteDecision::response_done &&
        decision!=DeliveryWriteDecision::transaction_done) {
        transaction->result.status=ExchangeStatus::send_error;
        transaction->resolved=true;
        *outcome=transaction->result;
        return PreparedWriteDisposition::rejected;
    }
    transaction->grant_delivery.commit_event();
    transaction->show_igo_delivery.commit();
    transaction->generic_event_delivery.commit();
    if (!transaction->social_delivery.commit()) {
        transaction->result.status=ExchangeStatus::dispatch_error;
        transaction->resolved=true;
        *outcome=transaction->result;
        return PreparedWriteDisposition::rejected;
    }
    transaction->result.dispatch_status=preloader_l::core::DispatchStatus::ok;
    transaction->resolved=true;
    *outcome=transaction->result;
    return PreparedWriteDisposition::done;
}

void destroy_prepared_response_transaction(PreparedResponseTransaction* transaction) noexcept {
    if(!transaction)return;
    // A cancelled or destroyed owner before full Response sends NO Grant commit.
    // If Response was already fully sent, any uncompleted Event remains retryable.
    if (!transaction->resolved && transaction->result.response_sent) {
        transaction->grant_delivery.retry_event();
        transaction->show_igo_delivery.retry();
        transaction->generic_event_delivery.retry();
    }
    transaction->~PreparedResponseTransaction();
#if defined(_WIN32)
    // The production allocator retains exact ownership for retry if the
    // physical HeapFree fails, including on callback-driven cancellation.
    (void)preloader_l::runtime::release_transient_bytes(transaction);
#endif
}

} } }
