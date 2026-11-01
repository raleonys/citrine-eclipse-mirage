// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/codec/hex_codec.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

using preloader_l::codec::HexStatus;
using preloader_l::codec::hex_decode_strict;
using preloader_l::codec::hex_encode_lower;

namespace {
bool all_equal(const std::uint8_t* p, std::size_t n, std::uint8_t value) noexcept
{
    for (std::size_t i = 0; i < n; ++i) if (p[i] != value) return false;
    return true;
}
}

int main()
{
    std::array<std::uint8_t, 512> decoded{};
    std::array<char, 512> encoded{};
    std::size_t written = 999;

    // Exact valid-domain cases used by the supplied original-instruction hex
    // differential harness.
    if (hex_decode_strict(nullptr, 0, nullptr, 0, &written) != HexStatus::ok || written != 0) return 1;

    if (hex_decode_strict("00", 2, decoded.data(), decoded.size(), &written) != HexStatus::ok ||
        written != 1 || decoded[0] != 0x00) return 2;

    if (hex_decode_strict("aA", 2, decoded.data(), decoded.size(), &written) != HexStatus::ok ||
        written != 1 || decoded[0] != 0xaa) return 3;

    constexpr char mixed[] = "ABCDEF0123456789";
    constexpr std::uint8_t mixed_expected[] = {0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89};
    if (hex_decode_strict(mixed, sizeof(mixed) - 1, decoded.data(), decoded.size(), &written) != HexStatus::ok ||
        written != sizeof(mixed_expected) || std::memcmp(decoded.data(), mixed_expected, sizeof(mixed_expected)) != 0) return 4;

    std::array<std::uint8_t, 256> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) bytes[i] = static_cast<std::uint8_t>(i);
    if (hex_encode_lower(bytes.data(), bytes.size(), encoded.data(), encoded.size(), &written) != HexStatus::ok || written != 512) return 5;
    if (encoded[0] != '0' || encoded[1] != '0' || encoded[510] != 'f' || encoded[511] != 'f') return 6;
    if (hex_decode_strict(encoded.data(), written, decoded.data(), decoded.size(), &written) != HexStatus::ok ||
        written != bytes.size() || std::memcmp(decoded.data(), bytes.data(), bytes.size()) != 0) return 7;

    // Deliberate clean-room policy carried forward from the curio v2 work:
    // malformed hex is rejected before writing anything.
    constexpr char bad1[] = "gg";
    constexpr char bad2[] = "00gg";
    constexpr char bad3[] = "1g";
    constexpr char bad4[] = "001";
    const char bad5[] = {'0', '0', '\0'};
    struct Bad { const char* p; std::size_t n; HexStatus status; };
    const Bad bad[] = {
        {bad1, 2, HexStatus::invalid_digit},
        {bad2, 4, HexStatus::invalid_digit},
        {bad3, 2, HexStatus::invalid_digit},
        {bad4, 3, HexStatus::invalid_length},
        {bad5, 3, HexStatus::invalid_length},
    };
    for (const auto& test : bad) {
        decoded.fill(0x77);
        written = 123;
        if (hex_decode_strict(test.p, test.n, decoded.data(), decoded.size(), &written) != test.status) return 10;
        if (written != 0 || !all_equal(decoded.data(), decoded.size(), 0x77)) return 11;
    }

    decoded.fill(0x77);
    written = 0;
    if (hex_decode_strict("0011", 4, decoded.data(), 1, &written) != HexStatus::capacity || written != 2) return 12;
    if (!all_equal(decoded.data(), decoded.size(), 0x77)) return 13;

    encoded.fill('x');
    written = 0;
    const std::uint8_t one[] = {0xab};
    if (hex_encode_lower(one, 1, encoded.data(), 1, &written) != HexStatus::capacity || written != 2) return 14;
    if (encoded[0] != 'x') return 15;

    // Overflow is detected before reading the caller's input pointer.
    written = 5;
    if (hex_encode_lower(one, static_cast<std::size_t>(-1), encoded.data(), encoded.size(), &written) != HexStatus::overflow || written != 0) return 16;

    return 0;
}
