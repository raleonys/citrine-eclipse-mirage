#include "defaults.hpp"
#include "../anadius-host-platform-source/assembly.hpp"
#include "../anadius-resources-factory-source/factory.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::initial_cells {
native_context::CallbackSeed make_callback_defaults(){
    // a37e0/a37f0/a3800: fresh empty source sets after observed native ctors.
    // a10d0/a10d4: initialized little-endian dwords10/1c.
    // a3888: initialized zero uint32 connect-attempt counter.
    return {{},{},{},16,28,0};
}
licensing_effects::AttemptState make_license_attempt_defaults(){
    // a39bc signed attempted-files dword; a39c0 blocked byte, initially zero.
    // These are separate cells from connect attempts and are not reset on use.
    return {0,0};
}
void select_defaults(host_platform::AssemblyInputs& host,resources_factory::Inputs& resources){
    if(host.seed||resources.effects.attempts)
        throw std::invalid_argument("Fresh callback and licensing attempt roots required; existing state cannot be replaced");
    if(!host.domain||host.domain!=resources.domain||host.domain->phase()!=thread_domain::Phase::Ready)
        throw std::invalid_argument("Initial cells require the same Ready host/worker Domain");
    if(host.callbacks.memory&&host.callbacks.memory->domain_identity()!=host.domain.get())
        throw std::invalid_argument("Callback memory must share the selected initial-cell Domain");
    auto callbacks=make_callback_defaults();
    auto attempts=std::make_shared<licensing_effects::AttemptState>(make_license_attempt_defaults());
    static_assert(std::is_nothrow_move_constructible_v<native_context::CallbackSeed>);
    host.seed.emplace(std::move(callbacks));
    resources.effects.attempts=std::move(attempts);
}
}
