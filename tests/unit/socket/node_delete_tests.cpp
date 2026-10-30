#include "tests/support/socket/node_delete_test_fixture.hpp"
#include <type_traits>
using namespace delete_test;
namespace {
using G=Graph;
static_assert(!std::is_copy_constructible_v<s::RecoveredNodeDeleteEffects>);
static_assert(!std::is_copy_assignable_v<s::RecoveredNodeDeleteEffects>);
static_assert(!std::is_move_constructible_v<s::RecoveredNodeDeleteEffects>);
static_assert(!std::is_move_assignable_v<s::RecoveredNodeDeleteEffects>);

void links(Fixture& f,std::initializer_list<unsigned> ids,std::uintptr_t owner=State::base){
    const std::vector<unsigned> list(ids);
    f.graph.set(owner+0x38,list.empty()?0:G::node(list.front()));
    f.graph.set(owner+0x40,list.empty()?0:G::node(list.back()));
    for(std::size_t i=0;i<list.size();++i){const auto n=G::node(list[i]);f.graph.set(n+0x10,owner);f.graph.set(n+0x48,i?G::node(list[i-1]):0);f.graph.set(n+0x50,i+1<list.size()?G::node(list[i+1]):0);}
}
void run(Fixture& f,bool attached){if(attached)f.unlink(State::base,G::node());else f.erase(G::node());}
void virtual_calls(const Fixture& f,std::uintptr_t probe=0xf001,std::uintptr_t destroy=0xf002,std::uintptr_t pool=G::pool(),std::uintptr_t release=0xe001){
    check(f.nodes.calls.size()==3&&f.nodes.trace==std::vector<std::string>{"probe","destroy","release"},"exact mandatory virtual effect order");
    const auto& p=f.nodes.calls[0];const auto& d=f.nodes.calls[1];const auto& r=f.nodes.calls[2];
    check(p.target==probe&&p.first==G::node(),"probe forwards captured target and node");
    check(d.target==destroy&&d.first==G::node()&&!d.flags,"destructor forwards live target, original node and zero flags");
    check(r.target==release&&r.first==pool&&r.second==G::node(),"release forwards cached pool and original node in second argument");
    check(!f.nodes.parents&&!f.nodes.deletes&&!f.nodes.tracks,"both final helpers execute bodies without old helper stubs or pool track");
}
std::uint64_t raw_word(const std::array<std::uint8_t,0x80>& bytes,std::size_t at){std::uint64_t n=0;for(unsigned i=0;i<8;++i)n|=std::uint64_t(bytes[at+i])<<(8*i);return n;}
void omit_node(Fixture& f,unsigned node,std::size_t at,std::size_t size){
    f.state.regions[450+node]={};auto bytes=std::span(f.graph.node_bytes[node]);
    f.state.regions[499]={G::node(node),bytes.first(at)};
    f.state.regions[500]={G::node(node)+at+size,bytes.subspan(at+size)};
}
void parent_positions(){
    for(unsigned selected=0;selected<4;++selected){Fixture f;f.construct();
        if(selected==3)links(f,{0});else if(selected==0)links(f,{0,1,2});else if(selected==1)links(f,{1,0,2});else links(f,{1,2,0});
        const auto first=selected==0?G::node(1):selected==3?0:G::node(1);
        const auto last=selected==2?G::node(2):selected==3?0:G::node(2);
        f.nodes.hook=[&](const auto& name){if(name=="probe")check(!f.graph.get(G::node()+0x10)&&!f.graph.get(G::node()+0x48)&&!f.graph.get(G::node()+0x50),"parent/previous/next zero before probe");};
        f.unlink(State::base,G::node());
        check(f.state.get(0x38)==first&&f.state.get(0x40)==last,"first/middle/last/single child parent endpoints");
        if(selected==0)check(!f.graph.get(G::node(1)+0x48),"first deletion repairs next previous link");
        if(selected==1)check(f.graph.get(G::node(1)+0x50)==G::node(2)&&f.graph.get(G::node(2)+0x48)==G::node(1),"middle deletion joins both neighbors");
        if(selected==2)check(!f.graph.get(G::node(2)+0x50),"last deletion repairs previous next link");
        virtual_calls(f);
    }
    {Fixture f;f.construct();links(f,{1,2});f.graph.set(G::node()+0x10,G::node(15));f.graph.set(G::node()+0x48,G::node(7));f.graph.set(G::node()+0x50,G::node(8));f.graph.set(G::node(7)+0x50,G::node());f.graph.set(G::node(8)+0x48,G::node());f.unlink(State::base,G::node());
        check(f.state.get(0x38)==G::node(1)&&f.state.get(0x40)==G::node(2),"mismatching passed parent endpoints stay intact without membership validation");
        check(f.graph.get(G::node(7)+0x50)==G::node(8)&&f.graph.get(G::node(8)+0x48)==G::node(7)&&!f.graph.get(G::node()+0x10),"mismatching stored parent does not prevent sibling repair and zeroing");virtual_calls(f);}
    {Fixture f;f.construct();links(f,{1});try{f.unlink(State::base,0);throw std::runtime_error("missing null unlink bounded failure");}catch(const s::UnsupportedInput&){check(f.nodes.calls.empty(),"003030 has no invented null-node no-op");}}
}
void standalone_and_probe(){
    {Fixture f;f.construct();f.state.regions.fill({});f.erase(0);check(f.nodes.calls.empty(),"003240 null node touches neither bounded memory nor callbacks");}
    {Fixture f;f.construct();links(f,{1,0,2});const auto before=f.graph.node_bytes;const auto first=f.state.get(0x38),last=f.state.get(0x40);f.nodes.hook=[&](const auto& name){if(name=="probe")check(f.graph.node_bytes==before,"standalone deletion preserves all links before probe");};f.erase(G::node());check(f.graph.node_bytes==before&&f.state.get(0x38)==first&&f.state.get(0x40)==last,"003240 performs no parent or sibling unlink");virtual_calls(f);}
    for(bool attached:{false,true})for(const std::uint64_t value:{0x100000000ULL,0x8000000000000000ULL}){Fixture f;f.construct();links(f,{0});f.nodes.probe_result=value;f.graph.set(G::node()+8,0xdead);run(f,attached);virtual_calls(f);}
}
void tracking(){
    for(bool attached:{false,true}){
        for(std::uint32_t count:{0U,0xffffffffU,0x80000000U}){Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0x104,count,4);f.state.put(0xa8,0xdead);run(f,attached);check(f.state.get(0x104,4)==count,"signed nonpositive count skips inaccessible tracking table");virtual_calls(f);}
        for(unsigned match:{0U,1U,2U,3U}){Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0x104,3,4);f.state.put(0xa8,G::tracking());for(unsigned i=0;i<3;++i)f.graph.set(G::tracking()+i*8,G::node(8+i));if(match<3)f.graph.set(G::tracking()+match*8,G::node());const auto dead=f.graph.get(G::tracking()+16);run(f,attached);
            check(f.state.get(0x104,4)==(match<3?2:3),"positive tracking count decremented only after match");
            for(unsigned i=0;i<3;++i)check(f.graph.get(G::tracking()+i*8)==(i==match?dead:G::node(8+i)),"first/middle/last swap or absent search retains exact slots including dead tail");virtual_calls(f);}
        {Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0x104,1,4);f.state.put(0xa8,G::tracking());f.graph.set(G::tracking(),G::node());run(f,attached);check(!f.state.get(0x104,4)&&f.graph.get(G::tracking())==G::node(),"single tracked node leaves dead slot while count becomes zero");}
        // Entry zero overlaps the count DWORD. The replacement store changes
        // count to three; native DEC must reload and produce two, not six.
        {Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0xa8,State::base+0x100);f.state.put(0x100,G::node());f.state.put(0x130,0x312345678ULL);run(f,attached);check(f.state.get(0x100,4)==0x12345678&&f.state.get(0x104,4)==2,"live DEC follows aliased replacement store rather than cached initial count");}
    }
    // Retained assembly 0030ca MOV EAX reloads/zero-extends count, whereas
    // 003292 uses cached positive R8. There is no intervening write or effect
    // between their initial and replacement count reads. Deterministic bounded
    // memory cannot distinguish those two reads; no synthetic callback or
    // concurrent mutation is introduced as purported native evidence.
}
void aliases_and_live_targets(){
    {Fixture f;f.construct();const auto parent=G::node(15);links(f,{0},parent);f.graph.set(G::node()+0x50,G::node(1));f.graph.set(G::node()+0x48,G::node(2));
        f.state.regions[465]={};auto bytes=std::span(f.graph.node_bytes[15]);f.state.regions[499]={parent,bytes.first(0x40)};f.state.regions[500]={parent+0x40,bytes.subspan(0x38,8)};f.state.regions[501]={parent+0x48,bytes.subspan(0x48)};
        f.unlink(parent,G::node());check(f.graph.get(parent+0x38)==G::node(1)&&f.graph.get(parent+0x40)==G::node(1),"last endpoint reread observes first endpoint store through shared backing");virtual_calls(f);}
    {Fixture f;f.construct();links(f,{1,0,2});auto bytes=std::span(f.graph.node_bytes[1]);f.state.regions[451]={};f.state.regions[499]={G::node(1),bytes.first(0x50)};f.state.regions[500]={G::node(1)+0x50,std::span(f.graph.node_bytes[0]).subspan(0x48,8)};f.state.regions[501]={G::node(1)+0x58,bytes.subspan(0x58)};
        f.unlink(State::base,G::node());check(f.graph.get(G::node(2)+0x48)==G::node(2),"next previous repair rereads node previous after aliased previous-next store");virtual_calls(f);}
    {Fixture f;f.construct();links(f,{1,0,2});auto bytes=std::span(f.graph.node_bytes[1]);std::array<std::uint8_t,0x80> changed_next{};constexpr std::uintptr_t changed=0x700000007ULL;
        // A partly overlapping previous-next store replaces the low DWORD of
        // node.next with seven. Both the next pointer and previous value must
        // be reread after that store; neither is a cached traversal field.
        f.state.regions[451]={};f.state.regions[499]={G::node(1),bytes.first(0x50)};f.state.regions[500]={G::node(1)+0x50,std::span(f.graph.node_bytes[0]).subspan(0x4c,8)};f.state.regions[501]={G::node(1)+0x58,bytes.subspan(0x58)};f.state.regions[502]={changed,changed_next};
        f.unlink(State::base,G::node());check(f.graph.get(changed+0x48)==0x1000020010000100ULL&&f.graph.get(G::node(2)+0x48)==G::node(),"next pointer and previous value both reload after partially overlapping sibling repair");virtual_calls(f);}
    {Fixture f;f.construct();links(f,{0});f.state.regions[450]={};f.state.regions[499]={G::node(),std::span(f.graph.node_bytes[0]).subspan(0x10,8)};f.state.regions[500]={G::node()+8,std::span(f.graph.node_bytes[0]).subspan(8)};f.graph.set(G::node(),G::table());f.nodes.hook=[&](const auto& name){if(name=="probe"){check(f.nodes.calls.back().target==0xf001&&!f.graph.get(G::node()),"cached vptr survives parent zero through shared backing");f.graph.set(G::node(),G::table());}};f.unlink(State::base,G::node());virtual_calls(f);}
    {Fixture f;f.construct();links(f,{0});f.graph.set(G::node(),G::node()+0x10);f.nodes.hook=[&](const auto& name){if(name=="probe"){check(!f.nodes.calls.back().target,"probe slot loaded from captured table after next-link zero");f.graph.set(G::node(),G::table());}};f.unlink(State::base,G::node());virtual_calls(f,0);}
    for(bool attached:{false,true}){Fixture f;f.construct();links(f,{0});f.nodes.hook=[&](const auto& name){if(name=="probe")f.graph.set(G::node(),G::table(1));if(name=="destroy"){check(f.nodes.calls.back().target==0xf012,"destructor uses vptr reloaded after probe");f.graph.set(G::node()+0x60,G::pool(1));f.graph.set(G::pool(),G::pooltable(2));}};run(f,attached);virtual_calls(f,0xf001,0xf012,G::pool(),0xe021);}
}
void throwing_prefixes(){
    for(bool attached:{false,true})for(const std::string stage:{"probe","destroy","release"}){Fixture f;f.construct();links(f,{0,1});f.nodes.probe_result=0;f.state.put(0x104,1,4);f.state.put(0xa8,G::tracking());f.graph.set(G::tracking(),G::node());f.nodes.hook=[&](const auto& name){if(name==stage)throw Failure{77};};try{run(f,attached);throw std::runtime_error("missing virtual failure");}catch(const Failure& e){
        check(e.code==77&&f.nodes.trace.back()==stage&&f.nodes.probes==1&&f.nodes.destroys==unsigned(stage!="probe")&&f.nodes.releases==unsigned(stage=="release"),"throw preserves exact effect prefix without invented subsequent effect");
        check(f.state.get(0x104,4)==unsigned(stage=="probe"),"tracking decrement occurs after successful probe and before destructor");
        check(f.state.get(0x38)==(attached?G::node(1):G::node())&&f.state.get(0x40)==G::node(1),"throw retains the helper-specific parent mutation prefix");
        check(f.graph.get(G::node()+0x10)==(attached?0:State::base)&&f.graph.get(G::node()+0x50)==(attached?0:G::node(1)),"throw retains helper-specific node-link mutation prefix");
        check(!f.nodes.parents&&!f.nodes.deletes,"throw path never routes through lower helper stubs");
    }}
}
void bounded_failures(){
    {Fixture f;f.construct();links(f,{0,1});omit_node(f,0,0x10,8);try{f.unlink(State::base,G::node());throw std::runtime_error("missing parent-store bounds failure");}catch(const s::UnsupportedInput&){check(f.state.get(0x38)==G::node(1)&&!raw_word(f.graph.node_bytes[0],0x50)&&!raw_word(f.graph.node_bytes[0],0x48)&&raw_word(f.graph.node_bytes[0],0x10)==State::base&&f.nodes.calls.empty(),"missing parent field preserves earlier unlink/zero stores and stops before probe");}}
    for(bool attached:{false,true})for(unsigned stage=0;stage<4;++stage){Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0xa8,G::tracking());f.state.put(0x104,1,4);f.graph.set(G::tracking(),G::node());
        if(stage==0){f.state.regions[466]={};auto bytes=std::span(f.graph.table_bytes[0]);f.state.regions[499]={G::table(),bytes.first(0x38)};f.state.regions[500]={G::table()+0x40,bytes.subspan(0x40)};}
        if(stage==1){f.state.omitted={{0x104,4}};f.graph.refresh();}
        if(stage==2){f.state.regions[466]={};f.state.regions[499]={G::table(),std::span(f.graph.table_bytes[0]).first(0x78)};}
        if(stage==3){f.state.regions[468]={};f.state.regions[499]={G::pooltable(),std::span(f.graph.pooltable_bytes[0]).first(0x18)};}
        try{run(f,attached);throw std::runtime_error("missing bounded target/field failure");}catch(const s::UnsupportedInput&){
            check(f.nodes.probes==unsigned(stage>0)&&f.nodes.destroys==unsigned(stage==3)&&!f.nodes.releases,"missing field stops at precise virtual call prefix");
            if(stage!=1)check(f.state.get(0x104,4)==unsigned(stage<2),"bounded failure retains completed tracking swap/decrement only");
            check(f.graph.get(G::node()+0x10)==(attached?0:State::base),"bounded failure retains helper-specific link mutation prefix");
        }
    }
    for(bool attached:{false,true}){Fixture f;f.construct();links(f,{0});f.nodes.probe_result=0;f.state.put(0x104,2,4);f.state.put(0xa8,G::tracking());f.graph.set(G::tracking(),G::node(8));f.state.regions[469]={};f.state.regions[499]={G::tracking(),std::span(f.graph.tracking_bytes[0]).first(8)};try{run(f,attached);throw std::runtime_error("missing search extent failure");}catch(const s::UnsupportedInput&){check(f.nodes.probes==1&&!f.nodes.destroys&&!f.nodes.releases&&f.state.get(0x104,4)==2&&f.graph.get(G::tracking())==G::node(8),"missing later search slot preserves unmatched tracking state and probe-only prefix");}}
}
}
int main(){try{parent_positions();standalone_and_probe();tracking();aliases_and_live_targets();throwing_prefixes();bounded_failures();std::cout<<"PASS: "<<checks<<" node delete checks; parent positions/mismatch, standalone null/link preservation, full64 probe, signed tracking/swap/dead slots, alias reloads, live targets and bounded/throw prefixes; native vtable/DOM/pool/ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
