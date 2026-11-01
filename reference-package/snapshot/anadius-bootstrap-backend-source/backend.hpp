#pragma once
#include "../anadius-bootstrap-owner-source/owner.hpp"
#include "../anadius-server-source/server.hpp"
#include "../anadius-online-source/online.hpp"
#include "../anadius-libraries-source/libraries.hpp"
#include "../anadius-host-init-source/host_init.hpp"
#include "../anadius-network-hook-source/hook_cleanup.hpp"
#include "tracked_threads.hpp"
#include <functional>

namespace anadius::bootstrap_runtime {class CallbackRegistry;}
namespace anadius::bootstrap_backend {
struct RuntimeSeed {
    execution_recovered::MutexState execution_mutex;
    std::int64_t frequency_cache;
    std::uint8_t server_ready,online_ready,libraries_ready,online;
    online_recovered::Module online_module;online_recovered::Object online_object;
    recovered::libraries::State libraries;
    host_init_recovered::Values registry_values;
    std::vector<host_init_recovered::Address> enabled_targets;
    std::array<host_init_recovered::Address,host_init_recovered::callback_count> originals;
};
// Source-only attachment point after the seed/config reach their stable owner.
// Factory construction must not publish callbacks, start workers or install
// hooks. The view is borrowed for this backend's lifetime, never copied state.
struct CallbackStateView {
    configuration::State& configuration;
    RuntimeSeed& runtime;
    execution_recovered::Execution& execution;
};
class CallbackAttachment {
public:
    virtual ~CallbackAttachment()=default;
    virtual void publish()=0;
    virtual void host_setup_begin() {}
    virtual void host_setup_complete() {}
    virtual bool pause_before_cleanup() {return true;}
    // Called only after stop/join AND application exit cleanup has drained.
    // Must independently require hook-disabled/external callback quiescence;
    // an empty exit queue or zero active-call snapshot is not that proof.
    virtual bool close_after_cleanup()=0;
};
using CallbackAttachmentFactory=std::function<std::unique_ptr<CallbackAttachment>(CallbackStateView)>;
// Optional SOURCE application-worker assembly. bind_state factory is called on
// controller; run_server on the recovered first worker. Milestones mean actual
// source calls returned, never preseeded ready bytes. failure must wake waiters.
class ApplicationWorker {
public:
    virtual ~ApplicationWorker()=default;
    virtual void run_server(std::uint16_t&,std::uint8_t&,execution_recovered::Execution&)noexcept=0;
    virtual void startup_complete(execution_recovered::ReadyStage,const execution_recovered::Execution*)noexcept=0;
    virtual void startup_failed(const execution_recovered::Execution*)noexcept=0;
    virtual void request_stop()=0;
    // After all TrackingThreads and per-image source leases have drained.
    virtual bool finish_after_join()=0;
    // Only a never-published backend with no tracked jobs can use this path.
    virtual bool cancel_unpublished()=0;
};
using ApplicationWorkerFactory=std::function<std::unique_ptr<ApplicationWorker>(CallbackStateView)>;
// Independent remaining-policy seam. SourceBackend itself supplies the six
// worker/host/exit routes, so they are deliberately absent here. The existing
// Services vtable and inheritance are unchanged.
class RemainingPolicies {
public:
    virtual ~RemainingPolicies()=default;
    virtual int register_exit(bootstrap_native_recovered::ExitCallback)=0;
    virtual void install_terminate(std::terminate_handler)=0;
    virtual void* log_insert(void*,std::uintptr_t)=0;
    virtual void log_finish(void*)=0;
    virtual void* allocate(std::size_t)=0;
    virtual void deallocate(void*,std::size_t)=0;
    [[noreturn]] virtual void thread_error(int)=0;
    virtual void guard_return()=0;
    virtual void cleanup_threads()=0;
    virtual void terminate_log()=0;
    [[noreturn]] virtual void unexpected_terminate_return()=0;
    // Controller only, after stop/join, source exit cleanup, attachment close
    // and registry.finish. True seals calls but keeps owners/metadata alive
    // through Resources destruction; actual retirement happens afterwards.
    virtual bool close_after_cleanup()=0;
};
using RemainingFactory=std::function<std::unique_ptr<RemainingPolicies>(bootstrap_runtime::CallbackRegistry&)>;
// New owning online assembly; existing Effects inheritance/vtable is preserved.
// Bind against this exact stable Backend view, with real Execution/ready routes.
class OnlineComponent:public online_recovered::Effects {
public:
    virtual bool matches(CallbackStateView)const noexcept=0;
    virtual bool close_after_cleanup()=0;
};
using OnlineFactory=std::function<std::unique_ptr<OnlineComponent>(CallbackStateView)>;
// Existing provider interfaces only. Components must be concrete implementations
// chosen by the caller. This backend supplies real application algorithms and
// join bookkeeping, not the still-missing OS/foreign-provider implementations.
struct Resources {
    // Optional backing state is destroyed last, after dependent providers.
    std::shared_ptr<void> state_lifetime;
    std::unique_ptr<configuration::Dependencies> configuration;
    std::unique_ptr<file_recovered::Factory> files;
    std::unique_ptr<startup_recovered::InputPlatform> input;
    std::unique_ptr<startup_recovered::ConsolePlatform> console;
    std::unique_ptr<threads_recovered::Effects> threads;
    std::unique_ptr<bootstrap_owner::Services> remaining;
    std::unique_ptr<execution_recovered::Platform> execution;
    std::unique_ptr<server_recovered::ServerBackend> server;
    std::unique_ptr<online_recovered::Effects> online;
    std::unique_ptr<recovered::libraries::SystemCalls> library_os;
    std::unique_ptr<recovered::libraries::Effects> library_effects;
    std::unique_ptr<host_init_recovered::Effects> host;
    std::unique_ptr<network_hook_recovered::CleanupEffects> hook_cleanup;
    // True must stop NEW application/worker/hook/late-exit/terminate admissions
    // and registrations, but allow already-created workers to finish. It must
    // not unload providers still in use. std::atexit alone cannot implement a
    // cancellable registrar. Application exit cleanup remains a separate source
    // route, not an invented part of this quiescence callback.
    std::function<bool()> stop_sources;
    // Default construction must expose a known empty API to Runtime's default
    // selector, rather than indeterminate callable pointers. Zero timeout is a
    // host nonblocking retry policy; callers can choose a finite explicit wait.
    JoinApi join_api{};
    std::uint32_t join_timeout_ms=0;
    // Optional extension appended after legacy resources. Empty preserves the
    // existing source path. Factory is invoked once in bind_state, unpublished.
    CallbackAttachmentFactory callback_attachment;
    // Optional one-assembly milestone observer. Owned until Execution and every
    // worker/callback user has been destroyed; not a worker join/health proof.
    std::shared_ptr<execution_recovered::Publication> startup_publication;
    // Exclusive with legacy server; invoked once at stable bind_state.
    ApplicationWorkerFactory application_worker;
    // Exclusive with legacy remaining. Runtime resolves this passive factory
    // once against its OWN registry, then retires its capture before source
    // publication. Returned provider owns all borrowed backing state. Appending
    // this field changes Resources HOST by-value layout: rebuild consumers.
    // Consume with take_resources at the caller sink, so libc++ small-buffer
    // callable donors cannot retain late callbacks to the returned provider.
    RemainingFactory remaining_factory{};
    // Exclusive with legacy online. Constructed once after bind_state makes
    // the actual configuration/runtime/Execution addresses stable.
    OnlineFactory online_factory{};
};
// Explicit consuming transfer. std::function move may clone an SBO capture in
// this toolchain; clear donor callables while the receiving owner is alive.
// In particular pass take_resources(resources), rather than std::move alone,
// when constructing a Runtime with the new remaining factory. Old by-value
// ABI stays available; external moved-from captures keep caller lifetime duties.
Resources take_resources(Resources&);
class SourceBackend final:public bootstrap_owner::ProviderOwner,public bootstrap_owner::Services {
public:
    SourceBackend(RuntimeSeed,Resources);
    SourceBackend(RuntimeSeed,Resources,std::unique_ptr<RemainingPolicies>);
    ~SourceBackend() noexcept;
    SourceBackend(const SourceBackend&)=delete;SourceBackend& operator=(const SourceBackend&)=delete;
    SourceBackend(SourceBackend&&)=delete;SourceBackend& operator=(SourceBackend&&)=delete;
    bootstrap_owner::Providers providers() override;
    void bind_state(configuration::State&,config_startup::ConsoleFlags) override;
    bool stop_and_join() override;
    int register_exit(bootstrap_native_recovered::ExitCallback) override;
    void install_terminate(std::terminate_handler) override;
    void* log_insert(void*,std::uintptr_t) override;
    void log_finish(void*) override;
    void* allocate(std::size_t) override;
    void deallocate(void*,std::size_t) override;
    recovered::NoArgumentFunction worker(recovered::Worker) override;
    recovered::BooleanFunction boolean_worker() override;
    [[noreturn]] void thread_error(int) override;
    void between_workers() override;
    void guard_return() override;
    void cleanup_threads() override;
    void cleanup_first() override;
    void cleanup_second() override;
    void cleanup_network() override;
    void terminate_log() override;
    [[noreturn]] void unexpected_terminate_return() override;
    const RuntimeSeed& state() const noexcept;
    std::size_t tracked_jobs() const;
    bool tracking_poisoned() const;
    bool finish_callback_attachment();
    bool finish_remaining_policies();
    bool finish_online_component();
    // Explicit owning-Runtime destructor route only: retire every provider,
    // then release bound configuration/root storage in037f70 group order.
    // Requires successful stop/join and the owning Runtime's complete normal
    // close. Caller owns configuration and keeps it alive through this call.
    // Standalone borrowed-state Backend destruction does not call this route.
    void retire_owned_state_037f70() noexcept;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
// Lifecycle methods use one non-reentrant control thread. Actual workers may
// run concurrently; remaining provider synchronization is mandatory. Function
// pointer routes have per-image leases; late calls after unpublish terminate
// without dereferencing the former backend. No PE entry or outer CRT entry is
// installed here. Rebuilt C++ bookkeeping uses its compiler CRT exactly once;
// a future compiler-CRT DLL entry must call only the application layer.
} // namespace anadius::bootstrap_backend
