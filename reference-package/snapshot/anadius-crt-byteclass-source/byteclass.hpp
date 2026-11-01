#pragma once
#include <cstdint>
#include "../anadius-crt-argv-parser-source/parse.hpp"

namespace anadius::crt_byteclass_recovered {
using Address=std::uint64_t;
struct LocaleContext {
    Address ptd,locale_info,multibyte_info;
    std::uint8_t clear_thread_flag;
};
class Effects {
public:
    virtual ~Effects()=default;
    // Rebuilt locale/PTD identities and live bounded table storage; tokens are
    // not original preferred image VAs. Updates remain serialized by provider.
    virtual LocaleContext locale_update_058b04(Address locale)=0;
    virtual std::uint8_t read_u8(Address)=0;
    virtual std::uint16_t read_u16(Address)=0;
    virtual std::uint32_t read_u32(Address)=0;
    virtual Address read_address(Address)=0;
    virtual void write_u32(Address,std::uint32_t)=0;
};
std::uint32_t classify_06f934(Address locale,std::uint32_t character,
    std::uint32_t ctype_mask,std::uint32_t multibyte_mask,Effects&);
std::uint32_t is_lead_06f9a8(std::uint32_t character,Effects&);
// New source bridge to the actual recovered classification bodies. Native
// locale selection, table/PTD ownership and SEH stay required dependencies.
class ParserEffects final:public crt_argv_parse_recovered::Effects {
public:
    explicit ParserEffects(crt_byteclass_recovered::Effects& effects):effects_(effects){}
    std::int32_t is_mb_lead_06f9a8(std::int32_t character) final;
private:
    crt_byteclass_recovered::Effects& effects_;
};
}
