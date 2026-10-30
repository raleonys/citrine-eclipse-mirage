#include "tests/support/socket/node_identify_test_fixture.hpp"
using namespace node_identify_test;
namespace {
struct Effects:s::RecoveredNodeIdentifyEffects {
    Memory& raw;Constructors constructors;node_parse_test::Lower primitives;
    std::vector<node_parse_test::Lower::Parse> others;std::vector<std::uint32_t> classified;
    std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    std::function<std::uintptr_t(std::uint64_t)> allocate_hook;
    std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copy_hook;
    std::function<std::uintptr_t(const node_parse_test::Lower::Parse&)> other_hook;
    unsigned workspaces=0;
    Effects(Memory& m,node_parse_test::Nodes& n,const s::NativeErrorArgumentTail& a):RecoveredNodeIdentifyEffects(m.memory,n,a),raw(m),primitives(m){primitives.compare_hook=[](const auto&){return 1;};}
    s::NativeNodeParseScratch acquire_parse_scratch(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{++workspaces;return {Memory::scratch_address,Memory::pair};}
    std::uintptr_t parse_other_virtual(std::uintptr_t t,std::uintptr_t n,std::uintptr_t c,std::uintptr_t p,std::uintptr_t l)override{others.push_back({t,n,c,p,l});return other_hook?other_hook(others.back()):0;}
    std::uintptr_t materialize_0035e0(std::uintptr_t p)override{return primitives.materialize_0035e0(p);}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{return primitives.compare_05c550(a,b,n);}
    void prepare_003aa0(std::uintptr_t o,std::uintptr_t n)override{primitives.prepare_003aa0(o,n);}
    void report_error(const s::NativeNodeParseError& e)override{raw.set(e.document+0x6c,static_cast<std::uint32_t>(e.code),4);}
    std::int32_t classify_051fc0(std::uint32_t b)override{classified.push_back(b);return parse_entry_test::controlled_space(b);}
    void free_04b0f8(std::uintptr_t p)override{frees.push_back(p);}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{allocations.push_back(n);if(!allocate_hook)throw std::runtime_error("missing supplied input allocation");return allocate_hook(n);}
    void copy_078d30(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{if(!copy_hook)throw std::runtime_error("missing supplied input copy");copy_hook(a,b,n);}
    void format_header_0060a0(std::uintptr_t,std::uint64_t,std::uintptr_t,std::uintptr_t,std::int32_t,std::uint32_t,std::uint32_t)override{throw std::runtime_error("unexpected supplied diagnostic formatting");}
    void format_separator_0060a0(std::uintptr_t,std::uint64_t,std::uintptr_t)override{throw std::runtime_error("unexpected separator");}
    void format_detail_006110(std::uintptr_t,std::uint64_t,std::uintptr_t,std::uintptr_t)override{throw std::runtime_error("unexpected detail formatter");}
    std::uintptr_t construct_001610(std::uintptr_t d,std::uintptr_t p)override{return constructors.construct_001610(d,p);}
    std::uintptr_t construct_001590(std::uintptr_t d,std::uintptr_t p)override{return constructors.construct_001590(d,p);}
    std::uintptr_t construct_001720(std::uintptr_t d,std::uintptr_t p)override{return constructors.construct_001720(d,p);}
    std::uintptr_t construct_0017b0(std::uintptr_t d,std::uintptr_t p)override{return constructors.construct_0017b0(d,p);}
    std::uintptr_t construct_001690(std::uintptr_t d,std::uintptr_t p)override{return constructors.construct_001690(d,p);}
};
struct Fixture {
    static constexpr std::uintptr_t copied_address=0x510000000ULL,document_table=0x18007fec8ULL;
    Memory raw;node_parse_test::Nodes nodes;s::NativeErrorArgumentTail arguments{0xff00};Effects effects{raw,nodes,arguments};
    s::NativeMessageDocument projection;s::NativeDocumentContext document{raw.memory,effects};
    std::array<std::uint8_t,64> copied{};std::array<std::uint8_t,0x88> table{};std::array<std::uint8_t,40> tokens{};
    std::size_t input_count=1;unsigned copies=0;
    Fixture(){put(table,0x80,0x180004970);for(unsigned i=0;i<5;++i)put(tokens,i*8,node_identify_test::Fixture::token(i));refresh();
        check(s::construct_native_document_001830(Memory::doc,1,0,document)==Memory::doc,"actual001830 constructor initializes mapped document and self-owner");
        effects.allocate_hook=[&](auto n){check(n==input_count+1,"actual004280 requests supplied input storage count+1");return copied_address;};
        effects.copy_hook=[&](auto a,auto b,std::uint64_t n){++copies;check(a==copied_address&&b==Memory::input_address&&n==input_count,"actual004280 copies exact input operands");auto to=raw.memory.view(a,n),from=raw.memory.view(b,n);std::memcpy(to.data(),from.data(),static_cast<std::size_t>(n));};
        effects.other_hook=[&](const auto&){return copied_address+input_count;};
    }
    void refresh(){raw.omitted={{Memory::doc+0x6c,4}};raw.extras={{Memory::doc+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{copied_address,copied},{document_table,table},{node_identify_test::Fixture::slots,tokens}};raw.refresh();}
    int parse(std::string_view v){input_count=v.size();raw.input.fill(0);std::copy(v.begin(),v.end(),raw.input.begin());return s::parse_native_document_004280(Memory::doc,Memory::input_address,v.size(),document);}
};
void source_chain(){
    struct Case{std::string_view input;unsigned match,address,pool,advance;};
    const std::array cases{Case{"x",5,0x1720,0x208,0},Case{"<?x",0,0x1610,0x288,2},Case{"123456789x",2,0x1720,0x208,9},Case{"<x",4,0x1690,0x108,1}};
    for(const auto& c:cases){Fixture f;f.effects.primitives.compare_hook=[&](const auto& v){return c.match<5&&v.right==node_identify_test::Fixture::token(c.match)?0:1;};
        check(f.parse(c.input)==0&&f.projection.error_6c==0&&f.effects.workspaces==1&&f.effects.others.size()==1,"actual001830/004280/0041b0/selected004970 reaches final0038b0 source and unknown child provider");
        check(f.effects.constructors.calls.size()==1&&f.effects.constructors.calls[0].address==c.address&&f.effects.constructors.calls[0].document==Memory::doc&&f.effects.constructors.calls[0].pool==Memory::doc+c.pool,"final Identify provider routes exact selected constructor and pool through mandatory lower");
        const auto p=f.effects.others[0];check(p.target==Memory::parse80&&p.node==Memory::child&&p.cursor==Fixture::copied_address+c.advance&&p.pair==Memory::pair&&p.line==Memory::line,"new Identify returns separate node/cursor while selected parser preserves target/workspace/line identities");
        check(f.raw.get(Memory::child+0x30,4)==1&&f.raw.get(Memory::doc+0x38)==Memory::child&&f.raw.get(Memory::doc+0x40)==Memory::child&&f.raw.get(Memory::child+0x10)==Memory::doc&&!f.raw.get(Memory::doc+0xa4,4),"existing parser appends identified controlled node and restores depth");
        check(f.effects.allocations==std::vector<std::uint64_t>{c.input.size()+1}&&f.copies==1&&f.effects.frees==std::vector<std::uintptr_t>{0}&&f.nodes.destroys.empty(),"successful finalprovider chain needs only supplied input storage and no invented constructor heap/cleanup");
        check(f.raw.objects[1][0x68]==(c.match==2?1:0)&&f.raw.memory.view(Memory::doc+0x6c,4).data()==reinterpret_cast<std::uint8_t*>(&f.projection.error_6c),"CDATA flag and shared error projection remain coherent across actual source composition");
    }
}
void direct_provider_and_boundaries(){
    {Fixture f;f.raw.input.fill(0);std::copy_n(" \nx",3,f.raw.input.begin());f.raw.set(Memory::line,17,4);f.effects.constructors.hook=[&](const auto&){f.raw.set(Memory::line,55,4);return Memory::child;};
        check(f.effects.identify_0038b0(Memory::doc,Memory::input_address,node_identify_test::Fixture::out)==Memory::input_address&&f.raw.get(Memory::line,4)==17&&f.raw.get(Memory::child+0x30,4)==55,"final inherited Identify bridge preserves fallback original cursor and rewinds original line after live node line");}
    {Fixture f;f.raw.input.fill(0);f.raw.input[0]=' ';f.raw.input[1]='\n';f.raw.set(Memory::line,8,4);check(f.effects.identify_0038b0(Memory::doc,Memory::input_address,node_identify_test::Fixture::out)==Memory::input_address+2&&!f.raw.get(node_identify_test::Fixture::out)&&f.raw.get(Memory::line,4)==9&&f.effects.constructors.calls.empty(),"final Identify trimmed-NUL output stays separate from input and requires no constructor");}
    {Fixture f;check(f.effects.parse_child_virtual(0xf999,Memory::child,0x123,0x456,0x789)==Fixture::copied_address+1&&f.effects.workspaces==0&&f.effects.constructors.calls.empty(),"N25 unknown virtual target remains mandatory and does not falsely execute Identify source");}
    {Fixture f;f.effects.constructors.hook=[](const auto&)->std::uintptr_t{throw Failure{};};fails([&]{(void)f.parse("x");});check(f.effects.workspaces==1&&f.raw.get(Memory::doc+0xa4,4)==1&&f.raw.get(Memory::doc+0x98)==Fixture::copied_address&&f.effects.others.empty()&&f.nodes.destroys.empty(),"constructor failure leaves actual copied input/parser depth prefix without invented rollback or unknown child call");}
}
}
int main(){try{source_chain();direct_provider_and_boundaries();std::cout<<"PASS: "<<checks<<" node-identify integration checks; actual001830/004280/0041b0/selected004970/final0038b0 plus typed supplied constructors,shared error,unknown targets and failure prefixes; constructor/DOM/nativeABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
