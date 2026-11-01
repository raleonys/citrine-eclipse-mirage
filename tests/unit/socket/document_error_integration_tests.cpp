// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/document_error_test_fixture.hpp"
using namespace document_error_test;
namespace {
struct Effects:s::DocumentErrorEffects {
    Operations& ops;Lower& lower;
    std::function<std::uintptr_t(std::uint64_t)> input_allocator;
    std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> input_copier;
    std::function<void(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uintptr_t)> virtual_hook;
    std::function<std::int32_t(std::uint32_t)> classifier;
    std::vector<std::uint32_t> classified;unsigned virtuals=0;
    Effects(s::ReceiveBufferMemory& m,s::NativeNodeClearEffects& n,const s::NativeErrorArgumentTail& a,Operations& o,Lower& l):DocumentErrorEffects(m,n,a),ops(o),lower(l){}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{if(input_allocator)return input_allocator(n);return ops.allocate(n);}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{if(input_copier)input_copier(to,from,n);else ops.copy(to,from,n);}
    void free_04b0f8(std::uintptr_t p)override{ops.free(p);}
    std::int32_t classify_051fc0(std::uint32_t b)override{classified.push_back(b);return classifier?classifier(b):parse_entry_test::controlled_space(b);}
    void parse_virtual(std::uintptr_t t,std::uintptr_t d,std::uintptr_t c,std::uintptr_t z,std::uintptr_t l)override{++virtuals;if(virtual_hook)virtual_hook(t,d,c,z,l);}
    void format_header_0060a0(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t n,std::int32_t i,std::uint32_t h,std::uint32_t l)override{lower.format_header_0060a0(d,c,f,n,i,h,l);}
    void format_separator_0060a0(std::uintptr_t d,std::uint64_t c,std::uintptr_t f)override{lower.format_separator_0060a0(d,c,f);}
    void format_detail_006110(std::uintptr_t d,std::uint64_t c,std::uintptr_t f,std::uintptr_t a)override{lower.format_detail_006110(d,c,f,a);}
};
struct Fixture {
    static constexpr std::uintptr_t input_address=0x730000000ULL,copy_address=0x830000000ULL,table_address=0x18007fec8ULL,target=0xf404;
    static constexpr std::array<unsigned,4> offsets{0x108,0x188,0x208,0x288};
    static constexpr std::uintptr_t array(unsigned k){return 0x930000000ULL+k*0x1000;}
    static constexpr std::uintptr_t block(unsigned k,unsigned n){return 0xa30000000ULL+k*0x1000+n*0x100;}
    Memory raw;Operations ops{raw};Lower lower{ops};pool_clear_test::Nodes nodes;s::NativeErrorArgumentTail arguments{0xf505};
    Effects effects{raw.memory,nodes,arguments,ops,lower};s::NativeDocumentContext document{raw.memory,effects};s::NativeMessageDocument projection;
    std::vector<std::uint8_t> input,copy;std::array<std::uint8_t,0x88> table{};std::array<std::array<std::uint8_t,24>,4> arrays{};
    unsigned input_allocations=0,input_copies=0;bool input_pending=false;
    explicit Fixture(std::vector<std::uint8_t> bytes={}):input(std::move(bytes)),copy(input.size()+1,0xcc){put(table,0x80,target);refresh();check(s::construct_native_document_001830(Memory::document_address,1,0,document)==Memory::document_address,"actual document constructor source binds shared error and fixture vtable identity");}
    void refresh(){raw.omitted={{Memory::document_address+0x6c,4}};raw.extras={{Memory::document_address+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{input_address,input},{copy_address,copy},{table_address,table}};for(unsigned k=0;k<4;++k)raw.extras.push_back({array(k),arrays[k]});raw.refresh();}
    void seed_pools(){constexpr unsigned counts[]{1,0,2,1};for(unsigned k=0;k<4;++k){auto off=offsets[k];raw.set(off+8,array(k));raw.set(off+0x60,3,4);raw.set(off+0x64,counts[k],4);for(unsigned at:{0x68U,0x70U,0x78U})raw.set(off+at,0x123456789ULL+at);for(unsigned n=0;n<counts[k];++n)put(arrays[k],n*8,block(k,n));}}
    int parse(){input_pending=!input.empty()&&input[0]!=0;effects.input_allocator=[&](std::uint64_t n){if(input_pending){input_pending=false;++input_allocations;check(n==input.size()+1,"004280 source requests count+1 input storage");return copy_address;}return ops.allocate(n);};effects.input_copier=[&](auto to,auto from,std::uint64_t n){if(to==copy_address&&from==input_address){++input_copies;check(n==input.size(),"004280 source copies exact input count");std::memcpy(copy.data(),input.data(),static_cast<std::size_t>(n));}else ops.copy(to,from,n);};return s::parse_native_document_004280(Memory::document_address,input_address,input.size(),document);}
};
void copied_empty_entry_and_cleanup(){
    for(const auto& bytes:std::array<std::vector<std::uint8_t>,3>{{{' ','\n'},{0xef,0xbb,0xbf},{'\t',' ','\r'}}}){
        Fixture f(bytes);f.lower.header_hook=[&]{f.seed_pools();};f.ops.free_hook=[&](unsigned,auto p){if(p==Fixture::block(3,0))f.projection.error_6c=47;};
        check(f.parse()==47&&f.projection.error_6c==47&&f.raw.memory.view(Memory::document_address+0x6c,4).data()==reinterpret_cast<std::uint8_t*>(&f.projection.error_6c),"004280 returns live shared error after actual entry,error helper and four-pool cleanup");
        check(f.input_allocations==1&&f.input_copies==1&&f.effects.virtuals==0&&f.lower.headers.size()==1&&f.lower.separators.empty()&&f.lower.details.empty(),"source copied-empty entry reaches final005820 without supplied virtual parsing");
        const auto& h=f.lower.headers[0];check(h.id==13&&h.hex==13&&h.line==0&&h.name==Memory::name&&h.destination==Memory::temporary_address&&h.capacity==1000,"source entry supplies13,0,null detail to actual error body and declared formatter");
        check(f.raw.get(0x90,4)==0&&f.raw.get(0xa0,4)==(bytes[0]==' '?2U:1U)&&f.raw.document[0x68]==(bytes[0]==0xef?1:0)&&f.raw.get(0x98)==Fixture::copy_address&&f.copy[f.input.size()]==0,"entry line/BOM and owned input remain coherent with error line0");
        check(f.raw.get(0x78,4)==0x200&&f.raw.get(0x80)==Memory::owned_address&&f.raw.get(0x88)==Memory::owned_address+f.lower.header_text.size()&&f.ops.allocations==std::vector<std::uint64_t>{1000,f.lower.header_text.size()+1},"actual error helper publishes separate owned diagnostic after scratch allocation");
        check(f.ops.frees==std::vector<std::uintptr_t>{0,Memory::temporary_address,Fixture::block(0,0),Fixture::block(2,1),Fixture::block(2,0),Fixture::block(3,0)}&&!f.nodes.calls,"actual d70/e10/dc0 cleanup runs source order on formatter-seeded records without DOM claim");
        for(unsigned k=0;k<4;++k){check(f.raw.get(Fixture::offsets[k]+0x64,4)==0&&f.raw.get(Fixture::offsets[k]+0x68)==0&&f.raw.get(Fixture::offsets[k]+0x70)==0&&f.raw.get(Fixture::offsets[k]+0x78)==0,"all four source pool counters and tracking fields cleared");check(f.raw.get(Fixture::offsets[k]+8)==Fixture::array(k)&&f.raw.get(Fixture::offsets[k]+0x60,4)==3,"pool array pointer and capacity retained");}
    }
    for(std::vector<std::uint8_t> bytes:std::array<std::vector<std::uint8_t>,2>{{{}, {0}}}){Fixture f(bytes);check(f.parse()==13&&f.projection.error_6c==13&&f.lower.headers.size()==1&&f.effects.classified.empty()&&!f.effects.virtuals&&!f.input_allocations&&!f.input_copies,"004280 early empty routes directly through final005820,without falsely executing0041b0");check(!f.raw.get(0xa0,4)&&!f.raw.get(0x30,4)&&f.ops.frees==std::vector<std::uintptr_t>{0,Memory::temporary_address},"early error retains constructor parse fields and cleanup prefix");}
}
void bridge_argument_transport_and_success(){
    {Fixture f({'x'});check(f.parse()==0&&f.projection.error_6c==0&&f.effects.virtuals==1&&f.lower.headers.empty()&&f.ops.allocations.empty()&&f.ops.frees==std::vector<std::uintptr_t>{0},"ordinary source entry uses declared virtual parser,not diagnostic body");}
    {Fixture f({'x'});f.effects.virtual_hook=[&](auto t,auto d,auto c,auto z,auto l){check(t==Fixture::target&&d==Memory::document_address&&c==Fixture::copy_address&&!z&&l==Memory::document_address+0xa0,"declared virtual receives exact entry identity and observed register arguments");f.effects.set_error_005820(d,13,55,0xf606);};check(f.parse()==13&&f.lower.details.size()==1&&f.lower.details[0].arguments==0xf505&&f.lower.details[0].format==0xf606&&f.lower.headers[0].line==55,"virtual fixture explicitly enters final diagnostic bridge with supplied argument tail");check(f.raw.get(0x90,4)==55&&f.raw.get(0x88)==Memory::owned_address+f.lower.header_text.size()+2+f.lower.detail_text.size(),"optional diagnostic body publishes concatenated supplied formatting result");}
    {Fixture f;for(const auto tail:{std::uintptr_t{0},std::uintptr_t{0xfeedbabe}}){f.arguments.opaque_cursor=tail;f.ops.allocations.clear();f.ops.frees.clear();f.ops.copies.clear();f.ops.events.clear();f.lower.header_hook=[&]{f.arguments.opaque_cursor=0xbadbad;};f.effects.set_error_005820(Memory::document_address,13,9,0xf707);check(f.lower.details.back().arguments==tail&&f.lower.details.back().format==0xf707,"bridge samples externally supplied per-call argument identity before lower callbacks mutate context");check(f.raw.get(0x90,4)==9&&f.raw.get(0x78,4)==0x200,"direct final bridge retains source diagnostic ownership across repeated invocations");}}
}
void failure_composition(){
    {Fixture f({' ','\n'});f.lower.header_hook=[&]{f.seed_pools();throw Failure{};};fails([&]{(void)f.parse();});check(f.projection.error_6c==13&&f.raw.get(0x90,4)==0&&f.raw.get(0xa0,4)==2&&f.raw.get(0x98)==Fixture::copy_address&&f.ops.frees==std::vector<std::uintptr_t>{0}&&f.ops.allocations==std::vector<std::uint64_t>{1000},"formatter failure preserves copied input and entry/error prefix,without scratch release or wrapper unwind");check(f.raw.get(Fixture::offsets[0]+0x64,4)==1&&f.raw.get(Fixture::offsets[2]+0x64,4)==2,"formatter failure prevents later wrapper pool cleanup");}
    {Fixture f({' ','\n'});f.ops.copy_hook=[](const auto&){throw Failure{};};fails([&]{(void)f.parse();});check(f.projection.error_6c==13&&f.raw.get(0x80)==Memory::owned_address&&!f.raw.get(0x78,4)&&!f.raw.get(0x88)&&f.ops.frees==std::vector<std::uintptr_t>{0}&&f.input_copies==1,"diagnostic copy failure retains final buffer pointer without publication or wrapper cleanup");}
    {Fixture f({' ','\n'});f.effects.classifier=[](auto)->std::int32_t{throw Failure{};};fails([&]{(void)f.parse();});check(!f.projection.error_6c&&f.lower.headers.empty()&&f.ops.allocations.empty()&&f.raw.get(0xa0,4)==1&&f.raw.get(0x30,4)==1&&f.input_copies==1,"classifier failure before error helper preserves entry initialization and copied input only");}
}
}
int main(){try{copied_empty_entry_and_cleanup();bridge_argument_transport_and_success();failure_composition();std::cout<<"PASS: "<<checks<<" document-error integration checks; actual001830/004280/0041b0/final005820 plus shared error,four-pool cleanup,explicit per-call detail context and failure prefixes; formatter/heap/virtualDOM/nativeABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
