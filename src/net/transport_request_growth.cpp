// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_request_growth.hpp"
#include <algorithm>

namespace citrine_eclipse_mirage::socket {
namespace {
void word(std::span<std::uint8_t> bytes,std::uint64_t value){for(unsigned i=0;i<8;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));}
std::uint64_t word(std::span<const std::uint8_t> bytes){std::uint64_t value=0;for(unsigned i=0;i<8;++i)value|=std::uint64_t{bytes[i]}<<(i*8);return value;}
std::uintptr_t aligned(std::uint64_t count,ReceiveBufferContext& context){
    const auto raw=context.api.allocate_04b0bc(count);if(!raw)context.api.invalid_parameter_05c974();
    const auto data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};word(context.memory.view(data-std::uintptr_t{8},8),raw);return data;
}
}
ReceiveBufferString& grow_replace_request_02b940(ReceiveBufferString& value,std::uint64_t extra,
    [[maybe_unused]]std::uint64_t unused_hint,std::uint64_t offset,std::uint64_t erase,
    std::uintptr_t source,std::uint64_t insert,ReceiveBufferContext& context){
    constexpr std::uint64_t maximum=0x7fffffffffffffffULL;
    const auto old_size=value.size_10;
    if(maximum-old_size<extra)context.api.length_error_00ce80();
    const auto old_capacity=value.capacity_18,new_size=old_size+extra,rounded=new_size|std::uint64_t{15};
    auto capacity=maximum;std::uintptr_t data=0;
    if(rounded<=maximum && old_capacity<=maximum-(old_capacity>>1)){
        capacity=std::max(rounded,old_capacity+(old_capacity>>1));const auto bytes=capacity+std::uint64_t{1};
        if(bytes>=4096){const auto count=bytes+std::uint64_t{39};if(count<=bytes)context.api.allocation_overflow_00cd30();data=aligned(count,context);}
        else if(bytes)data=context.api.allocate_04b0bc(bytes);
    }else data=aligned(0x8000000000000027ULL,context);
    const auto tail=old_size-offset-erase+std::uint64_t{1};
    value.capacity_18=capacity;value.size_10=new_size; //02ba1a/1e before COPY
    const auto insertion=data+static_cast<std::uintptr_t>(offset);
    if(old_capacity<=15){
        const auto prefix=reinterpret_cast<std::uintptr_t>(value.storage_00.data());
        context.api.copy_bytes(data,prefix,offset);
        context.api.copy_bytes(insertion,source,insert);
        context.api.copy_bytes(insertion+static_cast<std::uintptr_t>(insert),prefix+static_cast<std::uintptr_t>(offset+erase),tail);
    }else{
        const auto old_data=static_cast<std::uintptr_t>(word(value.storage_00)); // live after allocation/metadata
        context.api.copy_bytes(data,old_data,offset);
        context.api.copy_bytes(insertion,source,insert);
        context.api.copy_bytes(insertion+static_cast<std::uintptr_t>(insert),old_data+static_cast<std::uintptr_t>(offset+erase),tail);
        auto bytes=old_capacity+std::uint64_t{1};auto base=old_data;
        if(bytes>=4096){
            base=static_cast<std::uintptr_t>(word(context.memory.view(old_data-std::uintptr_t{8},8)));bytes+=39;
            if(old_data-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        }
        context.api.free_sized_04b0f8(base,bytes);
    }
    word(value.storage_00,data); //02bacd final pointer publication after old free
    return value;
}
ReceiveBufferString& RequestGrowthApi::grow_replace_02b940(ReceiveBufferString& value,
    std::uint64_t extra,std::uint64_t hint,std::uint64_t offset,std::uint64_t erase,
    std::uintptr_t source,std::uint64_t insert){
    return grow_replace_request_02b940(value,extra,hint,offset,erase,source,insert,context_);
}
} // namespace citrine_eclipse_mirage::socket
