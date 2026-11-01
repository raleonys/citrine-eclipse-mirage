#pragma once
#include <stdint.h>

namespace preloader_l { namespace debug {

enum SelfTestBit : uint32_t {
    self_test_hex     = 1u << 0,
    self_test_aes128  = 1u << 1,
    self_test_framing = 1u << 2,
    self_test_router  = 1u << 3,
};

struct SelfTestReport {
    uint32_t passed_mask;
    uint32_t failed_mask;
};

constexpr uint32_t self_test_all_mask =
    self_test_hex | self_test_aes128 | self_test_framing | self_test_router;

SelfTestReport run_self_test() noexcept;

} }
