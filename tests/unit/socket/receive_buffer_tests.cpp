// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/receive_buffer_test_fixture.hpp"
#include <algorithm>
using namespace buffer_test;
namespace {
void payload_oracle(){
    for(std::uint64_t capacity:{15ULL,16ULL,31ULL,64ULL,4094ULL,4095ULL,4096ULL})
    for(std::uint64_t size:{0ULL,7ULL,capacity})
    for(std::uint64_t target:{0ULL,1ULL,7ULL,15ULL,16ULL,17ULL,31ULL,32ULL,64ULL,512ULL,4094ULL,4095ULL,4096ULL,65536ULL})
    for(std::uint8_t fill:{std::uint8_t{0},std::uint8_t{0x7f},std::uint8_t{0xff}}){
        Fixture f;f.seed(capacity,size);std::string expected=f.text();expected.resize(static_cast<std::size_t>(target),static_cast<char>(fill));
        const auto pointer=s::receive_buffer_address(f.value);
        const auto prefix=f.value.storage_00;
        s::resize_receive_buffer_010040(f.value,target,fill,f.context);
        check(f.value.size_10==target&&f.text()==expected,"independent std::string payload oracle for shrink/in-place/growth including embedded NUL");
        check(f.memory.view(s::receive_buffer_address(f.value)+target,1)[0]==0,"NUL always follows requested size");
        if(target<=capacity){
            check(f.api.allocations.empty()&&f.api.frees.empty()&&f.value.capacity_18==capacity&&s::receive_buffer_address(f.value)==pointer,"shrink/in-capacity retains allocation and capacity");
            if(capacity>15)check(f.value.storage_00==prefix,"heap prefix unchanged in non-reallocation paths");
        }else{
            check(f.api.allocations.size()==1&&f.api.frees.size()==(capacity>15?1U:0U)&&f.value.capacity_18>=target,"growth allocates once and frees old heap only");
            const auto allocation=f.api.allocations[0];
            check(allocation==f.value.capacity_18+1+(f.value.capacity_18+1>=4096?39:0),"allocation includes NUL and only large alignment overhead");
            if(capacity>15)check(f.api.frees[0].second==capacity+1+(capacity+1>=4096?39:0),"sized free uses OLD capacity including large overhead");
            if(f.value.capacity_18+1>=4096)check((s::receive_buffer_address(f.value)&31)==0,"large data has32-byte alignment");
        }
    }
}
void capacities(){
    struct Case{std::uint64_t old_cap,old_size,target,new_cap;};
    const Case cases[]={{15,15,16,31},{16,16,17,31},{31,20,32,47},{31,31,48,63},{64,64,65,96},{4094,4094,4095,6141},{4095,4095,4096,6142},{15,0,4094,4095},{15,0,65536,65551}};
    for(const auto& c:cases){Fixture f;f.seed(c.old_cap,c.old_size);s::resize_receive_buffer_010040(f.value,c.target,0,f.context);check(f.value.capacity_18==c.new_cap,"explicit MSVC growth capacity anchors");}
    Fixture f;f.seed(15,3);const auto prefix=f.value.storage_00;
    auto& same=s::grow_receive_buffer_008350(f.value,17,0xdead,5,0xab,f.context);
    check(&same==&f.value&&f.value.size_10==20,"growth returns original object and ignores hint");
    auto b=f.memory.view(s::receive_buffer_address(f.value),21);
    check(std::equal(prefix.begin(),prefix.begin()+3,b.begin())&&std::all_of(b.begin()+3,b.begin()+8,[](auto x){return x==0xab;})&&b[8]==0&&b[20]==0xcc,"direct grow fill_count is independent of extra and NUL follows fill_count");
}
struct InjectedFailure:std::runtime_error {InjectedFailure():std::runtime_error("deliberate fixture stop") {}};
void failure_prefixes(){
    Fixture no_memory;no_memory.seed(15,7);const auto original=no_memory.value;
    no_memory.api.on_allocate=[](){throw std::bad_alloc();};rejects<std::bad_alloc>([&]{s::resize_receive_buffer_010040(no_memory.value,16,0,no_memory.context);});
    check(no_memory.value.size_10==original.size_10&&no_memory.value.capacity_18==original.capacity_18&&no_memory.value.storage_00==original.storage_00,"allocator failure precedes all metadata writes");
    Fixture small;small.seed(15,0);small.api.null_allocate=true;rejects<s::UnsupportedInput>([&]{s::resize_receive_buffer_010040(small.value,16,0,small.context);});
    check(small.value.size_10==16&&small.value.capacity_18==31&&small.api.frees.empty(),"small null reaches bounded memory failure after metadata; no invented invalid-parameter branch");
    Fixture large;large.seed(15,0);large.api.null_allocate=true;
    try{s::resize_receive_buffer_010040(large.value,4096,0,large.context);throw std::runtime_error("missing terminal");}catch(const Terminal& t){check(t.kind==3,"large null invokes invalid-parameter terminal");}
    check(large.value.size_10==0&&large.value.capacity_18==15,"large null precedes metadata stores");
    Fixture limit;limit.value.size_10=0x7fffffffffffffffULL;
    try{s::grow_receive_buffer_008350(limit.value,1,0,1,0,limit.context);throw std::runtime_error("missing length terminal");}catch(const Terminal& t){check(t.kind==1,"max_size overflow uses length-error boundary");}
    check(limit.api.allocations.empty(),"length check precedes allocation");
    for(std::uint64_t gap:{7ULL,8ULL,39ULL,40ULL}){
        Fixture f;f.seed(4095,4);const auto data=s::receive_buffer_address(f.value),real_base=word(std::span<const std::uint8_t>{f.memory.view(data-8,8)});
        word(f.memory.view(data-8,8),data-gap);
        if(gap==8||gap==39){
            // Validate header arithmetic independently of owned-free provenance.
            f.api.on_free=[&]{check(f.api.frees.back()==std::pair<std::uintptr_t,std::uint64_t>{data-gap,4135},"gap8..39 accepted and exact free size");throw InjectedFailure();};
            rejects<InjectedFailure>([&]{s::resize_receive_buffer_010040(f.value,5000,0,f.context);});
        }else{
            try{s::resize_receive_buffer_010040(f.value,5000,0,f.context);throw std::runtime_error("missing header terminal");}catch(const Terminal& t){check(t.kind==3,"header unsigned gap rejects below8 or above39");}
            check(f.api.frees.empty(),"bad header rejected before old free");
        }
        check(word(std::span<const std::uint8_t>{f.value.storage_00})==data&&f.value.size_10==5000&&f.value.capacity_18==6142&&real_base!=0,"failure prefix has new metadata and old pointer before final publication");
    }
}
struct Trace:s::ReceiveBufferApi {
    std::uintptr_t raw=0x20003;std::vector<std::uint64_t> allocated;std::vector<std::string> calls;
    std::function<void()> allocate,copy,fill,free;
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{calls.push_back("allocate");allocated.push_back(n);if(allocate)allocate();return raw;}
    void copy_bytes(std::uintptr_t,std::uintptr_t,std::uint64_t)override{calls.push_back("copy");if(copy)copy();}
    void fill_bytes(std::uintptr_t,std::uint8_t,std::uint64_t)override{calls.push_back("fill");if(fill)fill();}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override{calls.push_back("free");if(free)free();}
    [[noreturn]]void length_error_00ce80()override{throw Terminal(1);}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Terminal(2);}
    [[noreturn]]void invalid_parameter_05c974()override{throw Terminal(3);}
};
void symbolic_arithmetic_and_order(){
    // Sparse windows prove integer/control-flow contracts only, not huge memory IO.
    for(std::uint64_t old_cap:{15ULL,0x6000000000000000ULL}){
        s::ReceiveBufferString value;value.capacity_18=old_cap;word(value.storage_00,0x10020);
        std::array<std::uint8_t,8> new_header{},old_header{};word(old_header,0x10000);std::array<std::uint8_t,1> nul{{0xcc}};
        std::array<s::ReceiveBufferRegion,3> regions{{{0x20018,new_header},{0x10018,old_header},{0x20020,nul}}};s::ReceiveBufferMemory memory{regions};Trace api;s::ReceiveBufferContext context{memory,api};
        const auto extra=old_cap==15?0x7fffffffffffffffULL:1ULL;
        api.copy=[&]{check(value.size_10==extra&&value.capacity_18==0x7fffffffffffffffULL,"metadata updated before copy even in saturated arithmetic case");};
        s::grow_receive_buffer_008350(value,extra,0,0,0,context);
        check(api.allocated==std::vector<std::uint64_t>{0x8000000000000027ULL}&&word(std::span<const std::uint8_t>{new_header})==0x20003&&nul[0]==0,"max and saturated capacity request exact large bytes/header/NUL");
        check(s::receive_buffer_address(value)==0x20020&&api.calls==(old_cap==15?std::vector<std::string>{"allocate","copy","fill"}:std::vector<std::string>{"allocate","copy","fill","free"}),"normal ordering ends with old free before pointer publication");
    }
    Fixture f;f.seed(16,4);const auto pointer=s::receive_buffer_address(f.value);Trace api;api.raw=0x30000;
    std::array<std::uint8_t,32> data;data.fill(0xcc);f.regions[f.next_region++]={api.raw,data};s::ReceiveBufferContext context{f.memory,api};
    api.allocate=[&]{f.value.size_10=100;f.value.capacity_18=400;};
    api.copy=[&]{check(f.value.size_10==20&&f.value.capacity_18==31&&s::receive_buffer_address(f.value)==pointer,"entry size/capacity cached across allocator; old pointer still unpublished");};
    api.free=[&]{check(word(std::span<const std::uint8_t>{f.value.storage_00})==pointer&&data[20]==0,"new NUL precedes free and pointer store follows free");};
    s::grow_receive_buffer_008350(f.value,16,0,16,0,context);
    check(s::receive_buffer_address(f.value)==api.raw,"final pointer publication");
}
void mapping_contracts(){
    Fixture f;f.seed(15,7);f.regions[0]={};rejects<s::UnsupportedInput>([&]{s::resize_receive_buffer_010040(f.value,2,0,f.context);});check(f.value.size_10==2,"bounded missing mapping does not invent rollback");
    Fixture duplicate;duplicate.seed(15,7);duplicate.regions[1]=duplicate.regions[0];rejects<s::UnsupportedInput>([&]{s::resize_receive_buffer_010040(duplicate.value,2,0,duplicate.context);});
    Fixture zero;zero.api.copy_bytes(0,0,0);zero.api.fill_bytes(0,0,0);check(zero.api.allocations.empty(),"selected byte adapter accepts zero counts without dereferencing null");
}
}
int main(){try{payload_oracle();capacities();failure_prefixes();symbolic_arithmetic_and_order();mapping_contracts();std::cout<<"PASS: "<<checks<<" receive buffer core checks; actual010040/008350, independent byte payload oracle, explicit growth anchors, alignment/header/free, symbolic max arithmetic and exact failure prefixes; no original CRT body claim\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
