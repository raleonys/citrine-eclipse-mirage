#include "multibyte_tables.hpp"

namespace anadius::crt_multibyte_tables_recovered {
void reset_sbcs_06f050(Address record, Effects& fx) {
    for (Address i = 0; i != 257; ++i) fx.write_u8(record + 0x18 + i, 0);
    const Address initial = fx.initial_multibyte_0a1c70();
    fx.write_address(record + 4, 0);
    fx.write_address(record + 0x220, 0);
    for (Address i = 0; i != 6; ++i) fx.write_u16(record + 0xc + 2 * i, 0);
    for (Address i = 0; i != 257; ++i) {
        const auto byte = fx.read_u8(initial + 0x18 + i);
        fx.write_u8(record + 0x18 + i, byte);
    }
    for (Address i = 0; i != 256; ++i) {
        const auto byte = fx.read_u8(initial + 0x119 + i);
        fx.write_u8(record + 0x119 + i, byte);
    }
}

void build_case_tables_06f0e8(Address record, Effects& fx) {
    const auto codepage = fx.read_u32(record + 4);
    CPInfo info; // Only read after the successful provider's output contract.
    if (codepage == 65001 || fx.get_cp_info(codepage, info) == 0) {
        for (std::uint32_t i = 0; i != 256; ++i) {
            std::uint8_t mapped = 0;
            if (i - 0x41U < 26) {
                const auto old = fx.read_u8(record + 0x19 + i);
                fx.write_u8(record + 0x19 + i, static_cast<std::uint8_t>(old | 0x10));
                mapped = static_cast<std::uint8_t>(i + 0x20);
            } else if (i - 0x61U < 26) {
                const auto old = fx.read_u8(record + 0x19 + i);
                fx.write_u8(record + 0x19 + i, static_cast<std::uint8_t>(old | 0x20));
                mapped = static_cast<std::uint8_t>(i - 0x20);
            }
            fx.write_u8(record + 0x119 + i, mapped);
        }
        return;
    }
    std::array<std::uint8_t, 256> input;
    for (std::uint32_t i = 0; i != 256; ++i) input[i] = static_cast<std::uint8_t>(i);
    input[0] = 0x20;
    for (std::size_t pair = 0;; pair += 2) {
        if (pair >= info.lead_bytes.size())
            throw std::runtime_error("06f0e8 reconstruction boundary: unterminated CPInfo lead-byte pairs");
        const auto first = info.lead_bytes[pair];
        if (first == 0) break;
        const auto last = info.lead_bytes[pair + 1];
        for (std::uint32_t i = first; i <= last && i < 256; ++i) input[i] = 0x20;
    }
    std::array<WordOutput, 256> types;
    std::array<ByteOutput, 256> lower;
    std::array<ByteOutput, 256> upper;
    const auto type_cp = fx.read_u32(record + 4);
    (void)fx.get_string_type_06e210(0, 1, input, types, type_cp, 0);
    const auto lower_cp = fx.read_u32(record + 4);
    const auto lower_locale = fx.read_address(record + 0x220);
    (void)fx.map_string_06cc30(0, lower_locale, 0x100, input, lower, lower_cp, 0);
    const auto upper_cp = fx.read_u32(record + 4);
    const auto upper_locale = fx.read_address(record + 0x220);
    (void)fx.map_string_06cc30(0, upper_locale, 0x200, input, upper, upper_cp, 0);
    for (std::uint32_t i = 0; i != 256; ++i) {
        const auto type = types[i].get();
        std::uint8_t mapped = 0;
        if ((type & 1) != 0) {
            const auto old = fx.read_u8(record + 0x19 + i);
            fx.write_u8(record + 0x19 + i, static_cast<std::uint8_t>(old | 0x10));
            mapped = lower[i].get();
        } else if ((type & 2) != 0) {
            const auto old = fx.read_u8(record + 0x19 + i);
            fx.write_u8(record + 0x19 + i, static_cast<std::uint8_t>(old | 0x20));
            mapped = upper[i].get();
        }
        fx.write_u8(record + 0x119 + i, mapped);
    }
}
}
