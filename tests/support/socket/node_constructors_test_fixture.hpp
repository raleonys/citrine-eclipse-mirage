// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/xml/node_constructors.hpp"
#include "tests/support/socket/node_identify_test_fixture.hpp"

namespace node_constructors_test {
namespace s=preloader_l::socket;
using node_identify_test::check;using node_identify_test::checks;using node_identify_test::put;using node_identify_test::word;
using node_identify_test::Failure;using node_identify_test::fails;using node_identify_test::unmapped;
inline constexpr std::array<unsigned,5> addresses{0x1590,0x1610,0x1690,0x1720,0x17b0};
inline constexpr std::array<std::uintptr_t,5> tables{0x18007fbb8ULL,0x18007fc48ULL,0x18007fd78ULL,0x18007fb28ULL,0x18007fcd8ULL};
using Body=std::uintptr_t(*)(std::uintptr_t,std::uintptr_t,s::NativeNodeConstructorContext&);
inline constexpr std::array<Body,5> bodies{s::construct_node_001590,s::construct_node_001610,s::construct_node_001690,s::construct_node_001720,s::construct_node_0017b0};
struct Allocator:s::PoolAllocationDispatch {
    struct Call{std::uintptr_t target,pool;};std::vector<Call> calls;
    std::function<std::uintptr_t(const Call&)> hook;
    std::uintptr_t pool_allocate(std::uintptr_t t,std::uintptr_t p)override{calls.push_back({t,p});if(!hook)throw std::runtime_error("missing allocator fixture");return hook(calls.back());}
};
struct Append:s::PoolAppendEffects {
    s::ReceiveBufferMemory& memory;std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    std::function<std::uintptr_t(std::uint64_t)> allocate_hook;std::function<void()> copy_hook;
    explicit Append(s::ReceiveBufferMemory& m):memory(m){}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{allocations.push_back(n);if(!allocate_hook)throw std::runtime_error("unexpected append growth");return allocate_hook(n);}
    void copy_078d30(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{if(copy_hook)copy_hook();auto to=memory.view(a,n),from=memory.view(b,n);std::memmove(to.data(),from.data(),static_cast<std::size_t>(n));}
    void free_04b0f8(std::uintptr_t p)override{frees.push_back(p);}
};
struct Fixture {
    static constexpr std::uintptr_t doc=0x210000000ULL,node=0x220000000ULL,pool=0x230000000ULL,table=0x240000000ULL,grown=0x250000000ULL,target=0xf010;
    std::array<std::uint8_t,0x308> document{};std::array<std::uint8_t,0x90> item{};
    std::array<std::uint8_t,0x80> pool_bytes{};std::array<std::uint8_t,0x18> vtable{};std::array<std::uint8_t,128> growth{};
    std::vector<s::ReceiveBufferRegion> regions,extras;std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};Allocator allocator;Append append{memory};s::NativeNodeConstructorContext context{memory,allocator,append};
    std::uintptr_t allocation=node;
    Fixture(){document.fill(0);item.fill(0xa5);pool_bytes.fill(0x6b);put(document,0xa8,doc+0xb0);put(document,0x100,10,4);put(document,0x104,0,4);put(pool_bytes,0,table);put(vtable,0x10,target);allocator.hook=[&](const auto&){return allocation;};refresh();}
    void add(std::uintptr_t a,std::span<std::uint8_t> b){if(!omitted||omitted->first>=a+b.size()||omitted->first+omitted->second<=a){regions.push_back({a,b});return;}auto start=std::max(a,omitted->first),end=std::min(a+b.size(),omitted->first+omitted->second);if(start>a)regions.push_back({a,b.first(start-a)});if(end<a+b.size())regions.push_back({end,b.subspan(end-a)});}
    void refresh(){regions.clear();add(doc,document);add(node,item);add(pool,pool_bytes);add(table,vtable);add(grown,growth);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t p,unsigned n=8){return word(memory.view(p,n),0,n);}
    void set(std::uintptr_t p,std::uint64_t v,unsigned n=8){put(memory.view(p,n),0,v,n);}
    std::uintptr_t construct(unsigned k){return bodies[k](doc,pool,context);}
};
inline std::vector<std::uint8_t> image(std::span<const std::uint8_t> initial,unsigned kind,std::uintptr_t document,std::uintptr_t pool){
    std::vector<std::uint8_t> b(initial.begin(),initial.end());put(b,8,document);put(b,0x10,0);put(b,0x18,0,4);put(b,0x20,0);put(b,0x28,0);put(b,0x30,0,4);for(unsigned at:{0x38U,0x40U,0x48U,0x50U,0x58U})put(b,at,0);if(kind==2){put(b,0x68,0,4);put(b,0x70,0);}put(b,0,tables[kind]);if(kind==3)put(b,0x68,0,1);put(b,0x60,pool);return b;
}
} // namespace node_constructors_test
