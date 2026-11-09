// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/node_constructors.hpp"

namespace citrine_eclipse_mirage::socket {
struct NativeNodeInsertionContext {ReceiveBufferMemory& memory;NativeNodeClearEffects& nodes;};
// The observed owner argument is unused by003aa0. Linked children are detached
// without destruction; unlinked children leave the outstanding vector and are
// tracked by their live pool. The caller performs insertion afterward.
void prepare_native_insertion_003aa0(std::uintptr_t requested_owner,std::uintptr_t child,
    NativeNodeInsertionContext&);

class NodeInsertionEffects:public NodeConstructorEffects {
public:
    NodeInsertionEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes,
        const NativeErrorArgumentTail& arguments,PoolAllocationDispatch& allocator,
        NativeNodeParseIdentities parse_ids={},NativeNodeIdentifyIdentities identify_ids={},
        NativeNodeConstructorIdentities constructor_ids={})
        :NodeConstructorEffects(memory,nodes,arguments,allocator,parse_ids,identify_ids,constructor_ids),
         insertion_{memory,nodes}{}
    void prepare_003aa0(std::uintptr_t,std::uintptr_t) final;
private:NativeNodeInsertionContext insertion_;
};
} // namespace citrine_eclipse_mirage::socket
