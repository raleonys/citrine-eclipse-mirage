#include "bridge.hpp"
namespace anadius::crt_multibyte_bridge {
Effects::Effects(Dependencies d):d_(d){}
crt_byteclass_recovered::LocaleContext Effects::locale_update_058b04(Address a){
    crt_byteclass_recovered::LocaleContext context{};
    crt_locale_update_recovered::locale_update_058b04(context,a,d_.locale);
    return context;
}
Address Effects::read_address(Address a){return d_.memory.read_address(a);}
void Effects::write_address(Address a,Address b){d_.memory.write_address(a,b);}
std::uint8_t Effects::read_u8(Address a){return d_.memory.read_u8(a);}
void Effects::write_u8(Address a,std::uint8_t b){d_.memory.write_u8(a,b);}
std::uint32_t Effects::read_u32(Address a){return d_.memory.read_u32(a);}
void Effects::write_u32(Address a,std::uint32_t b){d_.memory.write_u32(a,b);}
std::uint32_t Effects::get_acp(){return d_.native.get_acp();}
std::uint32_t Effects::get_oemcp(){return d_.native.get_oemcp();}
void Effects::write_system_cp_flag_0a59d8(std::uint32_t b){d_.globals.system_cp_flag_0a59d8=b;}
Address Effects::type_table_0a59c0(){return d_.globals.type_table_0a59c0;}
Address Effects::case_table_0a59c8(){return d_.globals.case_table_0a59c8;}
Address Effects::initial_multibyte_0a1c70(){return d_.identities.initial_multibyte_0a1c70;}
void Effects::lock_0625f8(std::uint32_t i){d_.resources.lock_0625f8(i);}
void Effects::unlock_06264c(std::uint32_t i){d_.resources.unlock_06264c(i);}
std::uint32_t Effects::atomic_fetch_add_32(Address a,std::uint32_t i){return d_.resources.atomic_fetch_add_32(a,i);}
void Effects::free_0668c0(Address a){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(a),d_.heap,d_.heap_fx);}
crt_errors_recovered::Cell32 Effects::errno_cell_05d724(){return crt_errors_recovered::errno_cell_05d724(d_.errors,d_.error_fx);}
void Effects::invalid_parameter_05c954(){crt_invalid_recovered::invalid_parameter_noinfo_05c954(d_.invalid);}
std::uint32_t Effects::exchange_locale_changed_0a52d8(std::uint32_t b){return d_.native.exchange_32(reinterpret_cast<Address>(&d_.locale_globals.changed_0a52d8),b);}
Address Effects::malloc_06a308(std::uint64_t n){return reinterpret_cast<Address>(crt_realloc_recovered::malloc_base_06a308(n,d_.heap,d_.heap_fx));}
std::uint32_t Effects::build_multibyte_06f674(std::int32_t cp,Address a){return d_.native.build_multibyte_06f674(cp,a);}
Address Effects::update_thread_multibyte_06f540(Address a,Address slot){return crt_locale_thread_recovered::update_thread_multibyte_data_internal_06f540(a,slot,d_.thread);}
Address Effects::required_ptd_raw_064c64(){return reinterpret_cast<Address>(crt_ptd_recovered::required_raw_064c64(d_.ptd,d_.ptd_fx));}
std::uint8_t Effects::initialized_0a59dc(){return d_.globals.initialized_0a59dc;}
void Effects::set_initialized_0a59dc(std::uint8_t b){d_.globals.initialized_0a59dc=b;}
void Effects::set_type_table_0a59c0(Address a){d_.globals.type_table_0a59c0=a;}
void Effects::set_case_table_0a59c8(Address a){d_.globals.case_table_0a59c8=a;}
Address Effects::current_multibyte_slot_0a59d0(){return reinterpret_cast<Address>(&d_.locale_globals.multibyte_0a59d0);}
void Effects::set_initial_pair_multibyte_0a1940(Address a){d_.locale_globals.pair_0a1938[1]=a;}
std::uint32_t Effects::locale_mask_0a1c6c(){return d_.locale_globals.mask_0a1c6c;}
Policy::Policy(crt_wildcard_recovered::Effects& w,crt_module_path_recovered::Effects& p,
    const crt_multibyte_control_recovered::Identities& i,crt_multibyte_control_recovered::Effects& e)
    :crt_find_bridge::Policy(w,p),identities_(i),effects_(e){}
void Policy::initialize_multibyte_06f5f8(){(void)crt_multibyte_control_recovered::initialize_multibyte_06f5f8(identities_,effects_);}
}
