#include "native.hpp"
namespace anadius::crt_locale_native {
Resources::Resources(crt_locale_bridge::Globals& g,crt_locale_bridge::Memory& m,
    const crt_locale_ref_recovered::Identities& i,const crt_locale_free_recovered::Defaults& d,
    crt_heap_recovered::State& h,crt_heap_recovered::Effects& heap,
    crt_locks_recovered::Effects& l,crt_counter_native::Counters& c)
    :crt_locale_resource_bridge::Resources(g,m,i,d,h,heap),locks_(l),counters_(c){}
void Resources::lock_0625f8(std::uint32_t i){crt_locks_recovered::lock_acrt_0625f8(locks_,static_cast<std::int32_t>(i));}
void Resources::unlock_06264c(std::uint32_t i){crt_locks_recovered::unlock_acrt_06264c(locks_,static_cast<std::int32_t>(i));}
std::uint32_t Resources::atomic_fetch_add_32(crt_ptd_data_recovered::Address a,std::uint32_t n){return counters_.fetch_add(a,n);}
}
