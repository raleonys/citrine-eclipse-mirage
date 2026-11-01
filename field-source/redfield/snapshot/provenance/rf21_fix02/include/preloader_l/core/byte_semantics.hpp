#pragma once

#include <stdint.h>

namespace preloader_l::core {

// The model FNV loops sign-extend each byte before XOR. Express that
// behavior without relying on implementation-defined uint8_t -> int8_t casts.
constexpr uint64_t sign_extend_byte_u64(uint8_t value) noexcept
{
    const int64_t signed_value =
        value < 0x80u ? static_cast<int64_t>(value)
                      : static_cast<int64_t>(value) - 0x100;
    return static_cast<uint64_t>(signed_value);
}

constexpr uint32_t sign_extend_byte_u32(uint8_t value) noexcept
{
    const int32_t signed_value =
        value < 0x80u ? static_cast<int32_t>(value)
                      : static_cast<int32_t>(value) - 0x100;
    return static_cast<uint32_t>(signed_value);
}

} // namespace preloader_l::core
