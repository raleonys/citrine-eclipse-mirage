#include "preloader_l/net/transport_response_containers.hpp"
#include <algorithm>
#include <cstring>

namespace preloader_l::socket_recovered {
namespace {
std::uint64_t word(std::span<const std::uint8_t> bytes){
    std::uint64_t result=0;for(unsigned i=0;i<8;++i)result|=std::uint64_t{bytes[i]}<<(i*8);return result;
}
void word(std::span<std::uint8_t> bytes,std::uint64_t value){
    for(unsigned i=0;i<8;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));
}
std::uintptr_t aligned(std::uint64_t bytes,ReceiveBufferContext& context){
    const auto raw=context.api.allocate_04b0bc(bytes);
    if(!raw)context.api.invalid_parameter_05c974();
    const auto data=(raw+std::uintptr_t{39})&~std::uintptr_t{31};
    word(context.memory.view(data-std::uintptr_t{8},8),raw);return data;
}
}
ReceiveBufferString& copy_response_string_009eb0(ReceiveBufferString& out,const ReceiveBufferString& source,ReceiveBufferContext& context){
    out.storage_00.fill(0);out.size_10=0;out.capacity_18=0; // native constructor prefix
    const auto size=source.size_10;const bool heap=source.capacity_18>15;
    const auto data=receive_buffer_address(source); // saved before allocator/COPY
    constexpr std::uint64_t maximum=0x7fffffffffffffffULL;
    if(size>maximum)context.api.length_error_00ce80();
    if(size<=15){
        out.size_10=size;out.capacity_18=15;
        if(heap){const auto bytes=context.memory.view(data,16);std::copy(bytes.begin(),bytes.end(),out.storage_00.begin());}
        else out.storage_00=source.storage_00; // native inline MOVUPS16, not strlen/copy(size)
        return out;
    }
    const auto rounded=size|std::uint64_t{15};auto capacity=maximum;std::uintptr_t destination=0;
    if(rounded<=maximum){
        capacity=std::max(rounded,std::uint64_t{22});const auto bytes=capacity+std::uint64_t{1};
        if(bytes>=4096){
            const auto allocation_bytes=bytes+std::uint64_t{39};
            if(allocation_bytes<=bytes)context.api.allocation_overflow_00cd30();
            destination=aligned(allocation_bytes,context);
        }else if(bytes)destination=context.api.allocate_04b0bc(bytes);
    }else destination=aligned(0x8000000000000027ULL,context);
    word(out.storage_00,destination);out.size_10=size;out.capacity_18=capacity;
    context.api.copy_bytes(destination,data,size+std::uint64_t{1}); // includes NUL, preserves source
    return out;
}
std::uintptr_t DequeMapMemory::address_of(std::span<WriteFinishString*> slots) const{
    if(!slots.data())return 0;
    std::uintptr_t result=0;bool found=false;
    for(const auto& item:bindings){
        if(item.slots.data()!=slots.data() || item.slots.size()<slots.size())continue;
        if(found)throw UnsupportedInput("Deque map identity is ambiguous");
        found=true;result=item.address;
    }
    if(!found)throw UnsupportedInput("Deque map identity is unbound");
    return result;
}
std::span<WriteFinishString*> DequeMapMemory::view(std::uintptr_t address,std::uint64_t bytes) const{
    if(bytes%8)throw UnsupportedInput("Deque byte count is not a complete pointer slot");
    std::span<WriteFinishString*> result;bool found=false;
    for(const auto& item:bindings){
        if(!item.address || address<item.address)continue;
        const auto distance=address-item.address;if(distance%8)continue;
        const auto offset=distance/8,count=bytes/8;
        if(offset>item.slots.size() || count>item.slots.size()-offset)continue;
        if(found)throw UnsupportedInput("Deque map region is ambiguous");
        found=true;result=item.slots.subspan(static_cast<std::size_t>(offset),static_cast<std::size_t>(count));
    }
    if(!found)throw UnsupportedInput("Deque pointer access leaves bounded maps");
    return result;
}
void BoundedDequeGrowthApi::copy_slots(std::uintptr_t to,std::uintptr_t from,std::uint64_t bytes){
    if(!bytes)return;
    const auto destination=memory_.view(to,bytes),source=memory_.view(from,bytes);
    std::memmove(destination.data(),source.data(),destination.size()*sizeof(WriteFinishString*));
}
void BoundedDequeGrowthApi::zero_slots(std::uintptr_t to,std::uint64_t bytes){
    if(!bytes)return;
    const auto destination=memory_.view(to,bytes);std::fill(destination.begin(),destination.end(),nullptr);
}
void grow_response_deque_026670(WriteFinishOwner& queue,std::uint64_t extra,DequeGrowthContext& context){
    const auto old_size=queue.map_size_d8;auto candidate=old_size?old_size:std::uint64_t{1};
    while(candidate-old_size<extra || candidate<8){
        if(std::uint64_t{0x7ffffffffffffffULL}-candidate<candidate)context.api.length_error_018460();
        candidate+=candidate;
    }
    const auto offset=queue.offset_e0; //0266cd before allocation
    if(candidate>0x1fffffffffffffffULL)context.allocation.api.allocation_overflow_00cd30();
    const auto new_bytes=candidate*std::uint64_t{8};std::uintptr_t data=0;
    if(new_bytes>=4096){
        const auto bytes=new_bytes+std::uint64_t{39};
        if(bytes<=new_bytes)context.allocation.api.allocation_overflow_00cd30();
        data=aligned(bytes,context.allocation);
    }else if(new_bytes)data=context.allocation.api.allocate_04b0bc(new_bytes);
    auto offset_bytes=offset*std::uint64_t{8};
    const auto delta=candidate-queue.map_size_d8; //026758 live old-size reload
    auto source=context.maps.address_of(queue.map_d0);
    const auto tail_bytes=queue.map_size_d8*std::uint64_t{8}-offset_bytes;
    auto cursor=data+static_cast<std::uintptr_t>(offset_bytes);
    context.api.copy_slots(cursor,source+static_cast<std::uintptr_t>(offset_bytes),tail_bytes);
    source=context.maps.address_of(queue.map_d0);cursor+=static_cast<std::uintptr_t>(tail_bytes);
    if(offset>delta){
        const auto prefix_bytes=delta*std::uint64_t{8};
        context.api.copy_slots(cursor,source,prefix_bytes);
        source=context.maps.address_of(queue.map_d0)+static_cast<std::uintptr_t>(prefix_bytes);
        offset_bytes-=prefix_bytes;
        context.api.copy_slots(data,source,offset_bytes);
        context.api.zero_slots(data+static_cast<std::uintptr_t>(offset_bytes),prefix_bytes);
    }else{
        context.api.copy_slots(cursor,source,offset_bytes);
        context.api.zero_slots(cursor+static_cast<std::uintptr_t>(offset_bytes),(delta-offset)*std::uint64_t{8});
        context.api.zero_slots(data,offset_bytes);
    }
    source=context.maps.address_of(queue.map_d0); //0267eb live map/size before free
    if(source){
        auto bytes=queue.map_size_d8*std::uint64_t{8};auto base=source;
        if(bytes>=4096){
            base=static_cast<std::uintptr_t>(word(context.allocation.memory.view(source-std::uintptr_t{8},8)));
            bytes+=39;if(source-base-std::uintptr_t{8}>31)context.allocation.api.invalid_parameter_05c974();
        }
        context.allocation.api.free_sized_04b0f8(base,bytes); // authoritative02682c
    }
    queue.map_size_d8+=delta; //026831 reload AFTER free, not cached candidate
    queue.map_d0=context.maps.view(data,new_bytes); // pure host projection, not a native callback
}
ReceiveBufferString& RecoveredResponseContainersApi::copy_response_009eb0(ReceiveBufferString& to,ReceiveBufferString& from){
    return copy_response_string_009eb0(to,from,strings_);
}
void RecoveredResponseQueueApi::grow_deque_026670(WriteFinishOwner& owner,std::uint64_t extra){
    grow_response_deque_026670(owner,extra,growth_);
}
} // namespace preloader_l::socket_recovered
