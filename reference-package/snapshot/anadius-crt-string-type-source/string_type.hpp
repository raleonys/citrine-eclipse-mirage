#pragma once
#include <cstdint>
#include "../anadius-crt-byteclass-source/byteclass.hpp"

namespace anadius::crt_string_type_recovered {
using Address=std::uint64_t;
class Effects {
public:
    virtual ~Effects()=default;
    virtual crt_byteclass_recovered::LocaleContext locale_update_058b04(Address)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
    virtual Address malloc_06a308(std::uint64_t)=0;
    virtual void free_0668c0(Address)=0;
    // Returns the marker base of caller-owned scratch, aligned as the original
    // frame. The caller restores that scratch frame after this function returns.
    virtual Address stack_allocate(std::uint64_t rounded_bytes)=0;
    virtual void zero_bytes(Address,std::uint64_t)=0;
    virtual std::int32_t multibyte_to_wide_06ccc8(std::uint32_t codepage,
        std::uint32_t flags,Address source,std::int32_t count,Address destination,
        std::int32_t capacity)=0;
    virtual std::uint32_t native_get_string_type_w(std::uint32_t type,
        Address wide_source,std::int32_t count,Address output)=0;
};
std::uint32_t get_string_type_06e210(Address locale,std::uint32_t type,
    Address source,std::int32_t count,Address output,std::uint32_t codepage,
    std::int32_t strict,Effects&);
}
