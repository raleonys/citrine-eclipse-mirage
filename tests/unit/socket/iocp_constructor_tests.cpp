// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_iocp_constructor.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>
namespace s=laureondo::socket;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected controlled IOCP dependency");}
struct Raised {s::Error error;std::uintptr_t context;};
struct Gate final:s::SocketGate {void enter()override{unused();}void leave()override{unused();}};
struct Entry final:s::IocpSchedulerEntry {using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unused();}};
struct Fixture;
struct Api final:s::IocpConstructorApi {
    Fixture& f;explicit Api(Fixture& f):f(f){}
    std::uint64_t set_version_mask(std::uint64_t,std::uint32_t,std::uint8_t)override;
    std::int32_t verify_version(OSVERSIONINFOEXA&,std::uint32_t,std::uint64_t)override;
    std::uintptr_t create_completion_port(std::uintptr_t,std::uintptr_t,std::uintptr_t,std::uint32_t)override;
    std::uint32_t get_last_error()override;
    std::int32_t close_handle(std::uintptr_t)override;
};
struct Gates final:s::ServiceGateApi {
    Fixture& f;explicit Gates(Fixture& f):f(f){}
    std::int32_t initialize(CRITICAL_SECTION&,std::uint32_t)override;
    std::uint32_t get_last_error()override;
};
struct Errors final:s::Api {
    Fixture& f;explicit Errors(Fixture& f):f(f){}
    s::Category system_category_028620()override;
    void set_last_error(std::int32_t)override{unused();}std::int32_t get_last_error()override{unused();}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Effects final:s::IocpConstructionEffects {
    Fixture& f;explicit Effects(Fixture& f):f(f){}
    s::IocpWorker* allocate_worker_04b0bc(std::uint32_t)override;
    s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t)override;
    void start_worker_02a000(s::IocpWorker&,s::IocpWorkerFunction*,std::uint32_t)override;
    void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override;
    [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override;
};
struct Fixture {
    Gate gate;CRITICAL_SECTION section{};Entry entry{gate,section};Api api{*this};Gates gates{*this};Errors errors{*this};Effects effects{*this};
    s::IocpWorker fresh{0x111,0x222,0x333},old{0x444,0x555,0x666},other{0x777,0x888,0x999};
    s::IocpWorkerFunction function{0xaaa,0xbbb,0xccc,nullptr};s::CancelOperation sentinel;
    std::uintptr_t owner=0xfedcba9876543210ULL,port=0x8877665544332211ULL;
    std::uint64_t mask=0xaabbccddeeff0088ULL;
    std::int32_t version=1,gate_result=1,close_result=1;std::uint32_t gate_error=0,port_error=0,hint=0;
    bool worker_null=false,function_null=false;unsigned categories=0;std::vector<std::string> trace;
    std::function<void(std::string_view)> hook;
    Fixture(){
        entry.key={11,12};entry.next=&entry;entry.owner_context=13;entry.table_identity=14;entry.completion.port=15;
        entry.work_count_30=16;entry.high_34=17;entry.word_38=18;entry.timeout_40=19;
        entry.completion.head=&sentinel;entry.completion.tail=&sentinel;entry.completion.dispatch_required=20;
        entry.word_88=21;entry.word_90=22;entry.word_98=23;entry.concurrency_a0=24;entry.worker_a8=&old;
    }
    void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
    Entry& run(std::uint8_t own=0){return static_cast<Entry&>(s::construct_iocp_024990(entry,owner,hint,own,api,gates,errors,effects));}
};
std::uint64_t Api::set_version_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t cond){
    check(!mask && type==2 && cond==3,"VerSet exact zero/type2/condition3");
    const auto& e=f.entry;check(!e.key.type && !e.key.id && !e.next && e.owner_context==f.owner && e.table_identity==0x180084510ULL,"base header precedes first API");
    check(!e.completion.port && !e.work_count_30 && !e.high_34 && !e.word_38,"full count qword/port/word38 zero precede version API");
    check(e.timeout_40==19 && e.completion.head==&f.sentinel && e.word_88==21 && e.worker_a8==&f.old,"later fields untouched before version callback");f.event("mask");return f.mask;
}
std::int32_t Api::verify_version(OSVERSIONINFOEXA& version,std::uint32_t type,std::uint64_t mask){
    OSVERSIONINFOEXA expected{};expected.dwOSVersionInfoSize=0x9c;expected.dwMajorVersion=6;
    check(sizeof(version)==0x9c && !std::memcmp(&version,&expected,sizeof(version)),"entire156-byte version request zero except size and major6");
    check(type==2 && mask==f.mask,"Verify receives exact returned64-bit mask");f.event("verify");return f.version;
}
std::int32_t Gates::initialize(CRITICAL_SECTION& section,std::uint32_t spin){
    check(&section==&f.section && spin==0x80000000U,"borrowed section and exact unsigned spin");
    check(f.entry.timeout_40==(f.version?0xffffffffU:500U) && !f.entry.completion.head && !f.entry.completion.tail && !f.entry.completion.dispatch_required,"timeout and queue initialization before gate");
    check(f.entry.word_88==21 && f.entry.worker_a8==&f.old,"late fields not cleared before gate");f.event("gate");return f.gate_result;
}
std::uint32_t Gates::get_last_error(){f.event("gate-error");return f.gate_error;}
s::Category Errors::system_category_028620(){++f.categories;f.event("category");return {0x9922,laureondo::server::ErrorCategory::system};}
std::uintptr_t Api::create_completion_port(std::uintptr_t file,std::uintptr_t existing,std::uintptr_t key,std::uint32_t threads){
    check(file==~std::uintptr_t{0} && !existing && !key && threads==((f.hint&0x80000000U)?0xffffffffU:f.hint),"CreateIOCP exact invalid/null/zero/sign-normalized hint");
    check(!f.entry.word_88 && !f.entry.word_90 && !f.entry.word_98 && f.entry.concurrency_a0==f.hint && !f.entry.worker_a8,"late fields cleared and RAWhint stored before CreateIOCP");
    f.event("port");return f.port;
}
std::uint32_t Api::get_last_error(){check(!f.entry.completion.port,"returned null port stored before GetLastError");f.event("port-error");return f.port_error;}
std::int32_t Api::close_handle(std::uintptr_t handle){check(handle==f.old.handle_08,"CloseHandle receives live cached-old handle");f.event("close");return f.close_result;}
s::IocpWorker* Effects::allocate_worker_04b0bc(std::uint32_t bytes){check(bytes==24,"exact native24 request");f.event("allocate-worker");return f.worker_null?nullptr:&f.fresh;}
s::IocpWorkerFunction* Effects::allocate_function_04b0bc(std::uint32_t bytes){check(bytes==32 && f.fresh.word_00==0x111 && !f.fresh.handle_08 && !f.fresh.event_10,"native32 request after ONLY worker+8/+10 zero");f.event("allocate-function");return f.function_null?nullptr:&f.function;}
void Effects::start_worker_02a000(s::IocpWorker& worker,s::IocpWorkerFunction* function,std::uint32_t stack){
    check(&worker==&f.fresh && function==(f.function_null?nullptr:&f.function) && !stack,"required worker-start receives exact nullable function and zero stack");
    if(function)check(function->table_identity==0x180084710ULL && function->scheduler_18==&f.entry && function->event_08==0xbbb && function->event_10==0xccc,"function writes table/scheduler only");f.event("start");
}
void Effects::delete_worker_04b0f8(s::IocpWorker& worker,std::uint32_t bytes){check(&worker==&f.old && bytes==24,"delete cached old allocation24 after close even live slot changes");f.event("delete");}
[[noreturn]]void Effects::raise_028240(s::Error error,std::uintptr_t context){f.event("raise");throw Raised{error,context};}
void matrix(){
    for(auto version:{0,1,-1,std::numeric_limits<int>::min()})for(auto hint:{0U,1U,0x7fffffffU,0x80000000U,0xffffffffU})for(auto own:{std::uint8_t{0},std::uint8_t{1},std::uint8_t{128},std::uint8_t{255}}){
        Fixture f;f.version=version;f.hint=hint;f.gate_result=-1;
        f.hook=[&](auto name){if(name=="allocate-worker")check(f.entry.work_count_30==1 && !f.entry.high_34,"low work count increment precedes worker allocation");};
        check(&f.run(own)==&f.entry && f.entry.timeout_40==(version?0xffffffffU:500U),"constructor returns same entry and selects zero-only BOOL timeout");
        check(f.entry.completion.port==f.port && f.entry.concurrency_a0==hint && f.categories==1,"success retains fullwidth port/rawhint and unconditional gate category");
        check(f.entry.worker_a8==(own?&f.fresh:nullptr) && f.entry.work_count_30==static_cast<unsigned>(own!=0),"own-thread lowbyte nonzero controls complete path");
        std::vector<std::string> expected{"mask","verify","gate","category","port"};if(own){expected.insert(expected.end(),{"allocate-worker","allocate-function","start"});}
        check(f.trace==expected,"exact successful callback order; no initial worker teardown");
    }
}
void failures(){
    for(auto status:{1U,0x7fffffffU,0x80000000U,0xffffffffU}){
        Fixture f;f.gate_result=0;f.gate_error=status;f.hook=[&](auto name){if(name=="category")f.gate_error=0;};bool caught=false;
        try{f.run(1);}catch(const Raised& e){caught=true;check(e.context==0x180084428ULL && std::bit_cast<std::uint32_t>(e.error.value)==status && e.error.category.name_identity==0x9922,"gate failure exact32bits cached before category plus opaque context");}
        check(caught && f.trace==std::vector<std::string>{"mask","verify","gate","gate-error","category","raise"},"gate failure prevents CreateIOCP/worker/rollback");
        check(f.entry.word_88==21 && f.entry.word_90==22 && f.entry.word_98==23 && f.entry.concurrency_a0==24 && f.entry.worker_a8==&f.old && !f.entry.completion.port,"gate failure preserves partial state and all late fields");
    }
    for(auto status:{0U,1U,0x7fffffffU,0x80000000U,0xffffffffU}){
        Fixture f;f.port=0;f.port_error=status;f.hook=[&](auto name){if(name=="category" && f.categories==2)f.port_error=0;};bool caught=false;
        try{f.run(1);}catch(const Raised& e){caught=true;check(e.context==0x180084528ULL && std::bit_cast<std::uint32_t>(e.error.value)==status && e.error.category.name_identity==0x9922,"port failure cached32bits and exact opaque context");}
        check(caught==(status!=0) && f.categories==2 && !f.entry.completion.port,"zero port still gets error then category; only nonzero status raises");
        check(f.entry.worker_a8==(status?nullptr:&f.fresh) && f.entry.work_count_30==(status?0U:1U),"port error no worker/rollback; zeroerror continues worker path");
    }
    Fixture f;f.gate_result=0;f.gate_error=0;f.run();check(f.categories==1 && f.entry.completion.port==f.port,"gate API zero with LastError zero continues after category");
}
void callbacks(){
    for(bool null_worker:{false,true})for(bool null_function:{false,true})for(auto close_result:{0,-1}){
        Fixture f;f.worker_null=null_worker;f.function_null=null_function;f.close_result=close_result;
        f.hook=[&](auto name){
            if(name=="port"){f.entry.work_count_30=0xffffffffU;f.entry.high_34=0xaabbccddU;f.entry.worker_a8=&f.other;f.entry.word_38=101;f.entry.completion.port=0x9999;}
            if(name=="allocate-worker"){check(!f.entry.work_count_30 && f.entry.high_34==0xaabbccddU,"atomic increment wraps only low32 preserving live highword");if(null_worker)f.entry.worker_a8=&f.old;}
            if(name=="start"){f.fresh.handle_08=0x1234;f.fresh.event_10=0x5678;f.entry.worker_a8=&f.old;}
            if(name=="close")f.entry.worker_a8=&f.other;
            if(name=="delete"){check(f.entry.worker_a8==&f.other,"delete sees close callback mutation");f.entry.worker_a8=nullptr;f.entry.timeout_40=77;}
        };
        f.run(255);check(f.entry.worker_a8==(null_worker?nullptr:&f.fresh) && f.entry.timeout_40==77 && f.entry.word_38==101 && f.entry.completion.port==f.port,"cached fresh stored last and returned port stored after callback while unrelated mutations survive");
        check(f.trace[f.trace.size()-2]=="close" && f.trace.back()=="delete","oldworker cleanup after allocate/start regardless null fresh or CloseHandle zero");
        check(std::count(f.trace.begin(),f.trace.end(),"start")==static_cast<int>(!null_worker) && std::count(f.trace.begin(),f.trace.end(),"allocate-function")==static_cast<int>(!null_worker),"null24 skips function/start; null32 delegated once to required provider");
        if(!null_worker)check(f.fresh.handle_08==0x1234 && f.fresh.event_10==0x5678,"worker-start writes remain live without replaying zeroes");
    }
    Fixture f;f.hook=[&](auto name){if(name=="port"){f.entry.worker_a8=&f.old;f.entry.work_count_30=9;f.entry.high_34=33;f.entry.key={44,55};}};f.run(0);
    check(f.entry.worker_a8==&f.old && f.entry.work_count_30==9 && f.entry.high_34==33 && f.entry.key.type==44 && f.entry.key.id==55,"ownzero preserves callback worker/count/header without cleanup or replay");
    for(auto phase:{"allocate-worker","allocate-function","start"}){Fixture interrupted;interrupted.hook=[&](auto name){if(name==phase)throw std::runtime_error("provider stop");};bool caught=false;try{interrupted.run(1);}catch(const std::runtime_error&){caught=true;}check(caught && interrupted.entry.work_count_30==1 && !interrupted.entry.worker_a8 && std::count(interrupted.trace.begin(),interrupted.trace.end(),"close")==0 && std::count(interrupted.trace.begin(),interrupted.trace.end(),"delete")==0,"provider interruption has no fabricated decrement/teardown/rollback");}
}
struct Registry final:s::IocpSchedulerRegistryEffects {
    using IocpSchedulerRegistryEffects::IocpSchedulerRegistryEffects;
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t,std::uintptr_t)override{unused();}
    s::ServiceEntry* allocate_04b0bc(std::uint32_t)override{unused();}
};
struct Wrong final:s::ServiceEntry {void scalar_delete(std::uint32_t)override{unused();}};
void bridge(){Fixture f;Registry bridge(f.api,f.gates,f.errors,f.effects);Wrong wrong;wrong.key={11,12};bool caught=false;try{bridge.construct_scheduler_024990(wrong,1,2,3);}catch(const s::UnsupportedInput&){caught=true;}check(caught && f.trace.empty() && wrong.key.type==11 && wrong.key.id==12,"host dynamic type rejects before any effects or writes");check(bridge.construct_scheduler_024990(f.entry,f.owner,f.hint,0)==&f.entry && f.categories==1,"typed bridge invokes actual024990 body");}
}
int main(){try{matrix();failures();callbacks();bridge();std::cout<<"PASS: "<<checks<<" socket IOCP constructor checks; retained024990 ordering/partial failure/live callback boundaries with controlled providers only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
