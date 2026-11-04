// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_clear_test_fixture.hpp"
using namespace pool_clear_test;
namespace {
using B=Backing;
void zero_and_lifo(){
    for(unsigned variant=0;variant<3;++variant){
        {Fixture f;f.sentinel();f.raw.omitted={{B::pool_address+8,8}};f.raw.refresh();auto expected=f.raw.pool;reset_image(expected);f.direct(variant);check(f.raw.pool==expected&&f.docs.entered_frees.empty()&&!f.nodes.calls&&!f.docs.other_calls,"zero raw count skips missing data pointer and performs only three QWORD resets");}
        {Fixture f;f.sentinel();f.raw.count(4,10);constexpr std::array<std::uintptr_t,4> entries{0x1111,0,0x3333,0x4444};for(unsigned i=0;i<4;++i)put(f.raw.pool,0x10+i*8,entries[i]);auto expected=f.raw.pool;put(expected,0x64,0,4);reset_image(expected);const auto heaps=f.raw.heaps;const auto blocks=f.raw.blocks;
            f.docs.before_free=[&](auto){check(word(f.raw.pool,0x64,4)==4-f.docs.entered_frees.size(),"DWORD count decrement is committed before each mandatory free");};f.direct(variant);
            check(f.docs.completed_frees==std::vector<std::uintptr_t>{0x4444,0x3333,0,0x1111},"LIFO block order includes null entry without invented guard");check(f.raw.pool==expected&&f.raw.heaps==heaps&&f.raw.blocks==blocks,"clear preserves vptr,data,capacity,stale inline entries and all unrelated bytes; no array free");}
    }
}
void live_mutations_and_negative(){
    for(unsigned variant=0;variant<3;++variant){
        {Fixture f;f.raw.count(2,7);put(f.raw.pool,8,B::heap_address());put(f.raw.heaps[0],8,0xb002);put(f.raw.heaps[1],0,0xe005);put(f.raw.heaps[1],8,0xd004);put(f.raw.heaps[1],16,0xc003);f.docs.after_free=[&](auto){if(f.docs.completed_frees.size()==1){f.raw.count(3,7);put(f.raw.pool,8,B::heap_address(1));}else if(f.docs.completed_frees.size()==2)f.raw.count(1,7);};f.direct(variant);
            check(f.docs.completed_frees==std::vector<std::uintptr_t>{0xb002,0xc003,0xe005},"loop tests live post-free count and reloads changed pointer/index on each iteration");check(word(f.raw.pool,8)==B::heap_address(1)&&word(f.raw.pool,0x60,4)==7&&!word(f.raw.pool,0x64,4),"lower pointer/capacity mutations remain and no final count force-write is invented");}
        struct Negative{std::uint32_t old,next;std::uint64_t displacement;};constexpr Negative values[]{{0xffffffff,0xfffffffe,0xfffffffffffffff0ULL},{0x80000000,0x7fffffff,0xfffffffbfffffff8ULL}};
        for(auto c:values){Fixture f;f.raw.count(c.old,10);put(f.raw.pool,8,B::heap_address());const auto slot=B::heap_address()+c.displacement;put(f.raw.heaps[2],0,0xfefe);f.raw.extras.push_back({slot,std::span(f.raw.heaps[2]).first(8)});f.raw.refresh();f.docs.before_free=[&](auto){check(word(f.raw.pool,0x64,4)==c.next,"negative nonzero old count decrements raw32 before indexed free");};f.docs.after_free=[&](auto){put(f.raw.pool,0x64,0,4);};f.direct(variant);check(f.docs.completed_frees==std::vector<std::uintptr_t>{0xfefe},"signed negative old count selects bounded negative slot and terminates only via supplied mutation");}
        {Fixture f;f.raw.count(0xffffffff,10);put(f.raw.pool,8,B::heap_address());const auto before=f.raw.pool;unsupported([&]{f.direct(variant);});auto expected=before;put(expected,0x64,0xfffffffe,4);check(f.raw.pool==expected&&f.docs.entered_frees.empty(),"negative count with missing negative slot fails after decrement, not treated as empty pool");}
    }
}
void alias_order(){
    for(unsigned variant=0;variant<3;++variant){
        {Fixture f;f.raw.count(2,10);put(f.raw.pool,0x68,0);put(f.raw.heaps[2],0,0xaabb);f.raw.regions={{B::pool_address,std::span(f.raw.pool).first(8)},{B::pool_address+8,std::span(f.raw.pool).subspan(0x64,8)},{B::pool_address+0x10,std::span(f.raw.pool).subspan(0x10)},{9,std::span(f.raw.heaps[2]).first(8)}};f.raw.memory.regions=f.raw.regions;
            f.docs.after_free=[&](auto){put(f.raw.pool,0x64,0,4);};f.direct(variant);check(f.docs.completed_frees==std::vector<std::uintptr_t>{0xaabb},"decrement changes aliased data pointer from2to1; slot uses live pointer1 and cached old index2");}
        {Fixture f;f.raw.count(2,10);put(f.raw.pool,8,B::heap_address());put(f.raw.pool,0x68,0xabcddcba);f.raw.regions={{B::pool_address,f.raw.pool},{B::heap_address()+8,std::span(f.raw.pool).subspan(0x64,8)}};f.raw.memory.regions=f.raw.regions;f.docs.after_free=[&](auto){put(f.raw.pool,0x64,0,4);};f.direct(variant);
            check(f.docs.completed_frees==std::vector<std::uintptr_t>{0xabcddcba00000001ULL},"decrement updates backing of selected slot before block pointer read");}
    }
}
void failure_prefixes(){
    for(unsigned variant=0;variant<3;++variant){
        for(unsigned at:{0x64U,8U,0x10U}){Fixture f;f.sentinel();f.raw.count(1,10);const auto before=f.raw.pool;f.raw.omitted={{B::pool_address+at,at==0x64?4U:8U}};f.raw.refresh();unsupported([&]{f.direct(variant);});auto expected=before;if(at!=0x64)put(expected,0x64,0,4);check(f.raw.pool==expected&&f.docs.entered_frees.empty(),"missing count/data/slot retains exact pre-free decrement prefix without resets");}
        {Fixture f;f.sentinel();f.raw.count(1,10);const auto before=f.raw.pool;f.raw.extras.push_back({B::inline_address,std::span(f.raw.heaps[3]).first(8)});f.raw.refresh();unsupported([&]{f.direct(variant);});auto expected=before;put(expected,0x64,0,4);check(f.raw.pool==expected&&f.docs.entered_frees.empty(),"ambiguous slot fails after decrement and before free/reset");}
        for(bool after:{false,true}){Fixture f;f.sentinel();f.raw.count(2,10);put(f.raw.pool,0x10,0x1111);put(f.raw.pool,0x18,0x2222);auto expected=f.raw.pool;put(expected,0x64,0,4);const auto hook=[&](auto){if(f.docs.entered_frees.size()==2)throw Failure{77};};if(after)f.docs.after_free=hook;else f.docs.before_free=hook;failure([&]{f.direct(variant);});check(f.raw.pool==expected&&f.docs.entered_frees==std::vector<std::uintptr_t>{0x2222,0x1111}&&f.docs.completed_frees.size()==(after?2U:1U),"second lower failure preserves two decrements, completed free prefix and original reset fields");}
        {Fixture f;f.sentinel();f.raw.count(1,10);put(f.raw.pool,0x10,0x4545);auto expected=f.raw.pool;put(expected,0x64,0,4);f.docs.after_free=[&](auto){f.raw.omitted={{B::pool_address+0x64,4}};f.raw.refresh();};unsupported([&]{f.direct(variant);});check(f.raw.pool==expected&&f.docs.completed_frees==std::vector<std::uintptr_t>{0x4545},"live loop test after completed free can fail before any reset");}
        for(unsigned stop:{0x68U,0x70U,0x78U}){Fixture f;f.sentinel();auto expected=f.raw.pool;for(unsigned at:{0x68U,0x70U,0x78U})if(at<stop)put(expected,at,0);f.raw.omitted={{B::pool_address+stop,8}};f.raw.refresh();unsupported([&]{f.direct(variant);});check(f.raw.pool==expected&&f.docs.entered_frees.empty(),"missing QWORD reset retains preceding zero stores and later original fields");}
        {Fixture f;f.sentinel();const auto expected=f.raw.pool;f.raw.regions={{B::pool_address,std::span(f.raw.pool).first(0x68)},{B::pool_address+0x68,std::span(f.raw.pool).subspan(0x68,4)},{B::pool_address+0x6c,std::span(f.raw.pool).subspan(0x6c)}};f.raw.memory.regions=f.raw.regions;unsupported([&]{f.direct(variant);});check(f.raw.pool==expected,"two adjacent DWORD mappings cannot satisfy first native QWORD reset access");}
    }
}
}
int main(){try{zero_and_lifo();live_mutations_and_negative();alias_order();failure_prefixes();std::cout<<"PASS: "<<checks<<" pool clear core checks; three bodies, LIFO/raw negative counts, live free mutations, decrement aliases, QWORD reset order and bounded failure prefixes; lower free/ABI/EH remain supplied\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
