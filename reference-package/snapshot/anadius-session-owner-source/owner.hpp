#pragma once
#include "../anadius-socket-move-source/move.hpp"
#include "../anadius-socket-source/frame_read.hpp"
#include "../anadius-socket-source/cipher_session.hpp"

namespace anadius::socket_recovered {
struct OutputDeque;
struct DequeProxy {OutputDeque* owner_00=nullptr;std::uintptr_t zero_08=0;};
struct OutputDeque {DequeProxy* proxy_c8=nullptr;WriteFinishOwner fields;};
// Live typed projections. Identity+native offsets are routing labels only;
// never cast these objects onto a native MSVC session/control allocation.
struct SessionOwner {
    FrameReadOwner weak_self;
    AcceptorState socket_state;SocketObject socket{socket_state};
    ReceiveBufferString input;OutputDeque output;SessionCipher cipher;
    std::uintptr_t identity()const noexcept{return reinterpret_cast<std::uintptr_t>(this);}
    SessionOwner()=default;
    SessionOwner(const SessionOwner&)=delete;SessionOwner& operator=(const SessionOwner&)=delete;
};
struct SessionControl:IoReference {
    std::uintptr_t table_00=0; // native084750 identity, not an executable vptr
    SessionOwner object;
    // Required dispose/delete providers. Dispose releases object's owned
    // weak-self before the outer implicit-weak decrement. Native026570/
    // 017420/026290 bodies and their native allocation ABI remain unclaimed.
};
// Plain construction output slot, not RAII ownership: native stores overwrite
// it without releasing any prior control value.
struct SessionPair {SessionOwner* object=nullptr;IoReference* control=nullptr;};
class SessionAllocator {
public:
    virtual ~SessionAllocator()=default;
    // Native requests, not sizeof(host projection). Both are nonnull-or-throw.
    // Allocator retains partial allocations across a construction exception.
    virtual SessionControl* allocate_control_04b0bc(std::uint64_t native_bytes)=0;
    virtual DequeProxy* allocate_proxy_04b0bc(std::uint64_t native_bytes)=0;
};
// Caller owns stable scratch until success or separately proven cleanup.
// A throwing lower may have linked this temporary; no stack-dangling sidecar
// or guessed native EH rollback. Flags denote completed normal phases only.
struct ConstructionScratch {
    AcceptorState temporary_state;SocketObject temporary{temporary_state};
    SessionControl* allocation=nullptr;
    bool begun=false,temporary_live=false,owner_socket_live=false;
    bool proxy_live=false,published=false,complete=false;
    ConstructionScratch()=default;
    ConstructionScratch(const ConstructionScratch&)=delete;
    ConstructionScratch& operator=(const ConstructionScratch&)=delete;
};
struct ConstructionContext {
    SessionAllocator& allocation;ExecutorMove executor;LifecycleEffects& lifecycle;
    ConstructionScratch& scratch;
};
// Normal023000 body: explicit output stores occur AFTER temporary024db0 and
// BEFORE callback-sensitive self-weak replacement. Serialized live providers,
// disjoint output/source/storage and single-use scratch are required.
SessionPair& construct_session_owner_023000(SessionPair& output,SocketObject& pending,ConstructionContext&);
} // namespace anadius::socket_recovered
