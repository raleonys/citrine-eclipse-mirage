#include "preloader_l/codec/message_codec.hpp"
#include "preloader_l/codec/aes128_cipher.hpp"
#include "preloader_l/codec/hex_codec.hpp"
#include "preloader_l/core/checked_math.hpp"

namespace preloader_l { namespace codec {
namespace {
MessageCodecStatus check_key(const uint8_t* key, size_t key_length) noexcept
{
    if (key_length < 16) return MessageCodecStatus::key_too_short;
    return key ? MessageCodecStatus::ok : MessageCodecStatus::invalid_argument;
}

MessageCodecStatus map_hex(HexStatus status) noexcept
{
    switch (status) {
    case HexStatus::ok: return MessageCodecStatus::ok;
    case HexStatus::capacity: return MessageCodecStatus::capacity;
    case HexStatus::overflow: return MessageCodecStatus::overflow;
    case HexStatus::invalid_length:
    case HexStatus::invalid_digit: return MessageCodecStatus::invalid_hex;
    case HexStatus::invalid_argument: return MessageCodecStatus::invalid_argument;
    }
    return MessageCodecStatus::invalid_argument;
}
} // namespace

bool padded_size_16(size_t length, size_t* result) noexcept
{
    if (!result) return false;
    const size_t padding = 16u - (length & 15u);
    return preloader_l::core::checked_add_size(length, padding, *result);
}

MessageCodecStatus encrypt_padded(const uint8_t* key, size_t key_length,
                                  const uint8_t* input, size_t length,
                                  uint8_t* output, size_t capacity,
                                  size_t* written) noexcept
{
    if (written) *written = 0;
    const MessageCodecStatus key_status = check_key(key, key_length);
    if (key_status != MessageCodecStatus::ok) return key_status;
    if (!input && length != 0) return MessageCodecStatus::invalid_argument;

    size_t required = 0;
    if (!padded_size_16(length, &required)) return MessageCodecStatus::overflow;
    if (written) *written = required;
    if (capacity < required) return MessageCodecStatus::capacity;
    if (!output) return MessageCodecStatus::invalid_argument;

    Aes128Schedule schedule{};
    if (aes128_expand_key(key, key_length, &schedule) != AesStatus::ok) return MessageCodecStatus::invalid_argument;

    const uint8_t padding = static_cast<uint8_t>(required - length);
    for (size_t offset = 0; offset < required; offset += 16) {
        uint8_t block[16];
        for (unsigned i = 0; i < 16; ++i) {
            const size_t pos = offset + i;
            block[i] = pos < length ? input[pos] : padding;
        }
        aes128_encrypt_block_expanded(schedule, block, output + offset);
    }
    return MessageCodecStatus::ok;
}

MessageCodecStatus decrypt_raw(const uint8_t* key, size_t key_length,
                               const uint8_t* input, size_t length,
                               uint8_t* output, size_t capacity,
                               size_t* written) noexcept
{
    if (written) *written = 0;
    const MessageCodecStatus key_status = check_key(key, key_length);
    if (key_status != MessageCodecStatus::ok) return key_status;
    if (!input && length != 0) return MessageCodecStatus::invalid_argument;
    if (written) *written = length;
    if (capacity < length) return MessageCodecStatus::capacity;
    if (!output && length != 0) return MessageCodecStatus::invalid_argument;
    if (length == 0) return MessageCodecStatus::ok;

    Aes128Schedule schedule{};
    if (aes128_expand_key(key, key_length, &schedule) != AesStatus::ok) return MessageCodecStatus::invalid_argument;

    size_t offset = 0;
    for (; length - offset >= 16; offset += 16) {
        aes128_decrypt_block_expanded(schedule, input + offset, output + offset);
    }
    for (; offset < length; ++offset) output[offset] = input[offset];
    return MessageCodecStatus::ok;
}

MessageCodecStatus encode_response(const uint8_t* key, size_t key_length,
                                   const uint8_t* input, size_t length,
                                   char* output, size_t capacity,
                                   size_t* written) noexcept
{
    if (written) *written = 0;
    if (length == 0 || key_length == 0) return MessageCodecStatus::empty_message;
    const MessageCodecStatus key_status = check_key(key, key_length);
    if (key_status != MessageCodecStatus::ok) return key_status;
    if (!input) return MessageCodecStatus::invalid_argument;

    size_t padded = 0;
    if (!padded_size_16(length, &padded)) return MessageCodecStatus::overflow;
    size_t hex_chars = 0;
    if (!preloader_l::core::checked_mul_size(padded, 2, hex_chars)) return MessageCodecStatus::overflow;
    size_t required = 0;
    if (!preloader_l::core::checked_add_size(hex_chars, 1, required)) return MessageCodecStatus::overflow;
    if (written) *written = required;
    if (capacity < required) return MessageCodecStatus::capacity;
    if (!output) return MessageCodecStatus::invalid_argument;

    Aes128Schedule schedule{};
    if (aes128_expand_key(key, key_length, &schedule) != AesStatus::ok) return MessageCodecStatus::invalid_argument;
    const uint8_t padding = static_cast<uint8_t>(padded - length);
    constexpr char digits[] = "0123456789abcdef";
    for (size_t offset = 0; offset < padded; offset += 16) {
        uint8_t plain[16];
        uint8_t encrypted[16];
        for (unsigned i = 0; i < 16; ++i) {
            const size_t pos = offset + i;
            plain[i] = pos < length ? input[pos] : padding;
        }
        aes128_encrypt_block_expanded(schedule, plain, encrypted);
        for (unsigned i = 0; i < 16; ++i) {
            const size_t pos = offset + i;
            output[2 * pos] = digits[encrypted[i] >> 4];
            output[2 * pos + 1] = digits[encrypted[i] & 0x0f];
        }
    }
    output[required - 1] = '\0';
    return MessageCodecStatus::ok;
}

MessageCodecStatus decode_payload(const uint8_t* key, size_t key_length,
                                  const char* input, size_t length,
                                  uint8_t* output, size_t capacity,
                                  size_t* written) noexcept
{
    if (written) *written = 0;
    const MessageCodecStatus key_status = check_key(key, key_length);
    if (key_status != MessageCodecStatus::ok) return key_status;
    if (!input && length != 0) return MessageCodecStatus::invalid_argument;
    if (length != 0 && input[length - 1] == '\0') --length;

    size_t decoded = 0;
    const HexStatus hex_status = hex_decode_strict(input, length, output, capacity, &decoded);
    if (hex_status != HexStatus::ok) return map_hex(hex_status);
    return decrypt_raw(key, key_length, output, decoded, output, capacity, written);
}

} } // namespace preloader_l::codec
