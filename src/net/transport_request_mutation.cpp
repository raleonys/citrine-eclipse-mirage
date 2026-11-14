// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_request_mutation.hpp"
#include <algorithm>

namespace laureondo::socket {
namespace {
std::uintptr_t address(const ReceiveBufferString& value,std::uint64_t capacity){
    if(capacity<=15)return reinterpret_cast<std::uintptr_t>(value.storage_00.data());
    std::uint64_t result=0;for(unsigned i=0;i<8;++i)result|=std::uint64_t{value.storage_00[i]}<<(i*8);return static_cast<std::uintptr_t>(result);
}
std::uint8_t nibble(std::uint8_t value){
    if(value>='0'&&value<='9')return static_cast<std::uint8_t>(value-'0');
    if(value>='A'&&value<='F')return static_cast<std::uint8_t>(value-'A'+10);
    if(value>='a'&&value<='f')return static_cast<std::uint8_t>(value-'a'+10);
    return 0;
}
}
void decode_request_hex_03ce30(ReceiveBufferString& value,ReceiveBufferContext& context){
    const auto half=value.size_10>>1;
    for(std::uint64_t i=0;i<half;++i){
        const auto capacity=value.capacity_18;
        const auto high=context.memory.view(address(value,capacity)+static_cast<std::uintptr_t>(i*2),1)[0];
        const auto low=context.memory.view(address(value,capacity)+static_cast<std::uintptr_t>(i*2+1),1)[0];
        context.memory.view(address(value,capacity)+static_cast<std::uintptr_t>(i),1)[0]=static_cast<std::uint8_t>((nibble(high)<<4)|nibble(low));
    }
    resize_receive_buffer_010040(value,half,0,context); // native tail JMP03cefb
}
ReceiveBufferString& replace_request_034730(ReceiveBufferString& value,std::uint64_t offset,
    std::uint64_t requested_erase,std::uintptr_t source,std::uint64_t insert,RequestMutationContext& context){
    const auto old_size=value.size_10;
    if(old_size<offset)context.api.out_of_range_00cec0();
    const auto remaining=old_size-offset,erase=std::min(remaining,requested_erase);
    if(erase==insert){
        const auto destination=receive_buffer_address(value)+static_cast<std::uintptr_t>(offset);
        context.buffer.api.copy_bytes(destination,source,insert);
        return value;
    }
    const auto tail=remaining-erase;
    if(insert<erase){
        const auto destination=receive_buffer_address(value)+static_cast<std::uintptr_t>(offset);
        context.buffer.api.copy_bytes(destination,source,insert);
        context.buffer.api.copy_bytes(destination+static_cast<std::uintptr_t>(insert),
            destination+static_cast<std::uintptr_t>(erase),tail+std::uint64_t{1});
        value.size_10=old_size-erase+insert; // cached old-size, AFTER both copies
        return value;
    }
    const auto growth=insert-erase,capacity=value.capacity_18;
    if(growth>capacity-old_size)
        return context.api.grow_replace_02b940(value,growth,0,offset,erase,source,insert);
    value.size_10=old_size+growth; //03480b before suffix move and alias handling
    const auto base=address(value,capacity),destination=base+static_cast<std::uintptr_t>(offset);
    const auto suffix=destination+static_cast<std::uintptr_t>(erase);
    auto first=insert;
    if(source+static_cast<std::uintptr_t>(insert)>destination && source<=base+static_cast<std::uintptr_t>(old_size))
        first=suffix<=source?0:static_cast<std::uint64_t>(suffix-source);
    context.buffer.api.copy_bytes(suffix+static_cast<std::uintptr_t>(growth),suffix,tail+std::uint64_t{1});
    context.buffer.api.copy_bytes(destination,source,first);
    context.buffer.api.copy_bytes(destination+static_cast<std::uintptr_t>(first),
        source+static_cast<std::uintptr_t>(first+growth),insert-first);
    return value;
}
} // namespace laureondo::socket
