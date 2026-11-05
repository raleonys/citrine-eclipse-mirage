#pragma once
#include "platform.hpp"
#include "source_stop.hpp"
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
    // OriginalCrt admits callbacks from their actual successful-create slot
    // publication; native callback bodies perform their own original waits.
    // No blanket providers_ready or external unload proof is installed.
    native_context::ClosureProfile closure_profile=native_context::ClosureProfile::Cooperative;
    // Appended HOST layout field; rebuild all AssemblyInputs consumers.
private:
    bool consumed_=false;
    friend struct Assembly;
    friend Assembly install_host(bootstrap_backend::Resources&,AssemblyInputs&);
};
struct Assembly {
    std::shared_ptr<startup_publication::Milestones> milestones;
    std::shared_ptr<foreign_callback::Domain> callbacks;
    std::shared_ptr<const SourceStopReceipt> stopped_sources;
};
// Controller, outside loader lock. Rejects conflicting outputs/settings before
// consumption. Single use after validation; construction failure is not retried.
// The following all-ready/proof activation contract describes Cooperative.
// OriginalCrt instead checks actual original publication per callback, admits
// its source body and preserves that body's waits, including during host setup.
// Caller keeps permanent_errors alive for every future cached native entry.
// Existing host_setup_complete attempts activation on the controller. If not
// ready then, caller retries callbacks->activate_foreign() after real milestones;
// the provider proof remains mandatory. No worker activation or controller wait.
// Host create and callback Policy share an exact-slot per-hook original
// publisher. Successful create results are published before enable. It
// opens untouched connect and proven no-redirect80/443 passthrough during
// warming after PTD entry/per-hook publication. The web route also requires
// same-Execution bound-port payload plus actual Server finish event. Other no-redirect ports except
//3216 may run the existing wait(-1) on a foreign thread with wait-only PTD
// permission, then recheck original publication. Controller warming never
// gains this blocking route. Stop retains a waiter until it actually returns.
// Foreign3216/matching-port redirects may use a separate network-body scope
// after required wait and immutable port proof. Managed shared log and private
// callback counter are required; general callback/SDK permission stays closed
// during log/counter/UI/port-store body, then original proof is rechecked.
// It does not certify general callback admission or the loader-lock profile.
// Existing stop_sources is mandatory and consumed into an owning callback-first
// admission stopper. It may signal existing calls but cannot retire providers;
// already-created workers must not need new hooked calls after stopping.
// Hook-disable/external quiescence and Runtime close stay required.
Assembly install_host(bootstrap_backend::Resources&,AssemblyInputs&);
}
