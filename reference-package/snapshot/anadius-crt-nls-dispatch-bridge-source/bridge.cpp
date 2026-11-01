#include "bridge.hpp"
namespace anadius::crt_nls_dispatch_bridge {
Effects::Effects(crt_nls_bridge::NativeEffects& b,NativeCalls& n,crt_runtime_recovered::State& s,
    crt_runtime_recovered::Effects& r,crt_multibyte_record_bridge::Memory& m)
    :backend_(b),native_(n),runtime_(s),resolver_(r),memory_(m){}
std::uint32_t Effects::get_acp(){return backend_.get_acp();}
std::uint32_t Effects::get_oemcp(){return backend_.get_oemcp();}
std::uint32_t Effects::exchange_32(Address a,std::uint32_t v){return backend_.exchange_32(a,v);}
std::uint32_t Effects::is_valid_code_page(std::uint32_t cp){return backend_.is_valid_code_page(cp);}
std::uint32_t Effects::get_cp_info(std::uint32_t cp,CPInfo& out){return backend_.get_cp_info(cp,out);}
std::uint64_t Effects::begin_scratch_frame(){return backend_.begin_scratch_frame();}
void Effects::end_scratch_frame(std::uint64_t frame)noexcept{backend_.end_scratch_frame(frame);}
Address Effects::stack_allocate(std::uint64_t n){return backend_.stack_allocate(n);}
std::uint8_t Effects::read_input_tail(Address a){return backend_.read_input_tail(a);}
std::int32_t Effects::multi_byte_to_wide(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap){return backend_.multi_byte_to_wide(cp,f,s,n,d,cap);}
std::int32_t Effects::wide_char_to_multi_byte(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap,Address def,Address used,std::span<ByteOutput> out){return backend_.wide_char_to_multi_byte(cp,f,s,n,d,cap,def,used,out);}
std::uint32_t Effects::get_string_type_w(std::uint32_t type,Address s,std::int32_t n,Address d,std::span<WordOutput> out){return backend_.get_string_type_w(type,s,n,d,out);}
std::int32_t Effects::lc_map_string_0671d4(Address name,std::uint32_t flags,Address s,std::int32_t n,Address d,std::int32_t cap,Address v,Address r,std::uint64_t h,std::span<ByteOutput> out){
    struct Scope {std::span<ByteOutput>& slot;std::span<ByteOutput> old;~Scope(){slot=old;}} scope{output_,output_};
    output_=out;
    return crt_nls_dispatch_recovered::lc_map_string_0671d4(name,flags,s,n,d,cap,v,r,h,*this);
}
std::uint32_t Effects::locale_name_to_lcid_0672c0(Address name,std::uint32_t flags){return crt_nls_dispatch_recovered::locale_name_to_lcid_0672c0(name,flags,*this);}
Address Effects::resolve_0669b0(std::uint32_t p,std::string_view name,std::span<const std::uint32_t> modules){return crt_runtime_recovered::resolve_0669b0(runtime_,resolver_,p,name,modules);}
std::int32_t Effects::call_lc_map_string_ex(Address fn,Address name,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap,Address v,Address r,Address h){return native_.lc_map_string_ex(fn,name,f,s,n,d,cap,v,r,h,output_);}
std::int32_t Effects::call_lc_map_string_w(std::uint32_t id,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap){return native_.lc_map_string_w(id,f,s,n,d,cap,output_);}
std::uint32_t Effects::call_locale_name_to_lcid(Address fn,Address name,std::uint32_t f){return native_.locale_name_to_lcid(fn,name,f);}
std::uint32_t Effects::fallback_locale_name_07310c(Address name){return crt_locale_names_recovered::locale_name_to_lcid_07310c(name,*this);}
std::uint16_t Effects::read_u16(Address a){return memory_.read_u16(a);}
}
