#pragma once
#include "configuration.hpp"

namespace anadius::configuration {
// Optional source-only binding, separate from the legacy Dependencies vtable.
// SourceBackend supplies its RuntimeOwner's stable state before startup.
// Binding must be passive: no I/O, callbacks, worker or route publication.
// State outlives the provider; startup/configuration use remains serialized.
class StateBinding {
public:
    virtual ~StateBinding()=default;
    virtual void bind_configuration_state(State&)=0;
};
} // namespace anadius::configuration
