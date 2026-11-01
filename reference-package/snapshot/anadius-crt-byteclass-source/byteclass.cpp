#include "byteclass.hpp"

namespace anadius::crt_byteclass_recovered {
std::uint32_t classify_06f934(Address locale,std::uint32_t character,
    std::uint32_t ctype_mask,std::uint32_t multibyte_mask,Effects& effects) {
    const auto context=effects.locale_update_058b04(locale);
    const auto byte=static_cast<std::uint8_t>(character);
    bool matched=(effects.read_u8(context.multibyte_info+0x19+byte)&
        static_cast<std::uint8_t>(multibyte_mask))!=0;
    if(!matched&&ctype_mask!=0) {
        const auto table=effects.read_address(context.locale_info);
        matched=(effects.read_u16(table+std::uint64_t{byte}*2)&ctype_mask)!=0;
    }
    const auto result=matched?std::uint32_t{1}:std::uint32_t{0};
    if(context.clear_thread_flag!=0) {
        const auto address=context.ptd+0x3a8;
        effects.write_u32(address,effects.read_u32(address)&0xfffffffdU);
    }
    return result;
}
std::uint32_t is_lead_06f9a8(std::uint32_t character,Effects& effects) {
    return classify_06f934(0,character,0,4,effects);
}
std::int32_t ParserEffects::is_mb_lead_06f9a8(std::int32_t character) {
    return static_cast<std::int32_t>(is_lead_06f9a8(static_cast<std::uint32_t>(character),effects_));
}
}
