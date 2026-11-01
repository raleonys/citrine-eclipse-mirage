#include "strings.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include <algorithm>
#include <limits>

namespace anadius::session_profile {
namespace {
std::uintptr_t word(std::span<const std::uint8_t> b){std::uintptr_t n=0;for(unsigned i=0;i<8;++i)n|=std::uintptr_t(b[i])<<(i*8);return n;}
}
Strings::Strings(s::SessionMemoryDomain& memory,std::size_t n):memory_(memory),headers_(n){if(!n||n>1048576)throw std::invalid_argument("Bounded header capacity required");}
std::string Strings::read(const s::ReceiveBufferString& value)const{
    auto& context=memory_.buffer_context();
    if(value.size_10>value.capacity_18)throw s::UnsupportedInput("String size exceeds capacity");
    if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
    const auto bytes=context.memory.view(s::receive_buffer_address(value),value.size_10);
    return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
}
void Strings::prune(){
    auto& context=memory_.buffer_context();
    for(auto& h:headers_)if(h.address){
        unsigned matches=0;
        for(const auto& region:context.memory.regions)if(h.address>=region.address){
            const auto offset=h.address-region.address;
            if(offset<=region.bytes.size()&&8<=region.bytes.size()-offset)++matches;
        }
        if(!matches){h={};continue;}
        if(matches!=1)throw s::UnsupportedInput("Ambiguous live allocation header region");
        if(word(context.memory.view(h.address,8))!=h.allocation_base)h={};
    }
}
void Strings::track(const s::ReceiveBufferString& value){
    if(value.capacity_18<=15||value.capacity_18<4095)return;
    if(value.capacity_18==std::numeric_limits<std::uint64_t>::max())throw s::UnsupportedInput("Invalid owned string capacity");
    auto& context=memory_.buffer_context();const auto data=s::receive_buffer_address(value);
    const auto address=data-std::uintptr_t{8};const auto base=word(context.memory.view(address,8));
    if(data-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
    prune();
    for(auto& h:headers_)if(h.address==address){h={address,base};return;}
    for(auto& h:headers_)if(!h.address){h={address,base};return;}
    throw std::length_error("Owned string header registry is full");
}
void Strings::free_sized(std::uintptr_t address,std::uint64_t bytes){
    (void)memory_.buffer_context();
    // Remove metadata before freeing storage or invoking its provider callbacks.
    for(auto& h:headers_)if(h.address&&(h.allocation_base==address||h.address+std::uintptr_t{8}==address))h={};
    memory_.free_sized_04b0f8(address,bytes);
}
void Strings::release(s::ReceiveBufferString& value){
    auto& context=memory_.buffer_context();const auto capacity=value.capacity_18;
    if(capacity>15){
        if(capacity==std::numeric_limits<std::uint64_t>::max())throw s::UnsupportedInput("Invalid owned string capacity");
        auto bytes=capacity+std::uint64_t{1};auto address=s::receive_buffer_address(value);
        if(bytes>=4096){const auto base=word(context.memory.view(address-std::uintptr_t{8},8));if(address-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();address=base;bytes+=39;}
        free_sized(address,bytes);
    }
    value={};
}
void Strings::assign(s::ReceiveBufferString& value,std::string_view text){
    auto& context=memory_.buffer_context();s::ReceiveBufferString fresh;
    if(text.size()<=15){fresh.size_10=text.size();std::copy(text.begin(),text.end(),fresh.storage_00.begin());fresh.storage_00[text.size()]=0;}
    else{
        // Empty source avoids an unregistered local SSO read in008350. This is
        // a selected replacement contract, not native033d70 prefix semantics.
        s::resize_receive_buffer_010040(fresh,text.size(),0,context);
        auto bytes=context.memory.view(s::receive_buffer_address(fresh),text.size());std::copy(text.begin(),text.end(),bytes.begin());track(fresh);
    }
    release(value);value=fresh;
}
s::ReceiveBufferString& Strings::copy(s::ReceiveBufferString& out,const s::ReceiveBufferString& from){
    if(&out==&from)throw s::UnsupportedInput("Response copy requires disjoint prefixes");
    auto& result=s::copy_response_string_009eb0(out,from,memory_.buffer_context());track(result);return result;
}
std::span<s::WriteFinishAllocationHeader> Strings::bindings(){return headers_;}
std::size_t Strings::live_headers()const{return std::count_if(headers_.begin(),headers_.end(),[](const auto& h){return h.address!=0;});}
} // namespace anadius::session_profile
