#include "bridge.hpp"
namespace anadius::crt_locale_bridge {
ThreadEffects::ThreadEffects(Globals& g,Memory& m,crt_ptd_recovered::State& p,crt_ptd_recovered::Effects& pf,
    crt_ptd_data_recovered::Effects& d,crt_heap_recovered::State& h,crt_heap_recovered::Effects& hf)
    :globals_(g),memory_(m),ptd_(p),ptd_fx_(pf),data_(d),heap_(h),heap_fx_(hf){}
Address ThreadEffects::read_address(Address a){return memory_.read_address(a);}
void ThreadEffects::write_address(Address a,Address b){memory_.write_address(a,b);}
std::uint32_t ThreadEffects::read_u32(Address a){return memory_.read_u32(a);}
std::uint32_t ThreadEffects::locale_mask_0a1c6c(){return globals_.mask_0a1c6c;}
Address ThreadEffects::current_locale_0a52e0(){return globals_.locale_0a52e0;}
Address ThreadEffects::current_multibyte_slot_0a59d0(){return reinterpret_cast<Address>(&globals_.multibyte_0a59d0);}
Address ThreadEffects::initial_locale_0a17e0(){return globals_.initial_locale_0a17e0;}
Address ThreadEffects::initial_multibyte_0a1c70(){return globals_.initial_multibyte_0a1c70;}
Address ThreadEffects::required_ptd_064b90(){return reinterpret_cast<Address>(crt_ptd_recovered::required_preserve_error_064b90(ptd_,ptd_fx_));}
void ThreadEffects::lock_0625f8(std::uint32_t i){data_.lock_0625f8(i);}
void ThreadEffects::unlock_06264c(std::uint32_t i){data_.unlock_06264c(i);}
std::uint32_t ThreadEffects::atomic_fetch_add_32(Address a,std::uint32_t i){return data_.atomic_fetch_add_32(a,i);}
void ThreadEffects::free_0668c0(Address a){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(a),heap_,heap_fx_);}
void ThreadEffects::add_locale_ref_070634(Address a){data_.add_locale_ref_070634(a);}
void ThreadEffects::release_locale_ref_0708c0(Address a){data_.release_locale_ref_0708c0(a);}
void ThreadEffects::free_locale_0706c0(Address a){data_.free_locale_0706c0(a);}
void ThreadEffects::abort_05f03c(){ptd_fx_.abort_05f03c();}
UpdateEffects::UpdateEffects(Globals& g,Memory& m,ThreadEffects& t):globals_(g),memory_(m),thread_(t){}
Address UpdateEffects::read_address(Address a){return memory_.read_address(a);}
std::uint32_t UpdateEffects::read_u32(Address a){return memory_.read_u32(a);}
void UpdateEffects::write_u32(Address a,std::uint32_t v){memory_.write_u32(a,v);}
std::uint32_t UpdateEffects::locale_changed_0a52d8(){return globals_.changed_0a52d8;}
std::array<Address,2> UpdateEffects::global_pair_0a1938(){return globals_.pair_0a1938;}
Address UpdateEffects::required_ptd_064b90(){return thread_.required_ptd_064b90();}
Address UpdateEffects::current_locale_0a52e0(){return globals_.locale_0a52e0;}
Address UpdateEffects::current_multibyte_0a59d0(){return globals_.multibyte_0a59d0;}
std::uint32_t UpdateEffects::locale_mask_0a1c6c(){return globals_.mask_0a1c6c;}
Address UpdateEffects::update_thread_locale_070968(){return crt_locale_thread_recovered::update_thread_locale_070968(thread_);}
Address UpdateEffects::update_thread_multibyte_06f658(){return crt_locale_thread_recovered::update_thread_multibyte_06f658(thread_);}
WildcardEffects::WildcardEffects(crt_recalloc_bridge::Allocator& a,crt_conversion_recovered::Effects& c,
    crt_runtime_recovered::State& r,crt_runtime_recovered::Effects& rf,crt_conversion_api::FileApiEffects& f,
    crt_find_native::Session& s,crt_locale_update_recovered::Effects& l)
    :crt_find_bridge::WildcardEffects(a,c,r,rf,f,s),locale_(l){}
crt_byteclass_recovered::LocaleContext WildcardEffects::locale_update_058b04(Address a){
    // Zero is new metadata for the untouched fast-path PTD field. It is never
    // dereferenced when clear_thread_flag is zero; original fast paths leave it.
    crt_byteclass_recovered::LocaleContext context{};
    crt_locale_update_recovered::locale_update_058b04(context,a,locale_);
    return context;
}
}
