#include "native_document.hpp"
#include <bit>

namespace anadius::socket_recovered {
namespace {
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));
}
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(i*8);
    return value;
}
std::int32_t error(std::uintptr_t address,NativeDocumentContext& context){
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(load(context.memory,address+0x6c,4)));
}
}
std::uintptr_t construct_native_document_001830(std::uintptr_t address,std::uint8_t mode,
    std::uint32_t flags,NativeDocumentContext& context){
    auto& memory=context.memory;const auto ids=context.identities;
    // Preserve native store order and every untouched padding/inline byte.
    store(memory,address+0x10,0,8);store(memory,address+0x18,0,4);
    store(memory,address+0x20,0,8);store(memory,address+0x28,0,8);
    store(memory,address,ids.document,8);
    store(memory,address+0x30,0,4);store(memory,address+0x38,0,8);
    store(memory,address+0x40,0,8);store(memory,address+0x48,0,8);
    store(memory,address+0x50,0,8);store(memory,address+0x58,0,8);
    store(memory,address+0x60,0,8);store(memory,address+0x68,0,1);
    store(memory,address+0x69,mode,1);store(memory,address+0x6c,0,4);
    store(memory,address+0x70,flags,4);store(memory,address+0x78,0,4);
    store(memory,address+0x80,0,8);store(memory,address+0x88,0,8);
    store(memory,address+0x90,0,4);store(memory,address+0x98,0,8);
    store(memory,address+0xa0,0,8);store(memory,address+0xa8,address+0xb0,8);
    store(memory,address+0x100,10,8);store(memory,address+0x108,ids.pool120,8);
    store(memory,address+0x110,address+0x118,8);store(memory,address+0x168,10,8);
    store(memory,address+0x170,0,8);store(memory,address+0x178,0,8);
    store(memory,address+0x180,0,8);store(memory,address+0x188,ids.pool80,8);
    store(memory,address+0x190,address+0x198,8);store(memory,address+0x1e8,10,8);
    store(memory,address+0x1f0,0,8);store(memory,address+0x1f8,0,8);
    store(memory,address+0x200,0,8);store(memory,address+0x208,ids.pool112,8);
    store(memory,address+0x210,address+0x218,8);store(memory,address+0x268,10,8);
    store(memory,address+0x270,0,8);store(memory,address+0x278,0,8);
    store(memory,address+0x280,0,8);store(memory,address+0x288,ids.pool104,8);
    store(memory,address+0x290,address+0x298,8);store(memory,address+0x2e8,10,8);
    store(memory,address+0x2f0,0,8);store(memory,address+0x2f8,0,8);
    store(memory,address+0x300,0,8);store(memory,address+8,address,8);
    return address;
}
std::int32_t parse_native_document_004280(std::uintptr_t address,std::uintptr_t input,
    std::uint64_t count,NativeDocumentContext& context){
    auto& memory=context.memory;auto& effects=context.effects;
    effects.clear_002e60(address);
    if(!count||!input||!memory.view(input,1)[0])effects.set_error_005820(address,13,0,0);
    else{
        if(count==UINT64_MAX){do{++count;}while(memory.view(input+count,1)[0]);}
        const auto allocation=effects.allocate_04b100(count+1);
        store(memory,address+0x98,allocation,8);
        effects.copy_078d30(allocation,input,count);
        store(memory,load(memory,address+0x98,8)+count,0,1);
        effects.parse_0041b0(address);
        if(error(address,context)){
            effects.delete_nodes_003110(address);
            effects.clear_pool_002d70(address+0x108);
            effects.clear_pool_002d70(address+0x188);
            effects.clear_pool_002e10(address+0x208);
            effects.clear_pool_002dc0(address+0x288);
        }
    }
    return error(address,context);
}
void destroy_native_document_001e00(std::uintptr_t address,NativeDocumentContext& context){
    auto& memory=context.memory;auto& effects=context.effects;
    store(memory,address,context.identities.document,8);
    effects.clear_002e60(address);
    effects.destroy_pool_001c30(address+0x288);
    effects.destroy_pool_001cb0(address+0x208);
    effects.destroy_pool_001bb0(address+0x188);
    effects.destroy_pool_001d30(address+0x108);
    const auto vector=load(memory,address+0xa8,8);
    if(vector!=address+0xb0)effects.free_04b0f8(vector);
    if(load(memory,address+0x78,4)&0x200)effects.free_04b0f8(load(memory,address+0x80,8));
    store(memory,address+0x78,0,4);store(memory,address+0x80,0,8);store(memory,address+0x88,0,8);
    effects.destroy_node_base_001ea0(address);
}
std::uintptr_t RecoveredMessageDocumentApi::resolve(NativeMessageDocument& document){
    static_assert(sizeof(int)==4&&std::endian::native==std::endian::little);
    std::uintptr_t result=0;bool matched=false;
    for(const auto& binding:bindings_)if(binding.projection==&document){
        if(matched)throw UnsupportedInput("Native document identity is ambiguous");
        matched=true;result=binding.address;
    }
    if(!result)throw UnsupportedInput("Native document identity is unbound");
    if(documents_.memory.view(result+0x6c,4).data()!=reinterpret_cast<std::uint8_t*>(&document.error_6c))
        throw UnsupportedInput("Native document error cell must share caller storage");
    return result;
}
void RecoveredMessageDocumentApi::construct_document_001830(NativeMessageDocument& document,bool mode,std::uint32_t flags){
    (void)construct_native_document_001830(resolve(document),static_cast<std::uint8_t>(mode),flags,documents_);
}
int RecoveredMessageDocumentApi::parse_document_004280(NativeMessageDocument& document,std::uintptr_t data,std::uint64_t count){
    return parse_native_document_004280(resolve(document),data,count,documents_);
}
void RecoveredMessageDocumentApi::destroy_document_001e00(NativeMessageDocument& document){
    destroy_native_document_001e00(resolve(document),documents_);
}
} // namespace anadius::socket_recovered
