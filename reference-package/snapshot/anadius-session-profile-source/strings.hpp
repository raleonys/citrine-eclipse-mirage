#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include <vector>

namespace anadius::session_profile {
namespace s=socket_recovered;
// Bounded selected string adapter. Fresh allocation before old release; this
// normal-return bridge does not implement native033d70 intermediate/EH order.
// Parent Operation is required. Header span never reallocates after construction.
class Strings final {
public:
    Strings(s::SessionMemoryDomain&,std::size_t header_capacity);
    std::string read(const s::ReceiveBufferString&)const;
    void assign(s::ReceiveBufferString&,std::string_view);
    s::ReceiveBufferString& copy(s::ReceiveBufferString&,const s::ReceiveBufferString&);
    void track(const s::ReceiveBufferString&);
    void release(s::ReceiveBufferString&);
    void free_sized(std::uintptr_t,std::uint64_t);
    void prune();
    std::span<s::WriteFinishAllocationHeader> bindings();
    std::size_t live_headers()const;
    const s::SessionMemoryDomain* domain_identity()const noexcept{return &memory_;}
private:s::SessionMemoryDomain& memory_;std::vector<s::WriteFinishAllocationHeader> headers_;
};
} // namespace anadius::session_profile
