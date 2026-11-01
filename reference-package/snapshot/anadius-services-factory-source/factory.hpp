#pragma once
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../anadius-bootstrap-runtime-source/callbacks.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include "../anadius-services-completion-source/completion.hpp"

namespace anadius::services_factory {
struct Options {
    // Declared first and retired last. All borrowed provider/stream captures
    // must own their backing or be covered by this optional shared owner.
    std::shared_ptr<void> lifetime;
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<services_completion::BootstrapLogger> logger;
    std::shared_ptr<services_completion::ExitWaitRegistry> waits;
};
enum class Phase {Open,Checking,Closed,Retiring,Poisoned};
struct Snapshot {Phase phase;std::size_t active_calls;bool used;};
// Owns eleven selected source policies, not the six worker/cleanup bodies
// already implemented by SourceBackend. No giant Services lower is required.
// Registry outlives this provider. Construct/close/retire on its controller
// thread and fiber; ordinary routes may run on admitted same-Domain workers.
// External source admission stop, all worker/FLS joins and registry drain are
// required BEFORE close_after_cleanup; no count snapshot establishes them.
// A false known preflight leaves routes open for explicit deallocation/cleanup
// retry. Unknown outcomes poison and retain owners. Successful close seals
// routes but retains every owner for opaque Resources destructors; physical
// release occurs only in this facade's attached Retiring destructor.
// ApplicationMemory has no close method: actual tracking/empty checks precede
// releasing this shared owner. External aliases retain their own Domain lease.
// Failed construction retires only actually empty unpublished inputs. Callers
// must retain aliases for live/unknown inputs; no inferred rollback is supplied.
class Remaining final:public bootstrap_backend::RemainingPolicies {
public:
    Remaining(bootstrap_runtime::CallbackRegistry&,Options);
    ~Remaining()noexcept override;
    Remaining(const Remaining&)=delete;Remaining& operator=(const Remaining&)=delete;
    Remaining(Remaining&&)=delete;Remaining& operator=(Remaining&&)=delete;
    int register_exit(bootstrap_native_recovered::ExitCallback)override;
    void install_terminate(std::terminate_handler)override;
    void* log_insert(void*,std::uintptr_t)override;
    void log_finish(void*)override;
    void* allocate(std::size_t)override;
    void deallocate(void*,std::size_t)override;
    [[noreturn]]void thread_error(int)override;
    void guard_return()override;
    void cleanup_threads()override;
    void terminate_log()override;
    [[noreturn]]void unexpected_terminate_return()override;
    bool close_after_cleanup()override;
    Snapshot snapshot()const;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
};
std::unique_ptr<bootstrap_backend::RemainingPolicies> make_remaining(
    bootstrap_runtime::CallbackRegistry&,Options);
} // namespace anadius::services_factory
