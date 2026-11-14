// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_service_registry.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <vector>

namespace s=laureondo::socket;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Gate final:s::SocketGate {
    unsigned enters=0,leaves=0;int depth=0;std::vector<std::string> trace;
    std::function<void(bool)> hook;
    void enter()override{check(!depth,"normal registry gate enters from unlocked state");++depth;++enters;trace.push_back("enter");if(hook)hook(true);}
    void leave()override{check(depth==1,"normal registry gate leaves while held");++leaves;trace.push_back("leave");if(hook)hook(false);--depth;}
};
struct Entry final:s::ServiceEntry {
    unsigned deletes=0;std::function<void(std::uint32_t)> hook;
    explicit Entry(std::uintptr_t type=0,std::uintptr_t id=0){key={type,id};}
    void scalar_delete(std::uint32_t flags)override{++deletes;if(hook)hook(flags);}
};
struct Effects final:s::ServiceRegistryEffects {
    Gate& gate;Entry* fresh=nullptr;unsigned comparisons=0,creates=0;
    std::function<std::uint32_t(std::uintptr_t,std::uintptr_t)> compare;
    std::function<s::ServiceEntry*(s::ContextIdentity)> factory;
    explicit Effects(Gate& gate):gate(gate){}
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{
        check(gate.depth==1,"023a4d/023ac0 comparison runs inside registry gate");++comparisons;gate.trace.push_back("compare");
        check(right==0x1800a2628ULL,"fixed RTTI descriptor is passed at plus8");
        return compare?compare(left,right):1;
    }
    s::ServiceEntry* create_socket_service_0228e0(s::ContextIdentity context)override{
        check(!gate.depth,"023a66 leave precedes native factory call");++creates;gate.trace.push_back("create");
        check(context.value==0xfedcba9876543210ULL,"factory receives exact full-width original context");return factory?factory(context):fresh;
    }
};
constexpr s::ContextIdentity context{0xfedcba9876543210ULL};
void initializer(){
    struct Layout{std::uint64_t before=17;s::ServiceKey key{~std::uintptr_t{0},~std::uintptr_t{0}};std::uint64_t after=29;} value;
    s::initialize_service_key_022fa0(value.key);
    check(value.key.type==0x1800a2620ULL && !value.key.id,"022fb6/022fb9 overwrite both fixed key fields");
    check(value.before==17 && value.after==29,"fixed initializer writes only selected key");
}
void first_hits(){
    for(unsigned position=0;position<9;++position){
        Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);std::array<Entry,9> entries;
        for(unsigned i=0;i<entries.size();++i){entries[i].key={0x100000000ULL+0x100*i,~std::uintptr_t{0}};entries[i].next=i+1<entries.size()?&entries[i+1]:nullptr;}
        registry.head=&entries[0];effects.compare=[&](std::uintptr_t left,std::uintptr_t){check(left==entries[effects.comparisons-1].key.type+8,"first scan compares entry type plus8 in list order");return effects.comparisons==position+1?0U:0xffffffffU;};
        check(&s::resolve_service_0239c0(registry,context,effects)==&entries[position],"023b20 returns first matching live entry");
        check(effects.comparisons==position+1 && !effects.creates && gate.enters==1 && gate.leaves==1 && !gate.depth,"first hit stops before later entries and never constructs");
        for(const auto& entry:entries)check(!entry.deletes,"first hit deletes no borrowed registry entry");
    }
    // Return value is cached across the native leave import.
    Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry chosen(7),replacement(8);registry.head=&chosen;effects.compare=[](auto,auto){return 0U;};
    gate.hook=[&](bool enter){if(!enter)registry.head=&replacement;};
    check(&s::resolve_service_0239c0(registry,context,effects)==&chosen && registry.head==&replacement,"first leave mutation does not replace cached winner");
}
void predicates_and_publish(){
    for(std::uint32_t unequal:{1U,2U,0x80000000U,0xffffffffU}){
        Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry null_type(0,0x1800a2620ULL),wrong(0x100,1),fresh(0xdead,0xbeef),old_next;
        null_type.next=&wrong;registry.head=&null_type;fresh.next=&old_next;effects.fresh=&fresh;
        effects.compare=[&](std::uintptr_t left,std::uintptr_t){check(left==0x108,"null type skipped and nonzero ID never matches fixed ID0");return unequal;};
        gate.hook=[&](bool enter){if(enter && gate.enters==2)check(fresh.key.type==0x1800a2620ULL && !fresh.key.id,"023a81 copies complete key before second gate entry");};
        check(&s::resolve_service_0239c0(registry,context,effects)==&fresh,"every nonzero EAX result is mismatch");
        check(registry.head==&fresh && fresh.next==&null_type && null_type.next==&wrong,"023ad2 publication overwrites candidate next and prepends existing list");
        check(effects.comparisons==2 && effects.creates==1 && !fresh.deletes && !null_type.deletes && !wrong.deletes,"two passes skip null type and leave borrowed entries live");
        check(gate.trace==std::vector<std::string>{"enter","compare","leave","create","enter","compare","leave"},"miss path native effect order");
    }
    Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry fresh;effects.fresh=&fresh;
    check(&s::resolve_service_0239c0(registry,context,effects)==&fresh && registry.head==&fresh && !fresh.next,"empty list publishes fresh candidate with null next");
    check(!effects.comparisons && effects.creates==1 && !gate.depth,"empty scans invoke no RTTI comparison");
    // Identity equality alone is not the native matching predicate: EAX still
    // controls the result even for exactly equal opaque descriptor addresses.
    for(std::uintptr_t type:{std::uintptr_t{0x1800a2620ULL},~std::uintptr_t{0}}){
        Gate g;s::ServiceRegistry r(g);Effects e(g);Entry previous(type),candidate;e.fresh=&candidate;r.head=&previous;
        e.compare=[&](std::uintptr_t left,std::uintptr_t){check(left==type+8,"entry name adjustment uses full-width unsigned identity");return 2U;};
        check(&s::resolve_service_0239c0(r,context,e)==&candidate && e.comparisons==2 && e.creates==1,"same descriptor identity still calls required comparator and accepts only EAX zero");
    }
}
void live_scans(){
    for(bool second_pass:{false,true}){
        Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry first(0x100),stale(0x200),live(0x300),fresh;first.next=&stale;
        registry.head=second_pass?nullptr:&first;effects.fresh=&fresh;
        effects.factory=[&](s::ContextIdentity){registry.head=&first;return &fresh;};
        effects.compare=[&](std::uintptr_t left,std::uintptr_t){
            if(left==0x108){first.next=&live;return 1U;}
            check(left==0x308,"scan reloads live next after comparison instead of stale next");return 0U;
        };
        fresh.hook=[&](std::uint32_t flag){check(flag==1 && gate.depth==1,"second scan race deletes fresh entry with flag1 while gate held");};
        check(&s::resolve_service_0239c0(registry,context,effects)==&live,"both scans follow comparator-mutated next");
        check(effects.comparisons==2 && effects.creates==static_cast<unsigned>(second_pass) && fresh.deletes==static_cast<unsigned>(second_pass),"live next mutation changes exact scan count");
    }
    // First gate entry must be followed by a live registry head read.
    Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry winner(0x300);
    gate.hook=[&](bool enter){if(enter)registry.head=&winner;};effects.compare=[](auto,auto){return 0U;};
    check(&s::resolve_service_0239c0(registry,context,effects)==&winner && !effects.creates,"first scan sees gate-enter head mutation");
}
void duplicate_races(){
    for(unsigned insertion=0;insertion<3;++insertion){
        Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry candidate(99,88),winner(0x222,999),after(0x333),last(0x444);
        effects.fresh=&candidate;effects.factory=[&](s::ContextIdentity){check(candidate.key.type==99 && candidate.key.id==88,"factory sees own key before post-factory overwrite");if(insertion==0)registry.head=&winner;return &candidate;};
        gate.hook=[&](bool enter){if(!enter && gate.leaves==1 && insertion==1)registry.head=&winner;if(enter && gate.enters==2){check(candidate.key.type==0x1800a2620ULL && !candidate.key.id,"race second enter observes copied key");if(insertion==2)registry.head=&winner;}if(!enter && gate.leaves==2){check(candidate.deletes==1,"duplicate deletion precedes second gate leave");registry.head=&last;}};
        effects.compare=[&](std::uintptr_t left,std::uintptr_t){check(left==0x22a,"second scan starts with live inserted winner");return 0U;};
        candidate.hook=[&](std::uint32_t flags){check(flags==1 && gate.depth==1 && gate.leaves==1,"023b13 scalar destructor uses1 under second gate");registry.head=&after;};
        check(&s::resolve_service_0239c0(registry,context,effects)==&winner,"cached race winner survives destructor and gate-leave head mutations");
        check(registry.head==&last && candidate.deletes==1 && !winner.deletes && effects.creates==1 && effects.comparisons==1 && !gate.depth,"duplicate candidate only deleted once and not published");
        check(gate.trace==std::vector<std::string>{"enter","leave","create","enter","compare","leave"},"race normal native call order");
    }
}
void live_publication(){
    Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);Entry scanned(0x123),live_head(0,123),candidate(42,44),stale_next;
    effects.fresh=&candidate;candidate.next=&stale_next;
    effects.factory=[&](s::ContextIdentity){registry.head=&scanned;return &candidate;};
    effects.compare=[&](std::uintptr_t left,std::uintptr_t){check(left==0x12b,"publication fixture compares selected node");registry.head=&live_head;return 1U;};
    gate.hook=[&](bool enter){if(!enter && gate.leaves==2)check(registry.head==&candidate && candidate.next==&live_head,"publication writes both links before leave import");};
    check(&s::resolve_service_0239c0(registry,context,effects)==&candidate && candidate.next==&live_head,"023ad2 reloads registry head after second-miss comparison side effect");
    check(!candidate.deletes && effects.comparisons==1 && effects.creates==1,"second miss publishes without deleting new candidate");
}
void host_guard(){
    Gate gate;s::ServiceRegistry registry(gate);Effects effects(gate);bool caught=false;
    try{s::resolve_service_0239c0(registry,context,effects);}catch(const s::UnsupportedInput&){caught=true;}
    check(caught && gate.enters==1 && gate.leaves==1 && !gate.depth && !registry.head,"explicit host null-factory guard stops before second enter or publication");
}
}
int main(){try{initializer();first_hits();predicates_and_publish();live_scans();duplicate_races();live_publication();host_guard();std::cout<<"PASS: "<<checks<<" socket service registry checks; retained normal-flow predicates and controlled callback mutations only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
