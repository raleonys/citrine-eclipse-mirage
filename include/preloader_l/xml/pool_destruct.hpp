#pragma once
#include "preloader_l/xml/pool_clear.hpp"

namespace preloader_l::socket_recovered {
// Reuse the existing pointer-only document free boundary and bounded memory.
// Vtable immediates below are stored identities, never callable image pointers.
using NativePoolDestructContext=NativePoolClearContext;
void destroy_native_pool_001bb0(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001c30(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001cb0(std::uintptr_t pool,NativePoolDestructContext&);
void destroy_native_pool_001d30(std::uintptr_t pool,NativePoolDestructContext&);

class RecoveredPoolDestructEffects:public RecoveredPoolClearEffects {
public:
    RecoveredPoolDestructEffects(ReceiveBufferMemory& memory,NativeNodeClearEffects& nodes)
        :RecoveredPoolClearEffects(memory,nodes),pool_destruct_{memory,*this}{}
    void destroy_pool_001bb0(std::uintptr_t) final;
    void destroy_pool_001c30(std::uintptr_t) final;
    void destroy_pool_001cb0(std::uintptr_t) final;
    void destroy_pool_001d30(std::uintptr_t) final;
private:NativePoolDestructContext pool_destruct_;
};
} // namespace preloader_l::socket_recovered
