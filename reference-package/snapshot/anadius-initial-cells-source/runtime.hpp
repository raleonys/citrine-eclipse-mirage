#pragma once
#include "../anadius-bootstrap-runtime-source/observed_runtime.hpp"

namespace anadius::initial_cells {
// Explicit fresh-image projection of cells outside036d20: zero QPF cache,
// empty host registry map/target vector, ten zero original function slots.
// Does not reset live cells, execute001010/CRT, or infer hook success/ABI.
bootstrap_runtime::OutsideConstructorState make_outside_defaults();
// Selects both observed036d20 state and these outside-cell defaults. All
// default construction/preflight occurs before consuming Resources, through
// the existing observed-runtime sink. Actual providers/errors remain required.
// Passive controller construction outside loader lock; no entry publication,
// workers, file/config reads, hooks, native atexit or loader definitions.
std::unique_ptr<bootstrap_runtime::Runtime> make_fresh_runtime(
    bootstrap_backend::Resources&,bootstrap_native_recovered::Errors&,
    bootstrap_runtime::HostApi=bootstrap_runtime::native_host_api());
}
