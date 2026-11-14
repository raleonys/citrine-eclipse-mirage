// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/node_string.hpp"
#include "tests/support/socket/node_parse_test_fixture.hpp"

namespace node_string_test {
namespace s=laureondo::socket;
using node_parse_test::check;using node_parse_test::checks;using node_parse_test::put;using node_parse_test::word;
using node_parse_test::Failure;using node_parse_test::fails;using node_parse_test::unmapped;
struct Characters:s::NativeParseEntryEffects {
    std::vector<std::uint32_t> calls;std::function<std::int32_t(std::uint32_t)> hook;
    std::int32_t classify_051fc0(std::uint32_t b)override{calls.push_back(b);return hook?hook(b):parse_entry_test::controlled_space(b);}
    void parse_virtual(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{throw std::runtime_error("unexpected parse boundary");}
};
struct Parser:s::NativeNodeParseEffects {
    struct Compare{std::uintptr_t left,right;std::uint64_t count;};
    s::ReceiveBufferMemory& memory;std::vector<Compare> calls;std::function<std::int32_t(const Compare&)> hook;
    explicit Parser(s::ReceiveBufferMemory& m):memory(m){}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{calls.push_back({a,b,n});if(hook)return hook(calls.back());if(n>64)throw Failure{};for(std::uint64_t i=0;i<n;++i){const auto x=memory.view(a+i,1)[0],y=memory.view(b+i,1)[0];if(x!=y)return x<y?-1:1;if(!x)break;}return 0;}
    std::uintptr_t identify_0038b0(std::uintptr_t,std::uintptr_t,std::uintptr_t)override{throw Failure{};}
    std::uintptr_t parse_child_virtual(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{throw Failure{};}
    std::uintptr_t materialize_0035e0(std::uintptr_t)override{throw Failure{};}
    void prepare_003aa0(std::uintptr_t,std::uintptr_t)override{throw Failure{};}
    void free_04b0f8(std::uintptr_t)override{throw Failure{};}
    void report_error(const s::NativeNodeParseError&)override{throw Failure{};}
};
struct Lower:s::NativeNodeStringEffects {
    struct Copy{std::uintptr_t output;std::array<std::uint8_t,10> scratch;std::uint64_t count;};
    s::ReceiveBufferMemory& memory;std::vector<std::uintptr_t> decodes;std::vector<Copy> copies;
    std::function<std::uintptr_t(std::uintptr_t,std::span<std::uint8_t,10>,std::int32_t&)> decode_hook;
    std::function<void(const Copy&)> copy_hook;
    explicit Lower(s::ReceiveBufferMemory& m):memory(m){}
    std::uintptr_t decode_0033c0(std::uintptr_t p,std::span<std::uint8_t,10> b,std::int32_t& n)override{decodes.push_back(p);if(decode_hook)return decode_hook(p,b,n);return 0;}
    void copy_string_078d30(std::uintptr_t p,std::span<const std::uint8_t,10> b,std::uint64_t n)override{Copy c{p,{},n};std::copy(b.begin(),b.end(),c.scratch.begin());copies.push_back(c);if(copy_hook)copy_hook(copies.back());if(n>10)throw Failure{};auto out=memory.view(p,n);std::copy_n(b.begin(),static_cast<std::size_t>(n),out.begin());}
};
struct Fixture {
    static constexpr std::uintptr_t pair=0x610000000ULL,input=0x620000000ULL,table=0x18007f760ULL,names=0x18007f7b0ULL;
    std::array<std::uint8_t,24> object{};std::array<std::uint8_t,128> bytes{};
    std::array<std::uint8_t,80> entities{};std::array<std::uint8_t,28> names_bytes{};
    std::vector<s::ReceiveBufferRegion> regions,extras;std::vector<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};Characters characters;Parser parser{memory};Lower lower{memory};
    s::NativeNodeStringContext context{memory,characters,parser,lower};
    Fixture(){const std::array<std::uintptr_t,5> pointers{names,names+8,names+12,names+20,names+24};const std::array<unsigned,5> lengths{4,3,4,2,2},replacement{34,38,39,60,62};for(unsigned i=0;i<5;++i){put(entities,i*16,pointers[i]);put(entities,i*16+8,lengths[i],4);put(entities,i*16+12,replacement[i],1);}const std::array<std::string_view,5> literals{"quot","amp","apos","lt","gt"};for(unsigned i=0;i<5;++i)std::copy(literals[i].begin(),literals[i].end(),names_bytes.begin()+static_cast<std::ptrdiff_t>(pointers[i]-names));refresh();text("x",0x101);}
    void add(std::uintptr_t a,std::span<std::uint8_t> b,bool extra=false){std::vector<s::ReceiveBufferRegion> chunks{{a,b}};auto cuts=omitted;if(!extra)for(auto r:extras)cuts.push_back({r.address,r.bytes.size()});for(auto [start,count]:cuts){std::vector<s::ReceiveBufferRegion> next;for(auto r:chunks){auto end=r.address+r.bytes.size(),stop=start+count;if(stop<=r.address||start>=end){next.push_back(r);continue;}auto lo=std::max(start,r.address),hi=std::min(stop,end);if(lo>r.address)next.push_back({r.address,r.bytes.first(lo-r.address)});if(hi<end)next.push_back({hi,r.bytes.subspan(hi-r.address)});}chunks=std::move(next);}for(auto r:chunks)regions.push_back(r);}
    void refresh(){regions.clear();add(pair,object);add(input,bytes);add(table,entities);add(names,names_bytes);for(auto r:extras)add(r.address,r.bytes,true);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t a,unsigned n=8){return word(memory.view(a,n),0,n);}
    void set(std::uintptr_t a,std::uint64_t v,unsigned n=8){put(memory.view(a,n),0,v,n);}
    void text(std::string_view value,std::uint32_t flags=0x101){bytes.fill(0xa5);std::copy(value.begin(),value.end(),bytes.begin());set(pair,flags,4);set(pair+8,input);set(pair+16,input+value.size());}
    std::string result(std::uintptr_t p=input){std::string out;for(unsigned i=0;i<128;++i){const auto b=memory.view(p+i,1)[0];if(!b)return out;out.push_back(static_cast<char>(b));}throw std::runtime_error("fixture output lacks bounded NUL");}
    std::uintptr_t call(){return s::materialize_node_string_0035e0(pair,context);}
};
} // namespace node_string_test
