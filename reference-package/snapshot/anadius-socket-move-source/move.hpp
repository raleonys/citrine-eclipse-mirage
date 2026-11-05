#pragma once
#include "../anadius-socket-source/lifecycle.hpp"
#include "../anadius-socket-source/service_registry.hpp"

namespace anadius::socket_recovered {
// Required selected executor table78 slot+8 operation on the live payloads.
// table78 is an opaque identity, never invoked as an original image address.
// Initialized084928 selects0228c0; the selected body is implemented below.
struct ExecutorMove {
    void* context=nullptr;
    void (*move)(void*,ExecutorFields& destination,ExecutorFields& source,
                 std::uintptr_t table78)=nullptr;
};
// Native0228c0 copies payload+0/context and payload+0xc/zero_74, then writes
// destination+0x18=self. No source reset, table copies, allocation or SDK call.
// These are typed fields; no original object layout is overlaid here.
void move_executor_0228c0(ExecutorFields& destination,const ExecutorFields& source) noexcept;
// ONLY the observed084928 executor: slot0 ->017a30 RET0 and slot8 ->0228c0.
// Reject other table identities before effects instead of treating arbitrary
// executors as no-op owners. The source image/code lifetime remains external.
class RecoveredSocketExecutor final:public RegistryExecutorEffects {
public:
    static constexpr std::uintptr_t table_identity=0x180084928ULL;
    void executor_cleanup(SocketObject&,std::uintptr_t table78) override;
    static ExecutorMove move() noexcept;
private:
    static void invoke_move(void*,ExecutorFields&,ExecutorFields&,std::uintptr_t);
};
// Existing typed projections, not native layout overlays. Both objects AND
// their AcceptorState backing must be distinct and live. Destination must be
// fresh/unlinked; source/service/control and executor effects are serialized.
// SDK gate and control callbacks retain their existing source contracts.
void transfer_socket_028950(SocketService&,SocketObject& destination,SocketObject& source);
SocketObject& move_socket_023d20(SocketObject& destination,SocketObject& source,ExecutorMove);
} // namespace anadius::socket_recovered
