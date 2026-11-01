#include "initial_mb.hpp"
#include "../anadius-crt-multibyte-tables-source/multibyte_tables.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string_view>
namespace im=anadius::crt_initial_multibyte;
namespace mt=anadius::crt_multibyte_tables_recovered;
namespace {
unsigned checks=0;
void check(bool yes){++checks;if(!yes)throw std::runtime_error("initial MB check "+std::to_string(checks));}
struct Memory final:mt::Effects {
    std::array<std::uint8_t,im::record_size> output;
    Memory(){output.fill(0xa5);}
    std::uint8_t* at(mt::Address p,std::size_t n){check(p>=0x2000&&p-0x2000+n<=output.size());return output.data()+p-0x2000;}
    std::uint8_t read_u8(mt::Address p)override{check(p>=0x1000&&p<0x1000+im::record_size);return im::record_bytes()[p-0x1000];}
    std::uint16_t read_u16(mt::Address)override{throw std::logic_error("reset unused read16");}
    std::uint32_t read_u32(mt::Address)override{throw std::logic_error("reset unused read32");}
    mt::Address read_address(mt::Address)override{throw std::logic_error("reset unused read pointer");}
    void write_u8(mt::Address p,std::uint8_t v)override{*at(p,1)=v;}
    void write_u16(mt::Address p,std::uint16_t v)override{std::memcpy(at(p,2),&v,2);}
    void write_u32(mt::Address p,std::uint32_t v)override{std::memcpy(at(p,4),&v,4);}
    void write_address(mt::Address p,mt::Address v)override{std::memcpy(at(p,8),&v,8);}
    mt::Address initial_multibyte_0a1c70()override{return 0x1000;}
    std::uint32_t get_cp_info(std::uint32_t,mt::CPInfo&)override{throw std::logic_error("reset unused CPInfo");}
    std::uint32_t get_string_type_06e210(mt::Address,std::uint32_t,std::span<const std::uint8_t>,std::span<mt::WordOutput>,std::uint32_t,std::uint32_t)override{throw std::logic_error("reset unused type");}
    std::uint32_t map_string_06cc30(mt::Address,mt::Address,std::uint32_t,std::span<const std::uint8_t>,std::span<mt::ByteOutput>,std::uint32_t,std::uint32_t)override{throw std::logic_error("reset unused map");}
};
void verify(){
    const auto& record=im::record_bytes();const auto& type=im::initial_type_table();const auto& map=im::initial_case_table();
    for(unsigned i=0;i<0x18;++i)check(record[i]==0);
    for(unsigned i=0;i<257;++i)check(record[0x18+i]==type[i]);
    for(unsigned i=0;i<256;++i){check(record[0x119+i]==map[i]);const bool upper=i>='A'&&i<='Z',lower=i>='a'&&i<='z';check(type[i+1]==(upper?0x10:lower?0x20:0));check(map[i]==(upper?i+32:lower?i-32:0));}
    for(unsigned i=0x219;i<record.size();++i)check(record[i]==0);
    check(type[0]==0&&im::image_globals().locale_mask==0xfffffffeU&&im::image_globals().locale_changed==0&&im::image_globals().system_cp_flag==0&&im::image_globals().initialized==0);
    Memory memory;mt::reset_sbcs_06f050(0x2000,memory);
    for(unsigned i=0;i<record.size();++i){const auto expected=i<4||(i>=0x219&&i<0x220)?0xa5:record[i];check(memory.output[i]==expected);}
}
template<std::size_t N>void hex(const std::array<std::uint8_t,N>& a){std::cout<<'"'<<std::hex<<std::setfill('0');for(auto byte:a)std::cout<<std::setw(2)<<static_cast<unsigned>(byte);std::cout<<'"'<<std::dec;}
}
int main(int argc,char** argv){try{if(argc==2&&std::string_view(argv[1])=="--dump"){std::cout<<"{\"record\":";hex(im::record_bytes());std::cout<<",\"initial_type_table\":";hex(im::initial_type_table());std::cout<<",\"initial_case_table\":";hex(im::initial_case_table());std::cout<<",\"locale_mask\":"<<im::image_globals().locale_mask<<"}\n";return 0;}verify();std::cout<<"PASS "<<checks<<" image initial multibyte/reset checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
