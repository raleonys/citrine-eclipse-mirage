#include "observed_runtime.hpp"
#include "../anadius-config-defaults-source/defaults.hpp"
#include "../anadius-configuration-factory-source/factory.hpp"
#include <stdexcept>

namespace anadius::bootstrap_runtime {
std::unique_ptr<Runtime> make_observed_runtime_036d20(OutsideConstructorState outside,
    bootstrap_backend::Resources& resources,bootstrap_native_recovered::Errors& errors,HostApi host){
    if(auto* native=dynamic_cast<configuration_factory::Configuration*>(resources.configuration.get());
       native&&!native->accepts_observed_empty_paths())
        throw std::invalid_argument("Observed constructor requires fresh empty achievement path cells");
    bootstrap_owner::InitialState initial{
        configuration_defaults::make_configuration_defaults_036d20(),{0,0,0}};
    bootstrap_backend::RuntimeSeed seed{};
    execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);
    seed.frequency_cache=outside.frequency_cache;
    seed.registry_values=std::move(outside.registry_values);
    seed.enabled_targets=std::move(outside.enabled_targets);
    seed.originals=outside.originals;
    // Existing consuming sink owns all move/capture retirement and real Runtime
    // State binding; this route neither supplies fake providers nor republishes.
    return make_runtime(std::move(initial),std::move(seed),resources,errors,host);
}
}
