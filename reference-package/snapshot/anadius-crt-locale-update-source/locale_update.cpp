#include "locale_update.hpp"

namespace anadius::crt_locale_update_recovered {
void update_locale_info_064ef0(Address ptd,Address& field,Effects& effects) {
    const auto current=effects.current_locale_0a52e0();
    if(field==current) return;
    const auto flags=effects.read_u32(ptd+0x3a8);
    const auto mask=effects.locale_mask_0a1c6c();
    if((flags&mask)!=0) return;
    field=effects.update_thread_locale_070968();
}
void update_multibyte_info_064f5c(Address ptd,Address& field,Effects& effects) {
    const auto current=effects.current_multibyte_0a59d0();
    if(field==current) return;
    const auto flags=effects.read_u32(ptd+0x3a8);
    const auto mask=effects.locale_mask_0a1c6c();
    if((flags&mask)!=0) return;
    field=effects.update_thread_multibyte_06f658();
}
void locale_update_058b04(Context& out,Address locale,Effects& effects) {
    out.clear_thread_flag=0;
    if(locale!=0) {
        // The original MOVUPS snapshots the pair before storing either qword;
        // this also preserves overlapping input/output pair semantics.
        const std::array<Address,2> pair{
            effects.read_address(locale),effects.read_address(locale+8)};
        out.locale_info=pair[0];
        out.multibyte_info=pair[1];
        return;
    }
    if(effects.locale_changed_0a52d8()==0) {
        const auto pair=effects.global_pair_0a1938();
        out.locale_info=pair[0];
        out.multibyte_info=pair[1];
        return;
    }
    const auto ptd=effects.required_ptd_064b90();
    out.ptd=ptd;
    out.locale_info=effects.read_address(ptd+0x90);
    out.multibyte_info=effects.read_address(ptd+0x88);
    update_locale_info_064ef0(ptd,out.locale_info,effects);
    update_multibyte_info_064f5c(out.ptd,out.multibyte_info,effects);
    const auto flags_ptd=out.ptd;
    const auto flags=effects.read_u32(flags_ptd+0x3a8);
    if((flags&2u)!=0) return;
    effects.write_u32(flags_ptd+0x3a8,flags|2u);
    out.clear_thread_flag=1;
}
}
