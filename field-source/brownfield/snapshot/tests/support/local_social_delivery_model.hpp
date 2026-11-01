#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/runtime/local_social_state.hpp"

namespace preloader_l { namespace runtime {
// MODEL-ONLY transaction cutpoint. This is NOT a production send adapter and
// must not be invoked from untrusted LSX/socket ingress. Its LocalPrincipalLease
// is still minted by a TEST-ONLY issuer. The caller owns external serialization
// across prepare/send/commit; there is no peer authentication or local inbox ACK.
// A successful write is NOT evidence of client receipt, response consumption,
// crash-consistent exactly-once execution, or remote delivery.
enum class LocalSocialWireOutcome : uint8_t { no_bytes, partial_bytes, complete_bytes };
struct LocalSocialWireReceipt { LocalSocialWireOutcome outcome; size_t bytes_sent; };
using LocalSocialModelWrite = LocalSocialWireReceipt (*)(void*,const uint8_t*,size_t) noexcept;
enum class LocalSocialModelDelivery : uint8_t {
    invalid_input, prepare_rejected, no_bytes_rollback, partial_wire_ambiguous,
    inconsistent_wire_receipt, committed_local_only, commit_after_wire_failed
};
// `frame` is a pre-validated, completely serialized success response supplied
// by the trusted test owner; this model DOES NOT validate XML response grammar.
// Rollback affects local state only, NEVER bytes already written to the peer.
// On complete wire write followed by a failed commit the result is explicitly
// indeterminate: the client might have observed success without state mutation.
LocalSocialModelDelivery model_social_delivery(LocalSocialStore& store,
    LocalPrincipalLease lease, LocalSocialAction action, uint64_t target,
    const uint8_t* frame, size_t frame_size, LocalSocialModelWrite writer,
    void* writer_context, LocalSocialResult* prepare_detail) noexcept;
} }
