#include "bridge.hpp"
namespace anadius::crt_nls_native_bridge {
Backend::Backend(crt_nls_native::Calls& n,ContextEffects& c):native_(n),context_(c){}
std::uint32_t Backend::get_acp(){return native_.get_acp();}
std::uint32_t Backend::get_oemcp(){return native_.get_oemcp();}
std::uint32_t Backend::is_valid_code_page(std::uint32_t cp){return native_.is_valid_code_page(cp);}
std::uint32_t Backend::get_cp_info(std::uint32_t cp,CPInfo& out){return native_.get_cp_info(cp,out);}
std::uint32_t Backend::exchange_32(Address a,std::uint32_t v){return context_.exchange_32(a,v);}
std::uint64_t Backend::begin_scratch_frame(){return context_.begin_scratch_frame();}
void Backend::end_scratch_frame(std::uint64_t frame)noexcept{context_.end_scratch_frame(frame);}
Address Backend::stack_allocate(std::uint64_t n){return context_.stack_allocate(n);}
std::uint8_t Backend::read_input_tail(Address a){return context_.read_input_tail(a);}
std::int32_t Backend::multi_byte_to_wide(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap){return native_.multi_byte_to_wide(cp,f,s,n,d,cap);}
std::int32_t Backend::wide_char_to_multi_byte(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap,Address def,Address used,std::span<ByteOutput> out){return native_.wide_char_to_multi_byte(cp,f,s,n,d,cap,def,used,out);}
std::uint32_t Backend::get_string_type_w(std::uint32_t type,Address s,std::int32_t n,Address d,std::span<WordOutput> out){return native_.get_string_type_w(type,s,n,d,out);}
std::int32_t Backend::lc_map_string_0671d4(Address,std::uint32_t,Address,std::int32_t,Address,std::int32_t,Address,Address,std::uint64_t,std::span<ByteOutput>){throw std::logic_error("native NLS backend requires the recovered dispatch bridge");}
}
