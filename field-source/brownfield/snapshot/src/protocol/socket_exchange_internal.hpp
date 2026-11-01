#pragma once

#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/protocol/challenge_handshake.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace protocol { namespace detail {

// Shared Unix-second greeting seed for synchronous and IOCP sessions.
uint32_t current_greeting_seed() noexcept;

struct WireFrameView {
    size_t payload_length;
    size_t consume_length;
};

bool starts_lsx(const char* bytes, size_t length) noexcept;
bool first_ten_hex(const char* bytes, size_t length) noexcept;
// Nonblocking frame detection over ExchangeState::wire_response. Returns true
// only for a complete logical candidate; the caller consumes by consume_wire_frame().
// The existing wire classifier is the sole authority for both sync and async I/O.
bool scan_available_wire_frame(ExchangeState* state, WireFrameView* view) noexcept;
size_t lsx_frame_end(const char* xml, size_t length) noexcept;

ExchangeStatus send_all(preloader_l::net::TransportEndpoint endpoint,
                        const uint8_t* data,
                        size_t length,
                        size_t* total) noexcept;
ExchangeStatus receive_protocol_frame(preloader_l::net::TransportEndpoint endpoint,
                                      ExchangeState* state,
                                      WireFrameView* view) noexcept;
void consume_wire_frame(ExchangeState* state, size_t length) noexcept;

// One received logical frame is classified exactly once; no socket activity or
// transaction commit occurs here. HandshakeResult remains pending until a
// successful full reply send. xml_request points to state->decoded, whose
// lifetime ends at the next frame classification (single serialized session).
enum class WireStepKind : uint8_t { discarded, handshake_reply, xml_request, terminal };
struct ReceivedWireStep {
    WireStepKind kind;
    ExchangeStatus status;
    HandshakeResult handshake;
    size_t xml_length;
};
bool tolerate_malformed_frame(uint32_t* malformed_streak) noexcept;
ReceivedWireStep classify_received_wire_frame(ExchangeState* state, WireFrameView frame,
                                              uint32_t* malformed_streak) noexcept;
bool commit_sent_handshake(ExchangeState* state, WireFrameView frame,
                           const HandshakeResult& pending,
                           uint32_t* malformed_streak) noexcept;

struct DispatchTransactionResult {
    ExchangeStatus status;
    preloader_l::core::DispatchStatus dispatch_status;
    size_t bytes_sent;
    bool response_sent;
};

// Opaque production transaction owner: real dispatcher, Grant store and FIFO
// event reservation, retained ciphertext and independent send-completion
// boundaries. The Asio callback must keep this and its ExchangeState alive.
struct PreparedResponseTransaction;
struct PreparedWireFrame { const uint8_t* bytes; size_t length; };
enum class PreparedWriteDisposition : uint8_t { rejected, send_event, done };
PreparedResponseTransaction* prepare_response_transaction(
    ExchangeState* state,const char* xml,size_t xml_length,
    const preloader_l::core::DispatchEnvironment* environment,
    DispatchTransactionResult* outcome) noexcept;
bool prepared_response_frame(PreparedResponseTransaction*,PreparedWireFrame*) noexcept;
PreparedWriteDisposition complete_prepared_transaction_write(
    PreparedResponseTransaction*,bool successful,size_t transferred,
    PreparedWireFrame* next,DispatchTransactionResult* outcome) noexcept;
void destroy_prepared_response_transaction(PreparedResponseTransaction*) noexcept;

DispatchTransactionResult execute_response_transaction(
    preloader_l::net::TransportEndpoint endpoint,
    ExchangeState* state,
    const char* xml,
    size_t xml_length,
    const preloader_l::core::DispatchEnvironment* environment) noexcept;

} } }
