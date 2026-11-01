#include "native_api.hpp"
#include <stdexcept>
#include <vector>

namespace anadius::crt_nls_native {
namespace {
template<class T> T pointer(Address address) {
    static_assert(sizeof(std::uintptr_t)==sizeof(Address));
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
struct RestoreError {
    const Api& api;
    DWORD value;
    ~RestoreError() { api.set_last_error(value); }
};
template<class T> void invalidate(std::span<T> cells,std::size_t count) {
    for(std::size_t i=0;i<count;++i) cells[i]=T{};
}
}
const Api& native_api() noexcept {
    static const Api api{&::GetACP,&::GetOEMCP,&::IsValidCodePage,&::GetCPInfo,
        &::MultiByteToWideChar,&::WideCharToMultiByte,&::GetStringTypeW,
        &::GetLastError,&::SetLastError};
    return api;
}
Calls::Calls(const Api& api):api_(api) {
    if(!api_.get_acp||!api_.get_oemcp||!api_.is_valid_code_page||!api_.get_cp_info||
       !api_.multi_byte_to_wide||!api_.wide_char_to_multi_byte||!api_.get_string_type_w||
       !api_.get_last_error||!api_.set_last_error)
        throw std::invalid_argument("NLS native API table has a missing entry");
}
std::uint32_t Calls::get_acp() {return api_.get_acp();}
std::uint32_t Calls::get_oemcp() {return api_.get_oemcp();}
std::uint32_t Calls::is_valid_code_page(std::uint32_t cp) {
    return static_cast<std::uint32_t>(api_.is_valid_code_page(cp));
}
std::uint32_t Calls::get_cp_info(std::uint32_t cp,CPInfo& output) {
    RestoreError error{api_,api_.get_last_error()};
    CPINFO native{};
    api_.set_last_error(error.value);
    const auto result=api_.get_cp_info(cp,&native);
    error.value=api_.get_last_error();
    if(result!=0) {
        output.max_char_size=native.MaxCharSize;
        static_assert(sizeof(native.LeadByte)==12);
        for(std::size_t i=0;i<output.lead_bytes.size();++i)
            output.lead_bytes[i]=native.LeadByte[i];
    }
    return static_cast<std::uint32_t>(result);
}
std::int32_t Calls::multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity) {
    return api_.multi_byte_to_wide(cp,flags,pointer<LPCCH>(source),count,
        pointer<LPWSTR>(destination),capacity);
}
std::int32_t Calls::wide_char_to_multi_byte(std::uint32_t cp,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity,
    Address default_character,Address used_default,std::span<ByteOutput> output) {
    if(output.empty())
        return api_.wide_char_to_multi_byte(cp,flags,pointer<LPCWCH>(source),count,
            pointer<LPSTR>(destination),capacity,pointer<LPCCH>(default_character),
            pointer<LPBOOL>(used_default));
    RestoreError error{api_,api_.get_last_error()};
    if(capacity<0||static_cast<std::size_t>(capacity)>output.size())
        throw std::domain_error("NLS tracked byte capacity exceeds supplied cells");
    // Declared after error: allocation and destruction may change LastError.
    std::vector<CHAR> staged(static_cast<std::size_t>(capacity));
    api_.set_last_error(error.value);
    const auto result=api_.wide_char_to_multi_byte(cp,flags,pointer<LPCWCH>(source),count,
        capacity==0||destination==0?nullptr:staged.data(),capacity,pointer<LPCCH>(default_character),
        pointer<LPBOOL>(used_default));
    error.value=api_.get_last_error();
    if(capacity==0) return result;
    if(result<=0) invalidate(output,static_cast<std::size_t>(capacity));
    else {
        if(result>capacity||destination==0) {
            invalidate(output,static_cast<std::size_t>(capacity));
            throw std::domain_error("NLS API success lacks a valid bounded byte destination");
        }
        for(std::int32_t i=0;i<result;++i)
            output[static_cast<std::size_t>(i)].set(static_cast<std::uint8_t>(staged[static_cast<std::size_t>(i)]));
    }
    return result;
}
std::uint32_t Calls::get_string_type_w(std::uint32_t type,Address source,
    std::int32_t count,Address destination,std::span<WordOutput> output) {
    if(output.empty())
        return static_cast<std::uint32_t>(api_.get_string_type_w(type,pointer<LPCWCH>(source),
            count,pointer<LPWORD>(destination)));
    RestoreError error{api_,api_.get_last_error()};
    if(count<=0||static_cast<std::size_t>(count)>output.size())
        throw std::domain_error("NLS tracked WORD output requires bounded positive count");
    std::vector<WORD> staged(static_cast<std::size_t>(count));
    api_.set_last_error(error.value);
    const auto result=api_.get_string_type_w(type,pointer<LPCWCH>(source),count,
        destination==0?nullptr:staged.data());
    error.value=api_.get_last_error();
    if(result==0) invalidate(output,static_cast<std::size_t>(count));
    else {
        if(destination==0) {
            invalidate(output,static_cast<std::size_t>(count));
            throw std::domain_error("NLS API success lacks a valid WORD destination");
        }
        for(std::int32_t i=0;i<count;++i)
            output[static_cast<std::size_t>(i)].set(staged[static_cast<std::size_t>(i)]);
    }
    return static_cast<std::uint32_t>(result);
}
}
