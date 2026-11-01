#pragma once
#include "../anadius-application-worker-source/worker.hpp"

namespace anadius::resources_factory {struct Inputs;}
namespace anadius::initial_cells {
// Fresh projection of file-backed signed dword a1130 = -1, used by0314f0.
// The empty achievement_file is a legacy HOST placeholder: selected native
// Resources binds the actual live Runtime XML path through PathBinding.
// No live avatar counter or path is reset, and no native CRT code is run.
application_worker::Settings::StateSeed make_worker_defaults();
// Ready Domain, before Resources installation; reject any existing Worker
// state, including custom0/nonempty paths. Leaves configure/owners untouched.
void select_worker_defaults(resources_factory::Inputs&);
}
