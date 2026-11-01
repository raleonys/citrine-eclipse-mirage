#include "bridge.hpp"
namespace anadius::crt_sync_bridge {
VcrtLockCalls::VcrtLockCalls(crt_lock_native::Banks& b):banks_(b){}
std::int32_t VcrtLockCalls::call_initialize_critical_section_ex(vcrt_api_recovered::Address a,void* p,std::uint32_t s,std::uint32_t f){return banks_.init_ex_at(a,p,s,f);}
std::int32_t VcrtLockCalls::initialize_critical_section_and_spin_count(void* p,std::uint32_t s){return banks_.init_spin_at(p,s);}
PtdSyncEffects::PtdSyncEffects(crt_locks_recovered::Effects& l,AtomicCells& c):locks_(l),cells_(c){}
void PtdSyncEffects::lock_0625f8(std::uint32_t i){crt_locks_recovered::lock_acrt_0625f8(locks_,static_cast<std::int32_t>(i));}
void PtdSyncEffects::unlock_06264c(std::uint32_t i){crt_locks_recovered::unlock_acrt_06264c(locks_,static_cast<std::int32_t>(i));}
std::uint32_t PtdSyncEffects::atomic_fetch_add_32(crt_ptd_data_recovered::Address a,std::uint32_t n){return cells_.resolve(a).fetch_add(n);}
}
