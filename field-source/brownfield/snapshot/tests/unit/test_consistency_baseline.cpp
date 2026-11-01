#include "runtime/strict_consistency_status.hpp"
#include "runtime/strict_consistency_ledger.hpp"
#include "preloader_l/runtime/runtime_status.hpp"
#include <assert.h>
using namespace preloader_l::runtime;
int main(){
 const auto s=active_strict_consistency_status_snapshot();
 assert(s.strict_open_mask==strict_ledger::strict_open_mask);
 assert(s.strict_open_mask==0x00000000u);
 assert(s.conceptual_open_count==0u);
 assert(s.live_source_binding_mask==0u);
 assert(s.compatibility_current_host_na_mask==0u);
 assert(s.behavioral_binary_open_mask==strict_ledger::open_shared_hook_mask);
 return 0;
}
