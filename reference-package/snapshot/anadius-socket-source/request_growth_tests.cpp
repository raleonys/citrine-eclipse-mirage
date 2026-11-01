#include "request_growth_test_fixture.hpp"
using namespace growth_test;
namespace {
void external(){
    for(std::uint64_t size:{0,1,7,15,16,31,63,100,4094,4095,8191})for(std::uint64_t spare:{0,7,32})for(unsigned position:{0U,1U,2U})for(std::uint64_t count:{1,16,100,4096}){
        Fixture f;std::string original(size,'Q');if(size>3)original[size/2]=0;const auto oldcap=std::max<std::uint64_t>(15,size+spare);f.seed(original,oldcap);
        const auto offset=position==0?0:(position==1?size/2:size);const auto erase=std::min<std::uint64_t>(size-offset,3);const auto insert=count+erase;const auto source=f.source(insert);
        std::string bytes(reinterpret_cast<const char*>(f.external.data()),insert);auto expected=original;expected.replace(offset,erase,bytes);const auto prior=f.value().storage_00;const auto old_data=s::receive_buffer_address(f.value());
        const auto cap=std::max<std::uint64_t>((size+count)|15,oldcap+oldcap/2);
        f.f.copy.before=[&](unsigned n){check(f.value().size_10==expected.size()&&f.value().capacity_18==cap&&f.value().storage_00==prior,"new metadata before all3 copies; pointer prefix not yet published");check(n<=3,"exact3 lower copies");};
        auto& result=f.grow(count,offset,erase,source,insert);check(&result==&f.value()&&f.text()==expected&&f.value().capacity_18==cap,"independent binary std::string replace oracle and original return identity");
        const auto data=s::receive_buffer_address(f.value());check(data!=old_data&&f.f.f.memory.view(data+expected.size(),1)[0]==0,"independent new storage and original suffix NUL");
        check(f.f.f.api.allocations==std::vector<std::uint64_t>{cap+1+(cap+1>=4096?39U:0U)}&&f.f.copy.calls.size()==3,"exact rounded/geometric allocation and3 byte-copy calls");
        check(f.f.copy.calls[0].count==offset&&f.f.copy.calls[1].count==insert&&f.f.copy.calls[2].count==size-offset-erase+1,"prefix/insert/suffix+NUL exact byte counts");
        check(f.f.f.api.frees.size()==(oldcap>15?1U:0U),"old heap freed exactly once; inline never freed");
        if(oldcap>15)check(f.f.f.api.frees[0].second==oldcap+1+(oldcap+1>=4096?39U:0U),"sized old delete threshold includes39-byte header overhead");
        if(cap+1>=4096)check(data%32==0&&b::word(f.f.f.memory.view(data-8,8))==f.f.f.blocks.back()->raw,"large new allocation32-byte alignment and raw header");
        check(std::equal(prior.begin()+8,prior.end(),f.value().storage_00.begin()+8),"pointer publication modifies only first8 storage bytes");
    }
}
void closure_and_alias(){
    for(std::uint64_t size:{1,7,15,16,31,63})for(std::uint64_t source_offset:{0,1,5})for(unsigned pos:{0U,1U,2U}){
        if(source_offset>size)continue;Fixture f;std::string old;for(std::size_t i=0;i<size;++i)old.push_back(static_cast<char>('a'+i%23));f.seed(old,std::max<std::uint64_t>(15,size));
        const auto spare=f.value().capacity_18-size;const auto insert=std::min<std::uint64_t>(size-source_offset+1,16);if(insert<=spare)continue;
        const auto offset=pos==0?0:(pos==1?size/2:size),source=s::receive_buffer_address(f.value())+source_offset;auto expected=old;expected.insert(offset,(old+std::string("\0",1)).substr(source_offset,insert));
        auto& result=f.replace(offset,0,source,insert);check(&result==&f.value()&&f.text()==expected&&f.f.copy.calls.size()==3,"actual034730→final growth bridge→actual02b940 supports owned self-source before old free");
    }
    {Fixture f;f.seed("0123456789abcdef",16);const auto old=s::receive_buffer_address(f.value());const auto source=f.source(50);f.replace(2,99,source,50);check(f.text()==std::string("01")+std::string(reinterpret_cast<const char*>(f.external.data()),50)&&f.f.f.api.frees.size()==1&&s::receive_buffer_address(f.value())!=old,"actual caller clamps erase then closes formerly mandatory lower-growth hook");}
}
void prefixes(){
    for(unsigned phase:{1U,2U,3U}){Fixture f;f.seed("abcdef",16);const auto prior=f.value().storage_00;auto src=f.source(40);f.f.copy.before=[&](unsigned n){if(n==phase)throw Failure{9};};rejects<Failure>([&]{f.grow(38,2,2,src,40);});check(f.value().size_10==44&&f.value().capacity_18==47&&f.value().storage_00==prior&&f.f.f.api.frees.empty()&&f.f.copy.calls.size()==phase,"copy failure leaves new metadata/old pointer, no invented free/rollback");}
    {Fixture f;f.seed("abcdef",16);const auto prior=f.value();f.f.f.api.on_allocate=[](){throw Failure{9};};rejects<Failure>([&]{f.grow(38,2,2,0,40);});check(f.value().storage_00==prior.storage_00&&f.value().size_10==prior.size_10&&f.value().capacity_18==prior.capacity_18,"allocator failure precedes metadata writes");}
    {Fixture f;f.seed("abcdef",16);auto source=f.source(40);std::uintptr_t new_data=0;f.f.copy.after=[&](unsigned n){if(n==1)new_data=f.f.copy.calls[0].to;};f.f.f.api.on_free=[&](){check(f.f.copy.calls.size()==3&&f.value().size_10==44&&f.value().capacity_18==47,"old free occurs after all copies with metadata already published");f.value().storage_00.fill(0);f.value().size_10=77;f.value().capacity_18=88;};f.grow(38,2,2,source,40);check(s::receive_buffer_address(f.value())==new_data&&f.value().size_10==77&&f.value().capacity_18==88,"final pointer publication wins over free callback, metadata not rewritten");}
    {Fixture f;f.seed("abcdef",16);auto source=f.source(40);const auto prior=f.value().storage_00;f.f.f.api.on_free=[](){throw Failure{9};};rejects<Failure>([&]{f.grow(38,2,2,source,40);});check(f.value().storage_00==prior&&f.value().size_10==44&&f.f.copy.calls.size()==3,"free exception prevents final pointer publication without caller rollback");}
    {Fixture f;f.seed("abcdef",16);b::Fixture replacement;replacement.next_raw=0x700003;replacement.seed(16,6);const auto pointer=s::receive_buffer_address(replacement.value);f.f.f.regions[30]=replacement.regions[1];auto source=f.source(40);f.f.f.api.on_allocate=[&](){b::word(f.value().storage_00,pointer);};f.f.f.api.on_free=[&](){throw Failure{19};};try{f.grow(38,2,2,source,40);r::unexpected();}catch(const Failure& e){check(e.code==19&&f.f.copy.calls[0].from==pointer,"heap old pointer read live after allocation; cached old capacity selects branch");}}
    for(bool large:{false,true}){Fixture f;f.seed("abcdef",16);f.f.f.api.null_allocate=true;auto source=f.source(large?5000:40);try{f.grow(large?4998:38,2,2,source,large?5000:40);r::unexpected();}catch(const b::Terminal& e){check(large&&e.kind==3&&f.value().size_10==6&&f.f.copy.calls.empty(),"largeNULL invalid terminal before metadata/COPY");}catch(const s::UnsupportedInput&){check(!large&&f.value().size_10==44&&f.f.copy.calls.size()==1,"smallNULL unchecked until bounded COPY after metadata");}}
    {Fixture f;f.value().size_10=0x7fffffffffffffffULL;try{f.grow(1,0,0,0,1);r::unexpected();}catch(const b::Terminal& e){check(e.kind==1&&f.f.f.api.allocations.empty()&&f.f.copy.calls.empty(),"length terminal before old capacity/allocation");}}
    for(bool cap_overflow:{false,true}){Fixture f;f.seed("abcdef",15);if(cap_overflow)f.value().capacity_18=UINT64_MAX;else f.value().size_10=0x7ffffffffffffffeULL;f.f.f.api.on_allocate=[&](){check(f.f.f.api.allocations.back()==0x8000000000000027ULL,"symbolic maximum fallback allocation exact");throw Failure{9};};rejects<Failure>([&]{f.grow(1,0,0,0,1);});check(f.f.copy.calls.empty(),"huge symbolic provider failure no real allocation/COPY");}
    {Fixture f;f.seed(std::string(4095,'H'),4095);const auto data=s::receive_buffer_address(f.value());b::word(f.f.f.memory.view(data-8,8),data-40);auto source=f.source(1);try{f.grow(1,0,0,source,1);r::unexpected();}catch(const b::Terminal& e){check(e.kind==3&&f.f.copy.calls.size()==3&&f.f.f.api.frees.empty()&&s::receive_buffer_address(f.value())==data,"bad old header rejected after3 copies before free/final pointer");}}
}
}
int main(){try{external();closure_and_alias();prefixes();std::cout<<"PASS: "<<checks<<" socket request growth checks; actual02b940 and034730 provider closure, binary/alias oracle, metadata-before-COPY, sized-free-before-pointer, alignment and failure prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
