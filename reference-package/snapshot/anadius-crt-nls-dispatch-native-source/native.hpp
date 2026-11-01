#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "../anadius-crt-nls-dispatch-bridge-source/bridge.hpp"
namespace anadius::crt_nls_dispatch_native {
using Address=std::uint64_t;
using ByteOutput=crt_nls_dispatch_bridge::ByteOutput;
// SDK host pointers only. Dynamic Ex/Locale functions are supplied per call
// by the recovered resolver; this adapter does not load target DLLs.
struct Api {
    decltype(&::LCMapStringW) map_w;
    decltype(&::GetLastError) get_error;
    decltype(&::SetLastError) set_error;
};
Api system_api() noexcept;
class Calls final:public crt_nls_dispatch_bridge::NativeCalls {
public:
    explicit Calls(Api api=system_api());
    std::int32_t lc_map_string_ex(Address function,Address name,std::uint32_t flags,
        Address source,std::int32_t count,Address destination,std::int32_t capacity,
        Address version,Address reserved,Address sort_handle,std::span<ByteOutput>) final;
    std::int32_t lc_map_string_w(std::uint32_t lcid,std::uint32_t flags,
        Address source,std::int32_t count,Address destination,std::int32_t capacity,
        std::span<ByteOutput>) final;
    std::uint32_t locale_name_to_lcid(Address function,Address name,std::uint32_t flags) final;
private:
    Api api_;
};
}
