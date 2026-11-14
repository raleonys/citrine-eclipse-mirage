// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/xml/node_string.hpp"
#include <bit>

namespace laureondo::socket {
namespace {
std::uint64_t load(ReceiveBufferMemory& m,std::uintptr_t a,unsigned width=8){
    const auto b=m.view(a,width);std::uint64_t v=0;
    for(unsigned i=0;i<width;++i)v|=std::uint64_t{b[i]}<<(8*i);
    return v;
}
void store(ReceiveBufferMemory& m,std::uintptr_t a,std::uint64_t v,unsigned width=8){
    auto b=m.view(a,width);for(unsigned i=0;i<width;++i)b[i]=static_cast<std::uint8_t>(v>>(8*i));
}
std::uint64_t extend(std::uint32_t v){
    return static_cast<std::uint64_t>(static_cast<std::int64_t>(std::bit_cast<std::int32_t>(v)));
}
}
std::uintptr_t materialize_node_string_0035e0(std::uintptr_t pair,NativeNodeStringContext& c){
    auto& m=c.memory;
    if(!(load(m,pair,4)&0x100))return load(m,pair+8);
    store(m,load(m,pair+0x10),0,1);
    const auto flags=static_cast<std::uint32_t>(load(m,pair,4))^std::uint32_t{0x100};
    store(m,pair,flags,4);
    if(flags){
        auto output=load(m,pair+8),input=output;
        const auto entity_table=c.identities.entity_table;
        while(input<load(m,pair+0x10)){
            const auto iteration_flags=load(m,pair,4);
            if(iteration_flags&2){
                const auto b=load(m,input,1);
                if(b==13||b==10){
                    const auto following=load(m,input+1,1);
                    input+=(following==(b==13?10U:13U))?2:1;
                    store(m,output,10,1);++output;continue;
                }
            }
            if((iteration_flags&1)&&load(m,input,1)=='&'){
                if(load(m,input+1,1)=='#'){
                    std::int32_t count=0;std::array<std::uint8_t,10> scratch{};
                    const auto decoded=c.lower.decode_0033c0(input,scratch,count);
                    if(decoded){
                        const auto length=extend(static_cast<std::uint32_t>(count));
                        input=decoded;c.lower.copy_string_078d30(output,scratch,length);
                        output+=length;continue;
                    }
                }else{
                    bool matched=false;
                    for(unsigned row=0;row<5;++row){
                        const auto entry=entity_table+row*16;
                        const auto limit=extend(static_cast<std::uint32_t>(load(m,entry+8,4)));
                        const auto name=load(m,entry);
                        if(c.parser.compare_05c550(input+1,name,limit))continue;
                        const auto live_length=extend(static_cast<std::uint32_t>(load(m,entry+8,4)));
                        if(load(m,input+live_length+1,1)!=';')continue;
                        store(m,output,load(m,entry+12,1),1);++output;
                        const auto advance=static_cast<std::uint32_t>(load(m,entry+8,4))+std::uint32_t{2};
                        input+=extend(advance);matched=true;break;
                    }
                    //003728 jumps to003745, bypassing the byte store at003743.
                    if(!matched){++input;++output;}
                    continue;
                }
            }
            store(m,output,load(m,input,1),1);++input;++output;
        }
        store(m,output,0,1);
    }
    if(load(m,pair,1)&4){
        auto input=load(m,pair+8);
        for(;;){
            const auto b=static_cast<std::uint32_t>(load(m,input,1));
            if((b&0x80)||!c.characters.classify_051fc0(b))break;
            ++input;
        }
        store(m,pair+8,input);
        auto current=static_cast<std::uint32_t>(load(m,input,1));
        if(current){
            auto output=input;
            for(;;){
                if(!(current&0x80)&&c.characters.classify_051fc0(current)){
                    for(;;){
                        const auto b=static_cast<std::uint32_t>(load(m,input,1));
                        if((b&0x80)||!c.characters.classify_051fc0(b))break;
                        ++input;
                    }
                    if(!load(m,input,1))break;
                    store(m,output,0x20,1);++output;
                }
                current=static_cast<std::uint32_t>(load(m,input,1));++input;
                store(m,output,current,1);++output;
                current=static_cast<std::uint32_t>(load(m,input,1));
                if(!current)break;
            }
            store(m,output,0,1);
        }
    }
    store(m,pair,load(m,pair,4)&0x200,4);
    return load(m,pair+8);
}
std::uintptr_t NodeStringEffects::materialize_0035e0(std::uintptr_t pair){
    return materialize_node_string_0035e0(pair,string_);
}
} // namespace laureondo::socket
