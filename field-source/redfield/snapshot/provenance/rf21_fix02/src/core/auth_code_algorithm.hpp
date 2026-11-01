// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
#include <cstddef>
namespace redfield { namespace {
uint64_t mix64(uint64_t x) noexcept {
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31);
}

void generate_auth_code(char out[9],uint64_t seed) noexcept {
    static constexpr char alphabet[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    uint64_t state = seed;
    for (size_t i = 0; i < 8u; ++i) {
        state = mix64(state + UINT64_C(0x9e3779b97f4a7c15) + static_cast<uint64_t>(i));
        out[i] = alphabet[state % 62u];
    }
    out[8] = '\0';
}
} }
