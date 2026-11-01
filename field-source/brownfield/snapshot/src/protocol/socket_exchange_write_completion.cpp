#include "socket_exchange_write_completion.hpp"

namespace preloader_l { namespace protocol { namespace detail {
bool begin_delivery_writes(DeliveryWriteCursor* state,
                           size_t response_length, size_t event_length) noexcept {
    if (!state) return false;
    *state = DeliveryWriteCursor{response_length,event_length,DeliveryWritePhase::failed};
    if (!response_length || response_length > static_cast<size_t>(-1) - event_length)
        return false;
    state->phase = DeliveryWritePhase::response;
    return true;
}
DeliveryWriteDecision complete_delivery_write(DeliveryWriteCursor* state,
                                               bool successful, size_t transferred) noexcept {
    if (!state) return DeliveryWriteDecision::reject;
    if (state->phase != DeliveryWritePhase::response && state->phase != DeliveryWritePhase::event)
        return DeliveryWriteDecision::reject;
    const bool response = state->phase == DeliveryWritePhase::response;
    const size_t expected = response ? state->response_length : state->event_length;
    if (!successful || !expected || transferred != expected) {
        state->phase = DeliveryWritePhase::failed;
        return DeliveryWriteDecision::reject;
    }
    if (response && state->event_length) {
        state->phase = DeliveryWritePhase::event;
        return DeliveryWriteDecision::event_needed;
    }
    state->phase = DeliveryWritePhase::complete;
    return response ? DeliveryWriteDecision::response_done : DeliveryWriteDecision::transaction_done;
}
} } }
