// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/hash/fnv_hash.hpp"
#include "preloader_l/core/byte_semantics.hpp"

namespace preloader_l::hash {

uint64_t fnv1a64_signed_bytes(const uint8_t* bytes, size_t length) noexcept
{
    uint64_t value = fnv1a64_offset_basis;
    for (size_t i = 0; i < length; ++i) {
        value = (value ^ preloader_l::core::sign_extend_byte_u64(bytes[i])) * fnv1a64_prime;
    }
    return value;
}

uint32_t fnv1a32_signed_bytes(const uint8_t* bytes, size_t length,
                              uint32_t seed) noexcept
{
    for (size_t i = 0; i < length; ++i) {
        seed = (seed ^ preloader_l::core::sign_extend_byte_u32(bytes[i])) * fnv1a32_prime;
    }
    return seed;
}

} // namespace preloader_l::hash
