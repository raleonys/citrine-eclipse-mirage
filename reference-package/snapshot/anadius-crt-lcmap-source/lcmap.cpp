#include "lcmap.hpp"
#include <bit>

namespace anadius::crt_lcmap_recovered {
namespace {
Address allocate_wide(std::int32_t count,Effects& e) {
    const auto twice=static_cast<std::uint64_t>(static_cast<std::int64_t>(count))*2u;
    const auto size=twice+16u;
    if (!(twice<size)) return 0;
    Address base=0;
    if(size<=1024) {
        auto rounded=size+15u;
        if(rounded<=size) rounded=UINT64_C(0x0ffffffffffffff0);
        base=e.stack_allocate(rounded&~UINT64_C(15));
        if(base!=0) e.write_u32(base,0xcccc);
    } else {
        base=e.malloc_06a308(size);
        if(base!=0) e.write_u32(base,0xdddd);
    }
    return base==0?0:base+16u;
}
void release(Address data,Effects& e) {
    if(data!=0 && e.read_u32(data-16u)==0xdddd) e.free_0668c0(data-16u);
}
std::int32_t bounded_count(Address source,std::int32_t count,Effects& e) {
    auto length=static_cast<std::uint32_t>(strncnt_063168(source,static_cast<std::uint32_t>(count),e));
    if(length<static_cast<std::uint32_t>(count)) ++length;
    return std::bit_cast<std::int32_t>(length);
}
}
std::uint64_t strncnt_063168(Address source,std::uint64_t count,Effects& e) {
    std::uint64_t length=0;
    auto byte=e.read_u8(source);
    while(byte!=0 && length!=count) {
        ++length;
        byte=e.read_u8(source+length);
    }
    return length;
}
std::uint32_t lc_map_inner_06c8fc(const Context& locale,Address name,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity,
    std::uint32_t cp,std::int32_t invalid,Effects& e) {
    if(count>0) count=bounded_count(source,count,e);
    if(cp==0) cp=e.read_u32(locale.locale_info+12u);
    const auto wide_count=e.multi_byte_to_wide_06ccc8(cp,invalid!=0?9u:1u,source,count,0,0);
    if(wide_count==0) return 0;
    const auto first=allocate_wide(wide_count,e);
    if(first==0) return 0;
    std::uint32_t result=0;
    if(e.multi_byte_to_wide_06ccc8(cp,1,source,count,first,wide_count)!=0) {
        const auto map_count=e.lc_map_string_0671d4(name,flags,first,wide_count,0,0,0,0,0);
        if(map_count!=0) {
            if((flags&0x400u)!=0) {
                if(capacity==0) result=static_cast<std::uint32_t>(map_count);
                else if(map_count<=capacity)
                    result=static_cast<std::uint32_t>(e.lc_map_string_0671d4(
                        name,flags,first,wide_count,destination,capacity,0,0,0));
            } else {
                const auto second=allocate_wide(map_count,e);
                if(second!=0) {
                    if(e.lc_map_string_0671d4(name,flags,first,wide_count,second,map_count,0,0,0)!=0)
                        result=static_cast<std::uint32_t>(e.wide_char_to_multi_byte_06cd58(
                            cp,0,second,map_count,capacity==0?0:destination,capacity,0,0));
                    release(second,e);
                }
            }
        }
    }
    release(first,e);
    return result;
}
std::uint32_t lc_map_string_06cc30(Address locale,Address name,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity,
    std::uint32_t cp,std::int32_t invalid,Effects& e) {
    const auto context=e.locale_update_058b04(locale);
    const auto result=lc_map_inner_06c8fc(context,name,flags,source,count,destination,capacity,cp,invalid,e);
    if(context.clear_thread_flag!=0) {
        const auto address=context.ptd+0x3a8u;
        e.write_u32(address,e.read_u32(address)&~2u);
    }
    return result;
}
}
