#pragma once
#include "../anadius-crt-string-type-source/string_type.hpp"
#include "../anadius-crt-lcmap-source/lcmap.hpp"
#include "../anadius-crt-multibyte-record-bridge-source/bridge.hpp"
namespace anadius::crt_nls_bridge {
using Address=std::uint64_t;
using WordOutput=crt_multibyte_tables_recovered::WordOutput;
using ByteOutput=crt_multibyte_tables_recovered::ByteOutput;
using CPInfo=crt_multibyte_tables_recovered::CPInfo;
class Memory:public crt_multibyte_record_bridge::Memory {
public:
    virtual std::uint64_t borrow_readable(Address,std::uint64_t)=0;
    virtual void release_readable(std::uint64_t) noexcept=0;
};
class NativeEffects {
public:
    virtual ~NativeEffects()=default;
    virtual std::uint32_t get_acp()=0;
    virtual std::uint32_t get_oemcp()=0;
    virtual std::uint32_t exchange_32(Address,std::uint32_t)=0;
    virtual std::uint32_t is_valid_code_page(std::uint32_t)=0;
    virtual std::uint32_t get_cp_info(std::uint32_t,CPInfo&)=0;
    virtual std::uint64_t begin_scratch_frame()=0;
    virtual void end_scratch_frame(std::uint64_t) noexcept=0;
    virtual Address stack_allocate(std::uint64_t rounded_bytes)=0;
    // __strncnt touches input[count]. This byte is outside the logical input
    // span and must be supplied as known bytes or rejected by the provider.
    virtual std::uint8_t read_input_tail(Address)=0;
    virtual std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t)=0;
    virtual std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t,Address,Address,std::span<ByteOutput>)=0;
    virtual std::uint32_t get_string_type_w(std::uint32_t,Address,std::int32_t,
        Address,std::span<WordOutput>)=0;
    // The 0671d4 resolver/dispatch remains required. A nonempty tracked span
    // represents byte output (sort-key route); ordinary UTF16 scratch is raw.
    virtual std::int32_t lc_map_string_0671d4(Address,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t,Address,Address,std::uint64_t,
        std::span<ByteOutput>)=0;
};
struct Dependencies {
    Memory& memory;
    crt_locale_update_recovered::Effects& locale;
    crt_heap_recovered::State& heap;
    crt_realloc_recovered::Effects& heap_fx;
    NativeEffects& native;
};
// Replaces both previous lower-wrapper providers with recovered source.
// Borrowed dependencies must share one synchronized, live address domain.
class Effects final:public crt_multibyte_record_bridge::NativeEffects,
    private crt_string_type_recovered::Effects,private crt_lcmap_recovered::Effects,
    private crt_conversion_api::Effects {
public:
    explicit Effects(Dependencies);
    std::uint32_t get_acp() final;
    std::uint32_t get_oemcp() final;
    std::uint32_t exchange_32(Address,std::uint32_t) final;
    std::uint32_t is_valid_code_page(std::uint32_t) final;
    std::uint32_t get_cp_info(std::uint32_t,CPInfo&) final;
    std::uint32_t get_string_type_06e210(Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<WordOutput>,std::uint32_t,std::uint32_t) final;
    std::uint32_t map_string_06cc30(Address,Address,std::uint32_t,
        std::span<const std::uint8_t>,std::span<ByteOutput>,std::uint32_t,std::uint32_t) final;
private:
    struct Invocation;
    Dependencies d_;
    std::span<const std::uint8_t> input_{};
    std::span<WordOutput> words_{};
    std::span<ByteOutput> bytes_{};
    Address output_{};
    crt_byteclass_recovered::LocaleContext locale_update_058b04(Address) final;
    std::uint8_t read_u8(Address) final;
    std::uint32_t read_u32(Address) final;
    void write_u32(Address,std::uint32_t) final;
    Address malloc_06a308(std::uint64_t) final;
    void free_0668c0(Address) final;
    Address stack_allocate(std::uint64_t) final;
    void zero_bytes(Address,std::uint64_t) final;
    std::int32_t multibyte_to_wide_06ccc8(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address) final;
    std::uint32_t native_get_string_type_w(std::uint32_t,Address,std::int32_t,Address) final;
    std::int32_t lc_map_string_0671d4(Address,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::uint64_t) final;
    std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address) final;
    void write_bool32(Address,std::int32_t) final;
};
}
