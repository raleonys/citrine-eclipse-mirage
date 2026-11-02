// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/request_mutation_test_fixture.hpp"
using namespace mutation_test;
namespace {
void decode(){
    Fixture f;for(unsigned high=0;high<256;++high)for(unsigned low=0;low<256;++low){
        auto& value=f.f.value;value.size_10=2;value.capacity_18=15;value.storage_00.fill(0xcc);value.storage_00[0]=static_cast<std::uint8_t>(high);value.storage_00[1]=static_cast<std::uint8_t>(low);
        s::decode_request_hex_03ce30(value,f.buffer);check(value.size_10==1&&value.capacity_18==15&&value.storage_00[0]==oracle_digit(high)*16+oracle_digit(low)&&value.storage_00[1]==0&&value.storage_00[2]==0xcc,"exhaustive256x256 byte pairs match independent nibble table; metadata/NUL/tail preserved");
    }
    for(std::uint64_t size=0;size<70;++size)for(bool heap:{false,true}){Fixture x;std::string input;for(std::size_t i=0;i<size;++i)input.push_back(static_cast<char>((i*71+size)&255));x.seed(input,std::max<std::uint64_t>(size,heap?127:15));const auto cap=x.f.value.capacity_18;const auto data=s::receive_buffer_address(x.f.value);const auto raw=x.f.value.storage_00;
        s::decode_request_hex_03ce30(x.f.value,x.buffer);check(x.text()==oracle_decode(input)&&x.f.value.size_10==size/2&&x.f.value.capacity_18==cap,"variable binary/odd input decode with inline and heap prefixes");check(s::receive_buffer_address(x.f.value)==data&&x.f.memory.view(data+size/2,1)[0]==0&&x.f.api.allocations.empty()&&x.copy.calls.empty(),"decode retains allocation and uses actual010040 shrink/NUL without copy provider");if(cap>15)check(x.f.value.storage_00==raw,"heap prefix pointer bytes untouched by decode");
    }
    {Fixture x;x.seed("41424344",31);const auto address=s::receive_buffer_address(x.f.value);for(auto& region:x.f.regions)if(region.address==address)region.bytes=region.bytes.first(3);rejects<s::UnsupportedInput>([&]{s::decode_request_hex_03ce30(x.f.value,x.buffer);});check(x.f.value.size_10==8&&x.f.memory.view(address,1)[0]=='A',"bounded read failure preserves partial in-place write and original size before tail shrink");}
}
void replacement(){
    for(std::uint64_t size:{0,1,7,15,16,31,100})for(std::uint64_t offset:{0,1,5,15,31,100})for(std::uint64_t erased:{0ULL,1ULL,5ULL,10ULL,UINT64_MAX})for(std::uint64_t inserted:{0,1,4,10,16}){
        if(offset>size)continue;Fixture x;const std::string old(size,'x');x.seed(old,std::max<std::uint64_t>(15,size+32));const auto cut=std::min(erased,size-offset);std::string incoming(reinterpret_cast<const char*>(x.external.data()),inserted);auto expected=old;expected.replace(offset,cut,incoming);const auto cap=x.f.value.capacity_18;
        auto& result=x.replace(offset,erased,0x900000,inserted);check(&result==&x.f.value&&x.text()==expected&&x.f.value.capacity_18==cap,"external-source normal replace matches independent std::string oracle and return prefix");check(x.f.memory.view(s::receive_buffer_address(x.f.value)+expected.size(),1)[0]==0,"replace moves original terminator correctly");check(x.copy.calls.size()==(cut==inserted?1U:(inserted<cut?2U:3U))&&x.api.calls.empty(),"equal/shrink/spare-capacity exact byte-copy call counts including zero lengths");
    }
    unsigned bounded_aliases=0;
    for(std::uint64_t offset=0;offset<=16;++offset)for(std::uint64_t erased:{0,1,4,8})for(std::uint64_t source=0;source<=16;++source)for(std::uint64_t inserted:{0,1,3,6}){
        if(source+inserted>17)continue;Fixture x;const std::string old="abcdefghijklmnop";x.seed(old,63);const auto base=s::receive_buffer_address(x.f.value);const auto cut=std::min(erased,16-offset);auto snapshot=old+std::string("\0",1);auto expected=old;expected.replace(offset,cut,snapshot.substr(source,inserted));
        const bool hazard=inserted>cut&&source+inserted>offset&&source<=16&&source<offset+cut&&(offset+cut-source)>inserted;
        if(hazard){rejects<s::UnsupportedInput>([&]{x.replace(offset,erased,base+source,inserted);});check(x.f.value.size_10==16+inserted-cut&&x.copy.calls.size()==3&&x.copy.calls.back().count==inserted-(offset+cut-source),"raw native alias split may underflow third count; bounded failure retains published size, no corrective clamp");++bounded_aliases;}
        else {x.replace(offset,erased,base+source,inserted);check(x.text()==expected&&x.f.memory.view(base+expected.size(),1)[0]==0,"safe self-overlap cases preserve independent snapshot replacement oracle");}
    }
    check(bounded_aliases>0,"native arithmetic hazard cases explicitly exercised, no native-fault equivalence claim");
}
void call_boundaries(){
    {Fixture x;x.seed("abc",15);const auto before=x.f.value;try{x.replace(4,UINT64_MAX,0xbad,99);r::unexpected();}catch(const Failure& e){check(e.code==1&&x.api.range_errors==1&&x.copy.calls.empty()&&x.f.value.storage_00==before.storage_00&&x.f.value.size_10==3,"out-of-range before clamp/source/copy/metadata effects");}}
    {Fixture x;x.seed("abc",15);b::Fixture other;x.api.grow=[&](auto call)->s::ReceiveBufferString&{check(call.value==&x.f.value&&call.growth==19&&call.hint==0&&call.offset==2&&call.erase==1&&call.source==0x900000&&call.insert==20,"exact lower02b940 args include clamped erase and zero unused hint");return other.value;};auto& result=x.replace(2,99,0x900000,20);check(&result==&other.value&&x.f.value.size_10==3&&x.copy.calls.empty(),"growth provider return identity forwarded without invented in-place mutation");}
    {Fixture x;x.seed("abc",15);rejects<Failure>([&]{x.replace(0,0,0x900000,20);});check(x.f.value.size_10==3&&x.copy.calls.empty(),"lower growth failure no fabricated cleanup or caller mutation");}
    for(unsigned phase:{1U,2U,3U}){Fixture x;x.seed("abcdef",31);x.copy.before=[&](unsigned ordinal){if(ordinal==phase)throw Failure{9};};rejects<Failure>([&]{x.replace(2,1,0x900000,4);});check(x.f.value.size_10==9&&x.copy.calls.size()==phase,"spare-capacity growth publishes size before every failure prefix");}
    {Fixture x;x.seed("abcdef",31);x.copy.after=[&](unsigned ordinal){if(ordinal==1)x.f.value.size_10=99;};x.replace(2,3,0x900000,1);check(x.f.value.size_10==4,"shrink final size uses saved original size after provider mutation");}
    {Fixture x;x.seed("abcdef",31);x.copy.after=[&](unsigned ordinal){if(ordinal==1)x.f.value.size_10=99;};x.replace(2,1,0x900000,4);check(x.f.value.size_10==99,"grow path has no extra metadata store after copies");}
    {Fixture x;x.seed("abcdef",31);const auto pointer=s::receive_buffer_address(x.f.value);x.copy.after=[&](unsigned ordinal){if(ordinal==1){x.f.value.capacity_18=15;x.f.value.storage_00.fill(0);}};x.replace(2,3,0x900000,1);check(x.copy.calls[1].to==pointer+3&&x.copy.calls[1].from==pointer+5&&x.f.value.size_10==4,"shrink destination pointer cached across first copy despite live prefix replacement");}
    {Fixture x;x.seed("abcdef",31);x.copy.before=[](unsigned n){if(n==2)throw Failure{9};};rejects<Failure>([&]{x.replace(2,3,0x900000,1);});check(x.f.value.size_10==6&&x.text()[2]=='A',"shrink second-copy failure retains first bytes but old size");}
    {Fixture x;x.seed("abcdef",31);x.copy.after=[&](unsigned){x.f.value.size_10=99;};x.replace(2,2,0x900000,2);check(x.f.value.size_10==99,"equal replacement has no size store after provider callback");}
}
}
int main(){try{decode();replacement();call_boundaries();std::cout<<"PASS: "<<checks<<" socket request mutation checks; actual03ce30/010040/034730, exhaustive byte pairs, external and safe alias oracles, raw alias-bound failures, exact lower-growth and cached/live prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
