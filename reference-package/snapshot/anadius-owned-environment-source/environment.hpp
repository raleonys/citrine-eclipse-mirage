#pragma once
#include "../anadius-session-profile-source/profile.hpp"

namespace anadius::owned_environment {
namespace s=socket_recovered;namespace sp=session_profile;namespace rt=server_runtime;
enum class Phase {Fresh,Bound,Closing,Closed,Poisoned};
struct Snapshot {Phase phase;std::size_t environments,active_calls;std::uint64_t created,moved,copied,destroyed,handlers,responses;std::uintptr_t token;};
// Selected C++ environment/executor policy, not native0228c0/RTTI/vtable/EH ABI.
// Owned handlers execute inline on the admitted worker; responses use actual
//022e40 with force-post1. Context-free thunks resolve a registered capability,
// never a uintptr->object cast or a current/TLS singleton.
class Environment final {
public:
    explicit Environment(std::size_t capacity=4096);
    ~Environment()noexcept;
    Environment(const Environment&)=delete;Environment& operator=(const Environment&)=delete;
    sp::Environment configuration()const; // passive; backing owned by this object
    void bind(rt::Ports,std::function<void()> nonmutating_admission);
    void session(s::IoHandlerEnvironment& out,sp::SessionView);
    void accept(s::IoHandlerEnvironment& out,std::uintptr_t allocator_identity);
    void release(s::IoHandlerEnvironment&); // exact registered projection or empty
    // Whole Parent Operation. The bool is a TRUSTED external drain attestation;
    // refs0 alone is insufficient. Actual memory/IO/accept/work/queues must also
    // be zero before thunk/code backing and Parent lease are retired.
    bool close(bool externally_drained);
    Snapshot snapshot()const;
private:struct Control;std::shared_ptr<Control> control_;
};
} // namespace anadius::owned_environment
