// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/codec/crypto.hpp"

namespace preloader_l::token::constants {
// SHA-256-bound original .rdata constants, not a user's token or configuration.
inline constexpr crypto::Key key_084a50{
    0x41,0x32,0x72,0x2d,0xd0,0x82,0xef,0xb0,
    0xdc,0x64,0x57,0xc5,0x76,0x68,0xca,0x09
};
inline constexpr crypto::Key iv_084a60{};
inline constexpr std::size_t encrypted_offset = 0x41;
} // namespace preloader_l::token::constants
