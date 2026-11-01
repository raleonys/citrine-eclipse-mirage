#include "preloader_l/core/byte_semantics.hpp"
#include "preloader_l/core/checked_math.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>

namespace {
int require(bool condition, int code) noexcept
{
    return condition ? 0 : code;
}
}

int main()
{
    using preloader_l::core::checked_add_size;
    using preloader_l::core::checked_mul_size;
    using preloader_l::core::sign_extend_byte_u32;
    using preloader_l::core::sign_extend_byte_u64;

    std::size_t out = 99;
    if (int rc = require(checked_add_size(20, 22, out) && out == 42, 1)) return rc;
    if (int rc = require(!checked_add_size((std::numeric_limits<std::size_t>::max)(), 1, out) && out == 0, 2)) return rc;
    if (int rc = require(checked_mul_size(6, 7, out) && out == 42, 3)) return rc;
    if (int rc = require(checked_mul_size(0, (std::numeric_limits<std::size_t>::max)(), out) && out == 0, 4)) return rc;
    if (int rc = require(!checked_mul_size((std::numeric_limits<std::size_t>::max)(), 2, out) && out == 0, 5)) return rc;

    if (int rc = require(sign_extend_byte_u64(0x00) == UINT64_C(0x0000000000000000), 10)) return rc;
    if (int rc = require(sign_extend_byte_u64(0x7f) == UINT64_C(0x000000000000007f), 11)) return rc;
    if (int rc = require(sign_extend_byte_u64(0x80) == UINT64_C(0xffffffffffffff80), 12)) return rc;
    if (int rc = require(sign_extend_byte_u64(0xff) == UINT64_C(0xffffffffffffffff), 13)) return rc;
    if (int rc = require(sign_extend_byte_u32(0x80) == UINT32_C(0xffffff80), 14)) return rc;
    if (int rc = require(sign_extend_byte_u32(0xff) == UINT32_C(0xffffffff), 15)) return rc;

    return 0;
}
