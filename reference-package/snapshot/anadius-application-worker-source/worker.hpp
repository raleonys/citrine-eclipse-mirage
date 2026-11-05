#pragma once
#include "../anadius-application-graph-source/graph.hpp"
#include "../anadius-achievement-storage-source/path_binding.hpp"
#include "../anadius-license-source/auth_state.hpp"
#include "../anadius-achievement-runtime-source/mutex_binding.hpp"
#include "../anadius-application-effects-source/composition.hpp"
#include <optional>
#include <cstdint>

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
    // configure captures first, followed by Stop, Composer, seeds and lifetime.
    std::shared_ptr<application_effects::Composer> effects;
    // Optional controller-created cancellation owner for this exact server.
    // Must be Fresh/exclusively selected and created on this Worker's original
    // controller/fiber. Scope and independent other_work_drained proof remain
    // caller choices; no raw-job/foreign admission/finality proof is invented.
    // Worker binds this exact pointer into Runtime and requests its IOCP wake.
    // Added HOST Settings field: rebuild every allocating/passing consumer
    // when the user next authorizes a build. Existing artifacts are historical.
    std::shared_ptr<accept_stop::Stop> accept_stop;
    std::function<void(Inputs&)> configure;
    DWORD startup_timeout_ms=10000; // finite; timeout is unknown started state
    // Explicit optional HOST close-continuation policy. Zero keeps terminal
    // deferral behavior. Nonzero is ONE finite wait budget from first known
    // false drain; controller must retry_close(snapshot.close_epoch) per wait.
    // No polling, arbitrary-exception recovery, or controller-side destruction.
    // Added Settings field: rebuild all allocating/passing consumers.
    DWORD close_retry_timeout_ms=0;
};
// std::function moves can leave SBO donors. Consume those while the receiving
// Composer/lifetime are alive. Use this sink when transferring owning Settings;
// a caller's retained plain-moved donor still has external lifetime duties.
Settings take_settings(Settings&);
enum class Phase {Prepared,Preparing,Serving,WaitingStartup,Returned,Finishing,Closed,Poisoned,Retiring,WaitingClose};
struct Snapshot {
    Phase phase;DWORD controller_thread,worker_thread;
    bool online_complete,host_complete,stop_requested,pending_retained;
    worker_handoff::Snapshot memory;
    std::uint64_t close_epoch,close_retries;
};
// Controller factory; worker performs the actual source server_start029ea0.
// Explicit/SourceOwned HOST policy defers IO dispatch until real online + host
// completion. OriginalApplication admits dispatch without that additional
// barrier; recovered native bodies retain their own Execution waits. This
// distinction does not validate original concurrent startup/EH/loader behavior.
// No Parent/Graph/Execution metadata gate is held across startup waits.
// SourceOwned lifecycle binds its exact Runtime SessionIo/SessionIoPrepared
// Stop into Policy before Graph construction; missing/AcceptOnly/mixed owners
// reject. Local receipt proof never replaces external callback/image finality.
// OriginalApplication instead uses actual local source retirement; no selected
// Stop or external unload callback is required for that explicit profile.
// Startup timeout/failure/pre-admission stop retains poisoned Graph/IO/memory; no fake
// close_unstarted or successful drain is manufactured for armed AcceptEx.
// Explicit selected close continuation stays on this native worker until it
// succeeds or fails; a controller wake never transfers provider/PTD ownership.
// PathBinding and ApplicationJoinContinuation add secondary polymorphic bases: rebuild concrete Worker
// allocating/crosscasting/deleting consumers. Legacy ApplicationWorker and
// bootstrap Resources/CallbackStateView layouts are unchanged.
class Worker final:public b::ApplicationWorker,public b::ApplicationJoinContinuation,public achievement_storage::PathBinding,public license_recovered::AuthenticationBinding,public achievement_runtime::MutexBinding,public application_effects::OnlineBinding {
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
    // Controller only. Marks/wakes startup state and, when explicitly selected,
    // calls the same Runtime Stop::request outside the Worker mutex. FALSE from
    // that provider may be retryable or Poisoned; inspect its state and retain
    // unknown outcomes. Request acceptance is NOT kernel completion/drain:
    // actual cancellation/packet acknowledgement stay on the admitted worker.
    // No selected provider preserves the legacy startup-event-only behavior.
    void request_stop()override;
    // Concrete HOST API on the original controller/fiber. Known current epoch
    // only; stale/zero/nonwaiting calls return false without issuing a retry.
    // True signals one pending recheck, NOT proof of drain or successful close.
    // Retain this Worker until native join/finish; do not race its destruction.
    bool retry_close(std::uint64_t close_epoch);
    // SourceBackend controller hook after stop/pause. For enabled policy it
    // signals only a currently published epoch, returns false while running/
    // waiting/poisoned, and leaves native join/FLS/source leases as separate
    // mandatory proofs. Default-disabled policy keeps legacy join behavior.
    bool prepare_join()override;
    bool finish_after_join()override;
    bool cancel_unpublished()override;
    Snapshot snapshot()const;
private:
    struct Impl;explicit Worker(std::unique_ptr<Impl>)noexcept;
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::application_worker
