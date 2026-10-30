#include "tests/support/socket/node_parse_test_fixture.hpp"
using namespace node_parse_test;
namespace {
struct Effects:s::RecoveredNodeParseEffects {
    Memory& raw;Lower primitive;std::vector<s::NativeNodeParseScratch> workspaces;
    std::vector<Lower::Parse> others;std::vector<s::NativeNodeParseError> diagnostics;
    std::vector<std::uintptr_t> freed;std::vector<std::uint64_t> allocated;
    std::vector<document_error_test::Lower::Format> details;std::vector<document_error_test::Lower::Header> headers;
    std::function<std::uintptr_t(const Lower::Parse&)> other_hook;
    std::function<s::NativeNodeParseScratch()> scratch_hook;
    std::function<std::uintptr_t(std::uint64_t)> allocation_hook;
    std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copy_hook;
    std::function<void(const s::NativeNodeParseError&)> error_hook;
    unsigned classified=0;std::uintptr_t active_argument=0;const s::NativeNodeParseError* active_error=nullptr;
    Effects(Memory& m,Nodes& n,const s::NativeErrorArgumentTail& a):RecoveredNodeParseEffects(m.memory,n,a),raw(m),primitive(m){}
    s::NativeNodeParseScratch acquire_parse_scratch(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)override{
        const auto s=scratch_hook?scratch_hook():s::NativeNodeParseScratch{Memory::scratch_address,Memory::pair};workspaces.push_back(s);return s;
    }
    std::uintptr_t parse_other_virtual(std::uintptr_t t,std::uintptr_t n,std::uintptr_t c,std::uintptr_t p,std::uintptr_t l)override{others.push_back({t,n,c,p,l});return other_hook?other_hook(others.back()):0;}
    std::uintptr_t identify_0038b0(std::uintptr_t d,std::uintptr_t c,std::uintptr_t o)override{return primitive.identify_0038b0(d,c,o);}
    std::uintptr_t materialize_0035e0(std::uintptr_t p)override{return primitive.materialize_0035e0(p);}
    std::int32_t compare_05c550(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{return primitive.compare_05c550(a,b,n);}
    void prepare_003aa0(std::uintptr_t o,std::uintptr_t c)override{primitive.prepare_003aa0(o,c);}
    std::int32_t classify_051fc0(std::uint32_t b)override{++classified;return parse_entry_test::controlled_space(b);}
    void free_04b0f8(std::uintptr_t p)override{freed.push_back(p);}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{allocated.push_back(n);if(!allocation_hook)throw std::runtime_error("missing supplied allocation");return allocation_hook(n);}
    void copy_078d30(std::uintptr_t a,std::uintptr_t b,std::uint64_t n)override{if(!copy_hook)throw std::runtime_error("missing supplied copy");copy_hook(a,b,n);}
    void report_error(const s::NativeNodeParseError& e)override{
        diagnostics.push_back(e);if(error_hook){error_hook(diagnostics.back());return;}
        // This explicit transport supplies the invocation context. It does not
        // claim that the inherited four-argument bridge captured native varargs.
        active_error=&e;active_argument=0xf0000000ULL+diagnostics.size();
        s::NativeDocumentErrorContext c{raw.memory,*this,*this};
        s::set_document_error_005820(e.document,e.code,e.line,e.format,{active_argument},c);
        active_error=nullptr;
    }
    void text(std::uintptr_t d,std::string_view v){auto bytes=raw.memory.view(d,v.size()+1);std::memcpy(bytes.data(),v.data(),v.size());bytes[v.size()]=0;}
    void format_header_0060a0(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t n,std::int32_t id,std::uint32_t h,std::uint32_t l)override{headers.push_back({d,c,f,n,id,h,l});text(d,"controlled diagnostic");}
    void format_separator_0060a0(std::uintptr_t d,std::uint64_t,std::uintptr_t)override{text(d,": ");}
    void format_detail_006110(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t a)override{details.push_back({d,c,f,a});check(active_error&&a==active_argument&&f==active_error->format,"N25 typed error uses explicitly established N24 argument-tail context");text(d,active_error->has_pointer?"controlled pointer detail":"controlled no-argument detail");}
};
struct Fixture {
    static constexpr std::uintptr_t copy_address=0x510000000ULL,temporary=0x520000000ULL,owned=0x530000000ULL,
        document_table=0x18007fec8ULL,names_table=0x1800a1010ULL;
    Memory raw;Nodes nodes;s::NativeErrorArgumentTail arguments{0xfe00};Effects effects{raw,nodes,arguments};
    s::NativeMessageDocument projection;s::NativeDocumentContext document{raw.memory,effects};
    std::array<std::uint8_t,64> copied{};std::array<std::uint8_t,1024> temp{},result{};
    std::array<std::uint8_t,0x88> table{};std::array<std::uint8_t,19*8> names{};
    bool input_pending=false;std::size_t input_count=1;
    Fixture(){put(table,0x80,0x180004970);for(unsigned n=0;n<19;++n)put(names,n*8,0xf100+n);refresh();
        check(s::construct_native_document_001830(Memory::doc,1,0,document)==Memory::doc,"actual constructor source supplies document bytes and self owner");
        effects.allocation_hook=[&](std::uint64_t n){if(input_pending){input_pending=false;check(n==input_count+1,"actual004280 allocates input count+1");return copy_address;}return n==1000?temporary:owned;};
        effects.copy_hook=[&](auto a,auto b,std::uint64_t n){auto to=raw.memory.view(a,n),from=raw.memory.view(b,n);std::memmove(to.data(),from.data(),static_cast<std::size_t>(n));};
        effects.other_hook=[](const auto&){return copy_address+1;};
    }
    void refresh(){raw.omitted={{Memory::doc+0x6c,4}};raw.extras={{Memory::doc+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{copy_address,copied},{temporary,temp},{owned,result},{document_table,table},{names_table,names}};raw.refresh();}
    int parse(std::string_view input="x"){input_count=input.size();std::fill(raw.input.begin(),raw.input.end(),0);std::copy(input.begin(),input.end(),raw.input.begin());input_pending=!input.empty()&&input[0];return s::parse_native_document_004280(Memory::doc,Memory::input_address,input.size(),document);}
    void element(unsigned mode){raw.set(Memory::element+0x68,mode,4);nodes.probe_hook=[](auto t,auto n)->std::uint64_t{if(t==Memory::probe08&&n==Memory::child)return Memory::element;return t==Memory::probe38?1:0;};}
};
void actual_source_chain(){
    {Fixture f;check(f.parse()==0&&f.projection.error_6c==0&&f.effects.workspaces.size()==1&&f.effects.others.size()==1,"actual001830/004280/0041b0 enters selected004970 source and delegates unknown child target");const auto p=f.effects.others[0];check(p.target==Memory::parse80&&p.node==Memory::child&&p.cursor==Fixture::copy_address+3&&p.pair==Memory::pair&&p.line==Memory::doc+0xa0,"selected parser forwards separate Identify cursor and line-pointer to heterogeneous lower target");check(f.raw.get(Memory::doc+0x38)==Memory::child&&f.raw.get(Memory::doc+0x40)==Memory::child&&f.raw.get(Memory::child+0x10)==Memory::doc&&!f.raw.get(Memory::doc+0xa4,4),"selected source appends controlled child and restores depth");check(f.effects.allocated==std::vector<std::uint64_t>{2}&&f.effects.headers.empty()&&f.effects.freed==std::vector<std::uintptr_t>{0},"successful chain does not invent error formatting or scratch heap allocation");}
    {Fixture f;f.effects.other_hook=[](const auto&){return 0;};check(f.parse()==15&&f.projection.error_6c==15&&f.effects.diagnostics.size()==1&&f.effects.diagnostics[0].code==15&&!f.effects.diagnostics[0].has_pointer,"failed supplied child parse reaches actual005820 via explicit error transport");check(f.effects.headers.size()==1&&f.effects.headers[0].id==15&&f.effects.headers[0].line==27&&f.effects.details.empty(),"actual error source supplies cached child line and null detail formatting");check(f.nodes.destroys.size()==1&&f.nodes.releases.size()==1&&!f.raw.get(Memory::doc+0xa4,4)&&!f.raw.get(Memory::doc+0x38),"failed child is destroyed/released before wrapper clears four empty pools");check(f.raw.get(Memory::doc+0x78,4)==0x200&&f.raw.get(Memory::doc+0x80)==Fixture::owned&&f.raw.get(Memory::doc+0x88)==Fixture::owned+21,"actual diagnostic body publishes separately owned string");}
    {Fixture f;f.element(1);f.effects.other_hook=[&](const auto& p){f.raw.set(p.pair+8,0xf001);f.raw.set(p.pair+16,0xf002);return Fixture::copy_address+1;};f.effects.primitive.name_hook=[](auto){return 0xf999;};check(f.parse()==14&&f.effects.diagnostics[0].has_pointer&&f.effects.diagnostics[0].pointer==0xf999&&f.effects.details.size()==1,"N25 mismatch pointer argument is explicitly carried into N24 detail context");check(f.effects.headers[0].line==27&&f.effects.details[0].format==0x180080018&&!f.raw.get(Memory::doc+0xa4,4),"mismatch chain retains cached line,format identity and normal depth epilog");}
    {Fixture f;f.effects.primitive.prepare_hook=[&](auto,auto){f.projection.error_6c=37;};check(f.parse()==37&&f.nodes.destroys.size()==1&&f.nodes.releases.size()==1&&!f.raw.get(Memory::doc+0x38)&&!f.raw.get(Memory::doc+0x40),"wrapper observes shared error after append and executes existing source node cleanup");check(f.raw.memory.view(Memory::doc+0x6c,4).data()==reinterpret_cast<std::uint8_t*>(&f.projection.error_6c),"native message projection shares actual mapped error cell without writeback copy");}
}
void selected_fallback_recursion_and_failures(){
    {Fixture f;check(f.effects.parse_child_virtual(0xf000,Memory::child,0xabc,0xdef,0x123)==Fixture::copy_address+1&&f.effects.workspaces.empty()&&f.effects.others.size()==1,"unknown identity stays mandatory lower without acquiring source workspace");const auto p=f.effects.others[0];check(p.target==0xf000&&p.cursor==0xabc&&p.pair==0xdef&&p.line==0x123,"unknown target fallback preserves all observed argument identities");}
    {Fixture f;f.raw.set(Memory::doc+0xa4,99,4);f.raw.set(Memory::doc+0x98,Fixture::copy_address);f.copied[0]='x';f.effects.parse_virtual(0x180004970,Memory::doc,Fixture::copy_address,0,Memory::line);check(f.projection.error_6c==18&&f.effects.diagnostics[0].code==18&&f.effects.details.size()==1&&f.effects.workspaces.size()==1&&f.effects.others.empty()&&f.raw.get(Memory::doc+0xa4,4)==99,"selected virtual entry discards return while depth100 path uses actual error source and restores depth");}
    {Fixture f;unsigned acquire=0;f.effects.scratch_hook=[&]{++acquire;return s::NativeNodeParseScratch{Memory::scratch_address+(acquire-1)*0x100,Memory::pair+(acquire-1)*0x100};};
        std::array<std::uint8_t,0x80> nested{};f.raw.extras.push_back({Memory::scratch_address+0x100,nested});f.raw.refresh();
        f.effects.primitive.identify_hook=[&](const auto& a){if(f.effects.primitive.identifies.size()==1)f.raw.set(a.out,Memory::child);return a.cursor;};
        put(f.raw.vtable,0x80,0x180004970);f.effects.parse_virtual(0x180004970,Memory::doc,Memory::input_address,0,Memory::line);
        check(acquire==2&&f.effects.workspaces.size()==2&&f.effects.others.empty()&&f.effects.primitive.identifies.size()==2,"selected known target recursively reenters source with separate caller-owned invocation workspace");check(f.projection.error_6c==15&&!f.raw.get(Memory::doc+0xa4,4),"nested Identify null output returns0 and outer source reports child failure normally");}
    {Fixture f;f.effects.scratch_hook=[]()->s::NativeNodeParseScratch{throw Failure{};};fails([&]{(void)f.parse();});check(!f.raw.get(Memory::doc+0xa4,4)&&f.raw.get(Memory::doc+0x98)==Fixture::copy_address&&f.effects.others.empty(),"workspace acquisition failure occurs at explicit source-adapter boundary after input copy,before native depth projection");}
    {Fixture f;f.effects.other_hook=[](const auto&)->std::uintptr_t{throw Failure{};};fails([&]{(void)f.parse();});check(f.raw.get(Memory::doc+0xa4,4)==1&&f.nodes.destroys.empty()&&f.effects.diagnostics.empty(),"unknown child failure leaves committed selected source depth/scratch prefix without invented cleanup");}
    {Fixture f;f.effects.other_hook=[](const auto&){return 0;};f.effects.error_hook=[](const auto&){throw Failure{};};fails([&]{(void)f.parse();});check(f.nodes.destroys.size()==1&&f.nodes.releases.size()==1&&f.raw.get(Memory::doc+0xa4,4)==1&&f.effects.headers.empty(),"explicit diagnostic transport failure follows child cleanup but prevents normal depth epilog");}
}
}
int main(){try{actual_source_chain();selected_fallback_recursion_and_failures();std::cout<<"PASS: "<<checks<<" node-parse integration checks; actual001830/004280/0041b0/selected004970 plus explicit005820 transport,shared error/cleanup,unknown fallback,recursive workspace and failure prefixes; primitives/DOM/nativeABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
