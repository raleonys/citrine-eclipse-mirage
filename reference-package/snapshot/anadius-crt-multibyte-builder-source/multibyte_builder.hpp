#pragma once
#include <cstdint>
#include "../anadius-crt-multibyte-tables-source/multibyte_tables.hpp"

namespace anadius::crt_multibyte_builder_recovered {
using Address=std::uint64_t;
using CPInfo=crt_multibyte_tables_recovered::CPInfo;
class Effects {
public:
    virtual ~Effects()=default;
    virtual std::uint8_t read_u8(Address)=0;
    virtual void write_u8(Address,std::uint8_t)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual void write_u16(Address,std::uint16_t)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
    virtual void write_address(Address,Address)=0;
    virtual std::int32_t system_cp_06efd0(std::int32_t)=0;
    virtual std::uint32_t system_cp_flag_0a59d8()=0;
    virtual std::uint32_t is_valid_code_page(std::uint32_t)=0;
    virtual std::uint32_t get_cp_info(std::uint32_t,CPInfo&)=0;
    virtual void reset_sbcs_06f050(Address)=0;
    virtual void build_case_tables_06f0e8(Address)=0;
    virtual Address locale_name_08bf70()=0;
    virtual Address locale_name_08bf78()=0;
    virtual Address locale_name_08bf80()=0;
    virtual Address locale_name_08bf88()=0;
};
// Constants are the exact five shipped rows at 1800a20c0, not OS CP data.
std::uint32_t build_multibyte_06f674(std::int32_t request,Address record,Effects&);
}
