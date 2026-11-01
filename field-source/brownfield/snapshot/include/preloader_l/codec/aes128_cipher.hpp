#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace codec {

enum class AesStatus : uint8_t {
    ok = 0,
    invalid_argument,
    key_too_short,
};

struct Aes128Schedule {
    uint8_t bytes[176];
};

AesStatus aes128_expand_key(const uint8_t* key, size_t key_length,
                            Aes128Schedule* schedule) noexcept;
AesStatus aes128_encrypt_block(const uint8_t* key, size_t key_length,
                               const uint8_t input[16], uint8_t output[16]) noexcept;
AesStatus aes128_decrypt_block(const uint8_t* key, size_t key_length,
                               const uint8_t input[16], uint8_t output[16]) noexcept;

// Internal building blocks for later message-codec stages. Input and output may
// point to the same 16-byte block.
void aes128_encrypt_block_expanded(const Aes128Schedule& schedule,
                                   const uint8_t input[16], uint8_t output[16]) noexcept;
void aes128_decrypt_block_expanded(const Aes128Schedule& schedule,
                                   const uint8_t input[16], uint8_t output[16]) noexcept;

} } // namespace preloader_l::codec
