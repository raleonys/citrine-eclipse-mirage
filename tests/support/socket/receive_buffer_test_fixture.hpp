// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_receive_buffer.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace buffer_test {
namespace s=preloader_l::socket;
inline unsigned checks=0;
inline void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Terminal:std::runtime_error {unsigned kind;explicit Terminal(unsigned n):std::runtime_error("synthetic buffer terminal"),kind(n){}};
template<class E,class F>void rejects(F f){bool caught=false;try{f();}catch(const E&){caught=true;}check(caught,"expected specific failure propagates");}
inline void word(std::span<std::uint8_t> bytes,std::uint64_t value){for(unsigned i=0;i<8;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));}
inline std::uint64_t word(std::span<const std::uint8_t> bytes){std::uint64_t value=0;for(unsigned i=0;i<8;++i)value|=std::uint64_t{bytes[i]}<<(i*8);return value;}
struct Block {std::uintptr_t raw=0;std::vector<std::uint8_t> bytes;std::size_t region=0;};
struct Fixture;
struct OwnedApi:s::BoundedReceiveBufferApi {
    Fixture& f;std::vector<std::uint64_t> allocations;std::vector<std::pair<std::uintptr_t,std::uint64_t>> frees;
    bool null_allocate=false;std::function<void()> on_allocate,on_free;
    explicit OwnedApi(Fixture&,s::ReceiveBufferMemory&);
    std::uintptr_t allocate_04b0bc(std::uint64_t)override;
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
    [[noreturn]]void length_error_00ce80()override{throw Terminal(1);}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Terminal(2);}
    [[noreturn]]void invalid_parameter_05c974()override{throw Terminal(3);}
};
struct Fixture {
    s::ReceiveBufferString value;std::array<s::ReceiveBufferRegion,32> regions{};
    s::ReceiveBufferMemory memory{regions};OwnedApi api{*this,memory};s::ReceiveBufferContext context{memory,api};
    std::vector<std::unique_ptr<Block>> blocks;std::size_t next_region=1;std::uintptr_t next_raw=0x10003;
    Fixture(){regions[0]={reinterpret_cast<std::uintptr_t>(value.storage_00.data()),value.storage_00};}
    Block& add(std::uint64_t count){
        if(count>1024*1024)throw std::bad_alloc();
        auto p=std::make_unique<Block>();p->raw=next_raw;next_raw+=0x20000;p->bytes.resize(static_cast<std::size_t>(count),0xcc);p->region=next_region++;
        if(p->region>=regions.size())throw std::runtime_error("fixture region capacity");
        regions[p->region]={p->raw,p->bytes};blocks.push_back(std::move(p));return *blocks.back();
    }
    void seed(std::uint64_t capacity,std::uint64_t size){
        value.capacity_18=capacity;value.size_10=size;value.storage_00.fill(0xcc);
        if(capacity>15){
            const auto bytes=capacity+1;auto& b=add(bytes+(bytes>=4096?39:0));
            const auto data=bytes>=4096?(b.raw+39)&~std::uintptr_t{31}:b.raw;
            if(bytes>=4096)word(memory.view(data-8,8),b.raw);
            word(value.storage_00,data);
        }
        auto data=memory.view(s::receive_buffer_address(value),size+1);
        for(std::size_t i=0;i<size;++i)data[i]=static_cast<std::uint8_t>('a'+i%23);
        data[static_cast<std::size_t>(size)]=0;
    }
    std::string text(){auto b=memory.view(s::receive_buffer_address(value),value.size_10);return {reinterpret_cast<const char*>(b.data()),b.size()};}
    std::size_t live_blocks()const{std::size_t n=0;for(const auto& b:blocks)n+=bool(b);return n;}
};
inline OwnedApi::OwnedApi(Fixture& fixture,s::ReceiveBufferMemory& memory):BoundedReceiveBufferApi(memory),f(fixture){}
inline std::uintptr_t OwnedApi::allocate_04b0bc(std::uint64_t count){allocations.push_back(count);if(on_allocate)on_allocate();return null_allocate?0:f.add(count).raw;}
inline void OwnedApi::free_sized_04b0f8(std::uintptr_t address,std::uint64_t count){
    frees.emplace_back(address,count);if(on_free)on_free();
    for(auto& b:f.blocks)if(b&&b->raw==address){f.regions[b->region]={};b.reset();return;}
    throw std::runtime_error("free outside owned fixture");
}
} // namespace buffer_test
