#include "runtime/strict_consistency_status.hpp"
#include "runtime/strict_consistency_ledger.hpp"
#include "preloader_l/runtime/runtime_status.hpp"
#include <assert.h>
using namespace preloader_l::runtime;
int main(){
 const auto strict=active_strict_consistency_status_snapshot();
 const auto runtime=active_runtime_snapshot();
 assert(strict.strict_open_mask==strict_ledger::strict_open_mask);
 assert(strict.conceptual_open_count==strict_ledger::conceptual_open_count);
 assert(strict.current_host_blocker_mask==0u);
 assert(runtime.strict_model_consistency_open_mask==strict.strict_open_mask);
 return 0;
}
