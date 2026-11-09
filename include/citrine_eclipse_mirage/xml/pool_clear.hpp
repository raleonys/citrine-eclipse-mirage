// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/document_clear.hpp"

namespace citrine_eclipse_mirage::socket {
// Keep the existing pointer-only free provider. Observed caller EDX block-size
// hints are retained as static facts, not a new sized-free ABI or heap body.
struct NativePoolClearContext {ReceiveBufferMemory& memory;NativeDocumentEffects& effects;};
void clear_pool_blocks_002d70(std::uintptr_t pool,NativePoolClearContext&);
void clear_pool_blocks_002dc0(std::uintptr_t pool,NativePoolClearContext&);
void clear_pool_blocks_002e10(std::uintptr_t pool,NativePoolClearContext&);

// Retain the earlier document/node-clear source bindings and close exactly the
// three existing pool-clear providers. Pool destructors and free stay required.
class PoolClearEffects:public DocumentClearEffects {
public:
    PoolClearEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :DocumentClearEffects(memory,nodes),pool_clear_{memory,*this}{}
    void clear_pool_002d70(std::uintptr_t) final;
    void clear_pool_002dc0(std::uintptr_t) final;
    void clear_pool_002e10(std::uintptr_t) final;
private:NativePoolClearContext pool_clear_;
};
} // namespace citrine_eclipse_mirage::socket
