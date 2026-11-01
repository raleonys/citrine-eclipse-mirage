#include "module_path.hpp"
#include <bit>

namespace anadius::crt_module_path_recovered {
std::uint32_t fixed_to_narrow_0601c8(Address source, cv::Buffer& buffer,
                                    std::uint32_t cp, cv::Effects& effects) {
    if (source == 0) {
        buffer.owned = 0;
        buffer.data = 0;
        buffer.capacity = 0;
        buffer.length = 0;
        return 0;
    }
    const auto capacity_error = [&]() {
        // No allocator/free call here, including when owned was set.
        buffer.owned = 0;
        effects.errno_cell_05d724().store(34);
        buffer.owned = 0;
        buffer.capacity = 0;
        return std::uint32_t{34};
    };
    if (effects.read_u16(source) == 0) {
        if (buffer.capacity == 0) return capacity_error();
        effects.write_zero_u8(buffer.data);
        buffer.length = 0;
        return 0;
    }
    const auto invoke = [&](Address destination, std::int32_t capacity) {
        return effects.wide_char_to_multi_byte_06cd58(cp, 0, source, -1,
                                                     destination, capacity, 0, 0);
    };
    const auto conversion_error = [&]() {
        const auto error = effects.last_error();
        effects.map_os_error_05d698(error);
        return effects.errno_cell_05d724().load();
    };
    const auto query = invoke(0, 0);
    if (query == 0) return conversion_error();
    // MOVSXD then unsigned JBE; a negative query is a huge unsigned size.
    const auto required = static_cast<std::uint64_t>(static_cast<std::int64_t>(query));
    if (required > buffer.capacity) return capacity_error();
    const auto result = invoke(buffer.data, std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(buffer.capacity)));
    if (result == 0) return conversion_error();
    buffer.length = static_cast<std::uint64_t>(static_cast<std::int64_t>(result)) - 1;
    return 0;
}

std::uint32_t module_filename_06ecdc(Address module, Address destination,
                                    std::uint64_t capacity, Effects& effects) {
    const auto wide = effects.get_module_filename_w(module, 261);
    if (wide.count == 0) {
        const auto error = effects.last_error();
        effects.map_os_error_05d698(error);
        return 0;
    }
    cv::Buffer buffer{destination, static_cast<std::uint32_t>(capacity), 0, 0};
    const auto cp = effects.choose_code_page();
    (void)fixed_to_narrow_0601c8(wide.data, buffer, cp, effects);
    return static_cast<std::uint32_t>(buffer.length);
}
}
