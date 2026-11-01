#pragma once
#include "../anadius-session-profile-source/profile.hpp"
#include "../anadius-socket-source/native.hpp"
#include "../anadius-services-completion-source/exit_wait.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <memory>

namespace anadius::session_policy {
namespace sp=session_profile;namespace rt=server_runtime;namespace s=socket_recovered;namespace o=owned_server;

// These are still REQUIRED external decisions, not fabricated completion/drain
// algorithms. select_owner may return only its observed owner or nullptr; the
// Runtime validates that the live operation was neither freed nor replaced.
struct Lifecycle {
    std::shared_ptr<void> lifetime;
    std::function<s::IocpSchedulerEntry*(sp::Application&,s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)> select_owner;
    std::function<void(sp::Application&)> after;
    std::function<void(sp::Application&,std::uint64_t,const s::WorkerRunResult&)> returned;
    std::function<bool(o::ServerView,sp::Application&)> external_drained;
};
struct Options {
    std::shared_ptr<shared_log::Stream> stream;
    Lifecycle lifecycle;
    // Explicit fresh zero seed; SAME stable cell passed to Runtime dependencies.
    std::int32_t* guard_epoch=nullptr;
    s::Imports imports{};
    // Explicit HOST text policy, not a recovered native literal claim.
    std::string read_prefix="Read error: ",write_prefix="Write error: ",accept_prefix="Accept error: ";
    std::size_t maximum_message_bytes=1048576;
    // Optional explicit host formatter override; defaults use distinct
    // std::generic_category/system_category and labeled asio.misc messages.
    std::function<std::string(s::Category,std::int32_t)> format_override;
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
// All normal calls are nonreentrant, same bound worker/fiber under Parent
// Operation. Terminal forwarding remains possible after Profile is poisoned.
class Policy final:public sp::Policy,public std::enable_shared_from_this<Policy> {
public:
    static std::shared_ptr<Policy> create(Options);
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
    // A dedicated internally owned registry for THIS inline-job policy only;
    // never the bootstrap/compiler CRT's unrelated thread-exit registrations.
    services_completion::ExitWaitRegistry& exit_waits();
    // Profile invokes these only after retaining its completed Application.
    void bind(rt::Ports,std::function<void()> nonmutating_admission);
    bool close(bool external_attestation);
    Snapshot snapshot()const; // reentrant metadata on original worker/fiber only

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
    explicit Policy(Options);
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::session_policy
