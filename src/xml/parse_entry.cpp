// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/xml/parse_entry.hpp"

namespace laureondo::socket {
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
}
void parse_document_entry_0041b0(std::uintptr_t document,NativeParseEntryContext& context){
    auto& memory=context.memory;
    auto cursor=static_cast<std::uintptr_t>(load(memory,document+0x98,8));
    const auto line_counter=document+0xa0;
    store(memory,line_counter,1,4);
    store(memory,document+0x30,1,4);
    for(;;){
        const auto byte=memory.view(cursor,1)[0];
        if(byte&0x80)break;
        if(!context.lower.classify_051fc0(byte))break;
        // Classification may mutate input or counters. Native code rereads the
        // byte only on this branch and increments a live DWORD for LF.
        if(line_counter&&memory.view(cursor,1)[0]==0x0a){
            const auto line=static_cast<std::uint32_t>(load(memory,line_counter,4));
            store(memory,line_counter,line+std::uint32_t{1},4);
        }
        ++cursor;
    }
    store(memory,document+0x68,0,1);
    auto byte=memory.view(cursor,1)[0];
    if(byte==0xef&&memory.view(cursor+1,1)[0]==0xbb&&memory.view(cursor+2,1)[0]==0xbf){
        const auto advanced=cursor+3;
        store(memory,document+0x68,1,1);
        cursor=advanced;
        byte=memory.view(cursor,1)[0];
    }
    if(!byte){
        context.documents.set_error_005820(document,13,0,0);
        return;
    }
    const auto table=static_cast<std::uintptr_t>(load(memory,document,8));
    const auto target=static_cast<std::uintptr_t>(load(memory,table+0x80,8));
    context.lower.parse_virtual(target,document,cursor,0,document+0xa0);
}
void ParseEntryEffects::parse_0041b0(std::uintptr_t document){parse_document_entry_0041b0(document,parse_entry_);}
} // namespace laureondo::socket
