// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/pool_clear.hpp"

namespace laureondo::socket {
// Reuse the existing pointer-only document free boundary and bounded memory.
// Vtable immediates below are stored identities, never callable image pointers.
using NativePoolDestructContext=NativePoolClearContext;
void destroy_native_pool_001bb0(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001c30(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001cb0(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001d30(std::uintptr_t pool,NativePoolDestructContext&);

class PoolDestructEffects:public PoolClearEffects {
public:
    PoolDestructEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :PoolClearEffects(memory,nodes),pool_destruct_{memory,*this}{}
    void destroy_pool_001bb0(std::uintptr_t) final;
    void destroy_pool_001c30(std::uintptr_t) final;
    void destroy_pool_001cb0(std::uintptr_t) final;
    void destroy_pool_001d30(std::uintptr_t) final;
private:NativePoolDestructContext pool_destruct_;
};
} // namespace laureondo::socket
