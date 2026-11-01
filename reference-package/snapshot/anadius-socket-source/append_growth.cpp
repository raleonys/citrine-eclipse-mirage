#include "append_growth.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
namespace {
void word(std::span<std::uint8_t> bytes,std::uint64_t value){for(unsigned i=0;i<8;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));}
std::uint64_t word(std::span<const std::uint8_t> bytes){std::uint64_t value=0;for(unsigned i=0;i<8;++i)value|=std::uint64_t{bytes[i]}<<(i*8);return value;}
struct Prepared {std::uint64_t old_size,old_capacity;std::uintptr_t data;};
Prepared prepare(ReceiveBufferString& value,std::uint64_t extra,ReceiveBufferContext& context){
    constexpr std::uint64_t maximum=0x7fffffffffffffffULL;
    const auto old_size=value.size_10;if(maximum-old_size<extra)context.api.length_error_00ce80();
    const auto old_capacity=value.capacity_18,new_size=old_size+extra,rounded=new_size|std::uint64_t{15};
    auto capacity=maximum;std::uintptr_t data=0;
    if(rounded<=maximum&&old_capacity<=maximum-(old_capacity>>1)){
        capacity=std::max(rounded,old_capacity+(old_capacity>>1));const auto bytes=capacity+std::uint64_t{1};
        if(bytes>=4096){
            const auto count=bytes+std::uint64_t{39};if(count<=bytes)context.api.allocation_overflow_00cd30();
            const auto raw=context.api.allocate_04b0bc(count);if(!raw)context.api.invalid_parameter_05c974();
            data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};word(context.memory.view(data-8,8),raw);
        }else if(bytes)data=context.api.allocate_04b0bc(bytes);
    }else{
        const auto raw=context.api.allocate_04b0bc(0x8000000000000027ULL);if(!raw)context.api.invalid_parameter_05c974();
        data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};word(context.memory.view(data-8,8),raw);
    }
    value.size_10=new_size;value.capacity_18=capacity;return {old_size,old_capacity,data};
}
std::uintptr_t copy_old(ReceiveBufferString& value,const Prepared& p,ReceiveBufferContext& context){
    // Native reloads the old heap pointer after allocation and metadata stores.
    const auto old=p.old_capacity>15?static_cast<std::uintptr_t>(word(value.storage_00)):
        reinterpret_cast<std::uintptr_t>(value.storage_00.data());
    context.api.copy_bytes(p.data,old,p.old_size);return old;
}
void release_and_publish(ReceiveBufferString& value,const Prepared& p,std::uintptr_t old,ReceiveBufferContext& context){
    if(p.old_capacity>15){
        auto count=p.old_capacity+std::uint64_t{1};auto raw=old;
        if(count>=4096){raw=static_cast<std::uintptr_t>(word(context.memory.view(old-8,8)));count+=39;if(old-raw-8>31)context.api.invalid_parameter_05c974();}
        context.api.free_sized_04b0f8(raw,count);
    }
    word(value.storage_00,p.data); // only first8 bytes, after old free; metadata stays live
}
}
ReceiveBufferString& grow_append_character_012350(ReceiveBufferString& value,std::uint64_t extra,
    [[maybe_unused]]std::uint64_t unused_hint,std::uint8_t character,ReceiveBufferContext& context){
    const auto p=prepare(value,extra,context);const auto old=copy_old(value,p,context);
    context.memory.view(p.data+p.old_size,1)[0]=character;
    context.memory.view(p.data+p.old_size+1,1)[0]=0;
    release_and_publish(value,p,old,context);return value;
}
ReceiveBufferString& grow_append_bytes_007e30(ReceiveBufferString& value,std::uint64_t extra,
    [[maybe_unused]]std::uint64_t unused_hint,std::uintptr_t source,std::uint64_t count,ReceiveBufferContext& context){
    const auto p=prepare(value,extra,context);const auto old=copy_old(value,p,context);
    const auto tail=p.data+p.old_size;context.api.copy_bytes(tail,source,count);
    context.memory.view(tail+count,1)[0]=0;
    release_and_publish(value,p,old,context);return value;
}
void RecoveredMessageAppendApi::grow_nul_012350(ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uint8_t character){
    grow_append_character_012350(value,extra,hint,character,strings_);
}
void RecoveredMessageAppendApi::grow_append_007e30(ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uintptr_t source,std::uint64_t count){
    grow_append_bytes_007e30(value,extra,hint,source,count,strings_);
}
} // namespace anadius::socket_recovered
