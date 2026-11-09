// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/xml/node_insertion.hpp"
#include "tests/support/socket/node_constructors_test_fixture.hpp"

namespace node_insertion_test {
namespace s=citrine_eclipse_mirage::socket;
using node_constructors_test::check;using node_constructors_test::checks;
using node_constructors_test::put;using node_constructors_test::word;
using node_constructors_test::Failure;using node_constructors_test::fails;using node_constructors_test::unmapped;
struct Fixture {
    static constexpr std::uintptr_t doc=0x110000000ULL,child=0x120000000ULL,parent=0x130000000ULL,
        previous=0x140000000ULL,next=0x150000000ULL,pool=0x160000000ULL,table=0x170000000ULL,entries=0x180000000ULL,target=0xf820;
    std::array<std::uint8_t,0x308> document{};
    std::array<std::array<std::uint8_t,0x80>,5> objects{};
    std::array<std::uint8_t,0x28> vtable{};std::array<std::uint8_t,32> array{};
    std::vector<s::ReceiveBufferRegion> regions,extras;
    std::vector<std::pair<std::uintptr_t,std::size_t>> omitted;
    s::ReceiveBufferMemory memory{};node_parse_test::Nodes nodes;
    s::NativeNodeInsertionContext context{memory,nodes};
    Fixture(){refresh();set(child+8,doc);set(child+0x60,pool);set(pool,table);set(table+0x20,target);set(doc+0xa8,entries);}
    void add(std::uintptr_t address,std::span<std::uint8_t> bytes,bool extra=false){
        std::vector<s::ReceiveBufferRegion> chunks{{address,bytes}};
        auto cuts=omitted;if(!extra)for(auto r:extras)cuts.push_back({r.address,r.bytes.size()});
        for(auto [begin,count]:cuts){std::vector<s::ReceiveBufferRegion> out;for(auto r:chunks){const auto end=r.address+r.bytes.size(),cut_end=begin+count;
            if(cut_end<=r.address||begin>=end){out.push_back(r);continue;}
            const auto a=std::max(begin,r.address),b=std::min(cut_end,end);
            if(a>r.address)out.push_back({r.address,r.bytes.first(a-r.address)});
            if(b<end)out.push_back({b,r.bytes.subspan(b-r.address)});
        }chunks=std::move(out);}for(auto r:chunks)regions.push_back(r);
    }
    void refresh(){regions.clear();add(doc,document);const std::array addresses{child,parent,previous,next,pool};for(unsigned i=0;i<addresses.size();++i)add(addresses[i],objects[i]);add(table,vtable);add(entries,array);for(auto r:extras)add(r.address,r.bytes,true);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t a,unsigned n=8){return word(memory.view(a,n),0,n);}
    void set(std::uintptr_t a,std::uint64_t v,unsigned n=8){put(memory.view(a,n),0,v,n);}
    void call(std::uintptr_t c=child,std::uintptr_t owner=0xdeadbeef){s::prepare_native_insertion_003aa0(owner,c,context);}
    void linked(std::uintptr_t prev=0,std::uintptr_t following=0){set(child+0x10,parent);set(child+0x48,prev);set(child+0x50,following);set(parent+0x38,prev?prev:child);set(parent+0x40,following?following:child);if(prev)set(prev+0x50,child);if(following)set(following+0x48,child);}
    void expect_no_destroy(){check(nodes.probes.empty()&&nodes.destroys.empty()&&nodes.releases.empty(),"insertion never probes,destroys or releases a node");}
};
} // namespace node_insertion_test
