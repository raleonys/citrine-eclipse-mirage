#include "preloader_l/protocol/socket_exchange.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/core/request_dispatch.hpp"
#include "preloader_l/handlers/route_handlers.hpp"
#include "preloader_l/runtime/event_queue.hpp"
#include "../../src/protocol/socket_exchange_internal.hpp"
#include <cstring>

namespace preloader_l { namespace runtime {
void begin_outer_dispatch_transaction() noexcept {}
void end_outer_dispatch_transaction() noexcept {}
} }

namespace preloader_l { namespace handlers {
HandlerStatus prepare_grant_achievement_delivery(const preloader_l::xml::RequestView&,char*,size_t,size_t*,GrantAchievementDelivery*) noexcept { return HandlerStatus::response_error; }
preloader_l::runtime::PreparedQueuedEvent* take_grant_achievement_delivery_event(GrantAchievementDelivery*) noexcept { return nullptr; }
void commit_grant_achievement_delivery_store(GrantAchievementDelivery*) noexcept {}
void finish_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {}
void rollback_grant_achievement_delivery(GrantAchievementDelivery*) noexcept {}
} }
namespace preloader_l { namespace runtime {
bool begin_queued_event_delivery(PreparedQueuedEvent*,char*,size_t,size_t*,QueuedEventDelivery*) noexcept { return false; }
void commit_queued_event_delivery(QueuedEventDelivery*) noexcept {}
void rollback_queued_event_delivery(QueuedEventDelivery*) noexcept {}
void resolve_queued_event_delivery_for_retry(QueuedEventDelivery*) noexcept {}
} }

namespace preloader_l { namespace core {
DispatchResult dispatch_plaintext_request(const char*, size_t, char*, size_t) noexcept {
    return DispatchResult{DispatchStatus::invalid_argument, static_cast<preloader_l::router::RouteId>(0),
                          preloader_l::handlers::HandlerStatus::invalid_request, 0};
}
DispatchResult dispatch_plaintext_request_with_environment(
    const char*, size_t, char*, size_t, const DispatchEnvironment*) noexcept {
    return DispatchResult{DispatchStatus::invalid_argument, static_cast<preloader_l::router::RouteId>(0),
                          preloader_l::handlers::HandlerStatus::invalid_request, 0};
}
} }

namespace {
struct FakeTransport {
    const char* chunks[80]{};
    size_t sizes[80]{};
    unsigned chunk_count = 0;
    unsigned receive_index = 0;
    char output[8192]{};
    size_t output_size = 0;
};

preloader_l::net::TransportStatus receive(void* context, uint8_t* out, size_t capacity,
                                          size_t* received) noexcept {
    auto* f = static_cast<FakeTransport*>(context);
    if (!received) return preloader_l::net::TransportStatus::invalid_argument;
    *received = 0;
    if (f->receive_index >= f->chunk_count)
        return preloader_l::net::TransportStatus::closed;
    const size_t n = f->sizes[f->receive_index];
    if (n > capacity) return preloader_l::net::TransportStatus::io_error;
    std::memcpy(out, f->chunks[f->receive_index], n);
    ++f->receive_index;
    *received = n;
    return preloader_l::net::TransportStatus::ok;
}

preloader_l::net::TransportStatus send(void* context, const uint8_t* data, size_t length,
                                       size_t* sent) noexcept {
    auto* f = static_cast<FakeTransport*>(context);
    if (!sent || f->output_size + length > sizeof(f->output))
        return preloader_l::net::TransportStatus::io_error;
    std::memcpy(f->output + f->output_size, data, length);
    f->output_size += length;
    *sent = length;
    return preloader_l::net::TransportStatus::ok;
}

void close(void*) noexcept {}

unsigned count_token(const char* data, size_t length, const char* token) {
    const size_t token_length = std::strlen(token);
    unsigned count = 0;
    for (size_t i = 0; i + token_length <= length; ++i) {
        if (std::memcmp(data + i, token, token_length) == 0) ++count;
    }
    return count;
}

bool derive_key(uint8_t key[16]) {
    char reply[128]{};
    size_t written = 0;
    return preloader_l::codec::make_challenge_reply(
               reinterpret_cast<const uint8_t*>("a"), 1, "3", 1,
               key, reply, sizeof(reply), &written) == preloader_l::codec::SessionStatus::ok;
}

preloader_l::protocol::ExchangeStatus run(FakeTransport* fake,
                                          preloader_l::protocol::ExchangeState* state) {
    preloader_l::net::TransportEndpoint endpoint{fake, receive, send, close};
    size_t sent = 0;
    uint32_t frames = 0;
    return preloader_l::protocol::serve_protocol_session(endpoint, state, &sent, &frames);
}
}

