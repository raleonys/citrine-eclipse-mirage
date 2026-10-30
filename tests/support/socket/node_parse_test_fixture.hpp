#pragma once
#include "preloader_l/xml/node_parse.hpp"
#include "tests/support/socket/document_error_test_fixture.hpp"
#include <map>

namespace node_parse_test {
namespace s=preloader_l::socket_recovered;
using document_error_test::check;using document_error_test::checks;
using document_error_test::put;using document_error_test::word;
using document_error_test::Failure;using document_error_test::fails;using document_error_test::unmapped;
struct Memory {
    static constexpr std::uintptr_t owner=0x410000000ULL,doc=0x420000000ULL,child=0x430000000ULL,
        element=0x440000000ULL,declaration=0x450000000ULL,first=0x460000000ULL,last=0x470000000ULL,
        other_doc=0x480000000ULL,pool=0x490000000ULL,other_pool=0x4a0000000ULL,
        scratch_address=0x4b0000000ULL,input_address=0x4c0000000ULL,table=0x4d0000000ULL,
        pool_table=table+0x1000,other_pool_table=table+0x2000,entries=0x4e0000000ULL;
    static constexpr std::uintptr_t pair=scratch_address+0x20,output=scratch_address+0x60,line=doc+0xa0;
    static constexpr std::uintptr_t probe08=0xf008,probe30=0xf030,probe38=0xf038,probe48=0xf048,
        destroy78=0xf078,parse80=0xf080,release18=0xf118,track20=0xf120,other_release18=0xf218;
    std::array<std::uint8_t,0x308> document{},other_document{};
    std::array<std::array<std::uint8_t,0x90>,7> objects{};
    std::array<std::uint8_t,0xa0> scratch{};std::array<std::uint8_t,16> input{};
    std::array<std::uint8_t,0x88> vtable{};std::array<std::uint8_t,0x28> pool_vtable{},other_vtable{};
    std::array<std::uint8_t,32> allocation_entries{};
    std::vector<s::ReceiveBufferRegion> regions,extras;
    std::optional<std::pair<std::uintptr_t,std::size_t>> omitted;s::ReceiveBufferMemory memory{};
    Memory(){
        document.fill(0);other_document.fill(0);scratch.fill(0xa5);input.fill(0);input[0]='x';
        for(auto& o:objects)o.fill(0);
        for(unsigned slot:{8U,0x30U,0x38U,0x48U,0x78U,0x80U})put(vtable,slot,0xf000+slot);
        put(pool_vtable,0x18,release18);put(pool_vtable,0x20,track20);put(other_vtable,0x18,other_release18);
        refresh();
        for(auto a:{owner,child,element,declaration,first,last}){set(a,table);set(a+8,doc);}
        set(child+0x30,27,4);set(child+0x60,pool);set(pool,pool_table);set(other_pool,other_pool_table);
        set(doc+0xa0,7,4);set(doc+0xa8,entries);set(other_doc+0xa8,entries);
    }
    void add(std::uintptr_t a,std::span<std::uint8_t> b){
        if(!omitted||omitted->first>=a+b.size()||omitted->first+omitted->second<=a){regions.push_back({a,b});return;}
        const auto start=std::max(a,omitted->first),end=std::min(a+b.size(),omitted->first+omitted->second);
        if(start>a)regions.push_back({a,b.first(start-a)});if(end<a+b.size())regions.push_back({end,b.subspan(end-a)});
    }
    void refresh(){regions.clear();add(doc,document);add(other_doc,other_document);
        const std::array addresses{owner,child,element,declaration,first,last,pool};
        for(unsigned i=0;i<addresses.size();++i)add(addresses[i],objects[i]);
        add(other_pool,std::span(other_document).subspan(0x200,0x90));
        add(scratch_address,scratch);add(input_address,input);add(table,vtable);add(pool_table,pool_vtable);
        add(other_pool_table,other_vtable);add(entries,allocation_entries);for(auto r:extras)regions.push_back(r);memory.regions=regions;
    }
    std::uint64_t get(std::uintptr_t a,unsigned n=8){return word(memory.view(a,n),0,n);}
    void set(std::uintptr_t a,std::uint64_t v,unsigned n=8){put(memory.view(a,n),0,v,n);}
};
struct Nodes:s::NativeNodeClearEffects {
    struct Probe{std::uintptr_t target,node;};struct Destroy{std::uintptr_t target,node;std::uint32_t flags;};
    struct Release{std::uintptr_t target,pool,node;};struct Track{std::uintptr_t target,pool;};
    std::vector<Probe> probes;std::vector<Destroy> destroys;std::vector<Release> releases;std::vector<Track> tracks;
    std::function<std::uint64_t(std::uintptr_t,std::uintptr_t)> probe_hook;
    std::function<void(const Destroy&)> destroy_hook;std::function<void(const Release&)> release_hook;std::function<void(const Track&)> track_hook;
    std::uint64_t node_probe(std::uintptr_t t,std::uintptr_t n)override{probes.push_back({t,n});return probe_hook?probe_hook(t,n):(t==Memory::probe38?1:0);}
    void node_destroy(std::uintptr_t t,std::uintptr_t n,std::uint32_t f)override{destroys.push_back({t,n,f});if(destroy_hook)destroy_hook(destroys.back());}
    void pool_release(std::uintptr_t t,std::uintptr_t p,std::uintptr_t n)override{releases.push_back({t,p,n});if(release_hook)release_hook(releases.back());}
    void pool_track(std::uintptr_t t,std::uintptr_t p)override{tracks.push_back({t,p});if(track_hook)track_hook(tracks.back());}
    void remove_parent_003030(std::uintptr_t,std::uintptr_t)override{throw std::runtime_error("unexpected unlink helper");}
    void delete_node_003240(std::uintptr_t)override{throw std::runtime_error("unexpected generic delete helper");}
};
struct Lower:s::NativeNodeParseEffects {
    struct Identify{std::uintptr_t document,cursor,out;};struct Parse{std::uintptr_t target,node,cursor,pair,line;};
    struct Compare{std::uintptr_t left,right;std::uint64_t limit;};
    Memory& raw;std::vector<Identify> identifies;std::vector<Parse> parses;
    std::vector<std::uintptr_t> names,frees;std::vector<Compare> comparisons;std::vector<std::pair<std::uintptr_t,std::uintptr_t>> preparations;
    std::vector<s::NativeNodeParseError> errors;
    std::function<std::uintptr_t(const Identify&)> identify_hook;std::function<std::uintptr_t(const Parse&)> parse_hook;
    std::function<std::uintptr_t(std::uintptr_t)> name_hook;std::function<std::int32_t(const Compare&)> compare_hook;
    std::function<void(std::uintptr_t,std::uintptr_t)> prepare_hook;std::function<void(std::uintptr_t)> free_hook;
    std::function<void(const s::NativeNodeParseError&)> error_hook;
    explicit Lower(Memory& m):raw(m){}
    std::uintptr_t identify_0038b0(std::uintptr_t d,std::uintptr_t c,std::uintptr_t o)override{
        identifies.push_back({d,c,o});if(identify_hook)return identify_hook(identifies.back());raw.set(o,Memory::child);return c+3;
    }
    std::uintptr_t parse_child_virtual(std::uintptr_t t,std::uintptr_t n,std::uintptr_t c,std::uintptr_t p,std::uintptr_t l)override{
        parses.push_back({t,n,c,p,l});return parse_hook?parse_hook(parses.back()):Memory::input_address+1;
    }
    std::uintptr_t materialize_0035e0(std::uintptr_t p)override{names.push_back(p);return name_hook?name_hook(p):0xf777;}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{comparisons.push_back({a,b,n});return compare_hook?compare_hook(comparisons.back()):0;}
    void prepare_003aa0(std::uintptr_t o,std::uintptr_t c)override{preparations.push_back({o,c});if(prepare_hook)prepare_hook(o,c);}
    void free_04b0f8(std::uintptr_t p)override{frees.push_back(p);if(free_hook)free_hook(p);}
    void report_error(const s::NativeNodeParseError& e)override{errors.push_back(e);if(error_hook)error_hook(errors.back());else raw.set(e.document+0x6c,static_cast<std::uint32_t>(e.code),4);}
};
struct Fixture {
    Memory raw;Nodes nodes;Lower lower{raw};s::NativeNodeParseContext context{raw.memory,nodes,lower};
    std::uintptr_t parse(std::uintptr_t cursor=Memory::input_address,std::uintptr_t closing=0){return s::parse_node_004970(Memory::owner,cursor,closing,Memory::line,{Memory::scratch_address,Memory::pair},context);}
    void element(unsigned mode=0,bool nonempty=true){raw.set(Memory::element+0x68,mode,4);
        nodes.probe_hook=[](auto t,auto n){if(t==Memory::probe08&&n==Memory::child)return Memory::element;return t==Memory::probe38?std::uintptr_t{1}:std::uintptr_t{0};};
        lower.parse_hook=[&,nonempty](const auto& p){if(nonempty){raw.set(p.pair+8,0xfa01);raw.set(p.pair+0x10,0xfa02);}return Memory::input_address+1;};
    }
    void check_depth(std::uint32_t expected=0){check(raw.get(Memory::doc+0xa4,4)==expected,"normal epilog decrements live cached original-document depth");}
    void check_destroy(){check(nodes.destroys.size()==1&&nodes.destroys[0].target==Memory::destroy78&&nodes.destroys[0].node==Memory::child&&!nodes.destroys[0].flags,"source uses explicit child destructor target and zero flags");check(nodes.releases.size()==1&&nodes.releases[0].target==Memory::release18&&nodes.releases[0].pool==Memory::pool&&nodes.releases[0].node==Memory::child,"source releases cached pool through live release target");}
};
} // namespace node_parse_test
