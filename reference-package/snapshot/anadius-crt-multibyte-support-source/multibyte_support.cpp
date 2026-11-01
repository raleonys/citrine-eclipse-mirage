#include "multibyte_support.hpp"
#include <bit>

namespace anadius::crt_multibyte_support_recovered {
std::int32_t system_cp_06efd0(std::int32_t request,Effects& e) {
    const auto locale=e.locale_update_058b04(0);
    e.write_system_cp_flag_0a59d8(0);
    auto result=std::bit_cast<std::uint32_t>(request);
    if(request==-2) {
        e.write_system_cp_flag_0a59d8(1);
        result=e.get_oemcp();
    } else if(request==-3) {
        e.write_system_cp_flag_0a59d8(1);
        result=e.get_acp();
    } else if(request==-4) {
        e.write_system_cp_flag_0a59d8(1);
        result=e.read_u32(locale.locale_info+0xc);
    }
    if(locale.clear_thread_flag) {
        const auto flags=e.read_u32(locale.ptd+0x3a8);
        e.write_u32(locale.ptd+0x3a8,flags&0xfffffffdU);
    }
    return std::bit_cast<std::int32_t>(result);
}
namespace {
void invalid(Effects& e) {
    e.errno_cell_05d724().store(22);
    e.invalid_parameter_05c954();
}
void copy_256(Address destination,Address source,Effects& e) {
    // Two loop iterations, eight MOVUPS operations per iteration. Both
    // qwords of each source chunk are captured before any destination write.
    for(unsigned block=0;block<2;++block) {
        for(unsigned chunk=0;chunk<8;++chunk) {
            const auto a=e.read_address(source);
            const auto b=e.read_address(source+8);
            e.write_address(destination,a);
            e.write_address(destination+8,b);
            source+=16;
            destination+=16;
        }
    }
}
void zero(Address destination,unsigned bytes,Effects& e) {
    for(unsigned i=0;i<bytes;++i) e.write_u8(destination+i,0);
}
}
void publish_06ee00(Address& ptd,Address& current_slot,std::uint32_t lockid,
    std::uint32_t unlockid,Effects& e) {
    e.lock_0625f8(lockid);
    auto source=e.read_address(ptd+0x88)+0x18;
    auto destination=e.type_table_0a59c0();
    if(!destination) invalid(e);
    else if(!source) {
        zero(destination,257,e);
        invalid(e);
    } else {
        copy_256(destination,source,e);
        const auto tail=e.read_u8(source+256);
        e.write_u8(destination+256,tail);
    }
    source=e.read_address(ptd+0x88)+0x119;
    destination=e.case_table_0a59c8();
    if(!destination) invalid(e);
    else if(!source) {
        zero(destination,256,e);
        invalid(e);
    } else copy_256(destination,source,e);

    auto old=e.read_address(current_slot);
    const auto references=e.atomic_fetch_add_32(old,0xffffffffU);
    if(references==1) {
        old=e.read_address(current_slot);
        if(old!=e.initial_multibyte_0a1c70()) {
            // The machine performs a second load for the free argument.
            e.free_0668c0(e.read_address(current_slot));
        }
    }
    // Capture destination before loading PTD's replacement record, then
    // re-read PTD after publishing before incrementing its record reference.
    const auto current_ptd=ptd;
    const auto slot=current_slot;
    const auto replacement=e.read_address(current_ptd+0x88);
    e.write_address(slot,replacement);
    const auto referenced=e.read_address(ptd+0x88);
    (void)e.atomic_fetch_add_32(referenced,1);
    e.unlock_06264c(unlockid);
}
std::uint32_t mark_locale_changed_06532c(Effects& e) {
    return e.exchange_locale_changed_0a52d8(1);
}
}
