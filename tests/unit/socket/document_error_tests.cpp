// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/document_error_test_fixture.hpp"
using namespace document_error_test;
namespace {
void ordinary_and_optional(){
    for(const auto id:std::array<std::int32_t,4>{0,13,-1,31})for(const auto line:std::array<std::uint32_t,2>{0,UINT32_MAX}){
        Fixture f;f.raw.set_name(id,0xface0000ULL+static_cast<std::uint32_t>(id));auto expected=f.raw.document;
        f.set(id,line);const auto& h=f.lower.headers.at(0);
        check(h.destination==Memory::temporary_address&&h.capacity==1000&&h.format==0x180080068ULL&&h.name==0xface0000ULL+static_cast<std::uint32_t>(id)&&h.id==id&&h.hex==static_cast<std::uint32_t>(id)&&h.line==line,"cached register and stack header arguments; signed negative table index");
        f.complete(f.lower.header_text,static_cast<std::uint32_t>(id),line);
        check(f.lower.separators.empty()&&f.lower.details.empty()&&f.ops.frees==std::vector<std::uintptr_t>{Memory::temporary_address}&&f.ops.events==std::vector<std::string>{"allocate1","header","allocate2","copy","free"},"null detail skips optional formatting and exact normal effect order");
        put(expected,0x6c,static_cast<std::uint32_t>(id),4);put(expected,0x90,line,4);put(expected,0x78,0x200,4);put(expected,0x80,Memory::owned_address);put(expected,0x88,Memory::owned_address+f.lower.header_text.size());check(expected==f.raw.document,"only selected document fields change; all padding retained");
    }
    for(std::size_t used:{0U,999U,1000U,1001U}){
        Fixture f;f.lower.header_text.assign(used,'h');f.set(13,7,0xf010,0xf020);
        const auto& a=f.lower.separators.at(0);const auto& b=f.lower.details.at(0);
        check(a.destination==Memory::temporary_address+used&&a.capacity==std::uint64_t{1000}-used&&a.format==0x180080094ULL,"separator uses first scanned length and wrapping capacity subtraction");
        check(b.destination==Memory::temporary_address+used+2&&b.capacity==std::uint64_t{1000}-(used+2)&&b.format==0xf010&&b.arguments==0xf020,"detail rescans separator output and forwards explicit opaque argument identity");
        f.complete(f.lower.header_text+f.lower.separator_text+f.lower.detail_text);
        check(f.ops.events==std::vector<std::string>{"allocate1","header","separator","detail","allocate2","copy","free"},"optional formatting order has no invented buffer guard");
    }
}
void live_effects_and_ownership(){
    for(const std::uint32_t flags:{0U,1U,0x200U,0xffff0200U})for(const std::uintptr_t old:{std::uintptr_t{0},std::uintptr_t{0xabc000}}){Fixture f;f.raw.set(0x78,flags,4);f.raw.set(0x80,old);f.raw.set(0x88,old+3);f.set();auto freed=std::vector<std::uintptr_t>{};if(flags&0x200)freed.push_back(old);freed.push_back(Memory::temporary_address);check(f.ops.frees==freed,"first free depends only on live owned bit and accepts null pointer");f.complete(f.lower.header_text);}
    {
        Fixture f;f.raw.set(0x78,0x200,4);f.raw.set(0x80,0x1111);f.raw.set(0x88,0x2222);
        f.ops.free_hook=[&](unsigned n,auto p){if(n==1){check(p==0x1111&&f.raw.get(0x6c,4)==13&&f.raw.get(0x90,4)==7,"error and line are committed before old error free");f.raw.set(0x6c,99,4);f.raw.set(0x90,88,4);f.raw.set(0x80,0xaaaa);f.raw.set(0x88,0xbbbb);}else if(n==2){check(p==0x3333&&f.raw.get(0x78,4)==0xa200,"second free observes formatter-owned state live");f.raw.text(Memory::temporary_address,"changed by free");}else check(p==Memory::temporary_address&&f.raw.get(0x78,4)==0x200,"temporary released only after final publication");};
        f.ops.allocation_hook=[&](unsigned n,auto){if(n==1){check(!f.raw.get(0x78,4)&&!f.raw.get(0x80)&&!f.raw.get(0x88),"first release clears lower callback mutations before scratch allocation");f.raw.set_name(13,0xf909);f.context.identities={0xbad,0xbad,0xbad};}else check(!f.raw.get(0x78,4)&&!f.raw.get(0x80)&&!f.raw.get(0x88),"second release clears fields before final allocation");};
        f.lower.header_hook=[&]{const auto& h=f.lower.headers.back();check(h.id==13&&h.line==7&&h.name==0xf909&&h.format==0x180080068ULL,"cached code,line,constant identities coexist with post-allocation live table load");f.raw.set(0x78,0xa200,4);f.raw.set(0x80,0x3333);};f.set();
        f.complete("changed by free",99,88);check(f.ops.frees==std::vector<std::uintptr_t>{0x1111,0x3333,Memory::temporary_address},"both owned releases plus temporary release");
    }
    {Fixture f;f.ops.copy_hook=[&](const auto&){f.raw.set(0x80,0xfffffffffffffff0ULL);f.raw.set(0x78,0xdead,4);f.raw.set(0x88,0xabcd);f.raw.text(Memory::temporary_address,"different longer scratch data");};f.set();check(f.raw.get(0x80)==0xfffffffffffffff0ULL&&f.raw.get(0x88)==std::uintptr_t{0xfffffffffffffff0ULL}+f.lower.header_text.size()&&f.raw.get(0x78,4)==0x200,"post-copy live start plus cached preallocation length; unsigned end wrap");}
    {Fixture f;f.raw.omitted={{Memory::document_address+0x78,4}};f.raw.extras.push_back({Memory::document_address+0x78,std::span(f.raw.document).subspan(0x80,4)});f.raw.refresh();f.ops.copy_hook=[&](const auto&){f.raw.set(0x80,0x9988776655443322ULL);};f.set();check(f.raw.get(0x80)==0x9988776600000200ULL&&f.raw.get(0x88)==0x9988776655443322ULL+f.lower.header_text.size(),"end is computed before flag store even when separate logical fields alias physical bytes");}
}
void failure_prefixes(){
    {Fixture f;f.raw.set(0x78,0x200,4);f.raw.set(0x80,0x1111);f.ops.free_hook=[](auto,auto){throw Failure{};};fails([&]{f.set();});check(f.raw.get(0x6c,4)==13&&f.raw.get(0x90,4)==7&&f.raw.get(0x78,4)==0x200&&f.raw.get(0x80)==0x1111&&f.ops.allocations.empty(),"old-free failure preserves committed error fields and existing owner");}
    for(unsigned failed_allocation:{1U,2U}){Fixture f;f.ops.allocation_hook=[&](unsigned n,auto){if(n==failed_allocation)throw Failure{};};fails([&]{f.set();});check(f.raw.get(0x6c,4)==13&&f.raw.get(0x90,4)==7&&!f.raw.get(0x78,4)&&!f.raw.get(0x80)&&!f.raw.get(0x88)&&f.ops.copies.empty()&&f.ops.frees.empty(),"allocation failure leaves cleared error ownership and no rollback/free");check(f.lower.headers.size()==failed_allocation-1,"only earlier formatting occurs before allocation failure");}
    for(unsigned phase:{0U,1U,2U}){Fixture f;auto stop=[](){throw Failure{};};if(phase==0)f.lower.header_hook=stop;else if(phase==1)f.lower.separator_hook=stop;else f.lower.detail_hook=stop;fails([&]{f.set(13,7,0xf010,0xf020);});check(f.ops.allocations.size()==1&&f.ops.copies.empty()&&f.ops.frees.empty()&&!f.raw.get(0x78,4)&&!f.raw.get(0x80),"formatter failure retains scratch without invented cleanup");check(f.lower.headers.size()==1&&f.lower.separators.size()==(phase>=1?1U:0U)&&f.lower.details.size()==(phase==2?1U:0U),"formatter failure prevents later formatting phases");}
    {Fixture f;f.lower.header_hook=[&]{f.raw.set(0x78,0x200,4);f.raw.set(0x80,0x9999);};f.ops.free_hook=[](auto,auto){throw Failure{};};fails([&]{f.set();});check(f.raw.get(0x78,4)==0x200&&f.raw.get(0x80)==0x9999&&f.ops.allocations.size()==1&&f.ops.copies.empty(),"second free failure preserves formatter-owned state and scratch");}
    {Fixture f;f.ops.copy_hook=[](const auto&){throw Failure{};};fails([&]{f.set();});check(!f.raw.get(0x78,4)&&f.raw.get(0x80)==Memory::owned_address&&!f.raw.get(0x88)&&f.ops.frees.empty(),"copy failure keeps allocated pointer publication without flags,end or scratch release");}
    {Fixture f;f.ops.free_hook=[](auto,auto){throw Failure{};};fails([&]{f.set();});check(f.raw.get(0x78,4)==0x200&&f.raw.get(0x80)==Memory::owned_address&&f.raw.get(0x88)==Memory::owned_address+f.lower.header_text.size()&&f.ops.copies.size()==1,"final scratch free failure retains fully published owned result");}
    {Fixture f;f.raw.omitted={{Memory::names_address+13*8,8}};f.raw.refresh();unmapped([&]{f.set();});check(f.ops.allocations==std::vector<std::uint64_t>{1000}&&f.lower.headers.empty()&&!f.raw.get(0x78,4)&&!f.raw.get(0x80),"missing selected table mapping fails after scratch allocation and field clearing");}
    {Fixture f;f.lower.header_hook=[&]{std::fill(f.raw.temporary.begin(),f.raw.temporary.end(),0x41);};unmapped([&]{f.set();});check(f.ops.allocations.size()==1&&f.ops.copies.empty()&&f.ops.frees.empty()&&!f.raw.get(0x78,4),"unterminated supplied output fails at bounded scan; no artificial native length limit");}
    for(unsigned missing:{0x6cU,0x90U,0x78U,0x80U,0x88U}){Fixture f;f.raw.omitted={{Memory::document_address+missing,missing==0x80||missing==0x88?8U:4U}};f.raw.refresh();unmapped([&]{f.set();});check(f.ops.allocations.empty()&&f.lower.headers.empty()&&f.ops.frees.empty(),"initial field mapping failure prevents allocation and formatting");check(word(f.raw.document,0x6c,4)==(missing==0x6c?0xa5a5a5a5U:13U),"mapping failure retains precisely earlier code DWORD store");}
}
}
int main(){try{ordinary_and_optional();live_effects_and_ownership();failure_prefixes();std::cout<<"PASS: "<<checks<<" document-error core checks; cached header arguments,signed table lookup,two live ownership phases,opaque detail transport,wrapping capacities,postcopy start and committed failure prefixes; formatters/heap/nativeABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected failure after "<<checks<<" checks\n";return 1;}}
