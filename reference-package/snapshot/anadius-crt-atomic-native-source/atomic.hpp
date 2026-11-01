#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>

namespace anadius::crt_atomic_native {
// New owned native storage, not an adapter over arbitrary source-record bytes.
// Finish all concurrent operations before destruction. Address is only an
// identity for routing known cells; it does not transfer storage ownership.
class Cell32 final {
public:
    explicit Cell32(std::uint32_t value = 0) noexcept;
    Cell32(const Cell32&) = delete;
    Cell32& operator=(const Cell32&) = delete;
    Cell32(Cell32&&) = delete;
    Cell32& operator=(Cell32&&) = delete;

    std::uint32_t fetch_add(std::uint32_t increment) noexcept;
    std::uint32_t exchange(std::uint32_t value) noexcept;
    std::uint32_t load() const noexcept;
    std::uintptr_t address() const noexcept;

private:
    alignas(4) mutable LONG value_;
};
}
