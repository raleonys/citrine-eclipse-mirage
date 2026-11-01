#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l::core {

// Small, allocation-free helpers used by later buffer-oriented stages.
// On failure, the output is reset to zero so callers never consume a stale
// partially computed size.
constexpr bool checked_add_size(size_t a, size_t b, size_t& out) noexcept
{
    if (b > SIZE_MAX - a) {
        out = 0;
        return false;
    }
    out = a + b;
    return true;
}

constexpr bool checked_mul_size(size_t a, size_t b, size_t& out) noexcept
{
    if (a != 0 && b > SIZE_MAX / a) {
        out = 0;
        return false;
    }
    out = a * b;
    return true;
}

} // namespace preloader_l::core
