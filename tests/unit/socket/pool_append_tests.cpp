// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_append_test_fixture.hpp"
using namespace pool_append_test;
namespace {
using F=Fixture;
constexpr std::uintptr_t value=0xfedcba9876543210ULL;
void inline_and_growth(){
    for(unsigned n:{0U,1U,9U})for(std::uintptr_t block:{std::uintptr_t{0},value}){
        F f;f.count(n,10);auto expected=f.pool;const auto heaps=f.heaps;const auto blocks=f.blocks;put(expected,0x10+n*8,block);put(expected,0x64,n+1,4);f.append(block);
        check(f.pool==expected&&f.heaps==heaps&&f.blocks==blocks,"inline append changes exactly selected pointer slot and live count");check(f.effects.calls.empty(),"inline capacity bypasses all heap/copy/free effects");
    }
    {F f;f.count(2,2);put(f.pool,0x10,0x1111);put(f.pool,0x18,0x2222);auto pool=f.pool;auto heap=f.heaps[0];put(pool,8,F::heap_address());put(pool,0x60,6,4);put(pool,0x64,3,4);put(heap,0,0x1111);put(heap,8,0x2222);put(heap,16,value);f.append(value);
        check(f.pool==pool&&f.heaps[0]==heap,"inline growth copies old items then appends and commits doubled capacity");check(f.effects.calls.size()==2&&f.effects.calls[0].name=="array_allocate"&&f.effects.calls[0].bytes==48&&f.effects.calls[1].name=="copy"&&f.effects.calls[1].destination==F::heap_address()&&f.effects.calls[1].source==F::inline_address&&f.effects.calls[1].bytes==16&&f.effects.freed.empty(),"inline growth uses48bytes, copies16bytes and skips inline free");}
    {F f;f.count(6,6);put(f.pool,8,F::heap_address(1));for(unsigned i=0;i<6;++i)put(f.heaps[1],i*8,0x9000+i);auto pool=f.pool;auto heap=f.heaps[0];std::copy_n(f.heaps[1].begin(),48,heap.begin());put(heap,48,value);put(pool,8,F::heap_address());put(pool,0x60,14,4);put(pool,0x64,7,4);f.append(value);
        check(f.pool==pool&&f.heaps[0]==heap,"heap growth preserves all old values and appends to cached new storage");check(f.effects.calls.size()==3&&f.effects.calls[0].bytes==112&&f.effects.calls[1].bytes==48&&f.effects.freed==std::vector<std::uintptr_t>{F::heap_address(1)},"heap growth frees prior heap identity after copy and before pointer/capacity publication");}
}
void arithmetic_and_null(){
    struct Arithmetic {std::uint32_t count,cap;std::uint64_t bytes,copy;};
    constexpr Arithmetic cases[]{{0,0,16,0},{1,1,32,8},{0x3fffffff,0,0xffffffffffffffffULL,0x1fffffff8ULL},{0x7ffffffe,0,0xffffffffffffffffULL,0x3fffffff0ULL},{0xffffffff,0xffffffff,0,0xfffffffffffffff8ULL},{0xfffffffe,0xfffffffc,0xffffffffffffffffULL,0xfffffffffffffff0ULL},{0x80000000,0x80000000,16,0xfffffffc00000000ULL}};
    for(auto c:cases){F f;f.count(c.count,c.cap);const auto pool=f.pool;f.effects.before=[](const auto& stage){if(stage=="copy")throw Failure{77};};failure([&]{f.append(value);});check(f.effects.calls.size()==2&&f.effects.calls[0].bytes==c.bytes&&f.effects.calls[1].bytes==c.copy,"signed growth test,raw32 doubling,sign extension,MUL saturation and copy shift match explicit arithmetic edges");check(f.pool==pool,"huge symbolic allocation/copy prefix does not fabricate a real allocation or publication");}
    struct NoGrowth {std::uint32_t count,cap,next;std::uint64_t displacement;};
    constexpr NoGrowth no_growth[]{{0x7fffffff,0,0x80000000,0x3fffffff8ULL},{0xfffffffe,0,0xffffffff,0xfffffffffffffff0ULL},{0xffffffff,0,0,0xfffffffffffffff8ULL}};
    for(auto c:no_growth){F f;f.count(c.count,c.cap);put(f.pool,8,F::heap_address());const auto at=F::heap_address()+c.displacement;f.extras.push_back({at,std::span(f.heaps[2]).first(8)});f.refresh();f.append(value);check(f.effects.calls.empty()&&f.get(at)==value&&word(f.pool,0x64,4)==c.next,"signed next<=capacity permits negative/wrapped live indexes without added count guard");}
    {F f;f.count(1,2);put(f.pool,8,0xfffffffffffffff8ULL);f.extras.push_back({0,std::span(f.heaps[2]).first(8)});f.refresh();f.append(value);check(word(f.heaps[2],0)==value&&word(f.pool,0x64,4)==2&&f.effects.calls.empty(),"tail slot address wraps64 into an explicitly bounded zero-address mapping");}
    {F f;f.count(1,1);put(f.pool,0x10,0x11223344);f.forced_heap=0;f.extras.push_back({0,f.heaps[2]});f.refresh();auto expected=f.heaps[2];put(expected,0,0x11223344);put(expected,8,value);f.append(value);check(!word(f.pool,8)&&word(f.pool,0x60,4)==4&&word(f.pool,0x64,4)==2&&f.heaps[2]==expected,"null returned pointer is used through copy/publication/append when address0 is explicitly mapped");check(f.effects.freed.empty()&&f.effects.calls.size()==2,"null allocation gets no invented guard or inline free");}
}
void live_mutations_and_aliases(){
    {F f;f.count(1,1);put(f.pool,0x10,0xaaaa);put(f.heaps[2],0,0xbbbb);put(f.heaps[2],8,0xcccc);
        f.effects.after=[&](const auto& stage){if(stage=="array_allocate"){put(f.pool,8,F::heap_address(2));f.count(2,0xdeadbeef);}if(stage=="copy")put(f.pool,8,F::heap_address(3));if(stage=="free"){put(f.pool,8,F::heap_address(1));f.count(3,0xffffffff);}};
        f.append(value);check(f.effects.calls.size()==3&&f.effects.calls[0].bytes==32&&f.effects.calls[1].source==F::heap_address(2)&&f.effects.calls[1].bytes==16&&f.effects.freed==std::vector<std::uintptr_t>{F::heap_address(3)},"count/data reread after allocation and free pointer reread after copy");
        check(word(f.pool,8)==F::heap_address()&&word(f.pool,0x60,4)==4&&word(f.pool,0x64,4)==4&&word(f.heaps[0],0)==0xbbbb&&word(f.heaps[0],8)==0xcccc&&word(f.heaps[0],16)==0xc3c3c3c3c3c3c3c3ULL&&word(f.heaps[0],24)==value,"cached new pointer/capacity survive lower mutations; tail index rereads post-free count");}
    {F f;f.count(0,1);put(f.pool,8,F::array_address+0x5c);auto expected=f.pool;put(expected,0x64,0x12345678ffffffffULL);put(expected,0x64,0,4);f.append(0x12345678ffffffffULL);check(f.pool==expected&&f.effects.calls.empty(),"slot aliases count: store replaces DWORD, then live INC wrapsffffffff to0");}
    {F f;f.count(0,1);put(f.pool,8,F::array_address);f.append(value);check(word(f.pool,8)==value&&word(f.pool,0x64,4)==1&&f.effects.calls.empty(),"slot may overwrite data pointer itself; block is cached input and count still increments");}
    {F f;f.count(1,1);put(f.pool,8,F::heap_address(1));put(f.heaps[1],0,0x1234);f.effects.after=[&](const auto& stage){if(stage=="copy")put(f.pool,8,F::inline_address);};f.append(value);check(f.effects.freed.empty()&&word(f.pool,8)==F::heap_address()&&word(f.pool,0x64,4)==2,"copy mutation to inline identity suppresses free using live pointer comparison");}
    {F f;f.count(1,1);put(f.pool,0x10,0x4321);f.effects.after=[&](const auto& stage){if(stage=="copy")put(f.pool,8,0);};f.append(value);check(f.effects.freed==std::vector<std::uintptr_t>{0}&&word(f.pool,8)==F::heap_address(),"copy mutation to null still invokes mandatory free on0; no invented null guard");}
}
void lower_failure_prefixes(){
    for(const std::string stage:{"array_allocate","copy","free"})for(bool after:{false,true}){
        F f;f.count(1,1);put(f.pool,8,F::heap_address(1));put(f.heaps[1],0,0x123456789abcdef0ULL);const auto pool=f.pool;auto expected=f.heaps[0];
        const auto hook=[&](const auto& name){if(name==stage)throw Failure{77};};if(after)f.effects.after=hook;else f.effects.before=hook;
        if(stage=="free"||(stage=="copy"&&after))put(expected,0,0x123456789abcdef0ULL);
        failure([&]{f.append(value);});check(f.pool==pool&&f.heaps[0]==expected,"lower failure preserves old pool pointer/capacity/count and only completed copy side effect");
        check(f.effects.calls.size()==(stage=="array_allocate"?1U:stage=="copy"?2U:3U)&&f.effects.freed.size()==(stage=="free"&&after?1U:0U),"failure reaches exact lower prefix including completed free without rollback");
    }
}
void bounded_prefixes(){
    for(unsigned at:{0x64U,0x60U,8U,0x18U}){F f;f.count(1,2);f.omitted={{F::pool_address+at,at==8||at==0x18?8U:4U}};f.refresh();const auto pool=f.pool;unsupported([&]{f.append(value);});check(f.pool==pool&&f.effects.calls.empty(),"missing initial field or tail slot fails before count increment");}
    {F f;f.count(0,1);f.extras.push_back({F::inline_address,std::span(f.heaps[3]).first(8)});f.refresh();const auto pool=f.pool;unsupported([&]{f.append(value);});check(f.pool==pool,"ambiguous slot fails before write and count increment");}
    for(unsigned at:{8U,0x60U,0x64U}){F f;f.count(1,1);put(f.pool,8,F::heap_address(1));put(f.heaps[1],0,0x4545);auto expected=f.pool;
        f.effects.after=[&](const auto& stage){if(stage=="free"){f.omitted={{F::pool_address+at,at==8?8U:4U}};f.refresh();}};
        if(at!=8)put(expected,8,F::heap_address());if(at==0x64)put(expected,0x60,4,4);
        unsupported([&]{f.append(value);});check(f.pool==expected&&word(f.heaps[0],0)==0x4545&&f.effects.freed==std::vector<std::uintptr_t>{F::heap_address(1)},"post-free missing publication/tail field retains copied/freed prefix and preceding pointer/capacity stores");
    }
    {F f;f.count(1,1);f.effects.after=[&](const auto& stage){if(stage=="array_allocate"){f.omitted={{F::pool_address+0x64,4}};f.refresh();}};const auto pool=f.pool;unsupported([&]{f.append(value);});check(f.pool==pool&&f.effects.calls.size()==1,"count is reread after allocation before copy, bounded failure keeps allocation side effect only");}
    {F f;f.count(1,1);f.effects.after=[&](const auto& stage){if(stage=="copy"){f.extras.push_back({F::array_address,std::span(f.heaps[3]).first(8)});f.refresh();}};const auto pool=f.pool;unsupported([&]{f.append(value);});check(f.pool==pool&&f.effects.calls.size()==2&&f.effects.freed.empty(),"ambiguous live pointer after copy fails before free/publication");}
}
}
int main(){try{inline_and_growth();arithmetic_and_null();live_mutations_and_aliases();lower_failure_prefixes();bounded_prefixes();std::cout<<"PASS: "<<checks<<" pool pointer-array core checks; inline/heap growth, raw signed arithmetic and saturated sizing, live lower-effect mutations, aliases, mapped null and exact failure prefixes; heap/copy/free/ABI/EH remain supplied\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
