#include "bridge.hpp"
#include <stdexcept>
namespace anadius::crt_find_bridge {
SdkPathProvider::SdkPathProvider(std::span<WCHAR,264> scratch,Getter getter):scratch_(scratch),getter_(getter){
    if(!getter_)throw std::invalid_argument("module-path bridge requires typed GetModuleFileNameW");
}
crt_module_path_recovered::WideResult SdkPathProvider::get_module_filename_w(Address module,std::uint32_t capacity){
    if(capacity>scratch_.size())throw std::domain_error("module-path capacity exceeds supplied typed scratch");
    const auto count=getter_(reinterpret_cast<HMODULE>(module),scratch_.data(),capacity);
    return{reinterpret_cast<Address>(scratch_.data()),count};
}
PathEffects::PathEffects(crt_conversion_recovered::Effects& c,crt_wildcard_recovered::PreviousEffects& l,PathProvider& p)
    :conversion_(c),locale_(l),provider_(p){}
std::uint8_t PathEffects::read_u8(Address a){return conversion_.read_u8(a);}
std::uint16_t PathEffects::read_u16(Address a){return conversion_.read_u16(a);}
void PathEffects::write_zero_u8(Address a){conversion_.write_zero_u8(a);}
void PathEffects::write_zero_u16(Address a){conversion_.write_zero_u16(a);}
Address PathEffects::malloc_06a308(std::uint64_t n){return conversion_.malloc_06a308(n);}
void PathEffects::free_0668c0(Address a){conversion_.free_0668c0(a);}
std::int32_t PathEffects::multi_byte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t flags,Address s,std::int32_t n,Address d,std::int32_t cap){return conversion_.multi_byte_to_wide_06ccc8(cp,flags,s,n,d,cap);}
std::int32_t PathEffects::wide_char_to_multi_byte_06cd58(std::uint32_t cp,std::uint32_t flags,Address s,std::int32_t n,Address d,std::int32_t cap,Address def,Address used){return conversion_.wide_char_to_multi_byte_06cd58(cp,flags,s,n,d,cap,def,used);}
std::uint32_t PathEffects::last_error(){return conversion_.last_error();}
void PathEffects::map_os_error_05d698(std::uint32_t e){conversion_.map_os_error_05d698(e);}
crt_errors_recovered::Cell32 PathEffects::errno_cell_05d724(){return conversion_.errno_cell_05d724();}
crt_module_path_recovered::WideResult PathEffects::get_module_filename_w(Address m,std::uint32_t cap){return provider_.get_module_filename_w(m,cap);}
std::uint32_t PathEffects::choose_code_page(){return crt_wildcard_recovered::select_code_page_region(locale_);}
Policy::Policy(crt_wildcard_recovered::Effects& wildcard,crt_module_path_recovered::Effects& path):ExpansionPolicy(wildcard),path_(path){}
std::uint32_t Policy::module_filename_06ecdc(std::span<char,260> output){return crt_module_path_recovered::module_filename_06ecdc(0,reinterpret_cast<Address>(output.data()),output.size(),path_);}
WildcardEffects::WildcardEffects(crt_recalloc_bridge::Allocator& a,crt_conversion_recovered::Effects& c,
    crt_runtime_recovered::State& r,crt_runtime_recovered::Effects& rf,crt_conversion_api::FileApiEffects& f,crt_find_native::Session& s)
    :crt_conversion_bridge::WildcardEffects(a,c,r,rf,f),find_(s){}
Address WildcardEffects::find_first(Address p,Address& name){return find_.first(p,name);}
std::int32_t WildcardEffects::find_next(Address h,Address& name){return find_.next(h,name);}
void WildcardEffects::find_close(Address h){find_.close(h);}
}
