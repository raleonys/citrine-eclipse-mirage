#include "native.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <stdexcept>
#include <string_view>
#include <vector>
namespace n=anadius::crt_nls_dispatch_native;
namespace {
std::size_t checks{};
void check(bool b){++checks;if(!b)throw std::runtime_error("native dispatch check failed");}
template<class T> n::Address address(T p){return reinterpret_cast<std::uintptr_t>(p);}
struct Fixture {
    DWORD error{0x12345678}, incoming{}, after{0x87654321};
    int result{3}, calls{}; bool write{true};
    std::array<n::Address,9> args{};
} f;
DWORD WINAPI get_error(){return f.error;}
void WINAPI set_error(DWORD e){f.error=e;}
int body(LPWSTR dest,int cap){
    ++f.calls; f.incoming=f.error;
    if(f.write && dest && cap>0) {
        auto* bytes=reinterpret_cast<unsigned char*>(dest);
        for(int i=0;i<std::min(cap,std::max(f.result,1));++i)bytes[i]=static_cast<unsigned char>(0x80+i);
    }
    f.error=f.after;return f.result;
}
int WINAPI map_w(LCID lcid,DWORD flags,LPCWSTR src,int count,LPWSTR dest,int cap){
    f.args={lcid,flags,address(src),static_cast<n::Address>(static_cast<std::int64_t>(count)),address(dest),static_cast<n::Address>(static_cast<std::int64_t>(cap)),0,0,0};
    return body(dest,cap);
}
int WINAPI map_ex(LPCWSTR name,DWORD flags,LPCWSTR src,int count,LPWSTR dest,int cap,
    LPNLSVERSIONINFO version,LPVOID reserved,LPARAM handle){
    f.args={address(name),flags,address(src),static_cast<n::Address>(static_cast<std::int64_t>(count)),address(dest),static_cast<n::Address>(static_cast<std::int64_t>(cap)),address(version),address(reserved),static_cast<n::Address>(handle)};
    return body(dest,cap);
}
LCID WINAPI name_lcid(LPCWSTR name,DWORD flags){
    ++f.calls;f.incoming=f.error;f.args[0]=address(name);f.args[1]=flags;f.error=f.after;
    return static_cast<LCID>(f.result);
}
void fixtures(){
    n::Calls calls({&map_w,&get_error,&set_error});
    std::array<n::ByteOutput,9> cells;
    for(auto& cell:cells)cell.set(0x44);
    auto result=calls.lc_map_string_w(0xfedcba98,LCMAP_SORTKEY|NORM_IGNORECASE,0x3456,-7,0x4567,7,cells);
    check(result==3&&f.incoming==0x12345678&&f.error==f.after);
    check(f.args[0]==0xfedcba98&&f.args[1]==(LCMAP_SORTKEY|NORM_IGNORECASE)&&f.args[2]==0x3456);
    check(f.args[3]==static_cast<n::Address>(-7ll)&&f.args[4]!=0x4567&&f.args[4]%alignof(WCHAR)==0&&f.args[5]==7);
    for(unsigned i=0;i<cells.size();++i)check(cells[i].get()==(i<3?0x80+i:0x44));
    f.error=0x11111111; f.result=5;
    result=calls.lc_map_string_ex(address(&map_ex),0x1234,LCMAP_SORTKEY,0x3456,-1,0x4567,7,0x5678,0x6789,0xfedcba9876543210,cells);
    check(result==5&&f.incoming==0x11111111&&f.error==f.after);
    check(f.args[0]==0x1234&&f.args[2]==0x3456&&f.args[3]==~n::Address{}&&f.args[5]==7);
    check(f.args[6]==0x5678&&f.args[7]==0x6789&&f.args[8]==0xfedcba9876543210);
    // Failure may have partially written staging. Every possible output cell
    // becomes unknown; cells beyond capacity retain prior evidence.
    f.result=0;
    result=calls.lc_map_string_w(0x409,LCMAP_SORTKEY,0x12,2,0x34,7,cells);
    check(result==0&&f.error==f.after);
    for(unsigned i=0;i<7;++i)check(!cells[i].defined());
    check(cells[7].get()==0x44&&cells[8].get()==0x44);
    // Queries do not materialize bytes, even when a fixture returns a large size.
    f.result=0x76543210;
    result=calls.lc_map_string_w(0x409,LCMAP_SORTKEY,0x12,-1,0x12345678,0,cells);
    check(result==0x76543210&&f.args[4]==0&&f.args[5]==0&&!cells[0].defined());
    // Raw paths preserve every supplied pointer, signed count and full result.
    f.write=false;f.result=-123456789;
    result=calls.lc_map_string_ex(address(&map_ex),0x12,0xfedcba98,0x34,-37,0x56,-19,0x78,0x9a,0xffffeeeeaaaa1234,{});
    check(result==-123456789&&f.args[4]==0x56&&f.args[5]==static_cast<n::Address>(-19ll));
    check(f.args[1]==0xfedcba98&&f.args[6]==0x78&&f.args[7]==0x9a&&f.args[8]==0xffffeeeeaaaa1234);
    f.result=static_cast<int>(0xfedcba98u);f.error=0x11112222;
    check(calls.locale_name_to_lcid(address(&name_lcid),0xabcd,0x12345678)==0xfedcba98);
    check(f.args[0]==0xabcd&&f.args[1]==0x12345678&&f.incoming==0x11112222&&f.error==f.after);
    auto throws=[&](auto action){const auto before=f.calls;bool thrown=false;try{action();}catch(const std::invalid_argument&){thrown=true;}check(thrown&&f.calls==before);};
    throws([&]{calls.lc_map_string_w(0,LCMAP_UPPERCASE,1,1,2,1,cells);});
    throws([&]{calls.lc_map_string_w(0,LCMAP_SORTKEY,1,1,2,10,cells);});
    throws([&]{calls.lc_map_string_w(0,LCMAP_SORTKEY,1,1,2,-1,cells);});
    throws([&]{calls.lc_map_string_ex(0,0,0,0,0,0,0,0,0,0,{});});
    throws([&]{calls.locale_name_to_lcid(0,0,0);});
    bool thrown=false;f.result=8;
    for(auto& cell:cells)cell.set(0x66);
    try{calls.lc_map_string_w(0,LCMAP_SORTKEY,1,1,2,7,cells);}catch(const std::runtime_error&){thrown=true;}
    check(thrown&&f.error==f.after);
    for(unsigned i=0;i<7;++i)check(!cells[i].defined());
    check(cells[7].get()==0x66&&cells[8].get()==0x66);
    for(auto& cell:cells)cell.set(0x55);
    thrown=false;f.result=3;
    try{calls.lc_map_string_w(0,LCMAP_SORTKEY,1,1,0,7,cells);}catch(const std::runtime_error&){thrown=true;}
    check(thrown&&f.args[4]==0&&f.error==f.after);
    for(unsigned i=0;i<7;++i)check(!cells[i].defined());
    check(cells[7].get()==0x55&&cells[8].get()==0x55);
    f.result=0;
    check(calls.lc_map_string_w(0,LCMAP_SORTKEY,1,1,0,7,cells)==0&&f.args[4]==0);
    for(unsigned i=0;i<7;++i)check(!cells[i].defined());
}
void windows_literals(){
    n::Calls calls;
    // kernel32 is already loaded by the test executable. No LoadLibrary call,
    // target artifact, original image address, or user file participates.
    const auto kernel=::GetModuleHandleW(L"kernel32.dll");check(kernel!=nullptr);
    const auto ex=::GetProcAddress(kernel,"LCMapStringEx");
    const auto name=::GetProcAddress(kernel,"LocaleNameToLCID");check(ex&&name);
    constexpr WCHAR input[]=L"AbCd";constexpr WCHAR locale[]=L"en-US";
    std::array<WCHAR,16> output{};
    check(calls.lc_map_string_w(0x409,LCMAP_UPPERCASE,address(input),-1,address(output.data()),16,{})==5);
    check(std::wstring_view(output.data())==L"ABCD");
    check(calls.lc_map_string_ex(address(ex),address(locale),LCMAP_LOWERCASE,address(input),-1,address(output.data()),16,0,0,0,{})==5);
    check(std::wstring_view(output.data())==L"abcd");
    check(calls.locale_name_to_lcid(address(name),address(locale),0)==0x409);
    const auto required=calls.lc_map_string_ex(address(ex),address(locale),LCMAP_SORTKEY,address(input),-1,0,0,0,0,0,{});
    check(required>0);
    std::vector<n::ByteOutput> cells(static_cast<std::size_t>(required)+2);
    std::vector<WCHAR> direct((static_cast<std::size_t>(required)+1)/2);
    const auto direct_ex=reinterpret_cast<decltype(&::LCMapStringEx)>(ex);
    ::SetLastError(0x12345678);
    const auto expected=direct_ex(locale,LCMAP_SORTKEY,input,-1,direct.data(),required,nullptr,nullptr,0);
    const auto expected_error=::GetLastError();
    ::SetLastError(0x12345678);
    check(calls.lc_map_string_ex(address(ex),address(locale),LCMAP_SORTKEY,address(input),-1,1,required,0,0,0,cells)==expected);
    check(::GetLastError()==expected_error&&expected==required);
    const auto* bytes=reinterpret_cast<const unsigned char*>(direct.data());
    for(int i=0;i<required;++i)check(cells[static_cast<std::size_t>(i)].get()==bytes[i]);
    check(!cells[static_cast<std::size_t>(required)].defined());
    check(calls.lc_map_string_w(0x409,LCMAP_SORTKEY,address(input),-1,1,1,cells)==0);
    check(!cells[0].defined()&&::GetLastError()==ERROR_INSUFFICIENT_BUFFER);
    const auto w_required=calls.lc_map_string_w(0x409,LCMAP_SORTKEY,address(input),-1,0,0,{});
    check(w_required>0);
    std::vector<n::ByteOutput> w_cells(static_cast<std::size_t>(w_required));
    check(calls.lc_map_string_w(0x409,LCMAP_SORTKEY,address(input),-1,1,w_required,w_cells)==w_required);
    check(w_cells.back().get()==0);
}
}
int main(){try{fixtures();windows_literals();std::printf("PASS nls native dispatch: %zu checks; typed fixtures and Windows synthetic literals only\n",checks);return 0;}catch(const std::exception& e){std::fprintf(stderr,"FAIL %s (%zu checks)\n",e.what(),checks);return 1;}}
