#pragma once
#include <cstdint>
#include "../anadius-crt-byteclass-source/byteclass.hpp"

namespace anadius::crt_lcmap_recovered {
using Address=std::uint64_t;
using Context=crt_byteclass_recovered::LocaleContext;
class Effects {
public:
    virtual ~Effects()=default;
    virtual Context locale_update_058b04(Address locale)=0;
    virtual std::uint8_t read_u8(Address)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
    virtual Address malloc_06a308(std::uint64_t bytes)=0;
    virtual void free_0668c0(Address)=0;
    // Returns the marker address in caller-owned scratch, valid through this
    // call and independent of earlier allocations. No reconstructed stack ABI.
    virtual Address stack_allocate(std::uint64_t rounded_bytes)=0;
    virtual std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t,std::uint32_t,
        Address,std::int32_t,Address,std::int32_t)=0;
    virtual std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t,std::uint32_t,
        Address,std::int32_t,Address,std::int32_t,Address,Address)=0;
    // Required native dispatch/resolver body 0671d4 remains unreconstructed.
    virtual std::int32_t lc_map_string_0671d4(Address locale_name,std::uint32_t flags,
        Address source,std::int32_t source_count,Address destination,
        std::int32_t capacity,Address version,Address reserved,std::uint64_t sort_handle)=0;
};
std::uint64_t strncnt_063168(Address source,std::uint64_t count,Effects&);
std::uint32_t lc_map_inner_06c8fc(const Context&,Address locale_name,std::uint32_t flags,
    Address source,std::int32_t source_count,Address destination,
    std::int32_t capacity,std::uint32_t code_page,std::int32_t error_on_invalid,Effects&);
std::uint32_t lc_map_string_06cc30(Address locale,Address locale_name,std::uint32_t flags,
    Address source,std::int32_t source_count,Address destination,
    std::int32_t capacity,std::uint32_t code_page,std::int32_t error_on_invalid,Effects&);
}
