#pragma once
#include <cstdint>
#include "../anadius-crt-runtime-source/runtime.hpp"

namespace anadius::crt_conversion_api {
using Address = std::uint64_t;
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::int32_t multi_byte_to_wide(std::uint32_t code_page,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t capacity) = 0;
    virtual std::int32_t wide_char_to_multi_byte(std::uint32_t code_page,
        std::uint32_t flags, Address source, std::int32_t source_count,
        Address destination, std::int32_t capacity,
        Address default_character, Address used_default_character) = 0;
    virtual void write_bool32(Address, std::int32_t value) = 0;
};
std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t code_page,
    std::uint32_t flags, Address source, std::int32_t source_count,
    Address destination, std::int32_t capacity, Effects&);
std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t code_page,
    std::uint32_t flags, Address source, std::int32_t source_count,
    Address destination, std::int32_t capacity,
    Address default_character, Address used_default_character, Effects&);
class FileApiEffects {
public:
    virtual ~FileApiEffects() = default;
    virtual std::uint32_t call_are_file_apis_ansi(Address) = 0;
};
std::uint32_t file_api_ansi_066c1c(crt_runtime_recovered::State&,
    crt_runtime_recovered::Effects&, FileApiEffects&);
} // namespace anadius::crt_conversion_api
