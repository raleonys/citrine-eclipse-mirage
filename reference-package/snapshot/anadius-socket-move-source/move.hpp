#pragma once
#include "../anadius-socket-source/lifecycle.hpp"

namespace anadius::socket_recovered {
// Required selected executor table78 slot+8 operation on the live payloads.
// table78 is an opaque identity, never invoked as an original image address.
// Initialized084928 selects0228c0; its body is not present in retained evidence.
struct ExecutorMove {
    void* context=nullptr;
    void (*move)(void*,ExecutorFields& destination,ExecutorFields& source,
                 std::uintptr_t table78)=nullptr;
};
// Existing typed projections, not native layout overlays. Both objects AND
// their AcceptorState backing must be distinct and live. Destination must be
// fresh/unlinked; source/service/control and executor effects are serialized.
// SDK gate and control callbacks retain their existing source contracts.
void transfer_socket_028950(SocketService&,SocketObject& destination,SocketObject& source);
SocketObject& move_socket_023d20(SocketObject& destination,SocketObject& source,ExecutorMove);
} // namespace anadius::socket_recovered
