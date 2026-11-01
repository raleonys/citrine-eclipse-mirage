#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace codec {

enum class MessageCodecStatus : uint8_t {
    ok = 0,
    invalid_argument,
    key_too_short,
    capacity,
    invalid_hex,
    empty_message,
    overflow,
};

bool padded_size_16(size_t length, size_t* result) noexcept;

MessageCodecStatus encrypt_padded(const uint8_t* key, size_t key_length,
                                  const uint8_t* input, size_t length,
                                  uint8_t* output, size_t capacity,
                                  size_t* written) noexcept;

// Full 16-byte blocks are AES-decrypted; a final partial block is copied
// unchanged. Padding is deliberately NOT removed. input == output is supported.
MessageCodecStatus decrypt_raw(const uint8_t* key, size_t key_length,
                               const uint8_t* input, size_t length,
                               uint8_t* output, size_t capacity,
                               size_t* written) noexcept;

// Padded AES -> lowercase hex -> exactly one terminal NUL. *written includes NUL.
MessageCodecStatus encode_response(const uint8_t* key, size_t key_length,
                                   const uint8_t* input, size_t length,
                                   char* output, size_t capacity,
                                   size_t* written) noexcept;

// Accepts one optional terminal wire NUL, performs strict hex decoding, then
// raw AES decryption. Padding remains in the output.
MessageCodecStatus decode_payload(const uint8_t* key, size_t key_length,
                                  const char* input, size_t length,
                                  uint8_t* output, size_t capacity,
                                  size_t* written) noexcept;

} } // namespace preloader_l::codec
