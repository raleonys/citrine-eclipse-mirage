#pragma once
// Shared completion contract for sync send_all and future Asio IOCP async_write.
// It decides *when* response/event completion is trustworthy; it does not
// claim a remote/application ACK or own queue/store mutation resources.
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace protocol { namespace detail {

enum class DeliveryWritePhase : uint8_t { response, event, complete, failed };
enum class DeliveryWriteDecision : uint8_t { reject, response_done, event_needed, transaction_done };
struct DeliveryWriteCursor {
    size_t response_length;
    size_t event_length;
    DeliveryWritePhase phase;
};
// Initialization fails closed for an empty Response. Event may be absent.
bool begin_delivery_writes(DeliveryWriteCursor* state,
                           size_t response_length, size_t event_length) noexcept;
// Call ONCE after each full-frame sync send or Asio async_write completion.
// A short/partial completion or any error is terminal, even if n == expected.
// event_needed means the Response was accepted, but the Event must follow;
// transaction_done means no further bytes are pending. Repeated or out-of-
// order completions reject without issuing any further commit authorization.
DeliveryWriteDecision complete_delivery_write(DeliveryWriteCursor* state,
                                               bool successful, size_t transferred) noexcept;

} } }
