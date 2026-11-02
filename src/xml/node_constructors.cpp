// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/node_constructors.hpp"

namespace preloader_l::socket {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address){
    const auto bytes=memory.view(address,8);std::uint64_t value=0;
    for(unsigned i=0;i<8;++i)value|=std::uint64_t{bytes[i]}<<(8*i);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width=8){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(8*i));
}
std::uintptr_t construct(std::uintptr_t document,std::uintptr_t pool,unsigned kind,
    NativeNodeConstructorContext& context){
    auto& memory=context.memory;const auto table_id=context.identities.vtables[kind];
    const auto pool_table=load(memory,pool),allocation_target=load(memory,pool_table+0x10);
    const auto node=context.allocator.pool_allocate(allocation_target,pool);
    if(kind!=2)store(memory,node+8,document);
    store(memory,node+0x10,0);
    store(memory,node+0x18,0,4);
    store(memory,node+0x20,0);store(memory,node+0x28,0);
    store(memory,node+0x30,0,4);
    for(const auto offset:{0x38U,0x40U,0x48U,0x50U,0x58U})store(memory,node+offset,0);
    if(kind==2){
        store(memory,node+8,document);
        store(memory,node+0x68,0,4);
        store(memory,node+0x70,0);
    }
    store(memory,node,table_id);
    if(kind==3)store(memory,node+0x68,0,1);
    store(memory,node+0x60,pool);
    PoolAppendContext append{memory,context.append};
    append_pool_block_005440(document+0xa8,node,append);
    return node;
}
}
std::uintptr_t construct_node_001590(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext& c){return construct(document,pool,0,c);}
std::uintptr_t construct_node_001610(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext& c){return construct(document,pool,1,c);}
std::uintptr_t construct_node_001690(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext& c){return construct(document,pool,2,c);}
std::uintptr_t construct_node_001720(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext& c){return construct(document,pool,3,c);}
std::uintptr_t construct_node_0017b0(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext& c){return construct(document,pool,4,c);}
std::uintptr_t NodeConstructorEffects::construct_001590(std::uintptr_t document,std::uintptr_t pool){return construct_node_001590(document,pool,constructors_);}
std::uintptr_t NodeConstructorEffects::construct_001610(std::uintptr_t document,std::uintptr_t pool){return construct_node_001610(document,pool,constructors_);}
std::uintptr_t NodeConstructorEffects::construct_001690(std::uintptr_t document,std::uintptr_t pool){return construct_node_001690(document,pool,constructors_);}
std::uintptr_t NodeConstructorEffects::construct_001720(std::uintptr_t document,std::uintptr_t pool){return construct_node_001720(document,pool,constructors_);}
std::uintptr_t NodeConstructorEffects::construct_0017b0(std::uintptr_t document,std::uintptr_t pool){return construct_node_0017b0(document,pool,constructors_);}
} // namespace preloader_l::socket
