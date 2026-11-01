#include "termination.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace t=anadius::crt_termination_recovered;
namespace o=anadius::crt_onexit_recovered;
namespace {
std::size_t checks=0;
void check(bool value,const char* message) {
    ++checks;
    if(!value) throw std::runtime_error(message);
}
struct Exit {std::uint32_t status;};
struct Stop {};
struct Fixture final:t::Effects {
    o::State tables{0x89abcdef01234567ULL,{},{}};
    std::uint64_t encoded=tables.cookie_0a1580;
    t::State state{tables,encoded};
    std::vector<std::string> trace;
    std::map<t::Address,std::uint64_t> words;
    std::map<t::Address,std::function<void()>> callbacks;
    std::map<t::Address,std::uint32_t> image;
    std::function<void(t::Address,t::Address)> ranges;
    std::function<void()> termination_callback;
    std::function<void(std::uint32_t)> on_unlock;
    std::uint32_t first_policy=1,second_policy=0;
    t::Address module_image=0,module_output=0,procedure=0;
    std::int32_t module_result=0,terminate_result=0;
    std::int32_t depth=0;
    std::size_t image_reads=0;
    Fixture() {
        clear(tables.crt_0a52a8);clear(state.quick_table_a52c0);
    }
    void clear(o::Table& table) {
        const auto c=tables.cookie_0a1580;table={c,c,c};
    }
    void table(o::Table& table,t::Address base,std::initializer_list<t::Address> values) {
        auto p=base;
        for(auto v:values) {words[p]=o::encode_pointer(v,tables.cookie_0a1580);p+=8;}
        table={o::encode_pointer(base,tables.cookie_0a1580),
            o::encode_pointer(p,tables.cookie_0a1580),
            o::encode_pointer(p,tables.cookie_0a1580)};
    }
    void lock_0625f8(std::uint32_t id) override {
        trace.push_back("lock:"+std::to_string(id));++depth;
    }
    void unlock_06264c(std::uint32_t id) override {
        trace.push_back("unlock:"+std::to_string(id));--depth;
        if(on_unlock) on_unlock(id);
    }
    t::Address recalloc_070434(t::Address,std::uint64_t,std::uint64_t) override {
        throw std::logic_error("allocation not expected in termination fixture");
    }
    void free_0668c0(t::Address p) override {trace.push_back("free:"+std::to_string(p));}
    std::uint64_t read_word(t::Address p) override {return words.at(p);}
    void write_word(t::Address p,std::uint64_t v) override {words.at(p)=v;}
    void invoke_callback(t::Address p) override {
        trace.push_back("callback:"+std::to_string(p));callbacks.at(p)();
    }
    void invoke_termination_callback(t::Address p,std::uint32_t a,
        std::uint32_t b,std::uint32_t c) override {
        check(p==0x345678,"decoded callback identity");
        check(a==0 && b==0 && c==0,"three zero callback arguments");
        check(depth==1,"termination callback under outer lock");
        trace.push_back("termination");
        if(termination_callback) termination_callback();
    }
    void initterm_range_064614(t::Address begin,t::Address end) override {
        check(depth>=1,"range callback under lock");
        check((begin==0x18007f728 && end==0x18007f748) ||
            (begin==0x18007f750 && end==0x18007f758),"exact initializer range");
        trace.push_back(begin==0x18007f728?"preterm":"term");
        if(ranges) ranges(begin,end);
    }
    t::Address get_module_handle_w_null() override {
        trace.push_back("image");return module_image;
    }
    std::uint16_t read_image_u16(t::Address p) override {
        ++image_reads;return static_cast<std::uint16_t>(image.at(p));
    }
    std::uint32_t read_image_u32(t::Address p) override {
        ++image_reads;return image.at(p);
    }
    std::int32_t get_module_handle_ex_w(std::uint32_t flags,
        std::u16string_view name,t::Address& output) override {
        check(flags==0 && name==u"mscoree.dll","managed module exact request");
        check(output==0,"managed output initialized zero");
        trace.push_back("managed-module");output=module_output;return module_result;
    }
    t::Address get_proc_address(t::Address p,std::string_view name) override {
        check(p==module_output && name=="CorExitProcess","managed export request");
        trace.push_back("managed-proc");return procedure;
    }
    void invoke_cor_exit_process(t::Address p,std::uint32_t status) override {
        check(p==procedure,"managed callback identity");
        trace.push_back("cor:"+std::to_string(status));
    }
    bool free_library(t::Address p) override {
        check(p==module_output,"managed module free identity");
        trace.push_back("free-library");return false;
    }
    std::uint32_t policy_06c110() override {trace.push_back("policy1");return first_policy;}
    std::uint32_t policy_06c09c() override {trace.push_back("policy2");return second_policy;}
    t::Address get_current_process() override {trace.push_back("process");return 0xffffffffffffffffULL;}
    std::int32_t terminate_process(t::Address process,std::uint32_t status) override {
        check(process==UINT64_MAX,"opaque process handle forwarded");
        trace.push_back("terminate:"+std::to_string(status));return terminate_result;
    }
    [[noreturn]] void exit_process(std::uint32_t status) override {
        trace.push_back("exit:"+std::to_string(status));throw Exit{status};
    }
    bool saw(const std::string& x) const {
        for(const auto& s:trace) if(s==x) return true;
        return false;
    }
    std::size_t count(const std::string& x) const {
        std::size_t n=0;for(const auto& s:trace) if(s==x) ++n;return n;
    }
};
void returning_cexit() {
    Fixture f;
    f.encoded=o::encode_pointer(0x345678,f.tables.cookie_0a1580);
    f.table(f.tables.crt_0a52a8,0x1000,{11,12,0,13});
    f.table(f.state.quick_table_a52c0,0x2000,{99});
    for(auto p:{11,12,13}) f.callbacks[p]=[&,p]{
        check(f.depth==2,"recursive lock during onexit");
        check(t::c_termination_complete_05cdb8(f.state)==1,"begun visible to callback");
        check(f.words.at(0x1000+8*(p==11?0:p==12?1:3))==f.tables.cookie_0a1580,
            "slot cleared before callback");
    };
    t::cexit_05cd9c(f.state,f);
    const std::vector<std::string> wanted={"lock:2","termination","lock:2",
        "callback:13","callback:12","callback:11","free:4096","unlock:2",
        "preterm","term","unlock:2"};
    check(f.trace==wanted,"cexit exact normal call ordering");
    check(f.depth==0 && f.state.completed_a4f18==0,"cexit not permanent completed");
    check(t::c_termination_complete_05cdb8(f.state)==1,"complete getter returns begun cell");
    check(f.tables.crt_0a52a8.begin==f.tables.cookie_0a1580,"CRT table cleared");
    check(f.words.at(0x2000)!=f.tables.cookie_0a1580,"quick table unconsumed");
    t::cexit_05cd9c(f.state,f);
    check(f.count("termination")==2 && f.count("preterm")==2 && f.count("term")==2,
        "repeat cexit reruns ranges and callback");
    check(f.count("callback:13")==1,"repeat cexit sees empty table");
    check(!f.saw("image") && !f.saw("managed-module") && !f.saw("policy1"),
        "returning cexit skips image and terminal providers");
}
void modes_and_completion() {
    for(auto mode:std::array<std::uint32_t,7>{0,1,2,3,255,0x100,UINT32_MAX}) {
        for(auto returning:std::array<std::uint32_t,4>{0,1,0x100,UINT32_MAX}) {
            Fixture f;f.encoded=o::encode_pointer(0x345678,f.tables.cookie_0a1580);
            f.table(f.tables.crt_0a52a8,0x1000,{11});
            f.table(f.state.quick_table_a52c0,0x2000,{22});
            f.callbacks[11]=[]{ };f.callbacks[22]=[]{ };
            std::uint8_t completed=0;
            const std::int32_t id=2;
            t::guarded_call_05cb00(id,{mode,returning,completed},id,f.state,f);
            check(f.saw("termination")== (mode==0),"mode zero encoded callback only");
            check(f.saw("callback:11")== (mode==0),"mode zero standard table only");
            check(f.saw("callback:22")== (mode==1),"mode one quick table only");
            check(f.saw("preterm")== (mode==0),"preterm zero mode only");
            check(f.count("term")==1,"term range every mode");
            check(completed==(returning==0),"return flag full32 zero comparison");
            check(f.state.completed_a4f18==(returning==0),"persistent completed byte");
            check(f.state.begun_a4f08.load()==1 && f.depth==0,"begun and lock balance");
        }
    }
    Fixture f;f.state.begun_a4f08.store(0xdeadbeef);f.state.completed_a4f18=0x80;
    t::cexit_05cd9c(f.state,f);
    check(f.trace==std::vector<std::string>{"lock:2","unlock:2"},"completed skip inside outer lock");
    check(t::c_termination_complete_05cdb8(f.state)==0xdeadbeef,"getter preserves full dword");
}
void mutations() {
    Fixture f;std::uint32_t mode=0,returning=1;std::uint8_t complete=0;
    f.encoded=o::encode_pointer(0x345678,f.tables.cookie_0a1580);
    f.table(f.tables.crt_0a52a8,0x1000,{11});f.callbacks[11]=[]{ };
    f.termination_callback=[&]{mode=2;};
    f.ranges=[&](t::Address,t::Address){returning=0;};
    const std::int32_t id=2;
    t::guarded_call_05cb00(id,{mode,returning,complete},id,f.state,f);
    check(f.saw("callback:11"),"initial zero branch table selection retained after callback");
    check(!f.saw("preterm") && complete==1,"mode and return fields reread after callbacks");
    Fixture g;mode=1;returning=1;complete=0;
    g.table(g.state.quick_table_a52c0,0x2000,{22});g.callbacks[22]=[&]{mode=0;};
    t::guarded_call_05cb00(id,{mode,returning,complete},id,g.state,g);
    check(g.saw("preterm") && !g.saw("termination"),"mode mutation affects later range only");
    Fixture h;h.table(h.tables.crt_0a52a8,0x1000,{11});
    h.callbacks[11]=[&]{
        h.table(h.tables.crt_0a52a8,0x3000,{33});h.callbacks[33]=[]{ };
    };
    t::cexit_05cd9c(h.state,h);
    check(h.saw("callback:33") && h.saw("free:12288"),"actual onexit rescans changed global table");
    Fixture k;mode=2;returning=1;complete=0;std::int32_t lock_id=-3,unlock_id=7;
    k.ranges=[&](t::Address,t::Address){unlock_id=-8;};
    t::guarded_call_05cb00(lock_id,{mode,returning,complete},unlock_id,k.state,k);
    check(k.trace.front()=="lock:4294967293" && k.trace.back()=="unlock:4294967288",
        "guard lock IDs read independently with original dword bits");
}
void exception_boundary() {
    Fixture f;f.encoded=o::encode_pointer(0x345678,f.tables.cookie_0a1580);
    f.termination_callback=[]{throw Stop{};};
    try {t::cexit_05cd9c(f.state,f);check(false,"expected synthetic callback throw");}
    catch(const Stop&) {}
    check(f.depth==1 && !f.saw("unlock:2"),"no invented exception unwind unlock");
    check(f.state.begun_a4f08.load()==1 && !f.state.completed_a4f18,
        "only completed normal-path writes retained before throw");
}
void managed() {
    for(auto result:std::array<std::int32_t,5>{0,1,-1,0x100,INT32_MIN})
    for(auto module:std::array<t::Address,2>{0,0xabcdef})
    for(auto procedure:std::array<t::Address,2>{0,0x123456}) {
        Fixture f;f.module_result=result;f.module_output=module;f.procedure=procedure;
        t::managed_exit_05cd24(0xabcdef01,f);
        check(f.saw("managed-proc")== (result!=0),"module result EAX not AL");
        check(f.saw("cor:2882400001")== (result!=0 && procedure!=0),"managed callback selection");
        check(f.saw("free-library")== (module!=0),"nonzero output freed regardless result");
        check(f.trace.back()==(module?"free-library":result&&procedure?"cor:2882400001":
            result?"managed-proc":"managed-module"),"managed cleanup order");
    }
}
void terminal_policy() {
    for(auto first:std::array<std::uint32_t,7>{0,1,2,0x100,0x101,0x80000001,UINT32_MAX})
    for(auto second:std::array<std::uint32_t,7>{0,1,2,0x100,0x101,0x80000000,UINT32_MAX}) {
        Fixture f;f.first_policy=first;f.second_policy=second;
        const auto wanted=first!=1 && static_cast<std::uint8_t>(second)==0;
        check(t::use_terminate_process_05cd00(f)==wanted,"termination policy EAX/AL");
        check(f.saw("policy2")== (first!=1),"policy short circuit");
        f.trace.clear();f.module_output=0xabcdef;f.module_result=0x100;f.procedure=0x123456;
        try {t::terminate_05cccc(0xabcdef01,f);}
        catch(const Exit& e) {check(e.status==0xabcdef01,"terminal status exact bits");}
        check(f.saw("terminate:2882400001")==wanted,"optional TerminateProcess selection");
        check(f.saw("cor:2882400001") && f.trace.back()=="exit:2882400001",
            "TerminateProcess returning continues managed then ExitProcess");
        check(f.count("free-library")==1,"managed release on terminal normal path");
    }
}
void image_and_core() {
    // Every PE check short circuits. Bounded provider maps only reachable reads.
    for(int stop=0;stop!=7;++stop) {
        Fixture f;const t::Address base=0x100000,nt=base+0x200;
        f.module_image=stop==0?0:base;
        f.image[base]=stop==1?0:0x5a4d;f.image[base+0x3c]=0x200;
        f.image[nt]=stop==2?0:0x4550;f.image[nt+0x18]=stop==3?0x10b:0x20b;
        f.image[nt+0x84]=stop==4?14:15;f.image[nt+0xf8]=stop==5?0:1;
        try {t::common_exit_05cc04(77,2,0,f.state,f);}
        catch(const Exit& e) {check(e.status==77,"common core forwards exit status");}
        const std::array<std::size_t,7> reads={0,1,3,4,5,6,6};
        check(f.image_reads==reads[stop],"PE exact short circuit reads");
        check(f.count("managed-module")==static_cast<std::size_t>(stop==6?2:1),
            "managed image causes early and final managed exit attempts");
        check(f.state.completed_a4f18==1 && f.state.begun_a4f08.load()==1,
            "terminal core sets completion state before exit");
        check(f.count("term")==1 && !f.saw("preterm") && f.depth==0,"mode2 ranges and lock");
    }
    Fixture g;const t::Address base=0x100000,nt=base-0x200;
    g.module_image=base;g.image={{base,0x5a4d},{base+0x3c,0xfffffe00},
        {nt,0x4550},{nt+0x18,0x20b},{nt+0x84,UINT32_MAX},{nt+0xf8,0x80000000}};
    try {t::immediate_exit_05cdac(9,g.state,g);} catch(const Exit&) {}
    check(g.image_reads==6 && g.count("managed-module")==2,"signed e_lfanew and unsigned directory count");
    Fixture h;
    try {t::exit_05cdc0(10,h.state,h);} catch(const Exit&) {}
    check(h.saw("preterm") && h.saw("term") && h.state.completed_a4f18==1,
        "exit wrapper selects full mode zero");
}
}
int main() {
    try {
        returning_cexit();modes_and_completion();mutations();exception_boundary();
        managed();terminal_policy();image_and_core();
        std::cout<<"PASS: "<<checks<<" termination checks\n";
    } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}
