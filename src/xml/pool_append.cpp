// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/pool_append.hpp"
#include <bit>
#include <limits>

namespace citrine_eclipse_mirage::socket {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(8*i);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(8*i));
}
std::int32_t signed32(std::uint32_t value){return std::bit_cast<std::int32_t>(value);}
std::uint64_t extended32(std::uint32_t value){
    return static_cast<std::uint64_t>(static_cast<std::int64_t>(signed32(value)));
}
}
void append_pool_block_005440(std::uintptr_t array,std::uintptr_t block,PoolAppendContext& context){
    auto& memory=context.memory;
    const auto next=static_cast<std::uint32_t>(load(memory,array+0x5c,4))+std::uint32_t{1};
    const auto capacity=static_cast<std::uint32_t>(load(memory,array+0x58,4));
    if(signed32(next)>signed32(capacity)){
        const std::uint32_t doubled=next+next;
        const auto elements=extended32(doubled);
        constexpr auto maximum=std::numeric_limits<std::uint64_t>::max();
        const auto bytes=elements>maximum/8?maximum:elements*8;
        const auto data=context.effects.allocate_04b100(bytes);
        // The native body rereads count/data after allocation, and data again
        // after copy. Lower effects may mutate them; no prefetch or rollback.
        const auto copy_count=static_cast<std::uint32_t>(load(memory,array+0x5c,4));
        const auto old_data=static_cast<std::uintptr_t>(load(memory,array,8));
        context.effects.copy_078d30(data,old_data,extended32(copy_count)<<3);
        const auto current_data=static_cast<std::uintptr_t>(load(memory,array,8));
        if(current_data!=array+8)context.effects.free_04b0f8(current_data);
        store(memory,array,data,8);
        store(memory,array+0x58,doubled,4);
    }
    const auto count=static_cast<std::uint32_t>(load(memory,array+0x5c,4));
    const auto data=static_cast<std::uintptr_t>(load(memory,array,8));
    store(memory,data+(extended32(count)<<3),block,8);
    // The slot can alias count. INC reads its live value after the slot store.
    const auto live_count=static_cast<std::uint32_t>(load(memory,array+0x5c,4));
    store(memory,array+0x5c,live_count+std::uint32_t{1},4);
}
void PoolAppendEffectsAdapter::append_block_005440(std::uintptr_t array,std::uintptr_t block){
    PoolAppendContext context{memory_,*this};
    append_pool_block_005440(array,block,context);
}
} // namespace citrine_eclipse_mirage::socket
