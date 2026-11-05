#pragma once
#include "../anadius-bootstrap-runtime-source/factory.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include <atomic>

namespace anadius::pre_unload {
enum class Phase {Empty,Managed,ClosingRuntime,RetiringRuntime,ClosingDomain,Closed,Poisoned,Retiring,
    PublishingApplication,StartingApplication,ApplicationAttached};
using Entry=BOOL(WINAPI*)(void*) noexcept;
struct Snapshot {Phase phase;DWORD error;bool busy;};
// Explicit cooperative HOST pre-unload path. Never call inside DllMain, loader
// lock, process termination or a CRT exit callback. Same controller/fiber as
// Runtime and Domain; all borrowed policies/code must stay mapped and callable.
// Existing stop_sources/SDK/foreign drain obligations are NOT manufactured.
class Controller final {
public:
    // Allocate bookkeeping/control block BEFORE consuming either donor. The
    // Domain must be Ready and match Runtime's certified thread-provider root.
    // Unknown/custom thread provenance rejects; no arbitrary root is adopted.
    // Other provider-domain consistency and original controller/fiber remain
    // caller obligations. This method invokes no stop/SDK/provider callback.
    // Ownership is Runtime-before-Domain even through construction failures.
    static std::shared_ptr<Controller> take(std::unique_ptr<bootstrap_runtime::Runtime>&,
        std::shared_ptr<thread_domain::DomainStorage>&);
    // Own both roots BEFORE publishing or starting the application. Only a
    // Constructed Runtime is accepted; existing Bound take/shutdown users keep
    // their old route. Runs the real application entry synchronously once,
    // without waiting for startup milestones or joining workers. Success
    // leaves roots in this same owner for later cooperative shutdown.
    // Rejected preconditions perform no publication. An exception/false app
    // result after publication poisons and RETAINS both roots, then throws;
    // it is not complete attach rollback and must not become DllMain FALSE.
    // This is an owning source primitive, not a loader-safety certificate:
    // Runtime's outside-loader-lock contract still applies. The production
    // factory/profile and image lifetime must be resolved before loader use.
    void start_application(HINSTANCE module,LPVOID reserved);
    ~Controller() noexcept;
    Controller(const Controller&)=delete;Controller& operator=(const Controller&)=delete;
    BOOL shutdown() noexcept;
    static Entry entry() noexcept;
    void* context() noexcept{return this;}
    Snapshot snapshot() const noexcept;
    // Lock-free receipt of this managed owner's closure only. A future loader
    // consumer also needs external-entry exclusion, alias/code/CRT lifetime and
    // stable receipt storage. This alone is NOT permission to FreeLibrary.
    bool closed() const noexcept;
private:
    Controller() noexcept;
    static BOOL WINAPI invoke(void*) noexcept;
    BOOL result(DWORD) noexcept;
    bool poisoned() const noexcept;
    // Reverse order and explicit retirement keep Domain alive through Runtime.
    std::shared_ptr<thread_domain::DomainStorage> domain_;
    std::unique_ptr<bootstrap_runtime::Runtime> runtime_;
    std::atomic<Phase> phase_{Phase::Empty};std::atomic<DWORD> error_{0};std::atomic<bool> busy_{false};
    const DWORD controller_;void* const fiber_;
};
}
