#include "pool_release.hpp"

namespace anadius::socket_recovered {
namespace {
std::uint64_t load(ReceiveBufferMemory& memory,std::uintptr_t address,unsigned width){
    const auto bytes=memory.view(address,width);std::uint64_t value=0;
    for(unsigned i=0;i<width;++i)value|=std::uint64_t{bytes[i]}<<(i*8);
    return value;
}
void store(ReceiveBufferMemory& memory,std::uintptr_t address,std::uint64_t value,unsigned width){
    auto bytes=memory.view(address,width);
    for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));
}
}
void release_pool_item_0033a0(std::uintptr_t pool,std::uintptr_t item,ReceiveBufferMemory& memory){
    if(!item)return;
    const auto root=load(memory,pool+0x68,8);
    const auto count=static_cast<std::uint32_t>(load(memory,pool+0x70,4));
    store(memory,pool+0x70,count-std::uint32_t{1},4);
    store(memory,item,root,8);
    store(memory,pool+0x68,item,8);
}
void track_pool_005a20(std::uintptr_t pool,ReceiveBufferMemory& memory){
    const auto count=static_cast<std::uint32_t>(load(memory,pool+0x7c,4));
    store(memory,pool+0x7c,count-std::uint32_t{1},4);
}
void RecoveredPoolReleaseEffects::pool_release(std::uintptr_t target,std::uintptr_t pool,std::uintptr_t item){
    if(target!=targets_.release)throw UnsupportedInput("Pool release target is outside the selected source binding");
    release_pool_item_0033a0(pool,item,memory_);
}
void RecoveredPoolReleaseEffects::pool_track(std::uintptr_t target,std::uintptr_t pool){
    if(target!=targets_.track)throw UnsupportedInput("Pool tracking target is outside the selected source binding");
    track_pool_005a20(pool,memory_);
}
} // namespace anadius::socket_recovered
