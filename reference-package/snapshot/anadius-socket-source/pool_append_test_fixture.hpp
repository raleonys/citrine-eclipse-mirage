#pragma once
#include "pool_append.hpp"
#include "pool_release.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace pool_append_test {
namespace s=anadius::socket_recovered;
inline unsigned checks=0;
inline void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
inline std::uint64_t word(std::span<const std::uint8_t> b,std::size_t at,unsigned n=8){std::uint64_t v=0;for(unsigned i=0;i<n;++i)v|=std::uint64_t(b[at+i])<<(i*8);return v;}
inline void put(std::span<std::uint8_t> b,std::size_t at,std::uint64_t v,unsigned n=8){for(unsigned i=0;i<n;++i)b[at+i]=static_cast<std::uint8_t>(v>>(i*8));}
struct Type {std::uintptr_t target;unsigned stride,bytes,count;};
inline constexpr std::array<Type,4> types{{{0x180002b10ULL,120,4080,34},{0x180002600ULL,80,4080,51},{0x180002960ULL,112,4032,36},{0x1800027b0ULL,104,4056,39}}};
struct Failure {unsigned code;};
struct Fixture;
struct Effects:s::RecoveredPoolAppendEffects {
    struct Call {std::string name;std::uintptr_t destination=0,source=0;std::uint64_t bytes=0;};
    Fixture& f;std::vector<Call> calls;std::vector<std::uintptr_t> freed;
    std::function<void(const std::string&)> before,after;bool symbolic_copy=false;
    explicit Effects(Fixture&,s::ReceiveBufferMemory&);
    std::uintptr_t allocate_04b0bc(std::uint64_t)override;
    void fill_0793d0(std::uintptr_t,std::uint8_t,std::uint64_t)override;
    std::uintptr_t allocate_04b100(std::uint64_t)override;
    void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)override;
    void free_04b0f8(std::uintptr_t)override;
    void enter(const std::string& n){if(before)before(n);}
    void leave(const std::string& n){if(after)after(n);}
};
struct Fixture {
    static constexpr std::uintptr_t pool_address=0x320000,array_address=pool_address+8,inline_address=array_address+8;
    static constexpr std::uintptr_t heap_address(unsigned i=0){return 0x520000+i*0x10000;}
    static constexpr std::uintptr_t block_address(unsigned i=0){return 0x720000+i*0x10000;}
    static constexpr std::uintptr_t mode_address=0x1800a1550ULL,constants_address=0x1800800d0ULL;
    unsigned type=0,next_heap=0,next_block=0;std::optional<std::uintptr_t> forced_heap;
    std::array<std::uint8_t,0x80> pool{};
    std::array<std::array<std::uint8_t,160>,4> heaps{};
    std::array<std::vector<std::uint8_t>,8> blocks;
    std::array<std::uint8_t,4> mode{};std::array<std::uint8_t,96> constants{};
    std::vector<s::ReceiveBufferRegion> regions,extras;
    std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};Effects effects; s::PoolAppendContext append_context; s::PoolAllocationContext allocation_context;
    explicit Fixture(unsigned k=0,std::uint32_t cpu=0):type(k),effects(*this,memory),append_context{memory,effects},allocation_context{memory,effects}{
        pool.fill(0x5a);put(pool,8,inline_address);put(pool,0x60,10,4);put(pool,0x64,0,4);put(pool,0x68,0);for(unsigned at:{0x70U,0x74U,0x78U,0x7cU})put(pool,at,0,4);
        for(auto& h:heaps)h.fill(0xc3);for(auto& b:blocks)b.assign(types[k].bytes,0xa5);put(mode,0,cpu,4);
        // Exact N18 read96/96, ev_6174d38308fd71da7aa56dfcf6ae30de443aee787ea60c50ea732bc04bff3b6f.
        constexpr unsigned strides[]{80,104,112,120};for(unsigned i=0;i<4;++i){put(constants,16*i,strides[i]);put(constants,16*i+8,strides[i]);put(constants,64+4*i,1,4);put(constants,80+4*i,i,4);}refresh();
    }
    void add(std::uintptr_t address,std::span<std::uint8_t> bytes){
        if(!omitted||omitted->first>=address+bytes.size()||omitted->first+omitted->second<=address){regions.push_back({address,bytes});return;}
        const auto start=std::max(address,omitted->first),end=std::min(address+bytes.size(),omitted->first+omitted->second);
        if(start>address)regions.push_back({address,bytes.first(start-address)});if(end<address+bytes.size())regions.push_back({end,bytes.subspan(end-address)});
    }
    void refresh(){regions.clear();add(pool_address,pool);for(unsigned i=0;i<heaps.size();++i)add(heap_address(i),heaps[i]);for(unsigned i=0;i<blocks.size();++i)add(block_address(i),blocks[i]);add(mode_address,mode);add(constants_address,constants);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t p,unsigned n=8){return word(memory.view(p,n),0,n);}
    void set(std::uintptr_t p,std::uint64_t v,unsigned n=8){put(memory.view(p,n),0,v,n);}
    void count(std::uint32_t n,std::uint32_t capacity){put(pool,0x64,n,4);put(pool,0x60,capacity,4);}
    void append(std::uintptr_t p){s::append_pool_block_005440(array_address,p,append_context);}
    std::uintptr_t allocate(){s::RecoveredPoolAllocationEffects dispatch{allocation_context};return dispatch.pool_allocate(types[type].target,pool_address);}
};
inline Effects::Effects(Fixture& owner,s::ReceiveBufferMemory& memory):RecoveredPoolAppendEffects(memory),f(owner){}
inline std::uintptr_t Effects::allocate_04b0bc(std::uint64_t n){calls.push_back({"block_allocate",0,0,n});enter("block_allocate");check(f.next_block<f.blocks.size()&&n==types[f.type].bytes,"supplied block allocator admits exact bounded layout size");const auto p=Fixture::block_address(f.next_block++);leave("block_allocate");return p;}
inline void Effects::fill_0793d0(std::uintptr_t p,std::uint8_t value,std::uint64_t n){calls.push_back({"fill",p,value,n});enter("fill");auto b=f.memory.view(p,n);std::fill(b.begin(),b.end(),value);leave("fill");}
inline std::uintptr_t Effects::allocate_04b100(std::uint64_t n){calls.push_back({"array_allocate",0,0,n});enter("array_allocate");const auto p=f.forced_heap?*f.forced_heap:Fixture::heap_address(f.next_heap++);leave("array_allocate");return p;}
inline void Effects::copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n){calls.push_back({"copy",to,from,n});enter("copy");if(!symbolic_copy&&n){const auto destination=f.memory.view(to,n),source=f.memory.view(from,n);std::memmove(destination.data(),source.data(),static_cast<std::size_t>(n));}leave("copy");}
inline void Effects::free_04b0f8(std::uintptr_t p){calls.push_back({"free",p,0,0});enter("free");freed.push_back(p);leave("free");}
template<class F>inline void unsupported(F&& fn){bool caught=false;try{fn();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"bounded unsupported mapping result");}
template<class F>inline void failure(F&& fn,unsigned code=77){bool caught=false;try{fn();}catch(const Failure& e){caught=e.code==code;}check(caught,"supplied lower failure propagates without rollback");}
inline unsigned calls_named(const Effects& e,const std::string& name){return static_cast<unsigned>(std::count_if(e.calls.begin(),e.calls.end(),[&](const auto& c){return c.name==name;}));}
} // namespace pool_append_test
