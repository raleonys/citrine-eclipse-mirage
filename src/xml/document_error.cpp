#include "preloader_l/xml/document_error.hpp"

namespace preloader_l::socket_recovered {
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
std::uint64_t length(ReceiveBufferMemory& memory,std::uintptr_t address){
    std::uint64_t count=0;
    while(memory.view(address+count,1)[0])++count;
    return count;
}
void release_error(std::uintptr_t document,ReceiveBufferMemory& memory,NativeDocumentEffects& effects){
    if(load(memory,document+0x78,4)&0x200)
        effects.free_04b0f8(static_cast<std::uintptr_t>(load(memory,document+0x80,8)));
    store(memory,document+0x78,0,4);
    store(memory,document+0x80,0,8);
    store(memory,document+0x88,0,8);
}
}
void set_document_error_005820(std::uintptr_t document,std::int32_t error_id,
    std::uint32_t line,std::uintptr_t detail_format,NativeErrorArgumentTail arguments,
    NativeDocumentErrorContext& context){
    auto& memory=context.memory;auto& documents=context.documents;auto& lower=context.lower;
    const auto ids=context.identities;
    const auto index=static_cast<std::uint64_t>(static_cast<std::int64_t>(error_id));
    store(memory,document+0x6c,static_cast<std::uint32_t>(error_id),4);
    store(memory,document+0x90,line,4);
    release_error(document,memory,documents);
    const auto temporary=documents.allocate_04b100(1000);
    const auto name=static_cast<std::uintptr_t>(load(memory,ids.error_names+index*8,8));
    lower.format_header_0060a0(temporary,1000,ids.header_format,name,error_id,
        static_cast<std::uint32_t>(error_id),line);
    if(detail_format){
        auto used=length(memory,temporary);
        lower.format_separator_0060a0(temporary+used,std::uint64_t{1000}-used,ids.separator_format);
        used=length(memory,temporary);
        lower.format_detail_006110(temporary+used,std::uint64_t{1000}-used,
            detail_format,arguments.opaque_cursor);
    }
    // Formatting may have replaced the document's owned error storage.
    release_error(document,memory,documents);
    const auto used=length(memory,temporary);
    const auto owned=documents.allocate_04b100(used+1);
    store(memory,document+0x80,owned,8);
    documents.copy_078d30(owned,temporary,used+1);
    const auto end=load(memory,document+0x80,8)+used;
    store(memory,document+0x78,0x200,4);
    store(memory,document+0x88,end,8);
    documents.free_04b0f8(temporary);
}
void RecoveredDocumentErrorEffects::set_error_005820(std::uintptr_t document,
    std::int32_t error_id,std::uint32_t line,std::uintptr_t detail_format){
    set_document_error_005820(document,error_id,line,detail_format,arguments_,error_);
}
} // namespace preloader_l::socket_recovered
