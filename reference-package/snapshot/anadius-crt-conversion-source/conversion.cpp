#include "conversion.hpp"
#include <bit>

namespace anadius::crt_conversion_recovered {
namespace {
void release_owned(Buffer& buffer, Effects& effects) {
    if (buffer.owned != 0) {
        effects.free_0668c0(buffer.data);
        buffer.owned = 0;
    }
}
bool allocate(Buffer& buffer, std::uint64_t count, std::uint64_t width,
              Effects& effects) {
    release_owned(buffer, effects);
    buffer.data = effects.malloc_06a308(count * width);
    buffer.owned = buffer.data != 0;
    buffer.capacity = buffer.owned ? count : 0;
    return buffer.owned != 0;
}
template<bool ToWide>
std::uint32_t convert(Address source, Buffer& buffer, std::uint32_t cp,
                      Effects& effects) {
    constexpr std::uint64_t width = ToWide ? 2 : 1;
    if (source == 0) {
        release_owned(buffer, effects);
        buffer.data = 0;
        buffer.capacity = 0;
        buffer.length = 0;
        return 0;
    }
    const bool empty = ToWide ? effects.read_u8(source) == 0
                              : effects.read_u16(source) == 0;
    if (empty) {
        if (buffer.capacity == 0 && !allocate(buffer, 1, width, effects))
            return 12;
        if constexpr (ToWide) effects.write_zero_u16(buffer.data);
        else effects.write_zero_u8(buffer.data);
        buffer.length = 0;
        return 0;
    }
    const auto invoke = [&](Address destination, std::int32_t capacity) {
        if constexpr (ToWide)
            return effects.multi_byte_to_wide_06ccc8(cp, 9, source, -1,
                                                     destination, capacity);
        else
            return effects.wide_char_to_multi_byte_06cd58(cp, 0, source, -1,
                                                          destination, capacity, 0, 0);
    };
    const auto fail = [&]() {
        const auto error = effects.last_error();
        effects.map_os_error_05d698(error);
        return effects.errno_cell_05d724().load();
    };
    const auto required_result = invoke(0, 0);
    if (required_result == 0) return fail();
    // MOVSXD followed by unsigned JBE, including a negative provider result.
    const auto required = static_cast<std::uint64_t>(
        static_cast<std::int64_t>(required_result));
    if (buffer.capacity < required && !allocate(buffer, required, width, effects))
        return 12;
    // EDX is the low DWORD of capacity; preserve its bits even above INT_MAX.
    const auto capacity = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(buffer.capacity));
    const auto result = invoke(buffer.data, capacity);
    if (result == 0) return fail();
    // CDQE / DEC RAX: length is a sign-extended result minus one modulo 2^64.
    buffer.length = static_cast<std::uint64_t>(static_cast<std::int64_t>(result)) - 1;
    return 0;
}
}
std::uint32_t to_wide_06004c(Address source, Buffer& buffer, std::uint32_t cp,
                            Effects& effects) {
    return convert<true>(source, buffer, cp, effects);
}
std::uint32_t to_narrow_06e430(Address source, Buffer& buffer, std::uint32_t cp,
                              Effects& effects) {
    return convert<false>(source, buffer, cp, effects);
}
}
