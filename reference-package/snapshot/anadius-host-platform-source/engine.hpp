#pragma once
#include "assembly.hpp"
#include "../hook-engine-source/owned.hpp"

namespace anadius::host_platform {
// Before install_host, on its controller. Requires vacant engine/code slots and
// an explicit external_quiescence proof. Commits the same owning engine as API
// and code lifetime, composing external proof with actual engine-empty status.
// It does not run the engine, infer hook success or provide external drain.
// Lower Api must not be called/uninitialized through an independent alias.
void select_owned_engine(AssemblyInputs&,std::shared_ptr<recovered_hooks::OwnedEngine>);
// Original profile: same actual engine/code owner, with no cooperative ready/
// unload proof callback. Requires empty engine/code/proof slots. Passive.
void select_original_owned_engine(AssemblyInputs&,std::shared_ptr<recovered_hooks::OwnedEngine>);
}
