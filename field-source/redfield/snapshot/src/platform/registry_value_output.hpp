// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/platform/registry_policy.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l::platform::registry_value_output {

// Borrowed for one synchronous policy call. The capacity is captured before
// forwarding to the original API; *bytes may already contain its result.
// data is byte storage with no uint16_t alignment or object-type requirement.
struct Output {
    uint32_t capacity;
    uint32_t* type;
    uint8_t* data;
    uint32_t* bytes;
};

namespace detail {
inline int32_t prepare(const void* source,size_t length,uint32_t width,
                       Output output) noexcept {
    if ((!source && length) || (output.data && !output.bytes) ||
        length > (static_cast<size_t>(UINT32_MAX)-width)/width)
        return host_error_invalid_parameter;
    const auto required=static_cast<uint32_t>((length+1u)*width);
    if (output.type) *output.type=host_reg_sz;
    if (!output.data || output.capacity<required) {
        if (output.bytes) *output.bytes=required;
        return output.data ? host_error_more_data : host_error_success;
    }
    return host_error_success;
}
}

inline int32_t write_a(const char* source,size_t length,Output output) noexcept {
    const auto status=detail::prepare(source,length,1u,output);
    if (status!=host_error_success || !output.data) return status;
    for (size_t i=0;i<length;++i) output.data[i]=static_cast<uint8_t>(source[i]);
    output.data[length]=0;
    *output.bytes=static_cast<uint32_t>(length+1u);
    return host_error_success;
}

inline int32_t write_w(const uint16_t* source,size_t units,Output output) noexcept {
    const auto status=detail::prepare(source,units,2u,output);
    if (status!=host_error_success || !output.data) return status;
    // Win32 REG_SZ is UTF-16LE. Do not create uint16_t lvalues in the caller's
    // BYTE buffer: it may be unaligned or belong to a different object type.
    for (size_t i=0;i<units;++i) {
        output.data[2u*i]=static_cast<uint8_t>(source[i]&0xffu);
        output.data[2u*i+1u]=static_cast<uint8_t>(source[i]>>8u);
    }
    output.data[2u*units]=0;
    output.data[2u*units+1u]=0;
    *output.bytes=static_cast<uint32_t>((units+1u)*2u);
    return host_error_success;
}

} // namespace preloader_l::platform::registry_value_output
