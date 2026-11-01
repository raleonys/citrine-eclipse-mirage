#include "bridge.hpp"
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace anadius::vcrt_bridge {
LockEffects::LockEffects(crt_runtime_recovered::State& runtime,
    crt_runtime_recovered::Effects& effects,crt_locks_recovered::InitializeEffects& initialize,
    vcrt_api_recovered::State& api,vcrt_api_recovered::Effects& api_effects,LockPlatform& platform)
    :runtime_(runtime),runtime_effects_(effects),initialize_(initialize),api_(api),api_effects_(api_effects),platform_(platform){}
std::uint64_t LockEffects::initialize_acrt_06707c(crt_locks_recovered::LockRef ref,std::uint32_t spin,std::uint32_t flags) {
    return crt_locks_recovered::initialize_acrt_api_06707c(runtime_,runtime_effects_,initialize_,ref,spin,flags);
}
std::uint64_t LockEffects::initialize_vcrt_050c38(crt_locks_recovered::LockRef ref,std::uint32_t spin,std::uint32_t flags) {
    return static_cast<std::uint32_t>(vcrt_api_recovered::initialize_critical_section_ex_050c38(
        api_,api_effects_,platform_.storage(ref),spin,flags));
}
void LockEffects::delete_critical_section(crt_locks_recovered::LockRef ref){platform_.destroy(ref);}
void LockEffects::enter_critical_section(crt_locks_recovered::LockRef ref){platform_.enter(ref);}
void LockEffects::leave_critical_section(crt_locks_recovered::LockRef ref){platform_.leave(ref);}
PtdEffects::PtdEffects(vcrt_api_recovered::State& api,vcrt_api_recovered::Effects& api_effects,
    vcrt_api_recovered::FlsCallback callback,crt_heap_recovered::State& heap,crt_heap_recovered::Effects& heap_effects,
    crt_locks_recovered::State& locks,crt_locks_recovered::Effects& lock_effects,ThreadPolicy& policy)
    :api_(api),api_effects_(api_effects),callback_(callback),heap_(heap),heap_effects_(heap_effects),locks_(locks),lock_effects_(lock_effects),policy_(policy) {
    if(!callback_)throw std::invalid_argument("Rebuilt VCRT FLS destructor callback required");
}
std::uint32_t PtdEffects::fls_alloc_050b0c(vcrt_ptd_recovered::FlsCallback callback) {
    if(callback!=vcrt_ptd_recovered::FlsCallback::destroy_04d5d4)throw std::invalid_argument("Unknown VCRT FLS callback");
    return vcrt_api_recovered::fls_alloc_050b0c(api_,api_effects_,callback_);
}
bool PtdEffects::fls_free_050b54(std::uint32_t index){return vcrt_api_recovered::fls_free_050b54(api_,api_effects_,index)!=0;}
void* PtdEffects::fls_get_050b9c(std::uint32_t index){return vcrt_api_recovered::fls_get_value_050b9c(api_,api_effects_,index);}
bool PtdEffects::fls_set_050be4(std::uint32_t index,void* value){return vcrt_api_recovered::fls_set_value_050be4(api_,api_effects_,index,value)!=0;}
vcrt_ptd_recovered::Ptd* PtdEffects::calloc_05cae4(std::size_t count,std::size_t bytes) {
    if(count!=1||bytes!=sizeof(vcrt_ptd_recovered::Ptd))throw std::invalid_argument("Only the recovered VCRT PTD allocation shape is supported");
    auto* storage=crt_heap_recovered::calloc_base_066848(count,bytes,heap_,heap_effects_);
    return storage?std::construct_at(static_cast<vcrt_ptd_recovered::Ptd*>(storage)):nullptr;
}
void PtdEffects::free_05caec(void* value){crt_heap_recovered::free_0668c0(value,heap_,heap_effects_);}
std::uint32_t PtdEffects::last_error(){return api_effects_.last_error();}
void PtdEffects::set_last_error(std::uint32_t value){policy_.set_last_error(value);}
bool PtdEffects::initialize_locks_05093c(){return crt_locks_recovered::initialize_vcrt_05093c(locks_,lock_effects_);}
void PtdEffects::uninitialize_locks_050984(){(void)crt_locks_recovered::uninitialize_vcrt_050984(locks_,lock_effects_);}
[[noreturn]]void PtdEffects::abort_05f03c(){policy_.abort_05f03c();}
LifecycleBridge::LifecycleBridge(crt_initializers_recovered::State& state,
    std::span<crt_initializers_recovered::Pair> pairs,
    std::span<crt_initializers_recovered::Address> c,std::span<crt_initializers_recovered::Address> cpp,
    crt_initializers_recovered::Effects& effects,crt_ptd_recovered::State& ptd,crt_ptd_recovered::Effects& ptd_effects,Dependencies deps)
    :AcrtTablesBridge(state,pairs,c,cpp,effects,ptd,ptd_effects),deps_(deps){}
void LifecycleBridge::initialize_cpu_04b26c(){(void)crt_cpu_recovered::initialize_cpu_04b26c(deps_.cpu,deps_.cpu_effects);}
std::uint64_t LifecycleBridge::vcrt_initialize_04d55c(){return vcrt_ptd_recovered::initialize_04d55c(deps_.ptd,deps_.effects);}
std::uint64_t LifecycleBridge::vcrt_uninitialize_04d5a8(bool terminating){return vcrt_ptd_recovered::uninitialize_04d5a8(deps_.ptd,deps_.effects,terminating);}
std::uint64_t LifecycleBridge::vcrt_thread_attach_04d584(){return vcrt_ptd_recovered::thread_attach_04d584(deps_.ptd,deps_.effects);}
void LifecycleBridge::vcrt_thread_detach_04d598(){(void)vcrt_ptd_recovered::thread_detach_04d598(deps_.ptd,deps_.effects);}
void LifecycleBridge::vcrt_late_cleanup_04d5c4(){(void)vcrt_ptd_recovered::late_uninitialize_04d5c4(deps_.ptd,deps_.effects);}
static_assert(std::is_abstract_v<LifecycleBridge>);
static_assert(std::is_trivially_destructible_v<vcrt_ptd_recovered::Ptd>);
}
