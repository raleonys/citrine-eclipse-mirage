#pragma once
#include "pool_destruct.hpp"
#include "pool_clear_test_fixture.hpp"

namespace pool_destruct_test {
namespace s=anadius::socket_recovered;
using pool_clear_test::check;using pool_clear_test::checks;using pool_clear_test::word;using pool_clear_test::put;using pool_clear_test::Failure;
using pool_clear_test::unsupported;using pool_clear_test::failure;using pool_clear_test::types;
using Backing=pool_clear_test::Backing;using Nodes=pool_clear_test::Nodes;
inline constexpr std::uintptr_t base_table=0x18007fa18ULL;
// Order follows allocation source types120/80/112/104.
inline constexpr std::array<std::uintptr_t,4> type_tables{0x18007fe08ULL,0x18007fe38ULL,0x18007fe68ULL,0x18007fe98ULL};
struct Docs:s::RecoveredPoolDestructEffects {
    std::vector<std::uintptr_t> entered_frees,completed_frees,node_bases;unsigned other_calls=0;
    std::function<void(std::uintptr_t)> before_free,after_free,node_base;
    Docs(s::ReceiveBufferMemory& memory,Nodes& nodes):RecoveredPoolDestructEffects(memory,nodes){}
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected document provider in pool destructor test");}
    void set_error_005820(std::uintptr_t,std::int32_t,std::uint32_t,std::uintptr_t)override{unexpected();}
    std::uintptr_t allocate_04b100(std::uint64_t)override{unexpected();}
    void copy_078d30(std::uintptr_t,std::uintptr_t,std::uint64_t)override{unexpected();}
    void parse_0041b0(std::uintptr_t)override{unexpected();}
    void free_04b0f8(std::uintptr_t p)override{entered_frees.push_back(p);if(before_free)before_free(p);completed_frees.push_back(p);if(after_free)after_free(p);}
    void destroy_node_base_001ea0(std::uintptr_t p)override{node_bases.push_back(p);if(node_base)node_base(p);else unexpected();}
    void clear_type(unsigned k,std::uintptr_t p){if(k<2)clear_pool_002d70(p);else if(k==2)clear_pool_002e10(p);else clear_pool_002dc0(p);}
    void destroy_type(unsigned k,std::uintptr_t p){if(k==0)destroy_pool_001d30(p);else if(k==1)destroy_pool_001bb0(p);else if(k==2)destroy_pool_001cb0(p);else destroy_pool_001c30(p);}
};
struct Fixture {
    Backing raw;Nodes nodes;Docs docs;s::NativePoolDestructContext context;
    explicit Fixture(unsigned k=0,std::uint32_t cpu=0):raw(k,cpu),docs(raw.memory,nodes),context{raw.memory,docs}{}
    void direct(){switch(raw.type){case 0:s::destroy_native_pool_001d30(Backing::pool_address,context);break;case 1:s::destroy_native_pool_001bb0(Backing::pool_address,context);break;case 2:s::destroy_native_pool_001cb0(Backing::pool_address,context);break;default:s::destroy_native_pool_001c30(Backing::pool_address,context);}}
    void destroy(){docs.destroy_type(raw.type,Backing::pool_address);}
    void clear(){docs.clear_type(raw.type,Backing::pool_address);}
    void sentinel(){put(raw.pool,0x68,0x1122334455667788ULL);put(raw.pool,0x70,0x99aabbccddeeff00ULL);put(raw.pool,0x78,0x123456789abcdef0ULL);}
};
inline void reset_image(std::span<std::uint8_t> bytes){for(unsigned at:{0x68U,0x70U,0x78U})put(bytes,at,0);}
} // namespace pool_destruct_test
