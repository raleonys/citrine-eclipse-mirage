#include "bridge.hpp"
namespace anadius::crt_recalloc_bridge {
Address Allocator::calloc(std::uint64_t count,std::uint64_t size){
    return reinterpret_cast<Address>(crt_heap_recovered::calloc_base_066848(count,size,state_,effects_));
}
Address Allocator::recalloc(Address old,std::uint64_t count,std::uint64_t size){
    return reinterpret_cast<Address>(crt_recalloc_recovered::recalloc_base_070434(
        reinterpret_cast<void*>(old),count,size,state_,effects_));
}
void Allocator::free(Address address){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(address),state_,effects_);}
Address WildcardHeapEffects::calloc_066848(std::uint64_t count,std::uint64_t size){return allocator_.calloc(count,size);}
Address WildcardHeapEffects::recalloc_070434(Address old,std::uint64_t count,std::uint64_t size){return allocator_.recalloc(old,count,size);}
void WildcardHeapEffects::free_0668c0(Address address){allocator_.free(address);}
Address OnexitHeapEffects::recalloc_070434(Address old,std::uint64_t count,std::uint64_t size){return allocator_.recalloc(old,count,size);}
void OnexitHeapEffects::free_0668c0(Address address){allocator_.free(address);}
}
