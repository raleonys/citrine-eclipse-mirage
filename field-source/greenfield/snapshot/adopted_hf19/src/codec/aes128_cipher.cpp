#include "preloader_l/codec/aes128_cipher.hpp"

namespace preloader_l { namespace codec {
namespace {
constexpr uint8_t gf_multiply(uint8_t a, uint8_t b) noexcept
{
    uint8_t result = 0;
    for (unsigned i = 0; i < 8; ++i) {
        if (b & 1u) result ^= a;
        a = static_cast<uint8_t>((static_cast<unsigned>(a) << 1u) ^ ((a & 0x80u) ? 0x11bu : 0u));
        b >>= 1u;
    }
    return result;
}

constexpr uint8_t rotate8(uint8_t value, unsigned amount) noexcept
{
    return static_cast<uint8_t>((static_cast<unsigned>(value) << amount) |
                                (static_cast<unsigned>(value) >> (8u - amount)));
}

struct Boxes {
    uint8_t forward[256]{};
    uint8_t inverse[256]{};
};

constexpr Boxes make_boxes() noexcept
{
    Boxes result{};
    for (unsigned i = 0; i < 256; ++i) {
        uint8_t inverse = 0;
        if (i != 0) {
            inverse = 1;
            uint8_t factor = static_cast<uint8_t>(i);
            for (unsigned exponent = 254; exponent != 0; exponent >>= 1u) {
                if (exponent & 1u) inverse = gf_multiply(inverse, factor);
                factor = gf_multiply(factor, factor);
            }
        }
        const uint8_t value = static_cast<uint8_t>(
            inverse ^ rotate8(inverse, 1) ^ rotate8(inverse, 2) ^
            rotate8(inverse, 3) ^ rotate8(inverse, 4) ^ 0x63u);
        result.forward[i] = value;
        result.inverse[value] = static_cast<uint8_t>(i);
    }
    return result;
}

constexpr Boxes boxes = make_boxes();

void expand_key_unchecked(const uint8_t* key, uint8_t* rounds) noexcept
{
    for (unsigned i = 0; i < 16; ++i) rounds[i] = key[i];
    uint8_t rcon = 1;
    for (unsigned i = 16; i < 176; i += 4) {
        uint8_t t[4] = {rounds[i - 4], rounds[i - 3], rounds[i - 2], rounds[i - 1]};
        if ((i & 15u) == 0) {
            const uint8_t first = t[0];
            t[0] = static_cast<uint8_t>(boxes.forward[t[1]] ^ rcon);
            t[1] = boxes.forward[t[2]];
            t[2] = boxes.forward[t[3]];
            t[3] = boxes.forward[first];
            rcon = gf_multiply(rcon, 2);
        }
        for (unsigned j = 0; j < 4; ++j) rounds[i + j] = static_cast<uint8_t>(rounds[i + j - 16] ^ t[j]);
    }
}

void add_round_key(uint8_t* state, const uint8_t* key) noexcept
{
    for (unsigned i = 0; i < 16; ++i) state[i] ^= key[i];
}

void substitute(uint8_t* state, bool inverse) noexcept
{
    const uint8_t* table = inverse ? boxes.inverse : boxes.forward;
    for (unsigned i = 0; i < 16; ++i) state[i] = table[state[i]];
}

void shift_rows(uint8_t* state, bool inverse) noexcept
{
    uint8_t copy[16];
    for (unsigned i = 0; i < 16; ++i) copy[i] = state[i];
    for (unsigned row = 0; row < 4; ++row) {
        for (unsigned column = 0; column < 4; ++column) {
            const unsigned source = (column + (inverse ? 4u - row : row)) & 3u;
            state[4u * column + row] = copy[4u * source + row];
        }
    }
}

void mix_columns(uint8_t* state, bool inverse) noexcept
{
    for (unsigned column = 0; column < 4; ++column) {
        uint8_t* p = state + 4u * column;
        const uint8_t a = p[0], b = p[1], c = p[2], d = p[3];
        if (inverse) {
            p[0] = static_cast<uint8_t>(gf_multiply(a, 14) ^ gf_multiply(b, 11) ^ gf_multiply(c, 13) ^ gf_multiply(d, 9));
            p[1] = static_cast<uint8_t>(gf_multiply(a, 9) ^ gf_multiply(b, 14) ^ gf_multiply(c, 11) ^ gf_multiply(d, 13));
            p[2] = static_cast<uint8_t>(gf_multiply(a, 13) ^ gf_multiply(b, 9) ^ gf_multiply(c, 14) ^ gf_multiply(d, 11));
            p[3] = static_cast<uint8_t>(gf_multiply(a, 11) ^ gf_multiply(b, 13) ^ gf_multiply(c, 9) ^ gf_multiply(d, 14));
        } else {
            p[0] = static_cast<uint8_t>(gf_multiply(a, 2) ^ gf_multiply(b, 3) ^ c ^ d);
            p[1] = static_cast<uint8_t>(a ^ gf_multiply(b, 2) ^ gf_multiply(c, 3) ^ d);
            p[2] = static_cast<uint8_t>(a ^ b ^ gf_multiply(c, 2) ^ gf_multiply(d, 3));
            p[3] = static_cast<uint8_t>(gf_multiply(a, 3) ^ b ^ c ^ gf_multiply(d, 2));
        }
    }
}

void block(const Aes128Schedule& schedule, const uint8_t* input,
           uint8_t* output, bool inverse) noexcept
{
    uint8_t state[16];
    for (unsigned i = 0; i < 16; ++i) state[i] = input[i];

    if (inverse) {
        add_round_key(state, schedule.bytes + 160);
        for (int round = 9; round > 0; --round) {
            shift_rows(state, true);
            substitute(state, true);
            add_round_key(state, schedule.bytes + 16 * round);
            mix_columns(state, true);
        }
        shift_rows(state, true);
        substitute(state, true);
        add_round_key(state, schedule.bytes);
    } else {
        add_round_key(state, schedule.bytes);
        for (unsigned round = 1; round < 10; ++round) {
            substitute(state, false);
            shift_rows(state, false);
            mix_columns(state, false);
            add_round_key(state, schedule.bytes + 16 * round);
        }
        substitute(state, false);
        shift_rows(state, false);
        add_round_key(state, schedule.bytes + 160);
    }

    for (unsigned i = 0; i < 16; ++i) output[i] = state[i];
}
} // namespace

AesStatus aes128_expand_key(const uint8_t* key, size_t key_length,
                            Aes128Schedule* schedule) noexcept
{
    if (key_length < 16) return AesStatus::key_too_short;
    if (!key || !schedule) return AesStatus::invalid_argument;
    expand_key_unchecked(key, schedule->bytes);
    return AesStatus::ok;
}

void aes128_encrypt_block_expanded(const Aes128Schedule& schedule,
                                   const uint8_t input[16], uint8_t output[16]) noexcept
{
    block(schedule, input, output, false);
}

void aes128_decrypt_block_expanded(const Aes128Schedule& schedule,
                                   const uint8_t input[16], uint8_t output[16]) noexcept
{
    block(schedule, input, output, true);
}

AesStatus aes128_encrypt_block(const uint8_t* key, size_t key_length,
                               const uint8_t input[16], uint8_t output[16]) noexcept
{
    if (!input || !output) return AesStatus::invalid_argument;
    Aes128Schedule schedule{};
    const AesStatus status = aes128_expand_key(key, key_length, &schedule);
    if (status != AesStatus::ok) return status;
    aes128_encrypt_block_expanded(schedule, input, output);
    return AesStatus::ok;
}

AesStatus aes128_decrypt_block(const uint8_t* key, size_t key_length,
                               const uint8_t input[16], uint8_t output[16]) noexcept
{
    if (!input || !output) return AesStatus::invalid_argument;
    Aes128Schedule schedule{};
    const AesStatus status = aes128_expand_key(key, key_length, &schedule);
    if (status != AesStatus::ok) return status;
    aes128_decrypt_block_expanded(schedule, input, output);
    return AesStatus::ok;
}

} } // namespace preloader_l::codec
