#include "provider.hpp"
#include <array>
#include <cerrno>
#include <climits>
#include <iostream>
#include <system_error>
#include <vector>

namespace {
using namespace anadius;namespace p=callback_provider;namespace e=encoding_recovered;namespace c=callback_policy;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
template<class E,class F>void rejects(F&& fn,const char* message){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,message);}
struct Fault {};
struct Trace {unsigned count=0;std::uint32_t cp=0,flags=0;bool used=false,short_fill=false,replace=false,retry=false;DWORD error=0;};
class Lower final:public e::Calls {
public:
    std::shared_ptr<Trace> trace;explicit Lower(std::shared_ptr<Trace> t):trace(std::move(t)){}
    std::uint32_t locale_codepage_0629ec()override{throw std::logic_error("Ambient locale queried");}
    bool are_file_apis_ansi()override{throw std::logic_error("Ambient file mode queried");}
    std::uint32_t last_error()override{return trace->error;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* in,int n,char16_t* out,int cap)override{
        ++trace->count;trace->cp=cp;trace->flags=flags;
        if(out){if(cap<n)throw Fault{};for(int i=0;i<n;++i)out[i]=static_cast<unsigned char>(in[i]);}
        ::SetLastError(0x2233);return n;
    }
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* in,int n,char* out,int cap,bool* used)override{
        ++trace->count;trace->cp=cp;trace->flags=flags;trace->used=used!=nullptr;
        if(used)*used=trace->replace;
        if(trace->retry&&flags){trace->error=0x3ec;return 0;}
        if(out){if(cap<n)throw Fault{};for(int i=0;i<(trace->short_fill?1:n);++i)out[i]=static_cast<char>(in[i]);}
        ::SetLastError(0x3344);return out&&trace->short_fill?1:n;
    }
};
void locale_and_copy(){
    auto token=std::make_shared<int>(7);std::weak_ptr<int> weak=token;std::vector<p::InvalidCall> events;
    auto crt=p::make_crt("German_Germany.1252",[token,&events](auto event){events.push_back(event);::SetLastError(0x4567);});token.reset();
    require(!weak.expired(),"handler context retained");
    require(crt->compare_a("AbC","aBc")==0,"owned narrow comparison");
    require(crt->compare_w(L"\u00c4",L"\u00e4")==0,"explicit locale non ASCII fold");
    auto classic=p::make_crt("C",[](auto){throw Fault{};});
    c::ByteWide owned_only({},classic);
    require(owned_only.compare_utf16(u"AbC",u"aBc")==0,"owned bridge needs no unused fallback table");
    require(classic->compare_w(L"\u00c4",L"\u00e4")!=0,"independent C locale does not inherit German fold");
    require(crt->compare_a("\xc4","\xe4")==0,"owned narrow codepage non ASCII fold");
    char out[8];std::fill(std::begin(out),std::end(out),'!');errno=0;
    require(crt->copy_a(out,8,"Hi")==0&&std::string(out)=="Hi"&&out[3]=='!',"success copy only string and NUL");
    require(crt->copy_a(out,3,"Hi")==0,"exact narrow capacity");
    const char embedded[]{'A',0,'B',0};require(crt->copy_a(out,8,embedded)==0&&std::string(out)=="A","copy NUL termination");
    out[0]='Q';require(crt->copy_a(out,2,"long")==ERANGE&&out[0]==0&&out[1]==0&&errno==ERANGE,"range clears first only and errno");
    require(events.back().operation==p::Operation::CopyA&&events.back().reason==p::InvalidReason::Range&&events.back().error==ERANGE,"explicit invalid metadata");
    require(::GetLastError()==0x4567,"owned handler outgoing LastError");
    out[0]='Q';require(crt->copy_a(out,8,nullptr)==EINVAL&&out[0]==0&&errno==EINVAL,"null source prefix");
    require(crt->copy_a(nullptr,8,"x")==EINVAL,"null destination explicit handler");
    out[0]='Q';require(crt->copy_a(out,0,"x")==EINVAL&&out[0]=='Q',"zero capacity does not write");
    require(crt->compare_a(nullptr,"x")==INT_MAX&&errno==EINVAL,"null compare stays out of CRT invalid handler");
    require(crt->compare_w(L"x",nullptr)==INT_MAX,"wide null compare handled explicitly");
    wchar_t wide[5]{L'Q',L'!',L'!',L'!',L'!'};
    require(crt->copy_w(wide,5,L"Ab")==0&&std::wstring(wide)==L"Ab"&&wide[3]==L'!',"wide owned success tail");
    require(crt->copy_w(wide,1,L"Ab")==ERANGE&&wide[0]==0&&wide[1]==L'b',"wide range prefix only");
    rejects<std::domain_error>([&]{crt->copy_a(out,8,out);},"overlap rejected before copy");
    rejects<std::domain_error>([&]{crt->copy_a(out,std::size_t(INT_MAX)+1,"x");},"capacity excluded before source traversal");
    rejects<Fault>([&]{classic->copy_a(out,8,nullptr);},"owned invalid handler throw identity");
    require(out[0]==0,"throw keeps preclear");
    auto alias=crt;crt.reset();require(!weak.expired()&&alias->compare_a("a","A")==0,"copied owner retains locale and context");
    alias.reset();require(weak.expired(),"last owner releases invalid context");
    rejects<std::invalid_argument>([]{p::make_crt("",[](auto){});},"empty ambient locale rejected");
    rejects<std::invalid_argument>([]{p::make_crt(std::string("C\0ignored",9),[](auto){});},"embedded NUL locale rejected");
    rejects<std::invalid_argument>([]{p::make_crt("C",{});},"missing owned handler rejected");
    rejects<std::invalid_argument>([]{p::make_crt("not_a_real_locale_1234",[](auto){});},"unknown locale rejected");
}
class Probe final:public c::CrtProvider {
public:
    unsigned count=0;bool allowed=true;std::exception_ptr failure;
    int call(){++count;::SetLastError(0x8899);if(failure)std::rethrow_exception(failure);return 7;}
    int compare_a(const char*,const char*)override{return call();}
    int compare_w(const wchar_t*,const wchar_t*)override{return call();}
    int copy_a(char*,std::size_t,const char*)override{return call();}
    int copy_w(wchar_t*,std::size_t,const wchar_t*)override{return call();}
};
void checkpoints(){
    auto lower=std::make_shared<Probe>();unsigned probes=0;bool allowed=true;
    auto checked=p::check_crt(c::compiler_crt_api(),lower,[&]{++probes;::SetLastError(0xdead);return allowed;});
    ::SetLastError(0x1234);
    require(checked->compare_a("a","b")==7&&probes==2&&::GetLastError()==0x8899,"narrow compare pre/post error bookkeeping");
    require(checked->compare_w(L"a",L"b")==7&&probes==4,"wide comparison checkpoint");
    require(checked->copy_a(nullptr,0,nullptr)==7&&probes==6,"narrow copy exact invocation");
    require(checked->copy_w(nullptr,0,nullptr)==7&&probes==8,"wide copy exact invocation");
    allowed=false;rejects<std::logic_error>([&]{checked->compare_a("a","a");},"precheck rejects");
    require(lower->count==4,"rejected precheck never calls lower");allowed=true;
    lower->failure=std::make_exception_ptr(Fault{});bool same=false;
    try{checked->compare_w(L"a",L"a");}catch(...){same=std::current_exception()==lower->failure;}
    require(same&&::GetLastError()==0x8899,"valid postcheck retains exact exception and outgoing error");
    unsigned n=0;checked=p::check_crt(c::compiler_crt_api(),lower,[&]{return ++n==1;});
    rejects<std::logic_error>([&]{checked->copy_a(nullptr,0,nullptr);},"invalid postcondition wins over lower exception");
    auto raw=p::check_crt(c::compiler_crt_api(),{},[]{return true;});char out[4]{};
    require(raw->compare_a("a","A")==0&&raw->copy_a(out,4,"Hi")==0&&std::string(out)=="Hi","legacy table checkpoint adaptation");
    rejects<std::invalid_argument>([&]{p::check_crt(c::compiler_crt_api(),lower,{});},"missing permission rejected");
}
void encoding(){
    auto trace=std::make_shared<Trace>();e::PortableErrors errors;
    auto calls=p::make_encoding({p::Filesystem::Ansi,1252,437},std::make_unique<Lower>(trace));
    require(e::filesystem_codepage_048ac0(*calls)==0,"explicit ANSI selector keeps recovered alias");
    const auto text=e::narrow_006e60(std::u16string_view(u"A\0B",3),0,*calls,errors);
    require(text==std::string("A\0B",3)&&trace->count==2&&trace->cp==1252,"query fill fixed numeric ANSI and embedded NUL");
    require(trace->flags==0x400&&trace->used&&::GetLastError()==0x3344,"retained ANSI flags used pointer and lower error");
    trace->short_fill=true;require(e::narrow_006e60(u"ABC",0,*calls,errors)==std::string("A\0\0",3),"query length and short fill tail retained");trace->short_fill=false;
    require(e::filesystem_widen_006d70(std::string_view("A\0B",3),*calls,errors)==std::u16string(u"A\0B",3)&&trace->cp==1252&&trace->flags==8,"widen fixed numeric page flags and counted NUL");
    calls=p::make_encoding({p::Filesystem::Oem,1252,437},std::make_unique<Lower>(trace));
    require(e::filesystem_codepage_048ac0(*calls)==1,"frozen OEM selector");
    (void)e::narrow_006e60(u"own",1,*calls,errors);require(trace->cp==437&&trace->flags==0x400&&trace->used,"explicit numeric OEM resolution");
    calls=p::make_encoding({p::Filesystem::Utf8,1252,437},std::make_unique<Lower>(trace));
    require(e::filesystem_codepage_048ac0(*calls)==65001,"explicit UTF8 selector");
    (void)e::narrow_006e60(u"own",65001,*calls,errors);require(trace->cp==65001&&trace->flags==0x80&&!trace->used,"UTF8 retained flag route");
    (void)e::narrow_006e60(u"own",54936,*calls,errors);require(trace->cp==54936&&trace->flags==0x80&&!trace->used,"explicit GB18030 retained route");
    calls=p::make_encoding({p::Filesystem::Ansi,65001,437},std::make_unique<Lower>(trace));
    (void)e::narrow_006e60(u"own",0,*calls,errors);require(trace->cp==65001&&trace->flags==0x400&&trace->used,"UTF8 alias deliberately preserves pre-resolution flags");
    trace->replace=true;rejects<std::system_error>([&]{(void)e::narrow_006e60(u"own",0,*calls,errors);},"replacement is conversion error");trace->replace=false;
    trace->retry=true;(void)e::narrow_006e60(u"own",0,*calls,errors);require(trace->flags==0&&!trace->used&&trace->cp==65001,"retained invalidflags retry uses same numeric page");trace->retry=false;
    rejects<std::invalid_argument>([&]{calls->multi_byte_to_wide(3,8,"x",1,nullptr,0);},"thread ambient alias blocked");
    rejects<std::invalid_argument>([]{p::make_encoding({p::Filesystem::Ansi,0,437});},"numerical ANSI required");
    rejects<std::invalid_argument>([]{p::make_encoding({p::Filesystem::Oem,1252,3});},"numerical OEM required");
    rejects<std::invalid_argument>([]{p::make_encoding({p::Filesystem::Utf8,999999,437});},"unavailable numerical page rejected");
    auto native=p::make_encoding({p::Filesystem::Utf8,1252,437});
    require(e::narrow_006e60(u"\u00e9",65001,*native,errors)==std::string("\xc3\xa9",2),"actual Win32 numeric UTF8 conversion");
    native=p::make_encoding({p::Filesystem::Ansi,1252,437});
    require(e::narrow_006e60(u"\u00e9",0,*native,errors)==std::string("\xe9",1),"actual Win32 numeric ANSI conversion");
    c::Resources resources;auto* previous=resources.encoding.get();
    rejects<std::invalid_argument>([&]{p::select(resources,"C",[](auto){},{p::Filesystem::Utf8,0,437});},"selection construction failure");
    require(!resources.owned_crt&&resources.encoding.get()==previous,"selection strong construction guarantee");
}
}
int main(){try{locale_and_copy();checkpoints();encoding();std::cout<<"PASS: "<<checks<<" owned callback provider core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"unexpected own fault\n";return 2;}}
