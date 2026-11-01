#include "multibyte_control.hpp"
#include <array>
#include <cstring>

namespace anadius::crt_multibyte_control_recovered {
namespace support=crt_multibyte_support_recovered;
std::uint32_t set_multibyte_06f2d0(std::int32_t request,std::uint8_t initializing,
    Address ptd,Address current_slot,Effects& e) {
    (void)e.update_thread_multibyte_06f540(ptd,current_slot);
    const auto cp=support::system_cp_06efd0(request,e);
    const auto current=e.read_address(ptd+0x88);
    if(static_cast<std::uint32_t>(cp)==e.read_u32(current+4)) return 0;
    const auto fresh=e.malloc_06a308(0x228);
    if(fresh==0) {e.free_0668c0(0);return UINT32_MAX;}

    // The binary first copies all 0x228 bytes to a stack snapshot, then copies
    // the snapshot to the new allocation. Do not interleave reads and writes.
    std::array<std::byte,0x228> snapshot{};
    const auto source=e.read_address(ptd+0x88);
    for(std::size_t n=0;n<snapshot.size();n+=8) {
        const auto word=e.read_address(source+n);
        std::memcpy(snapshot.data()+n,&word,sizeof word);
    }
    for(std::size_t n=0;n<snapshot.size();n+=8) {
        Address word;
        std::memcpy(&word,snapshot.data()+n,sizeof word);
        e.write_address(fresh+n,word);
    }
    e.write_u32(fresh,0);
    const auto status=e.build_multibyte_06f674(cp,fresh);
    if(status==UINT32_MAX) {
        e.errno_cell_05d724().store(22);
        e.free_0668c0(fresh);
        return UINT32_MAX;
    }
    if(initializing==0) (void)support::mark_locale_changed_06532c(e);
    const auto old=e.read_address(ptd+0x88);
    const auto previous=e.atomic_fetch_add_32(old,UINT32_MAX);
    if(previous==1) {
        const auto candidate=e.read_address(ptd+0x88);
        if(candidate!=e.initial_multibyte_0a1c70()) e.free_0668c0(candidate);
    }
    e.write_u32(fresh,1);
    e.write_address(ptd+0x88,fresh);
    const auto flags=e.read_u32(ptd+0x3a8);
    if((e.locale_mask_0a1c6c()&flags)==0) {
        support::publish_06ee00(ptd,current_slot,5,5,e);
        if(initializing!=0) e.set_initial_pair_multibyte_0a1940(e.read_address(current_slot));
    }
    e.free_0668c0(0);
    return status;
}
bool initialize_multibyte_06f5f8(const Identities& ids,Effects& e) {
    if(e.initialized_0a59dc()==0) {
        e.set_case_table_0a59c8(ids.initial_case_table_0a1fb0);
        const auto slot=e.current_multibyte_slot_0a59d0();
        e.write_address(slot,ids.initial_multibyte_0a1c70);
        e.set_type_table_0a59c0(ids.initial_type_table_0a1ea0);
        const auto ptd=e.required_ptd_raw_064c64();
        (void)set_multibyte_06f2d0(-3,1,ptd,slot,e);
        e.set_initialized_0a59dc(1);
    }
    return true;
}
}
