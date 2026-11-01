// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/node_insertion_test_fixture.hpp"
#include "preloader_l/xml/pool_release.hpp"
using namespace node_constructors_test;
namespace {
using Memory=node_parse_test::Memory;
struct PoolLower:s::PoolAllocationEffects {
    unsigned calls=0;
    [[noreturn]]void unexpected(){++calls;throw std::runtime_error("recycled source pool path unexpectedly requested heap/fill/block append");}
    std::uintptr_t allocate_04b0bc(std::uint64_t)override{unexpected();}
    void fill_0793d0(std::uintptr_t,std::uint8_t,std::uint64_t)override{unexpected();}
    void append_block_005440(std::uintptr_t,std::uintptr_t)override{unexpected();}
};
struct Recording:s::PoolAllocationDispatch {
    s::PoolAllocationDispatch& source;std::vector<Allocator::Call> calls;
    explicit Recording(s::PoolAllocationDispatch& p):source(p){}
    std::uintptr_t pool_allocate(std::uintptr_t t,std::uintptr_t p)override{calls.push_back({t,p});return source.pool_allocate(t,p);}
};
struct Effects:s::NodeInsertionEffects {
    Memory& raw;node_parse_test::Lower primitives;std::vector<node_parse_test::Lower::Parse> others;
    std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    std::function<std::uintptr_t(std::uint64_t)> allocate_hook;std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copy_hook;
    std::function<std::uintptr_t(const node_parse_test::Lower::Parse&)> other_hook;
    unsigned workspaces=0;
    Effects(Memory& m,node_parse_test::Nodes& n,const s::NativeErrorArgumentTail& a,s::PoolAllocationDispatch& p):NodeInsertionEffects(m.memory,n,a,p),raw(m),primitives(m){primitives.compare_hook=[](const auto&){return 1;};}
    s::NativeNodeParseScratch acquire_parse_scratch(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{++workspaces;return {Memory::scratch_address,Memory::pair};}
    std::uintptr_t parse_other_virtual(std::uintptr_t t,std::uintptr_t n,std::uintptr_t c,std::uintptr_t p,std::uintptr_t l)override{others.push_back({t,n,c,p,l});return other_hook?other_hook(others.back()):0;}
    std::uintptr_t materialize_0035e0(std::uintptr_t p)override{return primitives.materialize_0035e0(p);}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{return primitives.compare_05c550(a,b,n);}
    void report_error(const s::NativeNodeParseError& e)override{raw.set(e.document+0x6c,static_cast<std::uint32_t>(e.code),4);}
    std::int32_t classify_051fc0(std::uint32_t b)override{return parse_entry_test::controlled_space(b);}
    void free_04b0f8(std::uintptr_t p)override{frees.push_back(p);}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{allocations.push_back(n);if(!allocate_hook)throw std::runtime_error("unexpected supplied heap allocation");return allocate_hook(n);}
    void copy_078d30(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{if(!copy_hook)throw std::runtime_error("unexpected supplied copy");copy_hook(a,b,n);}
    void format_header_0060a0(std::uintptr_t,std::uint64_t,std::uintptr_t,std::uintptr_t,std::int32_t,std::uint32_t,std::uint32_t)override{throw std::runtime_error("unexpected header formatting");}
    void format_separator_0060a0(std::uintptr_t,std::uint64_t,std::uintptr_t)override{throw std::runtime_error("unexpected separator formatting");}
    void format_detail_006110(std::uintptr_t,std::uint64_t,std::uintptr_t,std::uintptr_t)override{throw std::runtime_error("unexpected detail formatting");}
};
struct Fixture {
    static constexpr std::uintptr_t copied_address=0x510000000ULL,document_table=0x18007fec8ULL;
    static constexpr std::array<std::uintptr_t,4> pool_tables{0x18007fe08ULL,0x18007fe38ULL,0x18007fe68ULL,0x18007fe98ULL};
    static constexpr std::array<std::uintptr_t,4> pool_targets{0x180002b10ULL,0x180002600ULL,0x180002960ULL,0x1800027b0ULL};
    static constexpr std::array<unsigned,5> offsets{0x288,0x288,0x108,0x208,0x288};
    Memory raw;node_parse_test::Nodes nodes;s::NativeErrorArgumentTail arguments{0xf100};PoolLower pool_lower;
    s::PoolAllocationContext pool_context{raw.memory,pool_lower};s::PoolAllocationEffects selected{pool_context};Recording allocations{selected};
    Effects effects{raw,nodes,arguments,allocations};s::NativeDocumentContext document{raw.memory,effects};s::NativeMessageDocument projection;
    std::array<std::uint8_t,64> copied{};std::array<std::uint8_t,0x88> document_vtable{};
    std::array<std::array<std::uint8_t,0x88>,5> node_vtables{};
    std::array<std::array<std::uint8_t,0x28>,4> pool_vtables{};std::array<std::uint8_t,40> tokens{};
    unsigned copies=0;std::size_t input_count=1;
    Fixture(){put(document_vtable,0x80,0x180004970);for(auto& table:node_vtables)table=raw.vtable;
        for(unsigned i=0;i<4;++i){put(pool_vtables[i],0x10,pool_targets[i]);put(pool_vtables[i],0x20,0x180005a20ULL);}
        for(unsigned i=0;i<5;++i)put(tokens,i*8,node_identify_test::Fixture::token(i));refresh();
        check(s::construct_native_document_001830(Memory::doc,1,0,document)==Memory::doc,"actual001830 initializes document pools and allocation array");
        effects.allocate_hook=[&](auto n){check(n==input_count+1,"actual004280 requests input count+1 from controlled heap boundary");return copied_address;};
        effects.copy_hook=[&](auto a,auto b,std::uint64_t n){++copies;check(a==copied_address&&b==Memory::input_address&&n==input_count,"actual004280 copy operands remain exact");auto to=raw.memory.view(a,n),from=raw.memory.view(b,n);std::memcpy(to.data(),from.data(),static_cast<std::size_t>(n));};
        effects.other_hook=[&](const auto&){return copied_address+input_count;};
        nodes.track_hook=[&](const auto& t){check(t.target==0x180005a20ULL,"retained pool+20 identity forwards existing005a20");s::track_pool_005a20(t.pool,raw.memory);};
    }
    void refresh(){raw.omitted={{Memory::doc+0x6c,4}};raw.extras={{Memory::doc+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{copied_address,copied},{document_table,document_vtable},{node_identify_test::Fixture::slots,tokens}};
        for(unsigned i=0;i<5;++i)raw.extras.push_back({tables[i],node_vtables[i]});for(unsigned i=0;i<4;++i)raw.extras.push_back({pool_tables[i],pool_vtables[i]});raw.refresh();}
    void recycled(unsigned k){raw.set(Memory::doc+offsets[k]+0x68,Memory::child);raw.objects[1].fill(0xa5);raw.set(Memory::child,0);}
    std::uintptr_t construct(unsigned k){const auto p=Memory::doc+offsets[k];switch(k){case 0:return effects.construct_001590(Memory::doc,p);case 1:return effects.construct_001610(Memory::doc,p);case 2:return effects.construct_001690(Memory::doc,p);case 3:return effects.construct_001720(Memory::doc,p);default:return effects.construct_0017b0(Memory::doc,p);}}
    int parse(std::string_view v){input_count=v.size();raw.input.fill(0);std::copy(v.begin(),v.end(),raw.input.begin());return s::parse_native_document_004280(Memory::doc,Memory::input_address,v.size(),document);}
};
void constructor_insertion_chain(){
    for(unsigned k=0;k<5;++k){Fixture f;f.recycled(k);f.construct(k);const auto pool=Memory::doc+Fixture::offsets[k];
        const auto current=f.raw.get(pool+0x70,4),free_head=f.raw.get(pool+0x68);const auto child_before=f.raw.objects[1];
        check(f.raw.get(Memory::doc+0x104,4)==1&&f.raw.get(pool+0x7c,4)==1,"actual constructor registers outstanding node and source pool allocation");
        f.effects.prepare_003aa0(0xdeadbeef,Memory::child);
        check(!f.raw.get(Memory::doc+0x104,4)&&!f.raw.get(pool+0x7c,4),"final prepare bridges actual vector removal and reused005a20 tracking for all five constructors");
        check(f.raw.objects[1]==child_before&&f.raw.get(pool+0x70,4)==current&&f.raw.get(pool+0x68)==free_head,"tracking leaves node bytes,current allocation and free-list unchanged");
        check(f.nodes.tracks.size()==1&&f.nodes.tracks[0].pool==pool&&f.nodes.destroys.empty()&&f.nodes.releases.empty()&&f.effects.frees.empty(),"insertion never adds destructor,release,heap free or duplicate allocator");
    }
}
void existing_node_moves(){
    for(bool same:{false,true}){Fixture f;const auto owner=same?Memory::doc:Memory::owner;
        f.raw.set(Memory::child+0x10,Memory::doc);f.raw.set(Memory::child+0x48,0);f.raw.set(Memory::child+0x50,Memory::last);
        f.raw.set(Memory::doc+0x38,Memory::child);f.raw.set(Memory::doc+0x40,Memory::last);f.raw.set(Memory::last+0x48,Memory::child);
        const auto outstanding=f.raw.get(Memory::doc+0x104,4);f.effects.prepare_003aa0(owner,Memory::child);
        check(f.raw.get(Memory::doc+0x38)==Memory::last&&f.raw.get(Memory::doc+0x40)==Memory::last&&!f.raw.get(Memory::last+0x48),"same/different incoming owner uses actual old parent for detachment");
        check(!f.raw.get(Memory::child+0x10)&&!f.raw.get(Memory::child+0x48)&&!f.raw.get(Memory::child+0x50),"final bridge clears linked child before caller insertion");
        check(f.nodes.tracks.empty()&&f.nodes.destroys.empty()&&f.nodes.releases.empty()&&f.raw.get(Memory::doc+0x104,4)==outstanding,"linked final bridge leaves vector and pool path untouched");
    }
}
void caller_live_last_and_track_failure(){
    {Fixture f;f.recycled(3);f.nodes.track_hook=[&](const auto& t){s::track_pool_005a20(t.pool,f.raw.memory);f.raw.set(Memory::doc+0x38,Memory::last);f.raw.set(Memory::doc+0x40,Memory::last);};
        check(f.parse("text")==0,"actual parser/insertion chain returns through normal source epilog");
        check(f.raw.get(Memory::last+0x50)==Memory::child&&f.raw.get(Memory::child+0x48)==Memory::last&&f.raw.get(Memory::doc+0x40)==Memory::child,"004970 reloads owner.last after prepare tracking effect before append");
        check(!f.raw.get(Memory::doc+0x104,4)&&f.raw.get(Memory::child+0x10)==Memory::doc&&f.nodes.destroys.empty()&&f.effects.frees==std::vector<std::uintptr_t>{0},"source chain appends after committed removal; only preexisting document-clear free(null) occurs");
    }
    {Fixture f;f.recycled(3);f.nodes.track_hook=[](const auto&){throw Failure{};};fails([&]{f.parse("text");});
        check(!f.raw.get(Memory::doc+0x104,4)&&!f.raw.get(Memory::doc+0x38)&&!f.raw.get(Memory::child+0x10),"tracking failure retains vector removal and skips caller append without rollback");
        check(f.raw.get(Memory::doc+0xa4,4)==1&&f.nodes.destroys.empty()&&f.nodes.releases.empty()&&f.effects.frees==std::vector<std::uintptr_t>{0},"unwinding adds no native cleanup/free or invented depth recovery after preexisting clear(null)");
    }
}
void source_identify_parser_chain(){
    struct Case{std::string_view input;unsigned kind,match,advance;};
    const std::array cases{Case{"<!--x",0,1,4},Case{"<?x",1,0,2},Case{"<x",2,4,1},Case{"123456789x",3,2,9},Case{"<!x",4,3,2},Case{"text",3,5,0}};
    for(const auto& c:cases){Fixture f;f.recycled(c.kind);f.effects.primitives.compare_hook=[&](const auto& v){return c.match<5&&v.right==node_identify_test::Fixture::token(c.match)?0:1;};
        check(f.parse(c.input)==0&&f.projection.error_6c==0&&f.effects.workspaces==1&&f.effects.others.size()==1,"actual document/input/entry/selected-parser/Identify/final-constructor source chain succeeds with controlled child parse");
        const auto& p=f.effects.others[0];check(p.node==Memory::child&&p.cursor==Fixture::copied_address+c.advance&&p.target==Memory::parse80,"actual Identify routes five constructor sources and returns separate stripped cursor and node");
        const auto flag=c.kind==3?(c.match==2?1U:0U):c.kind==2?0U:0xa5U;
        check(f.raw.get(Memory::child)==tables[c.kind]&&f.raw.get(Memory::child+0x30,4)==1&&f.raw.objects[1][0x68]==flag,"Identify line and CDATA publication follow actual constructor writes while unrelated node types retain untouched bytes");
        check(f.raw.get(Memory::doc+0x38)==Memory::child&&f.raw.get(Memory::doc+0x40)==Memory::child&&f.raw.get(Memory::child+0x10)==Memory::doc&&!f.raw.get(Memory::doc+0xa4,4),"existing parser appends source-constructed child and restores depth");
        check(!f.raw.get(Memory::doc+0x104,4)&&f.raw.get(Memory::doc+0xb0)==Memory::child&&f.nodes.tracks.size()==1&&f.allocations.calls.size()==1&&!f.pool_lower.calls&&f.effects.allocations==std::vector<std::uint64_t>{c.input.size()+1}&&f.copies==1,"actual constructor/vector/prepare chain removes outstanding entry before append with one tracking call");
    }
    {Fixture f;f.recycled(3);std::copy_n(" \nx",3,f.raw.input.begin());f.raw.set(Memory::line,19,4);check(f.effects.identify_0038b0(Memory::doc,Memory::input_address,node_identify_test::Fixture::out)==Memory::input_address&&f.raw.get(Memory::line,4)==19&&f.raw.get(Memory::child+0x30,4)==20,"fallback rewinds initial line after actual text constructor and Identify node-line publication");}
    {Fixture f;check(f.effects.parse_child_virtual(0xf999,Memory::child,0x123,0x456,0x789)==Fixture::copied_address+1&&f.effects.workspaces==0&&f.allocations.calls.empty(),"inherited unknown target remains mandatory rather than a new generic parser closure");}
}
}
int main(){try{constructor_insertion_chain();existing_node_moves();caller_live_last_and_track_failure();source_identify_parser_chain();std::cout<<"PASS: "<<checks<<" node-insertion integration checks; final prepare with prior constructors,pool005a20,vector and document/Identify/parser chain; existing-node detach,live caller.last and failure prefixes; heap/unknown child/DOM/nativeABI open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
