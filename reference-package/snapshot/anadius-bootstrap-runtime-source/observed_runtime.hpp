#pragma once
#include "factory.hpp"
#include <utility>

namespace anadius::bootstrap_runtime {
// These cells are OUTSIDE036d20. The constructor route must not silently
// infer/reset their native initial values. Every argument is mandatory.
struct OutsideConstructorState {
    std::int64_t frequency_cache;
    host_init_recovered::Values registry_values;
    std::vector<host_init_recovered::Address> enabled_targets;
    std::array<host_init_recovered::Address,host_init_recovered::callback_count> originals;
    OutsideConstructorState(std::int64_t frequency,host_init_recovered::Values values,
        std::vector<host_init_recovered::Address> targets,
        std::array<host_init_recovered::Address,host_init_recovered::callback_count> slots)
        :frequency_cache(frequency),registry_values(std::move(values)),
         enabled_targets(std::move(targets)),originals(slots){}
};
// Selects the observed41 configuration defaults, zero console/online/ready
// fields, empty native module/path vectors and initialized type2 execution
// mutex. Backend owns the observed authentication/path/achievement-mutex cells.
// Host reserved mutex bytes use defined zero backing, not native ABI equality.
// All Resources/foreign policies, saved-avatar Worker seed, permanent errors
// and host API duties remain explicit. Passive, outside loader lock; does not
// publish/start workers, register01390's native atexit, or install a DLL entry.
// Default-state construction completes before consuming the Resources donor.
// Supplied providers must preserve passive binding. Known Configuration rejects
// preseeded/nonfresh XML/INI cells before consumption; ordinary make_runtime
// continues to accept explicit path seeds. Remaining provider state (including
// saved avatar and license retry/block cells) is not covered by the four fields.
std::unique_ptr<Runtime> make_observed_runtime_036d20(OutsideConstructorState,
    bootstrap_backend::Resources&,bootstrap_native_recovered::Errors&,
    HostApi=native_host_api());
}
