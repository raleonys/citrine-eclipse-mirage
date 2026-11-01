#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/protocol/challenge_handshake.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "socket_exchange_internal.hpp"
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
extern "C" {
struct PreloaderFileTime { uint32_t low; uint32_t high; };
__declspec(dllimport) void __stdcall GetSystemTimeAsFileTime(PreloaderFileTime*);
}
#endif

namespace preloader_l { namespace protocol {
namespace {
// The synchronous compatibility exchange validates its three callbacks locally.
// TransportEndpoint remains an ABI-neutral value carrier; the old generic
// socket_transport.cpp utility is no longer linked into normal Production.
// This is exactly the pre-existing net::valid_endpoint predicate.
bool valid_exchange_endpoint(const preloader_l::net::TransportEndpoint& ep) noexcept {
    return ep.receive != nullptr && ep.send != nullptr && ep.close != nullptr;
}

void secure_zero_bytes(void* memory, size_t size) noexcept {
    if (!memory) return;
    auto* bytes = static_cast<volatile uint8_t*>(memory);
    while (size != 0u) {
        *bytes++ = 0u;
        --size;
    }
}

} // anonymous helpers
namespace detail {
uint32_t current_greeting_seed() noexcept {
#if defined(_WIN32)
    PreloaderFileTime ft{}; GetSystemTimeAsFileTime(&ft);
    const uint64_t ticks=(static_cast<uint64_t>(ft.high)<<32)|ft.low;
    constexpr uint64_t epoch=UINT64_C(116444736000000000);
    if(ticks<=epoch) return 0;
    return static_cast<uint32_t>((ticks-epoch)/UINT64_C(10000000));
#else
    return 0;
#endif
}

}  // namespace detail

void reset_exchange(ExchangeState* s) noexcept {
    if (!s) return;
    // HOTFIX10/F1599-001: ExchangeState owns plaintext, decoded payload and
    // wire/ciphertext buffers in addition to the session key. Volatile writes
    // make teardown zeroization observable to the abstract machine so it is
    // not optimized away immediately before heap release.
    secure_zero_bytes(s, sizeof(*s));
}

ExchangeStatus serve_one_protocol_exchange(preloader_l::net::TransportEndpoint ep,
                                           ExchangeState* s,
                                           size_t* bytes_sent) noexcept {
    if (bytes_sent) *bytes_sent = 0;
    if (!s || !bytes_sent || !valid_exchange_endpoint(ep))
        return ExchangeStatus::invalid_argument;

    // Approved quality promotion: build a per-session behavioral-derived greeting.
    // Wire length still includes the terminal NUL.
    size_t total_sent = 0;
    size_t sent = 0;
    char greeting[256]{}; size_t greeting_n=0;
    if (build_challenge_greeting(detail::current_greeting_seed(),greeting,sizeof(greeting),&greeting_n)!=GreetingStatus::ok)
        return ExchangeStatus::handshake_error;
    ExchangeStatus status = detail::send_all(ep, reinterpret_cast<const uint8_t*>(greeting),
                                             greeting_n + 1, &sent);
    if (status != ExchangeStatus::ok) return status;
    total_sent += sent;

    char* input = s->wire_response;
    detail::WireFrameView frame{};
    status = detail::receive_protocol_frame(ep, s, &frame);
    if (status != ExchangeStatus::ok) return status;
    size_t received = frame.payload_length;

    // A complete logical frame beginning exactly with <LSX> is the handshake path.
    if (detail::starts_lsx(input, received)) {
        HandshakeResult h{};
        if (process_challenge_request(input, received, s->plain_response,
                                      sizeof(s->plain_response), &h) != HandshakeStatus::ok)
            return ExchangeStatus::handshake_error;
        // v4 handshake wire length includes the terminal NUL.
        if (h.response_length >= sizeof(s->plain_response)) return ExchangeStatus::handshake_error;
        status = detail::send_all(ep, reinterpret_cast<const uint8_t*>(s->plain_response),
                                  h.response_length + 1, &sent);
        if (status != ExchangeStatus::ok) return status;
        total_sent += sent;
        for (unsigned i = 0; i < 16; ++i) s->session_key[i] = h.session_key[i];
        s->established = true;
        // Bind only the host-controlled Config2 profile, never peer address or
        // request-supplied UserId. Unconfigured profiles stay unbound.
        s->local_social_session = preloader_l::runtime::LocalSocialSession{};
        (void)preloader_l::runtime::bind_host_social_profile(&s->local_social_session);

        // HF18/F1485-001: consume only the complete handshake. Any coalesced
        // trailing frame remains buffered and is selected without requiring a
        // fresh recv; a fragmented next frame is completed across recv calls.
        detail::consume_wire_frame(s, frame.consume_length);
        status = detail::receive_protocol_frame(ep, s, &frame);
        if (status != ExchangeStatus::ok) return status;
        received = frame.payload_length;
    }

    if (!s->established || !detail::first_ten_hex(input, received))
        return ExchangeStatus::frame_error;

    size_t decoded_n = 0;
    if (preloader_l::codec::decode_payload(s->session_key, 16, input, received,
                                       s->decoded, sizeof(s->decoded), &decoded_n) !=
        preloader_l::codec::MessageCodecStatus::ok)
        return ExchangeStatus::decode_error;

    if (!detail::starts_lsx(reinterpret_cast<const char*>(s->decoded), decoded_n))
        return ExchangeStatus::frame_error;
    size_t xml_n = detail::lsx_frame_end(reinterpret_cast<const char*>(s->decoded), decoded_n);
    if (xml_n == 0) xml_n = decoded_n;

    const auto transaction = detail::execute_response_transaction(
        ep, s, reinterpret_cast<const char*>(s->decoded), xml_n, nullptr);
    if (transaction.status != ExchangeStatus::ok) return transaction.status;
    if (!transaction.response_sent) return ExchangeStatus::dispatch_error;

    total_sent += transaction.bytes_sent;
    detail::consume_wire_frame(s, frame.consume_length);
    *bytes_sent = total_sent;
    return ExchangeStatus::ok;
}

ExchangeStatus serve_protocol_session_with_peer(
                                      preloader_l::net::TransportEndpoint ep,
                                      ExchangeState* s,
                                      SessionPeerContext peer,
                                      size_t* bytes_sent,
                                      uint32_t* frames_dispatched) noexcept {
    if (bytes_sent) *bytes_sent = 0;
    if (frames_dispatched) *frames_dispatched = 0;
    if (!s || !bytes_sent || !frames_dispatched || !valid_exchange_endpoint(ep))
        return ExchangeStatus::invalid_argument;

    reset_exchange(s);
    size_t total_sent = 0;
    size_t sent = 0;
    uint32_t malformed_streak = 0;
    char greeting[256]{}; size_t greeting_n=0;
    if (build_challenge_greeting(detail::current_greeting_seed(),greeting,sizeof(greeting),&greeting_n)!=GreetingStatus::ok)
        return ExchangeStatus::handshake_error;
    ExchangeStatus status = detail::send_all(ep, reinterpret_cast<const uint8_t*>(greeting),
                                             greeting_n + 1, &sent);
    if (status != ExchangeStatus::ok) return status;
    total_sent += sent;

    for (;;) {
        detail::WireFrameView frame{};
        status = detail::receive_protocol_frame(ep, s, &frame);
        if (status == ExchangeStatus::peer_closed) {
            *bytes_sent = total_sent;
            return ExchangeStatus::peer_closed;
        }
        if (status != ExchangeStatus::ok) return status;
        // Stage02: both the legacy WinSock session and the intended Asio IOCP
        // continuation use exactly one production handshake/decode classifier.
        // No response/event transaction ownership is moved before the async
        // send-completion boundary exists.
        const auto step = detail::classify_received_wire_frame(s, frame, &malformed_streak);
        if (step.kind == detail::WireStepKind::discarded) continue;
        if (step.kind == detail::WireStepKind::terminal) return step.status;
        if (step.kind == detail::WireStepKind::handshake_reply) {
            // Commit the session key only after the full handshake reply has
            // been sent. This is also the future Asio async_write completion
            // boundary; a partially sent reply must not establish a session.
            status = detail::send_all(ep, reinterpret_cast<const uint8_t*>(s->plain_response),
                                      step.handshake.response_length + 1u, &sent);
            if (status != ExchangeStatus::ok) return status;
            total_sent += sent;
            if (!detail::commit_sent_handshake(s, frame, step.handshake, &malformed_streak))
                return ExchangeStatus::handshake_error;
            continue;
        }
        if (step.kind != detail::WireStepKind::xml_request)
            return ExchangeStatus::dispatch_error;

        preloader_l::core::DispatchEnvironment environment{peer.address, peer.address_size};
        environment.local_social_session=&s->local_social_session;
        const auto transaction = detail::execute_response_transaction(
            ep, s, reinterpret_cast<const char*>(s->decoded), step.xml_length, &environment);
        if (transaction.status != ExchangeStatus::ok) return transaction.status;

        if (!transaction.response_sent) {
            if (transaction.dispatch_status == preloader_l::core::DispatchStatus::incomplete_request ||
                transaction.dispatch_status == preloader_l::core::DispatchStatus::malformed_request) {
                detail::consume_wire_frame(s, frame.consume_length);
                if (!detail::tolerate_malformed_frame(&malformed_streak))
                    return ExchangeStatus::frame_error;
                continue;
            }
            // Phase 1392 + F1427-001: registered-handler unsupported cases and
            // true unknown router misses emit no normal response and the same
            // persistent session continues to the next recv. Other dispatcher
            // failures remain terminal here.
            if (transaction.dispatch_status == preloader_l::core::DispatchStatus::unsupported_route ||
                transaction.dispatch_status == preloader_l::core::DispatchStatus::unknown_route ||
                transaction.dispatch_status == preloader_l::core::DispatchStatus::no_response_required) {
                malformed_streak = 0;
                detail::consume_wire_frame(s, frame.consume_length);
                continue;
            }
            return ExchangeStatus::dispatch_error;
        }

        total_sent += transaction.bytes_sent;
        malformed_streak = 0;
        detail::consume_wire_frame(s, frame.consume_length);
        ++*frames_dispatched;
        *bytes_sent = total_sent;
    }
}

ExchangeStatus serve_protocol_session(preloader_l::net::TransportEndpoint ep,
                                      ExchangeState* s,
                                      size_t* bytes_sent,
                                      uint32_t* frames_dispatched) noexcept {
    return serve_protocol_session_with_peer(
        ep, s, SessionPeerContext{nullptr, 0u}, bytes_sent, frames_dispatched);
}

} }
