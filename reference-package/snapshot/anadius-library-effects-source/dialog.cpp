#include "dialog.hpp"
#include <climits>
#include <stdexcept>

namespace anadius::library_effects {
DialogApi native_dialog_api() noexcept{return {::MultiByteToWideChar,::MessageBoxW};}
Dialog::Dialog(DialogApi api):api_(api){
    if(!api.convert||!api.message)throw std::invalid_argument("Complete typed dialog SDK API required");
}
std::wstring Dialog::widen(std::string_view text) const{
    if(text.empty())return {};
    // Explicit rebuilt domain boundary, not a recovered0452d0 length guard.
    if(text.size()>INT_MAX)throw std::length_error("Dialog input exceeds selected explicit-int domain");
    const auto size=static_cast<int>(text.size());
    const auto count=api_.convert(65001,0,text.data(),size,nullptr,0);
    if(count<0)throw std::domain_error("Negative conversion count violates Win32 API contract");
    std::wstring result(static_cast<std::size_t>(count),L'\0');
    //0452d0 ignores query/fill errors and retains the first queried length.
    // Even a zero query is followed by a second call with capacity zero.
    (void)api_.convert(65001,0,text.data(),size,result.data(),count);
    return result;
}
void Dialog::show(std::string_view title,std::string_view text,std::uint32_t flags) const{
    const auto caption=widen(title); //038bfc before038c09
    const auto body=widen(text);
    // Return mapping of038bc0 is outside the library Effects void contract.
    (void)api_.message(nullptr,body.c_str(),caption.c_str(),flags);
}
} // namespace anadius::library_effects
