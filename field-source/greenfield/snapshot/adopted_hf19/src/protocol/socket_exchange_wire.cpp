#include "socket_exchange_internal.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/xml/lsx_framing.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace protocol { namespace detail {
namespace {
bool is_hex(char c) noexcept {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool lsx_prefix_candidate(const char* bytes, size_t length) noexcept {
    if (!bytes) return false;
    constexpr char open[] = "<LSX>";
    const size_t compare = length < sizeof(open) - 1 ? length : sizeof(open) - 1;
    for (size_t i = 0; i < compare; ++i) {
        if (bytes[i] != open[i]) return false;
    }
    return true;
}

size_t first_zero(const char* bytes, size_t length) noexcept {
    if (!bytes) return static_cast<size_t>(-1);
    for (size_t i = 0; i < length; ++i) if (bytes[i] == '\0') return i;
    return static_cast<size_t>(-1);
}

size_t leading_hex_span(const char* bytes, size_t length) noexcept {
    size_t i = 0;
    while (i < length && is_hex(bytes[i])) ++i;
    return i;
}

size_t next_logical_candidate(const char* bytes,size_t length,bool established) noexcept {
    if(!bytes) return static_cast<size_t>(-1);
    for(size_t i=1u;i<length;++i){
        if(bytes[i]=='<' && lsx_prefix_candidate(bytes+i,length-i)) return i;
        if(established && is_hex(bytes[i]) && !is_hex(bytes[i-1])) {
            size_t j=i; while(j<length && is_hex(bytes[j])) ++j;
            const size_t run=j-i;
            // The established wire contract uses ten leading hex digits as
            // the ciphertext discriminator. Avoid retaining arbitrary one-byte
            // hex suffixes from malformed text as phantom future frames.
            if(run>=10u) return i;
            i=j?j-1u:i;
        }
    }
    return static_cast<size_t>(-1);
}

size_t first_complete_encrypted_prefix(ExchangeState* s) noexcept {
    if (!s || !s->established || s->wire_size < 32) return 0;
    const size_t hex_span = leading_hex_span(s->wire_response, s->wire_size);
    const size_t block_hex = (hex_span / 32u) * 32u;
    if (block_hex < 32u) return 0;

    size_t decoded_n = 0;
    if (preloader_l::codec::decode_payload(s->session_key, 16, s->wire_response, block_hex,
                                           s->decoded, sizeof(s->decoded), &decoded_n) !=
        preloader_l::codec::MessageCodecStatus::ok)
        return 0;
    if (!starts_lsx(reinterpret_cast<const char*>(s->decoded), decoded_n)) return 0;

    const size_t close = lsx_frame_end(reinterpret_cast<const char*>(s->decoded), decoded_n);
    if (close == 0) return 0;
    const size_t padding = 16u - (close & 15u);
    if (padding == 0 || padding > 16u || close > decoded_n || padding > decoded_n - close)
        return 0;
    const size_t padded = close + padding;
    for (size_t i = close; i < padded; ++i) {
        if (s->decoded[i] != static_cast<uint8_t>(padding)) return 0;
    }
    return padded * 2u;
}

}  // namespace

// Buffer-only scan over already-received bytes, shared by the existing
// synchronous transport and future Asio IOCP async ingress. Scanner may use
// ExchangeState::decoded as scratch. Caller owns append/timeout/consume;
// no socket API or transport ownership is used here.
bool scan_available_wire_frame(ExchangeState* s, WireFrameView* view) noexcept {
    if (view) *view = WireFrameView{0, 0};
    if (!s || !view || s->wire_size == 0) return false;

    const char* bytes = s->wire_response;
    if (lsx_prefix_candidate(bytes, s->wire_size)) {
        if (s->wire_size < 5u) return false;
        if (starts_lsx(bytes, s->wire_size)) {
            const size_t close = lsx_frame_end(bytes, s->wire_size);
            if (close == 0) {
                const size_t zero = first_zero(bytes, s->wire_size);
                if (zero == static_cast<size_t>(-1)) return false;
                *view = WireFrameView{zero, zero + 1u};
                return true;
            }
            size_t consume = close;
            if (consume < s->wire_size && bytes[consume] == '\0') ++consume;
            *view = WireFrameView{close, consume};
            return true;
        }
    }

    // Failure isolation is a logical-frame property, not a recv-chunk property.
    // Consume only the proven malformed prefix when a following LSX/ciphertext
    // candidate is already buffered (or starts at the terminal suffix).
    const size_t next_candidate=next_logical_candidate(bytes,s->wire_size,s->established);
    if(next_candidate!=static_cast<size_t>(-1)){
        *view=WireFrameView{next_candidate,next_candidate};
        return true;
    }

    // Before a key exists, a fully proven non-LSX buffer is one malformed unit.
    if (!s->established) {
        *view = WireFrameView{s->wire_size, s->wire_size};
        return true;
    }

    const size_t zero = first_zero(bytes, s->wire_size);
    const size_t encrypted_prefix = first_complete_encrypted_prefix(s);
    if (encrypted_prefix != 0 &&
        (zero == static_cast<size_t>(-1) || encrypted_prefix < zero)) {
        *view = WireFrameView{encrypted_prefix, encrypted_prefix};
        return true;
    }
    if (zero != static_cast<size_t>(-1)) {
        *view = WireFrameView{zero + 1u, zero + 1u};
        return true;
    }
    if (encrypted_prefix != 0) {
        *view = WireFrameView{encrypted_prefix, encrypted_prefix};
        return true;
    }

    // A fragment that cannot possibly become either an LSX handshake or a hex
    // ciphertext retains compatibility classification instead of poisoning later recv
    // data. Short all-hex prefixes remain eligible for reassembly.
    const size_t hex_span = leading_hex_span(bytes, s->wire_size);
    if (hex_span < s->wire_size || (s->wire_size >= 10u && !first_ten_hex(bytes, s->wire_size))) {
        *view = WireFrameView{s->wire_size, s->wire_size};
        return true;
    }
    if (s->wire_size >= 32u && (s->wire_size % 32u) == 0u) {
        size_t decoded_n = 0;
        if (preloader_l::codec::decode_payload(s->session_key, 16, bytes, s->wire_size,
                                               s->decoded, sizeof(s->decoded), &decoded_n) ==
                preloader_l::codec::MessageCodecStatus::ok &&
            !starts_lsx(reinterpret_cast<const char*>(s->decoded), decoded_n)) {
            *view = WireFrameView{s->wire_size, s->wire_size};
            return true;
        }
    }
    return false;
}

bool starts_lsx(const char* bytes, size_t length) noexcept {
    return bytes && length >= 5 && bytes[0] == '<' && bytes[1] == 'L' &&
           bytes[2] == 'S' && bytes[3] == 'X' && bytes[4] == '>';
}

bool first_ten_hex(const char* bytes, size_t length) noexcept {
    if (!bytes || length < 10) return false;
    for (size_t i = 0; i < 10; ++i) if (!is_hex(bytes[i])) return false;
    return true;
}

size_t lsx_frame_end(const char* xml, size_t length) noexcept {
    const auto scan = preloader_l::xml::scan_lsx_frame(xml, length);
    return scan.status == preloader_l::xml::FrameStatus::frame_ready ? scan.frame_length : 0u;
}

ExchangeStatus send_all(preloader_l::net::TransportEndpoint ep,
                        const uint8_t* data,
                        size_t n,
                        size_t* total) noexcept {
    if (!total || (!data && n) || !ep.send) return ExchangeStatus::invalid_argument;
    *total = 0;

    constexpr uint32_t kDefaultSendIdleTimeoutMs = 5000u;
    constexpr size_t kClocklessWouldBlockLimit = 1024u;
    const uint32_t idle_timeout = ep.send_idle_timeout_ms == 0u
        ? kDefaultSendIdleTimeoutMs
        : ep.send_idle_timeout_ms;
    const bool infinite = idle_timeout == 0xffffffffu;
    uint64_t last_progress = ep.monotonic_milliseconds
        ? ep.monotonic_milliseconds(ep.context)
        : 0u;
    size_t clockless_would_block = 0u;

    while (*total < n) {
        size_t sent = 0;
        const auto status = ep.send(ep.context, data + *total, n - *total, &sent);
        if (status == preloader_l::net::TransportStatus::would_block) {
            if (!infinite) {
                if (ep.monotonic_milliseconds) {
                    const uint64_t now = ep.monotonic_milliseconds(ep.context);
                    if (now - last_progress >= static_cast<uint64_t>(idle_timeout))
                        return ExchangeStatus::send_timeout;
                } else if (++clockless_would_block >= kClocklessWouldBlockLimit) {
                    return ExchangeStatus::send_timeout;
                }
            }
            continue;
        }
        if (status != preloader_l::net::TransportStatus::ok || sent == 0 || sent > n - *total)
            return ExchangeStatus::send_error;
        *total += sent;
        clockless_would_block = 0u;
        if (ep.monotonic_milliseconds)
            last_progress = ep.monotonic_milliseconds(ep.context);
    }
    return ExchangeStatus::ok;
}

void consume_wire_frame(ExchangeState* s, size_t length) noexcept {
    if (!s || length == 0 || length > s->wire_size) return;
    const size_t remaining = s->wire_size - length;
    for (size_t i = 0; i < remaining; ++i) s->wire_response[i] = s->wire_response[length + i];
    s->wire_size = remaining;
}

ExchangeStatus receive_protocol_frame(preloader_l::net::TransportEndpoint ep,
                                      ExchangeState* s,
                                      WireFrameView* view) noexcept {
    if (!s || !view || !ep.receive) return ExchangeStatus::invalid_argument;

    constexpr uint32_t kDefaultReceiveIdleTimeoutMs = 30000u;
    constexpr uint32_t kDefaultReceiveFrameTimeoutMs = 10000u;
    constexpr size_t kClocklessWouldBlockLimit = 1024u;
    const uint32_t idle_timeout = ep.receive_idle_timeout_ms == 0u
        ? kDefaultReceiveIdleTimeoutMs
        : ep.receive_idle_timeout_ms;
    const uint32_t frame_timeout = ep.receive_frame_timeout_ms == 0u
        ? kDefaultReceiveFrameTimeoutMs
        : ep.receive_frame_timeout_ms;
    const bool idle_infinite = idle_timeout == 0xffffffffu;
    const bool frame_infinite = frame_timeout == 0xffffffffu;
    uint64_t last_progress = ep.monotonic_milliseconds
        ? ep.monotonic_milliseconds(ep.context)
        : 0u;
    bool frame_started = s->wire_size != 0u;
    uint64_t frame_started_at = frame_started ? last_progress : 0u;
    size_t clockless_would_block = 0u;

    for (;;) {
        if (scan_available_wire_frame(s, view)) return ExchangeStatus::ok;
        if (s->wire_size >= sizeof(s->wire_response)) return ExchangeStatus::frame_error;

        size_t got = 0;
        preloader_l::net::TransportStatus r = preloader_l::net::TransportStatus::would_block;
        while (r == preloader_l::net::TransportStatus::would_block) {
            const size_t available = sizeof(s->wire_response) - s->wire_size;
            r = ep.receive(ep.context, reinterpret_cast<uint8_t*>(s->wire_response + s->wire_size),
                           available, &got);
            if (r == preloader_l::net::TransportStatus::would_block) {
                if (ep.monotonic_milliseconds) {
                    const uint64_t now = ep.monotonic_milliseconds(ep.context);
                    if (!idle_infinite && now - last_progress >= static_cast<uint64_t>(idle_timeout))
                        return ExchangeStatus::receive_timeout;
                    if (frame_started && !frame_infinite &&
                        now - frame_started_at >= static_cast<uint64_t>(frame_timeout))
                        return ExchangeStatus::receive_timeout;
                } else if (!idle_infinite && ++clockless_would_block >= kClocklessWouldBlockLimit) {
                    return ExchangeStatus::receive_timeout;
                }
            }
        }
        if (r == preloader_l::net::TransportStatus::closed)
            return s->wire_size == 0 ? ExchangeStatus::peer_closed : ExchangeStatus::frame_error;
        const size_t available = sizeof(s->wire_response) - s->wire_size;
        if (r != preloader_l::net::TransportStatus::ok || got == 0 || got > available)
            return ExchangeStatus::receive_error;
        s->wire_size += got;
        clockless_would_block = 0u;
        if (ep.monotonic_milliseconds) {
            const uint64_t now = ep.monotonic_milliseconds(ep.context);
            if (!frame_started) { frame_started = true; frame_started_at = now; }
            last_progress = now;
            if (!frame_infinite && now - frame_started_at >= static_cast<uint64_t>(frame_timeout) &&
                !scan_available_wire_frame(s, view))
                return ExchangeStatus::receive_timeout;
        } else if (!frame_started) {
            frame_started = true;
        }
    }
}

} } }
