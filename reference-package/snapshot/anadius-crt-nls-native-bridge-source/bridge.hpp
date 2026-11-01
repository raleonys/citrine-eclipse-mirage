#pragma once
#include "../anadius-crt-nls-native-source/native_api.hpp"
#include "../anadius-crt-nls-dispatch-native-source/native.hpp"
namespace anadius::crt_nls_native_bridge {
using Address=crt_nls_bridge::Address;
using ByteOutput=crt_nls_bridge::ByteOutput;
using WordOutput=crt_nls_bridge::WordOutput;
using CPInfo=crt_nls_bridge::CPInfo;
// Independent caller-owned domain. Native NLS APIs do not establish original
// PTD/global/heap/stack lifetimes or knowledge of adjacent input bytes.
class ContextEffects {
public:
    virtual ~ContextEffects()=default;
    virtual std::uint32_t exchange_32(Address,std::uint32_t)=0;
    virtual std::uint64_t begin_scratch_frame()=0;
    virtual void end_scratch_frame(std::uint64_t) noexcept=0;
    virtual Address stack_allocate(std::uint64_t)=0;
    virtual std::uint8_t read_input_tail(Address)=0;
};
// Supplies typed native CP/conversion/type APIs to the recovered dispatch/NLS
// composition. Use dispatch_native::Calls for its separate NativeCalls input.
class Backend final:public crt_nls_bridge::NativeEffects {
public:
    Backend(crt_nls_native::Calls&,ContextEffects&);
    std::uint32_t get_acp() final;
    std::uint32_t get_oemcp() final;
    std::uint32_t is_valid_code_page(std::uint32_t) final;
    std::uint32_t get_cp_info(std::uint32_t,CPInfo&) final;
    std::uint32_t exchange_32(Address,std::uint32_t) final;
    std::uint64_t begin_scratch_frame() final;
    void end_scratch_frame(std::uint64_t) noexcept final;
    Address stack_allocate(std::uint64_t) final;
    std::uint8_t read_input_tail(Address) final;
    std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::span<ByteOutput>) final;
    std::uint32_t get_string_type_w(std::uint32_t,Address,std::int32_t,Address,std::span<WordOutput>) final;
    // This slot belongs to the outer recovered dispatch composition, which
    // must be installed; it is never a synthetic success fallback.
    std::int32_t lc_map_string_0671d4(Address,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::uint64_t,std::span<ByteOutput>) final;
private:
    crt_nls_native::Calls& native_;
    ContextEffects& context_;
};
}
