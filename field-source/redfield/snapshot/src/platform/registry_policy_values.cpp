// SPDX-License-Identifier: GPL-3.0-only
#include "registry_policy_internal.hpp"
#include "registry_value_output.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#include <stdint.h>
#if defined(_WIN32)
extern "C" { __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int,unsigned long,const char*,int,uint16_t*,int); }
#endif
namespace preloader_l { namespace platform { namespace host_policy_detail {
namespace { constexpr unsigned int k_cp_utf8=65001u; }
int32_t write_a(preloader_l::config::ValueView value,const char* fallback,size_t fallback_n,
                uint32_t input_capacity,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    const char* src=value.present?value.data:fallback;const size_t n=value.present?value.size:fallback_n;
    return registry_value_output::write_a(src,n,{input_capacity,type,data,bytes});
}
int32_t write_w(preloader_l::config::ValueView value,const uint16_t* fallback,size_t fallback_n,
                uint32_t input_capacity,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    const uint16_t* src=fallback;size_t units=fallback_n;uint16_t* owned=nullptr;
#if defined(_WIN32)
    if(value.present){
        if(value.size>static_cast<size_t>(INT32_MAX))return host_error_invalid_parameter;
        if(value.size==0u){
            src=nullptr;units=0u;
        } else {
            const int need=MultiByteToWideChar(k_cp_utf8,0,value.data,static_cast<int>(value.size),nullptr,0);
            // Win32 reports conversion failure as zero.  A nonempty configured
            // value must never be reclassified as a successful empty REG_SZ.
            if(need<=0)return host_error_invalid_parameter;
            owned=static_cast<uint16_t*>(heap_alloc(static_cast<size_t>(need)*sizeof(uint16_t)));if(!owned)return host_error_invalid_parameter;
            if(MultiByteToWideChar(k_cp_utf8,0,value.data,static_cast<int>(value.size),owned,need)!=need){heap_free(owned);return host_error_invalid_parameter;}
            src=owned;units=static_cast<size_t>(need);
        }
    }
#else
    if(value.present)return host_error_invalid_parameter;
#endif
    const auto status=registry_value_output::write_w(src,units,{input_capacity,type,data,bytes});
    heap_free(owned);
    return status;
}
bool value_name_a(const char* name,preloader_l::config::ValueView policy) noexcept { return name&&policy.present&&eq_ci(name,cstr_len(name),policy.data,policy.size); }
bool value_name_w(const uint16_t* name,preloader_l::config::ValueView policy) noexcept { return name&&policy.present&&wide_eq_utf8_ci(name,policy.data,policy.size); }
} } }
