#include "bridge.hpp"
#include <bit>
#include <limits>
namespace anadius::crt_nls_bridge {
struct Effects::Invocation {
    Effects& e;
    std::span<const std::uint8_t> input;
    std::span<WordOutput> words;
    std::span<ByteOutput> bytes;
    Address output;
    std::uint64_t frame,ticket{};
    bool borrowed{};
    Invocation(Effects& fx,std::span<const std::uint8_t> in)
        :e(fx),input(e.input_),words(e.words_),bytes(e.bytes_),output(e.output_),frame(e.d_.native.begin_scratch_frame()){
        try{if(!in.empty()){ticket=e.d_.memory.borrow_readable(reinterpret_cast<Address>(in.data()),in.size());borrowed=true;}}
        catch(...){e.d_.native.end_scratch_frame(frame);throw;}
        e.input_=in;e.words_={};e.bytes_={};e.output_=0;
    }
    ~Invocation(){
        e.input_=input;e.words_=words;e.bytes_=bytes;e.output_=output;
        if(borrowed)e.d_.memory.release_readable(ticket);
        e.d_.native.end_scratch_frame(frame);
    }
};
Effects::Effects(Dependencies d):d_(d){}
std::uint32_t Effects::get_acp(){return d_.native.get_acp();}
std::uint32_t Effects::get_oemcp(){return d_.native.get_oemcp();}
std::uint32_t Effects::exchange_32(Address a,std::uint32_t v){return d_.native.exchange_32(a,v);}
std::uint32_t Effects::is_valid_code_page(std::uint32_t cp){return d_.native.is_valid_code_page(cp);}
std::uint32_t Effects::get_cp_info(std::uint32_t cp,CPInfo& out){return d_.native.get_cp_info(cp,out);}
namespace {
std::int32_t count(std::size_t n){if(n>static_cast<std::size_t>(INT32_MAX))throw std::out_of_range("NLS span exceeds signed input count");return static_cast<std::int32_t>(n);}
}
std::uint32_t Effects::get_string_type_06e210(Address l,std::uint32_t t,
    std::span<const std::uint8_t> in,std::span<WordOutput> out,std::uint32_t cp,std::uint32_t strict){
    Invocation frame(*this,in);words_=out;output_=reinterpret_cast<Address>(out.data());
    return crt_string_type_recovered::get_string_type_06e210(l,t,reinterpret_cast<Address>(in.data()),count(in.size()),output_,cp,std::bit_cast<std::int32_t>(strict),*this);
}
std::uint32_t Effects::map_string_06cc30(Address l,Address name,std::uint32_t flags,
    std::span<const std::uint8_t> in,std::span<ByteOutput> out,std::uint32_t cp,std::uint32_t strict){
    Invocation frame(*this,in);bytes_=out;output_=reinterpret_cast<Address>(out.data());
    return crt_lcmap_recovered::lc_map_string_06cc30(l,name,flags,reinterpret_cast<Address>(in.data()),count(in.size()),output_,count(out.size()),cp,std::bit_cast<std::int32_t>(strict),*this);
}
crt_byteclass_recovered::LocaleContext Effects::locale_update_058b04(Address a){
    crt_byteclass_recovered::LocaleContext c{};crt_locale_update_recovered::locale_update_058b04(c,a,d_.locale);return c;
}
std::uint8_t Effects::read_u8(Address a){
    if(!input_.empty()&&a==reinterpret_cast<Address>(input_.data())+input_.size())return d_.native.read_input_tail(a);
    return d_.memory.read_u8(a);
}
std::uint32_t Effects::read_u32(Address a){return d_.memory.read_u32(a);}
void Effects::write_u32(Address a,std::uint32_t v){d_.memory.write_u32(a,v);}
Address Effects::malloc_06a308(std::uint64_t n){return reinterpret_cast<Address>(crt_realloc_recovered::malloc_base_06a308(n,d_.heap,d_.heap_fx));}
void Effects::free_0668c0(Address a){crt_heap_recovered::free_0668c0(reinterpret_cast<void*>(a),d_.heap,d_.heap_fx);}
Address Effects::stack_allocate(std::uint64_t n){return d_.native.stack_allocate(n);}
void Effects::zero_bytes(Address a,std::uint64_t n){for(std::uint64_t i=0;i<n;++i)d_.memory.write_u8(a+i,0);}
std::int32_t Effects::multibyte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap){return multi_byte_to_wide_06ccc8(cp,f,s,n,d,cap);}
std::int32_t Effects::multi_byte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap){return crt_conversion_api::multi_byte_to_wide_06ccc8(cp,f,s,n,d,cap,*this);}
std::int32_t Effects::wide_char_to_multi_byte_06cd58(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address d,std::int32_t cap,Address def,Address used){return crt_conversion_api::wide_char_to_multi_byte_06cd58(cp,f,s,n,d,cap,def,used,*this);}
std::uint32_t Effects::native_get_string_type_w(std::uint32_t t,Address s,std::int32_t n,Address out){
    if(out!=output_)throw std::logic_error("NLS word output outside invocation");return d_.native.get_string_type_w(t,s,n,out,words_);
}
std::int32_t Effects::lc_map_string_0671d4(Address name,std::uint32_t f,Address s,std::int32_t n,Address out,std::int32_t cap,Address v,Address r,std::uint64_t h){
    return d_.native.lc_map_string_0671d4(name,f,s,n,out,cap,v,r,h,out==output_?bytes_:std::span<ByteOutput>{});
}
std::int32_t Effects::multi_byte_to_wide(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address out,std::int32_t cap){return d_.native.multi_byte_to_wide(cp,f,s,n,out,cap);}
std::int32_t Effects::wide_char_to_multi_byte(std::uint32_t cp,std::uint32_t f,Address s,std::int32_t n,Address out,std::int32_t cap,Address def,Address used){
    return d_.native.wide_char_to_multi_byte(cp,f,s,n,out,cap,def,used,out==output_?bytes_:std::span<ByteOutput>{});
}
void Effects::write_bool32(Address a,std::int32_t v){d_.memory.write_u32(a,std::bit_cast<std::uint32_t>(v));}
}
