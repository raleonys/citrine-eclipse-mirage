// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/codec/encoding.hpp"
#include <climits>
#include <exception>
#include <system_error>

namespace laureondo::encoding {
std::uint32_t filesystem_codepage_048ac0(Calls& calls) {
    if(calls.locale_codepage_0629ec()==65001)return 65001;
    return calls.are_file_apis_ansi()?0U:1U;
}
Result convert_narrow_048ae8(Calls& calls,std::uint32_t cp,const char* input,int size,char16_t* output,int capacity) {
    const int count=calls.multi_byte_to_wide(cp,8,input,size,output,capacity);
    return {count,count==0?calls.last_error():0U};
}
Result convert_wide_048b30(Calls& calls,std::uint32_t cp,const char16_t* input,int size,char* output,int capacity) {
    int count;
    if(cp==65001||cp==54936)count=calls.wide_to_multi_byte(cp,0x80,input,size,output,capacity,nullptr);
    else {
        bool used_default=false;
        count=calls.wide_to_multi_byte(cp,0x400,input,size,output,capacity,&used_default);
        if(used_default)return {0,0x459};
    }
    auto error=count==0?calls.last_error():0U;
    if(error==0x3ec) {
        count=calls.wide_to_multi_byte(cp,0,input,size,output,capacity,nullptr);
        error=count==0?calls.last_error():0U;
    }
    return {count,error};
}
namespace {
void verify(Result result,Errors& errors) {
    if(result.error){errors.filesystem_00cdb0(result.error);std::terminate();}
}
void length(std::size_t size,Errors& errors) {
    if(size>INT_MAX){errors.runtime_00cd70(22);std::terminate();}
}
}
Path filesystem_widen_006d70(std::string_view value,Calls& calls,Errors& errors) {
    // Original selects the file code page even for an empty input view.
    const auto cp=filesystem_codepage_048ac0(calls);
    if(value.empty())return {};
    length(value.size(),errors);
    const auto required=convert_narrow_048ae8(calls,cp,value.data(),static_cast<int>(value.size()),nullptr,0);
    verify(required,errors);
    Path output(static_cast<std::size_t>(required.count),u'\0');
    verify(convert_narrow_048ae8(calls,cp,value.data(),static_cast<int>(value.size()),output.data(),required.count),errors);
    return output; // length remains first-pass count even if second count differs
}
std::string narrow_006e60(std::u16string_view value,std::uint32_t cp,Calls& calls,Errors& errors) {
    if(value.empty())return {};
    length(value.size(),errors);
    const auto required=convert_wide_048b30(calls,cp,value.data(),static_cast<int>(value.size()),nullptr,0);
    verify(required,errors);
    std::string output(static_cast<std::size_t>(required.count),'\0');
    verify(convert_wide_048b30(calls,cp,value.data(),static_cast<int>(value.size()),output.data(),required.count),errors);
    return output;
}
std::string path_utf8_01e3f0(const Path& path,Calls& calls,Errors& errors) {
    return narrow_006e60(path,65001,calls,errors);
}
[[noreturn]] void PortableErrors::runtime_00cd70(int code){throw std::system_error(code,std::generic_category());}
[[noreturn]] void PortableErrors::filesystem_00cdb0(std::uint32_t code){throw std::system_error(static_cast<int>(code),std::system_category());}
} // namespace laureondo::encoding
