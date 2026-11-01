#pragma once
#include "platform.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-startup-publication-source/publication.hpp"
#include "../anadius-original-provider-source/originals.hpp"

namespace anadius::host_platform {
struct Assembly;
// Explicit selection of existing rebuilt providers. Does not load/patch code,
// publish callbacks, start workers or infer native image initializers.
struct AssemblyInputs {
    AssemblyInputs()=default;
    AssemblyInputs(const AssemblyInputs&)=delete;AssemblyInputs& operator=(const AssemblyInputs&)=delete;
    AssemblyInputs(AssemblyInputs&&)=delete;AssemblyInputs& operator=(AssemblyInputs&&)=delete;
    std::shared_ptr<thread_domain::DomainStorage> domain;
    std::shared_ptr<shared_log::Stream> stream;
    std::shared_ptr<recovered_hooks::Api> engine;
    // Must actually retain engine/trampoline code until external users drain.
    // Merely supplying a nonnull token does not establish that obligation.
    std::shared_ptr<void> code_lifetime;
    std::optional<native_context::CallbackSeed> seed;
    callback_policy::Resources callbacks;
    Resources host;
    native_recovered::Errors* permanent_errors=nullptr;
    std::function<bool()> providers_ready;
    native_context::Quiescence external_quiescence;
private:
    bool consumed_=false;
    friend struct Assembly;
    friend Assembly install_host(bootstrap_backend::Resources&,AssemblyInputs&);
};
struct Assembly {
    std::shared_ptr<startup_publication::Milestones> milestones;
    std::shared_ptr<foreign_callback::Domain> callbacks;
};
// Controller, outside loader lock. Rejects conflicting outputs/settings before
// consumption. Single use after validation; construction failure is not retried.
// Caller keeps permanent_errors alive for every future cached native entry.
// Existing host_setup_complete attempts activation on the controller. If not
// ready then, caller retries callbacks->activate_foreign() after real milestones;
// the provider proof remains mandatory. No worker activation or added wait.
// Existing stop_sources is mandatory and consumed into an owning callback-first
// admission stopper. It may signal existing calls but cannot retire providers;
// already-created workers must not need new hooked calls after stopping.
// Hook-disable/external quiescence and Runtime close stay required.
Assembly install_host(bootstrap_backend::Resources&,AssemblyInputs&);
}
