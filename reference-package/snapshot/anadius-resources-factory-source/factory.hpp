#pragma once
#include "../anadius-application-worker-source/worker.hpp"
#include "../anadius-application-effects-source/factory.hpp"
#include "../anadius-services-factory-source/factory.hpp"
#include "../anadius-library-effects-source/bound.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <string>

namespace anadius::host_platform {struct Assembly;}
namespace anadius::bootstrap_providers {class NativeThreads;}
namespace anadius::resources_factory {
// Concrete SDK leaf selection for the original server worker's configure
// callback. Defaults use real Windows/Winsock APIs. Construction does not open
// a socket, initialize Winsock, create an IOCP, wait, accept or start a worker.
struct WorkerIoImports {
    socket_recovered::AcceptorImports sockets{};
    socket_recovered::IocpConstructorImports iocp{};
    socket_recovered::ServiceGateImports gates{};
    socket_recovered::DispatcherImports dispatcher{};
    socket_recovered::AsyncAcceptImports accept{};
    socket_recovered::SocketFactoryImports factory{};
    decltype(&::CancelIoEx) cancel=&::CancelIoEx;
};
// Serial worker configuration, before Graph/server publication. All selected
// slots must be empty. Validate all imports, construct every adapter, then
// commit together. Server/Runtime share exact sockets and dispatcher owners.
// LastError is preserved. Existing KernelImports remain caller-selected.
// Does not supply cancellation registry/executor semantics, Graph lifecycle,
// TLS, completion-owner selection or external drain proof. Those remain real
// mandatory providers, never inferred from a native SDK table or counts.
// This worker path does not certify the synchronous loader attach profile.
void select_native_worker_io(application_worker::Inputs&,WorkerIoImports={});
// Exact recovered084928 executor only: actual0228c0 field move and observed
//017a30 cleanup return. Both executor slots must be empty; no generic executor
// or cancellation/drain semantics are inferred from this selection.
void select_recovered_socket_executor(application_worker::Inputs&);
// Opt-in source-owned lifecycle: existing completion owner/reap/worker-return
// routes supply the source decisions. Requires a real external_drained proof
// already supplied by the host and no custom owner selector. Optional after/
// returned observers remain; no opaque capture is copied or discarded here.
// This does not install Stop or establish IO/kernel/foreign/image quiescence.
// ApplicationWorker subsequently requires the exact Runtime SessionIo Stop and
// binds it as Policy's local packet/ack prerequisite before Graph construction.
void select_source_worker_lifecycle(application_worker::Inputs&);
// Original application source profile: actual owner/reap/worker-return and
// tracked IO/job/queue retirement, with no external unload proof or mandatory
// Stop selection. Reject custom owner/proof/source-stop slots; optional after/
// returned observers are retained. Does not assert foreign/image quiescence.
void select_original_worker_lifecycle(application_worker::Inputs&);
// Concrete native configure body for the image's Worker settings. Select real
// SDK IO adapters, recovered executor and OriginalApplication lifecycle with
// the supplied managed stream/version. These selected destination slots must
// be fresh. Build in local ownership, recheck and commit without throwing.
// No fake lifecycle callbacks, Winsock startup, socket/port creation, wait or
// worker start. Worker still binds its exact state/TLS and concrete effects.
// Empty version selects that Worker's actual Runtime configuration.sdk before
// Graph construction (native Challenge version cell0xa3c98).
void configure_native_worker(application_worker::Inputs&,
    std::shared_ptr<shared_log::Stream>,std::string version);
// Select actual detached message jobs for the configured original Worker.
// Construct on that server worker/fiber, before Graph takes these options.
// NativeThreads must have been prepared on its Ready Domain's controller;
// later bind_prepared validates that same Domain against the actual Parent.
// Fresh jobs slot only; no inline fallback or existing-provider replacement.
// Construction creates no child/event/IOCP and publishes no job or SDK work.
void select_native_message_jobs(application_worker::Inputs&,
    std::shared_ptr<bootstrap_providers::NativeThreads>);
// Owning settings for the selected source composition. Existing configuration
// (with PathBinding), console, host, hook cleanup and stop_sources stay caller
// supplied. Runtime's existing files/input/library_os/execution/join defaults
// remain its explicit policy. No global/loader/foreign-drain seed is invented.
struct Inputs {
    Inputs()=default;
    Inputs(const Inputs&)=delete;Inputs& operator=(const Inputs&)=delete;
    Inputs(Inputs&&)=delete;Inputs& operator=(Inputs&&)=delete;
    std::shared_ptr<thread_domain::DomainStorage> domain;
    std::shared_ptr<shared_log::Stream> stream;
    std::shared_ptr<library_effects::Dialog> dialog;
    std::shared_ptr<services_completion::ExitWaitRegistry> waits;
    library_effects::BoundOptions library;
    application_effects::Options effects;
    online_factory::Options online;
    // Last owning field: configure captures die before their backing settings.
    application_worker::Settings worker;
    thread_native::Api threads=thread_native::native_api();
private:
    bool consumed_=false;
    friend void install(bootstrap_backend::Resources&,Inputs&);
    friend void select_worker_stop(Inputs&,const host_platform::Assembly&,const accept_stop::Options&);
};
// Passive selection on the original Worker controller/fiber, outside loader
// lock, before install. Requires this Assembly's exact callback/memory Domain.
// Options must still supply real backing and the mandatory independent work
// proof (including raw/deferred work and admission outside this callback gate).
// Preserves caller scope; no stop, wait, cancel or drain happens here.
// Serial Inputs ownership required. Reservation spans opaque copy retirement;
// reentrant install/selection rejects before effects. Failure leaves the slot
// unchanged and Inputs available; successful selection does not consume Inputs.
void select_worker_stop(Inputs&,const host_platform::Assembly&,const accept_stop::Options&);
// Controller, Ready Domain, before any publication. Rejects conflicting slots
// and invalid mandatory settings BEFORE consumption. A consumed install is
// single-use. Each completed provider/factory is adopted directly in Resources;
// failure can leave an owned partial assembly and is not rolled back/retried.
// Runtime/admitted workers must retire before caller's Domain and backing.
void install(bootstrap_backend::Resources&,Inputs&);
}
