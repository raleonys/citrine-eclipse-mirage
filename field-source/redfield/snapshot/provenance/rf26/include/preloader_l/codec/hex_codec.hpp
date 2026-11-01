// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l::codec {

enum class HexStatus : uint8_t {
    ok = 0,
    invalid_argument,
    invalid_length,
    invalid_digit,
    capacity,
    overflow,
};

// Lowercase ASCII hex, no NUL terminator. *written is the exact number of
// characters required/written. No output bytes are written on error.
HexStatus hex_encode_lower(const uint8_t* input, size_t length,
                           char* output, size_t capacity,
                           size_t* written) noexcept;

// Strict even-length ASCII hex. Both upper- and lowercase digits are
// accepted. Malformed input is rejected before any output is written.
HexStatus hex_decode_strict(const char* input, size_t length,
                            uint8_t* output, size_t capacity,
                            size_t* written) noexcept;

} // namespace preloader_l::codec
