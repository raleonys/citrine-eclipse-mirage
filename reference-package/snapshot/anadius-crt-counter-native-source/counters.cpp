#include "counters.hpp"
#include <bit>
#include <stdexcept>

namespace anadius::crt_counter_native {
static_assert(sizeof(Address)==sizeof(std::uintptr_t));
static_assert(sizeof(LONG)==sizeof(std::uint32_t));
static_assert(alignof(LONG)>=4);
namespace {
Address address(LONG& value) noexcept{return reinterpret_cast<Address>(&value);}
LONG signed_bits(std::uint32_t value) noexcept{return std::bit_cast<LONG>(value);}
std::uint32_t unsigned_bits(LONG value) noexcept{return std::bit_cast<std::uint32_t>(value);}
}
Address Counters::bind(LONG& value){
    const auto identity=address(value);
    if((identity&3)!=0)throw std::invalid_argument("Native LONG counter is not 32-bit aligned");
    if(!values_.emplace(identity,&value).second)
        throw std::logic_error("Native LONG counter is already bound");
    return identity;
}
void Counters::unbind(LONG& value){
    const auto it=values_.find(address(value));
    if(it==values_.end()||it->second!=&value)
        throw std::out_of_range("Native LONG counter is not bound");
    values_.erase(it);
}
bool Counters::contains(Address identity) const noexcept{return values_.find(identity)!=values_.end();}
LONG* Counters::find(Address identity) const{
    const auto it=values_.find(identity);
    if(it==values_.end())throw std::out_of_range("Unknown native LONG counter identity");
    return it->second;
}
std::uint32_t Counters::fetch_add(Address identity,std::uint32_t increment){
    return unsigned_bits(::InterlockedExchangeAdd(find(identity),signed_bits(increment)));
}
std::uint32_t Counters::exchange(Address identity,std::uint32_t value){
    return unsigned_bits(::InterlockedExchange(find(identity),signed_bits(value)));
}
std::uint32_t Counters::load(Address identity) const{
    return unsigned_bits(::InterlockedCompareExchange(find(identity),0,0));
}
}
