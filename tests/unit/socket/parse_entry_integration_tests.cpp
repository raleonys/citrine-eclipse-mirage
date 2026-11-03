// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/parse_entry_test_fixture.hpp"
#include <cstring>
using namespace parse_entry_test;
namespace {
struct Effects:s::ParseEntryEffects {
    std::vector<std::uint32_t> classified;std::vector<Lower::Virtual> virtuals;std::vector<CoreDocs::Error> errors;std::vector<std::uintptr_t> freed;
    std::function<std::int32_t(std::uint32_t)> classifier;std::function<void(const Lower::Virtual&)> virtual_hook;std::function<void(const CoreDocs::Error&)> error_hook;
    std::function<std::uintptr_t(std::uint64_t)> allocator;std::function<void(std::uintptr_t,std::uintptr_t,std::uint64_t)> copier;std::function<void(std::uintptr_t)> free_hook;
    Effects(s::ReceiveBufferMemory& memory,s::NativeNodeClearEffects& nodes):ParseEntryEffects(memory,nodes){}
    std::int32_t classify_051fc0(std::uint32_t b)override{classified.push_back(b);return classifier?classifier(b):0;}
    void parse_virtual(std::uintptr_t t,std::uintptr_t d,std::uintptr_t c,std::uintptr_t z,std::uintptr_t l)override{virtuals.push_back({t,d,c,z,l});if(virtual_hook)virtual_hook(virtuals.back());}
    void set_error_005820(std::uintptr_t d,std::int32_t c,std::uint32_t l,std::uintptr_t p)override{errors.push_back({d,c,l,p});if(error_hook)error_hook(errors.back());}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{if(!allocator)throw std::runtime_error("missing supplied allocator");return allocator(n);}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{if(!copier)throw std::runtime_error("missing supplied copy");copier(to,from,n);}
    void free_04b0f8(std::uintptr_t p)override{freed.push_back(p);if(free_hook)free_hook(p);}
};
// The virtual parse target is supplied by this fixture, not recovered native
// slot+80 evidence. It seeds block records only; no native DOM is materialized.
struct Fixture {
    static constexpr std::uintptr_t address=0x470000000ULL,input_address=0x570000000ULL,buffer_address=0x670000000ULL,table_address=0x770000000ULL,target=0xf303;
    static constexpr std::array<unsigned,4> offsets{0x108,0x188,0x208,0x288};
    static constexpr std::uintptr_t array(unsigned k){return 0x970000000ULL+k*0x1000;}
    static constexpr std::uintptr_t block(unsigned k,unsigned n){return 0xa70000000ULL+k*0x1000+n*0x100;}
    s::NativeMessageDocument projection;std::array<std::uint8_t,0x308> raw{};std::vector<std::uint8_t> input,buffer;std::array<std::uint8_t,0x88> table{};std::array<std::array<std::uint8_t,24>,4> arrays{};
    std::vector<s::ReceiveBufferRegion> regions;s::ReceiveBufferMemory memory{};pool_clear_test::Nodes nodes;Effects effects{memory,nodes};s::NativeDocumentContext context{memory,effects};
    unsigned allocations=0,copies=0;bool virtual_error=false,throw_virtual=false,throw_error=false,throw_classifier=false;std::array<std::uint8_t,0x308> seeded{};
    explicit Fixture(std::vector<std::uint8_t> bytes):input(std::move(bytes)){
        raw.fill(0xa5);table.fill(0x7b);put(table,0x80,target);refresh();context.identities.document=table_address;
        effects.allocator=[&](std::uint64_t n){++allocations;check(n==input.size()+1,"004280 requests copied input length plus terminator");buffer.assign(static_cast<std::size_t>(n),0xcc);refresh();return buffer_address;};
        effects.copier=[&](auto to,auto from,std::uint64_t n){++copies;check(to==buffer_address&&from==input_address&&n==input.size(),"004280 copy arguments cover only controlled input bytes");const auto destination=memory.view(to,n),source=memory.view(from,n);std::memmove(destination.data(),source.data(),static_cast<std::size_t>(n));};
        effects.classifier=[&](auto byte){if(throw_classifier)throw Failure{77};return controlled_space(byte);};
        effects.error_hook=[&](const auto& e){check(e.document==address&&e.code==13&&!e.line&&!e.detail,"source parse entry delegates exact empty-input error13 boundary");projection.error_6c=e.code;if(throw_error)throw Failure{77};};
        effects.virtual_hook=[&](const auto& v){check(v.target==target&&v.document==address&&!v.zero&&v.line==address+0xa0,"actual0041b0 supplies declared virtual identity and exact register arguments");seed_pools();projection.error_6c=virtual_error?31:0;if(throw_virtual)throw Failure{77};};
        effects.free_hook=[&](auto p){if(p==block(3,0))projection.error_6c=47;};
        check(s::construct_native_document_001830(address,1,0,context)==address,"001830 source uses explicitly selected fixture document table");
    }
    void refresh(){regions={{address,std::span(raw).first(0x6c)},{address+0x6c,{reinterpret_cast<std::uint8_t*>(&projection.error_6c),4}},{address+0x70,std::span(raw).subspan(0x70)},{input_address,input},{table_address,table}};if(!buffer.empty())regions.push_back({buffer_address,buffer});for(unsigned k=0;k<4;++k)regions.push_back({array(k),arrays[k]});memory.regions=regions;}
    void seed_pools(){constexpr unsigned counts[]{1,0,2,1};for(unsigned k=0;k<4;++k){const auto off=offsets[k];put(raw,off+8,array(k));put(raw,off+0x60,3,4);put(raw,off+0x64,counts[k],4);put(raw,off+0x68,0x11110000+k);put(raw,off+0x70,0x2222000000000000ULL+k);put(raw,off+0x78,0x3333000000000000ULL+k);arrays[k].fill(0x7b);for(unsigned n=0;n<counts[k];++n)put(arrays[k],n*8,block(k,n));}seeded=raw;}
    int parse(){return s::parse_native_document_004280(address,input_address,input.size(),context);}
};
void source_success_and_virtual_error(){
    struct Input {std::vector<std::uint8_t> bytes;unsigned cursor,line,bom;};
    const std::array<Input,5> inputs{{{{' ','\n','x'},2,2,0},{{0xef,0xbb,0xbf,'x'},3,1,1},{{' ','\n',0xef,0xbb,0xbf,' '},5,2,1},{{0xef,0},0,1,0},{{0x80},0,1,0}}};
    for(const auto& input:inputs)for(bool error:{false,true}){Fixture f(input.bytes);f.virtual_error=error;const auto result=f.parse();check(result==(error?47:0)&&f.projection.error_6c==result&&f.memory.view(Fixture::address+0x6c,4).data()==reinterpret_cast<std::uint8_t*>(&f.projection.error_6c),"004280 returns live shared error after source entry and optional four-pool cleanup");
        check(f.effects.virtuals.size()==1&&f.effects.virtuals[0].cursor==Fixture::buffer_address+input.cursor&&f.effects.errors.empty()&&word(f.raw,0xa0,4)==input.line&&word(f.raw,0x30,4)==1&&f.raw[0x68]==input.bom&&f.allocations==1&&f.copies==1&&!f.nodes.calls,"copied input flows through final0041b0 and supplied virtual body with source line/BOM state");
        auto expected=f.seeded;if(error){for(unsigned k=0;k<4;++k){const auto off=Fixture::offsets[k];put(expected,off+0x64,0,4);for(unsigned at:{0x68U,0x70U,0x78U})put(expected,off+at,0);}}
        check(f.raw==expected&&f.effects.freed==(error?std::vector<std::uintptr_t>{0,Fixture::block(0,0),Fixture::block(2,1),Fixture::block(2,0),Fixture::block(3,0)}:std::vector<std::uintptr_t>{0}),"virtual error triggers actuald70twice/e10/dc0 in source order; success retains declared pool records");
        for(unsigned k=0;k<4;++k)check(word(f.raw,Fixture::offsets[k]+8)==Fixture::array(k)&&word(f.raw,Fixture::offsets[k]+0x60,4)==3&&word(f.arrays[k],16)==0x7b7b7b7b7b7b7b7bULL,"cleanup retains array pointer,capacity and stale record bytes without array free");
    }
}
void empty_entry_and_failures(){
    for(std::vector<std::uint8_t> bytes:std::array<std::vector<std::uint8_t>,2>{{{' ','\n'},{0xef,0xbb,0xbf}}}){Fixture f(bytes);const auto result=f.parse();check(result==13&&f.projection.error_6c==13&&f.effects.errors.size()==1&&f.effects.virtuals.empty()&&f.effects.freed==std::vector<std::uintptr_t>{0}&&f.allocations==1&&f.copies==1,"source entry empty-after-whitespace/BOM reaches supplied error then wrapper zero-count pool cleanup");check(word(f.raw,0xa0,4)==(bytes[0]==' '?2U:1U)&&f.raw[0x68]==(bytes[0]==0xef?1:0),"entry error retains source line and BOM side effects");}
    {Fixture f({0});check(f.parse()==13&&f.effects.errors.size()==1&&f.effects.classified.empty()&&f.effects.virtuals.empty()&&!f.allocations&&!f.copies&&!word(f.raw,0xa0,4)&&!word(f.raw,0x30,4),"004280 early empty check does not falsely claim to execute0041b0");}
    {Fixture f({0xef,0xbb,0xbf,'x'});f.virtual_error=true;f.throw_virtual=true;failure([&]{(void)f.parse();});check(f.effects.virtuals.size()==1&&f.effects.freed==std::vector<std::uintptr_t>{0}&&f.projection.error_6c==31&&word(f.raw,Fixture::offsets[0]+0x64,4)==1&&word(f.raw,Fixture::offsets[2]+0x64,4)==2&&f.raw[0x68]==1&&word(f.raw,0x98)==Fixture::buffer_address,"virtual failure retains owned input,BOM and supplied pool/error mutations while preventing wrapper cleanup");}
    {Fixture f({' ','\n'});f.throw_error=true;failure([&]{(void)f.parse();});check(f.projection.error_6c==13&&f.effects.errors.size()==1&&f.effects.virtuals.empty()&&word(f.raw,0xa0,4)==2&&word(f.raw,0x98)==Fixture::buffer_address&&f.effects.freed==std::vector<std::uintptr_t>{0},"error-provider failure preserves initialized entry and input ownership,without invented wrapper unwind");}
    {Fixture f({'x'});f.throw_classifier=true;failure([&]{(void)f.parse();});check(word(f.raw,0xa0,4)==1&&word(f.raw,0x30,4)==1&&word(f.raw,0x98)==Fixture::buffer_address&&f.effects.virtuals.empty()&&f.effects.errors.empty()&&f.effects.freed==std::vector<std::uintptr_t>{0},"classifier failure leaves wrapper copy and initialized DWORD prefix,without extra cleanup/provider calls");}
}
}
int main(){try{source_success_and_virtual_error();empty_entry_and_failures();std::cout<<"PASS: "<<checks<<" parse-entry integration checks; actual001830/004280 copy→final0041b0→controlled virtual/error with shared error and actual four-pool cleanup,success/failure ownership prefixes; classifier,error body,virtual DOM,heap/nativeABI/EH supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
