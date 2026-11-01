#include "byteclass.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace bc=anadius::crt_byteclass_recovered;
namespace {
unsigned checks;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("check "+std::to_string(checks));}
struct Fixture final:bc::Effects {
    static constexpr bc::Address ptd=0x1000,locale=0x2000,mb=0x3000,ctype=0x4000;
    std::array<std::uint8_t,256> multibyte{};std::array<std::uint16_t,256> flags{};
    std::uint32_t thread_flags=0xa5a5a5a7;std::uint8_t clear=0;bool fail_byte=false;
    bc::Address requested=~bc::Address{0};std::vector<char> events;
    bc::LocaleContext locale_update_058b04(bc::Address input)override{requested=input;events.push_back('l');return{ptd,locale,mb,clear};}
    std::uint8_t read_u8(bc::Address address)override{check(address>=mb+0x19&&address<mb+0x119);events.push_back('b');if(fail_byte)throw std::runtime_error("synthetic read failure");return multibyte[static_cast<std::size_t>(address-mb-0x19)];}
    std::uint16_t read_u16(bc::Address address)override{check(address>=ctype&&address<ctype+512&&(address-ctype)%2==0);events.push_back('w');return flags[static_cast<std::size_t>((address-ctype)/2)];}
    std::uint32_t read_u32(bc::Address address)override{check(address==ptd+0x3a8);events.push_back('r');return thread_flags;}
    bc::Address read_address(bc::Address address)override{check(address==locale);events.push_back('p');return ctype;}
    void write_u32(bc::Address address,std::uint32_t value)override{check(address==ptd+0x3a8);events.push_back('s');thread_flags=value;}
};
void masks_and_widths(){
    for(unsigned byte=0;byte<256;++byte)for(unsigned variant=0;variant<4;++variant){
        Fixture f;f.multibyte[byte]=static_cast<std::uint8_t>(byte);f.flags[byte]=static_cast<std::uint16_t>(byte*257U);
        const std::array<std::uint32_t,4> masks{0U,1U<<((byte+3)%8),0x10000U,0xffff0000U|byte};
        const auto mask=masks[variant];f.clear=static_cast<std::uint8_t>(variant==3?0x80:0);
        const auto result=bc::classify_06f934(0x5555,0xffff0000U|byte,mask,0x100U|4U,f);
        const bool mb=(byte&4)!=0;
        const bool expected=mb||((static_cast<std::uint16_t>(byte*257U)&mask)!=0);
        check(result==static_cast<unsigned>(expected));check(f.requested==0x5555);
        std::vector<char> events{'l','b'};if(!mb&&mask){events.push_back('p');events.push_back('w');}
        if(f.clear){events.push_back('r');events.push_back('s');}
        check(f.events==events);check(f.thread_flags==(f.clear?0xa5a5a5a5U:0xa5a5a5a7U));
    }
    Fixture high;high.multibyte[0x81]=0xff;high.flags[0x81]=0xffff;
    check(bc::classify_06f934(0,0x81,0,0x100,high)==0); // DIL, high mask bits discarded
    check(high.events==std::vector<char>({'l','b'}));
}
void wrapper_and_normal_flow(){
    for(unsigned byte=0;byte<256;++byte){Fixture f;f.multibyte[byte]=static_cast<std::uint8_t>(byte);
        bc::ParserEffects parser(f);const auto signed_byte=static_cast<std::int32_t>(static_cast<std::int8_t>(byte));
        check(parser.is_mb_lead_06f9a8(signed_byte)==static_cast<int>((byte&4)!=0));check(f.requested==0&&f.events==std::vector<char>({'l','b'}));
    }
    Fixture failure;failure.clear=1;failure.fail_byte=true;
    bool caught=false;try{(void)bc::is_lead_06f9a8(0x81,failure);}catch(const std::runtime_error&){caught=true;}
    check(caught&&failure.events==std::vector<char>({'l','b'})&&failure.thread_flags==0xa5a5a5a7);
}
}
int main(){try{masks_and_widths();wrapper_and_normal_flow();std::cout<<"PASS "<<checks<<" byte classification checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
