// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_allocate_test_fixture.hpp"
using namespace pool_allocate_test;
namespace {
using F=Fixture;
void fresh_images(){
    const s::PoolAllocationTargetIdentities identities;const s::PoolAllocationData data;
    for(unsigned k=0;k<4;++k)check(identities.allocate[k]==types[k].target,"four default allocation identities match retained shipped slots");
    check(data.cpu_mode==0x1800a1550ULL&&data.stride_constants==std::array<std::uintptr_t,4>{0x180080100ULL,0x1800800d0ULL,0x1800800f0ULL,0x1800800e0ULL}&&data.increment_constants==0x180080110ULL&&data.index_constants==0x180080120ULL,"default data anchors match retained instruction references");
    for(unsigned k=0;k<4;++k)for(std::uint32_t cpu:{0U,5U,6U,7U,0x7fffffffU,0xffffffffU,0x80000000U}){
        F f(k,cpu);auto expected_pool=f.pool;const auto constants=f.constants;const auto mode=f.mode;const auto other=f.blocks[1];
        put(expected_pool,0x10,F::block_address());put(expected_pool,0x64,1,4);put(expected_pool,0x68,F::block_address()+types[k].stride);for(unsigned at:{0x70U,0x74U,0x78U,0x7cU})put(expected_pool,at,1,4);
        check(f.allocate()==F::block_address(),"fresh allocation returns cached block, not its successor");
        check(f.pool==expected_pool,"entire pool prefix includes block record, root and exact four counters only");
        check(f.blocks[0]==free_image(types[k],F::block_address()),"entire zero-filled block has exact stride links and final null");
        check(f.blocks[1]==other&&f.blocks[2]==other&&f.blocks[3]==other&&f.constants==constants&&f.mode==mode,"other blocks and read-only analysis data unchanged");call_shape(f);
    }
}
void recycled_and_counters(){
    struct Case {std::uint32_t current,maximum,next,expected_max;};
    constexpr Case cases[]{{0,0,1,1},{5,4,6,6},{5,6,6,6},{0x7fffffff,0,0x80000000,0},{0xffffffff,0xffffffff,0,0},{0xfffffffe,0xffffffff,0xffffffff,0xffffffff},{0x80000000,0x80000000,0x80000001,0x80000001},{0x7ffffffe,0x80000000,0x7fffffff,0x7fffffff}};
    for(unsigned k=0;k<4;++k)for(auto c:cases){
        F f(k);const auto item=F::block_address()+types[k].stride*2;f.recycled(item,0x123456789abcdef0ULL);put(f.pool,0x70,c.current,4);put(f.pool,0x74,0xffffffff,4);put(f.pool,0x78,c.maximum,4);put(f.pool,0x7c,0xffffffff,4);
        auto expected=f.pool;const auto blocks=f.blocks;put(expected,0x68,0x123456789abcdef0ULL);put(expected,0x70,c.next,4);put(expected,0x74,0,4);put(expected,0x78,c.expected_max,4);put(expected,0x7c,0,4);
        // Recycled path cannot resolve CPU mode or SIMD tables at all.
        f.regions.resize(5);f.memory.regions=f.regions;f.effects.before=[](const auto&){throw std::runtime_error("unexpected lower effect");};
        check(f.allocate()==item,"recycled item identity returned unchanged");check(f.pool==expected,"signed maximum and raw32 current/total/untracked wrap match explicit edge cases");check(f.blocks==blocks&&f.effects.calls.empty(),"recycle preserves payload/link and bypasses all lower effects and data globals");
    }
}
void effect_prefixes(){
    for(unsigned k=0;k<4;++k)for(const std::string stage:{"allocate","fill","append"}){
        F f(k,6);const auto pool=f.pool;const auto blocks=f.blocks;
        f.effects.before=[&](const auto& name){if(name==stage)throw Failure{77};};
        bool caught=false;try{(void)f.allocate();}catch(const Failure& e){caught=e.code==77;}check(caught,"mandatory lower effect failure propagates");
        check(f.pool==pool,"failed effect before append completion leaves pool unchanged");
        check(f.blocks[0]==(stage=="append"?std::vector<std::uint8_t>(types[k].bytes,0):blocks[0]),"only completed fill remains after next effect throws");
        check(f.effects.calls.size()==(stage=="allocate"?1U:stage=="fill"?2U:3U),"callback failure stops at exact reached effect");
    }
    for(unsigned k=0;k<4;++k){
        F f(k);f.forced_allocation=0;const auto blocks=f.blocks;unsupported([&]{(void)f.allocate();});
        check(f.effects.calls.size()==2&&f.effects.calls[0].kind=="allocate"&&f.effects.calls[1].kind=="append"&&f.effects.calls[1].b==0,"null allocation skips fill but still appends null");
        check(word(f.pool,0x10)==0&&word(f.pool,0x64,4)==1&&!word(f.pool,0x68)&&!word(f.pool,0x70,4)&&f.blocks==blocks,"null block fails at unmapped first link after committed append; no invented null return");
        F g(k,0);g.effects.after=[&](const auto& name){if(name=="append"){put(g.mode,0,6,4);g.omitted={{F::constants_address,96}};g.refresh();}};
        unsupported([&]{(void)g.allocate();});check(g.effects.calls.size()==3&&word(g.pool,0x64,4)==1&&g.blocks[0]==std::vector<std::uint8_t>(types[k].bytes,0),"CPU mode is read after append; missing selected vector constants fails before link stores");
    }
}
void bounded_prefixes(){
    for(unsigned k=0;k<4;++k){
        for(unsigned at:{0x68U,0x70U,0x78U,0x74U,0x7cU}){
            F f(k);f.recycled(F::block_address(),0x1020304050607080ULL);auto expected=f.pool;
            f.omitted={{F::pool_address+at,at==0x68?8U:4U}};f.refresh();
            if(at!=0x68)put(expected,0x68,0x1020304050607080ULL);
            if(at==0x78||at==0x74||at==0x7c)put(expected,0x70,1,4);
            if(at==0x74||at==0x7c)put(expected,0x78,1,4);
            if(at==0x7c)put(expected,0x74,1,4);
            unsupported([&]{(void)f.allocate();});check(f.pool==expected&&f.effects.calls.empty(),"missing recycled field preserves exact committed pop/counter prefix");
        }
        {F f(k);f.recycled();f.omitted={{F::block_address(),8}};f.refresh();const auto pool=f.pool;unsupported([&]{(void)f.allocate();});check(f.pool==pool,"missing item link stops before root publication");}
        {F f(k);f.recycled();f.extras.push_back({F::pool_address+0x70,std::span(f.pool).subspan(0x70,4)});f.refresh();auto expected=f.pool;put(expected,0x68,0);unsupported([&]{(void)f.allocate();});check(f.pool==expected,"ambiguous current cell fails after pop without fabricated rollback");}
        {F f(k,6);f.omitted={{f.context.data.increment_constants+8,8}};f.refresh();unsupported([&]{(void)f.allocate();});check(f.blocks[0]==std::vector<std::uint8_t>(types[k].bytes,0)&&word(f.pool,0x64,4)==1,"SIMD data loads require complete16bytes including lanes not later consumed");}
        {F f(k,6);f.omitted={{F::block_address()+types[k].stride*2,8}};f.refresh();f.effects.before=[&](const auto& name){if(name=="fill")f.omitted.reset(),f.refresh();};f.effects.after=[&](const auto& name){if(name=="append"){f.omitted={{F::block_address()+types[k].stride*2,8}};f.refresh();}};
            unsupported([&]{(void)f.allocate();});auto image=std::vector<std::uint8_t>(types[k].bytes,0);put(image,0,F::block_address()+types[k].stride);put(image,types[k].stride,F::block_address()+2*types[k].stride);
            check(f.blocks[0]==image&&!word(f.pool,0x68)&&word(f.pool,0x64,4)==1,"third SIMD link failure retains first two sequential QWORD stores before root publication");}
    }
}
void alias_and_data(){
    for(unsigned k=0;k<4;++k){
        {F f(k);f.recycled(F::pool_address+0x70,0x12300000005ULL);put(f.pool,0x78,0,4);const auto item=F::pool_address+0x70;check(f.allocate()==item,"cached recycled item may alias counter fields");check(word(f.pool,0x68)==0x12300000005ULL&&word(f.pool,0x70,4)==6&&word(f.pool,0x74,4)==0x124&&word(f.pool,0x78,4)==6,"item link read occurs before live current and total mutations");}
        {F f(k);f.recycled();put(f.pool,0x74,0xffffffff,4);f.regions={{F::pool_address,std::span(f.pool).first(0x74)},{F::pool_address+0x74,std::span(f.pool).subspan(0x74,4)},{F::pool_address+0x78,std::span(f.pool).subspan(0x74,4)},{F::pool_address+0x7c,std::span(f.pool).subspan(0x70,4)},{F::block_address(),f.blocks[0]}};f.memory.regions=f.regions;
            check(f.allocate()==F::block_address(),"shared-backing aliases remain mapped uniquely per address");check(f.get(F::pool_address+0x70,4)==2&&f.get(F::pool_address+0x74,4)==2&&f.get(F::pool_address+0x78,4)==2&&f.get(F::pool_address+0x7c,4)==2,"max write affects live total, then live untracked read sees current alias");}
        {F f(k,6);const auto selected=f.context.data.stride_constants[k];f.omitted={{selected,16}};f.extras.push_back({selected,std::span(f.blocks[0]).first(16)});f.refresh();f.effects.after=[&](const auto& name){if(name=="append"){put(f.blocks[0],0,types[k].stride);put(f.blocks[0],8,types[k].stride);}};
            check(f.allocate()==F::block_address(),"vector constants may share backing with link cells in bounded projection");auto expected=free_image(types[k],F::block_address());put(expected,8,types[k].stride);check(f.blocks[0]==expected,"stride lanes are cached before first link overwrites aliased constant backing");}
        {F f(k,6);const auto at=static_cast<std::size_t>(f.context.data.stride_constants[k]-F::constants_address);put(f.constants,at,2);put(f.constants,at+8,3);put(f.constants,64,0xffffffff,4);put(f.constants,68,0x80000000,4);put(f.constants,80,0,4);put(f.constants,84,0,4);
            check(f.allocate()==F::block_address(),"custom raw SIMD lanes preserve cached returned block");check(word(f.blocks[0],0)==F::block_address()-std::uint64_t{2}&&word(f.blocks[0],types[k].stride)==F::block_address()-std::uint64_t{6442450944}&&word(f.blocks[0],types[k].stride*2)==F::block_address()+2&&word(f.blocks[0],types[k].stride*3)==F::block_address()-std::uint64_t{6442450938},"PADDD raw32 wrapping, PMOVSXDQ sign extension and lane-specific64bit multiply/add");}
    }
}
void dispatch_boundaries(){
    for(auto target:{std::uintptr_t{0},std::uintptr_t{0x180002b11ULL},std::uintptr_t{0x1800033a0ULL}}){F f;f.memory.regions={};s::PoolAllocationEffectsAdapter dispatch{f.context};unsupported([&]{(void)dispatch.pool_allocate(target,0);});check(f.effects.calls.empty(),"unknown source target rejected before any memory/effect access");}
    for(unsigned k=0;k<4;++k){F f(k);s::PoolAllocationTargetIdentities ids;ids.allocate={0xa001,0xa002,0xa003,0xa004};s::PoolAllocationEffectsAdapter dispatch{f.context,ids};unsupported([&]{(void)dispatch.pool_allocate(types[k].target,F::pool_address);});check(f.effects.calls.empty(),"supplied identities replace default dispatch bindings");check(dispatch.pool_allocate(ids.allocate[k],F::pool_address)==F::block_address()&&f.blocks[0]==free_image(types[k],F::block_address()),"selected custom identity reaches correct stride body");}
}
}
int main(){try{fresh_images();recycled_and_counters();effect_prefixes();bounded_prefixes();alias_and_data();dispatch_boundaries();std::cout<<"PASS: "<<checks<<" pool allocation core checks; four literal layouts, scalar/vector initialized images, recycled bypass, signed maxima/raw counters, ordered lower/mapping failures, shared aliases and raw SIMD lanes; native heap/append body/ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
