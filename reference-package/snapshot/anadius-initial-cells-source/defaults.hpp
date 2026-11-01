#pragma once
#include "../anadius-native-context-source/context.hpp"
#include "../anadius-licensing-effects-source/effects.hpp"

namespace anadius::host_platform {struct AssemblyInputs;}
namespace anadius::resources_factory {struct Inputs;}
namespace anadius::initial_cells {
// Explicit fresh-start projections of observed PE cells and001050/001090/
//0010d0 empty set construction. No native initializer/CRT code is executed.
// Original MSVC container/sentinel layout and exception ABI are not reproduced.
native_context::CallbackSeed make_callback_defaults();
licensing_effects::AttemptState make_license_attempt_defaults();
// Same Ready Domain, before host/resources install. Both destination roots
// must be absent; existing caller state is never reset, replaced or merged.
// All construction completes before either output is committed. Passive:
// no files, SDK calls, callback publication, worker start or hook installation.
void select_defaults(host_platform::AssemblyInputs&,resources_factory::Inputs&);
}
