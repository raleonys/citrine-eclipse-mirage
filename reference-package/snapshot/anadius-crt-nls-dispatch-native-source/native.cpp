#include "native.hpp"
#include <stdexcept>
#include <vector>

namespace anadius::crt_nls_dispatch_native {
namespace {
template<class T> T pointer(Address value) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(value));
}
// Allocation, teardown and boundary exceptions must not overwrite the API's
// LastError. Save before staging and capture immediately after the actual call.
class ErrorScope {
    Api api_;
    DWORD retained_;
public:
    explicit ErrorScope(Api api):api_(api),retained_(api.get_error()){}
    ~ErrorScope(){api_.set_error(retained_);}
    void before_call(){api_.set_error(retained_);}
    void after_call(){retained_=api_.get_error();}
};
template<class Invoke>
std::int32_t map(Api api,std::uint32_t flags,Address destination,
    std::int32_t capacity,std::span<ByteOutput> cells,Invoke invoke) {
    ErrorScope errors(api);
    if (!cells.empty() && !(flags&LCMAP_SORTKEY))
        throw std::invalid_argument("NLS native boundary: tracked cells require LCMAP_SORTKEY");
    if (!cells.empty() && capacity<0)
        throw std::invalid_argument("NLS native boundary: negative tracked capacity");
    if (!cells.empty() && capacity>0 && static_cast<std::size_t>(capacity)>cells.size())
        throw std::invalid_argument("NLS native boundary: tracked capacity exceeds byte cells");
    const bool tracked=!cells.empty() && capacity>0;
    // A null destination remains null, preserving native validation. A tracked
    // query has no host output allocation and passes null (SDK ignores it).
    // Raw pointers, including raw query pointers, pass without substitution.
    const bool staged=tracked && destination!=0;
    std::vector<WCHAR> storage(staged?(static_cast<std::size_t>(capacity)+1)/2:0);
    LPWSTR output=staged?storage.data():(!cells.empty() && capacity==0?nullptr:pointer<LPWSTR>(destination));
    errors.before_call();
    const auto result=invoke(output);
    errors.after_call();
    if (tracked) {
        if (result>0) {
            if (!staged || result>capacity) {
                for (std::int32_t i=0;i<capacity;++i) cells[static_cast<std::size_t>(i)]=ByteOutput{};
                throw std::runtime_error("NLS native boundary: successful output outside supplied storage");
            }
            const auto bytes=reinterpret_cast<const unsigned char*>(storage.data());
            for (std::int32_t i=0;i<result;++i) cells[static_cast<std::size_t>(i)].set(bytes[i]);
        } else {
            // Windows does not promise useful output bytes on failure. Do not
            // turn staging initialization or partial native writes into facts.
            for (std::int32_t i=0;i<capacity;++i) cells[static_cast<std::size_t>(i)]=ByteOutput{};
        }
    }
    return result;
}
}
Api system_api() noexcept {return {&::LCMapStringW,&::GetLastError,&::SetLastError};}
Calls::Calls(Api api):api_(api) {
    if (!api.map_w || !api.get_error || !api.set_error)
        throw std::invalid_argument("NLS native boundary: missing SDK API");
}
std::int32_t Calls::lc_map_string_ex(Address function,Address name,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity,
    Address version,Address reserved,Address sort_handle,std::span<ByteOutput> cells) {
    if (!function) throw std::invalid_argument("NLS native boundary: null LCMapStringEx pointer");
    const auto call=pointer<decltype(&::LCMapStringEx)>(function);
    return map(api_,flags,destination,capacity,cells,[&](LPWSTR output) {
        return call(pointer<LPCWSTR>(name),flags,pointer<LPCWSTR>(source),count,
            output,capacity,pointer<LPNLSVERSIONINFO>(version),pointer<LPVOID>(reserved),
            static_cast<LPARAM>(sort_handle));
    });
}
std::int32_t Calls::lc_map_string_w(std::uint32_t lcid,std::uint32_t flags,
    Address source,std::int32_t count,Address destination,std::int32_t capacity,
    std::span<ByteOutput> cells) {
    return map(api_,flags,destination,capacity,cells,[&](LPWSTR output) {
        return api_.map_w(lcid,flags,pointer<LPCWSTR>(source),count,output,capacity);
    });
}
std::uint32_t Calls::locale_name_to_lcid(Address function,Address name,std::uint32_t flags) {
    if (!function) throw std::invalid_argument("NLS native boundary: null LocaleNameToLCID pointer");
    ErrorScope errors(api_);
    const auto call=pointer<decltype(&::LocaleNameToLCID)>(function);
    errors.before_call();
    const auto result=call(pointer<LPCWSTR>(name),flags);
    errors.after_call();
    return result;
}
}
