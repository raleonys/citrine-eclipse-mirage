#pragma once
#include "../anadius-session-profile-source/profile.hpp"
#include "../anadius-socket-source/native.hpp"
#include "../anadius-services-completion-source/exit_wait.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <memory>

namespace anadius::session_policy {
namespace sp=session_profile;namespace rt=server_runtime;namespace s=socket_recovered;namespace o=owned_server;

// Value/control-flow projection of native asio.misc message slot020a80.
// Exact observed strings for codes1..4 and common fallback for all others.
// Does not reproduce MSVC string layout/capacity, out-parameter or vtable ABI.
using socket_recovered::misc_message_020a80;

enum class LifecycleMode {Explicit,SourceOwned,OriginalApplication};
// Explicit remains the existing required four-callback host profile.
// SourceOwned must be selected deliberately: preserve the observed completion
// owner, use Application's actual reap/prepared-result work and Policy's real
// worker-return validation. after/returned become optional observers. A custom
// owner selector is rejected in that mode; actual Stop receipts still precede
// this policy in Runtime. External drain is mandatory in BOTH modes and zero
// local counts never replace it in those cooperative profiles. OriginalApplication
// selects source decisions and actual tracked worker/IO retirement without an
// external unload proof. It makes no foreign/image lifetime attestation.
// Runtime validates live operation identity.
// SourceOwned gates that proof on local tracked IO/work retirement, under a
// short Parent operation before and after the opaque outside-Parent callback.
struct Lifecycle {
    std::shared_ptr<void> lifetime;
    std::function<s::IocpSchedulerEntry*(sp::Application&,s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)> select_owner;
    std::function<void(sp::Application&)> after;
    std::function<void(sp::Application&,std::uint64_t,const s::WorkerRunResult&)> returned;
    std::function<bool(o::ServerView,sp::Application&)> external_drained;
    // HOST by-value layout change: rebuild Options/Graph/Worker consumers.
    LifecycleMode mode=LifecycleMode::Explicit;
    // Exact Runtime Stop, bound by ApplicationWorker for SourceOwned. Optional
    // only for standalone lower Policy/Profile composition. Worker production
    // composition requires SessionIo/SessionIoPrepared and checks pointer identity.
    // Owns local packet/receipt prerequisite, never the real external proof.
    std::shared_ptr<accept_stop::Stop> source_stop;
};
// Shared passive profile validation for Policy and its Graph assembly. Keep
// their admission rules identical; no SDK, callback or provider call occurs.
bool lifecycle_configuration_valid(const Lifecycle&)noexcept;
struct Options {
    std::shared_ptr<shared_log::Stream> stream;
    Lifecycle lifecycle;
    // Explicit fresh zero seed; SAME stable cell passed to Runtime dependencies.
    std::int32_t* guard_epoch=nullptr;
    s::Imports imports{};
    // Observed native C strings at0845c8/0845d8/0845f8. Callers may explicitly
    // override this text policy; the original accept path uses "Exception: ".
    std::string read_prefix="Read error: ",write_prefix="Write error: ",accept_prefix="Exception: ";
    std::size_t maximum_message_bytes=1048576;
    // Optional explicit host formatter override; defaults use distinct
    // compiler generic-category, concrete020950 system and020a80 misc messages.
    std::function<std::string(s::Category,std::int32_t)> format_override;
    // Trailing HOST layout addition; rebuild all Options/Graph/Worker consumers.
    s::SystemMessageImports system_message{};
};
enum class Phase {Fresh,Bound,Closed,Poisoned,Retiring};
struct Snapshot {
    Phase phase;bool active,logging,guard_initialized,worker_active;
    std::uint64_t worker_initializations,worker_finishes,exit_cleanups,log_flushes;
};
// Concrete selected compiler policy. No MSVC EH/GS TLS/atexit/ostream ABI or
// foreign pending-exception transfer is inferred from these source operations.
// Create/bind/close/final destruction use ONE original worker/fiber outside
// loader lock (the dedicated exit registry has that controller identity).
// Default calls are nonreentrant, same bound worker/fiber under Parent
// Operation. Explicit job cleanup is limited to the current child's registry
// entries under a live Parent JobPermit. With immutable Concurrent Parent
// policy, each cleanup has its own registered actor/permit/checkpoint lifetime;
// healthy sibling cleanups and original-worker calls do not share active.
// Tracked environment-work release has a separate per-call admission/permit
// registry. Concurrent foreign work/cleanup and original ordinary calls can
// coexist; an actor cannot reenter its own pending/live call. Serialized keeps
// its existing exclusion and explicit child-versus-waiting-original contract.
// Terminal forwarding remains possible after Profile is poisoned.
class Policy final:public sp::Policy,public std::enable_shared_from_this<Policy> {
public:
    // Optional actual source Graph retirement for OriginalApplication only.
    // This is local cleanup, not an external drain/unload proof. It runs after
    // worker return and real local IO/work checks, outside Parent Operation.
    // HOST create signature changed: rebuild every caller consistently.
    using GraphRetirement=std::function<bool(o::ServerView,sp::Application&)>;
    static std::shared_ptr<Policy> create(Options,GraphRetirement={});
    ~Policy()noexcept override;
    Policy(const Policy&)=delete;Policy& operator=(const Policy&)=delete;
    // Passive composition: refuses mixed caller errors/policy/category tokens.
    // NativeApi construction invokes no SDK import and performs no WSAStartup.
    void install(sp::Dependencies&);
    std::shared_ptr<s::Api> errors();
    s::Category generic_category()const noexcept;
    s::Category system_category()const noexcept;
    s::Category misc_category()const noexcept;
    std::int32_t& guard_epoch()const noexcept;
    // A dedicated internally owned registry for THIS selected policy only;
    // never the bootstrap/compiler CRT's unrelated thread-exit registrations.
    services_completion::ExitWaitRegistry& exit_waits();
    // Profile invokes these only after retaining its completed Application.
    void bind(rt::Ports,std::function<void()> nonmutating_admission);
    // Existing04a47c for only this actual child thread's registered waits.
    // Caller keeps the exact Parent Operation and job source claim live.
    // Does not complete a message/job, free a closure, or prove native exit.
    // Normal close retires the callback while its exact reservation is live;
    // unknown effects retain that call and sticky Policy health. Whole Policy
    // close/drain returns false while healthy child calls are registered.
    void cleanup_job_exit_waits(const s::SessionMemoryDomain::JobPermit&,std::function<void()> nonmutating_job_admission);
    // Synchronous native0229a0 work drop and immediate02a190 on oldcount1.
    // The separate work reservation accepts only this Parent's original actor
    // or its live explicit child JobPermit. consumed stays true after the drop,
    // including a later failed post/permission/raise; callers must not replay it.
    // The registered call copies the child permit/checkpoint and retains source
    // lease backing. SDK api/errors/consumed arguments are synchronous borrows,
    // never retained for unknown-effect replay. Target destruction occurs while
    // the exact reservation is published; failed calls retain their own prefix.
    // This does not widen OwnedEnvironment's still separate scope limitations.
    void finish_environment_work(s::IocpSchedulerEntry&,const s::SessionMemoryDomain::JobPermit*,s::WorkerStopApi&,s::Api&,const std::function<void()>& checkpoint,bool& consumed)override;
    bool close(bool external_attestation);
    Snapshot snapshot()const; // original-only; active includes retained failed calls

    s::DispatcherEffects& guards()override;
    s::WriteFinishApi& logging()override;
    [[noreturn]]void bad_weak_ptr_026860()override;
    void construct_length_error(s::ReadLengthError&,std::uintptr_t)override;
    [[noreturn]]void throw_length_error(s::ReadLengthError&)override;
    [[noreturn]]void thread_error(int)override;
    void cleanup_exit_waits()override;
    [[noreturn]]void raise_write(s::Error&,std::uintptr_t)override;
    std::uint32_t exchange_stop(s::IocpSchedulerEntry&,std::uint32_t)override;
    std::uint32_t exchange_wake(s::IocpSchedulerEntry&,std::uint32_t)override;
    void initialize(std::array<std::uintptr_t,2>&)override;
    void finish(std::array<std::uintptr_t,2>&)override;
    [[noreturn]]void raise(s::Error,std::uintptr_t)override;
    s::IocpSchedulerEntry* completion_owner(sp::Application&,s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override;
    void after_completion(sp::Application&)override;
    void worker_returned(sp::Application&,std::uint64_t,const s::WorkerRunResult&)override;
    bool external_quiescent(o::ServerView,sp::Application&)override;
private:
    explicit Policy(Options,GraphRetirement);
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::session_policy
