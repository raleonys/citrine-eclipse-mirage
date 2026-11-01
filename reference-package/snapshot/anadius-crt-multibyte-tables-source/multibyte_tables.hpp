#pragma once
#include <array>
#include <cstdint>
#include <span>
#include <stdexcept>

namespace anadius::crt_multibyte_tables_recovered {
using Address = std::uint64_t;
struct CPInfo {
    std::uint32_t max_char_size;
    std::array<std::uint8_t, 12> lead_bytes;
};
// The original API output arrays were uninitialized. A consumed cell must have
// evidence-backed bytes supplied by its provider; API failure does not zero it.
template<class T> class OutputCell {
    T value_{};
    bool defined_{};
public:
    void set(T value) { value_ = value; defined_ = true; }
    bool defined() const { return defined_; }
    T get() const {
        if (!defined_) throw std::runtime_error("06f0e8 reconstruction boundary: consumed undefined API output");
        return value_;
    }
};
using ByteOutput = OutputCell<std::uint8_t>;
using WordOutput = OutputCell<std::uint16_t>;
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint8_t read_u8(Address) = 0;
    virtual std::uint16_t read_u16(Address) = 0;
    virtual std::uint32_t read_u32(Address) = 0;
    virtual Address read_address(Address) = 0;
    virtual void write_u8(Address, std::uint8_t) = 0;
    virtual void write_u16(Address, std::uint16_t) = 0;
    virtual void write_u32(Address, std::uint32_t) = 0;
    virtual void write_address(Address, Address) = 0;
    // A nonzero result requires all twelve lead bytes to have been supplied.
    virtual std::uint32_t get_cp_info(std::uint32_t, CPInfo&) = 0;
    virtual Address initial_multibyte_0a1c70() = 0;
    virtual std::uint32_t get_string_type_06e210(
        Address locale, std::uint32_t type, std::span<const std::uint8_t> input,
        std::span<WordOutput> output, std::uint32_t codepage, std::uint32_t error_flags) = 0;
    virtual std::uint32_t map_string_06cc30(
        Address locale_context, Address locale_name, std::uint32_t flags,
        std::span<const std::uint8_t> input, std::span<ByteOutput> output,
        std::uint32_t codepage, std::uint32_t error_flags) = 0;
};
void reset_sbcs_06f050(Address record, Effects&);
void build_case_tables_06f0e8(Address record, Effects&);
}
