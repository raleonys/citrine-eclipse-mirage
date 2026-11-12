// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/pool_destruct.hpp"
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
void destroy_pool(std::uintptr_t pool,std::uintptr_t type_table,NativePoolDestructContext& context){
    auto& memory=context.memory;
    const bool enter=load(memory,pool+0x64,4)!=0;
    store(memory,pool,type_table,8);
    // Native CMP precedes the vtable store. Preserve its entry condition even
    // if aliased backing makes that store change the live count. Calling an
    // N20 clear body here would introduce a different initial count test.
    if(enter){
        do{
            const auto old=static_cast<std::uint32_t>(load(memory,pool+0x64,4));
            const auto index=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(old)));
            store(memory,pool+0x64,old-std::uint32_t{1},4);
            const auto data=static_cast<std::uintptr_t>(load(memory,pool+8,8));
            const auto block=static_cast<std::uintptr_t>(load(memory,data+(index<<3)-8,8));
            context.effects.free_04b0f8(block);
        }while(load(memory,pool+0x64,4)!=0);
    }
    store(memory,pool+0x68,0,8);
    store(memory,pool+0x70,0,8);
    store(memory,pool+0x78,0,8);
    const auto data=static_cast<std::uintptr_t>(load(memory,pool+8,8));
    if(data!=pool+0x10)context.effects.free_04b0f8(data);
    store(memory,pool,0x18007fa18ULL,8);
}
}
// Caller EDX block hints are4080/4056/4032/4080. Array-free callsites define
// RCX only. Preserve the existing pointer-only interface, not a sized-free ABI.
void destroy_native_pool_001bb0(std::uintptr_t pool,NativePoolDestructContext& context){destroy_pool(pool,0x18007fe38ULL,context);}
void destroy_native_pool_001c30(std::uintptr_t pool,NativePoolDestructContext& context){destroy_pool(pool,0x18007fe98ULL,context);}
void destroy_native_pool_001cb0(std::uintptr_t pool,NativePoolDestructContext& context){destroy_pool(pool,0x18007fe68ULL,context);}
void destroy_native_pool_001d30(std::uintptr_t pool,NativePoolDestructContext& context){destroy_pool(pool,0x18007fe08ULL,context);}
void PoolDestructEffects::destroy_pool_001bb0(std::uintptr_t pool){destroy_native_pool_001bb0(pool,pool_destruct_);}
void PoolDestructEffects::destroy_pool_001c30(std::uintptr_t pool){destroy_native_pool_001c30(pool,pool_destruct_);}
void PoolDestructEffects::destroy_pool_001cb0(std::uintptr_t pool){destroy_native_pool_001cb0(pool,pool_destruct_);}
void PoolDestructEffects::destroy_pool_001d30(std::uintptr_t pool){destroy_native_pool_001d30(pool,pool_destruct_);}
} // namespace citrine_eclipse_mirage::socket
