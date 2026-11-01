#include "registry_policy_internal.hpp"
#include "preloader_l/platform/registry_policy.hpp"
#include <stdint.h>
#if defined(_WIN32)
extern "C" { __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int,unsigned long,const char*,int,uint16_t*,int); }
#endif
namespace preloader_l { namespace platform { namespace host_policy_detail {
namespace { constexpr unsigned int k_cp_utf8=65001u; void normalize_type(uint32_t* type) noexcept { if(type)*type=host_reg_sz; } }
int32_t write_a(preloader_l::config::ValueView value,const char* fallback,size_t fallback_n,
                uint32_t input_capacity,uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    const char* src=value.present?value.data:fallback;const size_t n=value.present?value.size:fallback_n;
    if((!src&&n)||(data&&!bytes)||n>static_cast<size_t>(UINT32_MAX)-1u)return host_error_invalid_parameter;
    const uint32_t required=static_cast<uint32_t>(n+1u);normalize_type(type);
    if(!data){if(bytes)*bytes=required;return host_error_success;}
    if(input_capacity<required){*bytes=required;return host_error_more_data;}
    for (size_t i = 0; i < n; ++i) data[i] = static_cast<uint8_t>(src[i]);
    data[n] = 0;
    *bytes = required;
    return host_error_success;
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
    if((!src&&units)||(data&&!bytes)||units>(static_cast<size_t>(UINT32_MAX)-2u)/2u){heap_free(owned);return host_error_invalid_parameter;}
    const uint32_t required=static_cast<uint32_t>((units+1u)*2u);normalize_type(type);
    if(!data){if(bytes)*bytes=required;heap_free(owned);return host_error_success;}
    if(input_capacity<required){*bytes=required;heap_free(owned);return host_error_more_data;}
    auto* out=reinterpret_cast<uint16_t*>(data);for(size_t i=0;i<units;++i)out[i]=src[i];out[units]=0;*bytes=required;heap_free(owned);return host_error_success;
}
bool value_name_a(const char* name,preloader_l::config::ValueView policy) noexcept { return name&&policy.present&&eq_ci(name,cstr_len(name),policy.data,policy.size); }
bool value_name_w(const uint16_t* name,preloader_l::config::ValueView policy) noexcept { return name&&policy.present&&wide_eq_utf8_ci(name,policy.data,policy.size); }
} } }
