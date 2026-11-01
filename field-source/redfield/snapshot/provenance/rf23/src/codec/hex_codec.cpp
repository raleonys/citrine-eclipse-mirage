// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/codec/hex_codec.hpp"
#include "redfield/core/checked_math.hpp"

namespace redfield::codec {
namespace {
constexpr char digits[] = "0123456789abcdef";

constexpr int nibble(unsigned char value) noexcept
{
    if (value >= static_cast<unsigned char>('0') && value <= static_cast<unsigned char>('9')) {
        return value - static_cast<unsigned char>('0');
    }
    if (value >= static_cast<unsigned char>('a') && value <= static_cast<unsigned char>('f')) {
        return value - static_cast<unsigned char>('a') + 10;
    }
    if (value >= static_cast<unsigned char>('A') && value <= static_cast<unsigned char>('F')) {
        return value - static_cast<unsigned char>('A') + 10;
    }
    return -1;
}
} // namespace

HexStatus hex_encode_lower(const uint8_t* input, size_t length,
                           char* output, size_t capacity,
                           size_t* written) noexcept
{
    if (written) *written = 0;
    if (!input && length != 0) return HexStatus::invalid_argument;

    size_t required = 0;
    if (!redfield::core::checked_mul_size(length, 2, required)) return HexStatus::overflow;
    if (written) *written = required;
    if (capacity < required) return HexStatus::capacity;
    if (required == 0) return HexStatus::ok;
    if (!output) return HexStatus::invalid_argument;

    for (size_t i = 0; i < length; ++i) {
        const uint8_t value = input[i];
        output[2 * i] = digits[value >> 4];
        output[2 * i + 1] = digits[value & 0x0f];
    }
    return HexStatus::ok;
}

HexStatus hex_decode_strict(const char* input, size_t length,
                            uint8_t* output, size_t capacity,
                            size_t* written) noexcept
{
    if (written) *written = 0;
    if (!input && length != 0) return HexStatus::invalid_argument;
    if ((length & 1u) != 0) return HexStatus::invalid_length;

    // Full validation precedes capacity/output checks so malformed input never
    // partially mutates the destination buffer.
    for (size_t i = 0; i < length; ++i) {
        if (nibble(static_cast<unsigned char>(input[i])) < 0) return HexStatus::invalid_digit;
    }

    const size_t required = length / 2;
    if (written) *written = required;
    if (capacity < required) return HexStatus::capacity;
    if (required == 0) return HexStatus::ok;
    if (!output) return HexStatus::invalid_argument;

    for (size_t i = 0; i < length; i += 2) {
        const int high = nibble(static_cast<unsigned char>(input[i]));
        const int low = nibble(static_cast<unsigned char>(input[i + 1]));
        output[i / 2] = static_cast<uint8_t>((high << 4) | low);
    }
    return HexStatus::ok;
}

} // namespace redfield::codec
