#pragma once
#include "pool_destruct.hpp"

namespace anadius::socket_recovered {
struct NativeNodeBaseContext {ReceiveBufferMemory& memory;NativeDocumentEffects& effects;};
void destroy_native_node_base_001ea0(std::uintptr_t node,NativeNodeBaseContext&);

// Retain all earlier document/node/pool source bridges. The already-bound
// delete_nodes provider is reused; pointer free and node virtual effects remain
// mandatory. The stored07fa98 identity is never called as a host vtable.
class RecoveredNodeBaseEffects:public RecoveredPoolDestructEffects {
public:
    RecoveredNodeBaseEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :RecoveredPoolDestructEffects(memory,nodes),node_base_{memory,*this}{}
    void destroy_node_base_001ea0(std::uintptr_t) final;
private:NativeNodeBaseContext node_base_;
};
} // namespace anadius::socket_recovered
