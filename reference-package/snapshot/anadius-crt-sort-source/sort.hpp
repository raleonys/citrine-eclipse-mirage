#pragma once
#include "../anadius-crt-error-source/errors.hpp"
#include <cstdint>

namespace anadius::crt_sort_recovered {
using Address = std::uint64_t;
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint8_t read_u8(Address) = 0;
    virtual void write_u8(Address, std::uint8_t) = 0;
    virtual std::int32_t compare(Address comparator, Address lhs, Address rhs) = 0;
    virtual crt_errors_recovered::Cell32 errno_cell_05d724() = 0;
    virtual void invalid_parameter_05c954() = 0;
};
// All addresses are opaque identities. Providers own bounds, lifetime and access.
// The comparator receives element addresses, never the values stored in slots.
void qsort_0768f0(Address base, std::uint64_t count, std::uint64_t width,
                 Address comparator, Effects&);
// Bounded offline machine-code evidence; absent from the retained Ghidra
// function inventory. Unsigned address order; does not dereference either slot.
std::int32_t compare_slot_addresses_06e41c(Address lhs, Address rhs) noexcept;
}
