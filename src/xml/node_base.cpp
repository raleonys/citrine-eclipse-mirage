// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/xml/node_base.hpp"

namespace laureondo::socket {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width=8){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(8*i);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width=8){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(8*i));
}
}
void destroy_native_node_base_001ea0(std::uintptr_t node,NativeNodeBaseContext& context){
    auto& memory=context.memory;
    store(memory,node,0x18007fa98ULL);
    context.effects.delete_nodes_003110(node);
    const auto parent=static_cast<std::uintptr_t>(load(memory,node+0x10));
    if(parent){
        const auto head=static_cast<std::uintptr_t>(load(memory,parent+0x38));
        if(head==node)store(memory,parent+0x38,load(memory,head+0x50));
        const auto tail=static_cast<std::uintptr_t>(load(memory,parent+0x40));
        if(tail==node)store(memory,parent+0x40,load(memory,tail+0x48));
        const auto previous=static_cast<std::uintptr_t>(load(memory,node+0x48));
        if(previous)store(memory,previous+0x50,load(memory,node+0x50));
        // The preceding neighbor store can alias these fields. Reload both
        // next and previous at the native points instead of caching a pair.
        const auto next=static_cast<std::uintptr_t>(load(memory,node+0x50));
        if(next)store(memory,next+0x48,load(memory,node+0x48));
        store(memory,node+0x50,0);
        store(memory,node+0x48,0);
        store(memory,node+0x10,0);
    }
    // A null parent preserves own sibling links. Flags/pointer are live after
    // child deletion and unlinking; free failures skip all final resets.
    if(load(memory,node+0x18,4)&0x200)
        context.effects.free_04b0f8(static_cast<std::uintptr_t>(load(memory,node+0x20)));
    store(memory,node+0x18,0,4);
    store(memory,node+0x20,0);
    store(memory,node+0x28,0);
}
void NodeBaseEffects::destroy_node_base_001ea0(std::uintptr_t node){destroy_native_node_base_001ea0(node,node_base_);}
} // namespace laureondo::socket
