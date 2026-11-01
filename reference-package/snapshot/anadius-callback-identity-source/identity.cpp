#include "identity.hpp"
#include <stdexcept>
#include <type_traits>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::callback_identity {
namespace {struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};}
CheckedEncoding::CheckedEncoding(std::unique_ptr<encoding_recovered::Calls> lower,std::function<bool()> permission)
    :lower_(std::move(lower)),permission_(std::move(permission)){
    if(!lower_||!permission_)throw std::invalid_argument("Owned encoding provider and explicit PTD permission required");
}
void CheckedEncoding::check(){PreserveError error;if(!permission_())throw std::logic_error("Encoding provider violated selected callback PTD permission");}
template<class F>auto CheckedEncoding::invoke(F&& call)->decltype(call()){
    check();decltype(call()) result;
    try{result=call();}catch(...){check();throw;}
    check();return result;
}
std::uint32_t CheckedEncoding::locale_codepage_0629ec(){return invoke([&]{return lower_->locale_codepage_0629ec();});}
bool CheckedEncoding::are_file_apis_ansi(){return invoke([&]{return lower_->are_file_apis_ansi();});}
int CheckedEncoding::multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* in,int count,char16_t* out,int capacity){
    return invoke([&]{return lower_->multi_byte_to_wide(cp,flags,in,count,out,capacity);});
}
int CheckedEncoding::wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* in,int count,char* out,int capacity,bool* used){
    return invoke([&]{return lower_->wide_to_multi_byte(cp,flags,in,count,out,capacity,used);});
}
std::uint32_t CheckedEncoding::last_error(){return invoke([&]{return lower_->last_error();});}
std::unique_ptr<encoding_recovered::Calls> check_encoding(std::unique_ptr<encoding_recovered::Calls> lower,std::function<bool()> permission){
    return std::make_unique<CheckedEncoding>(std::move(lower),std::move(permission));
}
static_assert(!std::is_abstract_v<CheckedEncoding>);
} // namespace anadius::callback_identity
