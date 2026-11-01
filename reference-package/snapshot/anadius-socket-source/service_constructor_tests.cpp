#include "service_constructor.hpp"
#include "recycle_storage.hpp"
#include <bit>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace s=anadius::socket_recovered;
namespace srv=anadius::server_recovered;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected constructor test dependency");}
struct Gate final:s::SocketGate {int depth=0;unsigned enters=0,leaves=0;std::function<void(bool)> hook;void enter()override{check(!depth,"gate acquired unlocked");++depth;++enters;if(hook)hook(true);}void leave()override{check(depth==1,"gate released held");if(hook)hook(false);--depth;++leaves;}};
struct Entry final:s::SocketServiceEntry {using SocketServiceEntry::SocketServiceEntry;unsigned deletes=0;void scalar_delete(std::uint32_t flags)override{check(flags==1,"duplicate scalar delete flag one");++deletes;}};
struct GateApi final:s::ServiceGateApi {
    std::vector<std::string>& trace;CRITICAL_SECTION& expected;std::int32_t result=1;std::uint32_t last=17;unsigned initializes=0,gets=0;std::function<void()> hook;
    GateApi(std::vector<std::string>& t,CRITICAL_SECTION& e):trace(t),expected(e){}
    std::int32_t initialize(CRITICAL_SECTION& section,std::uint32_t spin)override{check(&section==&expected && spin==0x80000000U,"027be4 exact borrowed SDK section and DWORD spin");trace.push_back("initialize");++initializes;if(hook)hook();return result;}
    std::uint32_t get_last_error()override{trace.push_back("last");++gets;return last;}
};
struct Errors final:s::Api {
    unsigned categories=0,sends=0;std::int32_t last=0;std::vector<std::string>* trace=nullptr;std::function<void()> hook;s::Socket wake=0xfedcba9876543210ULL;
    void set_last_error(std::int32_t value)override{last=value;}
    s::Category system_category_028620()override{++categories;if(trace)trace->push_back("category");if(hook)hook();return {0x9922,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{return last;}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffers,std::uint32_t flags)override{check(socket==wake && buffers.size()==1 && buffers[0].len==1 && buffers[0].buf[0]==0 && !flags,"real wake uses controlled NUL buffer");++sends;return {0,1};}
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Raised {s::Error error;std::string context;};
struct Construction final:s::ServiceConstructionEffects {
    std::vector<std::string>& trace;Entry* entry;unsigned allocations=0,resolves=0,raises=0;std::function<void()> allocate_hook;std::function<std::uintptr_t(std::uintptr_t)> resolve_hook;
    Construction(std::vector<std::string>& t,Entry* e):trace(t),entry(e){}
    s::SocketServiceEntry* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==0x80,"0228f2 request exactly 128 native bytes");++allocations;trace.push_back("allocate");if(allocate_hook)allocate_hook();return entry;}
    std::uintptr_t resolve_scheduler_023b30(std::uintptr_t identity)override{++resolves;trace.push_back("scheduler");return resolve_hook?resolve_hook(identity):identity;}
    [[noreturn]]void raise_028240(s::Error error,std::string_view context)override{++raises;trace.push_back("raise");throw Raised{error,std::string(context)};}
};
struct RegistryEffects final:s::ConstructorRegistryEffects {
    using ConstructorRegistryEffects::ConstructorRegistryEffects;
    unsigned comparisons=0;
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{check(left==0x1800a2628ULL && right==left,"real registry RTTI tokens");++comparisons;return 0;}
};
void helper(){
    CRITICAL_SECTION section{};std::vector<std::string> trace;GateApi api(trace,section);
    for(std::int32_t result:{1,2,-1,std::numeric_limits<std::int32_t>::min()}){api.result=result;api.last=0xffffffffU;trace.clear();const auto before=api.gets;check(s::initialize_gate_027be0(section,api)==0 && api.gets==before,"any nonzero BOOL succeeds without GetLastError");check(trace==std::vector<std::string>{"initialize"},"success helper order");}
    for(std::uint32_t last:{0U,1U,0x80000000U,0xffffffffU}){api.result=0;api.last=last;trace.clear();check(s::initialize_gate_027be0(section,api)==last,"zero BOOL returns exact GetLastError including zero");check(trace==std::vector<std::string>{"initialize","last"},"failure helper order");}
}
void null_allocation(){
    Gate gate;CRITICAL_SECTION section{};std::vector<std::string> trace;GateApi api(trace,section);Errors errors;errors.trace=&trace;Construction effects(trace,nullptr);s::SocketConstructionContext context{{7},9};
    check(!s::construct_service_0228e0(context,effects,api,errors),"null allocation returns null");
    check(effects.allocations==1 && !effects.resolves && !effects.raises && !api.initializes && !api.gets && !errors.categories && trace==std::vector<std::string>{"allocate"},"null stops before all header/scheduler/gate/category work");
}
void contracts(){
    // Evidence0228ef caches context before04b0bc;022929 reloads registry qword.
    for(std::int32_t initialized:{0,1,-1,2})for(std::uint32_t last:{0U,1U,0x7fffffffU,0x80000000U,0xffffffffU}){
        Gate gate;CRITICAL_SECTION section{};Entry entry(gate,section),old_next(gate,section);s::AcceptorState state;s::SocketObject old_head(state),mutated_head(state);
        entry.key={11,12};entry.next=&old_next;entry.owner_context={13};entry.implementation_context={14};entry.table_identity=15;entry.scheduler_identity=16;entry.word_38=17;entry.word_40=18;entry.word_48=19;entry.service.head=&old_head;entry.service.completion_port=0xfedcba9876543210ULL;
        s::SocketConstructionContext context{{0x123456789abcdef0ULL},0x1111222233334444ULL};const auto original=context.identity;
        std::vector<std::string> trace;GateApi api(trace,section);api.result=initialized;api.last=last;Errors errors;errors.trace=&trace;Construction effects(trace,&entry);
        effects.allocate_hook=[&]{context.identity={33};context.registry_identity=0xaabbccddeeff0099ULL;check(entry.key.type==11 && entry.table_identity==15,"allocator sees old fields before header writes");};
        effects.resolve_hook=[&](std::uintptr_t registry){check(registry==0xaabbccddeeff0099ULL,"scheduler sees live registry identity after allocation");check(!entry.key.type && !entry.key.id && !entry.next && entry.owner_context.value==original.value && entry.implementation_context.value==original.value && entry.table_identity==0x180084590ULL,"all base header stores precede scheduler callback and context is cached");check(entry.scheduler_identity==16 && entry.word_38==17 && entry.word_40==18 && entry.word_48==19,"scheduler callback precedes scheduler assignment and zeroing");entry.scheduler_identity=55;entry.word_38=66;entry.word_40=77;entry.word_48=88;entry.key.id=99;entry.owner_context={100};return 0xf0e0d0c0b0a09080ULL;};
        api.hook=[&]{check(entry.scheduler_identity==0xf0e0d0c0b0a09080ULL && !entry.word_38 && !entry.word_40 && !entry.word_48,"scheduler return assigned and three zeros precede gate init");check(entry.service.head==&old_head && entry.table_identity==0x180084590ULL,"head/final table stores delayed until after gate/category success");entry.word_38=111;entry.word_40=112;entry.word_48=113;};
        errors.hook=[&]{check(api.initializes==1 && api.gets==static_cast<unsigned>(!initialized),"category always follows completed helper including success");api.last=0x1234;entry.service.head=&mutated_head;entry.table_identity=0x12345;entry.implementation_context={202};entry.scheduler_identity=203;};
        const auto expected=initialized?0U:last;bool raised=false;try{check(s::construct_service_0228e0(context,effects,api,errors)==&entry,"successful constructor returns exact allocation");}catch(const Raised& failure){raised=true;check(std::bit_cast<std::uint32_t>(failure.error.value)==expected && failure.error.category.name_identity==0x9922 && failure.error.category.logical==srv::ErrorCategory::system && failure.context=="mutex","raise forwards cached DWORD bit pattern and exact category/context");}
        check(raised==(expected!=0) && effects.raises==static_cast<unsigned>(expected!=0) && errors.categories==1,"only nonzero cached helper status raises after exactly one category call");
        std::vector<std::string> expected_trace{"allocate","scheduler","initialize"};if(!initialized)expected_trace.push_back("last");expected_trace.push_back("category");if(expected)expected_trace.push_back("raise");check(trace==expected_trace,"complete native normal effect order");
        check(entry.service.head==(expected?&mutated_head:nullptr) && entry.table_identity==(expected?0x12345ULL:0x1800845b0ULL),"failure preserves callback partial state; success clears head then installs derived table");
        check(entry.key.id==99 && entry.owner_context.value==100 && entry.implementation_context.value==202 && entry.scheduler_identity==203 && entry.word_38==111 && entry.word_40==112 && entry.word_48==113 && entry.service.completion_port==0xfedcba9876543210ULL,"constructor never replays overwritten fields or changes unrelated completion port");
        check(!entry.deletes && !gate.enters && !gate.leaves,"constructor invents no deletion rollback or service gate acquisition");
    }
}
void registry_adapter(){
    Gate gate;CRITICAL_SECTION section{};Entry entry(gate,section),winner(gate,section);std::vector<std::string> trace;GateApi api(trace,section);Errors errors;s::SocketConstructionContext context{{42},77};Construction construction(trace,&entry);RegistryEffects effects(context,construction,api,errors);
    bool caught=false;try{effects.create_socket_service_0228e0({43});}catch(const s::UnsupportedInput&){caught=true;}check(caught && trace.empty(),"bound context guard precedes allocation/imports");
    s::ServiceRegistry registry(gate);construction.resolve_hook=[&](auto identity){check(identity==77 && !gate.depth,"constructor scheduler outside registry gate");winner.key={0x1800a2620ULL,0};registry.head=&winner;return 88;};
    check(&s::resolve_service_0239c0(registry,{42},effects)==&winner && entry.deletes==1 && !winner.deletes,"actual registry second pass deletes fully constructed duplicate and returns cached winner");
    check(entry.table_identity==0x1800845b0ULL && entry.key.type==0x1800a2620ULL && !entry.key.id && registry.head==&winner && gate.enters==2 && gate.leaves==2,"duplicate candidate completes constructor before key copy and scalar deletion");
}
// Full chain fixture follows below; every OS/allocator/exception effect is controlled.
}
int main(){try{helper();null_allocation();contracts();registry_adapter();std::cout<<"PASS: "<<checks<<" socket service constructor checks; retained0228e0/027be0 normal flow and controlled effects only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}

