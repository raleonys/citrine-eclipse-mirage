#include "support.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace s=anadius::crt_support_recovered;
namespace {
unsigned checks;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("check "+std::to_string(checks));}
struct Event {char kind;s::Address value;bool operator==(const Event&)const=default;};
struct Fixture final:s::Effects {
    s::Head expected{0x123};s::Address first=0;
    std::map<s::Address,s::Address> nodes;
    std::vector<Event> events;
    std::function<void(s::Address)> on_free;
    void initialize_slist_head(s::Head head)override{check(head.token==expected.token);events.push_back({'i',head.token});first=0;}
    s::Address interlocked_flush_slist(s::Head head)override {
        check(head.token==expected.token);events.push_back({'f',head.token});auto result=first;first=0;return result;
    }
    s::Address next(s::Address node)override{
        check(nodes.contains(node));events.push_back({'n',node});return nodes.at(node);
    }
    void free_05caec(s::Address node)override{
        check(node!=0&&nodes.contains(node));events.push_back({'d',node});if(on_free)on_free(node);nodes.erase(node);
    }
};
void options_and_slot(){
    for(const auto initial:std::array<std::uint64_t,8>{0,1,2,0x24,0xffffffff,0x8000000000000000ULL,0x1122334455667788ULL,~std::uint64_t{0}}){
        s::State state{initial,~initial,0x12345678};
        check(s::printf_options_0067e0(state)==&state.printf_options_0a37d0);
        check(s::scanf_options_0067f0(state)==&state.scanf_options_0a37d8);
        const auto* slot=s::tls_callback_slot_04c218(state);
        check(slot==&state.tls_callback_0a5a70&&*slot==0x12345678);
        s::configure_stdio_04c1fc(state);
        check(state.printf_options_0a37d0==(initial|0x24));
        check(state.scanf_options_0a37d8==((~initial)|2));
        check(*slot==0x12345678);
        s::configure_stdio_04c1fc(state);
        check(state.printf_options_0a37d0==(initial|0x24)&&state.scanf_options_0a37d8==((~initial)|2));
        state.tls_callback_0a5a70=0x87654321;
        check(s::tls_callback_slot_04c218(state)==slot&&*slot==0x87654321);
    }
}
void snapshots(){
    for(unsigned count=0;count<=64;++count){
        Fixture f;f.first=count?0x1000:0;
        std::vector<Event> expected{{'f',f.expected.token}};
        for(unsigned i=0;i<count;++i){const s::Address node=0x1000+i*16;f.nodes[node]=(i+1<count)?node+16:0;expected.push_back({'n',node});expected.push_back({'d',node});}
        if(count%2==0)s::release_list_04d2f8(f.expected,f);else s::release_slist_04c1f0(f.expected,f);
        check(f.nodes.empty()&&f.first==0);check(f.events==expected);
    }
    Fixture f;f.first=0x10;f.nodes={{0x10,0x20},{0x20,0}};
    f.on_free=[&](auto node){
        if(node==0x10){f.nodes[node]=0x999;f.nodes[0x30]=0;f.first=0x30;}
    };
    s::release_slist_04c1f0(f.expected,f);
    check(f.first==0x30&&f.nodes.size()==1&&f.nodes.contains(0x30));
    check(f.events==std::vector<Event>{{'f',0x123},{'n',0x10},{'d',0x10},{'n',0x20},{'d',0x20}});
    f.on_free={};s::release_slist_04c1f0(f.expected,f);check(f.nodes.empty());
}
void normal_flow_and_init(){
    Fixture f;f.first=0x10;f.nodes={{0x10,0x20},{0x20,0}};
    f.on_free=[](auto){throw std::runtime_error("provider failure");};
    bool propagated=false;
    try{s::release_list_04d2f8(f.expected,f);}catch(const std::runtime_error&){propagated=true;}
    check(propagated);check(f.events==std::vector<Event>{{'f',0x123},{'n',0x10},{'d',0x10}});
    check(f.nodes.size()==2); // no invented unwind cleanup or second flush
    Fixture init;init.first=0x20;s::initialize_slist_04c1e0(init.expected,init);
    check(init.first==0&&init.events==std::vector<Event>{{'i',0x123}});
}
}
int main(){try{options_and_slot();snapshots();normal_flow_and_init();std::cout<<"PASS "<<checks<<" CRT support checks\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
