#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace anadius::crt_locale_names_recovered {
using Address = std::uint64_t;
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint16_t read_u16(Address) = 0;
};
struct NameRow {
    std::u16string_view name;
    std::int32_t lcid_index;
    Address original_name_address;
};
// The right operand is a view of an immutable recovered static string. It is
// required to include its logical trailing NUL at right.data()[right.size()].
// This is a logical interface, not the original x64 ABI declaration.
std::int32_t ascii_wcsnicmp_0736d4(Address left, std::u16string_view right,
                                 std::uint64_t count, Effects&);
std::uint32_t locale_name_to_lcid_07310c(Address name, Effects&);
std::span<const NameRow, 228> shipped_name_rows();
std::span<const std::uint32_t, 228> shipped_lcid_values();
} // namespace anadius::crt_locale_names_recovered
