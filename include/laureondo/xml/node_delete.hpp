// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/document_clear.hpp"

namespace laureondo::socket {
struct NativeNodeDeleteContext {ReceiveBufferMemory& memory;NativeNodeClearEffects& effects;};
// Bounded normal projections. Virtual probe/destructor/pool release remain
// mandatory effects; the native tail-call return/stack ABI is not projected.
void unlink_native_node_003030(std::uintptr_t parent,std::uintptr_t node,NativeNodeDeleteContext&);
void delete_native_node_003240(std::uintptr_t node,NativeNodeDeleteContext&);
class NodeDeleteEffects:public NativeNodeClearEffects {
public:
    explicit NodeDeleteEffects(ReceiveBufferMemory& memory):nodes_{memory,*this}{}
    NodeDeleteEffects(const NodeDeleteEffects&)=delete;
    NodeDeleteEffects& operator=(const NodeDeleteEffects&)=delete;
    NodeDeleteEffects(NodeDeleteEffects&&)=delete;
    NodeDeleteEffects& operator=(NodeDeleteEffects&&)=delete;
    void remove_parent_003030(std::uintptr_t parent,std::uintptr_t node) final;
    void delete_node_003240(std::uintptr_t node) final;
private:NativeNodeDeleteContext nodes_;
};
} // namespace laureondo::socket
