#include "tests/support/socket/node_base_test_fixture.hpp"
using namespace node_base_test;
namespace {
using M=Memory;
// Source003110 is retained, while child probe/destructor targets below are
// explicit controlled lower boundaries. Pool release uses its real N17 source.
struct ChildNodes:s::NativeNodeClearEffects {
    Memory& raw;std::vector<std::uintptr_t> probes,destroys,releases;std::function<void(std::string,std::uintptr_t)> hook;unsigned other_calls=0;
    static constexpr std::uintptr_t pool=0x4a0000,table=0x490000,pool_table=0x4b0000;
    explicit ChildNodes(Memory& m):raw(m){}
    [[noreturn]]void unexpected(){++other_calls;throw std::runtime_error("unexpected child node effect");}
    std::uint64_t node_probe(std::uintptr_t target,std::uintptr_t p)override{check(target==0xf001&&!raw.get(p+0x10)&&!raw.get(p+0x48)&&!raw.get(p+0x50),"003110 unlinks child before supplied probe boundary");probes.push_back(p);if(hook)hook("probe",p);return 1;}
    void node_destroy(std::uintptr_t target,std::uintptr_t p,std::uint32_t flags)override{check(target==0xf002&&!flags,"003110 invokes controlled destructor target with flags0");destroys.push_back(p);if(hook)hook("destroy",p);}
    void pool_release(std::uintptr_t target,std::uintptr_t p,std::uintptr_t item)override{check(target==0x1800033a0ULL&&p==pool,"003110 dispatches controlled pool slot to retained0033a0 source");releases.push_back(item);s::release_pool_item_0033a0(p,item,raw.memory);if(hook)hook("release",item);}
    void pool_track(std::uintptr_t target,std::uintptr_t p)override{check(target==0x180005a20ULL,"controlled tracking identity");s::track_pool_005a20(p,raw.memory);}
    void remove_parent_003030(std::uintptr_t,std::uintptr_t)override{unexpected();}
    void delete_node_003240(std::uintptr_t)override{unexpected();}
};
struct ChildFixture {
    static constexpr std::uintptr_t child(unsigned i){return 0x450000+i*0x1000;}
    Memory raw;std::array<std::array<std::uint8_t,0x80>,3> children{};std::array<std::uint8_t,0x80> table{},pool{};std::array<std::uint8_t,0x30> pool_table{};
    ChildNodes nodes{raw};SourceDocs docs{raw.memory,nodes};unsigned count;
    explicit ChildFixture(unsigned n,bool parent):count(n){if(parent)raw.linked();put(raw.bytes,0x38,n?child(0):0);put(raw.bytes,0x40,n?child(n-1):0);put(raw.bytes,0x18,0x200,4);put(raw.bytes,0x1c,0xdeadbeef,4);put(raw.bytes,0x20,0x88112233);put(raw.bytes,0x28,24);
        put(table,0x38,0xf001);put(table,0x78,0xf002);put(pool,0,ChildNodes::pool_table);put(pool,0x70,n,4);put(pool,0x74,n,4);put(pool,0x78,n,4);put(pool_table,0x18,0x1800033a0ULL);put(pool_table,0x20,0x180005a20ULL);
        raw.extras={{ChildNodes::table,table},{ChildNodes::pool,pool},{ChildNodes::pool_table,pool_table}};
        for(unsigned i=0;i<n;++i){put(children[i],0,ChildNodes::table);put(children[i],8,M::node);put(children[i],0x10,M::node);put(children[i],0x48,i?child(i-1):0);put(children[i],0x50,i+1<n?child(i+1):0);put(children[i],0x60,ChildNodes::pool);raw.extras.push_back({child(i),children[i]});}raw.refresh();
    }
    void destroy(){docs.destroy_node_base_001ea0(M::node);}
};
void actual_child_source(){
    for(unsigned n:{0U,1U,3U})for(bool parent:{false,true}){ChildFixture f(n,parent);auto expected=f.raw.bytes;reset_value(expected);put(expected,0x38,0);put(expected,0x40,0);if(parent){put(expected,0x10,0);put(expected,0x48,0);put(expected,0x50,0);}f.destroy();
        check(f.raw.bytes==expected&&f.docs.completed_frees==std::vector<std::uintptr_t>{0x88112233}&&!f.docs.other_calls&&!f.nodes.other_calls,"final node-base bridge reuses actual003110 child source and then resets own value prefix");
        check(f.nodes.probes.size()==n&&f.nodes.destroys.size()==n&&f.nodes.releases.size()==n&&word(f.pool,0x70,4)==0&&word(f.pool,0x68)==(n?ChildFixture::child(n-1):0)&&word(f.pool,0x74,4)==n&&word(f.pool,0x78,4)==n,"owned children released by actual0033a0 with preserved total/highwater; virtual probe/destructor remain supplied");
        for(unsigned i=0;i<n;++i)check(f.nodes.probes[i]==ChildFixture::child(i)&&f.nodes.destroys[i]==ChildFixture::child(i)&&f.nodes.releases[i]==ChildFixture::child(i)&&word(f.children[i],0)==(i?ChildFixture::child(i-1):0)&&!word(f.children[i],0x10)&&!word(f.children[i],0x48)&&!word(f.children[i],0x50),"actual003110 traverses controlled children in source order and returns each to LIFO pool");
        if(parent)check(word(f.raw.parent_bytes,0x38)==M::next&&word(f.raw.parent_bytes,0x40)==M::previous&&word(f.raw.previous_bytes,0x50)==M::next&&word(f.raw.next_bytes,0x48)==M::previous,"own parent/sibling unlink follows completed child source chain");
    }
    {ChildFixture f(3,true);auto expected=f.raw.bytes;put(expected,0,node_table);put(expected,0x38,ChildFixture::child(2));f.nodes.hook=[](const auto& stage,auto p){if(stage=="probe"&&p==ChildFixture::child(1))throw Failure{77};};failure([&]{f.destroy();});check(f.raw.bytes==expected&&f.nodes.releases==std::vector<std::uintptr_t>{ChildFixture::child(0)}&&word(f.pool,0x70,4)==2&&word(f.pool,0x68)==ChildFixture::child(0)&&f.docs.entered_frees.empty(),"second child probe failure keeps first release and child unlink prefix while preventing own unlink/value cleanup");}
}

struct DocumentFixture {
    static constexpr std::uintptr_t base=0x460000000ULL,buffer=0x660000000ULL,vector=0xc60000000ULL,document_string=0xd60000000ULL,node_string=0xe60000000ULL,changed_node_string=0xe60001000ULL;
    static constexpr std::array<unsigned,4> offsets{0x108,0x188,0x208,0x288};
    static constexpr std::uintptr_t array(unsigned k){return 0x960000000ULL+k*0x1000;}
    static constexpr std::uintptr_t block(unsigned k,unsigned n){return 0xa60000000ULL+k*0x1000+n*0x100;}
    s::NativeMessageDocument projection;std::array<std::uint8_t,0x308> raw{};std::array<std::array<std::uint8_t,24>,4> arrays{};
    std::array<std::uint8_t,0x80> parent{},previous{},next{};std::vector<s::ReceiveBufferRegion> regions;s::ReceiveBufferMemory memory{};
    pool_clear_test::Nodes nodes;SourceDocs docs{memory,nodes};s::NativeDocumentContext context{memory,docs};std::array<unsigned,4> counts{1,0,2,1};bool external,linked,fail_node_free=false,fail_pool_free=false;
    std::array<std::uint8_t,0x308> seeded{};
    DocumentFixture(bool vector_external,bool parent_linked):external(vector_external),linked(parent_linked){
        raw.fill(0xa5);parent.fill(0xa5);previous.fill(0xa5);next.fill(0xa5);regions={{base,std::span(raw).first(0x6c)},{base+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{base+0x70,std::span(raw).subspan(0x70)},{M::parent,parent},{M::previous,previous},{M::next,next}};for(unsigned k=0;k<4;++k)regions.push_back({array(k),arrays[k]});memory.regions=regions;check(s::construct_native_document_001830(base,1,0,context)==base,"source document constructor prepares embedded pools before final node-base binding");
        for(unsigned k=0;k<4;++k){const auto off=offsets[k];put(raw,off+8,array(k));put(raw,off+0x60,3,4);put(raw,off+0x64,counts[k],4);put(raw,off+0x68,0x11110000+k);put(raw,off+0x70,0x2222000000000000ULL+k);put(raw,off+0x78,0x3333000000000000ULL+k);arrays[k].fill(0x7b);for(unsigned n=0;n<counts[k];++n)put(arrays[k],n*8,block(k,n));}
        put(raw,0x98,buffer);put(raw,0xa8,external?vector:base+0xb0);put(raw,0x78,0x200,4);put(raw,0x80,document_string);put(raw,0x88,16);put(raw,0x18,0x200,4);put(raw,0x1c,0xdeadbeef,4);put(raw,0x20,node_string);put(raw,0x28,25);
        put(raw,0x10,linked?M::parent:0);put(raw,0x48,linked?M::previous:0);put(raw,0x50,linked?M::next:0);put(parent,0x38,base);put(parent,0x40,base);put(previous,0x50,base);put(next,0x48,base);seeded=raw;
        docs.before_free=[&](auto p){if(fail_pool_free&&p==array(2))throw Failure{77};if(p==node_string||p==changed_node_string){check(word(raw,0)==node_table&&word(raw,0x18,4)==0x200&&word(raw,0x1c,4)==0xdeadbeef&&!word(raw,0x78,4)&&!word(raw,0x80)&&!word(raw,0x88),"node-base free follows document cleanup,stores node table and preserves node-prefix padding");for(unsigned k=0;k<4;++k)check(word(raw,offsets[k])==pool_destruct_test::base_table,"node-base source follows all four real pool destructor base stores");if(linked)check(!word(raw,0x10)&&!word(raw,0x48)&&!word(raw,0x50),"node-base free follows own unlink after document and pool cleanup");if(fail_node_free)throw Failure{77};}};
        docs.after_free=[&](auto p){if(p==vector){put(raw,0x18,0x200,4);put(raw,0x20,changed_node_string);}};
    }
    void destroy(){s::destroy_native_document_001e00(base,context);}
    std::vector<std::uintptr_t> expected_frees()const{std::vector<std::uintptr_t> expected{document_string,buffer};for(unsigned r=4;r;--r){const auto k=r-1;for(unsigned n=counts[k];n;--n)expected.push_back(block(k,n-1));expected.push_back(array(k));}if(external)expected.push_back(vector);expected.push_back(external?changed_node_string:node_string);return expected;}
};
void document_to_node_base(){
    for(bool external:{false,true})for(bool linked:{false,true}){DocumentFixture f(external,linked);const auto arrays=f.arrays;f.destroy();auto expected=f.seeded;put(expected,0,node_table);put(expected,0x18,0,4);put(expected,0x20,0);put(expected,0x28,0);put(expected,0x98,0);put(expected,0x78,0,4);put(expected,0x80,0);put(expected,0x88,0);put(expected,0xa4,0,4);if(linked){put(expected,0x10,0);put(expected,0x48,0);put(expected,0x50,0);}for(unsigned k=0;k<4;++k){const auto off=DocumentFixture::offsets[k];put(expected,off,pool_destruct_test::base_table);put(expected,off+0x64,0,4);for(unsigned at:{0x68U,0x70U,0x78U})put(expected,off+at,0);}
        check(f.raw==expected&&f.docs.completed_frees==f.expected_frees()&&f.arrays==arrays&&!f.nodes.calls&&!f.docs.other_calls&&f.projection.error_6c==0,"001e00 source completes four pool destructors and actual final001ea0 with exact field writes and ordered lower frees");
        if(linked)check(word(f.parent,0x38)==M::next&&word(f.parent,0x40)==M::previous&&word(f.previous,0x50)==M::next&&word(f.next,0x48)==M::previous,"document own parent/siblings unlink through real node-base source after empty-child deletion");
    }
    {DocumentFixture f(true,true);f.fail_node_free=true;failure([&]{f.destroy();});check(word(f.raw,0)==node_table&&!word(f.raw,0x10)&&!word(f.raw,0x48)&&!word(f.raw,0x50)&&word(f.raw,0x18,4)==0x200&&word(f.raw,0x20)==DocumentFixture::changed_node_string&&word(f.raw,0x28)==25&&f.docs.entered_frees.back()==DocumentFixture::changed_node_string&&f.docs.completed_frees.back()==DocumentFixture::vector,"node-base storage free failure retains completed document/pool cleanup and own unlink while preventing node-prefix resets");}
    {DocumentFixture f(true,true);f.fail_pool_free=true;failure([&]{f.destroy();});check(word(f.raw,0)==f.context.identities.document&&word(f.raw,0x10)==M::parent&&word(f.raw,0x18,4)==0x200&&word(f.raw,0x20)==DocumentFixture::node_string&&f.docs.entered_frees.back()==DocumentFixture::array(2),"earlier pool failure prevents final node-base table/parent/value changes entirely");}
}
}
int main(){try{actual_child_source();document_to_node_base();std::cout<<"PASS: "<<checks<<" node-base integration checks; actual003110 for zero/one/three owned children with source pool release,actual001e00 four-pool cleanup then final001ea0,parent unlink and failure prefixes; node virtual/free/heap/DOM/nativeABI/EH remain supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
