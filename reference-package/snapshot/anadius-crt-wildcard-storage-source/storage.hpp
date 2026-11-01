#pragma once
#include <cstdint>

namespace anadius::crt_wildcard_storage {
using Address = std::uint64_t;
struct Vector { Address begin{}, end{}, capacity_end{}; };
// Addresses are opaque storage identities. Providers validate readable/writable
// ranges, implement the named primitive, and preserve borrowed object lifetimes.
struct Effects {
    virtual ~Effects() = default;
    virtual std::uint64_t length(Address source) = 0;
    virtual Address calloc_066848(std::uint64_t count, std::uint64_t size) = 0;
    virtual Address recalloc_070434(Address old, std::uint64_t count, std::uint64_t size) = 0;
    virtual void free_0668c0(Address allocation) = 0;
    virtual std::uint32_t memcpy_s_0767fc(Address destination, std::uint64_t capacity,
                                        Address source, std::uint64_t count) = 0;
    virtual std::uint64_t read_word(Address source) = 0;
    virtual void write_word(Address destination, std::uint64_t value) = 0;
    [[noreturn]] virtual void watson_05c9a4() = 0;
};
std::uint32_t append_06e800(Address source, Address prefix, std::uint64_t prefix_bytes,
                            Vector& vector, Effects& effects);
}
