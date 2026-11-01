// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/node_string_test_fixture.hpp"
#include "tests/support/socket/node_constructors_test_fixture.hpp"
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
struct Effects:s::NodeStringEffects {
    Memory& raw;node_string_test::Lower strings;node_parse_test::Lower primitives;std::vector<node_parse_test::Lower::Parse> others;
    std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    std::function<std::uintptr_t(std::uint64_t)> allocate_hook;std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copy_hook;
    std::function<std::uintptr_t(const node_parse_test::Lower::Parse&)> other_hook;
    std::vector<s::NativeNodeParseError> errors;unsigned workspaces=0;
    Effects(Memory& m,node_parse_test::Nodes& n,const s::NativeErrorArgumentTail& a,s::PoolAllocationDispatch& p):NodeStringEffects(m.memory,n,a,p),raw(m),strings(m.memory),primitives(m){primitives.compare_hook=[](const auto&){return 1;};}
    s::NativeNodeParseScratch acquire_parse_scratch(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{++workspaces;return {Memory::scratch_address,Memory::pair};}
    std::uintptr_t parse_other_virtual(std::uintptr_t t,std::uintptr_t n,std::uintptr_t c,std::uintptr_t p,std::uintptr_t l)override{others.push_back({t,n,c,p,l});return other_hook?other_hook(others.back()):0;}
    std::uintptr_t decode_0033c0(std::uintptr_t p,std::span<std::uint8_t,10> b,std::int32_t& n)override{return strings.decode_0033c0(p,b,n);}
    void copy_string_078d30(std::uintptr_t p,std::span<const std::uint8_t,10> b,std::uint64_t n)override{strings.copy_string_078d30(p,b,n);}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{return primitives.compare_05c550(a,b,n);}
    void report_error(const s::NativeNodeParseError& e)override{errors.push_back(e);raw.set(e.document+0x6c,static_cast<std::uint32_t>(e.code),4);}
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
    node_string_test::Fixture literals;unsigned copies=0;std::size_t input_count=1;
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
        raw.extras.push_back({node_string_test::Fixture::table,literals.entities});raw.extras.push_back({node_string_test::Fixture::names,literals.names_bytes});for(unsigned i=0;i<5;++i)raw.extras.push_back({tables[i],node_vtables[i]});for(unsigned i=0;i<4;++i)raw.extras.push_back({pool_tables[i],pool_vtables[i]});raw.refresh();}
    void recycled(unsigned k){raw.set(Memory::doc+offsets[k]+0x68,Memory::child);raw.objects[1].fill(0xa5);raw.set(Memory::child,0);}
    std::uintptr_t construct(unsigned k){const auto p=Memory::doc+offsets[k];switch(k){case 0:return effects.construct_001590(Memory::doc,p);case 1:return effects.construct_001610(Memory::doc,p);case 2:return effects.construct_001690(Memory::doc,p);case 3:return effects.construct_001720(Memory::doc,p);default:return effects.construct_0017b0(Memory::doc,p);}}
    int parse(std::string_view v){input_count=v.size();raw.input.fill(0);std::copy(v.begin(),v.end(),raw.input.begin());return s::parse_native_document_004280(Memory::doc,Memory::input_address,v.size(),document);}
};
void final_materialize_constructor_chain(){
    for(unsigned k=0;k<5;++k){Fixture f;f.recycled(k);f.construct(k);const auto pool=Memory::doc+Fixture::offsets[k];
        std::copy_n(" \tx\r\ny ",7,f.copied.begin());f.raw.set(Memory::child+0x18,0x106,4);f.raw.set(Memory::child+0x20,Fixture::copied_address);f.raw.set(Memory::child+0x28,Fixture::copied_address+7);
        check(f.effects.materialize_0035e0(Memory::child+0x18)==Fixture::copied_address+2,"final materialize bridge accepts actual source-constructed pair and returns trimmed mapped start");
        check(std::string(reinterpret_cast<const char*>(f.copied.data()+2))=="x y"&&f.raw.get(Memory::child+0x18,4)==0,"actual constructor pair composes newline and whitespace materialization");
        f.effects.prepare_003aa0(Memory::owner,Memory::child);
        check(!f.raw.get(Memory::doc+0x104,4)&&!f.raw.get(pool+0x7c,4)&&f.raw.get(pool+0x70,4)==1,"inherited insertion still removes vector/track without freeing source-constructed materialized node");
        check(f.nodes.destroys.empty()&&f.nodes.releases.empty()&&f.effects.frees.empty(),"materialization/insertion adds no destructor/release/heap free");
    }
}
void parser_materializes_names(bool mismatch){
    Fixture f;f.recycled(2);const auto text=Fixture::copied_address;
    f.effects.primitives.compare_hook=[&](const auto& c){if(c.right==node_identify_test::Fixture::token(4))return 0;if(c.limit==0x7fffffff){check(c.left==text+17&&c.right==text+34,"004970 compares returned closing and element materialized names");return mismatch?1:0;}return 1;};
    f.nodes.probe_hook=[](auto t,auto n){if(t==Memory::probe08&&n==Memory::child)return Memory::child;return t==Memory::probe38?std::uintptr_t{1}:std::uintptr_t{0};};
    f.effects.other_hook=[&](const auto& p){
        const std::string_view element="  node  ",closing=mismatch?" other ":" node ";std::copy(element.begin(),element.end(),f.copied.begin()+32);std::copy(closing.begin(),closing.end(),f.copied.begin()+16);
        f.raw.set(Memory::child+0x18,0x104,4);f.raw.set(Memory::child+0x20,text+32);f.raw.set(Memory::child+0x28,text+32+element.size());
        f.raw.set(p.pair,0x104,4);f.raw.set(p.pair+8,text+16);f.raw.set(p.pair+0x10,text+16+closing.size());return text+2;
    };
    const auto code=f.parse("<x");
    check(code==(mismatch?14:0)&&f.effects.workspaces==1&&f.effects.others.size()==1,"actual document/entry/Identify/constructor/004970 chain uses final GetStr bridge");
    check(std::string(reinterpret_cast<const char*>(f.copied.data()+34))=="node"&&std::string(reinterpret_cast<const char*>(f.copied.data()+17))==(mismatch?"other":"node"),"both element and closing names are independently trimmed in place");
    check(f.raw.get(Memory::child+0x18,4)==0&&f.raw.get(Memory::pair,4)==0,"materialized pair flags are cleared without ownership flag fabrication");
    if(mismatch){check(f.effects.errors.size()==1&&f.effects.errors[0].code==14&&f.effects.errors[0].has_pointer&&f.effects.errors[0].pointer==text+34,"mismatch report receives pointer from subsequent early materialize call");check(f.nodes.destroys.size()==1&&f.nodes.releases.size()==1&&f.nodes.tracks.empty(),"existing parser mismatch cleanup remains separate from materialization/insertion");}
    else{check(f.effects.errors.empty()&&f.raw.get(Memory::child+0x10)==Memory::doc&&!f.raw.get(Memory::doc+0x104,4)&&f.nodes.tracks.size()==1,"matching materialized names reach actual insertion and owner append");check(f.nodes.destroys.empty()&&f.nodes.releases.empty()&&!f.raw.get(Memory::doc+0xa4,4),"matching path preserves child and restores parser depth");}
}
void named_numeric_final_bridge(){
    {Fixture f;std::copy_n("&amp;!",6,f.copied.begin());f.raw.set(Memory::pair,0x101,4);f.raw.set(Memory::pair+8,Fixture::copied_address);f.raw.set(Memory::pair+16,Fixture::copied_address+6);
        f.effects.primitives.compare_hook=[&](const auto& c){if(c.right==node_string_test::Fixture::names)return 1;check(c.right==node_string_test::Fixture::names+8&&c.limit==3,"final bridge forwards exact table pointer and bounded compare length");return 0;};
        check(f.effects.materialize_0035e0(Memory::pair)==Fixture::copied_address&&std::string(reinterpret_cast<const char*>(f.copied.data()))=="&!","final bridge decodes retained entity identities through controlled compare");
        check(f.effects.primitives.comparisons.size()==2&&f.effects.strings.decodes.empty(),"selected named path retains mandatory compare without inventing numeric decoder");
    }
    {Fixture f;std::copy_n("&#x;!",5,f.copied.begin());f.raw.set(Memory::pair,0x101,4);f.raw.set(Memory::pair+8,Fixture::copied_address);f.raw.set(Memory::pair+16,Fixture::copied_address+5);
        f.effects.strings.decode_hook=[](auto p,auto b,auto& n){check(n==0&&b[9]==0,"final bridge resets typed ten-byte decoder workspace");b[0]='N';n=1;return p+4;};
        check(f.effects.materialize_0035e0(Memory::pair)==Fixture::copied_address&&std::string(reinterpret_cast<const char*>(f.copied.data()))=="N!","final bridge delegates numeric decode/copy while preserving actual GetStr cursor state");
        check(f.effects.strings.decodes.size()==1&&f.effects.strings.copies.size()==1&&f.effects.strings.copies[0].count==1,"no native decoder/copy recovery is implied by supplied effects");
    }
}
}
int main(){try{final_materialize_constructor_chain();parser_materializes_names(false);parser_materializes_names(true);named_numeric_final_bridge();std::cout<<"PASS: "<<checks<<" node-string integration checks; final GetStr bridge with actual constructors/insertion and document/Identify/004970 matching/mismatch name paths; numeric/copy/compare/classifier and nativeABI remain supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
