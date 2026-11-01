#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "../anadius-crt-nls-bridge-source/bridge.hpp"

namespace anadius::crt_nls_native {
using Address=crt_nls_bridge::Address;
using ByteOutput=crt_nls_bridge::ByteOutput;
using WordOutput=crt_nls_bridge::WordOutput;
using CPInfo=crt_nls_bridge::CPInfo;
struct Api {
    decltype(&::GetACP) get_acp;
    decltype(&::GetOEMCP) get_oemcp;
    decltype(&::IsValidCodePage) is_valid_code_page;
    decltype(&::GetCPInfo) get_cp_info;
    decltype(&::MultiByteToWideChar) multi_byte_to_wide;
    decltype(&::WideCharToMultiByte) wide_char_to_multi_byte;
    decltype(&::GetStringTypeW) get_string_type_w;
    decltype(&::GetLastError) get_last_error;
    decltype(&::SetLastError) set_last_error;
};
const Api& native_api() noexcept;
// Real source/scratch/default-character/used-default addresses must be live
// native storage. Tracked destination tokens are opaque and are never cast.
class Calls {
public:
    explicit Calls(const Api& api=native_api());
    std::uint32_t get_acp();
    std::uint32_t get_oemcp();
    std::uint32_t is_valid_code_page(std::uint32_t);
    std::uint32_t get_cp_info(std::uint32_t,CPInfo&);
    std::int32_t multi_byte_to_wide(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t);
    std::int32_t wide_char_to_multi_byte(std::uint32_t,std::uint32_t,Address,
        std::int32_t,Address,std::int32_t,Address,Address,std::span<ByteOutput>);
    std::uint32_t get_string_type_w(std::uint32_t,Address,std::int32_t,
        Address,std::span<WordOutput>);
private:
    Api api_;
};
}
