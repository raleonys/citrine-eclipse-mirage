#pragma once

#include <stddef.h>

namespace preloader_l { namespace runtime {

// The curio outer consumer drains at most one event per response and pops
// the selected queue slot regardless of processing outcome.
struct PreparedQueuedEvent;
bool publish_show_igo_down_event() noexcept;
bool prepare_show_igo_down_event(PreparedQueuedEvent** prepared) noexcept;
bool publish_achievement_event(const char* xml,size_t length) noexcept;
// Allocate/copy an achievement event without publishing it. Commit is no-fail
// and transfers the prepared node into the FIFO; release discards it.
bool prepare_achievement_event(const char* xml,size_t length,PreparedQueuedEvent** prepared) noexcept;
void commit_prepared_achievement_event(PreparedQueuedEvent* prepared) noexcept;
void release_prepared_achievement_event(PreparedQueuedEvent* prepared) noexcept;

// Response-delivery reservation used by the shipping socket path. A candidate
// may be provisionally inserted at its FIFO position and remains invisible to
// other deliveries until the owning transaction commits. The selected event is
// marked reserved rather than removed, so commit consumes it and rollback
// restores its exact queue position. Reservation bookkeeping holds the delivery
// gate only for short queue mutations; encode/send never owns that global gate.
struct QueuedEventDelivery {
    void* selected;
    PreparedQueuedEvent* candidate;
    bool active;
};
bool begin_queued_event_delivery(PreparedQueuedEvent* candidate,char* output,size_t capacity,
                                 size_t* written,QueuedEventDelivery* delivery) noexcept;
void commit_queued_event_delivery(QueuedEventDelivery* delivery) noexcept;
void rollback_queued_event_delivery(QueuedEventDelivery* delivery) noexcept;
// A completed Response already committed its state: make the selected FIFO
// event retryable without deleting the transaction's provisional candidate.
// No remote receipt or exactly-once assertion is made by this transition.
void resolve_queued_event_delivery_for_retry(QueuedEventDelivery* delivery) noexcept;
size_t show_igo_down_event_size() noexcept;
// Returns the current FIFO head size. If the queue is empty, candidate_size is
// the size of an event that the probed handler will enqueue before the outer
// consumer drains one item.
size_t queued_event_preview_size(size_t candidate_size) noexcept;
// Generic transactional drain: capacity/copy failure never consumes the FIFO
// head; successful copy retires exactly one available event.
bool drain_one_queued_event(char* output,size_t capacity,size_t* written) noexcept;
bool clear_queued_events() noexcept;
void clear_queued_events_for_test() noexcept;
size_t queued_event_count_for_test() noexcept;
size_t queued_event_pending_free_count_for_test() noexcept;

// Clean two-pass integration guard. The behavioral selects/processes/pops one event
// under a single queue transaction; the shipping protocol performs a separate
// measure/render pair. Production socket paths hold this guard across both
// passes so queue head and stateful handler preview cannot drift between them.
// Shipping MSVC uses a blocking SRW lock for this long correctness transaction;
// it must not regress to a tight process-global spin loop.
void begin_outer_dispatch_transaction() noexcept;
void end_outer_dispatch_transaction() noexcept;

} }
