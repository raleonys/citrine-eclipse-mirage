// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_scheduler_registry.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <vector>
namespace s=preloader_l::socket;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Gate final:s::SocketGate {
    int depth=0;unsigned enters=0,leaves=0;std::function<void(bool)> hook;
    void enter()override{check(!depth,"registry enter from unlocked state");++depth;++enters;if(hook)hook(true);}
    void leave()override{check(depth==1,"registry leave held");if(hook)hook(false);--depth;++leaves;}
};
struct Entry final:s::ServiceEntry {unsigned deletes=0;std::function<void()> hook;void scalar_delete(std::uint32_t flags)override{check(flags==1,"duplicate scalar-delete flag one");++deletes;if(hook)hook();}};
struct Effects final:s::SchedulerRegistryEffects {
    Gate& gate;Entry* allocation;Entry* result;unsigned allocates=0,constructs=0,compares=0;
    std::function<void()> allocate_hook,construct_hook;std::function<std::uint32_t(std::uintptr_t,std::uintptr_t)> compare_hook;std::uintptr_t owner=0;
    Effects(Gate& gate,Entry* allocation,Entry* result):gate(gate),allocation(allocation),result(result){}
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{check(gate.depth==1 && right==0x1800a2548ULL,"RTTI compare under gate with scheduler descriptor plus eight");++compares;return compare_hook?compare_hook(left,right):0;}
    s::ServiceEntry* allocate_04b0bc(std::uint32_t bytes)override{check(!gate.depth && bytes==0xb0,"allocate exact176 outside gate");++allocates;if(allocate_hook)allocate_hook();return allocation;}
    s::ServiceEntry* construct_scheduler_024990(s::ServiceEntry& allocated,std::uintptr_t context,std::uint32_t hint,std::uint8_t own)override{check(!gate.depth && &allocated==allocation && context==owner && hint==0xffffffffU && own==1,"constructor exact allocation cached-owner concurrency and own-thread arguments");++constructs;if(construct_hook)construct_hook();return result;}
};
void key(){for(auto type:{std::uintptr_t{0},std::uintptr_t{1},std::numeric_limits<std::uintptr_t>::max()}){s::ServiceKey key{type,type};s::initialize_scheduler_key_022fd0(key);check(key.type==0x1800a2540ULL && !key.id,"fixed key overwrites both words");}}
void first_scan(){
    for(auto id:{std::uintptr_t{0},std::uintptr_t{1},std::numeric_limits<std::uintptr_t>::max()})for(auto result:{0U,1U,0x80000000U,0xffffffffU}){
        Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry null_type,first,last,fresh;
        null_type.key={0,id};null_type.next=&first;first.key={0x1800a2540ULL,id};first.next=&last;last.key={0x5550,id};registry.head=&null_type;
        Effects effects(gate,&fresh,&fresh);unsigned calls=0;effects.compare_hook=[&](auto left,auto){++calls;check(left==(calls%2==1?0x1800a2548ULL:0x5558ULL),"entry descriptor plus eight and finite traversal");return calls%2==1?result:0U;};
        gate.hook=[&](bool enter){if(!enter)registry.head=nullptr;};
        auto& found=s::resolve_scheduler_023b30(state,effects);
        check(&found==(result?&last:&first) && effects.compares==(result?2U:1U),"zero-only comparison matches even same addresses; key IDzero disables ID shortcut");
        check(!effects.allocates && !effects.constructs && !fresh.deletes && gate.enters==1 && gate.leaves==1 && !gate.depth,"first hit leaves and returns cached winner without allocation");
    }
    Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry first,skip,winner,fresh;
    first.key.type=99;skip.key.type=101;winner.key.type=103;first.next=&skip;registry.head=&first;Effects effects(gate,&fresh,&fresh);
    effects.compare_hook=[&](auto left,auto){if(left==107){first.next=&winner;return 1U;}check(left==111,"scan reloads next after comparison callback");return 0U;};
    check(&s::resolve_scheduler_023b30(state,effects)==&winner && effects.compares==2,"first scan follows live next link");
    registry.head=&skip;skip.key.type=0;winner.key.type=103;gate.hook=[&](bool enter){if(enter)registry.head=&winner;};effects.compare_hook=[&](auto left,auto){check(left==111,"head is loaded after first enter callback");return 0U;};
    check(&s::resolve_scheduler_023b30(state,effects)==&winner && !effects.allocates,"first scan observes live head changed by gate callback");
}
void misses(){
    for(unsigned mutation=0;mutation<4;++mutation){
        Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);state.owner_context=0x123456789abcdef0ULL;
        Entry allocation,fresh,old,live;allocation.key={11,12};fresh.key={13,14};fresh.next=&old;old.key.type=0;Effects effects(gate,&allocation,&fresh);effects.owner=state.owner_context;
        gate.hook=[&](bool enter){if(mutation==0 && enter && gate.enters==1)state.owner_context=88;if(mutation==1 && !enter && gate.leaves==0)state.owner_context=89;if(enter && gate.enters==2){check(fresh.key.type==0x1800a2540ULL && !fresh.key.id && fresh.next==&old,"candidate key copied before second enter without invented next initialization");registry.head=&old;}if(!enter && gate.leaves==1){check(registry.head==&fresh,"candidate publication before leave");registry.head=&live;}};
        effects.allocate_hook=[&]{check(fresh.key.type==13,"key unchanged before construction");if(mutation==2)state.owner_context=90;};
        effects.construct_hook=[&]{if(mutation==3)state.owner_context=91;check(fresh.key.type==13 && allocation.key.type==11,"constructor return projection distinct from allocation");};
        check(&s::resolve_scheduler_023b30(state,effects)==&fresh && registry.head==&live && fresh.next==&old,"second miss uses constructor return and cached candidate across leave");
        check(allocation.key.type==11 && allocation.key.id==12 && !allocation.deletes && !fresh.deletes && effects.allocates==1 && effects.constructs==1 && gate.enters==2 && gate.leaves==2 && !gate.depth,"normal publication preserves raw allocation and balances two gates");
    }
    for(bool null_allocation:{false,true}){
        Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry allocation;Effects effects(gate,null_allocation?nullptr:&allocation,nullptr);bool caught=false;
        try{s::resolve_scheduler_023b30(state,effects);}catch(const s::UnsupportedInput&){caught=true;}
        check(caught && effects.allocates==1 && effects.constructs==static_cast<unsigned>(!null_allocation) && gate.enters==1 && gate.leaves==1 && !gate.depth && !registry.head && !allocation.deletes,"null result explicit unsupported before key write/second enter, no invented cleanup");
    }
}
void second_scan(){
    for(auto result:{0U,1U,0x80000000U,0xffffffffU}){
        Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry allocation,fresh,first,skip,winner,new_head;
        Effects effects(gate,&allocation,&fresh);first.key={0x1800a2540ULL,0};skip.key.type=77;winner.key.type=88;first.next=&skip;
        effects.construct_hook=[&]{registry.head=&first;};
        effects.compare_hook=[&](auto left,auto){if(left==0x1800a2548ULL){first.next=&winner;return result;}check(left==96,"second scan uses next reloaded after compare");return 0U;};
        auto* expected=result?&winner:&first;
        fresh.hook=[&]{check(gate.depth==1 && fresh.key.type==0x1800a2540ULL && !fresh.key.id,"duplicate deletion under gate after key copy");registry.head=&new_head;expected->next=nullptr;};
        gate.hook=[&](bool enter){if(!enter && gate.leaves==1)registry.head=nullptr;};
        check(&s::resolve_scheduler_023b30(state,effects)==expected && fresh.deletes==1 && !allocation.deletes && !expected->deletes && !registry.head,"cached second winner survives delete and leave mutations");
        check(effects.compares==(result?2U:1U) && gate.enters==2 && gate.leaves==2 && !gate.depth,"second scan exact callback and gate counts");
    }
    Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry allocation,fresh,first,new_head;Effects effects(gate,&allocation,&fresh);first.key.type=12;
    effects.construct_hook=[&]{registry.head=&first;};effects.compare_hook=[&](auto,auto){registry.head=&new_head;return 1U;};
    check(&s::resolve_scheduler_023b30(state,effects)==&fresh && fresh.next==&new_head && registry.head==&fresh && !fresh.deletes,"second miss publication rereads registry head after live comparator mutation");
    Gate other_gate;s::ServiceRegistry other_registry(other_gate);s::SchedulerRegistry other_state(other_registry);Entry other_allocation,other_fresh;Effects other_effects(other_gate,&other_allocation,&other_fresh);
    other_gate.hook=[&](bool enter){if(enter && other_gate.enters==2){check(other_fresh.key.type==0x1800a2540ULL && !other_fresh.key.id,"key copied before second gate effect");other_fresh.key={0x1234,0x5678};}};
    check(&s::resolve_scheduler_023b30(other_state,other_effects)==&other_fresh && other_fresh.key.type==0x1234 && other_fresh.key.id==0x5678,"publication never replays key stores after callback-visible second enter mutation");
}
struct Bridge final:s::SchedulerConstructionEffects {
    using SchedulerConstructionEffects::SchedulerConstructionEffects;
    s::SocketServiceEntry* allocate_04b0bc(std::uint32_t)override{throw std::logic_error("unused socket allocation");}
    [[noreturn]]void raise_028240(s::Error,std::string_view)override{throw std::logic_error("unused raise");}
};
void bridge(){
    Gate gate;s::ServiceRegistry registry(gate);s::SchedulerRegistry state(registry);Entry allocation,fresh;Effects effects(gate,&allocation,&fresh);
    std::array<s::SchedulerRegistryBinding,1> registries{{{0xfedcba9876543210ULL,&state}}};std::array<s::SchedulerServiceBinding,1> services{{{&fresh,0x8877665544332211ULL}}};Bridge bridge(registries,effects,services);
    bool caught=false;try{bridge.resolve_scheduler_023b30(22);}catch(const s::UnsupportedInput&){caught=true;}check(caught && !gate.enters && !effects.allocates,"unknown registry identity fails before any effects");
    registries[0].registry=nullptr;caught=false;try{bridge.resolve_scheduler_023b30(registries[0].registry_identity);}catch(const s::UnsupportedInput&){caught=true;}check(caught && !gate.enters,"null registry binding fails without dereference");registries[0].registry=&state;
    services[0].entry=nullptr;caught=false;try{bridge.resolve_scheduler_023b30(registries[0].registry_identity);}catch(const s::UnsupportedInput&){caught=true;}
    check(caught && registry.head==&fresh && effects.allocates==1 && effects.constructs==1 && !fresh.deletes && !gate.depth,"missing identity projection preserves already published borrowed scheduler");
    services[0]={&fresh,0x8877665544332211ULL};check(bridge.resolve_scheduler_023b30(registries[0].registry_identity)==services[0].scheduler_identity && effects.allocates==1,"bound cached entry projects exact full-width identity");
    services[0].scheduler_identity=0;check(bridge.resolve_scheduler_023b30(registries[0].registry_identity)==0 && !fresh.deletes,"zero identity is a caller projection token and entries stay borrowed");
}
}
int main(){try{key();first_scan();misses();second_scan();bridge();std::cout<<"PASS: "<<checks<<" socket scheduler registry checks; retained022fd0/023b30 normal flow with controlled allocation/024990/RTTI only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
