#include "startup_bridge.hpp"
#include <stdexcept>

namespace anadius::crt_startup_bridge {
TerminationTablesBridge::TerminationTablesBridge(crt_initializers_recovered::Effects& effects,
    std::span<crt_initializers_recovered::Address> preterminators,
    std::span<crt_initializers_recovered::Address> terminators)
    :table_effects_(effects),preterminators_(preterminators),terminators_(terminators) {
    if(preterminators.size()!=4||terminators.size()!=1)
        throw std::invalid_argument("Recovered termination table widths are 4 and 1");
}
void TerminationTablesBridge::initterm_range_064614(crt_termination_recovered::Address begin,
    crt_termination_recovered::Address end) {
    if(begin==0x18007f728&&end==0x18007f748)
        crt_initializers_recovered::initterm_064614(preterminators_,table_effects_);
    else if(begin==0x18007f750&&end==0x18007f758)
        crt_initializers_recovered::initterm_064614(terminators_,table_effects_);
    else throw std::domain_error("Unsupported termination range selector");
}
StartupBridge::StartupBridge(crt_initializers_recovered::State& state,
    std::span<crt_initializers_recovered::Pair> pairs,
    std::span<crt_initializers_recovered::Address> c,
    std::span<crt_initializers_recovered::Address> cpp,
    crt_initializers_recovered::Effects& effects,
    crt_ptd_recovered::State& ptd,crt_ptd_recovered::Effects& ptd_effects,
    vcrt_bridge::Dependencies vcrt,Dependencies deps)
    :LifecycleBridge(state,pairs,c,cpp,effects,ptd,ptd_effects,vcrt),deps_(deps){}
void StartupBridge::initialize_slist_04c1e0(){crt_support_recovered::initialize_slist_04c1e0(deps_.head,deps_.list_effects);}
void StartupBridge::release_slist_04c1f0(){crt_support_recovered::release_slist_04c1f0(deps_.head,deps_.list_effects);}
void StartupBridge::configure_stdio_04c1fc(){crt_support_recovered::configure_stdio_04c1fc(deps_.support);}
const std::uintptr_t* StartupBridge::tls_callback_slot_04c218(){return crt_support_recovered::tls_callback_slot_04c218(deps_.support);}
void StartupBridge::initialize_narrow_environment_064054(){(void)crt_environment_recovered::thunk_initialize_064054(deps_.environment,deps_.environment_effects);}
std::uint32_t StartupBridge::c_termination_complete_05cdb8(){return crt_termination_recovered::c_termination_complete_05cdb8(deps_.termination);}
void StartupBridge::cexit_05cd9c(){crt_termination_recovered::cexit_05cd9c(deps_.termination,deps_.termination_effects);}
}
