#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include <vector>

namespace anadius::session_profile {
namespace s=socket_recovered;
// Selected string adapter. Fresh allocation before old release; this
// normal-return bridge does not implement native033d70 intermediate/EH order.
// Parent Operation is required. Serialized preserves its legacy header ledger.
// Concurrent uses the exact Parent header resolver, never a mutable union
// ledger; actual payload ownership/last-use transfer remains explicit.
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
    // Null in Serialized mode; Concurrent obtains this exact Parent owner only
    // from an admitted operation. Selected contexts use it authoritatively.
    s::AllocationHeaderResolver* header_resolver()const;
    std::string read(const s::ReceiveBufferString&)const;
    void assign(s::ReceiveBufferString&,std::string_view);
    s::ReceiveBufferString& copy(s::ReceiveBufferString&,const s::ReceiveBufferString&);
    void track(const s::ReceiveBufferString&);
    void release(s::ReceiveBufferString&);
    void free_sized(std::uintptr_t,std::uint64_t);
    // Before another actor becomes eligible to own/mutate/free a moved heap
    // string, end ALL producer aliases and relinquish its whole-ticket view.
    // Does not clear the prefix, free payload, or release SDK/foreign borrows.
    // Serialized is a compatibility no-op; SSO owns no Parent heap ticket.
    void relinquish(const s::ReceiveBufferString&);
    void prune();
    // Serialized-only legacy observations. Concurrent explicitly rejects these
    // APIs rather than fabricating empty catalogs/zero drain counts. Actual
    // ownership/drain is observed through Parent's authoritative allocation
    // ledger. No copied span may survive a Serialized ledger growth.
    std::span<s::WriteFinishAllocationHeader> bindings();
    std::size_t live_headers()const;
    const s::SessionMemoryDomain* domain_identity()const noexcept{return &memory_;}
private:
    void publish_header_views() noexcept;
    s::SessionMemoryDomain& memory_;const bool authoritative_;
    std::vector<s::WriteFinishAllocationHeader> headers_;
    std::span<s::WriteFinishAllocationHeader>* frame_headers_=nullptr;
    std::span<s::WriteFinishAllocationHeader>* finish_headers_=nullptr;
};
} // namespace anadius::session_profile
