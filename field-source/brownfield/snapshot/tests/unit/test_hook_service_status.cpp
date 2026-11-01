#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_consistency_open_registry_hook_engine_13a70==(1u<<0),"13A70 engine consistency bit drift");
static_assert(active_consistency_open_registry_hook_scope_13a70==active_consistency_open_registry_hook_engine_13a70,"compatibility 13A70 alias drift");
static_assert(active_consistency_open_optional_host_callback_1019d==(1u<<11),"1019D consistency bit drift");
int main(){
 const auto s=active_runtime_snapshot();
 if(s.unresolved_mask!=0u)return 1;
 if(!(s.runtime_active_mask&active_runtime_active_registry_stage_13a70))return 2;
 if(!(s.runtime_active_mask&active_runtime_active_htons_stage_12980))return 3;
 if(s.consistency_open_mask&active_consistency_open_registry_hook_engine_13a70)return 4;
 if(s.consistency_open_mask&active_consistency_open_htons_hook_engine_12980)return 5;
 if(s.consistency_open_mask&active_consistency_open_service_object_layout_7000)return 6;
 if(s.consistency_open_mask!=0u)return 7;
 if(s.strict_model_consistency_open_mask!=0u)return 8;
 return 0;
}
