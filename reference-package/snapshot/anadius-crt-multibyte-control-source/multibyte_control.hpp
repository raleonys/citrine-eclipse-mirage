#pragma once
#include <cstdint>
#include "../anadius-crt-multibyte-support-source/multibyte_support.hpp"

namespace anadius::crt_multibyte_control_recovered {
using Address=std::uint64_t;
class Effects:public crt_multibyte_support_recovered::Effects {
public:
    virtual Address malloc_06a308(std::uint64_t)=0;
    virtual std::uint32_t build_multibyte_06f674(std::int32_t,Address)=0;
    virtual Address update_thread_multibyte_06f540(Address,Address)=0;
    virtual Address required_ptd_raw_064c64()=0;
    virtual std::uint8_t initialized_0a59dc()=0;
    virtual void set_initialized_0a59dc(std::uint8_t)=0;
    virtual void set_type_table_0a59c0(Address)=0;
    virtual void set_case_table_0a59c8(Address)=0;
    virtual Address current_multibyte_slot_0a59d0()=0;
    virtual void set_initial_pair_multibyte_0a1940(Address)=0;
    virtual std::uint32_t locale_mask_0a1c6c()=0;
};
struct Identities {
    Address initial_multibyte_0a1c70;
    Address initial_type_table_0a1ea0;
    Address initial_case_table_0a1fb0;
};
std::uint32_t set_multibyte_06f2d0(std::int32_t request,std::uint8_t initializing,
    Address ptd,Address current_slot,Effects&);
// Only AL is the recovered result. Upper bits of the original RAX are not an ABI promise.
bool initialize_multibyte_06f5f8(const Identities&,Effects&);
}
