#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/config/store_runtime.hpp"

namespace preloader_l { namespace config {

enum class StorePayloadStatus : uint8_t {
    ok = 0,
    not_loaded,
    no_records,
    invalid_argument,
    output_too_small,
    size_overflow,
};

// Evidence-backed reconstruction of protected D5F0 / server +0x530.
//
// Oracle contract recovered from the supplied original binary:
//   output = (prefix_override if non-empty else 38 zero bytes)
//          + per-Entitlement body
//          + trailer
//
// Each Entitlement body is either:
//   * ProtoBytes verbatim, when ProtoBytes is non-empty; or
//   * generated protobuf-wire fields followed by Pad verbatim.
//
// AE00 auxiliary vector #2 is Store@PreambleB64 decoded from Base64; vector
// #3 is Store@TrailerB64 decoded from Base64. The explicit-span APIs remain
// available for oracle/regression work. The overloads without span arguments
// consume the active runtime Store snapshot's curio AE00 auxiliary spans.
StorePayloadStatus runtime_store_payload_size(
    StoreByteView prefix_override,
    StoreByteView trailer,
    size_t* output_size) noexcept;
StorePayloadStatus runtime_store_payload_size(size_t* output_size) noexcept;
StorePayloadStatus store_payload_size_from_source(
    const StoreMaterializationSource& source, size_t* output_size) noexcept;

// Transactional writer: on output_too_small, no output bytes are written and
// output_size receives the required size. A null output is valid only when
// capacity is zero and the required size is zero; use runtime_store_payload_size
// for size queries.
StorePayloadStatus build_runtime_store_payload(
    StoreByteView prefix_override,
    StoreByteView trailer,
    uint8_t* output,
    size_t capacity,
    size_t* output_size) noexcept;
StorePayloadStatus build_runtime_store_payload(
    uint8_t* output,
    size_t capacity,
    size_t* output_size) noexcept;
StorePayloadStatus build_store_payload_from_source(
    const StoreMaterializationSource& source,
    uint8_t* output, size_t capacity, size_t* output_size) noexcept;

} }
