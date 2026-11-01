#include "node_delete.hpp"
#include <bit>

namespace anadius::socket_recovered {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width=8){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(i*8);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width=8){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));
}
std::int32_t count(ReceiveBufferMemory& memory,std::uintptr_t document){
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(load(memory,document+0x104,4)));
}
}
void unlink_native_node_003030(std::uintptr_t parent,std::uintptr_t node,NativeNodeDeleteContext& context){
    auto& memory=context.memory;auto& effects=context.effects;
    const auto first=load(memory,parent+0x38);
    if(node==first)store(memory,parent+0x38,load(memory,first+0x50));
    const auto last=load(memory,parent+0x40);
    if(node==last)store(memory,parent+0x40,load(memory,last+0x48));
    const auto previous=load(memory,node+0x48);
    if(previous)store(memory,previous+0x50,load(memory,node+0x50));
    const auto next=load(memory,node+0x50);
    if(next)store(memory,next+0x48,load(memory,node+0x48));
    const auto table=load(memory,node);
    store(memory,node+0x50,0);store(memory,node+0x48,0);store(memory,node+0x10,0);
    if(!effects.node_probe(load(memory,table+0x38),node)){
        const auto document=load(memory,node+8);const auto initial=count(memory,document);
        if(initial>0){
            const auto entries=load(memory,document+0xa8),bound=static_cast<std::uint64_t>(initial);
            std::uint32_t index=0;
            for(std::uint64_t cursor=0;cursor<bound;++cursor,++index){
                if(load(memory,entries+cursor*8)!=node)continue;
                //0030ca MOV EAX zero-extends this LIVE DWORD, unlike003110.
                const auto live=static_cast<std::uint32_t>(load(memory,document+0x104,4));
                const auto slot=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(index)));
                const auto replacement=load(memory,entries+std::uint64_t{live}*8-8);
                store(memory,entries+slot*8,replacement);
                store(memory,document+0x104,static_cast<std::uint32_t>(load(memory,document+0x104,4))-std::uint32_t{1},4);
                break;
            }
        }
    }
    const auto live_table=load(memory,node),pool=load(memory,node+0x60);
    effects.node_destroy(load(memory,live_table+0x78),node,0);
    const auto pool_table=load(memory,pool);
    effects.pool_release(load(memory,pool_table+0x18),pool,node);
}
void delete_native_node_003240(std::uintptr_t node,NativeNodeDeleteContext& context){
    if(!node)return;
    auto& memory=context.memory;auto& effects=context.effects;
    const auto table=load(memory,node);
    if(!effects.node_probe(load(memory,table+0x38),node)){
        const auto document=load(memory,node+8);const auto initial=count(memory,document);
        if(initial>0){
            const auto entries=load(memory,document+0xa8),bound=static_cast<std::uint64_t>(initial);
            std::uint32_t index=0;
            for(std::uint64_t cursor=0;cursor<bound;++cursor,++index){
                if(load(memory,entries+cursor*8)!=node)continue;
                //003292 uses the initially cached positive count R8.
                const auto replacement=load(memory,entries+bound*8-8);
                const auto slot=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(index)));
                store(memory,entries+slot*8,replacement);
                store(memory,document+0x104,static_cast<std::uint32_t>(load(memory,document+0x104,4))-std::uint32_t{1},4);
                break;
            }
        }
    }
    const auto live_table=load(memory,node),pool=load(memory,node+0x60);
    effects.node_destroy(load(memory,live_table+0x78),node,0);
    const auto pool_table=load(memory,pool);
    effects.pool_release(load(memory,pool_table+0x18),pool,node);
}
void RecoveredNodeDeleteEffects::remove_parent_003030(std::uintptr_t parent,std::uintptr_t node){unlink_native_node_003030(parent,node,nodes_);}
void RecoveredNodeDeleteEffects::delete_node_003240(std::uintptr_t node){delete_native_node_003240(node,nodes_);}
} // namespace anadius::socket_recovered
