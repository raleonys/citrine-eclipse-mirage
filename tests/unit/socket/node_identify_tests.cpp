#include "tests/support/socket/node_identify_test_fixture.hpp"
using namespace node_identify_test;
namespace {
void ordered_selection(){
    constexpr unsigned addresses[]{0x1610,0x1590,0x1720,0x17b0,0x1690,0x1720};
    constexpr unsigned pools[]{0x288,0x288,0x208,0x288,0x108,0x208},advance[]{2,4,9,2,1};
    for(unsigned chosen=0;chosen<6;++chosen){Fixture f;f.input(" \nx");if(chosen<5)f.match(chosen);
        f.constructors.hook=[&](const auto&){check(f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"Identify does not clear output on entry or before constructor");f.raw.set(Memory::line,100+chosen,4);return Memory::child;};
        auto result=f.identify();check(result==(chosen==5?Memory::input_address:Memory::input_address+2+advance[chosen]),"five prefix branches advance cached candidate; fallback returns original untrimmed input");
        check(f.constructors.calls.size()==1&&f.constructors.calls[0].address==addresses[chosen]&&f.constructors.calls[0].document==Memory::doc&&f.constructors.calls[0].pool==Memory::doc+pools[chosen],"exact selected constructor/document/pool operands from retained assembly");
        check(f.raw.get(Memory::child+0x30,4)==100+chosen&&f.raw.get(Fixture::out)==Memory::child,"live constructor-mutated line is written before node output publication");
        check(f.raw.get(Memory::line,4)==(chosen==5?7U:100+chosen),"only fallback rewinds cached initial line");
        check(f.raw.get(Memory::child+0x68,4)==(chosen==2?0xa5a5a501U:0xa5a5a5a5U),"CDATA writes exactly one BYTE and other branches preserve subtype storage");
        check(f.characters.classified==std::vector<std::uint32_t>{' ','\n','x'},"whitespace classifier receives unsigned bytes in order");
        check(f.parser.comparisons.size()==std::min(chosen+1,5U),"token comparisons stop at first match");
        for(unsigned i=0;i<f.parser.comparisons.size();++i){const auto c=f.parser.comparisons[i];check(c.left==Memory::input_address+2&&c.right==Fixture::token(i)&&c.limit==advance[i],"live token comparison uses exact candidate/token/limit");}
    }
    for(unsigned chosen=0;chosen<5;++chosen){Fixture f;put(f.tokens,chosen*8,Memory::input_address);f.token_bytes=(chosen+1)*8;f.refresh();
        check(f.identify()==Memory::input_address+advance[chosen]&&f.parser.comparisons.size()==chosen,"pointer identity skips selected compare and never reads unmapped later slots");}
    {Fixture f;f.parser.compare_hook=[&](const auto&){put(f.tokens,8,Memory::input_address);return 1;};check(f.identify()==Memory::input_address+4&&f.parser.comparisons.size()==1&&f.constructors.calls[0].address==0x1590,"previous compare can change next live token slot to pointer identity");}
    {Fixture f;f.match(3);f.parser.compare_hook=[&](const auto& c){f.context.identities.token_slots.fill(0xbad);return c.right==Fixture::token(3)?0:1;};check(f.identify()==Memory::input_address+2&&f.constructors.calls[0].address==0x17b0,"selected identity addresses are invocation constants while slot values remain live");}
}
void whitespace_and_input_edges(){
    {Fixture f;f.input(" \n\t");f.token_bytes=0;f.refresh();check(f.identify()==Memory::input_address+3&&!f.raw.get(Fixture::out)&&f.raw.get(Memory::line,4)==8,"NUL returns trimmed candidate and out0 without rewind/token reads");check(f.characters.classified==std::vector<std::uint32_t>{' ','\n','\t',0}&&f.constructors.calls.empty()&&f.parser.comparisons.empty(),"NUL is classified before stopping; no token/constructor effects");}
    for(unsigned b:{0x80U,0xefU}){Fixture f;f.raw.input[0]=static_cast<std::uint8_t>(b);f.raw.input[1]=0xbb;f.raw.input[2]=0xbf;check(f.identify()==Memory::input_address&&f.characters.classified.empty()&&f.parser.comparisons.size()==5&&f.constructors.calls[0].address==0x1720,"high-bit input skips classifier and has no invented BOM recognition");}
    {Fixture f;f.input("Ax");f.characters.classifier=[&](auto b){if(b=='A'){f.raw.input[0]='\n';f.raw.set(Memory::line,0xffffffff,4);return 1;}return 0;};f.match(4);check(f.identify()==Memory::input_address+2&&f.raw.get(Memory::line,4)==0&&f.raw.get(Memory::child+0x30,4)==0,"classified byte is reread after callback and live DWORD LF increment wraps");}
    {Fixture f;f.raw.input[0]=0;f.raw.input[1]='x';f.characters.classifier=[](auto b){return b==0?1:0;};f.match(0);check(f.identify()==Memory::input_address+3&&f.characters.classified==std::vector<std::uint32_t>{0,'x'},"no separate early NUL guard bypasses a supplied nonzero classifier result");}
    {Fixture f;f.characters.classifier=[&](auto){f.raw.input[0]=0;return 0;};check(f.identify()==Memory::input_address&&!f.raw.get(Fixture::out)&&f.parser.comparisons.empty(),"stopping byte is reread after classifier before NUL/token branch");}
    {Fixture f;std::array<std::uint8_t,4> zero_line{};put(zero_line,0,9,4);f.extra_regions.push_back({0,zero_line});f.refresh();f.input("\nx");f.characters.classifier=[](auto b){return b=='\n'?1:0;};f.match(4);const auto wrapped=std::uintptr_t{0}-0xa0;
        check(f.identify(Memory::input_address,Fixture::out,wrapped)==Memory::input_address+2&&word(zero_line,0,4)==9,"wrapped cached line-address0 skips byte reread/LF increment but initial line mapping remains required");}
    {Fixture f;f.raw.omitted={{Memory::line,4}};f.refresh();unmapped([&]{(void)f.identify(0);});check(f.characters.classified.empty()&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"initial line mapping is accessed before even null input-byte access/output clear");}
}
void publication_aliases(){
    {Fixture f;f.match(2);f.raw.regions.erase(std::remove_if(f.raw.regions.begin(),f.raw.regions.end(),[](auto r){return r.address==Memory::doc;}),f.raw.regions.end());
        // Alias document line physically to child+68 without overlapping virtual ranges.
        f.raw.regions.push_back({Memory::doc,std::span(f.raw.document).first(0xa0)});f.raw.regions.push_back({Memory::line,std::span(f.raw.objects[1]).subspan(0x68,4)});f.raw.regions.push_back({Memory::doc+0xa4,std::span(f.raw.document).subspan(0xa4)});
        f.raw.memory.regions=f.raw.regions;put(f.raw.objects[1],0x68,0x12345678,4);check(f.identify()==Memory::input_address+9&&f.raw.get(Memory::child+0x30,4)==0x12345678&&f.raw.get(Memory::line,4)==0x12345601,"CDATA line is cached BEFORE aliased BYTE subtype flag write");}
    {Fixture f;f.input(" \nx");f.constructors.hook=[&](const auto&){f.raw.set(Memory::line,51,4);return Memory::child;};check(f.identify(Memory::input_address,Memory::child+0x30)==Memory::input_address&&f.raw.get(Memory::line,4)==7&&f.raw.get(Memory::child+0x30)==Memory::child,"fallback stores node line then rewinds document line before aliased output publication");}
    {Fixture f;f.match(0);check(f.identify(Memory::input_address,Memory::line)==Memory::input_address+2&&f.raw.get(Memory::child+0x30,4)==7&&f.raw.get(Memory::line)==Memory::child,"out-node publication may alias document line after node.line captured");}
    {Fixture f;f.match(2);check(f.identify(Memory::input_address,Memory::child+0x68)==Memory::input_address+9&&f.raw.get(Memory::child+0x68)==Memory::child&&f.raw.get(Memory::child+0x30,4)==7,"output alias may overwrite prior subtype BYTE only after its source order");}
    {Fixture f;std::array<std::uint8_t,0x90> null_node{};f.extra_regions.push_back({0,null_node});f.refresh();f.match(4);f.constructors.hook=[](const auto&){return std::uintptr_t{0};};check(f.identify()==Memory::input_address+1&&word(null_node,0x30,4)==7&&!f.raw.get(Fixture::out),"constructor0 is not guarded when explicitly mapped address0 supports native stores");}
}
void failure_prefixes(){
    {Fixture f;f.characters.classifier=[](auto)->std::int32_t{throw Failure{};};fails([&]{(void)f.identify();});check(f.parser.comparisons.empty()&&f.constructors.calls.empty()&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"classifier failure leaves original out value with no later effects");}
    for(unsigned phase=0;phase<5;++phase){Fixture f;f.input("\nx");f.parser.compare_hook=[&,phase](const auto&)->std::int32_t{if(f.parser.comparisons.size()==phase+1)throw Failure{};return 1;};fails([&]{(void)f.identify();});check(f.parser.comparisons.size()==phase+1&&f.constructors.calls.empty()&&f.raw.get(Memory::line,4)==8&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"compare failure preserves whitespace prefix and unmodified output");}
    for(unsigned chosen=0;chosen<6;++chosen){Fixture f;if(chosen<5)f.match(chosen);f.constructors.hook=[&](const auto&)->std::uintptr_t{f.raw.set(Memory::line,99,4);throw Failure{};};fails([&]{(void)f.identify();});check(f.raw.get(Memory::line,4)==99&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL&&f.raw.get(Memory::child+0x30,4)==0xa5a5a5a5U,"constructor failure retains callback line change without invented rewind/free/output publication");}
    {Fixture f;f.token_bytes=8;f.refresh();unmapped([&]{(void)f.identify();});check(f.parser.comparisons.size()==1&&f.constructors.calls.empty()&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"later missing token slot is reached lazily after earlier comparison");}
    {Fixture f;f.match(2);f.raw.omitted={{Memory::child+0x30,4}};f.refresh();unmapped([&]{(void)f.identify();});check(f.raw.get(Memory::child+0x68,1)==1&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"missing node.line mapping retains earlier CDATA BYTE but prevents publication");}
    {Fixture f;f.input("\nx");f.raw.omitted={{Fixture::out,8}};f.refresh();unmapped([&]{(void)f.identify();});check(f.raw.get(Memory::child+0x30,4)==8&&f.raw.get(Memory::line,4)==7,"missing final out mapping fails only after fallback node.line and line rewind stores");}
    {Fixture f;f.constructors.hook=[](const auto&){return std::uintptr_t{0};};unmapped([&]{(void)f.identify();});check(f.constructors.calls.size()==1&&f.raw.get(Fixture::out)==0xfeedfeedfeedfeedULL,"unmapped constructor0 fails at required node store without early guard");}
}
}
int main(){try{ordered_selection();whitespace_and_input_edges();publication_aliases();failure_prefixes();std::cout<<"PASS: "<<checks<<" node-identify core checks; initial/live line,ordered live token slots,pointer identity,constructor operands,CDATA/fallback aliases and failure prefixes; constructors/classifier/compare/nativeABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
