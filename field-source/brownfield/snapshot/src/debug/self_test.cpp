#include "preloader_l/debug/self_test.hpp"
#include "preloader_l/debug/debug_diagnostics.hpp"
#include "preloader_l/codec/aes128_cipher.hpp"
#include "preloader_l/codec/hex_codec.hpp"
#include "preloader_l/xml/lsx_framing.hpp"
#include "preloader_l/router/route_inventory.hpp"

namespace preloader_l { namespace debug {
namespace {
bool same_bytes(const uint8_t* a, const uint8_t* b, size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) if (a[i] != b[i]) return false;
    return true;
}

bool check_hex() noexcept {
    const uint8_t input[] = {0x00, 0x01, 0xab, 0xff};
    char encoded[8]{};
    size_t encoded_len = 0;
    if (preloader_l::codec::hex_encode_lower(input, sizeof(input), encoded, sizeof(encoded), &encoded_len) != preloader_l::codec::HexStatus::ok) return false;
    if (encoded_len != sizeof(encoded)) return false;
    const char expected[] = "0001abff";
    for (size_t i = 0; i < sizeof(encoded); ++i) if (encoded[i] != expected[i]) return false;
    uint8_t decoded[4]{};
    size_t decoded_len = 0;
    if (preloader_l::codec::hex_decode_strict(encoded, encoded_len, decoded, sizeof(decoded), &decoded_len) != preloader_l::codec::HexStatus::ok) return false;
    return decoded_len == sizeof(input) && same_bytes(decoded, input, sizeof(input));
}

bool check_aes128() noexcept {
    const uint8_t key[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    const uint8_t plain[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    const uint8_t expected[16] = {0x69,0xc4,0xe0,0xd8,0x6a,0x7b,0x04,0x30,0xd8,0xcd,0xb7,0x80,0x70,0xb4,0xc5,0x5a};
    uint8_t encrypted[16]{};
    uint8_t decrypted[16]{};
    if (preloader_l::codec::aes128_encrypt_block(key, sizeof(key), plain, encrypted) != preloader_l::codec::AesStatus::ok) return false;
    if (!same_bytes(encrypted, expected, sizeof(expected))) return false;
    if (preloader_l::codec::aes128_decrypt_block(key, sizeof(key), encrypted, decrypted) != preloader_l::codec::AesStatus::ok) return false;
    return same_bytes(decrypted, plain, sizeof(plain));
}

bool check_framing() noexcept {
    const char frame[] = "<LSX><Ping/></LSX>tail";
    const auto result = preloader_l::xml::scan_lsx_frame(frame, sizeof(frame) - 1);
    return result.status == preloader_l::xml::FrameStatus::frame_ready && result.frame_length == sizeof("<LSX><Ping/></LSX>") - 1;
}

bool check_router() noexcept {
    const char name[] = "GetConfig";
    const preloader_l::xml::TextSpan span{name, sizeof(name) - 1};
    const auto* route = preloader_l::router::find_known_route(span);
    return route && preloader_l::router::descriptor_route_id(*route) == preloader_l::router::RouteId::get_config;
}
}

SelfTestReport run_self_test() noexcept {
    SelfTestReport report{0u, 0u};
    const struct { uint32_t bit; bool (*fn)() noexcept; } checks[] = {
        {self_test_hex, &check_hex},
        {self_test_aes128, &check_aes128},
        {self_test_framing, &check_framing},
        {self_test_router, &check_router},
    };
    for (const auto& c : checks) {
        const bool ok = c.fn();
        if (ok) report.passed_mask |= c.bit;
        else report.failed_mask |= c.bit;
    }
    note_codec_check();
    note_framing_check();
    note_route_check();
    note_self_test(report.failed_mask == 0u && report.passed_mask == self_test_all_mask);
    return report;
}

} }
