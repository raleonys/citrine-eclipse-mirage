// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/xml/node_identify.hpp"

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
std::uintptr_t identify_node_0038b0(std::uintptr_t document,std::uintptr_t input,
    std::uintptr_t out_node,NativeNodeIdentifyContext& context){
    auto& m=context.memory;const auto ids=context.identities;
    const auto line_address=document+0xa0;
    const auto initial_line=static_cast<std::uint32_t>(load(m,line_address,4));
    auto candidate=input;
    for(;;){
        const auto byte=m.view(candidate,1)[0];
        if(byte&0x80)break;
        if(!context.characters.classify_051fc0(byte))break;
        if(line_address&&m.view(candidate,1)[0]==0x0a)
            store(m,line_address,static_cast<std::uint32_t>(load(m,line_address,4))+std::uint32_t{1},4);
        ++candidate;
    }
    if(!m.view(candidate,1)[0]){store(m,out_node,0);return candidate;}
    constexpr std::array<std::uint64_t,5> lengths{2,4,9,2,1};
    std::size_t selected=lengths.size();
    for(std::size_t i=0;i<lengths.size();++i){
        const auto token=load(m,ids.token_slots[i]);
        if(candidate==token||!context.parser.compare_05c550(candidate,token,lengths[i])){
            selected=i;break;
        }
    }
    auto& constructors=context.constructors;std::uintptr_t node=0,result=input;
    switch(selected){
    case 0:node=constructors.construct_001610(document,document+0x288);break;
    case 1:node=constructors.construct_001590(document,document+0x288);break;
    case 2:node=constructors.construct_001720(document,document+0x208);break;
    case 3:node=constructors.construct_0017b0(document,document+0x288);break;
    case 4:node=constructors.construct_001690(document,document+0x108);break;
    default:node=constructors.construct_001720(document,document+0x208);break;
    }
    const auto line=static_cast<std::uint32_t>(load(m,line_address,4));
    if(selected==2)store(m,node+0x68,1,1);
    store(m,node+0x30,line,4);
    if(selected==lengths.size())store(m,line_address,initial_line,4);
    else result=candidate+lengths[selected];
    store(m,out_node,node);
    return result;
}
std::uintptr_t NodeIdentifyEffects::identify_0038b0(std::uintptr_t document,
    std::uintptr_t input,std::uintptr_t out_node){
    return identify_node_0038b0(document,input,out_node,identify_);
}
} // namespace laureondo::socket
