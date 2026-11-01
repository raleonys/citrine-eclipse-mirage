#pragma once
#include "../anadius-application-graph-source/graph.hpp"
#include "../anadius-achievement-storage-source/path_binding.hpp"
#include "../anadius-license-source/auth_state.hpp"
#include "../anadius-achievement-runtime-source/mutex_binding.hpp"
#include "../anadius-application-effects-source/composition.hpp"
#include <optional>

namespace anadius::application_worker {
namespace b=bootstrap_backend;namespace g=application_graph;
// Lower SDK/content/license/lifecycle decisions only. Worker owns the actual
// Graph/Policy/Profile/epoch/TLS and exact RuntimeOwner StateView assembly.
struct Inputs {
    g::Options graph;owned_server::Dependencies server;server_runtime::Dependencies runtime;
};
struct Settings {
    // Explicit HOST seeds; these are not fields in configuration::State and
    // are never invented as evidence-backed native defaults.
    struct StateSeed {std::int32_t saved_avatar_size;configuration::Path achievement_file;};
    // achievement_file remains the legacy unbound seed. When shared native
    // cells are bound, a nonempty seed must initially match; Graph then borrows
    // the live XML cell and never writes this seed back over later updates.
    std::shared_ptr<void> lifetime; // owns every callback/service capture
    std::optional<StateSeed> state;
    // Optional concrete post-configure assembly. Exclusive with all3 legacy
    // configure effect slots. Requires bound Online, shared Paths and auth root.
    // Settings HOST layout changes: rebuild all Settings consumers. Declare
    // backing owners BEFORE configure so failed construction also destroys
    // configure captures first, followed by Composer, seeds and lifetime.
    std::shared_ptr<application_effects::Composer> effects;
    std::function<void(Inputs&)> configure;
    DWORD startup_timeout_ms=10000; // finite; timeout is unknown started state
};
// std::function moves can leave SBO donors. Consume those while the receiving
// Composer/lifetime are alive. Use this sink when transferring owning Settings;
// a caller's retained plain-moved donor still has external lifetime duties.
Settings take_settings(Settings&);
enum class Phase {Prepared,Preparing,Serving,WaitingStartup,Returned,Finishing,Closed,Poisoned,Retiring};
struct Snapshot {
    Phase phase;DWORD controller_thread,worker_thread;
    bool online_complete,host_complete,stop_requested,pending_retained;
    worker_handoff::Snapshot memory;
};
// Controller factory; worker performs the actual source server_start029ea0.
// Selected HOST policy defers IO dispatch until real online + host completion.
// It does NOT claim the original concurrent startup/EH/loader behavior.
// No Parent/Graph/Execution metadata gate is held across startup waits.
// Started timeout/failure/stop poisons and retains Graph/IO/memory; no fake
// close_unstarted or successful drain is manufactured for armed AcceptEx.
// PathBinding adds a secondary polymorphic base: rebuild concrete Worker
// allocating/crosscasting/deleting consumers. Legacy ApplicationWorker and
// bootstrap Resources/CallbackStateView layouts are unchanged.
class Worker final:public b::ApplicationWorker,public achievement_storage::PathBinding,public license_recovered::AuthenticationBinding,public achievement_runtime::MutexBinding,public application_effects::OnlineBinding {
public:
    static std::unique_ptr<Worker> create(std::shared_ptr<thread_domain::DomainStorage>,b::CallbackStateView,Settings);
    // Passive, one-shot binding on the original controller/fiber in Prepared.
    // Owner cells must outlive worker/Graph retirement, not only Cells::lifetime.
    void bind_achievement_paths(achievement_storage::Paths&)override;
    // Same passive Prepared/controller/fiber contract; cells outlive Graph.
    void bind_authentication_code(const license_recovered::AuthenticationCode&)override;
    void bind_achievement_mutex(execution_recovered::MutexState&,execution_recovered::Platform&)override;
    void bind_effects_online(b::OnlineComponent&)override;
    ~Worker()noexcept override;
    Worker(const Worker&)=delete;Worker& operator=(const Worker&)=delete;
    void run_server(std::uint16_t&,std::uint8_t&,execution_recovered::Execution&)noexcept override;
    void startup_complete(execution_recovered::ReadyStage,const execution_recovered::Execution*)noexcept override;
    void startup_failed(const execution_recovered::Execution*)noexcept override;
    // Called by independent controller stop_sources; wake only. It is NOT an
    // SDK cancellation/drain provider. Caller must still stop kernel borrowers.
    void request_stop()override;
    bool finish_after_join()override;
    bool cancel_unpublished()override;
    Snapshot snapshot()const;
private:
    struct Impl;explicit Worker(std::unique_ptr<Impl>)noexcept;
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::application_worker
