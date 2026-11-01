#pragma once
#include "../anadius-crt-conversion-source/conversion.hpp"
#include "../anadius-crt-conversion-api-source/api.hpp"
#include "../anadius-crt-recalloc-bridge-source/bridge.hpp"
namespace anadius::crt_conversion_bridge {
using Address=std::uint64_t;
class MemoryEffects {
public:
    virtual ~MemoryEffects()=default;
    virtual std::uint8_t read_u8(Address)=0;
    virtual std::uint16_t read_u16(Address)=0;
    virtual void write_zero_u8(Address)=0;
    virtual void write_zero_u16(Address)=0;
    virtual std::uint32_t last_error()=0;
};
// New source composition; dependencies are borrowed and must share a live
// provider-owned address domain. This class does not install native providers.
class BufferEffects final:public crt_conversion_recovered::Effects {
public:
    BufferEffects(crt_heap_recovered::State&,crt_realloc_recovered::Effects&,
        crt_errors_recovered::State&,crt_errors_recovered::Effects&,
        MemoryEffects&,crt_conversion_api::Effects&);
    std::uint8_t read_u8(Address) final;
    std::uint16_t read_u16(Address) final;
    void write_zero_u8(Address) final;
    void write_zero_u16(Address) final;
    Address malloc_06a308(std::uint64_t) final;
    void free_0668c0(Address) final;
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t,Address,Address) final;
    std::uint32_t last_error() final;
    void map_os_error_05d698(std::uint32_t) final;
    crt_errors_recovered::Cell32 errno_cell_05d724() final;
private:
    crt_heap_recovered::State& heap_;
    crt_realloc_recovered::Effects& heap_fx_;
    crt_errors_recovered::State& errors_;
    crt_errors_recovered::Effects& error_fx_;
    MemoryEffects& memory_;
    crt_conversion_api::Effects& api_;
};
class WildcardEffects:public crt_recalloc_bridge::WildcardHeapEffects {
public:
    WildcardEffects(crt_recalloc_bridge::Allocator&,crt_conversion_recovered::Effects&,
        crt_runtime_recovered::State&,crt_runtime_recovered::Effects&,
        crt_conversion_api::FileApiEffects&);
    crt_wildcard_search::Conversion to_wide_06004c(Address,std::uint32_t) final;
    crt_wildcard_search::Conversion to_narrow_06e430(Address,std::uint32_t) final;
    std::uint32_t file_api_ansi_066c1c() final;
private:
    crt_conversion_recovered::Effects& conversion_;
    crt_runtime_recovered::State& runtime_;
    crt_runtime_recovered::Effects& runtime_fx_;
    crt_conversion_api::FileApiEffects& file_api_;
};
}
