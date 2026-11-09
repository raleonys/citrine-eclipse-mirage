// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/pool_allocate.hpp"

namespace citrine_eclipse_mirage::socket {
// Mandatory lower effects, sharing the context's bounded memory. This stage
// reconstructs the pointer-array operation, not heap/copy/free instruction bodies.
class PoolAppendEffects {
public:
    virtual ~PoolAppendEffects()=default;
    virtual std::uintptr_t allocate_04b100(std::uint64_t bytes)=0;
    virtual void copy_078d30(std::uintptr_t destination,std::uintptr_t source,std::uint64_t bytes)=0;
    virtual void free_04b0f8(std::uintptr_t)=0;
};
struct PoolAppendContext {ReceiveBufferMemory& memory;PoolAppendEffects& effects;};
void append_pool_block_005440(std::uintptr_t array,std::uintptr_t block,PoolAppendContext&);

// Implements the N18 append provider. The four N18 pool
// bodies can now use this bridge; block allocation and fill remain mandatory,
// together with this stage's array allocation/copy/free providers.
class PoolAppendEffectsAdapter:public PoolAllocationEffects,public PoolAppendEffects {
public:
    explicit PoolAppendEffectsAdapter(ReceiveBufferMemory& memory):memory_(memory){}
    void append_block_005440(std::uintptr_t array,std::uintptr_t block) final;
private:ReceiveBufferMemory& memory_;
};
} // namespace citrine_eclipse_mirage::socket
