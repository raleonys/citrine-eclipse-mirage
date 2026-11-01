// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace redfield::hash {

inline constexpr uint64_t fnv1a64_offset_basis = UINT64_C(0xcbf29ce484222325);
inline constexpr uint64_t fnv1a64_prime = UINT64_C(0x100000001b3);
inline constexpr uint32_t fnv1a32_prime = UINT32_C(0x01000193);

// Model-compatible variant: each input byte is sign-extended before XOR.
// `bytes` may be null only when length == 0.
uint64_t fnv1a64_signed_bytes(const uint8_t* bytes, size_t length) noexcept;
uint32_t fnv1a32_signed_bytes(const uint8_t* bytes, size_t length,
                              uint32_t seed) noexcept;

} // namespace redfield::hash
