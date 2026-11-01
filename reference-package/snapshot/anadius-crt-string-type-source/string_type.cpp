#include "string_type.hpp"

namespace anadius::crt_string_type_recovered {
std::uint32_t get_string_type_06e210(Address locale,std::uint32_t type,
    Address source,std::int32_t count,Address output,std::uint32_t codepage,
    std::int32_t strict,Effects& effects) {
    const auto context=effects.locale_update_058b04(locale);
    if(codepage==0) codepage=effects.read_u32(context.locale_info+0xc);
    const auto query=effects.multibyte_to_wide_06ccc8(codepage,
        strict!=0 ? 9U : 1U,source,count,0,0);
    std::uint32_t result=0;
    Address wide=0;
    if(query!=0) {
        const auto bytes=static_cast<std::uint64_t>(static_cast<std::int64_t>(query))*2U;
        const auto added=bytes+16U;
        const auto total=bytes<added ? added : 0;
        if(total!=0) {
            Address base=0;
            if(total<=1024U) {
                auto rounded=total+15U;
                if(rounded<=total) rounded=0x0ffffffffffffff0ULL;
                rounded&=~std::uint64_t{15};
                base=effects.stack_allocate(rounded);
                if(base!=0) effects.write_u32(base,0xccccU);
            } else {
                base=effects.malloc_06a308(total);
                if(base!=0) effects.write_u32(base,0xddddU);
            }
            if(base!=0) wide=base+16U;
            if(wide!=0) {
                effects.zero_bytes(wide,bytes);
                const auto converted=effects.multibyte_to_wide_06ccc8(codepage,
                    1,source,count,wide,query);
                if(converted!=0)
                    result=effects.native_get_string_type_w(type,wide,converted,output);
            }
        }
        if(wide!=0 && effects.read_u32(wide-16U)==0xddddU)
            effects.free_0668c0(wide-16U);
    }
    if(context.clear_thread_flag!=0) {
        const auto flags=context.ptd+0x3a8;
        effects.write_u32(flags,effects.read_u32(flags)&0xfffffffdU);
    }
    return result;
}
}
