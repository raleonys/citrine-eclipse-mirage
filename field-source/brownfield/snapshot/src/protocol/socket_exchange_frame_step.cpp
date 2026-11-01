// HF12 Stage02: a single source-authoritative frame/handshake/decode decision
// for legacy WinSock sessions and future Asio IOCP-owned continuations.
// No socket I/O, async callback, event queue or transaction commit is performed
// by this module. See socket_exchange_transaction.cpp for response/event policy.
#include "socket_exchange_internal.hpp"
#include "preloader_l/codec/message_codec.hpp"

namespace preloader_l { namespace protocol { namespace detail {
namespace {
constexpr uint32_t kMalformedFrameLimit = 32u;
bool ignored_handshake_status(HandshakeStatus status) noexcept {
    return status == HandshakeStatus::malformed_request ||
           status == HandshakeStatus::wrong_command ||
           status == HandshakeStatus::missing_id ||
           status == HandshakeStatus::missing_key;
}
ReceivedWireStep terminal(ExchangeStatus status) noexcept {
    return ReceivedWireStep{WireStepKind::terminal,status,HandshakeResult{},0u};
}
ReceivedWireStep discard_or_fail(ExchangeState* s,WireFrameView frame,
                                 uint32_t* streak) noexcept {
    consume_wire_frame(s,frame.consume_length);
    return tolerate_malformed_frame(streak)
        ? ReceivedWireStep{WireStepKind::discarded,ExchangeStatus::ok,HandshakeResult{},0u}
        : terminal(ExchangeStatus::frame_error);
}
} // namespace

bool tolerate_malformed_frame(uint32_t* streak) noexcept {
    if (!streak || *streak >= kMalformedFrameLimit) return false;
    ++*streak;
    return true;
}
ReceivedWireStep classify_received_wire_frame(ExchangeState* s, WireFrameView frame,
                                              uint32_t* malformed_streak) noexcept {
    if (!s || !malformed_streak || frame.consume_length == 0u ||
        frame.consume_length > s->wire_size || frame.payload_length > frame.consume_length)
        return terminal(ExchangeStatus::invalid_argument);
    const char* input=s->wire_response;
    const size_t received=frame.payload_length;
    // A fresh <LSX> after a previous key remains a new handshake, never an
    // encrypted command. Reset the prior social binding before validation.
    if (starts_lsx(input,received)) {
        s->local_social_session=preloader_l::runtime::LocalSocialSession{};
        HandshakeResult pending{};
        const auto hs=process_challenge_request(input,received,s->plain_response,
                                                sizeof(s->plain_response),&pending);
        if (hs!=HandshakeStatus::ok) {
            if (ignored_handshake_status(hs)) return discard_or_fail(s,frame,malformed_streak);
            return terminal(ExchangeStatus::handshake_error);
        }
        if (pending.response_length>=sizeof(s->plain_response))
            return terminal(ExchangeStatus::handshake_error);
        return ReceivedWireStep{WireStepKind::handshake_reply,ExchangeStatus::ok,pending,0u};
    }
    if (!s->established || !first_ten_hex(input,received))
        return discard_or_fail(s,frame,malformed_streak);

    size_t decoded_n=0u;
    const auto ds=preloader_l::codec::decode_payload(s->session_key,16,input,received,
                                                      s->decoded,sizeof(s->decoded),&decoded_n);
    if (ds!=preloader_l::codec::MessageCodecStatus::ok) {
        if (ds==preloader_l::codec::MessageCodecStatus::invalid_hex)
            return discard_or_fail(s,frame,malformed_streak);
        return terminal(ExchangeStatus::decode_error);
    }
    if (!starts_lsx(reinterpret_cast<const char*>(s->decoded),decoded_n))
        return discard_or_fail(s,frame,malformed_streak);
    size_t xml_n=lsx_frame_end(reinterpret_cast<const char*>(s->decoded),decoded_n);
    if (xml_n==0u) xml_n=decoded_n;
    return ReceivedWireStep{WireStepKind::xml_request,ExchangeStatus::ok,HandshakeResult{},xml_n};
}

bool commit_sent_handshake(ExchangeState* s,WireFrameView frame,
                           const HandshakeResult& pending,uint32_t* malformed_streak) noexcept {
    if (!s || !malformed_streak || !pending.response_length ||
        pending.response_length >= sizeof(s->plain_response) ||
        frame.consume_length == 0u || frame.consume_length > s->wire_size) return false;
    // Commit must happen only after the peer-facing full reply send succeeds;
    // bind host Config2 identity, never peer/request-supplied UserId.
    for (unsigned i=0u;i<16u;++i) s->session_key[i]=pending.session_key[i];
    s->established=true;
    (void)preloader_l::runtime::bind_host_social_profile(&s->local_social_session);
    *malformed_streak=0u;
    consume_wire_frame(s,frame.consume_length);
    return true;
}
} } }
