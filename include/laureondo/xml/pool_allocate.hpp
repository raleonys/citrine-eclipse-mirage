// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_receive_buffer.hpp"
#include <array>

namespace laureondo::socket {
// Mandatory lower effects: this stage does not recover the heap, memset body,
// or pointer-array append/growth005440. All effects share the context memory.
class PoolAllocationEffects {
public:
    virtual ~PoolAllocationEffects()=default;
    virtual std::uintptr_t allocate_04b0bc(std::uint64_t bytes)=0;
    virtual void fill_0793d0(std::uintptr_t,std::uint8_t,std::uint64_t)=0;
    virtual void append_block_005440(std::uintptr_t array,std::uintptr_t block)=0;
};
struct PoolAllocationData {
    std::uintptr_t cpu_mode=0x1800a1550ULL;
    // Ordered as the four functions below:120,80,112,104 bytes per item.
    std::array<std::uintptr_t,4> stride_constants{
        0x180080100ULL,0x1800800d0ULL,0x1800800f0ULL,0x1800800e0ULL};
    std::uintptr_t increment_constants=0x180080110ULL;
    std::uintptr_t index_constants=0x180080120ULL;
};
struct PoolAllocationContext {
    ReceiveBufferMemory& memory;PoolAllocationEffects& effects;PoolAllocationData data{};
};
std::uintptr_t allocate_pool_item_002b10(std::uintptr_t pool,PoolAllocationContext&);
std::uintptr_t allocate_pool_item_002600(std::uintptr_t pool,PoolAllocationContext&);
std::uintptr_t allocate_pool_item_002960(std::uintptr_t pool,PoolAllocationContext&);
std::uintptr_t allocate_pool_item_0027b0(std::uintptr_t pool,PoolAllocationContext&);

struct PoolAllocationTargetIdentities {
    std::array<std::uintptr_t,4> allocate{
        0x180002b10ULL,0x180002600ULL,0x180002960ULL,0x1800027b0ULL};
};
// This is a new explicit source adapter. No historical node-clear allocation
// provider exists to close. Image values select bodies; they are never called.
class PoolAllocationDispatch {
public:
    virtual ~PoolAllocationDispatch()=default;
    virtual std::uintptr_t pool_allocate(std::uintptr_t target,std::uintptr_t pool)=0;
};
class PoolAllocationEffectsAdapter final:public PoolAllocationDispatch {
public:
    explicit PoolAllocationEffectsAdapter(PoolAllocationContext& context,
        PoolAllocationTargetIdentities targets={}):context_(context),targets_(targets){}
    std::uintptr_t pool_allocate(std::uintptr_t target,std::uintptr_t pool) final;
private:PoolAllocationContext& context_;PoolAllocationTargetIdentities targets_;
};
} // namespace laureondo::socket
