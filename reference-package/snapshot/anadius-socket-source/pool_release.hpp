#pragma once
#include "node_delete.hpp"

namespace anadius::socket_recovered {
void release_pool_item_0033a0(std::uintptr_t pool,std::uintptr_t item,ReceiveBufferMemory&);
void track_pool_005a20(std::uintptr_t pool,ReceiveBufferMemory&);
// Default targets are observed in four shipped MemPoolT vtables. Values are
// dispatch identities only, never cast to callable image function pointers.
struct PoolTargetIdentities {std::uintptr_t release=0x1800033a0ULL,track=0x180005a20ULL;};
class RecoveredPoolReleaseEffects:public RecoveredNodeDeleteEffects {
public:
    explicit RecoveredPoolReleaseEffects(ReceiveBufferMemory& memory,PoolTargetIdentities targets={})
        :RecoveredNodeDeleteEffects(memory),memory_(memory),targets_(targets){}
    void pool_release(std::uintptr_t target,std::uintptr_t pool,std::uintptr_t item) final;
    void pool_track(std::uintptr_t target,std::uintptr_t pool) final;
private:ReceiveBufferMemory& memory_;PoolTargetIdentities targets_;
};
} // namespace anadius::socket_recovered
