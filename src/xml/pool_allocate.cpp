#include "preloader_l/xml/pool_allocate.hpp"
#include <bit>

namespace preloader_l::socket_recovered {
namespace {
std::uint64_t word(std::span<const std::uint8_t> bytes,unsigned at,unsigned width){
    std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[at+i]}<<(8*i);
    return value;
}
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width){
    return word(memory.view(address,width),0,width);
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(8*i));
}
std::array<std::uint64_t,2> xmm(ReceiveBufferMemory& memory,std::uintptr_t address){
    const auto bytes=memory.view(address,16);
    return {word(bytes,0,8),word(bytes,8,8)};
}
std::int32_t signed32(std::uint32_t value){return std::bit_cast<std::int32_t>(value);}
void increment(ReceiveBufferMemory& memory,std::uintptr_t address){
    const auto old=static_cast<std::uint32_t>(load(memory,address,4));
    store(memory,address,old+std::uint32_t{1},4);
}
struct Layout {unsigned stride,items,vector_links,constant_index;};
std::uintptr_t allocate_item(std::uintptr_t pool,PoolAllocationContext& context,Layout layout){
    auto& memory=context.memory;
    auto item=static_cast<std::uintptr_t>(load(memory,pool+0x68,8));
    if(!item){
        const std::uint64_t bytes=layout.stride*layout.items;
        item=context.effects.allocate_04b0bc(bytes);
        if(item)context.effects.fill_0793d0(item,0,bytes);
        // Even a null allocation is appended and then used by the native body.
        // Missing bounded mappings expose that prefix, not a new safe return.
        context.effects.append_block_005440(pool+8,item);
        const auto mode=static_cast<std::uint32_t>(load(memory,context.data.cpu_mode,4));
        unsigned i=0;
        if(signed32(mode)>=6){
            // MOVDQA loads occur once before stores. Retain both64-bit stride
            // lanes and the low twoDWORDs of each MOVQ source. PADDD wraps32,
            // PMOVSXDQ sign extends, and multiply/add wrap64. This is a bounded
            // byte-state projection, not a claim about native SIMD faults/ABI.
            const auto stride=xmm(memory,context.data.stride_constants[layout.constant_index]);
            const auto increments=xmm(memory,context.data.increment_constants);
            const auto indices=xmm(memory,context.data.index_constants);
            const std::array<std::uint32_t,2> add{
                static_cast<std::uint32_t>(increments[0]),static_cast<std::uint32_t>(increments[0]>>32)};
            const std::array<std::uint32_t,2> index{
                static_cast<std::uint32_t>(indices[0]),static_cast<std::uint32_t>(indices[0]>>32)};
            for(;i<layout.vector_links;i+=4){
                for(unsigned pair=0;pair<2;++pair){
                    const std::uint32_t first=i+pair*2;
                    for(unsigned lane=0;lane<2;++lane){
                        const std::uint32_t next=first+index[lane]+add[lane];
                        const auto extended=static_cast<std::uint64_t>(static_cast<std::int64_t>(signed32(next)));
                        const auto link=std::uint64_t{item}+extended*stride[lane];
                        store(memory,item+std::uintptr_t{(i+pair*2+lane)*layout.stride},link,8);
                    }
                }
            }
        }
        for(;i+1<layout.items;++i)
            store(memory,item+std::uintptr_t{i*layout.stride},item+std::uintptr_t{(i+1)*layout.stride},8);
        store(memory,item+std::uintptr_t{(layout.items-1)*layout.stride},0,8);
        store(memory,pool+0x68,item,8);
    }
    const auto next=load(memory,item,8);
    store(memory,pool+0x68,next,8);
    const auto current=static_cast<std::uint32_t>(load(memory,pool+0x70,4))+std::uint32_t{1};
    store(memory,pool+0x70,current,4);
    const auto maximum=static_cast<std::uint32_t>(load(memory,pool+0x78,4));
    if(signed32(current)>signed32(maximum))store(memory,pool+0x78,current,4);
    increment(memory,pool+0x74);
    increment(memory,pool+0x7c);
    return item;
}
}
std::uintptr_t allocate_pool_item_002b10(std::uintptr_t pool,PoolAllocationContext& context){return allocate_item(pool,context,{120,34,32,0});}
std::uintptr_t allocate_pool_item_002600(std::uintptr_t pool,PoolAllocationContext& context){return allocate_item(pool,context,{80,51,48,1});}
std::uintptr_t allocate_pool_item_002960(std::uintptr_t pool,PoolAllocationContext& context){return allocate_item(pool,context,{112,36,32,2});}
std::uintptr_t allocate_pool_item_0027b0(std::uintptr_t pool,PoolAllocationContext& context){return allocate_item(pool,context,{104,39,36,3});}
std::uintptr_t RecoveredPoolAllocationEffects::pool_allocate(std::uintptr_t target,std::uintptr_t pool){
    if(target==targets_.allocate[0])return allocate_pool_item_002b10(pool,context_);
    if(target==targets_.allocate[1])return allocate_pool_item_002600(pool,context_);
    if(target==targets_.allocate[2])return allocate_pool_item_002960(pool,context_);
    if(target==targets_.allocate[3])return allocate_pool_item_0027b0(pool,context_);
    throw UnsupportedInput("Pool allocation target is outside the selected source binding");
}
} // namespace preloader_l::socket_recovered
