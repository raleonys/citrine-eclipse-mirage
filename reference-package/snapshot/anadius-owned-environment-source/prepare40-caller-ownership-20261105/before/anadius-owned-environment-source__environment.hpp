#pragma once
#include "../anadius-session-profile-source/profile.hpp"

namespace anadius::owned_environment {
namespace s=socket_recovered;namespace sp=session_profile;namespace rt=server_runtime;
enum class Phase {Fresh,Bound,Closing,Closed,Poisoned};
// OriginalDefault preserves the initial084908 constructor optimization and
// models the concrete initial/tracked family transitions through its bindings.
enum class ClassifierProfile {Host,OriginalDefault};
struct Snapshot {Phase phase;std::size_t environments,active_calls;std::uint64_t created,moved,copied,destroyed,handlers,responses;std::uintptr_t token;};
// Selected C++ environment/executor policy, not native0228c0/RTTI/vtable/EH ABI.
// Host handlers execute inline and responses force-post. OriginalDefault uses
// concrete family flags with022e40. Thunks resolve a registered capability,
// never a uintptr->object cast or a current/TLS singleton.
class Environment final {
    struct JobState;
public:
    class JobScope;
    // Initial host reference-ledger storage; live projections can grow it.
    explicit Environment(std::size_t capacity=4096,ClassifierProfile=ClassifierProfile::Host);
    ~Environment()noexcept;
    Environment(const Environment&)=delete;Environment& operator=(const Environment&)=delete;
    sp::Environment configuration()const; // passive; backing owned by this object
    void bind(rt::Ports,std::function<void()> nonmutating_admission);
    void session(s::IoHandlerEnvironment& out,sp::SessionView);
    void accept(s::IoHandlerEnvironment& out,std::uintptr_t allocator_identity);
    void release(s::IoHandlerEnvironment&); // exact registered projection or empty
    // Projection-only child scope. Caller keeps the protected source projection
    // and SessionOwner alive throughout the exact Parent Operation. Every scoped
    // temporary must be destroyed before close; no session/accept/close or
    // handler/response scheduling permission. Partial failure poisons, not cleanup.
    // Concurrent scopes share the same protected source/family/work registry;
    // each operation carries its explicit immutable actor context. Raw exported
    // callbacks remain original-worker operations, never ambient scope selectors.
    JobScope enter_job(const s::IoHandlerEnvironment&,sp::SessionView,const s::SessionMemoryDomain::JobPermit&,std::function<void()> checkpoint);
    // Whole Parent Operation. The bool is a TRUSTED external drain attestation;
    // refs0 alone is insufficient. Actual memory/IO/accept/work/queues must also
    // be zero before thunk/code backing and Parent lease are retired.
    bool close(bool externally_drained);
    Snapshot snapshot()const;
private:struct Control;std::shared_ptr<Control> control_;
};
class Environment::JobScope final {
public:
    ~JobScope()noexcept;
    JobScope(const JobScope&)=delete;JobScope& operator=(const JobScope&)=delete;
    JobScope(JobScope&&)=delete;JobScope& operator=(JobScope&&)=delete;
    s::IoEnvironmentCallOperations& operations();
    // Calls reserve complete input/output addresses before native prefix
    // writes or the raw type query. Empty results retain only scoped address
    // ownership, with no native Row/work/reference retain. Explicit operations
    // enforce that ownership; the raw global empty table is a trusted stateless
    // low-level projection and does not validate/grant child scope permission.
    s::IoHandlerEnvironment& construct_023dd0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& source,
        s::IoHandlerSentinels,s::IoEnvironmentConstructorContext&);
    void copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& source);
    void close(); // same actor, still inside the exact Parent Operation
private:
    friend class Environment;
    JobScope(std::shared_ptr<Control>,std::shared_ptr<JobState>)noexcept;
    std::shared_ptr<Control> control_; // owns the raw backlinks/code in state_
    std::shared_ptr<JobState> state_;
};
} // namespace anadius::owned_environment
