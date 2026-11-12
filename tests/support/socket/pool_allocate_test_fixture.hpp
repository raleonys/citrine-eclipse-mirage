// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/pool_allocate.hpp"
#include "citrine_eclipse_mirage/xml/pool_release.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace pool_allocate_test {
namespace s=citrine_eclipse_mirage::socket;
inline unsigned checks=0;
inline void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
inline std::uint64_t word(std::span<const std::uint8_t> b,std::size_t at,unsigned n=8){std::uint64_t v=0;for(unsigned i=0;i<n;++i)v|=std::uint64_t(b[at+i])<<(8*i);return v;}
inline void put(std::span<std::uint8_t> b,std::size_t at,std::uint64_t v,unsigned n=8){for(unsigned i=0;i<n;++i)b[at+i]=static_cast<std::uint8_t>(v>>(8*i));}
struct Type {std::uintptr_t target;unsigned stride,bytes,count;};
// Literal shipped vtable+10 identities and retained assembly allocation sizes.
inline constexpr std::array<Type,4> types{{{0x180002b10ULL,120,4080,34},{0x180002600ULL,80,4080,51},{0x180002960ULL,112,4032,36},{0x1800027b0ULL,104,4056,39}}};
struct Failure {unsigned code;};
struct Fixture;
struct Effects:s::PoolAllocationEffects {
    struct Call {std::string kind;std::uintptr_t a=0;std::uint64_t b=0,n=0;};
    Fixture& owner;std::vector<Call> calls;std::function<void(const std::string&)> before,after;
    explicit Effects(Fixture& f):owner(f){}
    std::uintptr_t allocate_04b0bc(std::uint64_t)override;
    void fill_0793d0(std::uintptr_t,std::uint8_t,std::uint64_t)override;
    void append_block_005440(std::uintptr_t,std::uintptr_t)override;
    void enter(const std::string& name){if(before)before(name);}
    void leave(const std::string& name){if(after)after(name);}
};
struct Fixture {
    static constexpr std::uintptr_t pool_address=0x310000,mode_address=0x1800a1550ULL,constants_address=0x1800800d0ULL;
    static constexpr std::uintptr_t block_address(unsigned i=0){return 0x710000+i*0x10000;}
    unsigned type=0,next_block=0;std::optional<std::uintptr_t> forced_allocation;
    std::array<std::uint8_t,0x80> pool{};std::array<std::vector<std::uint8_t>,4> blocks;
    std::array<std::uint8_t,4> mode{};std::array<std::uint8_t,96> constants{};
    std::vector<s::ReceiveBufferRegion> regions,extras;
    std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};Effects effects{*this};s::PoolAllocationContext context;
    explicit Fixture(unsigned k=0,std::uint32_t cpu=0):type(k),context{memory,effects}{
        pool.fill(0x5a);put(pool,8,pool_address+0x10);put(pool,0x60,10,4);put(pool,0x64,0,4);put(pool,0x68,0);for(unsigned at:{0x70U,0x74U,0x78U,0x7cU})put(pool,at,0,4);
        for(auto& b:blocks)b.assign(types[type].bytes,0xa5);put(mode,0,cpu,4);
        // Exact initialized analysis-space read96/96 at1800800d0:
        // ev_6174d38308fd71da7aa56dfcf6ae30de443aee787ea60c50ea732bc04bff3b6f.
        for(unsigned i=0;i<4;++i){constexpr unsigned values[]{80,104,112,120};put(constants,i*16,values[i]);put(constants,i*16+8,values[i]);}
        for(unsigned i=0;i<4;++i){put(constants,64+i*4,1,4);put(constants,80+i*4,i,4);}refresh();
    }
    void add(std::uintptr_t address,std::span<std::uint8_t> bytes){
        if(!omitted||omitted->first>=address+bytes.size()||omitted->first+omitted->second<=address){regions.push_back({address,bytes});return;}
        const auto start=std::max(address,omitted->first),end=std::min(address+bytes.size(),omitted->first+omitted->second);
        if(start>address)regions.push_back({address,bytes.first(start-address)});
        if(end<address+bytes.size())regions.push_back({end,bytes.subspan(end-address)});
    }
    void refresh(){regions.clear();add(pool_address,pool);for(unsigned i=0;i<blocks.size();++i)add(block_address(i),blocks[i]);add(mode_address,mode);add(constants_address,constants);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t p,unsigned n=8){return word(memory.view(p,n),0,n);}
    void set(std::uintptr_t p,std::uint64_t v,unsigned n=8){put(memory.view(p,n),0,v,n);}
    std::uintptr_t allocate(){s::PoolAllocationEffectsAdapter dispatch{context};return dispatch.pool_allocate(types[type].target,pool_address);}
    void recycled(std::uintptr_t item=block_address(),std::uint64_t next=0){put(pool,0x68,item);set(item,next);}
};
inline std::uintptr_t Effects::allocate_04b0bc(std::uint64_t n){calls.push_back({"allocate",0,0,n});enter("allocate");const auto result=owner.forced_allocation?*owner.forced_allocation:Fixture::block_address(owner.next_block++);leave("allocate");return result;}
inline void Effects::fill_0793d0(std::uintptr_t p,std::uint8_t value,std::uint64_t n){calls.push_back({"fill",p,value,n});enter("fill");auto bytes=owner.memory.view(p,n);std::fill(bytes.begin(),bytes.end(),value);leave("fill");}
inline void Effects::append_block_005440(std::uintptr_t array,std::uintptr_t p){
    calls.push_back({"append",array,p,0});enter("append");
    // Mandatory supplied lower effect: ONLY the normal inline-capacity case.
    // This fixture is not a recovered005440 body, allocator, heap or DOM.
    const auto count=owner.get(array+0x5c,4),capacity=owner.get(array+0x58,4);
    if(count>=capacity)throw std::runtime_error("fixture inline block list full");
    owner.set(owner.get(array)+count*8,p);owner.set(array+0x5c,count+1,4);leave("append");
}
inline std::vector<std::uint8_t> free_image(const Type& t,std::uintptr_t p){
    std::vector<std::uint8_t> expected(t.bytes,0);for(unsigned i=0;i<t.count;++i)put(expected,i*t.stride,i+1<t.count?p+(i+1)*t.stride:0);return expected;
}
inline void call_shape(const Fixture& f,unsigned count=1){check(f.effects.calls.size()==3*count,"one allocation/fill/append triplet per fresh block");for(unsigned i=0;i<count;++i){const auto* c=&f.effects.calls[i*3];check(c[0].kind=="allocate"&&c[0].n==types[f.type].bytes,"literal block size reaches allocator");check(c[1].kind=="fill"&&c[1].a==Fixture::block_address(i)&&c[1].b==0&&c[1].n==types[f.type].bytes,"allocated block alone zero-filled");check(c[2].kind=="append"&&c[2].a==Fixture::pool_address+8&&c[2].b==Fixture::block_address(i),"block recorded in pool array before link construction");}}
template<class F>inline void unsupported(F&& fn){bool caught=false;try{fn();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"explicit bounded unsupported result");}
} // namespace pool_allocate_test
