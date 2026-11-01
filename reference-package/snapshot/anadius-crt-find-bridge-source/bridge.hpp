#pragma once
#include "../anadius-crt-module-path-source/module_path.hpp"
#include "../anadius-crt-find-native-source/find_native.hpp"
#include "../anadius-crt-conversion-bridge-source/bridge.hpp"
namespace anadius::crt_find_bridge {
using Address=std::uint64_t;
class PathProvider {
public:
    virtual ~PathProvider()=default;
    // Owned per-invocation UTF16 scratch; successful path must be terminated
    // and readable for the fixed converter's two passes. Not a target address.
    virtual crt_module_path_recovered::WideResult get_module_filename_w(Address,std::uint32_t)=0;
};
class SdkPathProvider final:public PathProvider {
public:
    using Getter=decltype(&::GetModuleFileNameW);
    // Caller owns a live typed scratch array and serializes each invocation
    // through completion of module_filename_06ecdc; use separate arrays for overlap.
    explicit SdkPathProvider(std::span<WCHAR,264>,Getter=&::GetModuleFileNameW);
    crt_module_path_recovered::WideResult get_module_filename_w(Address,std::uint32_t) final;
private:
    std::span<WCHAR,264> scratch_;
    Getter getter_;
};
class PathEffects final:public crt_module_path_recovered::Effects {
public:
    PathEffects(crt_conversion_recovered::Effects&,crt_wildcard_recovered::PreviousEffects&,PathProvider&);
    std::uint8_t read_u8(Address) final;
    std::uint16_t read_u16(Address) final;
    void write_zero_u8(Address) final;
    void write_zero_u16(Address) final;
    Address malloc_06a308(std::uint64_t) final;
    void free_0668c0(Address) final;
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t) final;
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address) final;
    std::uint32_t last_error() final;
    void map_os_error_05d698(std::uint32_t) final;
    crt_errors_recovered::Cell32 errno_cell_05d724() final;
    crt_module_path_recovered::WideResult get_module_filename_w(Address,std::uint32_t) final;
    std::uint32_t choose_code_page() final;
private:
    crt_conversion_recovered::Effects& conversion_;
    crt_wildcard_recovered::PreviousEffects& locale_;
    PathProvider& provider_;
};
class Policy:public crt_wildcard_recovered::ExpansionPolicy {
public:
    Policy(crt_wildcard_recovered::Effects&,crt_module_path_recovered::Effects&);
    std::uint32_t module_filename_06ecdc(std::span<char,260>) final;
private:
    crt_module_path_recovered::Effects& path_;
};
class WildcardEffects:public crt_conversion_bridge::WildcardEffects {
public:
    WildcardEffects(crt_recalloc_bridge::Allocator&,crt_conversion_recovered::Effects&,
        crt_runtime_recovered::State&,crt_runtime_recovered::Effects&,
        crt_conversion_api::FileApiEffects&,crt_find_native::Session&);
    Address find_first(Address,Address&) final;
    std::int32_t find_next(Address,Address&) final;
    void find_close(Address) final;
private:
    crt_find_native::Session& find_;
};
}
