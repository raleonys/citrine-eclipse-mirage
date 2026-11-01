#include "errors.hpp"
#include "../anadius-thread-source/threads.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace er=anadius::crt_errors_recovered;
namespace th=anadius::threads_recovered;
namespace {
unsigned checks;
void require(bool b,const char* why) { ++checks; if(!b) throw std::runtime_error(why); }
struct Injected {};
std::uint32_t oracle(std::uint32_t n) {
    // Independent value-group specification of the retained binary pairs.
    switch(n) {
    case 2:case 3:case 15:case 18:case 53:case 67:case 161:case 206:return 2;
    case 4:return 24;
    case 5:case 16:case 33:case 65:case 82:case 83:case 108:case 132:case 158:case 167:return 13;
    case 6:case 114:case 130:return 9;
    case 7:case 8:case 9:case 1816:return 12;
    case 10:return 7;case 11:return 8;case 17:return 18;
    case 80:case 183:return 17;case 89:case 164:case 215:return 11;
    case 109:return 32;case 112:return 28;case 128:case 129:return 10;
    case 145:return 41;case 1113:return 42;
    default:break;
    }
    if(n>=19 && n<=36)return 13;
    if(n>=188 && n<=202)return 8;
    return 22;
}
struct Ptd { std::array<std::byte,0x29> bytes{}; void* base(){return bytes.data()+1;} };
er::Cell32 cell(Ptd& p,unsigned offset) {return er::Cell32(static_cast<std::byte*>(p.base())+offset);}
struct Lookup : er::Effects {
    std::vector<void*> results;
    unsigned calls{},throw_on{};
    std::function<void(unsigned)> before;
    void* optional_ptd_064d08() override {
        ++calls; if(before)before(calls);if(calls==throw_on)throw Injected{};
        if(calls>results.size())throw std::runtime_error("unexpected PTD lookup");
        return results[calls-1];
    }
};
void test_table() {
    for(std::uint32_t n=0;n<4096;++n)
        require(er::errno_from_os_error_05d650(n)==oracle(n),"table/range/default across low domain");
    constexpr std::array<std::uint32_t,11> high{0x7fffffff,0x80000000,0x80000001,
        0xffffffed,0xfffffff0,0xfffffff3,0xfffffffa,0xfffffffe,0xffffffff,65535,0x10000};
    for(auto n:high)require(er::errno_from_os_error_05d650(n)==22,"high unsigned codes do not enter wrapped ranges");
    require(er::errno_from_os_error_05d650(18)==2 && er::errno_from_os_error_05d650(19)==13
        && er::errno_from_os_error_05d650(36)==13 && er::errno_from_os_error_05d650(37)==22,"first range endpoints");
    require(er::errno_from_os_error_05d650(187)==22 && er::errno_from_os_error_05d650(188)==8
        && er::errno_from_os_error_05d650(202)==8 && er::errno_from_os_error_05d650(203)==22,"second range endpoints");
}
void test_cells() {
    er::State s;require(s.errno_fallback==12 && s.dos_fallback==8,"image fallback initializers");
    Lookup e;e.results={nullptr,nullptr};
    auto dos=er::dos_error_cell_05d704(s,e),crt=er::errno_cell_05d724(s,e);
    require(dos.raw()==&s.dos_fallback && crt.raw()==&s.errno_fallback,"distinct globals on null PTD");
    dos.store(0xffffffff);crt.store(0x80000000);
    require(dos.load()==0xffffffff && crt.load()==0x80000000 && e.calls==2,"all32bits preserved");
    Ptd p;Lookup q;q.results={p.base(),p.base()};
    auto d=er::dos_error_cell_05d704(s,q),r=er::errno_cell_05d724(s,q);
    require(d.raw()==static_cast<std::byte*>(p.base())+0x24 && r.raw()==static_cast<std::byte*>(p.base())+0x20,"unaligned PTD offsets");
    d.store(0xfedcba98);r.store(13);
    require(cell(p,0x24).load()==0xfedcba98 && cell(p,0x20).load()==13,"PTD writes distinct");
    require(s.dos_fallback==0xffffffff && s.errno_fallback==0x80000000,"PTD lookup leaves globals");
    long native=0;static_assert(sizeof(native)==4);er::Cell32(&native).store(0xffffffff);
    require(native==-1 && er::Cell32(&native).load()==0xffffffff,"native signed storage without alias cast");
}
void test_order() {
    for(unsigned selection=0;selection<4;++selection) {
        er::State s;Ptd a,b;cell(a,0x20).store(100);cell(a,0x24).store(101);
        cell(b,0x20).store(102);cell(b,0x24).store(103);
        Lookup e;e.results={(selection&1)?a.base():nullptr,(selection&2)?b.base():nullptr};
        e.before=[&](unsigned call){
            if(call==2)require(((selection&1)?cell(a,0x24).load():s.dos_fallback)==1113,"DOS written before second lookup");
        };
        er::map_os_error_05d698(1113,s,e);
        require(e.calls==2,"mapping performs two independent lookups");
        require(((selection&2)?cell(b,0x20).load():s.errno_fallback)==42,"second selected PTD gets mapped errno");
        require(cell(a,0x20).load()==100 && cell(b,0x24).load()==103,"other fields untouched");
        require((selection&1)||cell(a,0x24).load()==101,"null first lookup leaves first PTD");
        require((selection&2)||cell(b,0x20).load()==102,"null second lookup leaves second PTD");
    }
    for(unsigned fail=1;fail<=2;++fail) {
        er::State s;Lookup e;e.results={nullptr,nullptr};e.throw_on=fail;
        try{er::map_os_error_05d698(5,s,e);require(false,"lookup must throw");}catch(const Injected&){}
        require(e.calls==fail && s.errno_fallback==12 && s.dos_fallback==(fail==1?8u:5u),"exception retains only earlier writes");
    }
}
struct Thread : th::Effects {
    er::State state;Lookup lookup;th::Context context{};th::Context* slot{};
    bool fail_create{true},invalid_seen{};std::vector<int> events;
    th::Context* calloc_context_066848() override {events.push_back(1);return &context;}
    void free_0668c0(th::Context* p) override {events.push_back(p?9:2);}
    bool get_module_handle_ex_w(std::uint32_t,std::uintptr_t,th::Handle* out) override {events.push_back(3);*out=77;return true;}
    th::Handle create_thread(void*,std::uint64_t,th::Context*,std::uint32_t,std::uint32_t*) override {events.push_back(4);return fail_create?0:99;}
    std::uint32_t last_error() override {events.push_back(5);return 1113;}
    void write_error_slot_05d724(std::uint32_t code) override {er::errno_cell_05d724(state,lookup).store(code);}
    void invalid_parameter_05c954() override {invalid_seen=true;require(state.errno_fallback==22,"invalid handler observes errno write");}
    void map_os_error_05d698(std::uint32_t code) override {events.push_back(6);er::map_os_error_05d698(code,state,lookup);}
    bool close_handle(th::Handle) override {events.push_back(7);return true;}
    bool free_library(th::Handle h) override {require(h==77,"failure releases acquired module");events.push_back(8);return true;}
    th::Context*& required_thread_slot_064b90() override {return slot;}
    th::Context* optional_thread_context_064d08() override {return slot;}
    int runtime_policy_06c0d0() override {throw std::runtime_error("unused");}
    int initialize_runtime_067320(int) override {throw std::runtime_error("unused");}
    void uninitialize_runtime_06736c() override {throw std::runtime_error("unused");}
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t) override {throw std::runtime_error("unused");}
    [[noreturn]] void free_library_and_exit_thread(th::Handle,std::uint32_t) override {throw std::runtime_error("unused");}
    [[noreturn]] void exit_thread(std::uint32_t) override {throw std::runtime_error("unused");}
};
void test_thread_integration() {
    Thread f;f.lookup.results={nullptr,nullptr};std::uint32_t id=444;
    require(th::begin_thread_05e8e0(nullptr,0,123,456,0,&id,f)==0,"CRT thread failure result");
    require(id==444 && f.state.dos_fallback==1113 && f.state.errno_fallback==42,"real recovered mapper in failed thread launch");
    require(f.events==std::vector<int>{1,2,3,4,5,6,8,9},"error mapping before module/context cleanup");
    Thread invalid;invalid.lookup.results={nullptr};
    require(th::begin_thread_05e8e0(nullptr,0,0,456,0,&id,invalid)==0,"invalid entry returns zero");
    require(invalid.invalid_seen && invalid.lookup.calls==1 && invalid.state.dos_fallback==8 && invalid.events.empty(),"invalid entry only writes errno, no DOS mapping/allocation");
    Thread escaped;escaped.lookup.results={nullptr,nullptr};escaped.lookup.throw_on=2;
    try{(void)th::begin_thread_05e8e0(nullptr,0,123,456,0,&id,escaped);require(false,"lookup failure escapes caller");}catch(const Injected&){}
    require(escaped.state.dos_fallback==1113 && escaped.state.errno_fallback==12,"thread mapping exception retains DOS only");
    require(escaped.events==std::vector<int>{1,2,3,4,5,6},"no invented cleanup after mapping dependency throws");
}
}
int main() {try{test_table();test_cells();test_order();test_thread_integration();std::cout<<"PASS "<<checks<<" CRT error mapping checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
