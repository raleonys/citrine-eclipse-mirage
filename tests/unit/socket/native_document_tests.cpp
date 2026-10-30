#include "tests/support/socket/native_document_test_fixture.hpp"
using namespace document_test;
namespace {
struct Anchor{std::size_t offset;unsigned width;std::uint64_t value;};
// Independent retained-assembly fixture, ev_de11d55b943716ed390775f77e60b3f24b2c5f17431bc50f1f7fc7eca335c9c9.
// Values below use receiver0x400000000, mode0x9f, flags0x81234567.
constexpr Anchor anchors[]={
    {0x10,8,0x0ULL}, // 0x18000183a
    {0x18,4,0x0ULL}, // 0x18000183e
    {0x20,8,0x0ULL}, // 0x180001842
    {0x28,8,0x0ULL}, // 0x180001846
    {0x0,8,0x18007fec8ULL}, // 0x18000184a
    {0x30,4,0x0ULL}, // 0x180001854
    {0x38,8,0x0ULL}, // 0x180001858
    {0x40,8,0x0ULL}, // 0x18000185c
    {0x48,8,0x0ULL}, // 0x180001860
    {0x50,8,0x0ULL}, // 0x180001864
    {0x58,8,0x0ULL}, // 0x180001868
    {0x60,8,0x0ULL}, // 0x18000186c
    {0x68,1,0x0ULL}, // 0x180001870
    {0x69,1,0x9fULL}, // 0x180001874
    {0x6c,4,0x0ULL}, // 0x180001877
    {0x70,4,0x81234567ULL}, // 0x18000187b
    {0x78,4,0x0ULL}, // 0x18000187f
    {0x80,8,0x0ULL}, // 0x180001883
    {0x88,8,0x0ULL}, // 0x18000188a
    {0x90,4,0x0ULL}, // 0x180001891
    {0x98,8,0x0ULL}, // 0x180001898
    {0xa0,8,0x0ULL}, // 0x18000189f
    {0xa8,8,0x4000000b0ULL}, // 0x1800018a6
    {0x100,8,0xaULL}, // 0x1800018b4
    {0x108,8,0x18007fe08ULL}, // 0x1800018bf
    {0x110,8,0x400000118ULL}, // 0x1800018cd
    {0x168,8,0xaULL}, // 0x1800018db
    {0x170,8,0x0ULL}, // 0x1800018e6
    {0x178,8,0x0ULL}, // 0x1800018ed
    {0x180,8,0x0ULL}, // 0x1800018f4
    {0x188,8,0x18007fe38ULL}, // 0x1800018fb
    {0x190,8,0x400000198ULL}, // 0x180001909
    {0x1e8,8,0xaULL}, // 0x180001917
    {0x1f0,8,0x0ULL}, // 0x180001922
    {0x1f8,8,0x0ULL}, // 0x180001929
    {0x200,8,0x0ULL}, // 0x180001930
    {0x208,8,0x18007fe68ULL}, // 0x180001937
    {0x210,8,0x400000218ULL}, // 0x180001945
    {0x268,8,0xaULL}, // 0x180001953
    {0x270,8,0x0ULL}, // 0x18000195e
    {0x278,8,0x0ULL}, // 0x180001965
    {0x280,8,0x0ULL}, // 0x18000196c
    {0x288,8,0x18007fe98ULL}, // 0x180001973
    {0x290,8,0x400000298ULL}, // 0x180001981
    {0x2e8,8,0xaULL}, // 0x18000198b
    {0x2f0,8,0x0ULL}, // 0x180001996
    {0x2f8,8,0x0ULL}, // 0x18000199d
    {0x300,8,0x0ULL}, // 0x1800019a4
    {0x8,8,0x400000000ULL}, // 0x1800019ab
};
void overlay(std::array<std::uint8_t,0x308>& bytes,const Anchor& anchor){for(unsigned i=0;i<anchor.width;++i)bytes[anchor.offset+i]=static_cast<std::uint8_t>(anchor.value>>(i*8));}
void constructors(){
    Fixture f;check(f.construct(0x9f,0x81234567)==State::base,"constructor returns original receiver");
    std::array<std::uint8_t,0x308> expected;expected.fill(0xa5);for(auto anchor:anchors)overlay(expected,anchor);
    check(f.state.snapshot()==expected,"full independent image:49 fields and422 untouched inline/padding bytes");
    check(f.effects.trace.empty(),"constructor performs no invented lower helper calls");
    std::array<std::uint8_t,0x308> prefix;prefix.fill(0xa5);
    for(const auto anchor:anchors){Fixture cut;cut.state.omitted={{anchor.offset,anchor.width}};cut.state.refresh();try{cut.construct(0x9f,0x81234567);throw std::runtime_error("missing bounded constructor failure");}catch(const s::UnsupportedInput&){check(cut.state.snapshot()==prefix,"each missing mapped field preserves exact prior-store prefix and later untouched bytes");check(cut.effects.trace.empty(),"bounded constructor failure does not invent cleanup");}overlay(prefix,anchor);}
}
void validation_and_copy(){
    for(unsigned kind=0;kind<3;++kind){Fixture f;f.construct();f.document.error_6c=88;if(kind==2)f.state.seed("");const auto input=kind==0?std::uintptr_t{0xdead}:kind==1?0:State::input_address;const auto count=kind==0?0:UINT64_MAX;check(f.parse(input,count)==13&&f.document.error_6c==13,"count0/null/firstNUL returns live error13");check(f.effects.trace==std::vector<std::string>{"clear","set-error"}&&!f.effects.parses&&f.effects.allocations.empty(),"clear precedes validation; empty input avoids copy and error pool cleanup");}
    {Fixture f;f.construct();f.state.seed("<x/>");f.effects.hook=[&](auto name){if(name=="clear"){check(f.document.error_6c==71,"clear observes previous shared status");f.state.input[0]=0;}};f.document.error_6c=71;check(f.parse()==13,"first byte read occurs after clear callback");}
    for(bool explicit_count:{false,true}){Fixture f;f.construct();const auto input=std::string("<x/>\0ignored",12);f.state.seed(input);const auto count=explicit_count?input.size():4;check(f.parse(State::input_address,explicit_count?input.size():UINT64_MAX)==0,"embedded NUL accepted by lower C-string parser");check(f.effects.copy_count==count&&f.effects.allocations==std::vector<std::uint64_t>{count+1},"explicit count copies trailing bytes; sentinel count scans only through firstNUL");const auto ptr=f.state.get(0x98);check(f.state.memory.view(ptr+count,1)[0]==0&&f.effects.parsed->first_element()->name()=="x","caller terminates selected buffer at cached count before lower parser");f.destroy();check(!f.state.live(),"normal destructor lower clear releases owned parser buffer");}
    {Fixture f;f.construct();f.state.seed("<x/>");std::uintptr_t input=State::input_address;std::uint64_t count=4;f.effects.hook=[&](auto name){if(name=="clear"){input=0;count=0;}};check(f.parse(input,count)==0&&f.effects.copy_source==State::input_address&&f.effects.copy_count==4,"input and count cached by value across lower clear");}
    {Fixture f;f.construct();f.state.seed("<x/>");std::uintptr_t alternate=0;f.effects.hook=[&](auto name){if(name=="after-copy"){alternate=f.state.allocate(5);auto bytes=f.state.memory.view(alternate,5);std::copy_n("<y/>",4,bytes.begin());f.state.put(0x98,alternate);}if(name=="parse"){check(f.state.memory.view(alternate+4,1)[0]==0&&f.state.memory.view(f.effects.copy_destination+4,1)[0]==0xcc,"terminator targets reloaded live buffer, not allocation cached before copy");}};check(f.parse()==0&&f.effects.parsed->first_element()->name()=="y","lower parser sees replacement live buffer");f.state.release(f.effects.copy_destination);f.destroy();check(!f.state.live(),"controlled replacement buffer test cleanup balanced");}
    {Fixture f;f.construct();f.state.seed("x");const std::uint64_t count=0x8000000000000000ULL;f.effects.hook=[](auto name){if(name=="allocate")throw Failure{44};};try{f.parse(State::input_address,count);throw std::runtime_error("missing symbolic allocation stop");}catch(const Failure& e){check(e.code==44&&f.effects.allocations==std::vector<std::uint64_t>{count+1}&&!f.effects.copy_count&&!f.state.live(),"huge unsigned count+1 reaches mandatory allocator without fabricated length terminal or giant allocation");}}
    {Fixture f;f.construct();f.state.seed("x");f.effects.null_allocate=true;try{f.parse(State::input_address,1);throw std::runtime_error("null lower allocation accepted");}catch(const s::UnsupportedInput&){check(f.effects.copy_destination==0&&f.effects.copy_count==1&&f.state.get(0x98)==0&&f.effects.trace.back()=="copy","null allocation published and passed to copy without invented caller null guard");}}
}
void parser_errors(){
    {Fixture f;f.construct();f.state.seed("<x/>");f.effects.codes={7};f.effects.hook=[&](auto name){if(name=="delete-nodes")f.document.error_6c=0;if(name=="clear104")f.document.error_6c=-123;};check(f.parse()==-123&&f.document.error_6c==-123,"error branch selected before cleanup; signed error reloaded after final pool callback");check(f.effects.trace==std::vector<std::string>{"clear","allocate","copy","after-copy","parse","delete-nodes","clear120","clear80","clear112","clear104"},"native node/pool error cleanup order");}
    for(const auto stage:{"clear","allocate","copy","after-copy","parse","delete-nodes","clear120","clear80","clear112","clear104"}){Fixture f;f.construct();f.state.seed("<x/>");f.effects.codes={7};f.effects.hook=[&](auto name){if(name==stage)throw Failure{44};};try{f.parse();throw std::runtime_error("missing parser failure");}catch(const Failure& e){check(e.code==44&&f.effects.trace.back()==stage&&!f.effects.destroys,"each parser lower failure retains exact observed prefix with no invented document destructor");}}
}
void destructors(){
    {Fixture f;f.construct();f.effects.clear_model=false;f.state.put(0,0xeeee);f.state.put(0xa8,0x1111);f.state.put(0x78,0,4);f.state.put(0x80,0x2222);f.state.put(0x88,19);f.effects.hook=[&](auto name){if(name=="clear")check(f.state.get(0)==f.context.identities.document,"document vtable identity reset before clear");if(name=="free"&&f.effects.freed.size()==1){f.state.put(0x78,0x200,4);f.state.put(0x80,0x3333);}if(name=="base-destroy")check(!f.state.get(0x78,4)&&!f.state.get(0x80)&&!f.state.get(0x88),"flags/pointer/count reset before base destructor");};f.destroy();check(f.effects.freed==std::vector<std::uintptr_t>{0x1111,0x3333},"vector free callback can change live flags and later free pointer");check(f.effects.trace==std::vector<std::string>{"clear","destroy104","destroy112","destroy80","destroy120","free","free","base-destroy"},"four reverse pool destructors then vector and owned string frees");}
    for(std::uint32_t flags:{0U,0x400U,0x200U,0x600U}){Fixture f;f.construct();f.effects.clear_model=false;f.state.put(0x78,flags,4);f.state.put(0x80,0);f.destroy();check(f.effects.freed.size()==unsigned(bool(flags&0x200))&&(!(flags&0x200)||f.effects.freed[0]==0),"only ownership bit0x200 controls string free, including null pointer; inline vector never freed");}
    for(const auto stage:{"clear","destroy104","destroy112","destroy80","destroy120","free","base-destroy"}){Fixture f;f.construct();f.effects.clear_model=false;f.state.put(0xa8,0x1111);f.state.put(0x78,0x200,4);f.state.put(0x80,0x2222);f.state.put(0x88,17);f.effects.hook=[&](auto name){if(name==stage)throw Failure{44};};try{f.destroy();throw std::runtime_error("missing destructor failure");}catch(const Failure& e){check(e.code==44&&f.effects.trace.back()==stage&&!f.effects.destroys,"destructor failures stop without synthesized remaining cleanup");const bool reset=std::string_view(stage)=="base-destroy";check((f.state.get(0x78,4)==0)==reset&&(f.state.get(0x80)==0)==reset&&(f.state.get(0x88)==0)==reset,"destructor field reset belongs after normal frees and before base tail effect");}}
}
struct Bridge {
    native_message_test::Fixture native;s::NativeDispatchLocals scratch;dispatch_test::Bytes bytes{native,scratch};s::ReceiveBufferContext strings{native.core.containers.bytes.memory,bytes};s::SessionCipher session;
    s::RecoveredPrefixCipherBlocks blocks{native.core.containers.bytes.memory};s::PrefixCipherContext cipher{strings,blocks};std::array<s::PrefixCipherBinding,1> ciphers{{{0x120f0,&session.native()}}};
    dispatch_test::Effects dispatch{native,strings,scratch,session};s::NativeDispatchContext dispatch_context{strings,scratch,dispatch};
    State state{native.locals.document,&native.core.containers.bytes.memory};Effects effects{state,&native};s::NativeDocumentContext context{state.memory,effects};
    std::array<s::NativeDocumentBinding,2> bindings{{{&native.locals.document,State::base},{nullptr,0}}};
    Api api{cipher,ciphers,dispatch_context,context,{bindings.data(),1},native.api,dispatch.effects};
};
void call(Api& api,s::NativeMessageDocument& d,unsigned kind){if(kind==0)api.construct_document_001830(d,true,9);else if(kind==1)(void)api.parse_document_004280(d,State::input_address,UINT64_MAX);else api.destroy_document_001e00(d);}
void bindings(){
    for(unsigned invalid=0;invalid<4;++invalid)for(unsigned kind=0;kind<3;++kind){Bridge b;s::NativeMessageDocument other;auto& doc=invalid==3?other:b.native.locals.document;if(invalid==0)b.bindings[0].projection=nullptr;if(invalid==1)b.bindings[0].address=0;if(invalid==2)b.bindings[1]=b.bindings[0];if(invalid==3)b.bindings[0].projection=&other;Api api{b.cipher,b.ciphers,b.dispatch_context,b.context,{b.bindings.data(),invalid==2?2U:1U},b.native.api,b.dispatch.effects};const auto before=b.state.snapshot();try{call(api,doc,kind);throw std::runtime_error("bad binding accepted");}catch(const s::UnsupportedInput&){check(b.effects.trace.empty()&&b.state.snapshot()==before,"missing/zero/duplicate/mismatched error binding rejected before native effects or stores");}}
    {Bridge b;auto& d=b.native.locals.document;b.api.construct_document_001830(d,true,0x1234);check(d.error_6c==0&&b.state.get(0x69,1)==1&&b.state.get(0x70,4)==0x1234,"final constructor bridge writes actual caller error cell and defined fields");b.state.seed("<x/>");b.effects.codes={7};b.effects.hook=[&](auto name){if(name=="clear104")d.error_6c=-42;};check(b.api.parse_document_004280(d,State::input_address,UINT64_MAX)==-42&&b.state.get(0x6c,4)==std::uint32_t(-42),"final parser bridge returns the same live raw/caller status after cleanup callbacks");b.effects.hook=[&](auto name){if(name=="parse"){d.error_6c=-99;throw Failure{44};}};try{b.api.parse_document_004280(d,State::input_address,UINT64_MAX);throw std::runtime_error("missing bound failure");}catch(const Failure& e){check(e.code==44&&d.error_6c==-99&&b.state.get(0x6c,4)==std::uint32_t(-99)&&b.state.live()==1,"shared status and allocated buffer survive throw without normal-only writeback or invented cleanup");}b.effects.hook={};b.api.destroy_document_001e00(d);check(!b.state.live()&&b.effects.destroys==1&&!b.native.api.constructs&&!b.native.api.parses&&!b.native.api.destroys,"final document sources bypass old controlled API and release via mandatory lower clear");}
}
}
int main(){try{constructors();validation_and_copy();parser_errors();destructors();bindings();std::cout<<"PASS: "<<checks<<" native document checks;49 assembly anchored stores/holes/failure prefixes, live buffer/status, cleanup order and coherent final bindings; native lower providers/ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
