#pragma once
#include <array>
#include <cstdint>
#include "../anadius-crt-byteclass-source/byteclass.hpp"

namespace anadius::crt_locale_update_recovered {
using Address=std::uint64_t;
using Context=crt_byteclass_recovered::LocaleContext;
class Effects {
public:
    virtual ~Effects()=default;
    virtual Address read_address(Address)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
    virtual std::uint32_t locale_changed_0a52d8()=0;
    virtual std::array<Address,2> global_pair_0a1938()=0;
    virtual Address required_ptd_064b90()=0;
    virtual Address current_locale_0a52e0()=0;
    virtual Address current_multibyte_0a59d0()=0;
    virtual std::uint32_t locale_mask_0a1c6c()=0;
    virtual Address update_thread_locale_070968()=0;
    virtual Address update_thread_multibyte_06f658()=0;
};
void update_locale_info_064ef0(Address ptd,Address& field,Effects&);
void update_multibyte_info_064f5c(Address ptd,Address& field,Effects&);
// Writes only the original constructor's fields. In particular, fast paths
// preserve out.ptd. No destructor, SEH or native locale backend is implied.
void locale_update_058b04(Context& out,Address locale,Effects&);
}
