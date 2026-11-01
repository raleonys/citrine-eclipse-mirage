#pragma once
#include "../anadius-application-worker-source/worker.hpp"
#include "../anadius-application-effects-source/factory.hpp"
#include "../anadius-services-factory-source/factory.hpp"
#include "../anadius-library-effects-source/bound.hpp"
#include "../anadius-shared-log-source/stream.hpp"

namespace anadius::resources_factory {
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
};
// Controller, Ready Domain, before any publication. Rejects conflicting slots
// and invalid mandatory settings BEFORE consumption. A consumed install is
// single-use. Each completed provider/factory is adopted directly in Resources;
// failure can leave an owned partial assembly and is not rolled back/retried.
// Runtime/admitted workers must retire before caller's Domain and backing.
void install(bootstrap_backend::Resources&,Inputs&);
}
