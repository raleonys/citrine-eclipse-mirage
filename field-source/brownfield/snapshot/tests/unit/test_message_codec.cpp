#include "preloader_l/codec/message_codec.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace preloader_l::codec;

int main()
{
    const uint8_t key[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    size_t n = 0;
    if (!padded_size_16(0, &n) || n != 16) return 1;
    if (!padded_size_16(15, &n) || n != 16) return 2;
    if (!padded_size_16(16, &n) || n != 32) return 3;
    if (!padded_size_16(17, &n) || n != 32) return 4;
    if (padded_size_16(static_cast<size_t>(-1), &n)) return 5;

    const uint8_t hello[] = {'h','e','l','l','o'};
    const uint8_t expected_raw[16] = {0x5d,0x87,0x49,0xe2,0xaf,0x75,0x31,0xb2,0xbf,0x66,0x61,0xe9,0xe5,0xda,0xf0,0x12};
    uint8_t raw[64]{};
    size_t written = 0;
    if (encrypt_padded(key, 16, hello, sizeof hello, raw, sizeof raw, &written) != MessageCodecStatus::ok ||
        written != 16 || std::memcmp(raw, expected_raw, 16) != 0) return 6;

    char wire[128]{};
    constexpr char expected_wire[] = "5d8749e2af7531b2bf6661e9e5daf012";
    if (encode_response(key, 16, hello, sizeof hello, wire, sizeof wire, &written) != MessageCodecStatus::ok ||
        written != 33 || std::memcmp(wire, expected_wire, 32) != 0 || wire[32] != '\0') return 7;

    std::array<uint8_t, 64> decoded{};
    if (decode_payload(key, 16, wire, written, decoded.data(), decoded.size(), &written) != MessageCodecStatus::ok || written != 16) return 8;
    if (std::memcmp(decoded.data(), hello, sizeof hello) != 0) return 9;
    for (size_t i = sizeof hello; i < 16; ++i) if (decoded[i] != 11) return 10;

    // Full PKCS#7-style block is added for aligned plaintext.
    const uint8_t aligned[] = "0123456789abcdef";
    const uint8_t expected_aligned[32] = {
        0x28,0x15,0x67,0xab,0x2f,0x4c,0xf0,0xd7,0x3d,0x31,0x98,0x22,0x5b,0x8b,0x83,0x93,
        0x95,0x4f,0x64,0xf2,0xe4,0xe8,0x6e,0x9e,0xee,0x82,0xd2,0x02,0x16,0x68,0x48,0x99};
    if (encrypt_padded(key, 16, aligned, 16, raw, sizeof raw, &written) != MessageCodecStatus::ok ||
        written != 32 || std::memcmp(raw, expected_aligned, 32) != 0) return 11;

    // Model decrypt semantics: a partial final block is copied unchanged.
    uint8_t odd_cipher[17];
    std::memcpy(odd_cipher, expected_raw, 16); odd_cipher[16] = 0x5a;
    if (decrypt_raw(key, 16, odd_cipher, 17, decoded.data(), decoded.size(), &written) != MessageCodecStatus::ok || written != 17) return 12;
    if (std::memcmp(decoded.data(), hello, sizeof hello) != 0 || decoded[16] != 0x5a) return 13;

    raw[0] = 0x77;
    written = 0;
    if (encrypt_padded(key, 16, hello, sizeof hello, raw, 1, &written) != MessageCodecStatus::capacity || written != 16 || raw[0] != 0x77) return 14;
    if (encode_response(key, 16, nullptr, 0, wire, sizeof wire, &written) != MessageCodecStatus::empty_message) return 15;
    if (decode_payload(key, 16, "gg", 2, decoded.data(), decoded.size(), &written) != MessageCodecStatus::invalid_hex) return 16;
    if (encrypt_padded(key, 15, hello, sizeof hello, raw, sizeof raw, &written) != MessageCodecStatus::key_too_short) return 17;

    return 0;
}
