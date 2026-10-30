#include "preloader_l/xml/document_clear.hpp"
#include <bit>

namespace preloader_l::socket_recovered {
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
std::int32_t signed_count(ReceiveBufferMemory& memory,std::uintptr_t document){
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(load(memory,document+0x104,4)));
}
}
void delete_native_nodes_003110(std::uintptr_t owner,NativeDocumentClearContext& context){
    auto& memory=context.memory;auto& effects=context.nodes;
    auto node=load(memory,owner+0x38);
    while(node){
        store(memory,owner+0x38,load(memory,node+0x50));
        const auto last=load(memory,owner+0x40);
        if(node==last)store(memory,owner+0x40,load(memory,last+0x48));
        const auto previous=load(memory,node+0x48);
        if(previous)store(memory,previous+0x50,load(memory,node+0x50));
        const auto next=load(memory,node+0x50);
        if(next)store(memory,next+0x48,load(memory,node+0x48));
        const auto table=load(memory,node);
        store(memory,node+0x50,0);store(memory,node+0x48,0);store(memory,node+0x10,0);
        if(!effects.node_probe(load(memory,table+0x38),node)){
            const auto document=load(memory,node+8);const auto count=signed_count(memory,document);
            if(count>0){
                const auto entries=load(memory,document+0xa8);
                const auto bound=static_cast<std::uint64_t>(count);
                for(std::uint64_t index=0;index<bound;++index){
                    if(load(memory,entries+index*8)!=node)continue;
                    const auto live=static_cast<std::uint64_t>(static_cast<std::int64_t>(signed_count(memory,document)));
                    const auto replacement=load(memory,entries+live*8-8);
                    store(memory,entries+index*8,replacement);
                    // Native DEC rereads the DWORD after the swap store.
                    store(memory,document+0x104,static_cast<std::uint32_t>(load(memory,document+0x104,4))-std::uint32_t{1},4);
                    break;
                }
            }
        }
        const auto live_table=load(memory,node),pool=load(memory,node+0x60);
        effects.node_destroy(load(memory,live_table+0x78),node,0);
        // Pool is cached before destruction; its vtable and release target
        // are live after that callback. RDX=node is defined in0031f0.
        const auto pool_table=load(memory,pool);
        effects.pool_release(load(memory,pool_table+0x18),pool,node);
        node=load(memory,owner+0x38);
    }
    store(memory,owner+0x40,0);store(memory,owner+0x38,0);
}
void clear_native_document_002e60(std::uintptr_t document,NativeDocumentClearContext& context){
    auto& memory=context.memory;auto& effects=context.nodes;
    delete_native_nodes_003110(document,context);
    while(load(memory,document+0x104,4)!=0){
        const auto entries=load(memory,document+0xa8),node=load(memory,entries);
        const auto parent=load(memory,node+0x10);
        if(parent)effects.remove_parent_003030(parent,node);
        else{
            const auto pool=load(memory,node+0x60),table=load(memory,pool);
            effects.pool_track(load(memory,table+0x20),pool);
            effects.delete_node_003240(node);
        }
    }
    store(memory,document+0x6c,0,4);store(memory,document+0x90,0,4);
    if(load(memory,document+0x78,4)&0x200)context.documents.free_04b0f8(load(memory,document+0x80));
    store(memory,document+0x78,0,4);store(memory,document+0x80,0);store(memory,document+0x88,0);
    context.documents.free_04b0f8(load(memory,document+0x98));
    store(memory,document+0x98,0);store(memory,document+0xa4,0,4);
}
void RecoveredDocumentClearEffects::clear_002e60(std::uintptr_t document){clear_native_document_002e60(document,clear_);}
void RecoveredDocumentClearEffects::delete_nodes_003110(std::uintptr_t owner){delete_native_nodes_003110(owner,clear_);}
} // namespace preloader_l::socket_recovered
