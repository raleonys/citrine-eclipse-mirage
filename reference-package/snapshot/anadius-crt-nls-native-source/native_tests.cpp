#include "native_api.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <type_traits>
namespace n=anadius::crt_nls_native;
namespace {
std::uint64_t checks{};
void check(bool value) {++checks;if(!value) throw std::runtime_error("native fixture check failed");}
template<class T> n::Address address(T* p) {return reinterpret_cast<std::uintptr_t>(p);}
struct State {
    DWORD error=0x1111,incoming{};
    UINT cp{};DWORD flags{};LPCWCH wide{};LPCCH narrow{};int count{},capacity{};
    LPSTR byte_destination{};LPWSTR wide_destination{};LPWORD word_destination{};
    LPCCH default_character{};LPBOOL used_default{};
    int mode{};unsigned wide_calls{},type_calls{};
} state;
DWORD WINAPI last_error() {return state.error;}
void WINAPI set_error(DWORD value) {state.error=value;}
UINT WINAPI acp() {return 1252;}
UINT WINAPI oemcp() {return 437;}
BOOL WINAPI valid_cp(UINT cp) {state.cp=cp;return static_cast<BOOL>(0x12345678);}
BOOL WINAPI cp_info(UINT cp,LPCPINFO out) {
    state.incoming=state.error;state.error=0x2222;state.cp=cp;
    out->MaxCharSize=7;
    for(unsigned i=0;i<12;++i)out->LeadByte[i]=static_cast<BYTE>(0x80+i);
    return state.mode==1?0:-2;
}
int WINAPI mb_to_wide(UINT cp,DWORD flags,LPCCH source,int count,LPWSTR out,int capacity) {
    state.cp=cp;state.flags=flags;state.narrow=source;state.count=count;
    state.wide_destination=out;state.capacity=capacity;state.error=0x3333;
    if(capacity>0&&out)out[0]=L'Q';
    return 1;
}
int WINAPI wide_to_mb(UINT cp,DWORD flags,LPCWCH source,int count,LPSTR out,int capacity,
    LPCCH def,LPBOOL used) {
    ++state.wide_calls;state.cp=cp;state.flags=flags;state.wide=source;state.count=count;
    state.byte_destination=out;state.capacity=capacity;state.default_character=def;
    state.used_default=used;state.incoming=state.error;state.error=0x4444;
    if(used)*used=static_cast<BOOL>(0x76543210);
    if(capacity==0)return 7;
    if(capacity<0)return 0;
    if(out)for(int i=0;i<capacity;++i)out[i]=static_cast<char>(0xa0+i);
    if(state.mode==1)return 0;
    if(state.mode==2)return capacity+1;
    return capacity<2?capacity:2;
}
BOOL WINAPI get_type(DWORD type,LPCWCH source,int count,LPWORD out) {
    ++state.type_calls;state.flags=type;state.wide=source;state.count=count;
    state.word_destination=out;state.incoming=state.error;state.error=0x5555;
    const int written=count<0?3:count;
    if(out)for(int i=0;i<written;++i)out[i]=static_cast<WORD>(0x1200+i);
    return state.mode==1?0:static_cast<BOOL>(0x12345678);
}
const n::Api fixture{&acp,&oemcp,&valid_cp,&cp_info,&mb_to_wide,&wide_to_mb,
    &get_type,&last_error,&set_error};
template<class F> void domain(F fn) {
    bool caught=false;try{fn();}catch(const std::domain_error&){caught=true;}check(caught);
}
void test_fixture() {
    static_assert(std::is_same_v<decltype(n::Api::get_string_type_w),decltype(&::GetStringTypeW)>);
    static_assert(std::is_same_v<decltype(n::Api::wide_char_to_multi_byte),decltype(&::WideCharToMultiByte)>);
    n::Calls call(fixture);
    check(call.get_acp()==1252);check(call.get_oemcp()==437);
    check(call.is_valid_code_page(932)==0x12345678&&state.cp==932);
    n::CPInfo cp{99,{}};
    state.error=0x1111;state.mode=0;
    check(call.get_cp_info(932,cp)==0xfffffffeu);check(cp.max_char_size==7);
    for(unsigned i=0;i<12;++i)check(cp.lead_bytes[i]==0x80+i);
    check(state.incoming==0x1111&&state.error==0x2222&&state.cp==932);
    cp.max_char_size=99;cp.lead_bytes.fill(0x61);state.mode=1;
    check(call.get_cp_info(1251,cp)==0&&cp.max_char_size==99);
    for(auto b:cp.lead_bytes)check(b==0x61);
    constexpr char source[]="abc";wchar_t wide[4]{};
    check(call.multi_byte_to_wide(65001,7,address(source),-1,address(wide),4)==1);
    check(state.cp==65001&&state.flags==7&&state.narrow==source&&state.count==-1);
    check(state.wide_destination==wide&&state.capacity==4&&wide[0]==L'Q');
    check(state.error==0x3333);
    constexpr wchar_t input[]=L"AB";char default_character='?';BOOL used{};
    std::array<n::ByteOutput,5> bytes;for(auto& x:bytes)x.set(0x77);
    state.mode=0;state.error=0x1111;
    check(call.wide_char_to_multi_byte(932,9,address(input),2,0x11,4,
        address(&default_character),address(&used),bytes)==2);
    check(state.byte_destination!=reinterpret_cast<LPSTR>(0x11));
    check(state.cp==932&&state.flags==9&&state.wide==input&&state.count==2&&state.capacity==4);
    check(state.default_character==&default_character&&state.used_default==&used);
    check(static_cast<std::uint32_t>(used)==0x76543210&&state.incoming==0x1111&&state.error==0x4444);
    check(bytes[0].get()==0xa0&&bytes[1].get()==0xa1);
    for(unsigned i=2;i<5;++i)check(bytes[i].get()==0x77);
    state.mode=1;
    check(call.wide_char_to_multi_byte(0,0,address(input),2,0x11,4,0,0,bytes)==0);
    for(unsigned i=0;i<4;++i)check(!bytes[i].defined());
    check(bytes[4].get()==0x77&&state.error==0x4444);
    for(auto& x:bytes)x.set(0x66);
    check(call.wide_char_to_multi_byte(0,0,address(input),2,0,4,0,0,bytes)==0);
    check(state.byte_destination==nullptr&&state.error==0x4444);
    for(unsigned i=0;i<4;++i)check(!bytes[i].defined());
    check(bytes[4].get()==0x66);
    state.mode=0;
    domain([&]{call.wide_char_to_multi_byte(0,0,address(input),2,0,4,0,0,bytes);});
    check(state.byte_destination==nullptr&&state.error==0x4444);
    for(unsigned i=0;i<4;++i)check(!bytes[i].defined());
    for(auto& x:bytes)x.set(0x66);
    check(call.wide_char_to_multi_byte(0,0,address(input),2,0x11,0,0,0,bytes)==7);
    check(state.byte_destination==nullptr);for(auto& x:bytes)check(x.get()==0x66);
    state.mode=2;
    domain([&]{call.wide_char_to_multi_byte(0,0,address(input),2,0x11,4,0,0,bytes);});
    for(unsigned i=0;i<4;++i)check(!bytes[i].defined());
    check(state.error==0x4444);
    const auto before=state.wide_calls;state.error=0x9876;
    domain([&]{call.wide_char_to_multi_byte(0,0,address(input),2,0x11,-1,0,0,bytes);});
    domain([&]{call.wide_char_to_multi_byte(0,0,address(input),2,0x11,6,0,0,bytes);});
    check(state.wide_calls==before&&state.error==0x9876);
    char raw[5]{};state.mode=0;
    check(call.wide_char_to_multi_byte(1252,3,address(input),-1,address(raw),4,0,0,{})==2);
    check(state.byte_destination==raw&&state.count==-1&&static_cast<unsigned char>(raw[0])==0xa0);
    check(call.wide_char_to_multi_byte(1252,0,address(input),-7,0x22,-9,0,0,{})==0);
    check(state.byte_destination==reinterpret_cast<LPSTR>(0x22)&&state.count==-7&&state.capacity==-9);
    std::array<n::WordOutput,4> words;for(auto& x:words)x.set(0x7777);
    state.error=0x1111;
    check(call.get_string_type_w(1,address(input),3,0x11,words)==0x12345678);
    check(state.word_destination!=reinterpret_cast<LPWORD>(0x11));
    check(state.incoming==0x1111&&state.error==0x5555&&state.flags==1&&state.wide==input);
    for(unsigned i=0;i<3;++i)check(words[i].get()==0x1200+i);
    check(words[3].get()==0x7777);
    state.mode=1;check(call.get_string_type_w(3,address(input),3,0x11,words)==0);
    for(unsigned i=0;i<3;++i)check(!words[i].defined());
    check(words[3].get()==0x7777&&state.error==0x5555);
    for(auto& x:words)x.set(0x6666);
    check(call.get_string_type_w(1,address(input),3,0,words)==0);
    check(state.word_destination==nullptr&&state.error==0x5555);
    for(unsigned i=0;i<3;++i)check(!words[i].defined());
    check(words[3].get()==0x6666);
    state.mode=0;
    domain([&]{call.get_string_type_w(1,address(input),3,0,words);});
    check(state.word_destination==nullptr&&state.error==0x5555);
    for(unsigned i=0;i<3;++i)check(!words[i].defined());
    const auto type_before=state.type_calls;state.error=0x9876;
    for(int count:{-1,0,5})domain([&]{call.get_string_type_w(1,address(input),count,0x11,words);});
    check(state.type_calls==type_before&&state.error==0x9876);
    WORD raw_words[3]{};state.mode=0;
    check(call.get_string_type_w(1,address(input),-5,address(raw_words),{})==0x12345678);
    check(state.count==-5&&state.word_destination==raw_words&&raw_words[2]==0x1202);
    auto broken=fixture;broken.get_string_type_w=nullptr;bool caught=false;
    try{n::Calls unused(broken);}catch(const std::invalid_argument&){caught=true;}check(caught);
}
void test_native() {
    n::Calls call;
    check(call.get_acp()!=0);check(call.get_oemcp()!=0);check(call.is_valid_code_page(CP_UTF8)!=0);
    n::CPInfo cp{};check(call.get_cp_info(CP_UTF8,cp)!=0&&cp.max_char_size==4);
    constexpr char utf8[]={'A',static_cast<char>(0xc3),static_cast<char>(0xa9)};
    wchar_t wide[2]{};
    check(call.multi_byte_to_wide(CP_UTF8,MB_ERR_INVALID_CHARS,address(utf8),3,address(wide),2)==2);
    check(wide[0]==L'A'&&wide[1]==0xe9);
    std::array<n::ByteOutput,4> bytes;bytes[3].set(0x77);
    check(call.wide_char_to_multi_byte(CP_UTF8,0,address(wide),2,0x11,3,0,0,bytes)==3);
    for(unsigned i=0;i<3;++i)check(bytes[i].get()==static_cast<unsigned char>(utf8[i]));
    check(bytes[3].get()==0x77);
    constexpr wchar_t text[]=L"A1 ";std::array<n::WordOutput,4> words;words[3].set(0x7777);
    check(call.get_string_type_w(CT_CTYPE1,address(text),3,0x11,words)!=0);
    check((words[0].get()&C1_UPPER)!=0);check((words[1].get()&C1_DIGIT)!=0);
    check((words[2].get()&C1_SPACE)!=0&&words[3].get()==0x7777);
    char raw[8]{};
    check(call.wide_char_to_multi_byte(CP_UTF8,0,address(text),-1,address(raw),8,0,0,{})==4);
    check(raw[0]=='A'&&raw[1]=='1'&&raw[2]==' '&&raw[3]==0);
    WORD raw_words[4]{};
    check(call.get_string_type_w(CT_CTYPE1,address(text),-1,address(raw_words),{})!=0);
    check((raw_words[3]&C1_CNTRL)!=0);
    for(auto& x:bytes)x.set(0x44);SetLastError(0x1234);
    check(call.wide_char_to_multi_byte(CP_UTF8,0,address(wide),2,0x11,1,0,0,bytes)==0);
    check(GetLastError()==ERROR_INSUFFICIENT_BUFFER&&!bytes[0].defined());
    for(unsigned i=1;i<4;++i)check(bytes[i].get()==0x44);
    for(auto& x:words)x.set(0x5555);SetLastError(0x1234);
    check(call.get_string_type_w(0xffffffff,address(text),3,0x11,words)==0);
    check(GetLastError()!=0x1234);for(unsigned i=0;i<3;++i)check(!words[i].defined());
    check(words[3].get()==0x5555);
}
}
int main() {
    try{test_fixture();test_native();std::printf("PASS %llu checks; typed WINAPI fixtures and bounded synthetic native NLS calls\n",static_cast<unsigned long long>(checks));return 0;}
    catch(const std::exception& error){std::fprintf(stderr,"FAIL after %llu checks: %s\n",static_cast<unsigned long long>(checks),error.what());return 1;}
}
