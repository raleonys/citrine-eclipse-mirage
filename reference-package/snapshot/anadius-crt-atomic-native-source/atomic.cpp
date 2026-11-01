#include "atomic.hpp"
#include <bit>

namespace anadius::crt_atomic_native {
static_assert(sizeof(LONG) == sizeof(std::uint32_t));
static_assert(alignof(Cell32) >= 4);
namespace {
LONG as_long(std::uint32_t value) noexcept {
    return std::bit_cast<LONG>(value);
}
std::uint32_t as_unsigned(LONG value) noexcept {
    return std::bit_cast<std::uint32_t>(value);
}
}
Cell32::Cell32(std::uint32_t value) noexcept : value_(as_long(value)) {}
std::uint32_t Cell32::fetch_add(std::uint32_t increment) noexcept {
    return as_unsigned(::InterlockedExchangeAdd(&value_, as_long(increment)));
}
std::uint32_t Cell32::exchange(std::uint32_t value) noexcept {
    return as_unsigned(::InterlockedExchange(&value_, as_long(value)));
}
std::uint32_t Cell32::load() const noexcept {
    return as_unsigned(::InterlockedCompareExchange(&value_, 0, 0));
}
std::uintptr_t Cell32::address() const noexcept {
    return reinterpret_cast<std::uintptr_t>(&value_);
}
}