int main() {
    const char handshake[] =
        "<LSX><Request id=\"61\"><ChallengeResponse response=\"x\" key=\"a\" version=\"3\"/></Request></LSX>";

    // 1) A handshake split across recv calls must be reconstructed.
    {
        FakeTransport f{};
        constexpr size_t split = 19;
        f.chunks[0] = handshake;
        f.sizes[0] = split;
        f.chunks[1] = handshake + split;
        f.sizes[1] = sizeof(handshake) - 1 - split;
        f.chunk_count = 2;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 1;
        if (!state.established || f.receive_index != 2) return 2;
        if (count_token(f.output, f.output_size, "ChallengeAccepted") != 1) return 3;
    }

    // 2) Back-to-back plaintext handshakes in one recv must be consumed separately.
    {
        char combined[2 * sizeof(handshake)]{};
        const size_t n = sizeof(handshake) - 1;
        std::memcpy(combined, handshake, n);
        std::memcpy(combined + n, handshake, n);
        FakeTransport f{};
        f.chunks[0] = combined;
        f.sizes[0] = 2 * n;
        f.chunk_count = 1;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 4;
        if (count_token(f.output, f.output_size, "ChallengeAccepted") != 2) return 5;
    }

    uint8_t key[16]{};
    if (!derive_key(key)) return 6;
    const char opaque_plain[] = "not-lsx";
    char encrypted[128]{};
    size_t encrypted_n = 0;
    if (preloader_l::codec::encode_response(
            key, 16, reinterpret_cast<const uint8_t*>(opaque_plain), sizeof(opaque_plain) - 1,
            encrypted, sizeof(encrypted), &encrypted_n) != preloader_l::codec::MessageCodecStatus::ok)
        return 7;

    // 3) An encrypted frame split at an odd hex boundary must not be decoded per recv chunk.
    {
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        constexpr size_t split = 7;
        f.chunks[1] = encrypted;
        f.sizes[1] = split;
        f.chunks[2] = encrypted + split;
        f.sizes[2] = encrypted_n - split;
        f.chunk_count = 3;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 8;
        if (f.receive_index != 3) return 9;
    }

    // 4) Two NUL-terminated encrypted frames coalesced in one recv must not be decoded as one blob.
    {
        char combined[256]{};
        std::memcpy(combined, encrypted, encrypted_n);
        std::memcpy(combined + encrypted_n, encrypted, encrypted_n);
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        f.chunks[1] = combined;
        f.sizes[1] = 2 * encrypted_n;
        f.chunk_count = 2;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 10;
        if (f.receive_index != 2) return 11;
    }

    // 5) Optional-NUL compatibility: a complete first encrypted frame without
    // its terminal NUL must still be split from a following NUL-terminated frame.
    {
        char combined[256]{};
        const size_t first_n = encrypted_n - 1;
        std::memcpy(combined, encrypted, first_n);
        std::memcpy(combined + first_n, encrypted, encrypted_n);
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        f.chunks[1] = combined;
        f.sizes[1] = first_n + encrypted_n;
        f.chunk_count = 2;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 12;
        if (f.receive_index != 2) return 13;
    }

    // HOTFIX16/F1570-001: a close marker inside a quoted attribute must not
    // terminate the plaintext handshake before the real outer close marker.
    {
        const char quoted_handshake[] =
            "<LSX><Request id=\"61\"><ChallengeResponse response=\"x</LSX>y\" key=\"a\" version=\"3\"/></Request></LSX>";
        FakeTransport f{};
        f.chunks[0] = quoted_handshake;
        f.sizes[0] = sizeof(quoted_handshake) - 1;
        f.chunk_count = 1;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 14;
        if (!state.established || count_token(f.output, f.output_size, "ChallengeAccepted") != 1) return 15;
    }

    static const char garbage[] = "not-a-frame";

    // HOTFIX16/F1566-001: a bounded run of malformed logical frames remains
    // session-local and does not disconnect the peer.
    {
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        for (unsigned i = 0; i < 32; ++i) {
            f.chunks[i + 1] = garbage;
            f.sizes[i + 1] = sizeof(garbage) - 1;
        }
        f.chunk_count = 33;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 16;
        if (f.receive_index != 33) return 17;
    }

    // One more consecutive malformed frame crosses the abuse bound and ends
    // the session without mutating protocol state.
    {
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        for (unsigned i = 0; i < 33; ++i) {
            f.chunks[i + 1] = garbage;
            f.sizes[i + 1] = sizeof(garbage) - 1;
        }
        f.chunk_count = 34;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::frame_error) return 18;
        if (!state.established || f.receive_index != 34) return 19;
    }

    // A valid handshake resets the consecutive-malformed budget.
    {
        FakeTransport f{};
        f.chunks[0] = handshake;
        f.sizes[0] = sizeof(handshake) - 1;
        unsigned slot = 1;
        for (unsigned i = 0; i < 20; ++i, ++slot) {
            f.chunks[slot] = garbage;
            f.sizes[slot] = sizeof(garbage) - 1;
        }
        f.chunks[slot] = handshake;
        f.sizes[slot] = sizeof(handshake) - 1;
        ++slot;
        for (unsigned i = 0; i < 20; ++i, ++slot) {
            f.chunks[slot] = garbage;
            f.sizes[slot] = sizeof(garbage) - 1;
        }
        f.chunk_count = slot;
        preloader_l::protocol::ExchangeState state{};
        if (run(&f, &state) != preloader_l::protocol::ExchangeStatus::peer_closed) return 20;
        if (count_token(f.output, f.output_size, "ChallengeAccepted") != 2) return 21;
    }

    // HOTFIX10/F1599-001: reset is the teardown zeroization boundary. Fill the
    // complete object with non-zero payload material, then require every byte
    // (key, decoded/plain/wire buffers, flags, sizes and padding) to be cleared.
    {
        preloader_l::protocol::ExchangeState state{};
        std::memset(&state, 0xA5, sizeof(state));
        preloader_l::protocol::reset_exchange(&state);
        const auto* bytes = reinterpret_cast<const unsigned char*>(&state);
        for (size_t i = 0; i < sizeof(state); ++i) if (bytes[i] != 0u) return 22;
    }

    // HF03/F321-S226-001: coalescing cannot change failure isolation. A
    // malformed prefix is one logical unit and the following handshake stays
    // buffered for the next scan.
    {
        char combined[256]{}; static constexpr char prefix[]="bad-prefix";
        const size_t pn=sizeof(prefix)-1u, hn=sizeof(handshake)-1u;
        std::memcpy(combined,prefix,pn); std::memcpy(combined+pn,handshake,hn);
        FakeTransport f{}; f.chunks[0]=combined; f.sizes[0]=pn+hn; f.chunk_count=1;
        preloader_l::net::TransportEndpoint ep{&f,receive,send,close};
        preloader_l::protocol::ExchangeState state{}; preloader_l::protocol::detail::WireFrameView view{};
        if(preloader_l::protocol::detail::receive_protocol_frame(ep,&state,&view)!=preloader_l::protocol::ExchangeStatus::ok) return 23;
        if(view.payload_length!=pn||view.consume_length!=pn) return 24;
        preloader_l::protocol::detail::consume_wire_frame(&state,view.consume_length);
        if(preloader_l::protocol::detail::receive_protocol_frame(ep,&state,&view)!=preloader_l::protocol::ExchangeStatus::ok) return 25;
        if(view.payload_length!=hn) return 26;
    }
    // Same isolation after establishment for a following encrypted candidate.
    {
        char combined[256]{}; static constexpr char prefix[]="bad-prefix";
        const size_t pn=sizeof(prefix)-1u;
        std::memcpy(combined,prefix,pn); std::memcpy(combined+pn,encrypted,encrypted_n);
        FakeTransport f{}; f.chunks[0]=combined; f.sizes[0]=pn+encrypted_n; f.chunk_count=1;
        preloader_l::net::TransportEndpoint ep{&f,receive,send,close};
        preloader_l::protocol::ExchangeState state{}; state.established=true; std::memcpy(state.session_key,key,16u);
        preloader_l::protocol::detail::WireFrameView view{};
        if(preloader_l::protocol::detail::receive_protocol_frame(ep,&state,&view)!=preloader_l::protocol::ExchangeStatus::ok) return 27;
        if(view.payload_length!=pn||view.consume_length!=pn) return 28;
        preloader_l::protocol::detail::consume_wire_frame(&state,view.consume_length);
        if(preloader_l::protocol::detail::receive_protocol_frame(ep,&state,&view)!=preloader_l::protocol::ExchangeStatus::ok) return 29;
        if(view.consume_length!=encrypted_n) return 30;
    }
    // HF03/F321-S241-001: peer lexical ciphertext errors share the bounded
    // malformed-frame budget instead of terminating as internal decode_error.
    {
        static const char invalid_tail[]={'0','1','2','3','4','5','6','7','8','9','z','\0'};
        FakeTransport f{}; f.chunks[0]=handshake; f.sizes[0]=sizeof(handshake)-1u;
        for(unsigned i=0;i<32u;++i){ f.chunks[i+1u]=invalid_tail; f.sizes[i+1u]=sizeof(invalid_tail); }
        f.chunk_count=33u; preloader_l::protocol::ExchangeState state{};
        if(run(&f,&state)!=preloader_l::protocol::ExchangeStatus::peer_closed) return 31;
    }
    {
        static const char invalid_tail[]={'0','1','2','3','4','5','6','7','8','9','z','\0'};
        FakeTransport f{}; f.chunks[0]=handshake; f.sizes[0]=sizeof(handshake)-1u;
        for(unsigned i=0;i<33u;++i){ f.chunks[i+1u]=invalid_tail; f.sizes[i+1u]=sizeof(invalid_tail); }
        f.chunk_count=34u; preloader_l::protocol::ExchangeState state{};
        if(run(&f,&state)!=preloader_l::protocol::ExchangeStatus::frame_error) return 32;
    }
    {
        static const char odd_hex[]={'0','1','2','3','4','5','6','7','8','9','0','\0'};
        FakeTransport f{}; f.chunks[0]=handshake; f.sizes[0]=sizeof(handshake)-1u;
        f.chunks[1]=odd_hex; f.sizes[1]=sizeof(odd_hex); f.chunk_count=2u;
        preloader_l::protocol::ExchangeState state{};
        if(run(&f,&state)!=preloader_l::protocol::ExchangeStatus::peer_closed) return 33;
    }
    return 0;
}
