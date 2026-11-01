#pragma once
#include "document_clear.hpp"

namespace anadius::socket_recovered {
struct NativeNodeDeleteContext {ReceiveBufferMemory& memory;NativeNodeClearEffects& effects;};
// Bounded normal projections. Virtual probe/destructor/pool release remain
// mandatory effects; the native tail-call return/stack ABI is not projected.
void unlink_native_node_003030(std::uintptr_t parent,std::uintptr_t node,NativeNodeDeleteContext&);
void delete_native_node_003240(std::uintptr_t node,NativeNodeDeleteContext&);
class RecoveredNodeDeleteEffects:public NativeNodeClearEffects {
public:
    explicit RecoveredNodeDeleteEffects(ReceiveBufferMemory& memory):nodes_{memory,*this}{}
    RecoveredNodeDeleteEffects(const RecoveredNodeDeleteEffects&)=delete;
    RecoveredNodeDeleteEffects& operator=(const RecoveredNodeDeleteEffects&)=delete;
    RecoveredNodeDeleteEffects(RecoveredNodeDeleteEffects&&)=delete;
    RecoveredNodeDeleteEffects& operator=(RecoveredNodeDeleteEffects&&)=delete;
    void remove_parent_003030(std::uintptr_t parent,std::uintptr_t node) final;
    void delete_node_003240(std::uintptr_t node) final;
private:NativeNodeDeleteContext nodes_;
};
} // namespace anadius::socket_recovered
