#include "tests/support/socket/node_constructors_test_fixture.hpp"
using namespace node_constructors_test;
namespace {
void exact_images_and_cached_operands(){
    for(unsigned k=0;k<5;++k){Fixture f;const auto expected=image(f.item,k,Fixture::doc,Fixture::pool);
        check(f.construct(k)==Fixture::node,"five constructors return cached allocated node");
        check(std::equal(expected.begin(),expected.end(),f.item.begin()),"full raw image preserves every unwritten byte and native field widths");
        check(f.allocator.calls.size()==1&&f.allocator.calls[0].target==Fixture::target&&f.allocator.calls[0].pool==Fixture::pool,"live pool vtable+10 identity and pool reach abstract allocation dispatch");
        check(f.get(Fixture::doc+0x104,4)==1&&f.get(Fixture::doc+0xb0)==Fixture::node&&f.append.allocations.empty()&&f.append.frees.empty(),"actual005440 records constructed node in document allocation array");
        check(f.item[0x1c]==0xa5&&f.item[0x34]==0xa5&&f.item[0x6c]==0xa5,"base and element padding remain untouched");
        Fixture changed;changed.allocator.hook=[&](const auto& call){check(call.target==Fixture::target,"allocation target read before callback mutation");changed.set(Fixture::pool,0xf999);changed.set(Fixture::doc+8,0xf888);changed.context.identities.vtables.fill(0xf777);return Fixture::node;};
        check(changed.construct(k)==Fixture::node&&changed.get(Fixture::node)==tables[k]&&changed.get(Fixture::node+8)==Fixture::doc&&changed.get(Fixture::node+0x60)==Fixture::pool,"fixed selected table and cached document/pool survive allocation callback mutation");
    }
    for(unsigned k=0;k<5;++k){Fixture f;f.allocation=Fixture::pool;const auto expected=image(f.pool_bytes,k,Fixture::doc,Fixture::pool);check(f.construct(k)==Fixture::pool&&std::equal(expected.begin(),expected.end(),f.pool_bytes.begin()),"allocation may alias pool; already loaded allocation target is not reread after node stores");}
}
void append_growth_and_failure_prefixes(){
    for(unsigned k=0;k<5;++k){Fixture f;f.set(Fixture::doc+0x100,0,4);f.append.allocate_hook=[&](auto bytes){check(bytes==16,"reused005440 capacity growth requests two pointer slots");check(f.get(Fixture::node)==tables[k]&&f.get(Fixture::node+0x60)==Fixture::pool,"all constructor stores precede append allocation");f.set(Fixture::node+0x30,0x12345678,4);return Fixture::grown;};
        check(f.construct(k)==Fixture::node&&f.get(Fixture::node+0x30,4)==0x12345678,"append mutation does not replace cached node return or trigger constructor rewrite");check(f.get(Fixture::doc+0xa8)==Fixture::grown&&f.get(Fixture::grown)==Fixture::node&&f.get(Fixture::doc+0x104,4)==1,"actual005440 growth/copy/publish records node");
        Fixture fail;fail.allocator.hook=[](const auto&)->std::uintptr_t{throw Failure{};};const auto old=fail.item;fails([&]{(void)fail.construct(k);});check(fail.item==old&&fail.get(Fixture::doc+0x104,4)==0&&fail.append.frees.empty(),"allocation failure performs no node writes or invented cleanup");
        Fixture no_append;no_append.set(Fixture::doc+0x100,0,4);no_append.append.allocate_hook=[](auto)->std::uintptr_t{throw Failure{};};const auto expected=image(no_append.item,k,Fixture::doc,Fixture::pool);fails([&]{(void)no_append.construct(k);});check(std::equal(expected.begin(),expected.end(),no_append.item.begin())&&no_append.get(Fixture::doc+0x104,4)==0&&no_append.append.frees.empty(),"append failure retains complete initialized node without rollback or automatic node free");
    }
    for(unsigned k=0;k<5;++k){Fixture f;f.omitted={{Fixture::node+0x10,8}};f.refresh();unmapped([&]{(void)f.construct(k);});check(word(f.item,8)==(k==2?0xa5a5a5a5a5a5a5a5ULL:Fixture::doc),"element delays document store until after base zero prefix; other constructors publish document first");check(word(f.item,0)==0xa5a5a5a5a5a5a5a5ULL&&f.get(Fixture::doc+0x104,4)==0,"early missing node field prevents vtable publication and append");}
    {Fixture f;f.omitted={{Fixture::node+0x74,4}};f.refresh();unmapped([&]{(void)f.construct(2);});check(word(f.item,8)==Fixture::doc&&word(f.item,0x68,4)==0&&word(f.item,0)==0xa5a5a5a5a5a5a5a5ULL&&word(f.item,0x60)==0xa5a5a5a5a5a5a5a5ULL,"element QWORD+70 mapping failure occurs after DWORD+68 and before vtable/pool");}
    {Fixture f;f.omitted={{Fixture::node+0x68,1}};f.refresh();unmapped([&]{(void)f.construct(3);});check(word(f.item,0)==tables[3]&&word(f.item,0x60)==0xa5a5a5a5a5a5a5a5ULL,"text BYTE flag failure follows vtable but precedes pool store");}
    {Fixture f;f.omitted={{Fixture::table+0x10,8}};f.refresh();unmapped([&]{(void)f.construct(0);});check(f.allocator.calls.empty()&&word(f.item,0)==0xa5a5a5a5a5a5a5a5ULL,"unmapped allocation target fails before lower dispatch or node writes");}
    {Fixture f;f.allocation=0;std::array<std::uint8_t,0x90> zero{};zero.fill(0xa5);f.extras.push_back({0,zero});f.refresh();check(f.construct(2)==0&&word(zero,0)==tables[2]&&f.get(Fixture::doc+0x104,4)==1&&!f.get(Fixture::doc+0xb0),"mapped zero allocation has no invented null guard; source initializes and appends zero identity");}
}
}
int main(){try{exact_images_and_cached_operands();append_growth_and_failure_prefixes();std::cout<<"PASS: "<<checks<<" node-constructors core checks; five raw images,widths/padding,cached operands,live allocation target,source005440 growth and failure prefixes; heap/virtualABI supplied/open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}}
