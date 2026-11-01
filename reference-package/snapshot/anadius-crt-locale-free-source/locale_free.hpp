#pragma once
#include <array>
#include <cstdint>

namespace anadius::crt_locale_free_recovered {
using Address = std::uint64_t;
struct Defaults {
    Address lconv_0a1700{};
    Address category_name_0a1948{};
    Address initial_time_08ad20{};
    // Fresh values are read at each comparison, including after a free effect.
    std::array<Address, 19> lconv_pointer_values{};
};
struct Effects {
    virtual ~Effects() = default;
    virtual Address read_address(Address) = 0;
    virtual std::uint32_t read_u32(Address) = 0;
    virtual void free_0668c0(Address) = 0;
};
void free_pointer_array_0713bc(Address, std::uint64_t count, Effects&);
void free_time_071730(Address, Effects&);
void free_time_if_unreferenced_070860(Address, const Defaults&, Effects&);
void free_monetary_070a40(Address, const Defaults&, Effects&);
void free_numeric_071068(Address, const Defaults&, Effects&);
void free_locale_0706c0(Address, const Defaults&, Effects&);
}
