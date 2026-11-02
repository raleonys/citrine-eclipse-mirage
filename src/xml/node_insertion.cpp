// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/node_insertion.hpp"
#include <bit>

namespace preloader_l::socket {
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
void prepare_native_insertion_003aa0(std::uintptr_t,std::uintptr_t child,
    NativeNodeInsertionContext& context){
    auto& m=context.memory;const auto parent=load(m,child+0x10),parent_field=child+0x10;
    if(parent){
        const auto first=load(m,parent+0x38);
        if(child==first)store(m,parent+0x38,load(m,first+0x50));
        const auto last=load(m,parent+0x40);
        if(child==last)store(m,parent+0x40,load(m,last+0x48));
        const auto previous=load(m,child+0x48);
        if(previous)store(m,previous+0x50,load(m,child+0x50));
        const auto next=load(m,child+0x50);
        if(next)store(m,next+0x48,load(m,child+0x48));
        store(m,child+0x50,0);store(m,child+0x48,0);store(m,parent_field,0);
        return;
    }
    const auto document=load(m,child+8);
    const auto initial=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(load(m,document+0x104,4)));
    if(initial>0){
        const auto entries=load(m,document+0xa8),bound=static_cast<std::uint64_t>(initial);
        std::uint32_t index=0;
        for(std::uint64_t cursor=0;cursor<bound;++cursor,++index){
            if(load(m,entries+cursor*8)!=child)continue;
            //003b4b MOV EAX zero-extends the LIVE count for the tail load.
            const auto live=static_cast<std::uint32_t>(load(m,document+0x104,4));
            const auto replacement=load(m,entries+std::uint64_t{live}*8-8);
            const auto slot=static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(index)));
            store(m,entries+slot*8,replacement);
            store(m,document+0x104,static_cast<std::uint32_t>(load(m,document+0x104,4))-std::uint32_t{1},4);
            break;
        }
    }
    const auto pool=load(m,child+0x60),table=load(m,pool);
    context.nodes.pool_track(load(m,table+0x20),pool);
}
void NodeInsertionEffects::prepare_003aa0(std::uintptr_t owner,std::uintptr_t child){
    prepare_native_insertion_003aa0(owner,child,insertion_);
}
} // namespace preloader_l::socket
