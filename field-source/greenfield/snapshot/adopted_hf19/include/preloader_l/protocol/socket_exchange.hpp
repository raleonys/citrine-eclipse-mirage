#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/socket_transport.hpp"
#include "preloader_l/runtime/local_social_host.hpp"

namespace preloader_l { namespace protocol {

enum class ExchangeStatus : uint8_t {
    ok = 0, invalid_argument, receive_error, peer_closed, frame_error,
    handshake_error, decode_error, dispatch_error, encode_error, send_error,
    resource_error, send_timeout, receive_timeout
};

struct SessionPeerContext {
    const char* address;
    size_t address_size;
};

struct ExchangeState {
    uint8_t session_key[16];
    bool established;
    // Explicitly configured LOCAL-ONLY host identity, bound after handshake.
    // This is not remote Origin login/authentication.
    preloader_l::runtime::LocalSocialSession local_social_session;
    // The bounded wire buffer is 4096 bytes. Strict hex decoding therefore
    // produces at most 2048 decoded bytes.
    uint8_t decoded[2048];
    char plain_response[4096];
    // HF18/F1485-001: the existing 4 KiB wire buffer is now a bounded stream
    // reassembly buffer. It retains incomplete recv prefixes and any bytes
    // trailing the first complete logical frame; no retired 8 KiB accumulator
    // is reintroduced.
    char wire_response[4096];
    size_t wire_size;
};

void reset_exchange(ExchangeState* state) noexcept;
ExchangeStatus serve_one_protocol_exchange(preloader_l::net::TransportEndpoint endpoint,
                                           ExchangeState* state,
                                           size_t* bytes_sent) noexcept;
// Persistent curio session: greeting once, then repeated recv/dispatch/send
// until peer close or a local protocol error. HF18/F1485-001 retains incomplete
// stream prefixes across recv calls and independently consumes coalesced frames
// inside the existing bounded 4 KiB wire buffer.
ExchangeStatus serve_protocol_session(preloader_l::net::TransportEndpoint endpoint,
                                      ExchangeState* state,
                                      size_t* bytes_sent,
                                      uint32_t* frames_dispatched) noexcept;
ExchangeStatus serve_protocol_session_with_peer(
    preloader_l::net::TransportEndpoint endpoint,
    ExchangeState* state,
    SessionPeerContext peer,
    size_t* bytes_sent,
    uint32_t* frames_dispatched) noexcept;

} }
