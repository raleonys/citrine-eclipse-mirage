#pragma once
#include "../anadius-execution-source/execution.hpp"

namespace anadius::achievement_runtime {
// Optional source-only binding to Runtime's +428/a3e08 state and existing SDK
// platform. Both referents must outlive Worker/Graph and every locked prefix.
// Bind passively once before publication; no original vtable/ABI is claimed.
class MutexBinding {
public:
    virtual ~MutexBinding()=default;
    virtual void bind_achievement_mutex(execution_recovered::MutexState&,
                                      execution_recovered::Platform&)=0;
};
}
