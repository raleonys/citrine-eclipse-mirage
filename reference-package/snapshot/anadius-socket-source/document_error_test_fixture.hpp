#pragma once
#include "document_error.hpp"
#include "parse_entry_test_fixture.hpp"
#include <cstring>

namespace document_error_test {
namespace s=anadius::socket_recovered;
using parse_entry_test::check;using parse_entry_test::checks;using parse_entry_test::word;using parse_entry_test::put;
struct Failure{};
template<class F>void fails(F f){bool caught=false;try{f();}catch(const Failure&){caught=true;}check(caught,"controlled lower failure propagated");}
template<class F>void unmapped(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"bounded mapping failure propagated");}
struct Memory {
    static constexpr std::uintptr_t document_address=0x430000000ULL,temporary_address=0x530000000ULL,owned_address=0x630000000ULL;
    static constexpr std::uintptr_t names_address=0x1800a1010ULL,name=0xfeca1234ULL;
    std::array<std::uint8_t,0x308> document{};std::array<std::uint8_t,512> table{};
    std::vector<std::uint8_t> temporary=std::vector<std::uint8_t>(4096,0xcc),owned=std::vector<std::uint8_t>(4096,0xdd);
    std::vector<s::ReceiveBufferRegion> regions,extras;std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;s::ReceiveBufferMemory memory{};
    Memory(){document.fill(0xa5);for(unsigned at:{0x78U,0x80U,0x88U})put(document,at,0,at==0x78?4:8);set_name(13);refresh();}
    void set_name(std::int32_t id,std::uintptr_t n=name){const auto offset=std::int64_t{16}+std::int64_t{id}*8;check(offset>=0&&offset+8<=static_cast<std::int64_t>(table.size()),"fixture table bounds only");put(table,static_cast<std::size_t>(offset),n);}
    void add(std::uintptr_t a,std::span<std::uint8_t> b){if(!omitted||omitted->first>=a+b.size()||omitted->first+omitted->second<=a){regions.push_back({a,b});return;}const auto start=std::max(a,omitted->first),end=std::min(a+b.size(),omitted->first+omitted->second);if(start>a)regions.push_back({a,b.first(start-a)});if(end<a+b.size())regions.push_back({end,b.subspan(end-a)});}
    void refresh(){regions.clear();add(document_address,document);add(names_address-16,table);add(temporary_address,temporary);add(owned_address,owned);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    std::uint64_t get(unsigned at,unsigned width=8){return word(memory.view(document_address+at,width),0,width);}
    void set(unsigned at,std::uint64_t value,unsigned width=8){put(memory.view(document_address+at,width),0,value,width);}
    void text(std::uintptr_t address,std::string_view value){auto b=memory.view(address,value.size()+1);std::memcpy(b.data(),value.data(),value.size());b[value.size()]=0;}
};
struct Operations {
    Memory& raw;std::vector<std::string> events;std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    struct Copy{std::uintptr_t to,from;std::uint64_t count;};std::vector<Copy> copies;
    std::function<void(unsigned,std::uint64_t)> allocation_hook;std::function<void(unsigned,std::uintptr_t)> free_hook;std::function<void(const Copy&)> copy_hook;
    explicit Operations(Memory& m):raw(m){}
    std::uintptr_t allocate(std::uint64_t n){allocations.push_back(n);events.push_back("allocate"+std::to_string(allocations.size()));if(allocation_hook)allocation_hook(static_cast<unsigned>(allocations.size()),n);return allocations.size()==1?Memory::temporary_address:Memory::owned_address;}
    void free(std::uintptr_t p){frees.push_back(p);events.push_back("free");if(free_hook)free_hook(static_cast<unsigned>(frees.size()),p);}
    void copy(std::uintptr_t to,std::uintptr_t from,std::uint64_t n){copies.push_back({to,from,n});events.push_back("copy");auto a=raw.memory.view(to,n),b=raw.memory.view(from,n);std::memmove(a.data(),b.data(),static_cast<std::size_t>(n));if(copy_hook)copy_hook(copies.back());}
};
struct Docs:parse_entry_test::CoreDocs {
    Operations& ops;explicit Docs(Operations& o):ops(o){}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{return ops.allocate(n);}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{ops.copy(to,from,n);}
    void free_04b0f8(std::uintptr_t p)override{ops.free(p);}
};
struct Lower:s::NativeDocumentErrorEffects {
    struct Header{std::uintptr_t destination;std::uint64_t capacity;std::uintptr_t format,name;std::int32_t id;std::uint32_t hex,line;};
    struct Format{std::uintptr_t destination;std::uint64_t capacity;std::uintptr_t format,arguments;};
    Operations& ops;std::vector<Header> headers;std::vector<Format> separators,details;
    std::string header_text="controlled error",separator_text=": ",detail_text="controlled detail";
    std::function<void()> header_hook,separator_hook,detail_hook;
    explicit Lower(Operations& o):ops(o){}
    void format_header_0060a0(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t n,std::int32_t i,std::uint32_t h,std::uint32_t l)override{headers.push_back({d,c,f,n,i,h,l});ops.events.push_back("header");ops.raw.text(d,header_text);if(header_hook)header_hook();}
    void format_separator_0060a0(std::uintptr_t d,std::uint64_t c,std::uintptr_t f)override{separators.push_back({d,c,f,0});ops.events.push_back("separator");ops.raw.text(d,separator_text);if(separator_hook)separator_hook();}
    void format_detail_006110(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t a)override{details.push_back({d,c,f,a});ops.events.push_back("detail");ops.raw.text(d,detail_text);if(detail_hook)detail_hook();}
};
struct Fixture {
    Memory raw;Operations ops{raw};Docs docs{ops};Lower lower{ops};s::NativeDocumentErrorContext context{raw.memory,docs,lower};
    void set(std::int32_t id=13,std::uint32_t line=7,std::uintptr_t detail=0,std::uintptr_t arguments=0){s::set_document_error_005820(Memory::document_address,id,line,detail,{arguments},context);}
    void complete(std::string_view text,std::uint32_t error=13,std::uint32_t line=7){check(raw.get(0x6c,4)==error&&raw.get(0x90,4)==line,"code and line DWORD state");check(raw.get(0x78,4)==0x200&&raw.get(0x80)==Memory::owned_address&&raw.get(0x88)==Memory::owned_address+text.size(),"owned error start,end and exact flag");check(ops.allocations==std::vector<std::uint64_t>{1000,text.size()+1}&&ops.copies.size()==1&&ops.copies[0].to==Memory::owned_address&&ops.copies[0].from==Memory::temporary_address&&ops.copies[0].count==text.size()+1,"distinct scratch/final allocation and exact terminating copy");check(std::equal(text.begin(),text.end(),raw.owned.begin())&&raw.owned[text.size()]==0,"supplied formatter output copied with terminator");check(!docs.other_calls&&docs.errors.empty(),"core does not invoke additional document effects");}
};
} // namespace document_error_test
