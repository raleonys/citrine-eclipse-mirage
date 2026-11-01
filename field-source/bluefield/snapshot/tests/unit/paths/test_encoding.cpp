#include "src/paths/encoding.hpp"
#include <algorithm>
#include <climits>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <vector>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifdef _WIN32
#include <windows.h>
#endif

namespace enc=preloader_l::encoding;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Failure{bool runtime;std::uint32_t code;};
class Errors final:public enc::Errors {
public:
    [[noreturn]] void runtime_00cd70(int code)override{throw Failure{true,static_cast<std::uint32_t>(code)};}
    [[noreturn]] void filesystem_00cdb0(std::uint32_t code)override{throw Failure{false,code};}
};
struct Step {int count;std::uint32_t error=0;bool used_default=false;std::string narrow{};enc::Path wide{};};
struct Call {char direction;std::uint32_t cp,flags;int size,capacity;bool destination,used_default;};
class Calls final:public enc::Calls {
public:
    std::uint32_t cp=0,error=0;bool ansi=true;
    unsigned locale_calls=0,ansi_calls=0,error_calls=0;
    std::deque<Step> steps;
    std::vector<Call> calls;
    std::uint32_t locale_codepage_0629ec()override{++locale_calls;return cp;}
    bool are_file_apis_ansi()override{++ansi_calls;return ansi;}
    Step next(){check(!steps.empty(),"unexpected conversion call");auto result=steps.front();steps.pop_front();error=result.error;return result;}
    int multi_byte_to_wide(std::uint32_t page,std::uint32_t flags,const char*,int size,char16_t* output,int capacity)override {
        calls.push_back({'n',page,flags,size,capacity,output!=nullptr,false});const auto result=next();
        if(output&&capacity>0)std::copy_n(result.wide.begin(),std::min(result.wide.size(),static_cast<std::size_t>(capacity)),output);
        return result.count;
    }
    int wide_to_multi_byte(std::uint32_t page,std::uint32_t flags,const char16_t*,int size,char* output,int capacity,bool* used)override {
        calls.push_back({'w',page,flags,size,capacity,output!=nullptr,used!=nullptr});const auto result=next();
        if(used)*used=result.used_default;
        if(output&&capacity>0)std::copy_n(result.narrow.begin(),std::min(result.narrow.size(),static_cast<std::size_t>(capacity)),output);
        return result.count;
    }
    std::uint32_t last_error()override{++error_calls;return error;}
};
void synthetic() {
    Errors errors;
    for(const auto cp:{0U,1252U,65001U})for(const bool ansi:{false,true}) {
        Calls calls;calls.cp=cp;calls.ansi=ansi;
        check(enc::filesystem_codepage_048ac0(calls)==(cp==65001?65001U:(ansi?0U:1U)),"file codepage precedence");
        check(calls.locale_calls==1&&calls.ansi_calls==(cp==65001?0U:1U),"codepage call order");
    }
    Calls empty;
    check(enc::filesystem_widen_006d70({},empty,errors).empty()&&empty.locale_calls==1&&empty.ansi_calls==1,"empty filesystem path still selects codepage");
    check(enc::narrow_006e60({},65001,empty,errors).empty()&&empty.calls.empty(),"empty narrowing skips API");
    Calls narrow;narrow.cp=65001;narrow.steps={{3,0,false,{},{}},{1,0,false,{},u"X"}};
    const auto wide=enc::filesystem_widen_006d70(std::string("a\0b",3),narrow,errors);
    check(wide==enc::Path(u"X\0\0",3),"two-pass wide size retained");
    check(narrow.calls.size()==2&&narrow.calls[0].flags==8&&narrow.calls[0].size==3&&!narrow.calls[0].destination&&narrow.calls[1].capacity==3,"narrow explicit length/flags/query then fill");
    for(const auto page:{65001U,54936U,1252U}) {
        Calls calls;calls.steps={{1}};char output='?';
        check(enc::convert_wide_048b30(calls,page,u"x",1,&output,1).error==0,"wide successful conversion");
        check(calls.calls[0].flags==(page==1252?0x400U:0x80U)&&calls.calls[0].used_default==(page==1252),"wide flags/default pointer");
        check(calls.error_calls==0,"success does not read LastError");
    }
    Calls replacement;replacement.steps={{2,123,true}};
    const auto result=enc::convert_wide_048b30(replacement,1252,u"x",1,nullptr,0);
    check(result.count==0&&result.error==0x459&&replacement.error_calls==0,"default replacement is explicit translation error");
    Calls fallback;fallback.steps={{0,0x3ec},{1}};
    check(enc::convert_wide_048b30(fallback,65001,u"x",1,nullptr,0).count==1,"wide invalid flags fallback");
    check(fallback.calls.size()==2&&fallback.calls[1].flags==0&&!fallback.calls[1].used_default,"fallback flags0/null default");
    Calls failure;failure.steps={{0,87}};
    check(enc::convert_wide_048b30(failure,65001,u"x",1,nullptr,0).error==87&&failure.calls.size()==1,"other error not retried");
    Calls utf;utf.steps={{4},{1,0,false,"Z"}};
    check(enc::path_utf8_01e3f0(enc::Path(u"a\0b",3),utf,errors)==std::string("Z\0\0\0",4),"UTF8 two-pass size and NUL retained");
    check(utf.locale_calls==0&&utf.calls[0].cp==65001,"path UTF8 independent of filesystem codepage");
    for(const bool second:{false,true}) {
        Calls calls;calls.steps=second?std::deque<Step>{{2},{0,1113}}:std::deque<Step>{{0,1113}};
        bool caught=false;try{(void)enc::filesystem_widen_006d70("x",calls,errors);}catch(const Failure& error){caught=!error.runtime&&error.code==1113;}
        check(caught,"conversion query/fill error propagates");
    }
    Calls too_large;
    bool caught=false;try{(void)enc::filesystem_widen_006d70(std::string_view("x",static_cast<std::size_t>(INT_MAX)+1),too_large,errors);}catch(const Failure& error){caught=error.runtime&&error.code==22;}
    check(caught&&too_large.calls.empty()&&too_large.locale_calls==1,"oversized input guard before buffer access");
}
#ifdef _WIN32
void native() {
    enc::NativeCalls calls;enc::PortableErrors errors;
    const enc::Path unicode=u"C:\\fixture_\u00e7\u0131\u015f_\u03a9_\U0001f600.bin";
    const std::string expected="C:\\fixture_\xc3\xa7\xc4\xb1\xc5\x9f_\xce\xa9_\xf0\x9f\x98\x80.bin";
    check(enc::path_utf8_01e3f0(unicode,calls,errors)==expected,"native UTF16 to UTF8 known bytes");
    std::vector<char16_t> decoded(unicode.size());
    const auto result=enc::convert_narrow_048ae8(calls,65001,expected.data(),static_cast<int>(expected.size()),decoded.data(),static_cast<int>(decoded.size()));
    check(result.error==0&&enc::Path(decoded.begin(),decoded.end())==unicode,"native strict UTF8 decode");
    check(enc::path_utf8_01e3f0(enc::Path(u"a\0b",3),calls,errors)==std::string("a\0b",3),"native explicit NUL length");
    check(enc::filesystem_widen_006d70(".dlf",calls,errors)==u".dlf","native filesystem ASCII suffix");
    const char invalid[]={static_cast<char>(0xc3),'('};
    const auto invalid_result=enc::convert_narrow_048ae8(calls,65001,invalid,2,nullptr,0);
    check(invalid_result.count==0&&invalid_result.error==1113,"native invalid UTF8 error preserved across adapter temporaries");
    bool caught=false;try{(void)enc::path_utf8_01e3f0(enc::Path(1,static_cast<char16_t>(0xd800)),calls,errors);}catch(const std::system_error& error){caught=error.code().value()==1113;}
    check(caught,"native lone-surrogate strict conversion failure");
    check(enc::convert_wide_048b30(calls,1252,u"\u03a9",1,nullptr,0).error==1113,"native no-best-fit substitution rejected");
    wchar_t baseline=0x7777;char16_t adapted=0x7777;
    SetLastError(31337);
    const int direct=MultiByteToWideChar(65001,8,"abc",3,&baseline,1);
    const auto direct_error=GetLastError();SetLastError(31337);
    const int bridged=calls.multi_byte_to_wide(65001,8,"abc",3,&adapted,1);
    check(direct==bridged&&GetLastError()==direct_error&&adapted==static_cast<char16_t>(baseline),"native buffer/error matches direct Win32 on insufficient capacity");
    SetLastError(31337);
    const int query=MultiByteToWideChar(65001,8,"abc",3,nullptr,0);
    const auto query_error=GetLastError();SetLastError(31337);
    check(calls.multi_byte_to_wide(65001,8,"abc",3,nullptr,0)==query&&GetLastError()==query_error,"temporary allocation preserves API entry/exit error state");
}
#endif
}
int main(){try{synthetic();
#ifdef _WIN32
native();
#endif
std::cout<<"PASS: "<<checks<<" encoding checks; real Win32 only on synthetic strings, original targets not executed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
