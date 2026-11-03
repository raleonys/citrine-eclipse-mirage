// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/pool_clear_test_fixture.hpp"
#include <cstring>
using namespace pool_clear_test;
namespace {
using B=Backing;
void balanced_items(Fixture& f,unsigned blocks,unsigned first_block){
    auto& b=f.raw;const auto& t=types[b.type];const unsigned count=blocks*t.count;std::vector<std::uintptr_t> items;
    for(unsigned i=0;i<count;++i){const auto p=b.allocate();check(p==B::block_address(first_block+i/t.count)+(i%t.count)*t.stride,"allocation after clear uses supplied fresh blocks with exact four item extents");items.push_back(p);b.set(p+8,0xface000000000000ULL+i);b.set(p+t.stride-1,i%251+1,1);s::track_pool_005a20(B::pool_address,b.memory);check(word(b.pool,0x70,4)==i+1&&word(b.pool,0x74,4)==i+1&&word(b.pool,0x78,4)==i+1&&!word(b.pool,0x7c,4),"source allocation and tracking keep coherent current/total/maximum counts");}
    std::uintptr_t previous=0;for(unsigned i=0;i<count;++i){s::release_pool_item_0033a0(B::pool_address,items[i],b.memory);check(word(b.pool,0x68)==items[i]&&b.get(items[i])==previous&&word(b.pool,0x70,4)==count-i-1,"source release preserves LIFO across allocated blocks before clear");check(b.get(items[i]+8)==0xface000000000000ULL+i&&b.get(items[i]+t.stride-1,1)==i%251+1,"release preserves owned consumer payload outside firstword");previous=items[i];}
    const auto effects=b.effects.calls.size();for(unsigned i=0;i<count;++i){check(b.allocate()==items[count-i-1],"source allocation reuses exact reverse release order before lifecycle clear");s::track_pool_005a20(B::pool_address,b.memory);}
    check(!word(b.pool,0x68)&&word(b.pool,0x70,4)==count&&word(b.pool,0x74,4)==count*2&&word(b.pool,0x78,4)==count&&!word(b.pool,0x7c,4)&&b.effects.calls.size()==effects,"recycle roundtrip preserves block records and bypasses append/heap effects");
    for(unsigned i=0;i<count;++i)s::release_pool_item_0033a0(B::pool_address,items[i],b.memory);
    check(!word(b.pool,0x70,4)&&!word(b.pool,0x7c,4),"released/tracked items leave no live pool count before clear");
}
void clear_and_regrow(unsigned k,std::uint32_t cpu){
    Fixture f(k,cpu);auto& b=f.raw;b.count(0,1);balanced_items(f,3,0);check(word(b.pool,8)==B::heap_address()&&word(b.pool,0x60,4)==4&&word(b.pool,0x64,4)==3,"three blocks force first pointer-array source growth");
    const auto old_array=b.heaps[0];auto expected=b.pool;put(expected,0x64,0,4);reset_image(expected);const auto allocations=b.effects.calls.size();f.clear();
    check(f.docs.completed_frees==std::vector<std::uintptr_t>{B::block_address(2),B::block_address(1),B::block_address()}&&b.pool==expected&&b.heaps[0]==old_array,"new final clear bridge frees blocks LIFO and retains array allocation/capacity/stale entries");
    check(b.effects.calls.size()==allocations&&b.effects.freed.empty()&&!f.nodes.calls&&!f.docs.other_calls,"pool clear performs no array free, node callback or extra allocation");
    balanced_items(f,5,3);check(word(b.pool,8)==B::heap_address(1)&&word(b.pool,0x60,4)==10&&word(b.pool,0x64,4)==5&&b.next_block==8&&b.next_heap==2,"cleared array capacity reused for four fresh records then source005440 grows on fifth");
    check(b.effects.freed==std::vector<std::uintptr_t>{B::heap_address()}&&pool_append_test::calls_named(b.effects,"array_allocate")==2,"post-clear growth frees retained old array only during actual append reallocation");
    for(unsigned i=0;i<5;++i)check(b.get(B::heap_address(1)+i*8)==B::block_address(i+3),"new array contains only current five fresh block identities");
    const auto current_array=b.heaps[1];expected=b.pool;put(expected,0x64,0,4);reset_image(expected);f.clear();
    check(f.docs.completed_frees==std::vector<std::uintptr_t>{B::block_address(2),B::block_address(1),B::block_address(),B::block_address(7),B::block_address(6),B::block_address(5),B::block_address(4),B::block_address(3)}&&b.pool==expected&&b.heaps[1]==current_array,"second lifecycle clear resets counters and retains newly grown array with exact eight total block frees");
}

// Compact controlled parser fixture: empty native tree, mapped input copy and
// deliberately supplied block-record prefixes. No XML parser or native DOM
// allocation/materialization is asserted by this source-chain integration.
struct ErrorFixture {
    static constexpr std::uintptr_t base=0x430000000ULL,input_address=0x530000000ULL,buffer_address=0x630000000ULL;
    static constexpr std::array<unsigned,4> pool_offsets{0x108,0x188,0x208,0x288};
    static constexpr std::uintptr_t array_address(unsigned i){return 0x930000000ULL+i*0x1000;}
    static constexpr std::uintptr_t block_address(unsigned i,unsigned n){return 0xa30000000ULL+i*0x1000+n*0x100;}
    s::NativeMessageDocument projection;std::array<std::uint8_t,0x308> raw{};std::array<std::uint8_t,2> input{'x',0};std::array<std::uint8_t,16> buffer{};
    std::array<std::array<std::uint8_t,24>,4> arrays{};std::vector<s::ReceiveBufferRegion> regions;
    s::ReceiveBufferMemory memory{};Nodes nodes;Docs docs{memory,nodes};s::NativeDocumentContext context{memory,docs};
    std::array<unsigned,4> counts{};std::array<std::uint8_t,0x308> seeded{};unsigned parses=0,allocations=0,copies=0;bool fail_middle=false;
    explicit ErrorFixture(std::array<unsigned,4> c):counts(c){
        raw.fill(0xa5);buffer.fill(0xcc);regions={{base,std::span(raw).first(0x6c)},{base+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{base+0x70,std::span(raw).subspan(0x70)},{input_address,input},{buffer_address,buffer}};for(unsigned i=0;i<4;++i)regions.push_back({array_address(i),arrays[i]});memory.regions=regions;
        docs.allocator=[&](std::uint64_t n){++allocations;check(n==2,"004280 lower input allocation is exactly count+1");return buffer_address;};
        docs.copier=[&](auto to,auto from,std::uint64_t n){++copies;check(to==buffer_address&&from==input_address&&n==1,"004280 copied controlled input through supplied lower byte provider");auto target=memory.view(to,n),source=memory.view(from,n);std::memmove(target.data(),source.data(),static_cast<std::size_t>(n));};
        docs.parser=[&](auto p){++parses;check(p==base&&buffer[0]=='x'&&!buffer[1],"controlled lower parser sees terminated source input buffer");for(unsigned i=0;i<4;++i){const auto off=pool_offsets[i];put(raw,off+8,array_address(i));put(raw,off+0x60,3,4);put(raw,off+0x64,counts[i],4);put(raw,off+0x68,0x11110000+i);put(raw,off+0x70,0x2222000000000000ULL+i);put(raw,off+0x78,0x3333000000000000ULL+i);arrays[i].fill(0x7b);for(unsigned n=0;n<counts[i];++n)put(arrays[i],n*8,block_address(i,n));}projection.error_6c=31;seeded=raw;};
        docs.before_free=[&](auto p){if(p&&fail_middle&&p==block_address(2,counts[2]-1))throw Failure{77};};
        docs.after_free=[&](auto p){if(p&&p==block_address(3,0))projection.error_6c=47;};
        check(s::construct_native_document_001830(base,1,0,context)==base,"source constructor initializes four embedded pool prefixes");
    }
    int parse(){return s::parse_native_document_004280(base,input_address,1,context);}
};
void parse_error_source_chain(){
    for(auto counts:std::array<std::array<unsigned,4>,3>{{{0,0,0,0},{0,1,2,1},{2,2,2,2}}}){ErrorFixture f(counts);const auto result=f.parse();std::vector<std::uintptr_t> expected{0};for(unsigned i=0;i<4;++i)for(unsigned n=counts[i];n;--n)expected.push_back(ErrorFixture::block_address(i,n-1));
        check(result==(counts[3]?47:31)&&f.projection.error_6c==result&&f.memory.view(ErrorFixture::base+0x6c,4).data()==reinterpret_cast<std::uint8_t*>(&f.projection.error_6c),"004280 returns live actual shared error cell after all four source clear calls");
        check(f.docs.completed_frees==expected&&!f.nodes.calls&&!f.docs.other_calls&&f.parses==1&&f.allocations==1&&f.copies==1,"parse error path reaches d70twice,e10,dc0 in embedded-pool order with empty tree and mandatory lower providers");
        auto expected_raw=f.seeded;for(unsigned i=0;i<4;++i){const auto off=ErrorFixture::pool_offsets[i];put(expected_raw,off+0x64,0,4);for(unsigned at:{0x68U,0x70U,0x78U})put(expected_raw,off+at,0);}
        check(f.raw==expected_raw,"all four prefixes reset even for zero block count; data/capacity/vtable/inline and input ownership preserved");
        for(unsigned i=0;i<4;++i){std::array<std::uint8_t,24> entries;entries.fill(0x7b);for(unsigned n=0;n<counts[i];++n)put(entries,n*8,ErrorFixture::block_address(i,n));check(f.arrays[i]==entries,"parse error clear preserves stale array entries and does not free array storage");}
    }
    {ErrorFixture f({1,1,2,1});f.fail_middle=true;failure([&]{(void)f.parse();});check(f.docs.completed_frees==std::vector<std::uintptr_t>{0,ErrorFixture::block_address(0,0),ErrorFixture::block_address(1,0)}&&f.projection.error_6c==31,"middle pool lower failure retains earlier source clears and live error, preventing later cleanup");
        auto expected=f.seeded;for(unsigned i=0;i<2;++i){const auto off=ErrorFixture::pool_offsets[i];put(expected,off+0x64,0,4);for(unsigned at:{0x68U,0x70U,0x78U})put(expected,off+at,0);}put(expected,ErrorFixture::pool_offsets[2]+0x64,1,4);check(f.raw==expected,"failed third clear leaves its decrement committed, its resets absent and fourth pool untouched");}
}
}
int main(){try{for(unsigned k=0;k<4;++k)for(std::uint32_t cpu:{0U,6U})clear_and_regrow(k,cpu);parse_error_source_chain();std::cout<<"PASS: "<<checks<<" pool clear integration checks; four allocators scalar/SIMD through append/track/release/clear, retained-array reuse/regrowth and004280 four-pool parse-error cleanup; parser/heap/free supplied, nativeDOM/ABI/EH open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
