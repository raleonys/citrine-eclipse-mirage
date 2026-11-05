#include "io_completion.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace s=anadius::socket_recovered;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unexpected(){throw std::logic_error("Unexpected synthetic IO dependency");}
template<class F>void rejects(F action){bool caught=false;try{action();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"invalid bounded projection rejected");}
template<class F>void throws(F action){bool caught=false;try{action();}catch(const std::runtime_error&){caught=true;}check(caught,"provider exception propagates");}
struct Worker:s::WorkerRunApi{
 std::vector<std::uint32_t> indices;std::function<void*()> get;
 void* tls_get(std::uint32_t index)override{indices.push_back(index);return get?get():nullptr;}
 std::int32_t tls_set(std::uint32_t,void*)override{unexpected();}
 std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{unexpected();}
 std::uint32_t get_last_error()override{unexpected();}
};
struct Memory:s::PostedMemoryEffects{
 std::vector<std::uintptr_t> frees;std::function<void(s::OperationStorage&)> free;
 s::OperationStorage& allocate_04b0bc(std::uint64_t)override{unexpected();}
 s::PostedOperation& project_posted(s::OperationStorage&)override{unexpected();}
 void free_04b788(s::OperationStorage& storage)override{frees.push_back(storage.identity);if(free)free(storage);}
};
struct Errors:s::Api{
 unsigned categories=0;std::function<void()> category;
 s::Category system_category_028620()override{++categories;if(category)category();return {0x2222};}
 void set_last_error(std::int32_t)override{unexpected();}std::int32_t get_last_error()override{unexpected();}
 s::Conversion convert_acp(const char*,std::int32_t)override{unexpected();}
 s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unexpected();}
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unexpected();}
 std::int32_t select_write(s::Socket,std::int32_t)override{unexpected();}
};
struct Table:s::IoHandlerVtable{
 explicit Table(std::uintptr_t identity):IoHandlerVtable(identity){}
 unsigned destroys=0,moves=0,queries=0;
 std::function<void(s::IoHandlerEnvironment&)> destroy,query;
 std::function<void(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&)> move;
 std::function<bool(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,const s::IoHandlerSentinels&)> whole_move;
 std::function<bool(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,const s::IoHandlerSentinels&,s::IoEnvironmentPrepareBody&)> whole_prepare;
 void destroy_slot_00(s::IoHandlerEnvironment& env)override{++destroys;if(destroy)destroy(env);}
 void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;if(move)move(to,from);else{to.payload_00=from.payload_00;to.state_18=from.state_18;}}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment& env)override{++queries;if(query)query(env);return 0xabc;}
 bool try_move_environment_023cb0(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from,const s::IoHandlerSentinels& sentinels)override{return whole_move?whole_move(to,from,sentinels):false;}
 bool try_prepare_environment_40(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from,const s::IoHandlerSentinels& sentinels,s::IoEnvironmentPrepareBody& body)override{return whole_prepare?whole_prepare(to,from,sentinels,body):false;}
};
struct Ref:s::IoReference{
 unsigned disposes=0,deletes=0;std::function<void()> dispose,erase;
 void dispose_slot_00()override{++disposes;if(dispose)dispose();}
 void delete_slot_08()override{++deletes;if(erase)erase();}
};
struct Effects:s::IoCompletionEffects{
 unsigned receives=0,sends=0,receive_schedules=0,send_schedules=0,misc=0;
 std::function<void(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)> receive;
 std::function<void(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)> send;
 std::function<void(s::IoHandlerEnvironment&,s::ReceiveHandler&)> schedule_receive;
 std::function<void(s::IoHandlerEnvironment&,s::SendHandler&)> schedule_send;
 std::function<void()> misc_effect;
 s::Category misc_category_0285b0()override{++misc;if(misc_effect)misc_effect();return {0x3333};}
 void receive_body_025c40(s::ReceiveHandler& handler,s::Error& error,std::uint64_t bytes,std::uint32_t flag)override{++receives;if(receive)receive(handler,error,bytes,flag);}
 void send_body_0261d0(s::SendHandler& handler,s::Error& error,std::uint64_t bytes,std::uint32_t flag)override{++sends;if(send)send(handler,error,bytes,flag);}
 void schedule_receive_022c60(s::IoHandlerEnvironment& env,s::ReceiveHandler& handler)override{++receive_schedules;if(schedule_receive)schedule_receive(env,handler);}
 void schedule_send_0229d0(s::IoHandlerEnvironment& env,s::SendHandler& handler)override{++send_schedules;if(schedule_send)schedule_send(env,handler);}
};
struct Fixture{
 std::array<std::byte,1025> bytes{},other_bytes{};s::OperationStorage storage{0x10000,bytes},other{0x20000,other_bytes};
 s::ReceiveIoOperation receive{storage,{},0,nullptr,0,{},{} };s::SendIoOperation send{storage,{},nullptr,{},{} };
 std::uint32_t index=17;Worker worker;Memory memory;Errors errors;Effects effects;
 Table original{0x111},empty{0x777},temporary{0x888},alternate{0x999};s::IoHandlerExecutor executor{},sentinel_executor{},changed_executor{};
 s::WorkerThreadState state{};s::WorkerTlsContext context{nullptr,&state,nullptr};
 Fixture(){bytes.fill(std::byte{0xa5});other_bytes.fill(std::byte{0x5a});receive.environment_b0={ {11,12},&original,0,0x456,&executor};send.environment_a0=receive.environment_b0;receive.handler.words_60_78={1,2,3,4};receive.handler.byte_80=0x81;receive.handler.dword_84=0x12345678;receive.handler.words_88_98={5,6,7};receive.handler.object_a0=0x1234;send.handler.words_60_78={8,9,10,11};send.handler.dword_80=0x87654321;send.handler.word_88=12;send.handler.object_90=0x9876;}
 s::IoCompletionEnvironment env(){return {{index,{}},{&empty,0x846e8,&sentinel_executor,0x777}};}
};
bool same_prepare_fields(const s::IoHandlerEnvironment& a,const s::IoHandlerEnvironment& b){return a.payload_00==b.payload_00&&a.operations_10==b.operations_10&&a.state_18==b.state_18&&a.schedule_20==b.schedule_20&&a.executor_28==b.executor_28;}
struct PrepareObservation {Fixture& fixture;s::IoHandlerEnvironment* temporary=nullptr;unsigned factories=0;bool fail_factory=false;};
PrepareObservation* preparation_observation=nullptr;
void prepare_caller_factory(s::IoHandlerEnvironment& out,std::uintptr_t query,s::IoPrepareScratch&){auto& p=*preparation_observation;++p.factories;check(query==0xabc&&(!p.temporary||p.temporary==&out),"Actual cached40 constructs the helper's one real temporary after exact query");p.temporary=&out;out={{77,78},&p.fixture.temporary,55,0x7788,&p.fixture.changed_executor};if(p.fail_factory)throw std::runtime_error("Unknown factory40 outcome");}
struct PrepareOperations final:s::IoEnvironmentPrepareOperations {
 bool& held;unsigned queries=0,factories=0,moves=0,destroys=0;
 explicit PrepareOperations(bool& h):held(h){}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment& source)override{check(held,"Private prepare query remains inside the owning caller");++queries;return source.operations_10->query_slot_18(source);}
 void construct_slot_40(s::IoPrepareFunction cached,s::IoHandlerEnvironment& out,std::uintptr_t query,s::IoPrepareScratch& scratch)override{check(held&&cached==&prepare_caller_factory,"Private prepare bridge receives the exact function cached before query deletes executor");++factories;cached(out,query,scratch);}
 s::IoHandlerEnvironment& move_023cb0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,s::IoHandlerSentinels sentinels)override{check(held,"Private prepare move retains whole caller ownership");++moves;return s::move_io_environment_023cb0(out,from,sentinels);}
 void destroy_slot_00(s::IoHandlerEnvironment& value)override{check(held,"Private prepare guard destruction precedes owning caller completion");++destroys;value.operations_10->destroy_slot_00(value);}
};
void preparation_caller_boundaries(){
 for(unsigned mode=0;mode<3;++mode){
  Fixture f;PrepareObservation p{f};preparation_observation=&p;auto executor=std::make_unique<s::IoHandlerExecutor>(s::IoHandlerExecutor{&prepare_caller_factory});auto source=f.receive.environment_b0;source.executor_28=executor.get();s::IoHandlerEnvironment out{{99,98},&f.alternate,19,0x123,&f.sentinel_executor};const auto before_source=source,before_out=out;bool held=false;unsigned entries=0;PrepareOperations operations(held);
  f.original.query=[&](auto& value){if(executor){check(value.executor_28==executor.get(),"Prepare query sees its live original executor before deletion");executor.reset();value.executor_28=nullptr;}};
  f.temporary.move=[&](auto& destination,auto& from){destination.payload_00=from.payload_00;destination.state_18=from.state_18;from.operations_10=&f.alternate;from.executor_28=&f.executor;};
  f.alternate.destroy=[&](auto& value){check(!value.state_18&&value.executor_28==&f.sentinel_executor&&out.payload_00==std::array<std::uintptr_t,2>{77,78},"Prepare guard reads the LIVE post-move temporary target after native state/executor tail");};
  f.original.whole_prepare=[&](auto& destination,auto& from,const auto&,auto& body){++entries;check(same_prepare_fields(out,before_out)&&same_prepare_fields(source,before_source)&&&destination==&out&&&from==&source,"Prepare owning hook precedes source executor/query effects and destination writes");p.temporary=&body.temporary();check(p.temporary!=&out&&p.temporary!=&source,"Preparation body exposes only its distinct actual temporary address before admission");if(mode==0)return false;held=true;check(&body.run(&operations)==&out&&operations.queries==1&&operations.factories==1&&operations.moves==1&&operations.destroys==1,"One admitted prepare body includes factory/move and live guard destruction before returning");held=false;if(mode==2){f.executor.prepare_slot_40=&prepare_caller_factory;source.executor_28=&f.executor;return false;}return true;};
  if(mode==2)rejects([&]{s::prepare_io_environment_40(out,source,f.env().sentinels);});else check(&s::prepare_io_environment_40(out,source,f.env().sentinels)==&out,"Prepare helper returns its exact caller-owned output");
  check(entries==1&&!held&&!executor&&p.factories==1&&f.original.queries==1&&f.temporary.moves==1&&f.alternate.destroys==1&&out.state_18==55&&out.executor_28==&f.executor,"Fallback, handled body and malformed run-then-false each execute at most one native query/factory/move/guard sequence with no replay");
 }
 struct Refusal final:s::IoEnvironmentPreparation {unsigned entries=0;s::IoHandlerEnvironment& prepare(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,const s::IoHandlerSentinels&,s::IoEnvironmentPrepareBody&)override{++entries;throw s::UnsupportedInput("Explicit prepare refusal");}} refusal;
 Fixture f;s::IoHandlerEnvironment source,out{{91,92},&f.alternate,31,0x456,&f.changed_executor};const auto before=out;rejects([&]{s::prepare_io_environment_40(out,source,f.env().sentinels,&refusal);});check(refusal.entries==1&&same_prepare_fields(out,before)&&!source.operations_10&&!source.executor_28&&!f.original.queries,"Explicit preparation rejects before any source table/executor read, body attempt or destination write");
 preparation_observation=nullptr;
}
void preparation_unknown_prefixes(){
 for(unsigned phase=0;phase<4;++phase){
  Fixture f;PrepareObservation p{f};preparation_observation=&p;f.executor.prepare_slot_40=&prepare_caller_factory;auto source=f.receive.environment_b0;s::IoHandlerEnvironment out{{99,98},&f.alternate,19,0x123,&f.sentinel_executor};const auto before=out;
  if(phase==0)f.original.query=[](auto&){throw std::runtime_error("Unknown query outcome");};p.fail_factory=phase==1;
  f.temporary.move=[&](auto& destination,auto& from){check(from.operations_10==&f.empty&&from.schedule_20==0x846e8,"Prepare move failure boundary follows the actual native header prefix");if(phase==2)throw std::runtime_error("Unknown move outcome");destination.payload_00=from.payload_00;destination.state_18=from.state_18;from.operations_10=&f.alternate;from.executor_28=&f.executor;};
  f.alternate.destroy=[](auto& value){check(!value.state_18,"Throwing live temporary destroy follows the actual completed move tail");throw std::runtime_error("Unknown destroy outcome");};
  throws([&]{s::prepare_io_environment_40(out,source,f.env().sentinels);});
  check(f.original.queries==1&&p.factories==(phase==0?0U:1U)&&f.temporary.moves==(phase>=2?1U:0U)&&f.alternate.destroys==(phase==3?1U:0U)&&!f.original.destroys&&!f.temporary.destroys,"Unknown query/factory/move/guard results stop at their actual entered prefix without invented source/output cleanup");
  if(phase<2)check(same_prepare_fields(out,before),"Query or factory exception cannot commit the later destination move");
  else if(phase==2)check(out.operations_10==&f.temporary&&out.schedule_20==0x7788&&out.payload_00==before.payload_00&&out.state_18==before.state_18&&out.executor_28==before.executor_28,"Move exception retains header prefix only and never invents normal payload/state/executor tail");
  else check(out.operations_10==&f.temporary&&out.payload_00==std::array<std::uintptr_t,2>{77,78}&&out.state_18==55&&out.executor_28==&f.executor,"Destroy exception retains the completed move output without invented rollback or another destroy");
 }
 preparation_observation=nullptr;
}
void move_environment(){
 Fixture f;s::IoHandlerEnvironment source=f.receive.environment_b0,destination;destination.payload_00={99,98};
 f.original.move=[&](auto& to,auto& from){check(&to==&destination&&&from==&source,"move callback receives actual distinct destination and source");check(to.operations_10==&f.original&&to.schedule_20==0x456,"destination table and scheduler copied before callback");check(from.operations_10==&f.empty&&from.schedule_20==0x846e8&&from.state_18==27,"source table and scheduler reset before callback; state remains live");check(to.payload_00==std::array<std::uintptr_t,2>{99,98},"move helper does not invent payload memcpy");to.payload_00=from.payload_00;to.state_18=43;from.state_18=91;from.executor_28=&f.changed_executor;};
 source.state_18=27;auto& result=s::move_io_environment_023cb0(destination,source,f.env().sentinels);
 check(&result==&destination&&destination.payload_00==std::array<std::uintptr_t,2>{11,12}&&destination.state_18==43,"move callback owns payload and destination state");check(!source.state_18&&source.executor_28==&f.sentinel_executor&&destination.executor_28==&f.changed_executor,"state reset and executor live reload follow callback");
 Fixture broken;auto source2=broken.receive.environment_b0;s::IoHandlerEnvironment dest2;source2.operations_10=nullptr;rejects([&]{s::move_io_environment_023cb0(dest2,source2,broken.env().sentinels);});check(source2.operations_10==&broken.empty&&dest2.operations_10==nullptr&&source2.executor_28==&broken.executor,"failed move leaves native preceding assignments; no rollback");
 Fixture failed;auto from=failed.receive.environment_b0;s::IoHandlerEnvironment to;from.state_18=7;failed.original.move=[](auto&,auto&){throw std::runtime_error("move");};throws([&]{s::move_io_environment_023cb0(to,from,failed.env().sentinels);});check(from.state_18==7&&from.executor_28==&failed.executor&&from.operations_10==&failed.empty,"throwing move does not invent normal tail resets");
}
void owning_move_boundary(){
 struct Slot final:s::IoEnvironmentMoveSlot {
  Table& original;bool& claimed;unsigned calls=0;
  Slot(Table& t,bool& c):original(t),claimed(c){}
  void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{check(claimed,"Private move slot runs only inside its whole-operation claim");++calls;original.move_slot_10(to,from);}
 };
 Fixture f;auto source=f.receive.environment_b0;s::IoHandlerEnvironment destination{{99,98},&f.alternate,55,0x123,&f.sentinel_executor};bool claimed=false;unsigned entries=0;Slot slot(f.original,claimed);
 f.original.whole_move=[&](auto& to,auto& from,const auto& sentinels){
  ++entries;check(!claimed&&&to==&destination&&&from==&source&&from.operations_10==&f.original&&from.schedule_20==0x456&&to.operations_10==&f.alternate&&to.schedule_20==0x123,"Owning move entry observes exact full source and untouched destination before native header writes");claimed=true;
  auto& result=s::move_io_environment_023cb0(to,from,sentinels,&slot);
  check(&result==&to&&from.state_18==0&&from.executor_28==&f.sentinel_executor&&to.executor_28==&f.changed_executor,"Whole-operation claim outlives native state reset and live executor reload");claimed=false;return true;
 };
 f.original.move=[&](auto& to,auto& from){check(claimed&&to.operations_10==&f.original&&from.operations_10==&f.empty&&from.schedule_20==0x846e8,"Private slot sees the native committed headers after the whole claim");to.payload_00=from.payload_00;to.state_18=37;from.executor_28=&f.changed_executor;};
 check(&s::move_io_environment_023cb0(destination,source,f.env().sentinels)==&destination&&entries==1&&slot.calls==1&&f.original.moves==1&&!claimed&&destination.state_18==37,"Handled whole move invokes one admitted slot without recursively dispatching or replaying the native body");
 Fixture rejected;auto from=rejected.receive.environment_b0;s::IoHandlerEnvironment to{{77,78},&rejected.alternate,19,0x101,&rejected.changed_executor};
 rejected.original.whole_move=[](auto&,auto&,const auto&)->bool{throw s::UnsupportedInput("Rejected full move claim");};
 rejects([&]{s::move_io_environment_023cb0(to,from,rejected.env().sentinels);});
 check(from.operations_10==&rejected.original&&from.schedule_20==0x456&&from.executor_28==&rejected.executor&&from.payload_00==std::array<std::uintptr_t,2>{11,12}&&to.operations_10==&rejected.alternate&&to.schedule_20==0x101&&to.state_18==19&&to.payload_00==std::array<std::uintptr_t,2>{77,78}&&!rejected.original.moves,"Rejected whole claim performs no source/destination prefix write or native slot callback");
 Fixture failed;auto pending=failed.receive.environment_b0;s::IoHandlerEnvironment partial;bool held=true;Slot failed_slot(failed.original,held);pending.state_18=7;failed.original.move=[](auto&,auto&){throw std::runtime_error("Admitted slot outcome unknown");};
 throws([&]{s::move_io_environment_023cb0(partial,pending,failed.env().sentinels,&failed_slot);});
 check(failed_slot.calls==1&&pending.operations_10==&failed.empty&&pending.schedule_20==0x846e8&&pending.state_18==7&&pending.executor_28==&failed.executor&&partial.operations_10==&failed.original,"Unknown admitted-slot outcome retains native preceding writes and executes no normal state/executor tail");
}
void storage_release(){
 for(std::uint64_t size=0;size<=1021;++size)for(unsigned occupancy=0;occupancy<3;++occupancy){Fixture f;f.bytes[size]=std::byte{0x39};if(occupancy)f.state.cache_00[0]=0x111;if(occupancy==2)f.state.cache_00[1]=0x222;f.worker.get=[&](){return &f.context;};s::release_io_storage_026a00(f.storage,size,f.env().storage,f.worker,f.memory);check(f.worker.indices==std::vector<std::uint32_t>{17},"every release reads current TLS exactly once before size decision");if(size<=1020&&occupancy<2){check(f.memory.frees.empty()&&f.state.cache_00[occupancy]==f.storage.identity&&f.bytes[0]==std::byte{0x39},"eligible bounded size copies marker and uses first empty two-slot cache");check(f.state.cache_00[2]==0,"cache never uses third or later free slot");}else check(f.memory.frees==std::vector<std::uintptr_t>{0x10000},"oversize or full first-two slots always call mandatory free provider");}
 Fixture previous;s::WorkerTlsContext top{nullptr,nullptr,&previous.context};previous.worker.get=[&](){return &top;};s::release_io_storage_026a00(previous.storage,0xe0,previous.env().storage,previous.worker,previous.memory);check(previous.memory.frees.size()==1&&!previous.state.cache_00[0],"release uses current TLS state only, never predecessor");
 Fixture live;live.index=55;live.worker.get=[&](){live.state.cache_00[0]=0x333;live.bytes[0xd0]=std::byte{0x45};return &live.context;};s::release_io_storage_026a00(live.storage,0xd0,live.env().storage,live.worker,live.memory);check(live.worker.indices==std::vector<std::uint32_t>{55}&&live.state.cache_00[1]==0x10000&&live.bytes[0]==std::byte{0x45},"TLS callback mutations to slots and marker are read live");
 Fixture bounded;bounded.storage.bytes=std::span<std::byte>(bounded.bytes.data(),100);bounded.worker.get=[&](){return &bounded.context;};rejects([&]{s::release_io_storage_026a00(bounded.storage,100,bounded.env().storage,bounded.worker,bounded.memory);});check(bounded.worker.indices.size()==1&&!bounded.state.cache_00[0]&&bounded.memory.frees.empty(),"bounded marker check follows TLS/candidate but precedes cache publication");
 Fixture huge;s::release_io_storage_026a00(huge.storage,UINT64_MAX,huge.env().storage,huge.worker,huge.memory);check(huge.worker.indices.size()==1&&huge.memory.frees.size()==1,"unsigned huge size still reads TLS then frees without marker indexing");
}
template<bool Receive>void cleanup(){
 using Operation=std::conditional_t<Receive,s::ReceiveIoOperation,s::SendIoOperation>;
 for(std::uint32_t strong:{0U,1U,2U,UINT32_MAX})for(std::uint32_t weak:{0U,1U,2U}){
  Fixture f;Ref owned,borrowed;owned.strong_08=strong;owned.weak_0c=weak;borrowed.weak_0c=weak;Operation* op;
  if constexpr(Receive){op=&f.receive;op->handler.control_a8=&owned;op->weak_48=&borrowed;}else{op=&f.send;op->handler.control_98=&owned;op->weak_40=&borrowed;}
  s::IoCleanupFrame<Operation> frame{42,&f.storage,op};f.empty.destroy=[&](auto&){unexpected();};
  if constexpr(Receive)s::cleanup_receive_0293f0(frame,f.env(),f.worker,f.memory);else s::cleanup_send_0294c0(frame,f.env(),f.worker,f.memory);
  check(owned.strong_08==strong-1U&&owned.disposes==(strong==1?1U:0U),"strong unsigned32 decrement wraps; dispose only old1");check(owned.weak_0c==(strong==1?weak-1U:weak)&&owned.deletes==(strong==1&&weak==1?1U:0U),"owned weak decremented only after last-strong dispose");check(borrowed.weak_0c==weak-1U&&borrowed.deletes==(weak==1?1U:0U),"operation weak unsigned decrement and old1 delete");check(f.original.destroys==1&&!frame.operation_10&&!frame.allocation_08&&f.memory.frees==std::vector<std::uintptr_t>{0x10000},"normal cleanup destroys environment then references then allocation once");
  if constexpr(Receive)s::cleanup_receive_0293f0(frame,f.env(),f.worker,f.memory);else s::cleanup_send_0294c0(frame,f.env(),f.worker,f.memory);
  check(f.original.destroys==1&&f.worker.indices.size()==1,"second cleared cleanup has no effects");
 }
 Fixture f;Ref old_owned,live_owned,old_weak,live_weak;Operation* op;
 if constexpr(Receive){op=&f.receive;op->handler.control_a8=&old_owned;op->weak_48=&old_weak;}else{op=&f.send;op->handler.control_98=&old_owned;op->weak_40=&old_weak;}
 s::IoCleanupFrame<Operation> frame{0,&f.storage,op};
 f.original.destroy=[&](auto&){if constexpr(Receive)op->handler.control_a8=&live_owned;else op->handler.control_98=&live_owned;frame.operation_10=nullptr;frame.allocation_08=&f.other;f.index=19;};
 live_owned.dispose=[&](){live_owned.weak_0c=2;if constexpr(Receive)op->weak_48=&live_weak;else op->weak_40=&live_weak;};
 f.worker.get=[&]()->void*{check(!frame.operation_10&&frame.allocation_08==&f.other,"cleanup clears frame object despite callback mutation then loads allocation");frame.allocation_08=&f.storage;return nullptr;};
 f.memory.free=[&](auto& block){check(&block==&f.other,"allocation pointer cached across TLS callback");frame.allocation_08=&f.storage;};
 if constexpr(Receive)s::cleanup_receive_0293f0(frame,f.env(),f.worker,f.memory);else s::cleanup_send_0294c0(frame,f.env(),f.worker,f.memory);
 check(old_owned.strong_08==1&&live_owned.strong_08==0&&live_owned.disposes==1&&live_owned.weak_0c==1&&!live_owned.deletes,"owned pointer reread after environment destruction; weak count reread after dispose");check(old_weak.weak_0c==1&&live_weak.deletes==1&&f.worker.indices==std::vector<std::uint32_t>{19}&&!frame.allocation_08,"weak pointer reread after strong callbacks; live TLS and unconditional allocation clear");
 Fixture throwing;Operation* failing;if constexpr(Receive)failing=&throwing.receive;else failing=&throwing.send;s::IoCleanupFrame<Operation> pending{0,&throwing.storage,failing};throwing.original.destroy=[](auto&){throw std::runtime_error("destroy");};throws([&]{if constexpr(Receive)s::cleanup_receive_0293f0(pending,throwing.env(),throwing.worker,throwing.memory);else s::cleanup_send_0294c0(pending,throwing.env(),throwing.worker,throwing.memory);});check(pending.operation_10==failing&&pending.allocation_08==&throwing.storage&&throwing.worker.indices.empty(),"throwing destructor does not invent reference/allocation release or tail clears");
}
void direct_helpers(){
 Fixture f;s::ReceiveHandler r;r.error={-7,{99}};r.transferred=UINT64_MAX;s::SendHandler t;t.error={45,{91}};t.transferred=0x123456789ULL;
 f.effects.receive=[&](auto& h,auto& error,auto bytes,auto flag){check(&h==&r&&&error!=&h.error&&error.value==-7&&bytes==UINT64_MAX&&!flag,"receive helper copies error, retains handler identity and full64 bytes/literal0");error.value=5;h.error.value=6;h.transferred=0;};s::invoke_receive_021a40(r,f.effects);check(r.error.value==6&&!r.transferred,"provider private error mutation does not alias handler error");
 f.effects.send=[&](auto& h,auto& error,auto bytes,auto flag){check(&h==&t&&&error!=&h.error&&error.value==45&&bytes==0x123456789ULL&&!flag,"send helper copies defined error fields and fullwidth count");error.category.name_identity=1;};s::invoke_send_021a80(t,f.effects);check(t.error.category.name_identity==91,"send copied error category remains private");
}
template<bool Receive>void error_matrix(){
 for(int input:{0,64,1234,10040,234,-9})for(bool owner:{false,true})for(unsigned flags:{0U,0x10U})for(std::uint64_t requested:{0ULL,1ULL})for(std::uint64_t bytes:{0ULL,0x100000009ULL})for(int weak_kind:{0,1,2}){
  Fixture f;Ref weak,owned;weak.strong_08=weak_kind==2?1:0;weak.weak_0c=2;owned.strong_08=2;owned.weak_0c=3;
  f.receive.state_38=static_cast<std::uint8_t>(flags);f.receive.requested_58=requested;f.receive.weak_48=weak_kind?&weak:nullptr;f.receive.handler.control_a8=&owned;f.send.weak_40=weak_kind?&weak:nullptr;f.send.handler.control_98=&owned;
  int expected=input;std::uintptr_t category=0x1111;
  if(input==64){expected=weak_kind==2?10054:995;category=0x2222;}else if(input==1234){expected=10061;category=0x2222;}else if(Receive&&(input==10040||input==234))expected=0;else if(Receive&&!input&&!bytes&&(flags&0x10)&&requested){expected=2;category=0x3333;}
  unsigned observed=0;
  auto check_payload=[&](auto& handler,auto& error,auto actual_bytes,auto flag){++observed;check(f.memory.frees==std::vector<std::uintptr_t>{0x10000}&&f.empty.destroys==1,"original allocation cleanup completes before handler invocation");check(error.value==expected&&error.category.name_identity==category&&actual_bytes==bytes&&!flag,"native error mapping/EOF branching and full64 byte transport");check(owned.strong_08==2,"moved handler strong reference remains owned during callback");check(weak.weak_0c==(weak_kind?1U:2U),"operation weak reference released before invocation");if constexpr(Receive){check(handler.words_60_78==std::array<std::uint64_t,4>{1,2,3,4}&&handler.byte_80==0x81&&handler.dword_84==0x12345678&&handler.words_88_98==std::array<std::uint64_t,3>{5,6,7}&&handler.object_a0==0x1234,"receive defined payload fields retain native widths");check(!f.receive.handler.object_a0&&!f.receive.handler.control_a8,"original receive owned pair cleared before cleanup");}else{check(handler.words_60_78==std::array<std::uint64_t,4>{8,9,10,11}&&handler.dword_80==0x87654321&&handler.word_88==12&&handler.object_90==0x9876,"send defined payload excludes uninitialized dummy prefix");check(!f.send.handler.object_90&&!f.send.handler.control_98,"original send owned pair cleared before cleanup");}};
  if constexpr(Receive)f.effects.receive=check_payload;else f.effects.send=check_payload;
  s::Error incoming{input,{0x1111}};
  if constexpr(Receive)s::complete_receive_027710(owner?1:0,f.receive,incoming,bytes,f.env(),f.worker,f.memory,f.errors,f.effects);else s::complete_send_027a20(owner?1:0,f.send,incoming,bytes,f.env(),f.worker,f.memory,f.errors,f.effects);
  check(observed==(owner?1U:0U)&&f.original.moves==1&&f.original.destroys==1&&f.empty.destroys==1,"owner null skips invocation but retains moved environment destruction");check(owned.strong_08==1&&!owned.disposes&&f.worker.indices.size()==1&&f.memory.frees.size()==1,"handler strong released once and second cleanup empty");check(f.errors.categories==((input==64||input==1234)?1U:0U)&&f.effects.misc==((Receive&&!input&&!bytes&&(flags&0x10)&&requested)?1U:0U),"category providers called only on exact native branches");check(incoming.value==input&&incoming.category.name_identity==0x1111,"input error remains independent of handler local error");
 }
}
void live_error_and_release(){
 Fixture f;Ref weak,replacement,owned,live_owned;weak.strong_08=0;replacement.strong_08=1;replacement.weak_0c=2;owned.strong_08=2;live_owned.strong_08=2;
 f.receive.weak_48=&weak;f.receive.handler.control_a8=&owned;s::Error error{64,{0x1111}};
 f.original.move=[&](auto& to,auto& from){to.payload_00=from.payload_00;to.state_18=0;f.receive.weak_48=&replacement;error.value=1234;};
 f.errors.category=[&](){replacement.strong_08=0;f.receive.handler.dword_84=0xface;};
 f.effects.receive=[&](auto& h,auto& e,auto,auto){check(e.value==10054&&e.category.name_identity==0x2222&&h.dword_84==0xface,"incoming code snapshotted before move; live weak branch cached before category; payload after category");h.control_a8=&live_owned;};
 s::complete_receive_027710(1,f.receive,error,9,f.env(),f.worker,f.memory,f.errors,f.effects);check(owned.strong_08==2&&live_owned.strong_08==1&&weak.weak_0c==1&&replacement.weak_0c==1,"final strong pointer reread from moved handler after callback; original weak target live after move");
 Fixture eof;eof.receive.state_38=0x10;eof.receive.requested_58=1;eof.effects.misc_effect=[&](){eof.receive.handler.byte_80=0x99;};eof.effects.receive=[&](auto& h,auto& e,auto,auto){check(e.value==2&&h.byte_80==0x99,"EOF category provider effects precede handler snapshot");};s::complete_receive_027710(1,eof.receive,{},0,eof.env(),eof.worker,eof.memory,eof.errors,eof.effects);
 Fixture send;Ref previous,next;previous.strong_08=2;next.strong_08=2;send.send.handler.control_98=&previous;send.send.environment_a0.state_18=4;send.effects.schedule_send=[&](auto& env,auto& h){check(env.state_18==4&&env.executor_28==&send.executor&&send.memory.frees.size()==1,"send scheduler gets moved live environment after early cleanup");h.control_98=&next;};s::complete_send_027a20(1,send.send,{234,{99}},0,send.env(),send.worker,send.memory,send.errors,send.effects);check(previous.strong_08==2&&next.strong_08==1&&send.effects.send_schedules==1&&!send.effects.sends,"send polymorphic branch delegates required scheduler then rereads local ownership");
}
Fixture* active=nullptr;
void wrong_prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&){unexpected();}
void prepare(s::IoHandlerEnvironment& out,std::uintptr_t query,s::IoPrepareScratch&){auto& f=*active;check(query==0xabc&&f.original.queries==1&&f.memory.frees.size()==1,"cached factory invoked after query and early original cleanup");out={{77,78},&f.temporary,55,0x7788,&f.changed_executor};}
void polymorphic_receive(){
 for(bool guard:{false,true}){Fixture f;active=&f;auto executor=std::make_unique<s::IoHandlerExecutor>();executor->prepare_slot_40=&prepare;f.receive.environment_b0.executor_28=executor.get();f.receive.environment_b0.state_18=9;
  f.original.query=[&](auto& env){check(env.executor_28==executor.get(),"query sees original executor identity");executor->prepare_slot_40=&wrong_prepare;executor.reset();env.executor_28=nullptr;};
  f.temporary.move=[&](auto& to,auto& from){check(from.operations_10==&f.empty&&from.schedule_20==0x846e8,"nested move resets temporary tables before callback");to.payload_00=from.payload_00;to.state_18=from.state_18;from.executor_28=&f.executor;if(!guard)from.operations_10=&f.alternate;};
  f.effects.schedule_receive=[&](auto& nested,auto& handler){check(nested.operations_10==&f.temporary&&nested.payload_00==std::array<std::uintptr_t,2>{77,78}&&nested.state_18==55&&nested.executor_28==&f.executor,"actual nested move retains prepared payload and live executor");check(f.alternate.destroys==(guard?0U:1U)&&handler.object_a0==0x1234,"temporary live destructor guard precedes scheduler");nested.operations_10=&f.alternate;};
  s::complete_receive_027710(1,f.receive,{},8,f.env(),f.worker,f.memory,f.errors,f.effects);
  check(!executor&&f.original.queries==1&&f.temporary.moves==1&&f.effects.receive_schedules==1&&!f.effects.receives,"receive cached raw factory survives executor lifetime end during query");check(f.alternate.destroys==(guard?1U:2U)&&f.original.destroys==1&&f.empty.destroys==1,"nested live table destroyed unconditionally; guard skips only temporary destructor");
 }
 Fixture missing;missing.receive.environment_b0.state_18=1;rejects([&]{s::complete_receive_027710(1,missing.receive,{},8,missing.env(),missing.worker,missing.memory,missing.errors,missing.effects);});check(missing.memory.frees.size()==1&&!missing.original.destroys,"missing factory boundary follows early cleanup without invented normal tail");
 Fixture throwing;Ref owned;owned.strong_08=2;throwing.receive.handler.control_a8=&owned;throwing.effects.receive=[](auto&,auto&,auto,auto){throw std::runtime_error("handler");};throws([&]{s::complete_receive_027710(1,throwing.receive,{},8,throwing.env(),throwing.worker,throwing.memory,throwing.errors,throwing.effects);});check(throwing.memory.frees.size()==1&&owned.strong_08==2&&!throwing.original.destroys,"throwing callback propagates after early cleanup without invented local release/destruction");
 Fixture sendthrow;sendthrow.send.environment_a0.state_18=1;sendthrow.effects.schedule_send=[](auto&,auto&){throw std::runtime_error("schedule");};throws([&]{s::complete_send_027a20(1,sendthrow.send,{},8,sendthrow.env(),sendthrow.worker,sendthrow.memory,sendthrow.errors,sendthrow.effects);});check(sendthrow.memory.frees.size()==1&&!sendthrow.original.destroys,"throwing send scheduler has no fabricated final environment cleanup");
}
}
int main(){try{preparation_caller_boundaries();preparation_unknown_prefixes();move_environment();owning_move_boundary();storage_release();cleanup<true>();cleanup<false>();direct_helpers();error_matrix<true>();error_matrix<false>();live_error_and_release();polymorphic_receive();std::cout<<"PASS: "<<checks<<" socket IO completion checks; eight retained normal bodies, defined-field error mapping, live environment/reference mutations, unsigned counters, two-slot storage and polymorphic factory sequencing\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
