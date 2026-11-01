#pragma once
#include <cstdint>
#include "../anadius-crt-error-source/errors.hpp"

namespace anadius::crt_conversion_recovered {
using Address = std::uint64_t;
// New source representation of original fields +10/+18/+20/+28.
// Capacity and length count destination code units; this is not its full ABI.
struct Buffer {
    Address data{};
    std::uint64_t capacity{}, length{};
    std::uint8_t owned{};
};
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint8_t read_u8(Address) = 0;
    virtual std::uint16_t read_u16(Address) = 0;
    virtual void write_zero_u8(Address) = 0;
    virtual void write_zero_u16(Address) = 0;
    virtual Address malloc_06a308(std::uint64_t bytes) = 0;
    virtual void free_0668c0(Address) = 0;
    virtual std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t cp,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t capacity) = 0;
    virtual std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t cp,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t capacity, Address default_char,
        Address used_default) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual void map_os_error_05d698(std::uint32_t) = 0;
    virtual crt_errors_recovered::Cell32 errno_cell_05d724() = 0;
};
std::uint32_t to_wide_06004c(Address source, Buffer&, std::uint32_t cp, Effects&);
std::uint32_t to_narrow_06e430(Address source, Buffer&, std::uint32_t cp, Effects&);
}
