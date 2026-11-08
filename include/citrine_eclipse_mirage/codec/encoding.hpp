// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/config/configuration.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace citrine_eclipse_mirage::encoding {
using Path=configuration::Path;
struct Result {int count;std::uint32_t error;};
class Calls {
public:
    virtual ~Calls()=default;
    virtual std::uint32_t locale_codepage_0629ec()=0;
    virtual bool are_file_apis_ansi()=0;
    virtual int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char*,int,char16_t*,int)=0;
    virtual int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t*,int,char*,int,bool* used_default)=0;
    virtual std::uint32_t last_error()=0;
};
class Errors {
public:
    virtual ~Errors()=default;
    [[noreturn]] virtual void runtime_00cd70(int code)=0;
    [[noreturn]] virtual void filesystem_00cdb0(std::uint32_t code)=0;
};
std::uint32_t filesystem_codepage_048ac0(Calls&);
Result convert_narrow_048ae8(Calls&,std::uint32_t,const char*,int,char16_t*,int);
Result convert_wide_048b30(Calls&,std::uint32_t,const char16_t*,int,char*,int);
Path filesystem_widen_006d70(std::string_view,Calls&,Errors&);
std::string narrow_006e60(std::u16string_view,std::uint32_t codepage,Calls&,Errors&);
std::string path_utf8_01e3f0(const Path&,Calls&,Errors&);

// Genuine Win32 conversion calls; linked CRT locale belongs to this rebuilt
// process. Original static CRT/thread locale/exception ABI equality is unproven.
class NativeCalls final:public Calls {
public:
    std::uint32_t locale_codepage_0629ec()override;
    bool are_file_apis_ansi()override;
    int multi_byte_to_wide(std::uint32_t,std::uint32_t,const char*,int,char16_t*,int)override;
    int wide_to_multi_byte(std::uint32_t,std::uint32_t,const char16_t*,int,char*,int,bool*)override;
    std::uint32_t last_error()override;
};
// Selected portable std::system_error representation, NOT original MSVC EH ABI.
class PortableErrors final:public Errors {
public:
    [[noreturn]] void runtime_00cd70(int)override;
    [[noreturn]] void filesystem_00cdb0(std::uint32_t)override;
};
} // namespace citrine_eclipse_mirage::encoding
