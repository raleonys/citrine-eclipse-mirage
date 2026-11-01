#include "initial_locale.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::thread_domain {
namespace data=crt_ptd_data_recovered;
InitialLocale::InitialLocale(crt_heap_recovered::State& heap,crt_heap_recovered::Effects& effects,
    crt_lock_native::Banks& banks,std::uintptr_t exception_identity)
    :heap_(heap),heap_effects_(effects),banks_(banks),exception_identity_(exception_identity){
    if(!exception_identity_)throw std::invalid_argument("A live explicit exception-table identity is required");
}
data::Identities InitialLocale::ids() const noexcept {
    const auto& globals=image_.globals();
    return {exception_identity_,static_cast<std::uintptr_t>(globals.initial_multibyte),
        static_cast<std::uintptr_t>(globals.initial_locale)};
}
std::array<std::uintptr_t,2> InitialLocale::pair() const noexcept {
    const auto identity=ids();return {identity.initial_locale,identity.multibyte_info};
}
crt_locks_recovered::LockRef InitialLocale::lock_ref(std::uint32_t id){
    if(id!=4&&id!=5)throw std::out_of_range("Initial locale supports only ACRT locks4/5");
    return {crt_locks_recovered::Bank::acrt,static_cast<std::int64_t>(id)};
}
void InitialLocale::lock_0625f8(std::uint32_t id){banks_.enter(lock_ref(id));}
void InitialLocale::unlock_06264c(std::uint32_t id){banks_.leave(lock_ref(id));}
std::uint32_t InitialLocale::atomic_fetch_add_32(data::Address address,std::uint32_t increment){
    if(address!=ids().multibyte_info)throw std::out_of_range("Foreign multibyte counter is outside initial-locale domain");
    return image_.atomic_fetch_add_32(address,increment);
}
void InitialLocale::free_0668c0(data::Address address){
    crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(address),heap_,heap_effects_);
}
data::Address InitialLocale::current_locale_0a52e0(){return ids().initial_locale;}
void InitialLocale::require_initial(data::Address locale) const {
    if(locale!=ids().initial_locale)throw std::out_of_range("Foreign/dynamic locale is outside initial-locale domain");
}
void InitialLocale::release_locale_ref_0708c0(data::Address locale){
    require_initial(locale);
    crt_locale_ref_recovered::release_locale_ref_0708c0(locale,image_.reference_identities(),image_);
}
std::uint32_t InitialLocale::locale_ref_count_10(data::Address locale){
    require_initial(locale);return image_.counter_value(locale+0x10);
}
void InitialLocale::free_locale_0706c0(data::Address){
    throw std::out_of_range("Initial locale cannot be deep-freed; dynamic locales are unsupported");
}
void InitialLocale::add_locale_ref_070634(data::Address locale){
    require_initial(locale);
    crt_locale_ref_recovered::add_locale_ref_070634(locale,image_.reference_identities(),image_);
}
static_assert(!std::is_abstract_v<InitialLocale>);
static_assert(!std::is_copy_constructible_v<InitialLocale>&&!std::is_move_constructible_v<InitialLocale>);
static_assert(sizeof(data::Address)==sizeof(crt_initial_state::Address));
} // namespace anadius::thread_domain
