// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_allocate_test_fixture.hpp"
#include <set>
using namespace pool_allocate_test;
namespace {
using F=Fixture;
void payload(F& f,std::uintptr_t item,unsigned id){f.set(item+8,0xfedc000000000000ULL+id);f.set(item+types[f.type].stride-1,(id%251)+1,1);}
void payload_matches(F& f,std::uintptr_t item,unsigned id){check(f.get(item+8)==0xfedc000000000000ULL+id&&f.get(item+types[f.type].stride-1,1)==(id%251)+1,"consumer payload outside free-list firstword survives release and reuse");}
void cycle(unsigned k,std::uint32_t mode){
    F f(k,mode);const auto& t=types[k];const unsigned count=t.count*2+3;std::vector<std::uintptr_t> items;std::vector<unsigned> release_order;
    for(unsigned i=0;i<count;++i){
        const auto item=f.allocate();check(item==F::block_address(i/t.count)+(i%t.count)*t.stride,"source allocation walks exact first/second/third block item extents");items.push_back(item);payload(f,item,i);
        check(word(f.pool,0x70,4)==i+1&&word(f.pool,0x74,4)==i+1&&word(f.pool,0x78,4)==i+1&&word(f.pool,0x7c,4)==i+1,"allocation counters advance across fresh-block and recycled-root paths");
    }
    check(f.next_block==3&&word(f.pool,0x64,4)==3&&word(f.pool,0x68)==F::block_address(2)+3*t.stride,"exactly three lower allocations/records and correct remaining third-block root");call_shape(f,3);
    for(unsigned i=0;i<3;++i)check(word(f.pool,0x10+i*8)==F::block_address(i),"supplied pointer-array normal effect retains every allocated block once");
    for(unsigned i=0;i<count;++i){s::track_pool_005a20(F::pool_address,f.memory);check(word(f.pool,0x7c,4)==count-i-1&&word(f.pool,0x70,4)==count,"real tracking body changes only untracked counter");}
    for(unsigned residue=0;residue<3;++residue)for(unsigned i=residue;i<count;i+=3)release_order.push_back(i);
    const auto unused_tail=word(f.pool,0x68);auto previous=unused_tail;
    for(unsigned n=0;n<count;++n){const auto id=release_order[n];const auto item=items[id];s::release_pool_item_0033a0(F::pool_address,item,f.memory);
        check(word(f.pool,0x68)==item&&f.get(item)==previous&&word(f.pool,0x70,4)==count-n-1,"source release pushes allocated item on LIFO list and decrements current");payload_matches(f,item,id);previous=item;
        check(word(f.pool,0x74,4)==count&&word(f.pool,0x78,4)==count&&!word(f.pool,0x7c,4),"release preserves cumulative total/highwater and already-balanced tracking");
    }
    const auto calls=f.effects.calls.size();const auto records=word(f.pool,0x64,4);
    // Hide every data global after initialization. All subsequent allocations
    // must use the recycled list and cannot reach a lower callback.
    f.regions.resize(5);f.memory.regions=f.regions;f.effects.before=[](const auto&){throw std::runtime_error("recycled integration unexpectedly requests lower effect");};
    for(unsigned n=0;n<count;++n){const auto id=release_order[count-n-1];const auto item=f.allocate();
        check(item==items[id],"source allocation after source release returns reverse release order");payload_matches(f,item,id);
        check(word(f.pool,0x70,4)==n+1&&word(f.pool,0x74,4)==count+n+1&&word(f.pool,0x78,4)==count&&word(f.pool,0x7c,4)==n+1,"reuse increments total/untracked without changing historical maximum");
    }
    check(word(f.pool,0x68)==unused_tail&&f.effects.calls.size()==calls&&word(f.pool,0x64,4)==records,"reused sequence rejoins untouched unused tail without allocation/fill/append/global reads");
    for(unsigned i=0;i<count;++i){s::track_pool_005a20(F::pool_address,f.memory);s::release_pool_item_0033a0(F::pool_address,items[i],f.memory);}
    check(!word(f.pool,0x70,4)&&!word(f.pool,0x7c,4)&&word(f.pool,0x74,4)==2*count&&word(f.pool,0x78,4)==count,"second tracking/release cycle balances current and untracked while preserving totals");
    std::set<std::uintptr_t> free_items;auto cursor=word(f.pool,0x68);
    for(unsigned i=0;i<t.count*3;++i){check(cursor!=0&&free_items.insert(cursor).second,"three complete blocks form an acyclic free-list with no lost or duplicate item");const auto block=(cursor-F::block_address())/0x10000,index=(cursor-F::block_address())%0x10000;check(block<3&&index<t.bytes&&index%t.stride==0,"every free-list identity is a valid item start of its recorded block");cursor=f.get(cursor);}
    check(!cursor&&free_items.size()==t.count*3,"final list terminates after all allocated and never-used items exactly once");
    for(unsigned i=0;i<count;++i)payload_matches(f,items[i],i);
}
void committed_failed_growth(unsigned k){
    F f(k,6);const auto count=types[k].count;std::vector<std::uintptr_t> items;
    for(unsigned i=0;i<count;++i){items.push_back(f.allocate());s::track_pool_005a20(F::pool_address,f.memory);}
    check(!word(f.pool,0x68)&&word(f.pool,0x70,4)==count&&!word(f.pool,0x7c,4),"first block exhausted with live tracked items");
    f.effects.after=[&](const auto& stage){if(stage=="append"){f.omitted={{f.context.data.index_constants,16}};f.refresh();}};
    unsupported([&]{(void)f.allocate();});
    check(word(f.pool,0x64,4)==2&&word(f.pool,0x18)==F::block_address(1)&&word(f.pool,0x70,4)==count&&word(f.pool,0x74,4)==count&&!word(f.pool,0x68),"failed new-block constants load preserves appended ownership and prior live counters, no fake rollback");
    check(f.blocks[1]==std::vector<std::uint8_t>(types[k].bytes,0),"second block was filled before constants failure but list links were not initialized");
    const auto old_calls=f.effects.calls.size();s::release_pool_item_0033a0(F::pool_address,items[0],f.memory);check(f.allocate()==items[0],"prior live item can be released and recycled after failed fresh-block initialization");
    check(f.effects.calls.size()==old_calls&&word(f.pool,0x70,4)==count&&word(f.pool,0x74,4)==count+1&&word(f.pool,0x7c,4)==1,"recycling remains independent of the missing SIMD global after failure");
}
}
int main(){try{for(unsigned k=0;k<4;++k){cycle(k,0);cycle(k,6);committed_failed_growth(k);}std::cout<<"PASS: "<<checks<<" pool allocation integration checks; four exact item extents across three blocks, source tracking/release/LIFO reuse, payload retention and committed failed-growth prefix; lower heap/fill/append supplied, no parser/DOM/nativeABI/EH claim\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
