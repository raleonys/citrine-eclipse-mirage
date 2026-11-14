// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/node_base.hpp"
#include "tests/support/socket/pool_destruct_test_fixture.hpp"

namespace node_base_test {
namespace s=laureondo::socket;
using pool_destruct_test::check;using pool_destruct_test::checks;using pool_destruct_test::word;using pool_destruct_test::put;using pool_destruct_test::Failure;
using pool_destruct_test::unsupported;using pool_destruct_test::failure;
inline constexpr std::uintptr_t node_table=0x18007fa98ULL;
struct CoreDocs:s::NativeDocumentEffects {
    std::vector<std::uintptr_t> children,entered_frees,completed_frees;unsigned other_calls=0;
    std::function<void(std::uintptr_t)> child_hook,before_free,after_free;
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected provider in node-base core test");}
    void delete_nodes_003110(std::uintptr_t p)override{children.push_back(p);if(child_hook)child_hook(p);}
    void free_04b0f8(std::uintptr_t p)override{entered_frees.push_back(p);if(before_free)before_free(p);completed_frees.push_back(p);if(after_free)after_free(p);}
    void clear_002e60(std::uintptr_t)override{unexpected();}
    void set_error_005820(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t)override{unexpected();}
    std::uintptr_t allocate_04b100(std::uint64_t)override{unexpected();}
    void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)override{unexpected();}
    void parse_0041b0(std::uintptr_t)override{unexpected();}
    void clear_pool_002d70(std::uintptr_t)override{unexpected();}
    void clear_pool_002e10(std::uintptr_t)override{unexpected();}
    void clear_pool_002dc0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001c30(std::uintptr_t)override{unexpected();}
    void destroy_pool_001cb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001bb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001d30(std::uintptr_t)override{unexpected();}
    void destroy_node_base_001ea0(std::uintptr_t)override{unexpected();}
};
struct Memory {
    static constexpr std::uintptr_t node=0x350000,parent=0x360000,previous=0x370000,next=0x380000;
    std::array<std::uint8_t,0x80> bytes{},parent_bytes{},previous_bytes{},next_bytes{};
    std::vector<s::ReceiveBufferRegion> regions,extras;std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;s::ReceiveBufferMemory memory{};
    Memory(){for(auto* b:{&bytes,&parent_bytes,&previous_bytes,&next_bytes})b->fill(0xa5);for(unsigned at:{0x10U,0x20U,0x28U,0x38U,0x40U,0x48U,0x50U})put(bytes,at,0);put(bytes,0,0xf001);put(bytes,0x18,0,4);refresh();}
    void add(std::uintptr_t address,std::span<std::uint8_t> b){if(!omitted||omitted->first>=address+b.size()||omitted->first+omitted->second<=address){regions.push_back({address,b});return;}const auto start=std::max(address,omitted->first),end=std::min(address+b.size(),omitted->first+omitted->second);if(start>address)regions.push_back({address,b.first(start-address)});if(end<address+b.size())regions.push_back({end,b.subspan(end-address)});}
    void refresh(){regions.clear();add(node,bytes);add(parent,parent_bytes);add(previous,previous_bytes);add(next,next_bytes);for(auto r:extras)regions.push_back(r);memory.regions=regions;}
    std::uint64_t get(std::uintptr_t p,unsigned n=8){return word(memory.view(p,n),0,n);}
    void set(std::uintptr_t p,std::uint64_t v,unsigned n=8){put(memory.view(p,n),0,v,n);}
    void linked(){put(bytes,0x10,parent);put(bytes,0x48,previous);put(bytes,0x50,next);put(parent_bytes,0x38,node);put(parent_bytes,0x40,node);put(previous_bytes,0x50,node);put(next_bytes,0x48,node);}
};
struct CoreFixture {
    Memory raw;CoreDocs docs;s::NativeNodeBaseContext context{raw.memory,docs};
    void destroy(){s::destroy_native_node_base_001ea0(Memory::node,context);}
};
struct SourceDocs:s::NodeBaseEffects {
    std::vector<std::uintptr_t> entered_frees,completed_frees;std::function<void(std::uintptr_t)> before_free,after_free;unsigned other_calls=0;
    SourceDocs(s::ReceiveBufferMemory& memory,s::NativeNodeClearEffects& nodes):NodeBaseEffects(memory,nodes){}
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected lower document provider in node-base source integration");}
    void set_error_005820(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t)override{unexpected();}
    std::uintptr_t allocate_04b100(std::uint64_t)override{unexpected();}
    void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)override{unexpected();}
    void parse_0041b0(std::uintptr_t)override{unexpected();}
    void free_04b0f8(std::uintptr_t p)override{entered_frees.push_back(p);if(before_free)before_free(p);completed_frees.push_back(p);if(after_free)after_free(p);}
};
inline void reset_value(std::span<std::uint8_t> bytes){put(bytes,0,node_table);put(bytes,0x18,0,4);put(bytes,0x20,0);put(bytes,0x28,0);}
} // namespace node_base_test
