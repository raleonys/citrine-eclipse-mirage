#pragma once
#include <cstdint>
#include "../anadius-crt-byteclass-source/byteclass.hpp"
#include "../anadius-crt-error-source/errors.hpp"

namespace anadius::crt_multibyte_support_recovered {
using Address=std::uint64_t;
class Effects {
public:
    virtual ~Effects()=default;
    virtual crt_byteclass_recovered::LocaleContext locale_update_058b04(Address)=0;
    virtual Address read_address(Address)=0;
    virtual void write_address(Address,Address)=0;
    virtual std::uint8_t read_u8(Address)=0;
    virtual void write_u8(Address,std::uint8_t)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
    virtual std::uint32_t get_acp()=0;
    virtual std::uint32_t get_oemcp()=0;
    virtual void write_system_cp_flag_0a59d8(std::uint32_t)=0;
    virtual Address type_table_0a59c0()=0;
    virtual Address case_table_0a59c8()=0;
    virtual Address initial_multibyte_0a1c70()=0;
    virtual void lock_0625f8(std::uint32_t)=0;
    virtual void unlock_06264c(std::uint32_t)=0;
    virtual std::uint32_t atomic_fetch_add_32(Address,std::uint32_t)=0;
    virtual void free_0668c0(Address)=0;
    virtual crt_errors_recovered::Cell32 errno_cell_05d724()=0;
    virtual void invalid_parameter_05c954()=0;
    virtual std::uint32_t exchange_locale_changed_0a52d8(std::uint32_t)=0;
};
std::int32_t system_cp_06efd0(std::int32_t request,Effects&);
// References represent the caller's two mutable pointer locals. The helper
// intentionally re-reads those locals after effects, as the assembly does.
void publish_06ee00(Address& ptd,Address& current_slot,std::uint32_t lockid,
    std::uint32_t unlockid,Effects&);
std::uint32_t mark_locale_changed_06532c(Effects&);
}
