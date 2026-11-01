#pragma once
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include <functional>
#include <memory>

namespace anadius::owned_deque {
namespace s=socket_recovered;
enum class Phase {Ready,Growing,Poisoned,Closed,Retiring};
struct Snapshot {Phase phase;std::size_t successful_growths,allocated_maps;bool active;};
// Owns the growth provider and a Parent lease, not the output maps. Parent's
// authoritative typed-map ledger covers initial/grown/directly disposed maps.
// Construct/grow/first close on the bound worker under its whole Operation.
// Checkpoint is a nonmutating source-admission probe, not an owner/map/refcount
// mutation callback. Complete source admission must be checked; no SDK/job drain is
// implied. Unknown calls retain descriptors, not caller stack-prefix lifetimes.
class Deques final {
public:
    explicit Deques(s::SessionMemoryDomain&);
    ~Deques()noexcept;
    Deques(const Deques&)=delete;Deques& operator=(const Deques&)=delete;
    void grow(s::WriteFinishOwner&,std::uint64_t extra,std::function<void()> checkpoint);
    bool close(); // seal and release lease only; no map free, cancel or rollback
    Snapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::owned_deque
