#include "multibyte_builder.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>
namespace b=anadius::crt_multibyte_builder_recovered;
std::uint64_t checks{};
void require(bool ok){++checks;if(!ok)throw std::runtime_error("builder check failed");}
struct Fixture final:b::Effects {
    static constexpr b::Address base=0x10000;
    std::array<std::uint8_t,0x228> bytes;
    std::uint32_t cp{},flag{},valid=0x80000000,info_result=0x80000000;
    b::CPInfo info{};
    std::uint32_t system_calls{},valid_calls{},info_calls{},flag_calls{},sbcs_calls{},case_calls{},valid_arg{},info_arg{},locale_calls{};
    std::int32_t request{};
    bool mutate_cp_on_trail{};
    std::vector<std::uint32_t> write_offsets;
    Fixture(){bytes.fill(0xa5);}
    std::size_t at(b::Address address,std::size_t size=1){require(address>=base&&address+size<=base+bytes.size());return static_cast<std::size_t>(address-base);}
    template<class T>T read(b::Address address){T value;std::memcpy(&value,bytes.data()+at(address,sizeof(T)),sizeof(T));return value;}
    template<class T>void write(b::Address address,T value){const auto off=at(address,sizeof(T));write_offsets.push_back(static_cast<std::uint32_t>(off));std::memcpy(bytes.data()+off,&value,sizeof(T));}
    std::uint8_t read_u8(b::Address a)override{return read<std::uint8_t>(a);}
    void write_u8(b::Address a,std::uint8_t v)override{write(a,v);if(mutate_cp_on_trail&&a==base+0x117){write_u32(base+4,932);}}
    std::uint32_t read_u32(b::Address a)override{return read<std::uint32_t>(a);}
    void write_u16(b::Address a,std::uint16_t v)override{write(a,v);}
    void write_u32(b::Address a,std::uint32_t v)override{write(a,v);}
    void write_address(b::Address a,b::Address v)override{write(a,v);}
    std::int32_t system_cp_06efd0(std::int32_t r)override{++system_calls;request=r;return std::bit_cast<std::int32_t>(cp);}
    std::uint32_t system_cp_flag_0a59d8()override{++flag_calls;return flag;}
    std::uint32_t is_valid_code_page(std::uint32_t v)override{++valid_calls;valid_arg=v;return valid;}
    std::uint32_t get_cp_info(std::uint32_t v,b::CPInfo& out)override{++info_calls;info_arg=v;out=info;return info_result;}
    void reset_sbcs_06f050(b::Address a)override{require(a==base);++sbcs_calls;}
    void build_case_tables_06f0e8(b::Address a)override{require(a==base);++case_calls;}
    b::Address locale_name_08bf70()override{++locale_calls;return 0x70000;}
    b::Address locale_name_08bf78()override{++locale_calls;return 0x78000;}
    b::Address locale_name_08bf80()override{++locale_calls;return 0x80000;}
    b::Address locale_name_08bf88()override{++locale_calls;return 0x88000;}
    std::uint32_t run(std::int32_t r=-3){return b::build_multibyte_06f674(r,base,*this);}
};
bool in(unsigned value,unsigned first,unsigned last){return value>=first&&value<=last;}
void builtins(){
    constexpr std::array<unsigned,5> cps{932,936,949,950,1361};
    constexpr std::array<std::array<std::uint16_t,6>,5> words{{{0x8260,0x8279,33,0,0,0},{0xa3c1,0xa3da,32,0,0,0},{0xa3c1,0xa3da,32,0,0,0},{0xa2cf,0xa2e4,26,0xa2e5,0xa2e8,91},{0xda51,0xda5e,32,0xda5f,0xda6a,50}}};
    constexpr std::array<b::Address,5> names{0x70000,0x78000,0x80000,0x88000,0};
    for(std::size_t row=0;row<cps.size();++row){
        Fixture f;f.cp=cps[row];f.valid=0;require(f.run()==0);
        require(f.system_calls==1&&f.request==-3&&f.valid_calls==0&&f.info_calls==0&&f.sbcs_calls==0&&f.case_calls==1);
        require(f.read_u32(f.base+4)==cps[row]&&f.read_u32(f.base+8)==1);
        require(f.read<b::Address>(f.base+0x220)==names[row]);
        for(unsigned i=0;i!=6;++i)require(f.read<std::uint16_t>(f.base+0xc+i*2)==words[row][i]);
        require(f.bytes[0x18]==0);
        for(unsigned c=0;c!=256;++c){
            unsigned flags=0;
            switch(cps[row]){
            case 932:if(in(c,0xa6,0xdf))flags|=1;if(in(c,0xa1,0xa5))flags|=2;if(in(c,0x81,0x9f)||in(c,0xe0,0xfc))flags|=4;if(in(c,0x40,0x7e)||in(c,0x80,0xfc))flags|=8;break;
            case 936:if(in(c,0x81,0xfe))flags|=4;if(in(c,0x40,0xfe))flags|=8;break;
            case 949:if(in(c,0x81,0xfe))flags|=4;if(in(c,0x41,0xfe))flags|=8;break;
            case 950:if(in(c,0x81,0xfe))flags|=4;if(in(c,0x40,0x7e)||in(c,0xa1,0xfe))flags|=8;break;
            case 1361:if(in(c,0x81,0xd3)||in(c,0xd8,0xde)||in(c,0xe0,0xf9))flags|=4;if(in(c,0x31,0x7e)||in(c,0x81,0xfe))flags|=8;break;
            }
            require(f.bytes[0x19+c]==flags);
        }
        for(unsigned i=0x119;i<0x220;++i)require(f.bytes[i]==0xa5);
        require(f.bytes[0]==0xa5&&f.bytes[3]==0xa5);
        require(f.write_offsets.front()==0x18&&f.write_offsets[256]==0x118);
    }
}
void no_write_paths(){
    for(auto cp:{0u,65000u,0xffffffffu,1234u}){
        Fixture f;f.cp=cp;f.valid=0;const auto before=f.bytes;
        const auto status=f.run(17);
        require(status==(cp==0?0u:0xffffffffu));require(f.bytes==before);
        require(f.sbcs_calls==(cp==0?1u:0u));require(f.case_calls==0&&f.info_calls==0&&f.flag_calls==0);
        require(f.valid_calls==((cp==0||cp==65000)?0u:1u));
        if(f.valid_calls)require(f.valid_arg==(cp&0xffff));
    }
    for(auto flag:{0u,1u,0x80000000u}){
        Fixture f;f.cp=1234;f.info_result=0;f.flag=flag;auto before=f.bytes;
        require(f.run()==(flag?0u:0xffffffffu));require(f.bytes==before);
        require(f.sbcs_calls==(flag?1u:0u)&&f.case_calls==0&&f.flag_calls==1);
    }
}
void utf8(){
    Fixture f;f.cp=65001;const auto before=f.bytes;require(f.run()==0);
    require(f.valid_calls==1&&f.info_calls==0&&f.flag_calls==0&&f.case_calls==1);
    for(unsigned i=0;i<f.bytes.size();++i){
        if(in(i,4,0x1d)||in(i,0x220,0x227))continue;
        require(f.bytes[i]==before[i]);
    }
    require(f.read<b::Address>(f.base+4)==65001&&f.read<b::Address>(f.base+0x220)==0);
    for(unsigned i=0xc;i!=0x1e;++i)require(f.bytes[i]==0);
    const std::vector<std::uint32_t> order{4,0x220,0x18,0x1c,0xc,0xe,0x10,0x12,0x14,0x16};
    require(f.write_offsets==order);
}
void dynamic(){
    // Every byte as a lead endpoint, including zero terminator and 0xff.
    for(unsigned first=0;first!=256;++first){
        for(auto last:{0u,first,255u}){
            Fixture f;f.cp=0x80011234;f.info.max_char_size=2;
            f.info.lead_bytes[0]=static_cast<std::uint8_t>(first);f.info.lead_bytes[1]=static_cast<std::uint8_t>(last);
            require(f.run()==0&&f.valid_arg==0x1234&&f.info_arg==0x80011234);
            require(f.case_calls==1&&f.flag_calls==0&&f.read_u32(f.base+8)==1);
            require(f.read<b::Address>(f.base+0x220)==0);
            require(f.bytes[0x18]==0);
            for(unsigned c=0;c!=256;++c){
                const auto lead=first!=0&&last!=0&&in(c,first,last);
                const auto trail=in(c,1,254);
                require(f.bytes[0x19+c]==((lead?4:0)|(trail?8:0)));
            }
            for(unsigned i=0xc;i!=0x18;++i)require(f.bytes[i]==0);
        }
    }
    // Reversed interval is skipped and a later interval is still consumed.
    Fixture reversed;reversed.cp=1234;reversed.info.max_char_size=2;reversed.info.lead_bytes={200,100,254,255,0,0,0,0,0,0,0,0};
    require(reversed.run()==0&&reversed.bytes[0x118]==4&&reversed.bytes[0x117]==12&&reversed.bytes[0xe1]==8);
    for(auto max:{0u,1u,3u,4u,0xffffffffu}){
        Fixture f;f.cp=1234;f.info.max_char_size=max;f.info.lead_bytes={1,255,0,0,0,0,0,0,0,0,0,0};require(f.run()==0);
        require(f.read_u32(f.base+8)==0&&f.locale_calls==0);
        for(unsigned i=0x18;i!=0x119;++i)require(f.bytes[i]==0);
    }
    // Provider instrumentation changes the stored CP after the final trail OR;
    // locale selection must reread the record rather than cached request CP.
    Fixture reload;reload.cp=1234;reload.info.max_char_size=2;reload.mutate_cp_on_trail=true;
    require(reload.run()==0);require(reload.read<b::Address>(reload.base+0x220)==0x70000&&reload.locale_calls==1);
    Fixture malformed;malformed.cp=1234;malformed.info.max_char_size=2;malformed.info.lead_bytes.fill(1);
    bool boundary=false;try{(void)malformed.run();}catch(const std::out_of_range&){boundary=true;}
    require(boundary&&malformed.case_calls==0);
}
int main(){try{builtins();no_write_paths();utf8();dynamic();std::printf("PASS multibyte builder: %llu checks; builtin constants + synthetic CP providers, no native API/target execution\n",static_cast<unsigned long long>(checks));return 0;}catch(const std::exception& e){std::fprintf(stderr,"%s after %llu checks\n",e.what(),static_cast<unsigned long long>(checks));return 1;}}
