#pragma once
#include "../anadius-crt-nls-dispatch-source/nls_dispatch.hpp"
#include "../anadius-crt-locale-names-source/locale_names.hpp"
#include "../anadius-crt-nls-bridge-source/bridge.hpp"
namespace anadius::crt_nls_dispatch_bridge {
using Address=std::uint64_t;
using ByteOutput=crt_nls_bridge::ByteOutput;
using WordOutput=crt_nls_bridge::WordOutput;
using CPInfo=crt_nls_bridge::CPInfo;
class NativeCalls {
public:
    virtual ~NativeCalls()=default;
    virtual std::int32_t lc_map_string_ex(Address function,Address name,std::uint32_t flags,
        Address source,std::int32_t count,Address destination,std::int32_t capacity,
        Address version,Address reserved,Address sort_handle,std::span<ByteOutput>)=0;
    virtual std::int32_t lc_map_string_w(std::uint32_t lcid,std::uint32_t flags,
        Address source,std::int32_t count,Address destination,std::int32_t capacity,
        std::span<ByteOutput>)=0;
    virtual std::uint32_t locale_name_to_lcid(Address function,Address name,std::uint32_t flags)=0;
};
// The prior native backend still supplies CP/conversion/type/scratch APIs.
// Its old lc_map_string_0671d4 slot is never called by this adapter.
class Effects final:public crt_nls_bridge::NativeEffects,
    private crt_nls_dispatch_recovered::Effects,private crt_locale_names_recovered::Effects {
public:
    Effects(crt_nls_bridge::NativeEffects&,NativeCalls&,crt_runtime_recovered::State&,
        crt_runtime_recovered::Effects&,crt_multibyte_record_bridge::Memory&);
    std::uint32_t get_acp() final;
    std::uint32_t get_oemcp() final;
    std::uint32_t exchange_32(Address,std::uint32_t) final;
    std::uint32_t is_valid_code_page(std::uint32_t) final;
    std::uint32_t get_cp_info(std::uint32_t,CPInfo&) final;
    std::uint64_t begin_scratch_frame() final;
    void end_scratch_frame(std::uint64_t) noexcept final;
    Address stack_allocate(std::uint64_t) final;
    std::uint8_t read_input_tail(Address) final;
    std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::span<ByteOutput>) final;
    std::uint32_t get_string_type_w(std::uint32_t,Address,std::int32_t,Address,std::span<WordOutput>) final;
    std::int32_t lc_map_string_0671d4(Address,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::uint64_t,std::span<ByteOutput>) final;
    std::uint32_t locale_name_to_lcid_0672c0(Address,std::uint32_t);
private:
    crt_nls_bridge::NativeEffects& backend_;
    NativeCalls& native_;
    crt_runtime_recovered::State& runtime_;
    crt_runtime_recovered::Effects& resolver_;
    crt_multibyte_record_bridge::Memory& memory_;
    std::span<ByteOutput> output_{};
    Address resolve_0669b0(std::uint32_t,std::string_view,std::span<const std::uint32_t>) final;
    std::int32_t call_lc_map_string_ex(Address,Address,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,Address) final;
    std::int32_t call_lc_map_string_w(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::uint32_t call_locale_name_to_lcid(Address,Address,std::uint32_t) final;
    std::uint32_t fallback_locale_name_07310c(Address) final;
    std::uint16_t read_u16(Address) final;
};
}
