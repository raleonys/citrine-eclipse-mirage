// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_worker_run.hpp"
#include <bit>
#include <functional>
#include <iostream>
#include <vector>
namespace s=laureondo::socket;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected worker run dependency");}
struct Raised{s::Error error;std::uintptr_t message;};
struct Gate:s::SocketGate{void enter()override{unused();}void leave()override{unused();}};
struct Entry:s::IocpSchedulerEntry{using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unused();}};
struct Fixture;
struct Api:s::WorkerRunApi{
 Fixture& f;explicit Api(Fixture& f):f(f){}
 void* tls_get(std::uint32_t)override;std::int32_t tls_set(std::uint32_t,void*)override;
 std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override;
 std::uint32_t get_last_error()override;
};
struct Errors:s::Api{
 Fixture& f;explicit Errors(Fixture& f):f(f){}
 s::Category system_category_028620()override;
 void set_last_error(std::int32_t)override{unused();}std::int32_t get_last_error()override{unused();}
 s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
 s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}
 std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Effects:s::WorkerRunEffects{
 Fixture& f;explicit Effects(Fixture& f):f(f){}
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry&,std::uint32_t)override;
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry&,std::uint32_t)override;
 std::uint64_t dispatch_027c10(s::IocpSchedulerEntry&,std::uint32_t,s::WorkerThreadState&,s::WorkerRunResult&)override;
 void initialize_04a240(std::array<std::uintptr_t,2>&)override;
 void finish_04a2cc(std::array<std::uintptr_t,2>&)override;
 void free_cache_04b788(std::uintptr_t)override;
 void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override;
 [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override;
};
struct Fixture{
 Gate gate;CRITICAL_SECTION section{};Entry scheduler{gate,section};
 Api api{*this};Errors errors{*this};Effects effects{*this};std::uint32_t index=0xfedcba98U,status=0;
 s::Category generic{0x8899,laureondo::server::ErrorCategory::other};
 std::int32_t boolean=0,set_result=0;void* tls=reinterpret_cast<void*>(0x12345678ULL);
 std::vector<std::string> trace;std::vector<std::pair<std::uint32_t,void*>> setters;
 std::vector<std::uint32_t> getters;std::vector<std::uintptr_t> freed;
 std::vector<std::uint64_t> dispatches{0};unsigned calls=0,deletes=0;bool heap_delete=false;
 s::WorkerThreadState* state=nullptr;std::array<std::uintptr_t,2>* initialized=nullptr;
 s::IocpWorkerFunction* expected=nullptr;std::function<void(std::string_view)> hook;
 std::function<std::uint64_t(s::WorkerThreadState&,s::WorkerRunResult&)> dispatcher;
 void event(std::string_view name){trace.emplace_back(name);if(hook)hook(name);}
 s::WorkerRunEnvironment environment(){return {index,generic};}
 std::uint64_t run(s::WorkerRunResult& r){return s::run_worker_0298b0(scheduler,r,environment(),api,errors,effects);}
};
void* Api::tls_get(std::uint32_t index){f.getters.push_back(index);auto* value=f.tls;f.event("get");return value;}
std::int32_t Api::tls_set(std::uint32_t index,void* value){f.setters.emplace_back(index,value);f.tls=value;f.event("set");return f.set_result;}
std::int32_t Api::post_stop(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* operation){check(port==f.scheduler.completion.port && !bytes && !key && !operation,"stop posts live port with zero bytes/key and null OVERLAPPED");f.event("post");return f.boolean;}
std::uint32_t Api::get_last_error(){auto value=f.status;f.event("error");return value;}
s::Category Errors::system_category_028620(){f.event("category");return {0x9922,laureondo::server::ErrorCategory::system};}
std::uint32_t Effects::exchange_stop_34(s::IocpSchedulerEntry& scheduler,std::uint32_t value){check(&scheduler==&f.scheduler && value==1,"stop exact target and exchange1");auto old=scheduler.high_34;scheduler.high_34=value;f.event("stop");return old;}
std::uint32_t Effects::exchange_wake_38(s::IocpSchedulerEntry& scheduler,std::uint32_t value){check(&scheduler==&f.scheduler && value==1,"wake exact target and low32 exchange1");auto old=static_cast<std::uint32_t>(scheduler.word_38);scheduler.word_38=(scheduler.word_38&0xffffffff00000000ULL)|value;f.event("wake");return old;}
std::uint64_t Effects::dispatch_027c10(s::IocpSchedulerEntry& scheduler,std::uint32_t timeout,s::WorkerThreadState& state,s::WorkerRunResult& result){check(&scheduler==&f.scheduler && timeout==0xffffffffU,"dispatch exact scheduler and infinite timeout");auto* context=static_cast<s::WorkerTlsContext*>(f.tls);check(context && context->state==&state && context->port_key==&scheduler.completion.port,"TLS context contains address of live port and exact state identity");check(f.initialized==&state.context_58,"same initialized CRT pair passed in dispatcher state");f.state=&state;f.event("dispatch");++f.calls;if(f.dispatcher)return f.dispatcher(state,result);return f.dispatches.at(f.calls-1);}
void Effects::initialize_04a240(std::array<std::uintptr_t,2>& context){check(context==std::array<std::uintptr_t,2>{0,0},"CRT pair zero before initialize");f.initialized=&context;f.event("initialize");context={0x1111,0x2222};}
void Effects::finish_04a2cc(std::array<std::uintptr_t,2>& context){check(&context==f.initialized && context==std::array<std::uintptr_t,2>{0x1111,0x2222},"finish receives same initialized pair after cache cleanup");f.event("finish");}
void Effects::free_cache_04b788(std::uintptr_t pointer){f.freed.push_back(pointer);f.event("free");}
void Effects::delete_function_04b0f8(s::IocpWorkerFunction& function,std::uint32_t bytes){check(&function==f.expected && bytes==32 && function.table_identity==0x1800844b8ULL,"delete observes base table before exact32-byte free");++f.deletes;f.event("delete");if(f.heap_delete){delete &function;f.expected=nullptr;}}
[[noreturn]]void Effects::raise_028240(s::Error error,std::uintptr_t message){f.event("raise");throw Raised{error,message};}
void stopping(){
 for(auto stop:{0U,1U,0x80000000U,0xffffffffU})for(auto wake:{0U,1U,0x80000000U,0xffffffffU})for(auto boolean:{0,1,-1}){
  Fixture f;f.scheduler.high_34=stop;f.scheduler.word_38=0xfedcba9800000000ULL|wake;f.boolean=boolean;f.scheduler.completion.port=123;
  f.hook=[&](auto name){if(name=="wake")f.scheduler.completion.port=0x8877665544332211ULL;};
  s::stop_worker_02a190(f.scheduler,f.api,f.errors,f.effects);
  std::vector<std::string> want{"stop"};if(!stop){want.push_back("wake");if(!wake){want.push_back("post");if(!boolean)want.insert(want.end(),{"error","category"});}}
  check(f.trace==want,"stop early returns and error0 category ordering");check(f.scheduler.high_34==1 && f.scheduler.word_38==(0xfedcba9800000000ULL|(stop?wake:1U)),"only tested low32 flags change; upper wake word survives");
 }
 for(auto status:{1U,0x7fffffffU,0x80000000U,0xffffffffU}){Fixture f;f.status=status;f.hook=[&](auto name){if(name=="error" || name=="category")f.status=0;};bool caught=false;try{s::stop_worker_02a190(f.scheduler,f.api,f.errors,f.effects);}catch(const Raised& e){caught=true;check(std::bit_cast<std::uint32_t>(e.error.value)==status && e.error.category.name_identity==0x9922 && e.message==0x180084530ULL,"stop caches full status before category and passes exact opaque message");}check(caught && f.scheduler.high_34==1 && f.scheduler.word_38==1 && f.trace==std::vector<std::string>{"stop","wake","post","error","category","raise"},"raised stop failure never rolls flags back");}
}
void idle(){
 for(auto boolean:{0,1,-1}){Fixture f;f.boolean=boolean;f.scheduler.work_count_30=0;f.scheduler.high_34=0;f.scheduler.word_38=0xaabbccdd00000000ULL;s::WorkerRunResult result{{s::Error{72,{0xdead}}}};f.hook=[&](auto){check(result.error && result.error->value==72,"idle writes result only after stop completes");};check(!f.run(result) && result.error && result.error->value==0 && result.error->category.name_identity==f.generic.name_identity && result.error->category.logical==f.generic.logical && f.scheduler.work_count_30==0,"idle zero count and logical generic success without changing work");check(f.getters.empty() && !f.initialized,"idle skips state/TLS/dispatch/CRT cleanup entirely");}
 Fixture f;f.status=5;s::WorkerRunResult result;try{f.run(result);check(false,"idle raised stop must propagate");}catch(const Raised&){check(!result.error,"no successful logical error initialized after raised stop");}
}
void active(){
 for(auto work:{1U,0x80000000U,0xffffffffU})for(auto set_result:{0,-1})for(auto handled:{1ULL,0x100000000ULL,0x8000000000000000ULL,0xffffffffffffffffULL}){
  Fixture f;f.scheduler.work_count_30=work;f.set_result=set_result;f.dispatches={handled,handled,0};auto* prior=f.tls;s::WorkerRunResult result;
  f.hook=[&](auto name){if(name=="get")f.index=17;if(name=="dispatch"){check(f.state->pending_50==0 && f.state->cache_00==std::array<std::uintptr_t,10>{},"active state pending and all10cache slots zero before dispatch");check(static_cast<s::WorkerTlsContext*>(f.tls)->previous==prior,"TLS frame stores prior getter result");f.index=29;}};
  check(f.run(result)==2 && f.calls==3 && !result.error,"any nonzero64-bit dispatch increments once and active result remains unwritten");check(f.scheduler.work_count_30==work && !f.scheduler.high_34,"active fetch-add0 preserves full unsigned low32 work count and skips stop");check(f.getters==std::vector<std::uint32_t>{0xfedcba98U} && f.setters.size()==2 && f.setters[0].first==17 && f.setters[1]==std::pair<std::uint32_t,void*>{29,prior},"each TLS API reloads live index; setter failure ignored and previous restored");check(f.trace==std::vector<std::string>{"initialize","get","set","dispatch","dispatch","dispatch","set","finish"},"active exact state/TLS/loop/restore/cleanup ordering");
 }
 Fixture f;f.scheduler.work_count_30=1;s::WorkerRunResult result;f.dispatcher=[&](auto& state,auto& r){check(state.cache_00==std::array<std::uintptr_t,10>{},"cache initially zero");for(std::size_t i=0;i<10;++i)state.cache_00[i]=100+i;r.error=s::Error{-77,{0x3344}};return 0;};f.hook=[&](auto name){if(name=="free" && f.freed.size()==1){f.state->cache_00[1]=0;f.state->cache_00[2]=0xffffffffffffffffULL;}if(name=="finish")check(f.freed.size()==9 && f.trace[f.trace.size()-2]=="free","finish after ascending cleanup slots including last slot");};check(!f.run(result) && result.error && result.error->value==-77 && result.error->category.name_identity==0x3344,"zero first dispatcher returns0 and preserves dispatcher-written error");check(f.freed==std::vector<std::uintptr_t>{100,0xffffffffffffffffULL,103,104,105,106,107,108,109},"ascending LIVE slot reload observes earlier free mutations and skips new null");
 check(s::increment_worker_count(0)==1 && s::increment_worker_count(0xffffffffULL)==0x100000000ULL && s::increment_worker_count(0xfffffffffffffffeULL)==0xffffffffffffffffULL && s::increment_worker_count(0xffffffffffffffffULL)==0xffffffffffffffffULL,"uint64 count saturation boundary independent of impractical iteration count");
 Fixture changed;changed.scheduler.work_count_30=1;s::WorkerRunResult changed_result;auto* replacement=reinterpret_cast<void*>(0xfedcba9876543210ULL);changed.dispatcher=[&](auto&,auto&){static_cast<s::WorkerTlsContext*>(changed.tls)->previous=replacement;return 0;};check(!changed.run(changed_result) && changed.tls==replacement,"restore reloads LIVE TLS frame previous slot changed by dispatcher");
 Fixture restored;restored.scheduler.work_count_30=1;s::WorkerRunResult restored_result;restored.dispatcher=[](auto& state,auto&){state.cache_00[0]=100;return 0;};restored.hook=[&](auto name){if(name=="set" && restored.setters.size()==2){restored.state->cache_00[0]=0;restored.state->cache_00[8]=333;restored.state->cache_00[9]=333;}};check(!restored.run(restored_result) && restored.freed==std::vector<std::uintptr_t>{333,333},"cache slots loaded after TLS restore callback; repeated live pointer is freed twice without invented dedup/reset");
}
void exceptions(){
 for(auto stop:{"initialize","get","set","dispatch","free","finish"}){Fixture f;f.scheduler.work_count_30=1;f.dispatcher=[](auto& state,auto&){state.cache_00[0]=123;return 0;};f.hook=[&](auto name){if(name==stop)throw std::runtime_error("selected callback interruption");};s::WorkerRunResult result;bool caught=false;try{f.run(result);}catch(const std::runtime_error&){caught=true;}check(caught && f.trace.back()==stop,"provider throw propagates without invented restoration/free/finish/unwind");}
}
void nested(){
 Fixture outer,inner;outer.scheduler.work_count_30=1;inner.scheduler.work_count_30=1;s::WorkerRunResult result;
 outer.dispatcher=[&](auto& state,auto&){auto* frame=outer.tls;inner.tls=frame;s::WorkerRunResult r;check(!inner.run(r) && inner.tls==frame,"nested run restores outer exact TLS frame identity");check(static_cast<s::WorkerTlsContext*>(frame)->state==&state,"nested return leaves outer frame and state live");return 0;};auto* prior=outer.tls;check(!outer.run(result) && outer.tls==prior,"outer frame restores original prior after controlled nested dispatcher");
}
void slots(){
 for(auto flags:{0U,1U,2U,3U,0x80000000U,0x80000001U,0xffffffffU}){
  Fixture f;s::IocpWorkerFunction function{0xffff,17,29,&f.scheduler};f.expected=&function;auto identity=reinterpret_cast<std::uintptr_t>(&function);check(s::delete_worker_function_026320(function,flags,f.effects)==identity && function.table_identity==0x1800844b8ULL && f.deletes==(flags&1U),"scalar delete tests bit0 only and returns original identity");check(function.event_08==17 && function.event_10==29 && function.scheduler_18==&f.scheduler,"delete body adds no events/scheduler destruction");
 }
 Fixture f;f.expected=new s::IocpWorkerFunction{0xffff,17,29,&f.scheduler};f.heap_delete=true;auto identity=reinterpret_cast<std::uintptr_t>(f.expected);check(s::delete_worker_function_026320(*f.expected,1,f.effects)==identity && !f.expected,"return identity after actual heap delete without postdelete access");
 Fixture g;s::IocpWorkerFunction function{1,2,3,&g.scheduler};g.boolean=1;s::run_worker_function_029890(function,g.environment(),g.api,g.errors,g.effects);check(g.scheduler.high_34==1 && function.table_identity==1 && g.trace==std::vector<std::string>{"stop","wake","post"},"run slot calls actual idle run/stop while discarding logical result");
 function.scheduler_18=nullptr;auto old=g.trace;bool caught=false;try{s::run_worker_function_029890(function,g.environment(),g.api,g.errors,g.effects);}catch(const s::UnsupportedInput&){caught=true;}check(caught && g.trace==old,"null scheduler rejects unsupported native dereference without callbacks");
}
}
int main(){try{stopping();idle();active();exceptions();nested();slots();std::cout<<"PASS: "<<checks<<" socket worker run checks; retained026320/029890/0298b0/02a190 logical status, low32 effects, TLS ordering, dispatch saturation, live cache cleanup and lifetime\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
