// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/pool_clear.hpp"
#include <bit>

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
void clear_blocks(std::uintptr_t pool,NativePoolClearContext& context){
    auto& memory=context.memory;
    while(load(memory,pool+0x64,4)!=0){
        const auto old=static_cast<std::uint32_t>(load(memory,pool+0x64,4));
        const auto index=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(old)));
        store(memory,pool+0x64,old-std::uint32_t{1},4);
        const auto data=static_cast<std::uintptr_t>(load(memory,pool+8,8));
        const auto block=static_cast<std::uintptr_t>(load(memory,data+(index<<3)-8,8));
        context.effects.free_04b0f8(block);
    }
    // Three native QWORD stores, in order. Keep their access granularity and
    // completed prefix; no separate final count write or array storage reset.
    store(memory,pool+0x68,0,8);
    store(memory,pool+0x70,0,8);
    store(memory,pool+0x78,0,8);
}
}
// Retained EDX hints:4080,4056,4032 respectively. The shared existing lower
// interface projects RCX only; no register/sized-free ABI equivalence claimed.
void clear_pool_blocks_002d70(std::uintptr_t pool,NativePoolClearContext& context){clear_blocks(pool,context);}
void clear_pool_blocks_002dc0(std::uintptr_t pool,NativePoolClearContext& context){clear_blocks(pool,context);}
void clear_pool_blocks_002e10(std::uintptr_t pool,NativePoolClearContext& context){clear_blocks(pool,context);}
void PoolClearEffects::clear_pool_002d70(std::uintptr_t pool){clear_pool_blocks_002d70(pool,pool_clear_);}
void PoolClearEffects::clear_pool_002dc0(std::uintptr_t pool){clear_pool_blocks_002dc0(pool,pool_clear_);}
void PoolClearEffects::clear_pool_002e10(std::uintptr_t pool){clear_pool_blocks_002e10(pool,pool_clear_);}
} // namespace citrine_eclipse_mirage::socket
