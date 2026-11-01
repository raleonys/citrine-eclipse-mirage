// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_worker_startup.hpp"
#include <algorithm>
#include <bit>
#include <functional>
#include <iostream>
#include <vector>
namespace s=preloader_l::socket;
namespace t=preloader_l::threads;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected worker dependency");}
struct Raised{s::Error error;std::uintptr_t context;};
struct Fixture;
struct Api final:s::WorkerStartupApi{
 Fixture& f;explicit Api(Fixture& f):f(f){}
 std::uintptr_t create_event(std::uintptr_t,std::int32_t,std::int32_t,std::uintptr_t)override;
 std::uint32_t get_last_error()override;
 std::int32_t close_handle(std::uintptr_t)override;
 std::uint32_t wait_one(std::uintptr_t,std::uint32_t)override;
 std::int32_t set_event(std::uintptr_t)override;
 std::uint32_t sleep_ex(std::uint32_t,std::int32_t)override;
};
struct Errors final:s::Api{
 Fixture& f;explicit Errors(Fixture& f):f(f){}
 s::Category system_category_028620()override;
 void set_last_error(std::int32_t)override{unused();}std::int32_t get_last_error()override{unused();}
 s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
 s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}
 std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Effects:s::WorkerStartupEffects{
 Fixture& f;explicit Effects(Fixture&);
 s::IocpWorker* allocate_worker_04b0bc(std::uint32_t)override{unused();}
 s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t)override{unused();}
 void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override{unused();}
 [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override;
 std::uintptr_t begin_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::IocpWorkerFunction*,std::uint32_t,std::uint32_t&)override;
 void delete_function_slot_00(s::IocpWorkerFunction&,std::uint32_t)override;
 void invoke_function_slot_08(s::IocpWorkerFunction&)override;
};
struct Fixture{
 Api api{*this};Errors errors{*this};Effects effects{*this};s::IocpWorker worker{11,22,33};s::IocpWorkerFunction function{44,55,66,nullptr};
 s::IocpWorkerFunction* expected=&function;
 std::uintptr_t entry=0x123456789abcdef0ULL,exit=0xfedcba9876543210ULL,thread=0x9988776655443322ULL;
 std::uint32_t status=0,stack=0,wait_result=0,sleep_result=0;std::int32_t api_result=1;
 unsigned creates=0,categories=0,deletes=0,runs=0;bool heap_delete=false;
 std::vector<std::string> trace;std::vector<std::uintptr_t> closed,waited,signaled;
 std::function<void(std::string_view)> hook;
 void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
 void run(){effects.start_worker_02a000(worker,expected,stack);}
};
Effects::Effects(Fixture& f):WorkerStartupEffects(f.api,f.errors),f(f){}
std::uintptr_t Api::create_event(std::uintptr_t security,std::int32_t manual,std::int32_t initial,std::uintptr_t name){check(!security && manual==1 && !initial && !name,"exact unnamed manual nonsignaled event arguments");const auto value=++f.creates==1?f.entry:f.exit;f.event(f.creates==1?"create-entry":"create-exit");return value;}
std::uint32_t Api::get_last_error(){auto value=f.status;f.event("error");return value;}
std::int32_t Api::close_handle(std::uintptr_t h){f.closed.push_back(h);f.event("close");return f.api_result;}
std::uint32_t Api::wait_one(std::uintptr_t h,std::uint32_t timeout){check(timeout==0xffffffffU,"infinite entry handshake wait");f.waited.push_back(h);f.event("wait");return f.wait_result;}
std::int32_t Api::set_event(std::uintptr_t h){f.signaled.push_back(h);f.event("signal");return f.api_result;}
std::uint32_t Api::sleep_ex(std::uint32_t ms,std::int32_t alertable){check(ms==0xffffffffU && alertable==1,"infinite alertable SleepEx once");f.event("sleep");return f.sleep_result;}
s::Category Errors::system_category_028620(){++f.categories;f.event("category");return {0x9922,preloader_l::server::ErrorCategory::system};}
[[noreturn]]void Effects::raise_028240(s::Error error,std::uintptr_t context){f.event("raise");throw Raised{error,context};}
std::uintptr_t Effects::begin_thread_05e8e0(std::uintptr_t security,std::uint32_t stack,std::uintptr_t entry,s::IocpWorkerFunction* argument,std::uint32_t flags,std::uint32_t& id){check(!security && stack==f.stack && entry==0x18002a4a0ULL && argument==f.expected && !flags && !id,"cached uint32 stack, opaque entry, live argument, flags0 and zero local id");auto result=f.thread;f.event("thread");id=0xfedcba98U;return result;}
void Effects::delete_function_slot_00(s::IocpWorkerFunction& function,std::uint32_t flags){check(&function==f.expected && flags==1,"exact deleting slot target and flag1");++f.deletes;f.event("delete");if(f.heap_delete){delete &function;f.expected=nullptr;}}
void Effects::invoke_function_slot_08(s::IocpWorkerFunction& function){check(&function==f.expected,"exact run slot target");++f.runs;f.event("run");}
void success_matrix(){
 for(auto stack:{0U,1U,0x80000000U,0xffffffffU})for(auto result:{0,-1})for(auto wait:{0U,0x80U,0x102U,0xffffffffU}){
  Fixture f;f.stack=stack;f.api_result=result;f.wait_result=wait;
  f.hook=[&](auto name){if(name=="create-entry")check(f.function.event_08==55 && f.worker.event_10==33,"first API before any function store");if(name=="create-exit")check(f.function.event_08==f.entry && f.function.event_10==66 && f.worker.event_10==33,"first event stored before second API; second fields unchanged");if(name=="thread"){check(f.worker.event_10==f.exit && f.function.event_10==f.exit && f.worker.handle_08==22,"both exit fields stored before thread call");f.function.event_08=77;f.function.event_10=88;f.worker.handle_08=99;}if(name=="wait")check(f.worker.handle_08==f.thread,"returned thread stored after callback before wait");};
  f.run();check(f.trace==std::vector<std::string>{"create-entry","create-exit","thread","wait","close"},"success exact callback order, ignored API return values");check(f.closed==std::vector<std::uintptr_t>{f.entry} && f.waited==f.closed,"cached first event retained despite function mutation");check(f.worker.handle_08==f.thread && f.worker.event_10==f.exit && !f.deletes && !f.categories,"success fields and no error/category/delete calls");
 }
 Fixture f;f.expected=nullptr;bool caught=false;try{f.run();}catch(const s::UnsupportedInput&){caught=true;}check(caught && f.trace==std::vector<std::string>{"create-entry"} && f.closed.empty() && f.worker.handle_08==22,"null projection rejected only after first API, without invented cleanup");
}
void failures(){
 for(unsigned phase=0;phase<3;++phase)for(auto status:{1U,0x7fffffffU,0x80000000U,0xffffffffU}){
  Fixture f;f.status=status;if(phase==0)f.entry=0;else if(phase==1)f.exit=0;else f.thread=0;
  f.hook=[&](auto name){if(name=="delete" || name=="category")f.status=0;if(name=="delete" && phase==2){f.function.event_08=111;f.worker.event_10=222;}if(name=="close" && f.closed.size()==1)f.worker.event_10=333;};
  bool caught=false;try{f.run();}catch(const Raised& e){caught=true;check(std::bit_cast<std::uint32_t>(e.error.value)==status && e.error.category.name_identity==0x9922,"cached LastError bits survive delete/category mutations");check(e.context==(phase==0?0x1800844c8ULL:phase==1?0x1800844e0ULL:0x1800844f4ULL),"phase-specific original message identity");}
  check(caught && f.deletes==1 && f.categories==1 && f.waited.empty(),"nonzero failure raises once without final wait or fabricated unwind");
  std::vector<std::string> want{"create-entry"};if(phase>0)want.push_back("create-exit");if(phase==2)want.push_back("thread");want.insert(want.end(),{"error","delete"});if(phase==2)want.insert(want.end(),{"close","close"});want.insert(want.end(),{"category","raise"});check(f.trace==want,"failure ordering caches error before delete, category after cleanup");
  if(phase==2)check(f.closed==std::vector<std::uintptr_t>{f.entry,333} && !f.worker.handle_08,"close cached entry then live worker exit reloaded after close callback");else check(f.closed.empty(),"event creation failures invent no CloseHandle");
 }
 for(bool entryzero:{false,true})for(bool exitzero:{false,true}){
  Fixture f;if(entryzero)f.entry=0;if(exitzero)f.exit=0;f.thread=0;f.run();
  check(f.deletes==1+static_cast<unsigned>(entryzero)+static_cast<unsigned>(exitzero) && f.categories==f.deletes,"zero error continues with live borrowed projection through repeated delete callbacks");
  std::vector<std::uintptr_t> expected;if(!entryzero)expected.push_back(f.entry);if(!exitzero)expected.push_back(f.exit);if(!entryzero)expected.push_back(f.entry);check(f.closed==expected,"thread zeroerror preserves conditional cleanup and repeated cached entry close");check(f.waited.size()==static_cast<unsigned>(!entryzero),"zero error final wait only for cached nonzero entry");
 }
 for(auto stop:{"create-entry","create-exit","thread","wait","close"}){Fixture f;f.hook=[&](auto name){if(name==stop)throw std::runtime_error("selected provider stop");};bool caught=false;try{f.run();}catch(const std::runtime_error&){caught=true;}check(caught && !f.deletes && !f.categories && f.trace.back()==stop,"provider exception propagates without invented rollback or exception conversion");}
}
void entry(){
 for(auto result:{0,-1})for(auto sleep:{0U,0xc0U,0xffffffffU}){
  Fixture f;f.api_result=result;f.sleep_result=sleep;
  f.hook=[&](auto name){if(name=="signal" && f.signaled.size()==1)f.function.event_10=77;if(name=="run")f.function.event_10=88;if(name=="delete")f.function.event_10=99;};
  check(!s::worker_entry_02a4a0(f.function,f.api,f.effects),"entry returns uint32zero after ignored SleepEx result");check(f.trace==std::vector<std::string>{"signal","run","delete","signal","sleep"} && f.signaled==std::vector<std::uintptr_t>{55,88},"entry caches live exit after run and before delete");check(f.runs==1 && f.deletes==1 && !f.categories,"entry invokes each virtual slot once");
 }
 Fixture f;f.expected=new s::IocpWorkerFunction{44,55,66,nullptr};f.heap_delete=true;check(!s::worker_entry_02a4a0(*f.expected,f.api,f.effects) && !f.expected && f.signaled==std::vector<std::uintptr_t>{55,66},"actual heap lifetime may end at delete; subsequent source effects use cached exit only");
 for(auto stop:{"signal","run","delete","sleep"}){Fixture g;g.hook=[&](auto name){if(name==stop)throw std::runtime_error("stop entry");};bool caught=false;try{s::worker_entry_02a4a0(g.function,g.api,g.effects);}catch(const std::runtime_error&){caught=true;}check(caught && g.trace.back()==stop,"entry callback interruption has no fabricated cleanup/signaling/loop");}
}
struct ThreadEffects final:t::Effects{
 t::Context context{},*slot=nullptr;bool allocation=true;std::uintptr_t returned=0xaabbccddeeff0011ULL;std::uint32_t requested_stack=0,status=5;void* security=nullptr;unsigned frees=0,maps=0,creates=0;std::function<std::uint32_t(std::uintptr_t,std::uintptr_t)> dispatch;
 t::Context* calloc_context_066848()override{context={};return allocation?&context:nullptr;}
 void free_0668c0(t::Context* p)override{check(!p || p==&context,"CRT context cleanup receives owned projection or null");++frees;}
 bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t address,t::Handle* result)override{check(flags==4 && address==0x18002a4a0ULL && result==&context.module,"existing CRT receives opaque worker entry and proper module slot");*result=0x7788;return false;}
 t::Handle create_thread(void* sec,std::uint64_t stack,t::Context* p,std::uint32_t flags,std::uint32_t* id)override{check(sec==security && stack==requested_stack && p==&context && !flags && id,"existing CRT forwards security/zeroextended stack/context/flags");++creates;*id=0xfedcba98U;return returned;}
 std::uint32_t last_error()override{return status;}void write_error_slot_05d724(std::uint32_t)override{unused();}void invalid_parameter_05c954()override{unused();}
 void map_os_error_05d698(std::uint32_t value)override{check(value==status,"existing CRT caches and maps OS error");++maps;}
 bool close_handle(t::Handle)override{unused();}bool free_library(t::Handle value)override{check(value==0x7788,"failed CRT thread releases module token");return false;}
 t::Context*& required_thread_slot_064b90()override{return slot;}t::Context* optional_thread_context_064d08()override{return slot;}
 int runtime_policy_06c0d0()override{return 0;}int initialize_runtime_067320(int)override{unused();}void uninitialize_runtime_06736c()override{unused();}
 std::uint32_t invoke_entry(std::uintptr_t identity,std::uintptr_t argument)override{return dispatch(identity,argument);}
 [[noreturn]]void free_library_and_exit_thread(t::Handle,std::uint32_t)override{unused();}[[noreturn]]void exit_thread(std::uint32_t)override{unused();}
};
struct Recovered final:s::WorkerStartupEffectsAdapter{
 Fixture& f;Recovered(Fixture& f,ThreadEffects& t):WorkerStartupEffectsAdapter(f.api,f.errors,t),f(f){}
 s::IocpWorker* allocate_worker_04b0bc(std::uint32_t)override{unused();}s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t)override{unused();}
 void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override{unused();}[[noreturn]]void raise_028240(s::Error e,std::uintptr_t c)override{throw Raised{e,c};}
 void delete_function_slot_00(s::IocpWorkerFunction& p,std::uint32_t flags)override{check(&p==f.expected && flags==1,"recovered bridge delete slot target");++f.deletes;}
 void invoke_function_slot_08(s::IocpWorkerFunction& p)override{check(&p==f.expected,"recovered bridge run target");++f.runs;}
};
void recovered_bridge(){
 for(auto stack:{0U,1U,0x80000000U,0xffffffffU}){Fixture f;ThreadEffects threads;Recovered system_services(f,threads);threads.requested_stack=stack;system_services.start_worker_02a000(f.worker,&f.function,stack);check(f.worker.handle_08==threads.returned && threads.context.entry==0x18002a4a0ULL && threads.context.argument==reinterpret_cast<std::uintptr_t>(&f.function) && threads.creates==1 && threads.frees==1,"actual unchanged05e8e0 bridge stores worker handle and opaque context with unconditional free(null)");threads.dispatch=[&](auto identity,auto argument){check(identity==0x18002a4a0ULL && argument==reinterpret_cast<std::uintptr_t>(&f.function),"controlled dispatch compares token, never executes image address");return s::worker_entry_02a4a0(*reinterpret_cast<s::IocpWorkerFunction*>(argument),f.api,system_services);};check(!threads.invoke_entry(threads.context.entry,threads.context.argument) && f.runs==1 && f.deletes==1,"controlled source worker entry executes through synthetic CRT Effects routing");}
 for(bool allocated:{false,true}){Fixture f;ThreadEffects threads;threads.allocation=allocated;threads.returned=0;Recovered system_services(f,threads);f.status=7;bool caught=false;try{system_services.start_worker_02a000(f.worker,&f.function,0);}catch(const Raised& e){caught=e.context==0x1800844f4ULL && e.error.value==7;}check(caught && !f.worker.handle_08 && threads.creates==static_cast<unsigned>(allocated) && threads.maps==static_cast<unsigned>(allocated) && threads.frees==1+static_cast<unsigned>(allocated),"existing CRT allocation/thread failures feed recovered startup failure with proper ownership effects");}
}
}
int main(){try{success_matrix();failures();entry();recovered_bridge();std::cout<<"PASS: "<<checks<<" socket worker startup checks; retained02a000/02a4a0 callbacks, cached errors/handles, controlled lifetime and unchanged recovered05e8e0 bridge\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
