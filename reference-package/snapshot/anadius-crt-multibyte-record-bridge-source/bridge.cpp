#include "bridge.hpp"
namespace anadius::crt_multibyte_record_bridge {
RecordNative::RecordNative(Memory& m,crt_multibyte_bridge::Globals& g,
    const Identities& i,crt_multibyte_record_bridge::NativeEffects& n)
    :memory_(m),globals_(g),identities_(i),native_(n){}
void RecordNative::bind_system_cp(crt_multibyte_support_recovered::Effects& e){
    if(support_&&support_!=&e)throw std::logic_error("record bridge support domain already bound");
    support_=&e;
}
std::uint32_t RecordNative::get_acp(){return native_.get_acp();}
std::uint32_t RecordNative::get_oemcp(){return native_.get_oemcp();}
std::uint32_t RecordNative::exchange_32(Address a,std::uint32_t v){return native_.exchange_32(a,v);}
std::uint32_t RecordNative::build_multibyte_06f674(std::int32_t cp,Address a){return crt_multibyte_builder_recovered::build_multibyte_06f674(cp,a,*this);}
std::uint8_t RecordNative::read_u8(Address a){return memory_.read_u8(a);}
std::uint16_t RecordNative::read_u16(Address a){return memory_.read_u16(a);}
std::uint32_t RecordNative::read_u32(Address a){return memory_.read_u32(a);}
Address RecordNative::read_address(Address a){return memory_.read_address(a);}
void RecordNative::write_u8(Address a,std::uint8_t v){memory_.write_u8(a,v);}
void RecordNative::write_u16(Address a,std::uint16_t v){memory_.write_u16(a,v);}
void RecordNative::write_u32(Address a,std::uint32_t v){memory_.write_u32(a,v);}
void RecordNative::write_address(Address a,Address v){memory_.write_address(a,v);}
std::int32_t RecordNative::system_cp_06efd0(std::int32_t cp){
    if(!support_)throw std::logic_error("record bridge requires live systemCP binding");
    return crt_multibyte_support_recovered::system_cp_06efd0(cp,*support_);
}
std::uint32_t RecordNative::system_cp_flag_0a59d8(){return globals_.system_cp_flag_0a59d8;}
std::uint32_t RecordNative::is_valid_code_page(std::uint32_t cp){return native_.is_valid_code_page(cp);}
std::uint32_t RecordNative::get_cp_info(std::uint32_t cp,CPInfo& out){return native_.get_cp_info(cp,out);}
Address RecordNative::initial_multibyte_0a1c70(){return identities_.initial_multibyte_0a1c70;}
void RecordNative::reset_sbcs_06f050(Address a){crt_multibyte_tables_recovered::reset_sbcs_06f050(a,*this);}
void RecordNative::build_case_tables_06f0e8(Address a){crt_multibyte_tables_recovered::build_case_tables_06f0e8(a,*this);}
Address RecordNative::locale_name_08bf70(){return identities_.locale_names[0];}
Address RecordNative::locale_name_08bf78(){return identities_.locale_names[1];}
Address RecordNative::locale_name_08bf80(){return identities_.locale_names[2];}
Address RecordNative::locale_name_08bf88(){return identities_.locale_names[3];}
std::uint32_t RecordNative::get_string_type_06e210(Address l,std::uint32_t t,
    std::span<const std::uint8_t> in,std::span<WordOutput> out,std::uint32_t cp,std::uint32_t flags){
    return native_.get_string_type_06e210(l,t,in,out,cp,flags);
}
std::uint32_t RecordNative::map_string_06cc30(Address l,Address n,std::uint32_t f,
    std::span<const std::uint8_t> in,std::span<ByteOutput> out,std::uint32_t cp,std::uint32_t flags){
    return native_.map_string_06cc30(l,n,f,in,out,cp,flags);
}
}
