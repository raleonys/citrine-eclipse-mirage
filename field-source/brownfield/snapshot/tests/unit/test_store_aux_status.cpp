#include "preloader_l/runtime/runtime_status.hpp"
using namespace preloader_l::runtime;
static_assert(active_curio_ae00_aux_spans==(1u<<8),"aux curio bit drift");
static_assert(active_integrated_ae00_aux_spans==(1u<<8),"aux integrated bit drift");
static_assert(active_runtime_active_ae00_aux_spans==(1u<<8),"aux active bit drift");
int main(){
 const auto s=active_runtime_snapshot();
 if(!(s.curio_mask&active_curio_ae00_aux_spans))return 1;
 if(!(s.integrated_mask&active_integrated_ae00_aux_spans))return 2;
 if(!(s.runtime_active_mask&active_runtime_active_ae00_aux_spans))return 3;
 if(s.unresolved_mask!=0u)return 4;
 if(s.consistency_open_mask&active_consistency_open_ae00_aux_vector_object_abi)return 5;
 if(s.strict_model_consistency_open_mask!=0u)return 6;
 return 0;
}
