#include "preloader_l/codec/aes128_cipher.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

using preloader_l::codec::AesStatus;
using preloader_l::codec::aes128_decrypt_block;
using preloader_l::codec::aes128_encrypt_block;

namespace {
bool same(const uint8_t* a, const uint8_t* b, size_t n) noexcept { return std::memcmp(a, b, n) == 0; }
}

int main()
{
    const uint8_t key[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    const uint8_t zero[16] = {};
    const uint8_t ref_zero[16] = {0xc6,0xa1,0x3b,0x37,0x87,0x8f,0x5b,0x82,0x6f,0x4f,0x81,0x62,0xa1,0xc8,0xd8,0x79};
    uint8_t out[16]{};
    if (aes128_encrypt_block(key, 16, zero, out) != AesStatus::ok || !same(out, ref_zero, 16)) return 1;

    const uint8_t seq[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    const uint8_t ref_seq[16] = {0x0a,0x94,0x0b,0xb5,0x41,0x6e,0xf0,0x45,0xf1,0xc3,0x94,0x58,0xc6,0x53,0xea,0x5a};
    if (aes128_encrypt_block(key, 16, seq, out) != AesStatus::ok || !same(out, ref_seq, 16)) return 2;

    // FIPS-197 Appendix C.1 independent reference vector.
    const uint8_t fips_plain[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    const uint8_t fips_cipher[16] = {0x69,0xc4,0xe0,0xd8,0x6a,0x7b,0x04,0x30,0xd8,0xcd,0xb7,0x80,0x70,0xb4,0xc5,0x5a};
    if (aes128_encrypt_block(key, 16, fips_plain, out) != AesStatus::ok || !same(out, fips_cipher, 16)) return 3;
    if (aes128_decrypt_block(key, 16, fips_cipher, out) != AesStatus::ok || !same(out, fips_plain, 16)) return 4;

    // In-place operation is required by later decode composition.
    std::memcpy(out, ref_seq, 16);
    if (aes128_decrypt_block(key, 16, out, out) != AesStatus::ok || !same(out, seq, 16)) return 5;

    uint8_t long_key[17];
    for (unsigned i = 0; i < 16; ++i) long_key[i] = key[i];
    long_key[16] = 0xff;
    if (aes128_encrypt_block(long_key, 17, zero, out) != AesStatus::ok || !same(out, ref_zero, 16)) return 6;

    if (aes128_encrypt_block(key, 15, zero, out) != AesStatus::key_too_short) return 7;
    if (aes128_encrypt_block(nullptr, 16, zero, out) != AesStatus::invalid_argument) return 8;
    if (aes128_encrypt_block(key, 16, nullptr, out) != AesStatus::invalid_argument) return 9;

    return 0;
}
