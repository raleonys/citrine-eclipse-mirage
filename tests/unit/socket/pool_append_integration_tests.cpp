#include "tests/support/socket/pool_append_test_fixture.hpp"
using namespace pool_append_test;
namespace {
using F=Fixture;
void stamp(F& f,std::uintptr_t item,unsigned id){f.set(item+8,0xface000000000000ULL+id);f.set(item+types[f.type].stride-1,id%251+1,1);}
void stamped(F& f,std::uintptr_t item,unsigned id){check(f.get(item+8)==0xface000000000000ULL+id&&f.get(item+types[f.type].stride-1,1)==id%251+1,"consumer payload stays intact through source track/release/reuse");}
void seven_block_cycle(unsigned k,std::uint32_t cpu){
    F f(k,cpu);f.count(0,1);const auto& t=types[k];const unsigned count=t.count*7;std::vector<std::uintptr_t> items;std::vector<unsigned> released;
    // The supplied free effect removes the old array mapping after successful
    // second growth; later source operations must use the new array alone.
    f.effects.after=[&](const auto& stage){if(stage=="free"){f.omitted={{F::heap_address(),160}};f.refresh();}};
    for(unsigned i=0;i<count;++i){const auto p=f.allocate();check(p==F::block_address(i/t.count)+(i%t.count)*t.stride,"four N18 source bodies allocate seven blocks with exact item extents through recovered005440");items.push_back(p);stamp(f,p,i);
        check(word(f.pool,0x70,4)==i+1&&word(f.pool,0x74,4)==i+1&&word(f.pool,0x78,4)==i+1&&word(f.pool,0x7c,4)==i+1,"normal counters remain coherent across pointer-array storage growth");
    }
    check(f.next_block==7&&f.next_heap==2&&word(f.pool,8)==F::heap_address(1)&&word(f.pool,0x60,4)==10&&word(f.pool,0x64,4)==7&&!word(f.pool,0x68),"controlled inline capacity1 forces two source growths and seven recorded full blocks");
    check(calls_named(f.effects,"array_allocate")==2&&calls_named(f.effects,"copy")==2&&calls_named(f.effects,"free")==1&&f.effects.freed==std::vector<std::uintptr_t>{F::heap_address()},"inline-to-heap skips free; heap-to-heap frees only old array once");
    std::vector<Effects::Call> array_calls;for(const auto& c:f.effects.calls)if(c.name=="array_allocate"||c.name=="copy"||c.name=="free")array_calls.push_back(c);
    check(array_calls.size()==5&&array_calls[0].name=="array_allocate"&&array_calls[0].bytes==32&&array_calls[1].name=="copy"&&array_calls[1].source==F::inline_address&&array_calls[1].bytes==8&&array_calls[2].name=="array_allocate"&&array_calls[2].bytes==80&&array_calls[3].name=="copy"&&array_calls[3].source==F::heap_address()&&array_calls[3].bytes==32&&array_calls[4].name=="free","two actual005440 growth paths retain exact allocation/copy/free sequence and arguments");
    for(unsigned i=0;i<7;++i)check(f.get(F::heap_address(1)+i*8)==F::block_address(i),"current grown array retains each of seven block identities exactly once");
    check(word(f.pool,0x10)==F::block_address()&&word(f.heaps[1],56)==0xc3c3c3c3c3c3c3c3ULL,"old inline first entry retained and uninitialized new capacity untouched");
    for(unsigned i=0;i<count;++i){s::track_pool_005a20(F::pool_address,f.memory);check(word(f.pool,0x7c,4)==count-i-1&&word(f.pool,0x70,4)==count,"N17 tracking decrements only untracked after source allocation");}
    for(unsigned residue=0;residue<5;++residue)for(unsigned i=residue;i<count;i+=5)released.push_back(i);
    std::uintptr_t previous=0;
    for(unsigned i=0;i<count;++i){const auto id=released[i];const auto item=items[id];s::release_pool_item_0033a0(F::pool_address,item,f.memory);check(word(f.pool,0x68)==item&&f.get(item)==previous&&word(f.pool,0x70,4)==count-i-1,"N17 source release links items from all seven blocks in controlled LIFO order");stamped(f,item,id);previous=item;}
    const auto calls=f.effects.calls.size();f.effects.before=[](const auto&){throw std::runtime_error("source reuse unexpectedly reached lower effect");};
    // Drop mode/constants and keep the missing freed-array mapping. Reuse must
    // not consult either the allocator or the retained SIMD/global data.
    f.regions.erase(std::remove_if(f.regions.begin(),f.regions.end(),[](const auto& r){return r.address==F::mode_address||r.address==F::constants_address;}),f.regions.end());f.memory.regions=f.regions;
    for(unsigned i=0;i<count;++i){const auto id=released[count-i-1];const auto p=f.allocate();check(p==items[id],"source allocation reuses released items in reverse order without new array growth");stamped(f,p,id);check(word(f.pool,0x70,4)==i+1&&word(f.pool,0x74,4)==count+i+1&&word(f.pool,0x78,4)==count&&word(f.pool,0x7c,4)==i+1,"recycle preserves highwater and advances total/untracked through recovered bridge stack");}
    check(!word(f.pool,0x68)&&f.effects.calls.size()==calls&&word(f.pool,0x64,4)==7&&word(f.pool,8)==F::heap_address(1),"all seven blocks reused without reaching supplied effects or old freed array");
    for(unsigned i=0;i<count;++i){s::track_pool_005a20(F::pool_address,f.memory);s::release_pool_item_0033a0(F::pool_address,items[i],f.memory);}
    check(!word(f.pool,0x70,4)&&!word(f.pool,0x7c,4)&&word(f.pool,0x74,4)==2*count&&word(f.pool,0x78,4)==count,"completed second cycle balances live counts and preserves cumulative total/highwater");
    std::set<std::uintptr_t> free_items;auto p=word(f.pool,0x68);
    for(unsigned i=0;i<count;++i){check(p&&free_items.insert(p).second,"free list has no duplicate/lost/cyclic item across seven blocks");const auto block=(p-F::block_address())/0x10000,offset=(p-F::block_address())%0x10000;check(block<7&&offset<t.bytes&&offset%t.stride==0,"free-list pointer is a valid literal item start");p=f.get(p);}
    check(!p&&free_items.size()==count,"free list terminates after all seven-block items exactly once");
}
void failed_growth_prefixes(unsigned k,std::uint32_t cpu){
    {F f(k,cpu);f.count(0,1);const auto n=types[k].count;std::vector<std::uintptr_t> items;
        for(unsigned i=0;i<n;++i){items.push_back(f.allocate());s::track_pool_005a20(F::pool_address,f.memory);}
        f.effects.after=[](const auto& stage){if(stage=="copy")throw Failure{77};};failure([&]{(void)f.allocate();});
        check(f.next_block==2&&f.next_heap==1&&word(f.pool,8)==F::inline_address&&word(f.pool,0x60,4)==1&&word(f.pool,0x64,4)==1&&!word(f.pool,0x68)&&word(f.pool,0x70,4)==n&&word(f.pool,0x74,4)==n&&!word(f.pool,0x7c,4),"copy failure after fresh block fill retains allocation effects without publishing new pointer/capacity/block record or counters");
        check(word(f.heaps[0],0)==F::block_address()&&f.blocks[1]==std::vector<std::uint8_t>(types[k].bytes,0)&&f.effects.freed.empty(),"completed copy and fresh-block zero fill remain visible without invented cleanup");
        f.effects.after={};check(f.allocate()==F::block_address(2),"retry requests a distinct supplied block; failed allocation is not silently rolled back or reused");
        check(word(f.pool,8)==F::heap_address(1)&&word(f.pool,0x60,4)==4&&word(f.pool,0x64,4)==2&&f.get(F::heap_address(1))==F::block_address()&&f.get(F::heap_address(1)+8)==F::block_address(2)&&word(f.pool,0x70,4)==n+1,"successful retry records first and third allocated blocks through recovered append body");
        const auto effects=f.effects.calls.size();s::release_pool_item_0033a0(F::pool_address,items[0],f.memory);check(f.allocate()==items[0]&&f.effects.calls.size()==effects,"source release/reuse remains available after bounded append failure and later successful growth");
    }
    {F f(k,cpu);f.count(0,1);const auto n=types[k].count*4;for(unsigned i=0;i<n;++i){(void)f.allocate();s::track_pool_005a20(F::pool_address,f.memory);}f.effects.after=[](const auto& stage){if(stage=="free")throw Failure{77};};failure([&]{(void)f.allocate();});
        check(f.next_block==5&&f.next_heap==2&&f.effects.freed==std::vector<std::uintptr_t>{F::heap_address()}&&word(f.pool,8)==F::heap_address()&&word(f.pool,0x60,4)==4&&word(f.pool,0x64,4)==4&&!word(f.pool,0x68)&&word(f.pool,0x70,4)==n&&word(f.pool,0x74,4)==n&&!word(f.pool,0x7c,4),"free-after-side-effect failure retains old published pointer/capacity plus recorded free; no invented native unwind/rollback");
        for(unsigned i=0;i<4;++i)check(word(f.heaps[1],i*8)==F::block_address(i),"copy to next array completed before failed free prevents publication");
        check(f.blocks[4]==std::vector<std::uint8_t>(types[k].bytes,0),"fifth block fill precedes append failure and has no source free-list initialization");
    }
}
}
int main(){try{for(unsigned k=0;k<4;++k)for(std::uint32_t cpu:{0U,6U}){seven_block_cycle(k,cpu);failed_growth_prefixes(k,cpu);}std::cout<<"PASS: "<<checks<<" pool pointer-array integration checks; four source allocators scalar/SIMD, seven blocks with inline-to-heap/heap-to-heap growth, track/release/reuse and committed failures; lower allocation/fill/copy/free supplied, no parser/DOM/nativeABI/EH claim\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
