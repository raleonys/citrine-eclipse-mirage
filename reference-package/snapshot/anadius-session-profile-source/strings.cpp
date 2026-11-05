#include "strings.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include <algorithm>
#include <limits>

namespace anadius::session_profile {
namespace {
std::uintptr_t word(std::span<const std::uint8_t> b){std::uintptr_t n=0;for(unsigned i=0;i<8;++i)n|=std::uintptr_t(b[i])<<(i*8);return n;}
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
}
Strings::Strings(s::SessionMemoryDomain& memory,std::size_t n):memory_(memory),authoritative_(memory.operation_policy()==s::OperationPolicy::OriginalConcurrentActors),headers_(authoritative_?0:n){if(!n||n>1048576)throw std::invalid_argument("Bounded initial header capacity required");}
void Strings::publish_header_views() noexcept {
    const std::span<s::WriteFinishAllocationHeader> current=headers_;
    if(frame_headers_)*frame_headers_=current;
    if(finish_headers_)*finish_headers_=current;
}
void Strings::bind_header_views(std::span<s::WriteFinishAllocationHeader>& frame,
                                std::span<s::WriteFinishAllocationHeader>& finish){
    if(authoritative_)throw std::logic_error("Concurrent strings require exact header resolver, not bound ledger spans");
    if(frame_headers_||finish_headers_||&frame==&finish)throw std::logic_error("Distinct header views may be bound only once");
    frame_headers_=&frame;finish_headers_=&finish;publish_header_views();
}
s::AllocationHeaderResolver* Strings::header_resolver()const{
    return authoritative_?&memory_.allocation_header_resolver():nullptr;
}
std::string Strings::read(const s::ReceiveBufferString& value)const{
    auto& context=memory_.buffer_context();
    if(value.size_10>value.capacity_18)throw s::UnsupportedInput("String size exceeds capacity");
    if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
    const auto bytes=context.memory.view(s::receive_buffer_address(value),value.size_10);
    return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
}
void Strings::prune(){
    auto& context=memory_.buffer_context();
    // No Concurrent ledger exists. In particular, do not scan or pin another
    // actor's headers merely for housekeeping or an apparent drain count.
    if(authoritative_)return;
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
    if(authoritative_)(void)memory_.buffer_context();
    if(value.capacity_18<=15||value.capacity_18<4095)return;
    if(value.capacity_18==std::numeric_limits<std::uint64_t>::max())throw s::UnsupportedInput("Invalid owned string capacity");
    auto& context=memory_.buffer_context();const auto data=s::receive_buffer_address(value);
    const auto address=data-std::uintptr_t{8};std::uintptr_t base=0;
    if(authoritative_){
        const auto match=header_resolver()->lookup_header(address,base);
        if(match==s::MemoryLookup::Ambiguous)throw s::UnsupportedInput("Ambiguous live allocation header region");
        if(match!=s::MemoryLookup::Found)throw s::UnsupportedInput("Owned string header is not a live Parent allocation");
    }else base=word(context.memory.view(address,8));
    if(data-base-std::uintptr_t{8}>31)context.api.invalid_parameter_05c974();
    if(authoritative_)return; // Exact requested ticket is the sole authority.
    prune();
    for(auto& h:headers_)if(h.address==address){h={address,base};return;}
    for(auto& h:headers_)if(!h.address){h={address,base};return;}
    PreserveError error; // Only extra metadata allocation/free preserves API state.
    const auto count=headers_.size(),limit=headers_.max_size();
    if(count==limit)throw std::length_error("Owned string header ledger exceeds vector capacity");
    const auto grown=count>limit-count?limit:count*2;
    const auto* previous=headers_.data();
    // Allocation can fail after the source payload allocation completed. Keep
    // the old ledger/views intact and let the existing source guard retain that
    // prefix; neither replay nor free the completed payload here.
    std::vector<s::WriteFinishAllocationHeader> replacement(grown);
    (void)memory_.buffer_context();
    if(headers_.size()!=count||headers_.data()!=previous)throw std::logic_error("Header ledger changed during growth allocation");
    // An allocator callback may have changed rows without moving the table.
    // Reuse its newly published duplicate/free slot instead of adding a second
    // row for the same native header or unnecessarily replacing live views.
    prune();
    for(auto& h:headers_)if(h.address==address){h={address,base};return;}
    for(auto& h:headers_)if(!h.address){h={address,base};return;}
    std::copy(headers_.begin(),headers_.end(),replacement.begin());
    replacement[count]={address,base};
    headers_.swap(replacement);
    // No opaque operation between swap and these noexcept field assignments.
    // Publish BEFORE replacement destroys the old metadata allocation.
    publish_header_views();
}
void Strings::free_sized(std::uintptr_t address,std::uint64_t bytes){
    (void)memory_.buffer_context();
    // Remove metadata before freeing storage or invoking its provider callbacks.
    if(!authoritative_)for(auto& h:headers_)if(h.address&&(h.allocation_base==address||h.address+std::uintptr_t{8}==address))h={};
    memory_.free_sized_04b0f8(address,bytes);
}
void Strings::relinquish(const s::ReceiveBufferString& value){
    if(!authoritative_)return;
    (void)memory_.buffer_context();
    if(value.size_10>value.capacity_18)throw s::UnsupportedInput("Transferred string size exceeds capacity");
    if(value.capacity_18<=15)return;
    if(value.capacity_18==std::numeric_limits<std::uint64_t>::max())throw s::UnsupportedInput("Invalid transferred string capacity");
    memory_.relinquish_operation_view(s::receive_buffer_address(value),value.capacity_18+std::uint64_t{1});
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
std::span<s::WriteFinishAllocationHeader> Strings::bindings(){
    if(authoritative_)throw std::logic_error("Concurrent strings have no borrowed header ledger; use the exact resolver");
    return headers_;
}
std::size_t Strings::live_headers()const{
    if(authoritative_)throw std::logic_error("Concurrent string drain requires the authoritative Parent ledger, not a legacy header count");
    return std::count_if(headers_.begin(),headers_.end(),[](const auto& h){return h.address!=0;});
}
} // namespace anadius::session_profile
