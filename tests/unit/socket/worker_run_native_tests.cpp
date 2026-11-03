// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_worker_run_native.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <vector>
namespace s=preloader_l::socket;namespace t=preloader_l::threads;
namespace {
unsigned checks=0;void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected native run integration dependency");}
struct SDK{unsigned calls=0,posts=0;DWORD index=0,bytes=0,error=0;void* value=nullptr;HANDLE port=nullptr;ULONG_PTR key=0;LPOVERLAPPED operation=nullptr;BOOL boolean=0;};SDK* sdk=nullptr;
LPVOID WINAPI get_tls(DWORD index){check(sdk!=nullptr,"synthetic TLS fixture bound");++sdk->calls;sdk->index=index;return sdk->value;}
BOOL WINAPI set_tls(DWORD index,LPVOID value){check(sdk!=nullptr,"synthetic TLS fixture bound");++sdk->calls;sdk->index=index;sdk->value=value;return sdk->boolean;}
BOOL WINAPI post(HANDLE port,DWORD bytes,ULONG_PTR key,LPOVERLAPPED operation){check(sdk!=nullptr,"synthetic completion fixture bound");++sdk->calls;++sdk->posts;sdk->port=port;sdk->bytes=bytes;sdk->key=key;sdk->operation=operation;return sdk->boolean;}
DWORD WINAPI get_error(){check(sdk!=nullptr,"synthetic error fixture bound");++sdk->calls;return sdk->error;}
s::WorkerRunImports imports(){s::WorkerRunImports i;i.tls_get=get_tls;i.tls_set=set_tls;i.post_stop=post;i.get_last_error=get_error;return i;}
void wrappers(){
 {s::NativeWorkerRunApi api;check(!sdk,"default constructor has no SDK side effects");}
 SDK f;sdk=&f;s::NativeWorkerRunApi api(imports());check(!f.calls,"injected constructor has no SDK side effects");OVERLAPPED operation{};
 for(auto index:{0U,1U,0x80000000U,0xffffffffU})for(auto pointer:{std::uintptr_t{0},std::uintptr_t{0x8877665544332211ULL},std::uintptr_t{0xffffffffffffffffULL}}){f.value=reinterpret_cast<void*>(pointer);check(api.tls_get(index)==f.value && f.index==index,"TlsGetValue preserves index and fullwidth pointer result");for(auto boolean:{0,1,-1,std::numeric_limits<int>::min()}){f.boolean=boolean;check(api.tls_set(index,reinterpret_cast<void*>(pointer))==boolean && f.index==index && f.value==reinterpret_cast<void*>(pointer),"TlsSetValue preserves DWORD/fullwidth pointer/raw BOOL");}}
 for(auto port:{std::uintptr_t{0},std::uintptr_t{0xffffffffffffffffULL},std::uintptr_t{0x8877665544332211ULL}})for(auto bytes:{0U,1U,0x80000000U,0xffffffffU})for(auto key:{std::uintptr_t{0},std::uintptr_t{0xfedcba9876543210ULL}})for(auto* op:{static_cast<OVERLAPPED*>(nullptr),&operation})for(auto boolean:{0,1,-1,std::numeric_limits<int>::min()}){f.boolean=boolean;check(api.post_stop(port,bytes,key,op)==boolean && f.port==reinterpret_cast<HANDLE>(port) && f.bytes==bytes && f.key==key && f.operation==op,"PostQueuedCompletionStatus preserves all SDK argument widths and pointer identity");}
 for(auto error:{0U,1U,0x80000000U,0xffffffffU}){f.error=error;check(api.get_last_error()==error,"GetLastError retains exact unsigned32bits");}
 check(f.calls==256,"four injected wrappers exact call count");sdk=nullptr;
}
struct Gate:s::SocketGate{void enter()override{unused();}void leave()override{unused();}};
struct Entry:s::IocpSchedulerEntry{using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unused();}};
struct Errors:s::Api{
 unsigned categories=0;s::Category system_category_028620()override{++categories;return {0x9922};}
 void set_last_error(std::int32_t)override{unused();}std::int32_t get_last_error()override{unused();}
 s::Conversion convert_acp(const char*,std::int32_t)override{unused();}s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unused();}std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Startup:s::WorkerStartupApi{
 unsigned creates=0,waits=0,closes=0,signals=0,sleeps=0;std::vector<std::string>& trace;explicit Startup(std::vector<std::string>& trace):trace(trace){}
 std::uintptr_t create_event(std::uintptr_t sec,std::int32_t manual,std::int32_t initial,std::uintptr_t name)override{check(!sec && manual==1 && !initial && !name,"composed startup event arguments");trace.push_back("create");return ++creates==1?0x1111:0x2222;}
 std::uint32_t get_last_error()override{unused();}
 std::int32_t close_handle(std::uintptr_t handle)override{check(handle==0x1111,"startup closes cached entry only");++closes;trace.push_back("close");return 0;}
 std::uint32_t wait_one(std::uintptr_t handle,std::uint32_t timeout)override{check(handle==0x1111 && timeout==0xffffffffU,"startup controlled infinite wait");++waits;trace.push_back("wait");return 0xffffffffU;}
 std::int32_t set_event(std::uintptr_t handle)override{check(handle==(signals?0x2222:0x1111),"entry signals actual entry then cached exit after delete");++signals;trace.push_back("signal");return 0;}
 std::uint32_t sleep_ex(std::uint32_t ms,std::int32_t alertable)override{check(ms==0xffffffffU && alertable==1,"entry controlled sleep arguments");++sleeps;trace.push_back("sleep");return 0xc0;}
};
struct Threads:t::Effects{
 t::Context context{},*slot=nullptr;unsigned creates=0,frees=0;std::uint32_t stack=0;std::function<std::uint32_t(std::uintptr_t,std::uintptr_t)> route;
 t::Context* calloc_context_066848()override{context={};return &context;}
 void free_0668c0(t::Context* p)override{check(!p,"unchanged context constructor calls free(null)");++frees;}
 bool get_module_handle_ex_w(std::uint32_t flags,std::uintptr_t address,t::Handle* module)override{check(flags==4 && address==0x18002a4a0ULL && module==&context.module,"unchanged CRT treats original entry VA as opaque module lookup argument");*module=0x7766;return false;}
 t::Handle create_thread(void* security,std::uint64_t requested,t::Context* p,std::uint32_t flags,std::uint32_t* id)override{check(!security && requested==stack && p==&context && !flags && id,"unchanged recovered thread bridge arguments");++creates;*id=91;return 0x3333;}
 std::uint32_t last_error()override{unused();}void write_error_slot_05d724(std::uint32_t)override{unused();}void invalid_parameter_05c954()override{unused();}void map_os_error_05d698(std::uint32_t)override{unused();}bool close_handle(t::Handle)override{unused();}bool free_library(t::Handle)override{unused();}
 t::Context*& required_thread_slot_064b90()override{return slot;}t::Context* optional_thread_context_064d08()override{return slot;}int runtime_policy_06c0d0()override{unused();}int initialize_runtime_067320(int)override{unused();}void uninitialize_runtime_06736c()override{unused();}
 std::uint32_t invoke_entry(std::uintptr_t entry,std::uintptr_t argument)override{return route(entry,argument);}
 [[noreturn]]void free_library_and_exit_thread(t::Handle,std::uint32_t)override{unused();}[[noreturn]]void exit_thread(std::uint32_t)override{unused();}
};
struct Effects:s::WorkerRunEffects{
 s::IocpSchedulerEntry& scheduler;SDK& sdk;std::vector<std::string>& trace;s::IocpWorkerFunction* function=nullptr;unsigned dispatches=0,deletes=0;bool active=false;
 Effects(s::IocpSchedulerEntry& scheduler,SDK& sdk,std::vector<std::string>& trace):scheduler(scheduler),sdk(sdk),trace(trace){}
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry& value,std::uint32_t next)override{check(&value==&scheduler && next==1,"composed run actual stop exchange");auto prior=value.high_34;value.high_34=next;trace.push_back("stop");return prior;}
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry& value,std::uint32_t next)override{check(&value==&scheduler && next==1,"composed run actual wake exchange");auto prior=static_cast<std::uint32_t>(value.word_38);value.word_38=(value.word_38&0xffffffff00000000ULL)|next;trace.push_back("wake");return prior;}
 std::uint64_t dispatch_027c10(s::IocpSchedulerEntry& value,std::uint32_t timeout,s::WorkerThreadState& state,s::WorkerRunResult& result)override{check(active && &value==&scheduler && timeout==0xffffffffU,"required controlled dispatcher sees recovered run target");auto* frame=static_cast<s::WorkerTlsContext*>(sdk.value);check(frame && frame->port_key==&value.completion.port && frame->state==&state && !state.pending_50,"same live TLS context and pending state used by actual run");trace.push_back("dispatch");++dispatches;if(dispatches==1){state.cache_00[9]=0x9876;result.error=s::Error{88,{0x5555}};return 0x8000000000000000ULL;}return 0;}
 void initialize_04a240(std::array<std::uintptr_t,2>& pair)override{check(pair==std::array<std::uintptr_t,2>{},"composed CRT pair zero before required provider");trace.push_back("initialize");pair={11,12};}
 void finish_04a2cc(std::array<std::uintptr_t,2>& pair)override{check(pair==std::array<std::uintptr_t,2>{11,12},"composed finish after exact same CRT pair");trace.push_back("finish");}
 void free_cache_04b788(std::uintptr_t pointer)override{check(pointer==0x9876,"composed cache exact slot9 pointer");trace.push_back("free-cache");}
 void delete_function_04b0f8(s::IocpWorkerFunction& value,std::uint32_t bytes)override{check(&value==function && value.table_identity==0x1800844b8ULL && bytes==32,"actual scalar delete base table before native-size free");trace.push_back("delete");++deletes;delete function;function=nullptr;}
 [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unused();}
};
struct Workers:s::WorkerFunctionEffects{
 s::IocpWorker worker;Effects& effects;
 Workers(Startup& startup,Errors& errors,Threads& threads,s::WorkerRunEnvironment environment,s::WorkerRunApi& api,Effects& effects):WorkerFunctionEffects(startup,errors,threads,environment,api,effects),effects(effects){}
 s::IocpWorker* allocate_worker_04b0bc(std::uint32_t bytes)override{check(bytes==24,"actual024990 requests native24 worker bytes");return &worker;}
 s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t bytes)override{check(bytes==32,"actual024990 requests native32 function bytes");effects.function=new s::IocpWorkerFunction;return effects.function;}
 void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override{unused();}[[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unused();}
};
struct Constructor:s::IocpConstructorApi{
 std::uint64_t set_version_mask(std::uint64_t mask,std::uint32_t type,std::uint8_t condition)override{check(!mask && type==2 && condition==3,"actual constructor version request");return 17;}
 std::int32_t verify_version(OSVERSIONINFOEXA&,std::uint32_t,std::uint64_t)override{return 1;}
 std::uintptr_t create_completion_port(std::uintptr_t file,std::uintptr_t existing,std::uintptr_t key,std::uint32_t threads)override{check(file==0xffffffffffffffffULL && !existing && !key && !threads,"actual constructor creates IOCP token with observed parameters");return 0x8877665544332211ULL;}
 std::uint32_t get_last_error()override{unused();}std::int32_t close_handle(std::uintptr_t)override{unused();}
};
struct ServiceGate:s::ServiceGateApi{
 std::int32_t initialize(CRITICAL_SECTION&,std::uint32_t spin)override{check(spin==0x80000000U,"actual constructor gate request");return 1;}
 std::uint32_t get_last_error()override{unused();}
};
void composition(){
 for(bool active:{false,true}){SDK f;sdk=&f;f.boolean=1;f.value=reinterpret_cast<void*>(std::uintptr_t{0xdeadbeef});auto* prior=f.value;s::NativeWorkerRunApi api(imports());Gate gate;CRITICAL_SECTION section{};Entry scheduler(gate,section);std::vector<std::string> trace;Startup startup(trace);Errors errors;Threads threads;Effects effects(scheduler,f,trace);effects.active=active;std::uint32_t index=77;Workers workers(startup,errors,threads,{index,{0x7788}},api,effects);Constructor constructor;ServiceGate gates;
  check(&s::construct_iocp_024990(scheduler,42,0,1,constructor,gates,errors,workers)==&scheduler,"actual024990 returns same scheduler using real worker startup source");
  check(scheduler.worker_a8==&workers.worker && workers.worker.handle_08==0x3333 && workers.worker.event_10==0x2222 && startup.creates==2 && startup.waits==1 && startup.closes==1 && threads.creates==1 && threads.frees==1,"actual024990/02a000/05e8e0 handshake shares worker context and live handles");
  auto* function=effects.function;check(function && function->scheduler_18==&scheduler && function->table_identity==0x180084710ULL && threads.context.entry==0x18002a4a0ULL && threads.context.argument==reinterpret_cast<std::uintptr_t>(function),"constructor binds real scheduler and opaque source entry token to recovered thread argument");
  if(!active)scheduler.work_count_30=0;const auto work=scheduler.work_count_30.load();scheduler.word_38=0xfedcba9800000000ULL;
  threads.route=[&](auto entry,auto argument){check(entry==0x18002a4a0ULL && argument==reinterpret_cast<std::uintptr_t>(function),"manual synthetic dispatch routes tokens to source without executing an image address");return s::worker_entry_02a4a0(*reinterpret_cast<s::IocpWorkerFunction*>(argument),startup,workers);};
  check(!threads.invoke_entry(threads.context.entry,threads.context.argument) && !effects.function && effects.deletes==1 && startup.signals==2 && startup.sleeps==1,"actual02a4a0 invokes actual029890/run and026320 including safe heap lifetime end");
  check(scheduler.work_count_30==work && f.value==prior && errors.categories==1,"run slot preserves work and prior TLS; only earlier successful gate constructor requests category");
  std::vector<std::string> expected{"create","create","wait","close","signal"};if(active)expected.insert(expected.end(),{"initialize","dispatch","dispatch","free-cache","finish"});else expected.insert(expected.end(),{"stop","wake"});expected.insert(expected.end(),{"delete","signal","sleep"});check(trace==expected,"composed recovered chain exact callback ordering through cache cleanup and deletion before exit signal");
  check(effects.dispatches==(active?2U:0U) && f.posts==(active?0U:1U) && scheduler.word_38==(active?0xfedcba9800000000ULL:0xfedcba9800000001ULL),"active controlled dispatcher and idle actual stop remain distinct paths and preserve wake high32");if(!active)check(f.port==reinterpret_cast<HANDLE>(scheduler.completion.port) && !f.bytes && !f.key && !f.operation,"actual stop uses injected SDK completion function with live created IOCP identity");sdk=nullptr;
 }
}
}
int main(){try{wrappers();composition();std::cout<<"PASS: "<<checks<<" socket worker run native checks; four injected SDK imports and actual024990/02a000/05e8e0/02a4a0/029890/0298b0/02a190/026320 source composition with controlled dispatcher and lifetime\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
