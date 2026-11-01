#include "preloader_l/codec/session_crypto.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace preloader_l::codec;

int main()
{
    uint8_t key[16]{};
    const uint8_t zero_key[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    if (derive_session_key(0, key) != SessionStatus::ok || std::memcmp(key, zero_key, 16) != 0) return 1;

    const uint8_t seed1[16] = {0xf1,0x48,0x70,0x5a,0xa6,0x16,0x79,0x2a,0xb9,0x98,0x0b,0x6f,0x39,0x69,0xe3,0x42};
    if (derive_session_key(1, key) != SessionStatus::ok || std::memcmp(key, seed1, 16) != 0) return 2;
    const uint8_t seed65535[16] = {0xe7,0x58,0xc5,0xac,0x1e,0x42,0x9d,0xaa,0xd6,0x49,0x38,0x08,0x70,0x9d,0x18,0x0f};
    if (derive_session_key(65535, key) != SessionStatus::ok || std::memcmp(key, seed65535, 16) != 0) return 3;

    const uint8_t challenge[] = {'a'};
    char reply[128]{};
    size_t written = 0;
    constexpr char expected_reply[] = "fe123486508b0eb503a8468388ef5812";
    const uint8_t expected_response_seed_a[16] = {0x4e,0x54,0x1f,0x16,0x5f,0x6a,0x47,0xa9,0xc5,0x18,0xd6,0x66,0x8a,0x14,0x45,0x37};
    const uint8_t expected_hf14_dynamic_a[16] = {0xb0,0xb7,0x4a,0xc9,0x14,0x8c,0x10,0xef,0x7b,0x0a,0xb9,0xca,0xaf,0x68,0xf4,0x58};
    if (make_challenge_reply(challenge, 1, "3", 1, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        written != 32 || std::memcmp(reply, expected_reply, 32) != 0 || reply[32] != '\0' ||
        std::memcmp(key, expected_response_seed_a, 16) != 0) return 4;

    if (make_challenge_reply(challenge, 1, "2", 1, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 5;
    if (make_challenge_reply(challenge, 1, nullptr, 0, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 6;
    if (make_challenge_reply(challenge, 1, "", 0, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 7;
    if (make_challenge_reply(challenge, 1, "garbage", 7, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 8;
    if (make_challenge_reply(challenge, 1, "  +3junk", 8, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 9;

    // PRE-AUDIT Wave5 / F09: Donor1's exact version parser is a signed
    // 32-bit decimal parser with saturation. These vectors pin the boundary
    // semantics while ensuring the implementation contains no signed-overflow UB.
    if (make_challenge_reply(challenge, 1, "2147483647", 10, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 15;
    if (make_challenge_reply(challenge, 1, "2147483648", 10, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 16;
    if (make_challenge_reply(challenge, 1, "4294967296", 10, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 17;
    if (make_challenge_reply(challenge, 1, "-2147483648", 11, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 18;
    if (make_challenge_reply(challenge, 1, "-2147483649", 11, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 19;
    if (make_challenge_reply(challenge, 1, "999999999999999999999", 21, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 20;
    if (make_challenge_reply(challenge, 1, "0003", 4, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, expected_hf14_dynamic_a, 16) != 0) return 21;
    if (make_challenge_reply(challenge, 1, "+", 1, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        std::memcmp(key, zero_key, 16) != 0) return 22;

    const uint8_t challenge16[] = "0123456789abcdef";
    constexpr char expected16[] = "281567ab2f4cf0d73d3198225b8b8393954f64f2e4e86e9eee82d20216684899";
    const uint8_t key16[16] = {0xec,0xd4,0x05,0xe4,0x65,0x37,0xb7,0x84,0xa6,0x55,0xf5,0x3b,0x68,0x97,0xdb,0xc8};
    if (make_challenge_reply(challenge16, 16, "3", 1, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        written != 64 || std::memcmp(reply, expected16, 64) != 0 || std::memcmp(key, key16, 16) != 0) return 10;

    const uint8_t expected_empty_hf14_dynamic[16] = {0xed,0x4c,0xa8,0x5b,0x5c,0xbf,0x51,0x99,0x39,0x9c,0x9f,0x43,0xbe,0xc7,0x33,0x88};
    constexpr char expected_empty_reply[] = "954f64f2e4e86e9eee82d20216684899";
    const uint8_t empty_key_marker = 0;
    if (make_challenge_reply(&empty_key_marker, 0, nullptr, 0, key, reply, sizeof reply, &written) != SessionStatus::ok ||
        written != 32 || std::memcmp(reply, expected_empty_reply, 32) != 0 ||
        std::memcmp(key, expected_empty_hf14_dynamic, 16) != 0) return 11;

    reply[0] = 'x'; written = 0;
    if (make_challenge_reply(challenge, 1, "3", 1, key, reply, 4, &written) != SessionStatus::capacity || written != 32 || reply[0] != 'x') return 12;
    if (derive_session_key(1, nullptr) != SessionStatus::invalid_argument) return 13;
    if (make_challenge_reply(nullptr, 1, "3", 1, key, reply, sizeof reply, &written) != SessionStatus::invalid_argument) return 14;

    return 0;
}
