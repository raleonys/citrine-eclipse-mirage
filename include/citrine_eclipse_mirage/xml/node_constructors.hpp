// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/node_identify.hpp"
#include "citrine_eclipse_mirage/xml/pool_append.hpp"

namespace citrine_eclipse_mirage::socket {
struct NativeNodeConstructorIdentities {
    // Ordered by constructor1590,1610,1690,1720,17b0. Values identify image
    // tables only; no original-image function pointer is invoked.
    std::array<std::uintptr_t,5> vtables{
        0x18007fbb8ULL,0x18007fc48ULL,0x18007fd78ULL,0x18007fb28ULL,0x18007fcd8ULL};
};
struct NativeNodeConstructorContext {
    ReceiveBufferMemory& memory;PoolAllocationDispatch& allocator;
    PoolAppendEffects& append;NativeNodeConstructorIdentities identities{};
};
std::uintptr_t construct_node_001590(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext&);
std::uintptr_t construct_node_001610(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext&);
std::uintptr_t construct_node_001690(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext&);
std::uintptr_t construct_node_001720(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext&);
std::uintptr_t construct_node_0017b0(std::uintptr_t document,std::uintptr_t pool,NativeNodeConstructorContext&);

// Reuse the existing pool allocation dispatch and005440 source body. The
// allocator and append heap/copy/free effects remain caller-supplied; this
// binds five constructor providers, not a native heap or generic DOM ABI.
class NodeConstructorEffects:public NodeIdentifyEffects,
    public PoolAppendEffects {
public:
    NodeConstructorEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments,PoolAllocationDispatch& allocator,
        NativeNodeParseIdentities parse_ids={},NativeNodeIdentifyIdentities identify_ids={},
        NativeNodeConstructorIdentities constructor_ids={})
        :NodeIdentifyEffects(memory,nodes,arguments,parse_ids,identify_ids),
         constructors_{memory,allocator,*this,constructor_ids}{}
    std::uintptr_t construct_001590(std::uintptr_t,std::uintptr_t) final;
    std::uintptr_t construct_001610(std::uintptr_t,std::uintptr_t) final;
    std::uintptr_t construct_001690(std::uintptr_t,std::uintptr_t) final;
    std::uintptr_t construct_001720(std::uintptr_t,std::uintptr_t) final;
    std::uintptr_t construct_0017b0(std::uintptr_t,std::uintptr_t) final;
private:NativeNodeConstructorContext constructors_;
};
} // namespace citrine_eclipse_mirage::socket
