#pragma once
#include "../anadius-execution-source/execution.hpp"

namespace anadius::library_effects {
class PhaseBinding {
public:
    virtual ~PhaseBinding()=default;
    // Passive one-shot binding before source publication. Referents outlive
    // library workers, joins and provider retirement.
    virtual void bind_library_phase(execution_recovered::Execution&,std::uint8_t&)=0;
};
}
