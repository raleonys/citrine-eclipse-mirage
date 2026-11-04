// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/response_containers_test_fixture.hpp"
using namespace container_test;
namespace {
void copying(){
    for(auto size:{0ULL,1ULL,7ULL,15ULL,16ULL,17ULL,31ULL,32ULL,63ULL,128ULL,4094ULL,4095ULL,4096ULL,8192ULL})for(bool force_heap:{false,true}){
        Fixture f;f.seed_string(size,std::max(size,force_heap?31ULL:15ULL));auto& source=f.bytes.pool.value;auto before=source;
        const auto expected=f.bytes.text(source);s::ReceiveBufferString destination;destination.storage_00.fill(0x77);destination.size_10=999;destination.capacity_18=555;
        f.bytes.pool.api.allocations.clear();const auto old_data=s::receive_buffer_address(source);
        f.bytes.hook=[&](auto event){if(event=="allocate")check(!destination.size_10&&!destination.capacity_18&&destination.storage_00==std::array<std::uint8_t,16>{},"constructor zero prefix precedes allocation");if(event=="copy")check(destination.size_10==size&&destination.capacity_18==((size|15)<22?22:(size|15)),"heap metadata published before size+1 copy");};
        auto& out=s::copy_response_string_009eb0(destination,source,f.strings);check(&out==&destination&&f.bytes.text(out)==expected,"independent full payload copy and return identity");
        check(source.storage_00==before.storage_00&&source.size_10==before.size_10&&source.capacity_18==before.capacity_18,"COPY never resets or changes source prefix");
        if(size<=15){check(out.capacity_18==15&&f.bytes.copies.empty()&&f.bytes.pool.api.allocations.empty(),"inline COPY uses no allocation or078d30 provider");
            const auto input=before.capacity_18<=15?std::span<const std::uint8_t>(before.storage_00):std::span<const std::uint8_t>(f.bytes.memory.view(old_data,16));check(std::equal(input.begin(),input.end(),out.storage_00.begin()),"native short copy reads all16 bytes including trailing bytes");
        }else{const auto cap=size|15;check(out.capacity_18==cap&&s::receive_buffer_address(out)!=old_data,"heap independent rounded capacity");
            check(f.bytes.pool.api.allocations==std::vector<std::uint64_t>{cap+1+(cap+1>=4096?39U:0U)}&&f.bytes.copies.back().count==size+1,"exact allocation threshold and copied NUL byte count");
            const auto data=s::receive_buffer_address(out);check(f.bytes.memory.view(data+size,1)[0]==0,"NUL copied from source rather than invented strlen boundary");
            if(cap+1>=4096)check(data%32==0&&b::word(f.bytes.memory.view(data-8,8))==f.bytes.pool.blocks.back()->raw,"large aligned allocation records raw header");
            f.bytes.memory.view(old_data,1)[0]='X';check(f.bytes.text(out)==expected,"destination survives source mutation");f.bytes.destroy(out);
        }
    }
    {Fixture f;f.seed_string(32,63);auto& source=f.bytes.pool.value;const auto data=s::receive_buffer_address(source);f.bytes.pool.api.on_allocate=[&](){source.size_10=1;source.capacity_18=15;source.storage_00.fill('Q');};s::ReceiveBufferString out;s::copy_response_string_009eb0(out,source,f.strings);check(out.size_10==32&&f.bytes.copies[0].from==data&&f.bytes.text(out).substr(0,4)=="abcd","length and data cached across allocator mutation");}
    {Fixture f;f.seed_string(16,16);s::ReceiveBufferString out;f.bytes.hook=[&](auto event){if(event=="copy")throw Failure{9};};rejects<Failure>([&]{s::copy_response_string_009eb0(out,f.bytes.pool.value,f.strings);});check(out.size_10==16&&out.capacity_18==31&&f.bytes.pool.api.frees.empty(),"COPY exception retains observed published prefix, no invented rollback");}
    for(auto length:{0x8000000000000000ULL,UINT64_MAX}){Fixture f;s::ReceiveBufferString source,out;source.size_10=length;out.size_10=123;try{s::copy_response_string_009eb0(out,source,f.strings);r::unexpected();}catch(const Failure& e){check(e.code==1&&!out.size_10&&!out.capacity_18&&out.storage_00==std::array<std::uint8_t,16>{},"length terminal follows destination prefix zeroing");}check(f.bytes.pool.api.allocations.empty(),"oversize length does not allocate");}
    for(auto size:{16ULL,4095ULL}){Fixture f;f.seed_string(size,size);f.bytes.pool.api.null_allocate=true;s::ReceiveBufferString out;bool failed=false;
        try{s::copy_response_string_009eb0(out,f.bytes.pool.value,f.strings);}catch(const Failure& e){failed=true;check(size==4095&&e.code==3&&!out.size_10&&!out.capacity_18,"large null allocation terminal before publication");}catch(const s::UnsupportedInput&){failed=true;check(size==16&&out.size_10==16&&out.capacity_18==31&&s::receive_buffer_address(out)==0,"small null is unchecked before COPY, bounded provider rejects");}check(failed,"null paths terminate at correct boundary");}
    {Fixture f;s::ReceiveBufferString source,out;source.size_10=0x7fffffffffffffffULL;f.bytes.pool.api.on_allocate=[&](){check(f.bytes.pool.api.allocations.back()==0x8000000000000027ULL,"symbolic maximum allocation arithmetic");throw Failure{90};};rejects<Failure>([&]{s::copy_response_string_009eb0(out,source,f.strings);});check(!out.size_10&&!out.capacity_18&&f.bytes.copies.empty(),"symbolic huge allocation failure before pointer publish");}
}
void growth(){
    for(std::uint64_t old:{0,1,2,3,4,5,6,7,8,9,16,64,512})for(std::uint64_t extra:{0,1,2,9,100})for(std::uint64_t offset:{0,1,3,7}){
        if((!old&&offset)||(old&&offset>=old))continue;Fixture f;f.seed_map(old,offset);auto before=std::vector<s::WriteFinishString*>(f.queue.map_d0.begin(),f.queue.map_d0.end());auto capacity=old?old:1;while(capacity<8||capacity-old<extra)capacity*=2;
        std::vector<s::WriteFinishString*> expected(capacity,nullptr);const auto delta=capacity-old;
        for(std::size_t i=0;i<old;++i)expected[i>=offset?i:(i<delta?old+i:i-delta)]=before[i];
        const auto count=f.queue.count_e8;const auto old_address=f.bytes.maps.address_of(f.queue.map_d0);f.grow(extra);
        check(f.queue.map_size_d8==capacity&&std::equal(expected.begin(),expected.end(),f.queue.map_d0.begin()),"independent pointer permutation/zero oracle for both native growth branches");
        check(f.queue.offset_e0==offset&&f.queue.count_e8==count,"growth changes only map pointer/size without consuming elements");
        check(f.bytes.pool.api.allocations==std::vector<std::uint64_t>{capacity*8+(capacity*8>=4096?39U:0U)},"native pointer map exact allocation size");
        check(f.bytes.pool.api.frees.size()==(old?1U:0U),"old map freed exactly once iff nonnull");
        if(old)check(f.bytes.pool.api.frees[0].second==old*8+(old*8>=4096?39U:0U),"assembly02682c passes sized map delete bytes");
        check(f.api.copies.size()==(offset>delta?3U:2U)&&f.api.zeros.size()==(offset>delta?1U:2U),"both native copy/fill call sequences including zero-byte calls preserved");
        check(f.api.copies.front().from==old_address+offset*8&&f.api.copies.front().count==(old-offset)*8,"first copy literal saved-offset suffix");
    }
}
void reloads(){
    {Fixture f;f.seed_map(8,3);const auto original=f.queue.map_d0;auto raw=f.bytes.allocate_04b0bc(64);auto alternate=f.bytes.maps.view(raw,64);for(std::size_t i=0;i<8;++i)alternate[i]=&f.cells[100+i];
        f.api.hook=[&](auto event){if(event=="copy1")f.queue.map_d0=alternate;};f.grow(1);
        check(f.queue.map_d0[3]==original[3]&&f.queue.map_d0[8]==&f.cells[100]&&f.queue.map_d0[10]==&f.cells[102],"map pointer reloaded after first COPY for prefix and final free");check(f.bytes.pool.api.frees.back().first==raw,"final old map free uses live replaced map");}
    {Fixture f;f.seed_map(8,3);f.bytes.hook=[&](auto event){if(event=="allocate"){f.queue.offset_e0=7;f.queue.count_e8=99;}if(event=="free"){f.queue.map_size_d8=100;f.queue.count_e8=101;}};f.grow(1);
        check(f.api.copies[0].count==40&&f.queue.map_size_d8==108&&f.queue.offset_e0==7&&f.queue.count_e8==101,"saved offset/delta, live size ADD after free; provider field mutations preserved");}
    {Fixture f;f.seed_map(8,3);f.bytes.hook=[&](auto event){if(event=="allocate")f.queue.map_size_d8=7;};f.grow(1);check(f.api.copies[0].count==32&&f.queue.map_size_d8==16,"old size reloaded after allocation for delta and suffix length");}
    {Fixture f;f.seed_map(8,3);f.api.hook=[&](auto event){if(event=="copy2")throw Failure{9};};const auto before=f.queue.map_d0.data();rejects<Failure>([&]{f.grow(1);});check(f.queue.map_d0.data()==before&&f.queue.map_size_d8==8&&f.bytes.pool.api.frees.empty(),"copy failure before free/publish has no invented cleanup or rollback");}
    for(auto size:{0ULL,8ULL,0x1fffffffffffffffULL}){Fixture f;f.queue.map_size_d8=size;try{f.grow(UINT64_MAX);r::unexpected();}catch(const Failure& e){check(e.code==4&&f.bytes.pool.api.allocations.empty(),"doubling length guard before allocation");}}
    {Fixture f;f.queue.map_size_d8=0x2000000000000000ULL;try{f.grow(0);r::unexpected();}catch(const Failure& e){check(e.code==2&&f.bytes.pool.api.allocations.empty(),"preallocation map byte overflow terminal");}}
    {Fixture f;f.seed_map(512);const auto data=f.bytes.maps.address_of(f.queue.map_d0);b::word(f.bytes.memory.view(data-8,8),data-40);auto* before=f.queue.map_d0.data();try{f.grow(1);r::unexpected();}catch(const Failure& e){check(e.code==3&&f.bytes.pool.api.frees.empty()&&f.queue.map_d0.data()==before&&f.queue.map_size_d8==512,"invalid large old-map header fails after copies before free/publication");}}
    {Fixture f;f.seed_map(512);f.bytes.pool.api.null_allocate=true;try{f.grow(1);r::unexpected();}catch(const Failure& e){check(e.code==3&&f.api.copies.empty()&&f.bytes.pool.api.frees.empty(),"large null new-map terminal before any copy/free");}}
    {Fixture f;f.seed_map(8);f.bytes.pool.api.null_allocate=true;rejects<s::UnsupportedInput>([&]{f.grow(1);});check(f.bytes.pool.api.frees.empty()&&f.queue.map_size_d8==8,"small null new map rejected by bounded COPY, no invented invalid handler");}
    {Fixture f;f.seed_map(8);const auto address=f.bytes.maps.address_of(f.queue.map_d0);rejects<s::UnsupportedInput>([&]{f.bytes.maps.view(address+1,8);});rejects<s::UnsupportedInput>([&]{f.bytes.maps.view(address,7);});f.bytes.bindings[31]=f.bytes.bindings[0];rejects<s::UnsupportedInput>([&]{f.bytes.maps.address_of(f.queue.map_d0);});}
}
}
int main(){try{copying();growth();reloads();std::cout<<"PASS: "<<checks<<" socket response container checks; actual009eb0/026670, independent copy/NUL/alignment, pointer permutation and zero oracle, cached/live fields and terminal prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
