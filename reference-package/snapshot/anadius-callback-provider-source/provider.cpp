#include "provider.hpp"
#include <cerrno>
#include <climits>
#include <cstring>
#include <cwchar>
#include <locale.h>
#include <stdexcept>

namespace anadius::callback_provider {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
bool overlap(const void* a,std::size_t an,const void* b,std::size_t bn){
    const auto x=reinterpret_cast<std::uintptr_t>(a),y=reinterpret_cast<std::uintptr_t>(b);
    return x<=y?y-x<an:x-y<bn;
}
class OwnedCrt final:public callback_policy::CrtProvider {
    _locale_t locale_=nullptr;InvalidHandler invalid_;
    int invalid(Operation op,InvalidReason why,int error){invalid_({op,why,error});errno=error;return error;}
    template<class C>int copy(C* out,std::size_t capacity,const C* in,Operation op){
        if(capacity>INT_MAX)throw std::domain_error("Owned copy capacity exceeds INT_MAX");
        if(!out||!capacity)return invalid(op,InvalidReason::Destination,EINVAL);
        if(!in){out[0]=0;return invalid(op,InvalidReason::NullInput,EINVAL);}
        const auto length=std::char_traits<C>::length(in);
        // Like ByteWide, overlaps are an excluded domain, not invalid-handler input.
        if(overlap(out,capacity*sizeof(C),in,(length+1)*sizeof(C)))throw std::domain_error("Overlapping owned copy");
        if(length>=capacity){out[0]=0;return invalid(op,InvalidReason::Range,ERANGE);}
        std::memcpy(out,in,(length+1)*sizeof(C));return 0;
    }
public:
    OwnedCrt(const std::string& locale,InvalidHandler handler):invalid_(std::move(handler)){
        Error error;
        if(locale.empty()||locale.find('\0')!=std::string::npos||!invalid_)throw std::invalid_argument("Explicit locale and owned invalid handler required");
        locale_=::_create_locale(LC_ALL,locale.c_str());
        if(!locale_)throw std::invalid_argument("Compiler locale is unavailable");
    }
    ~OwnedCrt()override{Error error;::_free_locale(locale_);}
    int compare_a(const char* a,const char* b)override{
        if(!a||!b){invalid(Operation::CompareA,InvalidReason::NullInput,EINVAL);return INT_MAX;}
        return ::_stricmp_l(a,b,locale_);
    }
    int compare_w(const wchar_t* a,const wchar_t* b)override{
        if(!a||!b){invalid(Operation::CompareW,InvalidReason::NullInput,EINVAL);return INT_MAX;}
        return ::_wcsicmp_l(a,b,locale_);
    }
    int copy_a(char* a,std::size_t n,const char* b)override{return copy(a,n,b,Operation::CopyA);}
    int copy_w(wchar_t* a,std::size_t n,const wchar_t* b)override{return copy(a,n,b,Operation::CopyW);}
};
class CheckedCrt final:public callback_policy::CrtProvider {
    callback_policy::CrtApi fallback_;std::shared_ptr<callback_policy::CrtProvider> lower_;std::function<bool()> permission_;
    void check(){Error error;if(!permission_())throw std::logic_error("CRT provider violated selected callback PTD permission");}
    template<class F>int invoke(F&& fn){check();int result;try{result=fn();}catch(...){check();throw;}check();return result;}
public:
    CheckedCrt(callback_policy::CrtApi api,std::shared_ptr<callback_policy::CrtProvider> lower,std::function<bool()> permission)
        :fallback_(api),lower_(std::move(lower)),permission_(std::move(permission)){
        if(!permission_||(!lower_&&(!api.compare_a||!api.copy_a||!api.compare_w||!api.copy_w)))throw std::invalid_argument("Complete CRT provider and permission required");
    }
    int compare_a(const char* a,const char* b)override{return invoke([&]{return lower_?lower_->compare_a(a,b):fallback_.compare_a(a,b);});}
    int copy_a(char* a,std::size_t n,const char* b)override{return invoke([&]{return lower_?lower_->copy_a(a,n,b):fallback_.copy_a(a,n,b);});}
    int compare_w(const wchar_t* a,const wchar_t* b)override{return invoke([&]{return lower_?lower_->compare_w(a,b):fallback_.compare_w(a,b);});}
    int copy_w(wchar_t* a,std::size_t n,const wchar_t* b)override{return invoke([&]{return lower_?lower_->copy_w(a,n,b):fallback_.copy_w(a,n,b);});}
};
class NumericEncoding final:public encoding_recovered::Calls {
    const EncodingSelection selection_;std::unique_ptr<encoding_recovered::Calls> lower_;
    std::uint32_t resolve(std::uint32_t cp)const{
        if(cp==0)return selection_.ansi_codepage;
        if(cp==1)return selection_.oem_codepage;
        if(cp==2||cp==3)throw std::invalid_argument("Ambient MAC/thread codepage aliases are unsupported");
        return cp;
    }
public:
    NumericEncoding(EncodingSelection value,std::unique_ptr<encoding_recovered::Calls> lower):selection_(value),lower_(std::move(lower)){
        Error error;
        if(!lower_||static_cast<unsigned>(value.filesystem)>static_cast<unsigned>(Filesystem::Oem)||value.ansi_codepage<4||value.oem_codepage<4||!::IsValidCodePage(value.ansi_codepage)||!::IsValidCodePage(value.oem_codepage))throw std::invalid_argument("Explicit available numerical ANSI/OEM codepages required");
    }
    std::uint32_t locale_codepage_0629ec()override{return selection_.filesystem==Filesystem::Utf8?65001U:0U;}
    bool are_file_apis_ansi()override{return selection_.filesystem==Filesystem::Ansi;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* in,int count,char16_t* out,int capacity)override{return lower_->multi_byte_to_wide(resolve(cp),flags,in,count,out,capacity);}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* in,int count,char* out,int capacity,bool* used)override{return lower_->wide_to_multi_byte(resolve(cp),flags,in,count,out,capacity,used);}
    std::uint32_t last_error()override{return lower_->last_error();}
};
}
std::shared_ptr<callback_policy::CrtProvider> make_crt(std::string locale,InvalidHandler handler){return std::make_shared<OwnedCrt>(locale,std::move(handler));}
std::shared_ptr<callback_policy::CrtProvider> check_crt(callback_policy::CrtApi api,std::shared_ptr<callback_policy::CrtProvider> lower,std::function<bool()> permission){return std::make_shared<CheckedCrt>(api,std::move(lower),std::move(permission));}
std::unique_ptr<encoding_recovered::Calls> make_encoding(EncodingSelection selection,std::unique_ptr<encoding_recovered::Calls> lower){return std::make_unique<NumericEncoding>(selection,std::move(lower));}
void select(callback_policy::Resources& resources,std::string locale,InvalidHandler handler,EncodingSelection selection){
    auto crt=make_crt(std::move(locale),std::move(handler));auto encoding=make_encoding(selection);
    resources.owned_crt=std::move(crt);resources.encoding=std::move(encoding);
}
} // namespace anadius::callback_provider
