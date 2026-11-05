#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include <vector>

namespace anadius::session_profile {
namespace s=socket_recovered;
// Selected string adapter. Fresh allocation before old release; this
// normal-return bridge does not implement native033d70 intermediate/EH order.
// Parent Operation is required. Header capacity is an initial allocation;
// selected Profile contexts bind live span fields that follow ledger growth.
class Strings final {
public:
    Strings(s::SessionMemoryDomain&,std::size_t header_capacity);
    Strings(const Strings&)=delete;
    Strings& operator=(const Strings&)=delete;
    Strings(Strings&&)=delete;
    Strings& operator=(Strings&&)=delete;
    // Passive once-only binding before publication. Both destination fields
    // belong to this Strings owner's stable Profile and outlive ledger use.
    void bind_header_views(std::span<s::WriteFinishAllocationHeader>& frame,
                           std::span<s::WriteFinishAllocationHeader>& finish);
    std::string read(const s::ReceiveBufferString&)const;
    void assign(s::ReceiveBufferString&,std::string_view);
    s::ReceiveBufferString& copy(s::ReceiveBufferString&,const s::ReceiveBufferString&);
    void track(const s::ReceiveBufferString&);
    void release(s::ReceiveBufferString&);
    void free_sized(std::uintptr_t,std::uint64_t);
    void prune();
    // Snapshot only: retain no copied span across an operation that may grow.
    // Selected Profile consumers instead retain the bound live span fields.
    std::span<s::WriteFinishAllocationHeader> bindings();
    std::size_t live_headers()const;
    const s::SessionMemoryDomain* domain_identity()const noexcept{return &memory_;}
private:
    void publish_header_views() noexcept;
    s::SessionMemoryDomain& memory_;std::vector<s::WriteFinishAllocationHeader> headers_;
    std::span<s::WriteFinishAllocationHeader>* frame_headers_=nullptr;
    std::span<s::WriteFinishAllocationHeader>* finish_headers_=nullptr;
};
} // namespace anadius::session_profile
