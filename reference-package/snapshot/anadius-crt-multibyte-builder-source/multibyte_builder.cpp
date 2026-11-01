#include "multibyte_builder.hpp"
#include <array>
#include <stdexcept>

namespace anadius::crt_multibyte_builder_recovered {
namespace {
struct Builtin {
    std::uint32_t cp;
    std::array<std::uint16_t,6> words;
    std::array<std::array<std::uint8_t,8>,4> ranges;
};
constexpr std::array<std::uint8_t,4> flags{1,2,4,8};
constexpr std::array<Builtin,5> builtins{{
    {932,{33376,33401,33,0,0,0},{{{166,223,0,0,0,0,0,0},{161,165,0,0,0,0,0,0},{129,159,224,252,0,0,0,0},{64,126,128,252,0,0,0,0}}}},
    {936,{41921,41946,32,0,0,0},{{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{129,254,0,0,0,0,0,0},{64,254,0,0,0,0,0,0}}}},
    {949,{41921,41946,32,0,0,0},{{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{129,254,0,0,0,0,0,0},{65,254,0,0,0,0,0,0}}}},
    {950,{41679,41700,26,41701,41704,91},{{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{129,254,0,0,0,0,0,0},{64,126,161,254,0,0,0,0}}}},
    {1361,{55889,55902,32,55903,55914,50},{{{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{129,211,216,222,224,249,0,0},{49,126,129,254,0,0,0,0}}}}
}};
Address locale_name(std::uint32_t cp,Effects& fx) {
    if(cp==932)return fx.locale_name_08bf70();
    if(cp==936)return fx.locale_name_08bf78();
    if(cp==949)return fx.locale_name_08bf80();
    if(cp==950)return fx.locale_name_08bf88();
    return 0;
}
void clear_type(Address record,Effects& fx) {
    for(std::uint32_t i=0;i!=257;++i)fx.write_u8(record+0x18+i,0);
}
void or_byte(Address address,std::uint8_t flag,Effects& fx) {
    const auto previous=fx.read_u8(address);
    fx.write_u8(address,static_cast<std::uint8_t>(previous|flag));
}
}
std::uint32_t build_multibyte_06f674(std::int32_t request,Address record,Effects& fx) {
    const auto cp=static_cast<std::uint32_t>(fx.system_cp_06efd0(request));
    if(cp==0){fx.reset_sbcs_06f050(record);return 0;}
    for(const auto& row:builtins){
        if(row.cp!=cp)continue;
        clear_type(record,fx);
        for(std::uint32_t group=0;group!=4;++group){
            const auto& ranges=row.ranges[group];
            for(std::uint32_t pair=0;ranges[pair]!=0&&ranges[pair+1]!=0;pair+=2){
                auto value=static_cast<std::uint32_t>(ranges[pair]);
                while(value<=ranges[pair+1]){
                    const auto index=value+1;
                    if(index>=257)break;
                    ++value;
                    or_byte(record+0x18+index,flags[group],fx);
                }
            }
        }
        fx.write_u32(record+4,cp);
        fx.write_u32(record+8,1);
        fx.write_address(record+0x220,locale_name(cp,fx));
        for(std::uint32_t i=0;i!=6;++i)fx.write_u16(record+0xc+i*2,row.words[i]);
        fx.build_case_tables_06f0e8(record);
        return 0;
    }
    if(cp==65000||fx.is_valid_code_page(cp&0xffff)==0)return 0xffffffff;
    if(cp==65001){
        fx.write_address(record+4,65001);
        fx.write_address(record+0x220,0);
        fx.write_u32(record+0x18,0);
        fx.write_u16(record+0x1c,0);
    }else{
        CPInfo info{};
        if(fx.get_cp_info(cp,info)==0){
            if(fx.system_cp_flag_0a59d8()==0)return 0xffffffff;
            fx.reset_sbcs_06f050(record);
            return 0;
        }
        clear_type(record,fx);
        const auto is_dbcs=info.max_char_size==2;
        fx.write_u32(record+4,cp);
        fx.write_address(record+0x220,0);
        if(is_dbcs){
            for(std::size_t pair=0;;pair+=2){
                if(pair>=info.lead_bytes.size())
                    throw std::out_of_range("06f674 reconstruction boundary: unterminated CPInfo lead bytes");
                if(info.lead_bytes[pair]==0||info.lead_bytes[pair+1]==0)break;
                const auto first=static_cast<std::uint32_t>(info.lead_bytes[pair]);
                const auto last=static_cast<std::uint32_t>(info.lead_bytes[pair+1]);
                if(first<=last){
                    auto index=first+1;
                    auto count=last-first+1;
                    do{or_byte(record+0x18+index,4,fx);++index;}while(--count!=0);
                }
            }
            for(std::uint32_t i=0;i!=254;++i)or_byte(record+0x1a+i,8,fx);
            // Assembly reloads the stored CP after table writes.
            fx.write_address(record+0x220,locale_name(fx.read_u32(record+4),fx));
        }
        fx.write_u32(record+8,is_dbcs?1:0);
    }
    for(std::uint32_t i=0;i!=6;++i)fx.write_u16(record+0xc+i*2,0);
    fx.build_case_tables_06f0e8(record);
    return 0;
}
}
