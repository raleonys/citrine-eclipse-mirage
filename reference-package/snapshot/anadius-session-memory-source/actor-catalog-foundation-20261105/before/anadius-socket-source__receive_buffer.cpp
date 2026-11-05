#include "receive_buffer.hpp"
#include <algorithm>
#include <cstring>

namespace anadius::socket_recovered {
namespace {
std::uint64_t read_word(std::span<const std::uint8_t> bytes){
    std::uint64_t value=0;for(unsigned i=0;i<8;++i)value|=std::uint64_t{bytes[i]}<<(i*8);return value;
}
void write_word(std::span<std::uint8_t> bytes,std::uint64_t value){
    for(unsigned i=0;i<8;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));
}
void terminate(std::uintptr_t address,ReceiveBufferMemory& memory){memory.view(address,1)[0]=0;}
}
std::span<std::uint8_t> ReceiveBufferMemory::view(std::uintptr_t address,std::uint64_t count) const{
    std::span<std::uint8_t> found;bool matched=false;
    for(const auto& region:regions){
        if(address<region.address)continue;
        const auto offset=address-region.address;
        if(offset>region.bytes.size() || count>region.bytes.size()-offset)continue;
        if(matched)throw UnsupportedInput("Receive buffer memory mapping is ambiguous");
        matched=true;found=region.bytes.subspan(static_cast<std::size_t>(offset),static_cast<std::size_t>(count));
    }
    if(!matched)throw UnsupportedInput("Receive buffer memory leaves bounded regions");
    return found;
}
void BoundedReceiveBufferApi::copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t count){
    if(!count)return;
    const auto destination=memory_.view(to,count),source=memory_.view(from,count);
    std::memmove(destination.data(),source.data(),destination.size());
}
void BoundedReceiveBufferApi::fill_bytes(std::uintptr_t address,std::uint8_t value,std::uint64_t count){
    if(!count)return;
    const auto destination=memory_.view(address,count);
    std::fill(destination.begin(),destination.end(),value);
}
std::uintptr_t receive_buffer_address(const ReceiveBufferString& value){
    return value.capacity_18>15?static_cast<std::uintptr_t>(read_word(value.storage_00)):
        reinterpret_cast<std::uintptr_t>(value.storage_00.data());
}
ReceiveBufferString& grow_receive_buffer_008350(ReceiveBufferString& value,std::uint64_t extra,
    [[maybe_unused]]std::uint64_t unused_hint,std::uint64_t fill_count,std::uint8_t fill,ReceiveBufferContext& context){
    constexpr std::uint64_t maximum=0x7fffffffffffffffULL;
    const auto old_size=value.size_10;
    if(maximum-old_size<extra)context.api.length_error_00ce80();
    const auto old_capacity=value.capacity_18,new_size=old_size+extra;
    const auto rounded=new_size|std::uint64_t{15};
    std::uint64_t capacity=maximum;
    std::uintptr_t data=0;
    if(rounded<=maximum && old_capacity<=maximum-(old_capacity>>1)){
        capacity=std::max(rounded,old_capacity+(old_capacity>>1));
        const auto bytes=capacity+std::uint64_t{1};
        if(bytes>=4096){
            const auto allocation_bytes=bytes+std::uint64_t{39};
            if(allocation_bytes<=bytes)context.api.allocation_overflow_00cd30();
            const auto raw=context.api.allocate_04b0bc(allocation_bytes);
            if(!raw)context.api.invalid_parameter_05c974();
            data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};
            write_word(context.memory.view(data-std::uintptr_t{8},8),raw);
        }else if(bytes){data=context.api.allocate_04b0bc(bytes);}
    }else{
        const auto raw=context.api.allocate_04b0bc(0x8000000000000027ULL);
        if(!raw)context.api.invalid_parameter_05c974();
        data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};
        write_word(context.memory.view(data-std::uintptr_t{8},8),raw);
    }
    value.size_10=new_size;value.capacity_18=capacity;
    const auto tail=data+static_cast<std::uintptr_t>(old_size);
    if(old_capacity<=15){
        context.api.copy_bytes(data,reinterpret_cast<std::uintptr_t>(value.storage_00.data()),old_size);
        context.api.fill_bytes(tail,fill,fill_count);
        terminate(tail+static_cast<std::uintptr_t>(fill_count),context.memory);
    }else{
        // Native loads the old pointer only after allocation and metadata stores.
        const auto old_data=static_cast<std::uintptr_t>(read_word(value.storage_00));
        context.api.copy_bytes(data,old_data,old_size);
        context.api.fill_bytes(tail,fill,fill_count);
        terminate(tail+static_cast<std::uintptr_t>(fill_count),context.memory);
        auto bytes=old_capacity+std::uint64_t{1};
        auto base=old_data;
        if(bytes>=4096){
            base=static_cast<std::uintptr_t>(read_word(context.memory.view(old_data-std::uintptr_t{8},8)));
            bytes+=39;
            if(old_data-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
        }
        context.api.free_sized_04b0f8(base,bytes);
    }
    // Final pointer publication is after copy/fill/NUL and old allocation free.
    write_word(value.storage_00,data);
    return value;
}
void resize_receive_buffer_010040(ReceiveBufferString& value,std::uint64_t new_size,std::uint8_t fill,ReceiveBufferContext& context){
    const auto old_size=value.size_10;
    if(new_size<=old_size){
        value.size_10=new_size;
        terminate(receive_buffer_address(value)+static_cast<std::uintptr_t>(new_size),context.memory);
        return;
    }
    const auto extra=new_size-old_size,capacity=value.capacity_18;
    if(extra<=capacity-old_size){
        value.size_10=new_size;
        const auto tail=receive_buffer_address(value)+static_cast<std::uintptr_t>(old_size);
        context.api.fill_bytes(tail,fill,extra);
        terminate(tail+static_cast<std::uintptr_t>(extra),context.memory);
    }else grow_receive_buffer_008350(value,extra,0,extra,fill,context);
}
} // namespace anadius::socket_recovered
