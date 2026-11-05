#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include <functional>
#include <memory>

namespace anadius::owned_deque {
namespace s=socket_recovered;
enum class Phase {Ready,Growing,Poisoned,Closed,Retiring};
// Concurrent healthy calls keep phase Ready; active aggregates their own call
// and admission rows. Failed retained descriptors remain Poisoned, not active.
struct Snapshot {Phase phase;std::size_t successful_growths,allocated_maps;bool active;};
// Owns the growth provider and a Parent lease, not the output maps. Parent's
// authoritative typed-map ledger covers initial/grown/directly disposed maps.
// Construct/grow/first close on the bound worker under its whole Operation.
// Checkpoint is a nonmutating source-admission probe, not an owner/map/refcount
// mutation callback. Complete source admission must be checked; no SDK/job drain is
// implied. Unknown calls retain descriptors, not caller stack-prefix lifetimes.
// Concurrent Parent policy gives distinct queue/actor calls private adapters;
// it does not synchronize arbitrary writes to one queue or admit upper actors.
class Deques final {
public:
    explicit Deques(s::SessionMemoryDomain&);
    ~Deques()noexcept;
    Deques(const Deques&)=delete;Deques& operator=(const Deques&)=delete;
    void grow(s::WriteFinishOwner&,std::uint64_t extra,std::function<void()> checkpoint);
    // Internal callable sinks are emptied before final retirement checks.
    // Caller-held copies/plain-moved donors still need their own backing life.
    // Explicit child route only. Copied permit expires with its exact whole
    // Parent Operation; original construction/close and default grow unchanged.
    void grow(s::WriteFinishOwner&,std::uint64_t extra,std::function<void()> checkpoint,const s::SessionMemoryDomain::JobPermit&);
    // Concurrent active/admitting calls return false without poisoning them.
    bool close(); // seal and release lease only; no map free, cancel or rollback
    Snapshot snapshot()const;
private:
    void grow_impl(s::WriteFinishOwner&,std::uint64_t,std::function<void()>&,const s::SessionMemoryDomain::JobPermit*);
    void grow_concurrent(s::WriteFinishOwner&,std::uint64_t,std::function<void()>&,const s::SessionMemoryDomain::JobPermit*);
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::owned_deque
