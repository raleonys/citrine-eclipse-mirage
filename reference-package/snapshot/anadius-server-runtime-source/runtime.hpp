#pragma once
#include "../anadius-owned-server-source/server.hpp"
#include "../anadius-session-flow-source/coherent_io.hpp"
#include "../anadius-accept-helpers-source/bindings.hpp"
#include "../anadius-accept-stop-source/stop.hpp"

namespace anadius::server_runtime {
namespace s=socket_recovered;namespace o=owned_server;
class WorkerControl {
public:virtual ~WorkerControl()=default;
    virtual std::uint32_t exchange_stop(s::IocpSchedulerEntry&,std::uint32_t)=0;
    virtual std::uint32_t exchange_wake(s::IocpSchedulerEntry&,std::uint32_t)=0;
    virtual void initialize(std::array<std::uintptr_t,2>&)=0;
    virtual void finish(std::array<std::uintptr_t,2>&)=0;
    [[noreturn]]virtual void raise(s::Error,std::uintptr_t)=0;
};
struct Environment {
    s::IoHandlerSentinels sentinels;s::IoEnvironmentConstructorContext& constructor;
    std::span<s::IoEnvironmentSourceBinding> sources;
};
struct Ports {
    o::ServerView view; // copy the projection; never retain a call-local view&
    s::OwnedAcceptMemory& accept;s::CoherentIoMemory& memory;
    s::WorkerRunApi& worker;s::DispatcherApi& completion;
    s::IoCompletionEnvironment environment;std::uintptr_t server_identity;
};
struct Bindings {
    s::FrameReadContext& frame;s::WriteFinishApi& logging;
    s::IoCompletionEffects& completion;s::DispatcherEffects& guard;WorkerControl& worker;
    std::span<s::WriteFinishCategoryBinding> categories;
    std::span<s::AcceptScheduleBinding> accept_schedules;
    s::ComposedWriteContext* write=nullptr;
};
// Owned application contexts/registries have stable capacity and outlive all
// packets/jobs. No submit/run/complete algorithm is delegated to this interface.
// Message/profile ABI, scheduling and raw-job/kernel finality remain providers.
class Application:public s::FlowApplication {
public:
    virtual Bindings bindings()=0;
    virtual void construct_accept_environment(s::IoHandlerEnvironment&,std::uintptr_t allocator)=0;
    virtual void reset_pending_owner(s::AcceptPending&)=0;
    // Called only for an observed packet under whole-worker admission. May
    // select null for an explicitly evidenced terminal packet, or same owner.
    // It must not free/reproject the operation. Runtime performs actual routing.
    virtual s::IocpSchedulerEntry* completion_owner(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)=0;
    virtual void after_completion()=0; // old operation may already be destroyed
    virtual void worker_returned(std::uint64_t,const s::WorkerRunResult&)=0;
};
class Profile {
public:virtual ~Profile()=default;
    virtual Environment environment(o::ServerView,std::uintptr_t server_identity)=0;
    // Construct/bind contexts only; no worker/SDK publication before Runtime
    // completes binding. Borrow Ports by value; do not retain backend ownership.
    virtual std::unique_ptr<Application> create(Ports)=0;
    // External stop, joined raw jobs and observed final kernel packets, outside
    // parent Operation. Counts alone are insufficient. No native unload proof.
    // Destruction after true must have no remaining source cleanup effects.
    virtual bool external_quiescent(o::ServerView,Application&)=0;
};
struct Dependencies {
    // Optional when providers/Profile already own the exact TLS/epoch/executor
    // and borrowed environment state. Those lifetimes remain mandatory.
    std::shared_ptr<void> state_lifetime;
    std::shared_ptr<s::Api> errors;std::shared_ptr<s::AcceptorApi> sockets;
    std::shared_ptr<s::DispatcherApi> dispatcher;std::shared_ptr<s::WorkerRunApi> worker;
    std::shared_ptr<s::AsyncAcceptApi> accept;std::shared_ptr<s::SocketFactoryApi> factory;
    std::shared_ptr<s::RetryCancelApi> cancel;std::shared_ptr<Profile> profile;
    std::uint32_t* tls_index=nullptr;std::int32_t* guard_epoch=nullptr;
    s::Category generic_category{},system_category{},misc_category{};
    s::ExecutorMove executor;std::size_t capacity=64;
    // Optional selected accept-only stop policy. Receive/send/raw-job/external
    // finality remains independently required. By-value HOST layout changed;
    // all dependent factories/Graph/Worker translation units must be rebuilt.
    std::shared_ptr<anadius::accept_stop::Stop> accept_stop{};
};
enum class Phase {Fresh,Binding,Ready,Running,Returned,Quiescing,Closed,Poisoned,Retiring};
struct Snapshot {Phase phase;std::size_t active_calls;bool graph_present;std::uint64_t handled;};
// Selected source composition. All lifecycle calls use one thread/fiber outside
// loader lock; submit/run use B35's existing whole parent Operation. Unknown
// prefixes are retained/poisoned; successful explicit quiescence is mandatory
// before destruction of a bound graph. No original native class/ABI/body claim.
class Runtime final:public o::Runtime {
public:
    explicit Runtime(Dependencies);
    ~Runtime()noexcept;
    Runtime(const Runtime&)=delete;Runtime& operator=(const Runtime&)=delete;
    void submit_accept(o::ServerView&)override;
    // B35 constructs admitted for this view's parent. The server factory fixes
    // that route; arbitrary admitted instances from another domain are invalid.
    server_recovered::Error run(o::ServerView&,o::AdmittedDispatcherApi&)override;
    bool quiescent(o::ServerView&)override;
    Snapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::server_runtime
