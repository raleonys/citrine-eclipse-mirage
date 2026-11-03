// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_release_test_fixture.hpp"
#include <type_traits>
using namespace pool_test;
namespace {
using G=Graph;
static_assert(!std::is_copy_constructible_v<s::PoolReleaseEffects>);
static_assert(!std::is_copy_assignable_v<s::PoolReleaseEffects>);
static_assert(!std::is_move_constructible_v<s::PoolReleaseEffects>);
static_assert(!std::is_move_assignable_v<s::PoolReleaseEffects>);

// Literal slot bytes from retained node-delete-next-vtables.json, complete
// read192 at 0x18007fe08, ev_7349a6f3b8db519dad314c7de8d04be1e89ad0c44e1b592d75731e5c20e44706.
// These anchors establish the default identities independently of the fixture.
constexpr std::array<std::uintptr_t,4> observed_tables{
    0x18007fe08ULL,0x18007fe38ULL,0x18007fe68ULL,0x18007fe98ULL};
constexpr std::array<std::uint8_t,16> observed_slots{
    0xa0,0x33,0x00,0x80,0x01,0,0,0,0x20,0x5a,0x00,0x80,0x01,0,0,0};
std::uint64_t word(std::span<const std::uint8_t> bytes,std::size_t at,unsigned width=8){
    std::uint64_t value=0;for(unsigned i=0;i<width;++i)value|=std::uint64_t(bytes[at+i])<<(8*i);return value;
}
void put(std::span<std::uint8_t> bytes,std::size_t at,std::uint64_t value,unsigned width=8){
    for(unsigned i=0;i<width;++i)bytes[at+i]=static_cast<std::uint8_t>(value>>(8*i));
}
void quiet(Fixture& f){
    f.nodes.hook=[](const std::string&){throw std::runtime_error("unexpected node callback in pool body");};
    f.docs.lower.hook=[](const std::string&){throw std::runtime_error("unexpected document/heap provider in pool body");};
}
void no_effects(const Fixture& f){
    check(!f.nodes.probes&&!f.nodes.destroys&&!f.nodes.releases&&!f.nodes.tracks&&!f.nodes.parents&&!f.nodes.deletes&&f.nodes.calls.empty()&&f.nodes.trace.empty(),"pool body invokes no virtual/node helper provider");
    check(f.docs.lower.trace.empty()&&f.docs.lower.allocations.empty()&&f.docs.lower.freed.empty()&&!f.docs.lower.parses&&!f.docs.lower.clears&&!f.docs.lower.destroys&&!f.docs.lower.node_clears&&!f.state.live(),"pool body invokes no supplied allocator/free/document provider");
}
struct Snapshot {
    decltype(Pools::bytes) pools;decltype(G::node_bytes) nodes;
    decltype(G::table_bytes) tables;decltype(G::pooltable_bytes) pooltables;
    decltype(G::tracking_bytes) tracking;decltype(G::pool_bytes) old_pools;
    std::array<std::uint8_t,0x308> document;
    explicit Snapshot(const Fixture& f):pools(f.pools.bytes),nodes(f.graph.node_bytes),tables(f.graph.table_bytes),pooltables(f.graph.pooltable_bytes),tracking(f.graph.tracking_bytes),old_pools(f.graph.pool_bytes),document(f.state.snapshot()){}
    void matches(const Fixture& f)const{
        check(pools==f.pools.bytes&&nodes==f.graph.node_bytes,"exact full pool/item bytes match completed store prefix");
        check(tables==f.graph.table_bytes&&pooltables==f.graph.pooltable_bytes&&tracking==f.graph.tracking_bytes&&old_pools==f.graph.pool_bytes&&document==f.state.snapshot(),"unrelated node tables, old pool projection, tracking and complete document bytes preserved");
    }
};
template<class F>void unsupported(F&& call){bool caught=false;try{call();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"bounded unsupported outcome at selected memory/dispatch boundary");}

void null_and_bindings(){
    const s::PoolTargetIdentities defaults;
    check(defaults.release==word(observed_slots,0)&&defaults.track==word(observed_slots,8),"default identities match retained shipped slot+18/+20 bytes");
    for(unsigned i=0;i<observed_tables.size();++i){
        check(observed_tables[i]==0x18007fe08ULL+i*0x30,"four observed MemPoolT table anchors retain 0x30 stride");
        Fixture f;quiet(f);check(f.graph.get(G::pooltable(i)+0x18)==word(observed_slots,0)&&f.graph.get(G::pooltable(i)+0x20)==word(observed_slots,8),"each controlled pool uses independently anchored release/track identities");
        const Snapshot expected(f);f.state.regions.fill({});
        s::release_pool_item_0033a0(G::pool(i),0,f.state.memory);
        f.nodes.pool_release(defaults.release,G::pool(i),0);
        s::release_pool_item_0033a0(0,0,f.state.memory);
        expected.matches(f);no_effects(f);
    }
}
void lifo_and_widths(){
    for(unsigned pool=0;pool<4;++pool){
        Fixture f;quiet(f);f.pools.bytes[pool].fill(0xa6);f.pools.set(pool,0x68,G::node(5));f.pools.set(pool,0x70,3,4);
        for(unsigned n:{0U,2U,3U})f.graph.node_bytes[n].fill(0xd7);
        Snapshot expected(f);
        // Independent expected chain: 3 -> 0 -> 2 -> old root 5. Every byte
        // outside three first words, root QWORD and current-count DWORD survives.
        put(expected.nodes[2],0,G::node(5));put(expected.nodes[0],0,G::node(2));put(expected.nodes[3],0,G::node());
        put(expected.pools[pool],0x68,G::node(3));put(expected.pools[pool],0x70,0,4);
        f.release(G::pool(pool),G::node(2));f.release(G::pool(pool),G::node());f.release(G::pool(pool),G::node(3));
        check(f.pools.get(pool,0x68)==G::node(3)&&f.graph.get(G::node(3))==G::node()&&f.graph.get(G::node())==G::node(2)&&f.graph.get(G::node(2))==G::node(5),"three releases prepend exact LIFO chain without traversing or freeing old root");
        expected.matches(f);no_effects(f);
    }
    constexpr std::array<std::uint32_t,5> counts{0,1,0x7fffffffU,0x80000000U,0xffffffffU};
    constexpr std::array<std::uint32_t,5> decremented{0xffffffffU,0,0x7ffffffeU,0x7fffffffU,0xfffffffeU};
    for(unsigned pool=0;pool<4;++pool)for(unsigned j=0;j<counts.size();++j){
        Fixture f;quiet(f);f.pools.bytes[pool].fill(0xbc);f.pools.set(pool,0x68,0xfedcba9876543210ULL);f.pools.set(pool,0x70,counts[j],4);f.pools.set(pool,0x7c,0x88776655U,4);
        Snapshot expected(f);put(expected.pools[pool],0x68,G::node());put(expected.pools[pool],0x70,decremented[j],4);put(expected.nodes[0],0,0xfedcba9876543210ULL);
        f.release(G::pool(pool),G::node());
        check(f.pools.get(pool,0x70,4)==decremented[j]&&f.pools.get(pool,0x7c,4)==0x88776655U,"release raw32 zero/one/signed-boundary/maximum decrement leaves tracking count intact");
        expected.matches(f);no_effects(f);
    }
}
void tracking_only(){
    constexpr std::array<std::uint32_t,5> counts{0,1,0x7fffffffU,0x80000000U,0xffffffffU};
    constexpr std::array<std::uint32_t,5> decremented{0xffffffffU,0,0x7ffffffeU,0x7fffffffU,0xfffffffeU};
    for(unsigned pool=0;pool<4;++pool)for(unsigned j=0;j<counts.size();++j){
        Fixture f;quiet(f);f.pools.bytes[pool].fill(0x9e);f.pools.set(pool,0x7c,counts[j],4);Snapshot expected(f);put(expected.pools[pool],0x7c,decremented[j],4);
        f.track(G::pool(pool));check(f.pools.get(pool,0x7c,4)==decremented[j],"track raw32 wraps at zero and preserves signed bit transitions");expected.matches(f);no_effects(f);
    }
    {Fixture f;quiet(f);f.state.regions.fill({});f.state.regions[499]={G::pool()+0x7c,std::span(f.pools.bytes[0]).subspan(0x7c,4)};put(f.pools.bytes[0],0x7c,1,4);Snapshot expected(f);put(expected.pools[0],0x7c,0,4);f.track(G::pool());expected.matches(f);no_effects(f);}
    {Fixture f;quiet(f);f.state.regions.fill({});Snapshot expected(f);unsupported([&]{f.track(G::pool());});expected.matches(f);no_effects(f);}
    {Fixture f;quiet(f);f.pools.omitted={{0x7f,1}};f.pools.refresh();Snapshot expected(f);unsupported([&]{f.track(G::pool());});expected.matches(f);no_effects(f);}
}
void alias_order(){
    // Distinct pool root/count guest addresses share the low root DWORD.
    // This distinguishes MOV root before DEC from a root reload after DEC.
    {Fixture f;quiet(f);auto bytes=std::span(f.pools.bytes[0]);put(bytes,0x68,0x1122334400000009ULL);f.state.regions[467]={};
        f.state.regions[499]={G::pool(),bytes.first(0x68)};f.state.regions[500]={G::pool()+0x68,bytes.subspan(0x68,8)};
        f.state.regions[501]={G::pool()+0x70,bytes.subspan(0x68,4)};f.state.regions[502]={G::pool()+0x74,bytes.subspan(0x74)};
        Snapshot expected(f);put(expected.nodes[0],0,0x1122334400000009ULL);put(expected.pools[0],0x68,G::node());
        f.release(G::pool(),G::node());check(f.graph.get(G::node())==0x1122334400000009ULL,"item receives root cached before aliased count DEC");
        check(f.pools.get(0,0x70,4)==0x10000000U,"final root publication also reaches shared count backing");expected.matches(f);no_effects(f);
    }
    for(unsigned at:{0U,0x68U,0x6cU,0x70U}){
        Fixture f;quiet(f);f.pools.bytes[0].fill(0xc3);f.pools.set(0,0x68,0x1122334455667788ULL);f.pools.set(0,0x70,9,4);Snapshot expected(f);const auto item=G::pool()+at;
        // Fixed final byte anchors for each overlap, derived from the retained
        // MOV/DEC/MOV/MOV sequence, rather than a second implementation loop.
        if(at==0){put(expected.pools[0],0,0x1122334455667788ULL);put(expected.pools[0],0x70,8,4);}
        if(at==0x68)put(expected.pools[0],0x70,8,4);
        if(at==0x6c)put(expected.pools[0],0x70,0x11223344U,4);
        if(at==0x70)put(expected.pools[0],0x70,0x1122334455667788ULL);
        put(expected.pools[0],0x68,item);f.release(G::pool(),item);
        check(f.pools.get(0,0x68)==item,"root publication follows aliased item-first-word store");expected.matches(f);no_effects(f);
    }
}
void bounded_prefixes(){
    for(unsigned stage=0;stage<5;++stage){
        Fixture f;quiet(f);f.pools.set(0,0x68,0x8877665544332211ULL);f.pools.set(0,0x70,7,4);Snapshot expected(f);
        if(stage==0)f.pools.omitted={{0x68,8}};
        if(stage==1)f.pools.omitted={{0x70,4}};
        if(stage==2)f.pools.omitted={{0x73,1}};
        if(stage<3)f.pools.refresh();
        if(stage==3)f.state.regions[450]={};
        if(stage==4){f.state.regions[450]={};f.state.regions[499]={G::node(),std::span(f.graph.node_bytes[0]).first(7)};}
        if(stage>=3)put(expected.pools[0],0x70,6,4);
        unsupported([&]{f.release(G::pool(),G::node());});expected.matches(f);no_effects(f);
    }
    // Ambiguity is likewise a bounded projection failure, not native EH/fault
    // evidence. Duplicate root/count/item windows expose exact prior stores.
    for(unsigned stage=0;stage<3;++stage){
        Fixture f;quiet(f);f.pools.set(0,0x68,0x8877665544332211ULL);f.pools.set(0,0x70,7,4);Snapshot expected(f);
        if(stage==0)f.state.regions[499]={G::pool()+0x68,std::span(f.pools.bytes[0]).subspan(0x68,8)};
        if(stage==1)f.state.regions[499]={G::pool()+0x70,std::span(f.pools.bytes[0]).subspan(0x70,4)};
        if(stage==2){f.state.regions[499]={G::node(),std::span(f.graph.node_bytes[0]).first(8)};put(expected.pools[0],0x70,6,4);}
        unsupported([&]{f.release(G::pool(),G::node());});expected.matches(f);no_effects(f);
    }
    // The final root store uses exactly the span read at entry. With static
    // descriptors and pure memory operations its mapping cannot disappear
    // independently; do not fabricate a callback to claim that prefix tested.
    {Fixture f;quiet(f);f.state.regions.fill({});auto bytes=std::span(f.pools.bytes[0]);f.state.regions[499]={0x68,bytes.subspan(0x68,8)};f.state.regions[500]={0x70,bytes.subspan(0x70,4)};f.state.regions[501]={G::node(),std::span(f.graph.node_bytes[0]).first(8)};
        put(bytes,0x68,0);put(bytes,0x70,1,4);Snapshot expected(f);put(expected.pools[0],0x68,G::node());put(expected.pools[0],0x70,0,4);put(expected.nodes[0],0,0);
        f.release(0,G::node());expected.matches(f);no_effects(f);
    }
}
void dispatch_boundaries(){
    for(bool null_item:{false,true})for(std::uintptr_t target:{0ULL,0x1800033a1ULL,0x180005a20ULL}){
        Fixture f;quiet(f);const Snapshot expected(f);if(null_item)f.state.regions.fill({});unsupported([&]{f.nodes.pool_release(target,G::pool(),null_item?0:G::node());});expected.matches(f);no_effects(f);
    }
    for(std::uintptr_t target:{0ULL,0x180005a21ULL,0x1800033a0ULL}){Fixture f;quiet(f);const Snapshot expected(f);f.state.regions.fill({});unsupported([&]{f.nodes.pool_track(target,G::pool());});expected.matches(f);no_effects(f);}
    {Fixture f;quiet(f);Nodes selected(f.graph,f.pools,{0xab01,0xab02});f.pools.set(0,0x68,0x8877665544332211ULL);f.pools.set(0,0x70,1,4);f.pools.set(0,0x7c,1,4);Snapshot expected(f);
        unsupported([&]{selected.pool_release(0x1800033a0ULL,G::pool(),G::node());});unsupported([&]{selected.pool_release(0xab02,G::pool(),0);});unsupported([&]{selected.pool_track(0x180005a20ULL,G::pool());});expected.matches(f);
        selected.pool_release(0xab01,G::pool(),G::node());selected.pool_track(0xab02,G::pool());put(expected.nodes[0],0,0x8877665544332211ULL);put(expected.pools[0],0x68,G::node());put(expected.pools[0],0x70,0,4);put(expected.pools[0],0x7c,0,4);expected.matches(f);
        check(selected.calls.empty()&&selected.trace.empty()&&!selected.releases&&!selected.tracks&&!selected.probes&&!selected.destroys&&!selected.parents&&!selected.deletes,"explicit supplied target identities select memory bodies without lower stubs");no_effects(f);
    }
}
}
int main(){try{null_and_bindings();lifo_and_widths();tracking_only();alias_order();bounded_prefixes();dispatch_boundaries();std::cout<<"PASS: "<<checks<<" pool release/track checks; retained vtable defaults, four pools, exact LIFO/full-prefix preservation, raw32 wrapping, root-cache/shared-backing/item aliases, bounded prefixes and selected identities; native allocator/free, ABI and EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
