#include "pool_destruct_test_fixture.hpp"
using namespace pool_destruct_test;
namespace {
using B=Backing;
void allocate_track_release(Fixture& f,unsigned nblocks,unsigned first){
    auto& b=f.raw;const auto& t=types[b.type];const unsigned count=t.count*nblocks;std::vector<std::uintptr_t> items;
    for(unsigned i=0;i<count;++i){const auto item=b.allocate();check(item==B::block_address(first+i/t.count)+(i%t.count)*t.stride,"source allocator uses exact owned block/item identity before pool lifecycle operation");items.push_back(item);b.set(item+8,0xface000000000000ULL+i);s::track_pool_005a20(B::pool_address,b.memory);check(word(b.pool,0x70,4)==i+1&&word(b.pool,0x74,4)==i+1&&!word(b.pool,0x7c,4),"source allocation/track counters remain coherent across append growth");}
    std::uintptr_t previous=0;for(unsigned i=0;i<count;++i){s::release_pool_item_0033a0(B::pool_address,items[i],b.memory);check(word(b.pool,0x68)==items[i]&&b.get(items[i])==previous&&b.get(items[i]+8)==0xface000000000000ULL+i,"source release retains payload and forms exact LIFO free list");previous=items[i];}
    const auto effects=b.effects.calls.size();for(unsigned i=0;i<count;++i){check(b.allocate()==items[count-i-1],"source allocator reuses source-released items in reverse order");s::track_pool_005a20(B::pool_address,b.memory);}
    check(b.effects.calls.size()==effects&&word(b.pool,0x70,4)==count&&word(b.pool,0x74,4)==count*2&&word(b.pool,0x78,4)==count&&!word(b.pool,0x7c,4),"reuse bypasses lower allocation/append and preserves tracked live counts");
    for(auto item:items)s::release_pool_item_0033a0(B::pool_address,item,b.memory);
}
void source_pool_lifecycle(unsigned k,std::uint32_t cpu){
    Fixture f(k,cpu);auto& b=f.raw;put(b.pool,0,type_tables[k]);b.count(0,1);allocate_track_release(f,3,0);
    check(word(b.pool,8)==B::heap_address()&&word(b.pool,0x60,4)==4&&word(b.pool,0x64,4)==3,"three first-generation blocks force source inline-to-heap append growth");
    f.clear();check(f.docs.completed_frees==std::vector<std::uintptr_t>{B::block_address(2),B::block_address(1),B::block_address()}&&word(b.pool,0)==type_tables[k]&&word(b.pool,8)==B::heap_address()&&word(b.pool,0x60,4)==4&&!word(b.pool,0x64,4),"N20 source clear frees first generation while retaining type table and array capacity");
    allocate_track_release(f,5,3);check(b.next_block==8&&b.next_heap==2&&word(b.pool,8)==B::heap_address(1)&&word(b.pool,0x60,4)==10&&word(b.pool,0x64,4)==5&&b.effects.freed==std::vector<std::uintptr_t>{B::heap_address()},"post-clear allocations reuse old array then source append regrows on fifth block and frees old array");
    auto expected=b.pool;put(expected,0,base_table);put(expected,0x64,0,4);reset_image(expected);const auto heaps=b.heaps;const auto effects=b.effects.calls.size();
    f.docs.before_free=[&](auto p){check(word(b.pool,0)==type_tables[k],"N21 destructor keeps concrete type table through every free callback");if(p==B::heap_address(1))check(!word(b.pool,0x68)&&!word(b.pool,0x70)&&!word(b.pool,0x78)&&!word(b.pool,0x64,4),"pointer-array free occurs after block loop and paired counter resets");};
    f.destroy();check(f.docs.completed_frees==std::vector<std::uintptr_t>{B::block_address(2),B::block_address(1),B::block_address(),B::block_address(7),B::block_address(6),B::block_address(5),B::block_address(4),B::block_address(3),B::heap_address(1)},"actual final destructor bridge frees second-generation blocks LIFO then current array exactly once");
    check(b.pool==expected&&b.heaps==heaps&&b.effects.calls.size()==effects&&!f.nodes.calls&&!f.docs.other_calls,"destructor ends at base table and retains stale pointer/capacity/array bytes without other lower effects");
    for(unsigned i=0;i<5;++i)check(word(b.heaps[1],i*8)==B::block_address(i+3),"freed-array contents are not cleared or reset to inline by source destructor");
}

// Controlled empty-tree document fixture. Arrays and block identities are
// supplied lower-provider data; no native parser/DOM/heap or node-base body is
// reconstructed by this fixture. Existing001830/001e00 source is exercised.
struct DocumentFixture {
    static constexpr std::uintptr_t address=0x440000000ULL,buffer=0x640000000ULL,vector=0xc40000000ULL,string1=0xd40000000ULL,string2=0xd40001000ULL;
    static constexpr std::array<unsigned,4> offsets{0x108,0x188,0x208,0x288};
    static constexpr std::uintptr_t array(unsigned k){return 0x940000000ULL+k*0x1000;}
    static constexpr std::uintptr_t block(unsigned k,unsigned n){return 0xa40000000ULL+k*0x1000+n*0x100;}
    s::NativeMessageDocument projection;std::array<std::uint8_t,0x308> raw{};std::array<std::array<std::uint8_t,24>,4> arrays{};
    std::vector<s::ReceiveBufferRegion> regions;s::ReceiveBufferMemory memory{};Nodes nodes;Docs docs{memory,nodes};s::NativeDocumentContext context{memory,docs};
    std::array<unsigned,4> counts{2,1,0,2};std::array<std::uint8_t,0x308> seeded{};bool external_vector=true,fail_array=false,fail_node_base=false;
    explicit DocumentFixture(bool external=true):external_vector(external){
        raw.fill(0xa5);regions={{address,std::span(raw).first(0x6c)},{address+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{address+0x70,std::span(raw).subspan(0x70)}};for(unsigned k=0;k<4;++k)regions.push_back({array(k),arrays[k]});memory.regions=regions;
        check(s::construct_native_document_001830(address,1,0,context)==address,"001830 source initializes all four embedded type tables");
        for(unsigned k=0;k<4;++k){const auto off=offsets[k];put(raw,off+8,array(k));put(raw,off+0x60,3,4);put(raw,off+0x64,counts[k],4);put(raw,off+0x68,0x11110000+k);put(raw,off+0x70,0x2222000000000000ULL+k);put(raw,off+0x78,0x3333000000000000ULL+k);arrays[k].fill(0x7b);for(unsigned n=0;n<counts[k];++n)put(arrays[k],n*8,block(k,n));}
        put(raw,0x98,buffer);put(raw,0xa8,external_vector?vector:address+0xb0);put(raw,0x78,0x200,4);put(raw,0x80,string1);put(raw,0x88,16);seeded=raw;
        docs.before_free=[&](auto p){
            for(unsigned k=0;k<4;++k){bool relevant=p==array(k);for(unsigned n=0;n<counts[k];++n)relevant=relevant||p==block(k,n);if(!relevant)continue;
                check(word(raw,offsets[k])==type_tables[k],"001e00 selected pool installs concrete table during every block/array free");for(unsigned j=k+1;j<4;++j)check(word(raw,offsets[j])==base_table,"document destroys higher embedded pool indexes first");
                if(p==array(k))check(!word(raw,offsets[k]+0x64,4)&&!word(raw,offsets[k]+0x68)&&!word(raw,offsets[k]+0x70)&&!word(raw,offsets[k]+0x78),"document pool array free follows complete pool resets");
            }
            if(p==vector){for(unsigned k=0;k<4;++k)check(word(raw,offsets[k])==base_table,"document vector free occurs after all four base-table publications");}
        };
        docs.after_free=[&](auto p){if(fail_array&&p==array(2))throw Failure{77};if(p==vector){put(raw,0x78,0x200,4);put(raw,0x80,string2);put(raw,0x88,24);}};
        docs.node_base=[&](auto p){check(p==address&&word(raw,0)==context.identities.document&&!word(raw,0x78,4)&&!word(raw,0x80)&&!word(raw,0x88),"mandatory node-base callback follows document string resets and retains source document table");for(unsigned k=0;k<4;++k)check(word(raw,offsets[k])==base_table,"node-base callback follows all four pool base-table stores");if(fail_node_base)throw Failure{77};};
    }
    void destroy(){s::destroy_native_document_001e00(address,context);}
    std::vector<std::uintptr_t> expected_frees()const{std::vector<std::uintptr_t> expected{string1,buffer};for(unsigned r=4;r;--r){const auto k=r-1;for(unsigned n=counts[k];n;--n)expected.push_back(block(k,n-1));expected.push_back(array(k));}if(external_vector){expected.push_back(vector);expected.push_back(string2);}return expected;}
};
void document_source_lifecycle(){
    for(bool external:{false,true}){DocumentFixture f(external);const auto entries=f.arrays;f.destroy();check(f.docs.completed_frees==f.expected_frees()&&f.docs.node_bases==std::vector<std::uintptr_t>{DocumentFixture::address}&&!f.nodes.calls&&!f.docs.other_calls,"001e00 actual source invokes reverse104/112/80/120 pool destructors then vector/string/node-base lower providers");
        auto expected=f.seeded;put(expected,0x98,0);put(expected,0x78,0,4);put(expected,0x80,0);put(expected,0x88,0);put(expected,0xa4,0,4);for(unsigned k=0;k<4;++k){const auto off=DocumentFixture::offsets[k];put(expected,off,base_table);put(expected,off+0x64,0,4);for(unsigned at:{0x68U,0x70U,0x78U})put(expected,off+at,0);}
        check(f.raw==expected&&f.arrays==entries&&f.projection.error_6c==0,"complete document path preserves pool data/capacity/stale entries and document vector while clearing only observed fields");
    }
    {DocumentFixture f;f.fail_array=true;failure([&]{f.destroy();});check(word(f.raw,DocumentFixture::offsets[3])==base_table&&word(f.raw,DocumentFixture::offsets[2])==type_tables[2]&&word(f.raw,DocumentFixture::offsets[1])==type_tables[1]&&word(f.raw,DocumentFixture::offsets[0])==type_tables[0],"failed112array free retains prior104base table,current112type table and untouched80/120 pools");check(f.docs.completed_frees.back()==DocumentFixture::array(2)&&f.docs.node_bases.empty()&&!word(f.raw,DocumentFixture::offsets[2]+0x78)&&word(f.raw,DocumentFixture::offsets[1]+0x68)!=0,"post-free failure preserves reset prefix,prevents later pool/vector/base callbacks and adds no rollback");}
    {DocumentFixture f;f.fail_node_base=true;failure([&]{f.destroy();});check(f.docs.completed_frees==f.expected_frees()&&f.docs.node_bases==std::vector<std::uintptr_t>{DocumentFixture::address},"mandatory node-base failure propagates after all completed pool/vector/string cleanup");}
}
}
int main(){try{for(unsigned k=0;k<4;++k)for(std::uint32_t cpu:{0U,6U})source_pool_lifecycle(k,cpu);document_source_lifecycle();std::cout<<"PASS: "<<checks<<" pool destructor integration checks; four scalar/SIMD source allocation/append/track/release/clear/destruct chains and001e00 reverse embedded-pool/vector/string cleanup; node-base/heap/free supplied,nativeDOM/ABI/EH open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
