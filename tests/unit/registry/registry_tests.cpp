// SPDX-License-Identifier: GPL-3.0-only

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#include "citrine_eclipse_mirage/platform/registry.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <type_traits>
#include <vector>
using namespace citrine_eclipse_mirage::registry;
namespace {
int checks=0;
void check(bool ok) { ++checks; if(!ok) throw std::runtime_error("check "+std::to_string(checks)); }
struct Mock final: Effects {
    std::vector<std::string> events;
    Status open_status=5,query_status=87,close_status=0;
    Handle native_handle=0x42,next_handle=0x1000;
    int copy_status=0;
    bool mutate_query=true,throw_wait=false,throw_alloc=false;
    std::uint32_t query_type=0,query_size=777;
    std::size_t copy_capacity=0;
    Handle observed_root=0;
    std::uint32_t observed_options=0,observed_access=0;
    void* observed_result=nullptr;
    void* observed_type=nullptr;
    void* observed_size=nullptr;
    std::function<void()> conversion_effect;
    std::string filesystem_narrow(std::u16string_view s) override {
        events.push_back("convert"); if(conversion_effect)conversion_effect();
        std::string r; for(auto c:s) { if(c>127)throw std::runtime_error("synthetic ASCII conversion only"); r+=static_cast<char>(c); } return r;
    }
    template<class C> static int compare(const C* a,const C* b) {
        const auto lower=[](C c){return c>=C('A')&&c<=C('Z')?c+C('a')-C('A'):c;};
        while(*a && lower(*a)==lower(*b)){++a;++b;} return lower(*a)-lower(*b);
    }
    int crt_stricmp(const char* a,const char* b)override {events.push_back("cmpA");return compare(a,b);}
    int crt_wcsicmp(const char16_t* a,const char16_t* b)override {events.push_back("cmpW");return compare(a,b);}
    int crt_strcpy_s(char* a,std::size_t n,const char* b)override {
        events.push_back("copyA");copy_capacity=n;if(copy_status)return copy_status;
        const auto length=std::strlen(b)+1;check(length<=n);std::memcpy(a,b,length);return 0;
    }
    int crt_wcscpy_s(char16_t* a,std::size_t n,const char16_t* b)override {
        events.push_back("copyW");copy_capacity=n;if(copy_status)return copy_status;
        const auto length=std::char_traits<char16_t>::length(b)+1;check(length<=n);std::memcpy(a,b,length*2);return 0;
    }
    template<class O> Status open(const O& a){observed_root=a.root;observed_options=a.options;observed_access=a.access;observed_result=a.result.raw();if(!open_status&&a.result)a.result.store(native_handle);return open_status;}
    Status original_open_a(const OpenA& a)override {events.push_back("openA");return open(a);}
    Status original_open_w(const OpenW& a)override {events.push_back("openW");return open(a);}
    template<class Q> Status query(const Q& a){observed_type=a.type.raw();observed_size=a.size.raw();if(mutate_query){if(a.size)a.size.store(query_size);if(a.type)a.type.store(query_type);}return query_status;}
    Status original_query_a(const QueryA& a)override {events.push_back("queryA");return query(a);}
    Status original_query_w(const QueryW& a)override {events.push_back("queryW");return query(a);}
    Status original_close(Handle)override{events.push_back("close");return close_status;}
    Handle allocate_4_04b0bc()override{events.push_back("alloc4");if(throw_alloc)throw std::bad_alloc();return next_handle++;}
    void free_04b0f8(Handle)override{events.push_back("free");}
    void wait_036880(std::int32_t n)override{check(n==-1);events.push_back("wait");if(throw_wait)throw std::runtime_error("wait");}
    void log_literal_006880(std::uint64_t v)override{events.push_back("log:"+std::to_string(v));}
    void log_text_007990(std::string_view v)override{events.push_back("text:"+std::string(v));}
    void finish_log_0088e0()override{events.push_back("finish");}
};
struct Fixture {
    Values values{{"ClientKey",u"Software\\Synthetic"},{"ClientKey2",u"Software\\Synthetic2"},
      {"LanguageKey",u"Software\\SyntheticLanguage"},{"ClientValue",u"Install"},
      {"LanguageValue",u"Locale"},{"ClientPath",u"C:\\synthetic"},{"Language",u"en_US"}};
    Handles client,language,fake;
    std::uint8_t enabled=1;
    Mock e;
    StateRefs state(){return {values,client,language,fake,enabled};}
};
void match_tests(){
    Fixture f;
    check(compare_narrow_0086b0(f.values,f.e,nullptr,"ClientKey")==1);check(f.e.events.empty());
    check(compare_wide_0087f0(f.values,f.e,nullptr,"ClientKey")==1);check(f.e.events.empty());
    check(compare_narrow_0086b0(f.values,f.e,"software\\synthetic","ClientKey")==0);
    check(compare_wide_0087f0(f.values,f.e,u"SOFTWARE\\SYNTHETIC","ClientKey")==0);
    f.e.events.clear();check(resolve_narrow_008980(f.values,f.e,"clientkey").empty());check(f.e.events.empty());
    f.values["Empty"]=u"";check(resolve_narrow_008980(f.values,f.e,"Empty").empty());check(f.e.events==std::vector<std::string>{"convert"});
    f.values["Binary"]=std::u16string(u"A\0B",3);
    check(resolve_narrow_008980(f.values,f.e,"Binary")==std::string("A\0B",3));
    check(resolve_wide_008b80(f.values,"Binary")==std::u16string(u"A\0B",3));
    check(compare_wide_0087f0(f.values,f.e,u"a","Binary")==0);
    check(compare_narrow_0086b0(f.values,f.e,"a","Binary")==0);
}
void open_tests(){
    for(bool wide:{false,true})for(bool enabled:{false,true})for(Status status:{0u,2u,5u,0xffffffffu})for(int which=0;which<5;++which){
        Fixture f;f.enabled=enabled;f.e.open_status=status;Handle h=0x999;
        constexpr const char* names[]{"software\\synthetic","SOFTWARE\\SYNTHETIC2","Software\\SyntheticLanguage","other",nullptr};
        constexpr const char16_t* wnames[]{u"software\\synthetic",u"SOFTWARE\\SYNTHETIC2",u"Software\\SyntheticLanguage",u"other",nullptr};
        const auto result=wide?open_w_00f960(f.state(),f.e,{0x123,wnames[which],11,22,&h}):open_a_00f800(f.state(),f.e,{0x123,names[which],11,22,&h});
        const bool matched=which<2||(which==2&&enabled);
        check(result==(matched?0:status));check(f.e.events.front()==(wide?"openW":"openA"));
        check(f.client.contains(h)==(matched&&which<2));check(f.language.contains(h)==(matched&&which==2));
        check(f.fake.contains(h)==(matched&&status!=0));
        check(h==(matched&&status?0x1000:status?0x999:0x42));
        check(f.e.observed_root==0x123&&f.e.observed_options==11&&f.e.observed_access==22);
    }
    Fixture f;f.e.throw_alloc=true;Handle h=4;
    try{open_a_00f800(f.state(),f.e,{0,"Software\\Synthetic",0,0,&h});check(false);}catch(const std::bad_alloc&){check(true);}
    check(h==4&&f.client.empty()&&f.fake.empty());
    try{open_a_00f800(f.state(),f.e,{0,"Software\\Synthetic",0,0,nullptr});check(false);}catch(const OutOfEvidenceInput&){check(true);}
    f.e.throw_alloc=false;f.e.open_status=0;
    check(open_a_00f800(f.state(),f.e,{0,"Software\\Synthetic",0,0,&h})==0);
    check(open_a_00f800(f.state(),f.e,{0,"Software\\Synthetic2",0,0,&h})==0);
    check(f.client.size()==1&&f.fake.empty());
}
void query_tests(){
    for(bool wide:{false,true})for(std::uint32_t capacity:{0u,1u,12u,13u,24u,26u,500u})for(bool data:{false,true})for(bool size:{false,true}){
        Fixture f;f.client.insert(9);f.fake.insert(9);
        std::array<char16_t,256> buffer;buffer.fill(u'Z');std::uint32_t n=capacity,type=99;
        const auto result=wide?query_w_00fb60(f.state(),f.e,{9,u"INSTALL",nullptr,&type,data?buffer.data():nullptr,size?&n:nullptr}):query_a_00fac0(f.state(),f.e,{9,"INSTALL",nullptr,&type,data?buffer.data():nullptr,size?&n:nullptr});
        const std::uint32_t need=wide?26:13;
        check(result==(!size||(data&&capacity<need)?234u:0u));
        check(type==1);if(size)check(n==need);
        check(f.e.events.front()==(wide?"queryW":"queryA"));
        if(size&&data&&capacity>=need){check(f.e.copy_capacity==13);if(wide)check(buffer[0]==u'C');else check(reinterpret_cast<char*>(buffer.data())[0]=='C');}
        else check(buffer[0]==u'Z');
    }
    Fixture f;f.language.insert(8);f.e.query_type=7;std::uint32_t n=50,type=0;std::array<char,50>b{};
    check(query_a_00fac0(f.state(),f.e,{8,"Locale",nullptr,&type,b.data(),&n})==0);check(type==7&&n==6&&std::string(b.data())=="en_US");
    check(std::find(f.e.events.begin(),f.e.events.end(),"wait")==f.e.events.end());
    f.client.insert(8);f.values["ClientValue"]=u"Locale";f.e.events.clear();n=50;
    check(query_a_00fac0(f.state(),f.e,{8,"Locale",nullptr,nullptr,b.data(),&n})==0);check(std::string(b.data())=="C:\\synthetic");
    check(std::find(f.e.events.begin(),f.e.events.end(),"wait")!=f.e.events.end());
    f.e.copy_status=34;n=50;check(query_a_00fac0(f.state(),f.e,{8,"Locale",nullptr,nullptr,b.data(),&n})==234);
    f.e.events.clear();f.fake.insert(11);n=50;type=0;check(query_a_00fac0(f.state(),f.e,{11,nullptr,nullptr,&type,b.data(),&n})==2);check(n==777&&type==7);
    f.e.events.clear();check(query_w_00fb60(f.state(),f.e,{777,u"other",nullptr,nullptr,nullptr,nullptr})==87);check(f.e.events==std::vector<std::string>{"queryW"});
    f.values["Embedded"]=std::u16string(u"A\0B",3);b.fill('Z');n=50;f.e.copy_status=0;
    check(write_value_a_0091a0(f.values,f.e,b.data(),&n,"Embedded",50)==0);check(n==4&&b[0]=='A'&&b[1]==0&&b[2]=='Z');
    f.e.events.clear();check(write_value_a_0091a0(f.values,f.e,b.data(),nullptr,"Embedded",50)==234);check(f.e.events.empty());
    // Original stores capacity before conversion and re-reads it after conversion.
    n=50;f.e.conversion_effect=[&]{n=0;};check(write_value_a_0091a0(f.values,f.e,b.data(),&n,"Embedded",50)==234);check(n==4);
    f.e.conversion_effect={};f.e.throw_wait=true;f.e.query_type=0;f.e.events.clear();n=50;type=7;
    try{query_a_00fac0(f.state(),f.e,{8,"Locale",nullptr,&type,b.data(),&n});check(false);}catch(const std::runtime_error&){check(true);}
    check(type==0&&n==777);
    check(std::find(f.e.events.begin(),f.e.events.end(),"copyA")==f.e.events.end());
    check(f.e.events.back()=="wait");
}
void close_tests(){
    for(bool fabricated:{false,true})for(Status status:{0u,6u,0xffffffffu}){
        Fixture f;f.client.insert(4);f.language.insert(4);if(fabricated)f.fake.insert(4);f.e.close_status=status;
        check(close_00f5b0(f.state(),f.e,4)==(fabricated?0:status));
        const bool removed=fabricated||status==0;
        check(f.client.contains(4)==!removed);check(f.language.contains(4)==!removed);check(!f.fake.contains(4));
        check(f.e.events.front()==(fabricated?"free":"close"));
    }
}
void abi_cell_tests(){
    static_assert(sizeof(DWORD)==sizeof(std::uint32_t));
    static_assert(sizeof(HKEY)==sizeof(Handle));
    static_assert(!std::is_same_v<DWORD,std::uint32_t>);
    static_assert(!std::is_convertible_v<DWORD*,DwordCell>);
    static_assert(!std::is_convertible_v<PHKEY,HandleCell>);
    static_assert(!std::is_convertible_v<DwordCell,std::uint32_t*>);
    static_assert(!std::is_convertible_v<HandleCell,Handle*>);
    static_assert(std::is_convertible_v<std::uint32_t*,DwordCell>);
    static_assert(std::is_convertible_v<Handle*,HandleCell>);

    DwordCell missing=nullptr;
    check(!missing&&missing.raw()==nullptr);
    check(!DwordCell::from_raw(nullptr));
    check(!HandleCell{});
    try{(void)missing.load();check(false);}catch(const std::logic_error&){check(true);}
    try{missing.store(0);check(false);}catch(const std::logic_error&){check(true);}

    // Actual SDK scalar types are accessed only through copied bytes; neither
    // DWORD* -> uint32_t* nor PHKEY -> uint64_t* type-punning occurs.
    DWORD native_size=50,native_type=17;
    auto size=DwordCell::from_raw(&native_size);
    auto type=DwordCell::from_raw(&native_type);
    check(size.raw()==static_cast<void*>(&native_size)&&type.raw()==static_cast<void*>(&native_type));
    const std::uint32_t snapshot=size.load();
    native_size=7;
    check(snapshot==50&&size.load()==7);
    const auto same_size=DwordCell::from_raw(&native_size);
    same_size.store(0xffffffffu);
    check(native_size==0xfffffffful&&size.load()==0xffffffffu);
    type.store(0x12345678u);
    check(native_type==0x12345678ul);
    HKEY native_result=reinterpret_cast<HKEY>(std::uintptr_t{0x42});
    const auto result=HandleCell::from_raw(&native_result);
    check(result.raw()==static_cast<void*>(&native_result)&&result.load()==0x42);
    result.store(0x76543210);
    check(reinterpret_cast<std::uintptr_t>(native_result)==0x76543210);

    std::array<unsigned char,18> unaligned;
    unaligned.fill(0x5a);
    const auto unaligned32=DwordCell::from_raw(unaligned.data()+1);
    unaligned32.store(0x12345678u);
    check(unaligned32.load()==0x12345678u);
    check(unaligned.front()==0x5a&&unaligned[5]==0x5a);
    const auto unaligned64=HandleCell::from_raw(unaligned.data()+7);
    unaligned64.store(0xfedcba9876543210ull);
    check(unaligned64.load()==0xfedcba9876543210ull);
    check(unaligned[6]==0x5a&&unaligned[15]==0x5a);

    Fixture f;f.e.open_status=0;
    native_result=nullptr;
    check(open_a_00f800(f.state(),f.e,{0,"Software\\Synthetic",0,0,result})==0);
    check(f.e.observed_result==&native_result&&reinterpret_cast<std::uintptr_t>(native_result)==0x42);
    check(f.client.contains(0x42)&&f.fake.empty());
    f.e.open_status=5;
    check(open_w_00f960(f.state(),f.e,{0,u"Software\\Synthetic",0,0,result})==0);
    check(f.e.observed_result==&native_result&&reinterpret_cast<std::uintptr_t>(native_result)==0x1000);
    check(f.client.contains(0x1000)&&f.fake.contains(0x1000));

    std::array<char,50> buffer{};
    native_size=50;native_type=99;
    check(query_a_00fac0(f.state(),f.e,{0x42,"Install",nullptr,type,buffer.data(),size})==0);
    check(f.e.observed_size==&native_size&&f.e.observed_type==&native_type);
    check(native_size==13&&native_type==1&&std::string(buffer.data())=="C:\\synthetic");
    // Snapshot precedes original writes (777), while the conversion sees the
    // restored 50 and can mutate the caller's actual SDK DWORD immediately.
    native_size=50;native_type=99;
    f.e.conversion_effect=[&]{check(native_size==50);native_size=0;};
    check(write_value_a_0091a0(f.values,f.e,buffer.data(),size,"ClientPath",50)==234);
    check(native_size==13);
    f.e.conversion_effect={};
    native_size=50;native_type=99;
    f.e.query_type=7;
    check(query_w_00fb60(f.state(),f.e,{0x1000,u"other",nullptr,type,nullptr,size})==2);
    check(native_size==777&&native_type==7);
    native_size=50;native_type=99;
    f.e.query_type=0;f.e.throw_wait=true;
    try{query_a_00fac0(f.state(),f.e,{0x42,"Install",nullptr,type,buffer.data(),size});check(false);}catch(const std::runtime_error&){check(true);}
    check(native_size==777&&native_type==0);
}
}
int main(){try{match_tests();open_tests();query_tests();close_tests();abi_cell_tests();std::cout<<"registry-source: "<<checks<<" checks passed\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
