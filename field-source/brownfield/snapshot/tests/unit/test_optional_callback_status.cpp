#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_curio_optional_host_hook_6b31==(1u<<9),"6B31 curio bit drift");
static_assert(active_curio_optional_hook_installer_11a38==(1u<<10),"11A38 curio bit drift");
static_assert(active_consistency_open_optional_host_hook_engine_6b31==(1u<<9),"6B31 consistency bit drift");
static_assert(active_consistency_open_optional_hook_installer_11a38==(1u<<10),"11A38 consistency bit drift");
static_assert(active_consistency_open_optional_host_callback_1019d==(1u<<11),"1019D consistency bit drift");
int main(){
 const auto s=active_runtime_snapshot();
 if(!(s.curio_mask&active_curio_optional_host_hook_6b31))return 1;
 if(!(s.curio_mask&active_curio_optional_hook_installer_11a38))return 2;
 if(s.integrated_mask&(1u<<9))return 3;
 if(s.integrated_mask&(1u<<10))return 4;
 if(s.runtime_active_mask&(1u<<9))return 5;
 if(s.runtime_active_mask&(1u<<10))return 6;
 if(s.consistency_open_mask&active_consistency_open_optional_host_hook_engine_6b31)return 7;
 if(s.consistency_open_mask&active_consistency_open_optional_hook_installer_11a38)return 8;
 if(s.consistency_open_mask&active_consistency_open_optional_host_callback_1019d)return 9;
 if(s.unresolved_mask!=0u)return 10;
 if(s.strict_model_consistency_open_mask!=0u)return 11;
 return 0;
}
