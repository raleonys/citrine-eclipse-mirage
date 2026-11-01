#include "sort.hpp"
#include <array>
#include <bit>
#include <stdexcept>

namespace anadius::crt_sort_recovered {
namespace {
void swap_bytes(Address left, Address right, std::uint64_t width, Effects& fx) {
    if (left == right) return;
    // Every retained swap reads the right byte, then the left byte, writes
    // left, then writes right. Preserve that order for partial provider failure.
    for (std::uint64_t i = 0; i != width; ++i) {
        const auto right_byte = fx.read_u8(right + i);
        const auto left_byte = fx.read_u8(left + i);
        fx.write_u8(left + i, right_byte);
        fx.write_u8(right + i, left_byte);
    }
}
}
std::int32_t compare_slot_addresses_06e41c(Address lhs, Address rhs) noexcept {
    if (lhs < rhs) return -1;
    return lhs > rhs ? 1 : 0;
}
void qsort_0768f0(Address lo, std::uint64_t count, std::uint64_t width,
                 Address comparator, Effects& fx) {
    if ((lo == 0 && count != 0) || width == 0 || comparator == 0) {
        fx.errno_cell_05d724().store(0x16);
        fx.invalid_parameter_05c954();
        return;
    }
    if (count < 2) return;
    std::array<Address, 62> low_stack{};
    std::array<Address, 62> high_stack{};
    std::size_t depth = 0;
    Address hi = lo + (count - 1) * width;
    Address mid = 0, loguy = 0, higuy = 0, previous_hi = 0;
    std::uint64_t size = 0;
    auto push = [&](Address low, Address high) {
        // The original has two fixed 62-entry arrays and no explicit check.
        // This guard marks unsupported overflow rather than corrupting host memory.
        if (depth == low_stack.size()) throw std::domain_error("qsort stack exceeds 62 entries");
        low_stack[depth] = low;
        high_stack[depth] = high;
        ++depth;
    };
next_range:
    size = (hi - lo) / width + 1;
    if (size <= 8) {
        while (hi > lo) {
            Address maximum = lo;
            for (Address p = lo + width; p <= hi; p += width) {
                if (fx.compare(comparator, p, maximum) > 0) maximum = p;
            }
            swap_bytes(maximum, hi, width, fx);
            hi -= width;
        }
        goto pop_range;
    }
    mid = lo + (size >> 1) * width;
    if (fx.compare(comparator, lo, mid) > 0) swap_bytes(lo, mid, width, fx);
    if (fx.compare(comparator, lo, hi) > 0) swap_bytes(lo, hi, width, fx);
    if (fx.compare(comparator, mid, hi) > 0) swap_bytes(mid, hi, width, fx);
    loguy = lo;
    higuy = hi;
partition:
    if (loguy < mid) {
left_before_pivot:
        loguy += width;
        if (loguy >= mid) goto left_after_pivot;
        if (fx.compare(comparator, loguy, mid) <= 0) goto left_before_pivot;
        goto scan_right;
    }
left_after_pivot:
    loguy += width;
    if (loguy > hi) goto scan_right;
    if (fx.compare(comparator, loguy, mid) <= 0) goto left_after_pivot;
scan_right:
    previous_hi = higuy;
    higuy -= width;
    if (higuy > mid && fx.compare(comparator, higuy, mid) > 0) goto scan_right;
    if (higuy >= loguy) {
        swap_bytes(loguy, higuy, width, fx);
        if (mid == higuy) mid = loguy;
        goto partition;
    }
    if (mid < previous_hi) {
equal_before_pivot:
        previous_hi -= width;
        if (previous_hi <= mid) goto equal_after_pivot;
        if (fx.compare(comparator, previous_hi, mid) == 0) goto equal_before_pivot;
        goto select_range;
    }
equal_after_pivot:
    previous_hi -= width;
    if (previous_hi <= lo) goto select_range;
    if (fx.compare(comparator, previous_hi, mid) == 0) goto equal_after_pivot;
select_range:
    // CMP/JL is signed here, although address comparisons above are unsigned.
    if (std::bit_cast<std::int64_t>(previous_hi - lo) <
        std::bit_cast<std::int64_t>(hi - loguy)) {
        if (loguy < hi) push(loguy, hi);
        if (previous_hi <= lo) goto pop_range;
        hi = previous_hi;
        goto next_range;
    }
    if (lo < previous_hi) push(lo, previous_hi);
    if (loguy >= hi) goto pop_range;
    lo = loguy;
    goto next_range;
pop_range:
    if (depth == 0) return;
    --depth;
    lo = low_stack[depth];
    hi = high_stack[depth];
    goto next_range;
}
}
