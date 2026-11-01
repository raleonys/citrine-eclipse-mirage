// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/xml/pool_clear.hpp"
#include "tests/support/socket/pool_append_test_fixture.hpp"

namespace pool_clear_test {
namespace s=preloader_l::socket;
using pool_append_test::check;using pool_append_test::checks;using pool_append_test::word;using pool_append_test::put;using pool_append_test::Failure;
using pool_append_test::unsupported;using pool_append_test::failure;using pool_append_test::types;
using Backing=pool_append_test::Fixture;
struct Nodes:s::NativeNodeClearEffects {
    unsigned calls=0;
    [[noreturn]]void unexpected(){++calls;throw std::runtime_error("unexpected node provider in empty-tree pool test");}
    std::uint64_t node_probe(std::uintptr_t,std::uintptr_t)override{unexpected();}
    void node_destroy(std::uintptr_t,std::uintptr_t,std::uint32_t)override{unexpected();}
    void pool_release(std::uintptr_t,std::uintptr_t,std::uintptr_t)override{unexpected();}
    void pool_track(std::uintptr_t,std::uintptr_t)override{unexpected();}
    void remove_parent_003030(std::uintptr_t,std::uintptr_t)override{unexpected();}
    void delete_node_003240(std::uintptr_t)override{unexpected();}
};
struct Docs:s::PoolClearEffects {
    std::vector<std::uintptr_t> entered_frees,completed_frees;unsigned other_calls=0;
    std::function<void(std::uintptr_t)> before_free,after_free;
    std::function<std::uintptr_t(std::uint64_t)> allocator;
    std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copier;
    std::function<void(std::uintptr_t)> parser;
    std::function<void(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t)> error_setter;
    Docs(s::ReceiveBufferMemory& memory,Nodes& nodes):PoolClearEffects(memory,nodes){}
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected document provider in pool clear test");}
    void set_error_005820(std::uintptr_t p,std::int32_t c,std::uint32_t n,std::uintptr_t d)override{if(error_setter)error_setter(p,c,n,d);else unexpected();}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{if(allocator)return allocator(n);unexpected();}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{if(copier)copier(to,from,n);else unexpected();}
    void parse_0041b0(std::uintptr_t p)override{if(parser)parser(p);else unexpected();}
    void destroy_pool_001c30(std::uintptr_t)override{unexpected();}
    void destroy_pool_001cb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001bb0(std::uintptr_t)override{unexpected();}
    void destroy_pool_001d30(std::uintptr_t)override{unexpected();}
    void free_04b0f8(std::uintptr_t p)override{entered_frees.push_back(p);if(before_free)before_free(p);completed_frees.push_back(p);if(after_free)after_free(p);}
    void destroy_node_base_001ea0(std::uintptr_t)override{unexpected();}
    void clear_type(unsigned k,std::uintptr_t p){if(k<2)clear_pool_002d70(p);else if(k==2)clear_pool_002e10(p);else clear_pool_002dc0(p);}
};
struct Fixture {
    Backing raw;Nodes nodes;Docs docs; s::NativePoolClearContext context;
    explicit Fixture(unsigned type=0,std::uint32_t cpu=0):raw(type,cpu),docs(raw.memory,nodes),context{raw.memory,docs}{}
    void direct(unsigned variant){if(variant==0)s::clear_pool_blocks_002d70(Backing::pool_address,context);else if(variant==1)s::clear_pool_blocks_002dc0(Backing::pool_address,context);else s::clear_pool_blocks_002e10(Backing::pool_address,context);}
    void clear(){docs.clear_type(raw.type,Backing::pool_address);}
    void sentinel(){put(raw.pool,0x68,0x1122334455667788ULL);put(raw.pool,0x70,0x99aabbccddeeff00ULL);put(raw.pool,0x78,0x123456789abcdef0ULL);}
};
inline void reset_image(std::span<std::uint8_t> bytes){for(unsigned at:{0x68U,0x70U,0x78U})put(bytes,at,0);}
} // namespace pool_clear_test
