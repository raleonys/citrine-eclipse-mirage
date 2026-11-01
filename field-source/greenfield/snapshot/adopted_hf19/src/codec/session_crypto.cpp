#include "preloader_l/codec/session_crypto.hpp"
#include "preloader_l/codec/aes128_cipher.hpp"
#include "preloader_l/codec/hex_codec.hpp"
#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/core/checked_math.hpp"

namespace preloader_l { namespace codec {
namespace {
int behavioral_atoi_span(const char* text, size_t length) noexcept {
    if (!text || length == 0) return 0;
    size_t i = 0;
    while (i < length && (text[i] == ' ' || text[i] == '\t' || text[i] == '\r' || text[i] == '\n' || text[i] == '\v' || text[i] == '\f')) ++i;
    bool negative = false;
    if (i < length && (text[i] == '+' || text[i] == '-')) { negative = text[i] == '-'; ++i; }

    // Donor1's exact decimal helper is a signed 32-bit strtol-family parser:
    // it consumes the decimal prefix and saturates overflow to INT32_MIN/MAX.
    // Express that contract with defined arithmetic; in particular never negate
    // INT32_MIN in C++ (F09).
    const uint32_t limit = negative ? UINT32_C(0x80000000) : UINT32_C(0x7fffffff);
    uint32_t value = 0;
    bool any = false;
    bool overflow = false;
    for (; i < length; ++i) {
        const unsigned char c = static_cast<unsigned char>(text[i]);
        if (c < '0' || c > '9') break;
        any = true;
        const uint32_t digit = static_cast<uint32_t>(c - '0');
        if (!overflow) {
            if (value > (limit - digit) / 10u) {
                overflow = true;
                value = limit;
            } else {
                value = value * 10u + digit;
            }
        }
    }
    if (!any) return 0;
    if (overflow) return negative ? (-INT32_C(2147483647) - 1) : INT32_C(2147483647);
    if (!negative) return static_cast<int32_t>(value);
    if (value == UINT32_C(0x80000000)) return -INT32_C(2147483647) - 1;
    return -static_cast<int32_t>(value);
}

SessionStatus derive_dynamic_session_key(const uint8_t* key, size_t length, uint8_t output[16]) noexcept {
    if (!output || (!key && length != 0)) return SessionStatus::invalid_argument;
    const uint8_t b0 = length > 0 ? key[0] : 0u;
    const uint8_t b1 = length > 1 ? key[1] : 0u;
    const uint32_t hi = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(b0)));
    const uint32_t lo = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int8_t>(b1)));
    uint32_t state = ((hi << 8) | lo) + UINT32_C(0x3d);
    for (unsigned i = 0; i < 16; ++i) {
        state = state * UINT32_C(0x343fd) + UINT32_C(0x269ec3);
        output[i] = static_cast<uint8_t>(state >> 16);
    }
    return SessionStatus::ok;
}

}

SessionStatus derive_session_key(uint16_t seed, uint8_t output[16]) noexcept
{
    if (!output) return SessionStatus::invalid_argument;
    uint32_t state = static_cast<uint32_t>(seed) + UINT32_C(0x3d);
    for (unsigned i = 0; i < 16; ++i) {
        if (seed == 0) {
            output[i] = static_cast<uint8_t>(i);
        } else {
            state = state * UINT32_C(0x343fd) + UINT32_C(0x269ec3);
            output[i] = static_cast<uint8_t>(state >> 16);
        }
    }
    return SessionStatus::ok;
}

SessionStatus make_challenge_reply(const uint8_t* challenge, size_t length,
                                   const char* version, size_t version_length,
                                   uint8_t session_key[16],
                                   char* reply, size_t capacity,
                                   size_t* written) noexcept
{
    if (written) *written = 0;
    if ((!challenge && length != 0) || (!version && version_length != 0) || !session_key) {
        return SessionStatus::invalid_argument;
    }

    size_t padded = 0;
    if (!padded_size_16(length, &padded)) return SessionStatus::overflow;
    size_t hex_chars = 0;
    if (!preloader_l::core::checked_mul_size(padded, 2, hex_chars)) return SessionStatus::overflow;
    size_t required = 0;
    if (!preloader_l::core::checked_add_size(hex_chars, 1, required)) return SessionStatus::overflow;
    if (written) *written = hex_chars;
    if (capacity < required) return SessionStatus::capacity;
    if (!reply) return SessionStatus::invalid_argument;

    uint8_t fixed_key[16];
    if (derive_session_key(0, fixed_key) != SessionStatus::ok) return SessionStatus::invalid_argument;
    Aes128Schedule schedule{};
    if (aes128_expand_key(fixed_key, 16, &schedule) != AesStatus::ok) return SessionStatus::invalid_argument;

    const uint8_t padding = static_cast<uint8_t>(padded - length);
    for (size_t offset = 0; offset < padded; offset += 16) {
        uint8_t plain[16];
        uint8_t encrypted[16];
        for (unsigned i = 0; i < 16; ++i) {
            const size_t pos = offset + i;
            plain[i] = pos < length ? challenge[pos] : padding;
        }
        aes128_encrypt_block_expanded(schedule, plain, encrypted);
        size_t block_written = 0;
        if (hex_encode_lower(encrypted, 16, reply + 2 * offset, capacity - 2 * offset, &block_written) != HexStatus::ok || block_written != 32) {
            return SessionStatus::invalid_argument;
        }
    }
    reply[hex_chars] = '\0';

    // Preserve HF14 semantics everywhere except the proven live-consumer path:
    // exact one-byte version "3" derives its seed from ChallengeAccepted/@response[0:2].
    // <=2 remains the fixed 00..0F key; missing and non-exact-3 >2 inputs retain
    // HF14's behavioral-derived incoming ChallengeResponse/@key[0:2] policy.
    if (version && behavioral_atoi_span(version, version_length) <= 2)
        return derive_session_key(0, session_key);
    if (version && version_length == 1 && version[0] == '3') {
        const uint16_t seed = static_cast<uint16_t>(
            (static_cast<uint16_t>(static_cast<uint8_t>(reply[0])) << 8) |
             static_cast<uint8_t>(reply[1]));
        return derive_session_key(seed, session_key);
    }
    return derive_dynamic_session_key(challenge, length, session_key);
}

} } // namespace preloader_l::codec
