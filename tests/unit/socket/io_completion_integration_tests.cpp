// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_io_completion_dispatch.hpp"
#include "preloader_l/net/transport_completion_dispatcher_native.hpp"
#include "preloader_l/net/transport_worker_run_native.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace s=preloader_l::socket;
namespace {
unsigned checks=0;void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unexpected(){throw std::logic_error("Unexpected IO integration dependency");}
struct Gate:s::SocketGate{unsigned depth=0;void enter()override{++depth;}void leave()override{check(depth!=0,"integration queue gate balanced");--depth;}};
struct Entry:s::IocpSchedulerEntry{using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unexpected();}};
struct Errors:s::Api{unsigned categories=0;s::Category system_category_028620()override{++categories;return {0x2222};}void set_last_error(std::int32_t)override{unexpected();}std::int32_t get_last_error()override{unexpected();}s::Conversion convert_acp(const char*,std::int32_t)override{unexpected();}s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unexpected();}s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unexpected();}std::int32_t select_write(s::Socket,std::int32_t)override{unexpected();}};
struct Table:s::IoHandlerVtable{
 unsigned moves=0,destroys=0;std::function<void()> destroy;
 explicit Table(std::uintptr_t id):IoHandlerVtable(id){}
 void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;if(destroy)destroy();}
 void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;to.payload_00=from.payload_00;to.state_18=from.state_18;}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unexpected();}
};
struct Effects:s::IoCompletionEffects{
 unsigned receives=0,sends=0;std::function<void(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)> receive;std::function<void(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)> send;
 s::Category misc_category_0285b0()override{unexpected();}
 void receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{++receives;if(receive)receive(h,e,bytes,flag);}
 void send_body_0261d0(s::SendHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{++sends;if(send)send(h,e,bytes,flag);}
 void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unexpected();}
 void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unexpected();}
};
struct Fixture;Fixture* active=nullptr;
LPVOID WINAPI tls_get(DWORD);BOOL WINAPI tls_set(DWORD,LPVOID);BOOL WINAPI post(HANDLE,DWORD,ULONG_PTR,LPOVERLAPPED);VOID WINAPI set_error(DWORD);DWORD WINAPI get_error();BOOL WINAPI get(HANDLE,LPDWORD,PULONG_PTR,LPOVERLAPPED*,DWORD);BOOL WINAPI timer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,LPVOID,BOOL);
struct Node{std::array<std::byte,1025> bytes{};s::OperationStorage storage;std::unique_ptr<s::ReceiveIoOperation> receive;std::unique_ptr<s::SendIoOperation> send;std::unique_ptr<s::PostedOperation> posted;explicit Node(std::uintptr_t id):storage{id,bytes}{}};
struct Memory:s::PostedMemoryEffects{
 Fixture& f;std::array<std::unique_ptr<Node>,8> nodes;std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;bool delete_on_free=false;
 explicit Memory(Fixture& fixture):f(fixture){}Node& add();s::OperationStorage& allocate_04b0bc(std::uint64_t)override;void free_04b788(s::OperationStorage&)override;s::PostedOperation& project_posted(s::OperationStorage&)override;
};
struct Routing:s::IoDispatcherEffects{
 Fixture& f;unsigned unknown=0;std::function<void(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)> other;
 explicit Routing(Fixture&);
 void complete_unknown(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override;
 std::uintptr_t capture_tls_slot()override{return 0x1234;}std::int32_t thread_epoch(std::uintptr_t token)override{check(token==0x1234,"same captured TLS guard identity");return 0;}
 void guard_header_04b838(std::int32_t&)override{unexpected();}void register_atexit_04b770(std::uintptr_t)override{unexpected();}void guard_footer_04b7cc(std::int32_t&)override{unexpected();}
 std::uint32_t read_stop_34(s::IocpSchedulerEntry& entry)override{return entry.high_34;}
 std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>&,const std::array<std::uintptr_t,2>&)override{unexpected();}
 [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{unexpected();}
};
struct Worker:s::DispatcherWorkerEffects{
 Fixture& f;explicit Worker(Fixture&);
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry&,std::uint32_t)override;
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry&,std::uint32_t)override;
 void initialize_04a240(std::array<std::uintptr_t,2>&)override;void finish_04a2cc(std::array<std::uintptr_t,2>&)override;
 void free_cache_04b788(std::uintptr_t)override;void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override{unexpected();}
 [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unexpected();}
};
struct Packet{OVERLAPPED* identity=nullptr;DWORD bytes=0,error=0;};
struct Functor:s::PostedFunctor{unsigned invokes=0,discards=0;std::function<void()> action;void call_slot_00(std::uint8_t flag)override{if(flag){++invokes;if(action)action();}else ++discards;}};
struct Fixture{
 Gate gate;CRITICAL_SECTION cs{};Entry entry{gate,cs};Errors errors;Effects effects;Table original{0x111},empty{0x777};s::IoHandlerExecutor executor{};std::uint32_t index=17;std::int32_t guard=0;
 std::array<s::OperationStorageBinding,8> storage_bindings{};std::array<s::PostedOperationBinding,8> posted_bindings{};std::array<s::IoCompletionBinding,8> io_bindings{};std::array<s::DispatcherOperationBinding,8> dispatcher_bindings{};
 s::DispatcherState state{entry,nullptr,0,0,dispatcher_bindings};
 s::NativeWorkerRunApi worker_api{{&tls_get,&tls_set,&post,&get_error}};s::NativeDispatcherApi api{{&set_error,&get_error,&get,&post,&timer}};
 Memory memory{*this};Routing routing{*this};Worker worker{*this};s::PostedExecutor posted_executor{0x1234ab,&entry,1};
 void* tls=nullptr;DWORD error=0;std::vector<Packet> packets;std::size_t cursor=0;unsigned posts=0,gets=0,tls_gets=0,tls_sets=0;bool fail_first=false;std::vector<std::string> events;
 Fixture(){active=this;entry.completion.port=0xabcdef1234ULL;entry.timeout_40=0xffffffffU;}
 s::IoCompletionEnvironment environment(){return {{index,storage_bindings},{&empty,0x846e8,&executor,0x777}};}
 Node& receive(){auto& n=memory.add();n.receive=std::make_unique<s::ReceiveIoOperation>(s::ReceiveIoOperation{n.storage,{},0x10,nullptr,99,{},{{11,12},&original,0,0x456,&executor}});n.receive->handler.object_a0=0x1234;n.bytes[0xe0]=std::byte{56};bind(n);return n;}
 Node& send(){auto& n=memory.add();n.send=std::make_unique<s::SendIoOperation>(s::SendIoOperation{n.storage,{},nullptr,{},{{21,22},&original,0,0x456,&executor}});n.send->handler.object_90=0x9876;n.bytes[0xd0]=std::byte{52};bind(n);return n;}
 void bind(Node& n){for(std::size_t i=0;i<memory.nodes.size();++i)if(memory.nodes[i].get()==&n){auto* op=n.receive?&n.receive->operation:n.send?&n.send->operation:&n.posted->operation;io_bindings[i]={op,n.receive.get(),n.send.get()};posted_bindings[i]={op,n.posted.get()};dispatcher_bindings[i]={op,{},0,&op->overlapped};return;}unexpected();}
 void submit(s::PostedFunctor*& source){s::dispatch_posted_022e40(posted_executor,source,environment().storage,worker_api,api,memory);}
 std::uint64_t run(){s::WorkerRunResult result;const auto count=s::run_worker_0298b0(entry,result,{index,{0x1111}},worker_api,errors,worker);check(result.error&&result.error->value==0&&result.error->category.name_identity==0x1111,"actual worker source returns logical generic success");return count;}
};
Routing::Routing(Fixture& fixture):IoDispatcherEffects(fixture.posted_bindings,fixture.io_bindings,fixture.environment(),fixture.worker_api,fixture.memory,fixture.errors,fixture.effects),f(fixture){}
Worker::Worker(Fixture& fixture):DispatcherWorkerEffects(fixture.state,{fixture.guard,{0x1111},{0x2222}},fixture.api,fixture.routing,fixture.worker_api,fixture.errors),f(fixture){}
void Routing::complete_unknown(s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){++unknown;if(other)other(owner,op,error,bytes);else unexpected();}
std::uint32_t Worker::exchange_stop_34(s::IocpSchedulerEntry& entry,std::uint32_t value){const auto old=entry.high_34;entry.high_34=value;f.events.push_back("stop");return old;}
std::uint32_t Worker::exchange_wake_38(s::IocpSchedulerEntry& entry,std::uint32_t value){const auto old=static_cast<std::uint32_t>(entry.word_38);entry.word_38=(entry.word_38&0xffffffff00000000ULL)|value;return old;}
void Worker::initialize_04a240(std::array<std::uintptr_t,2>& context){check(context==std::array<std::uintptr_t,2>{},"worker initializes zero CRT context");context={11,12};f.events.push_back("initialize");}
void Worker::finish_04a2cc(std::array<std::uintptr_t,2>& context){check(context==std::array<std::uintptr_t,2>{11,12},"same required CRT context finalized");f.events.push_back("finish");}
void Worker::free_cache_04b788(std::uintptr_t identity){check(!f.tls,"outer worker restores prior TLS before freeing returned IO/posted storage");for(auto& binding:f.storage_bindings)if(binding.identity==identity&&binding.storage){f.memory.free_04b788(*binding.storage);return;}unexpected();}
Node& Memory::add(){for(std::size_t i=0;i<nodes.size();++i)if(!nodes[i]){nodes[i]=std::make_unique<Node>(0x10000+i*0x1000);f.storage_bindings[i]={nodes[i]->storage.identity,&nodes[i]->storage};return *nodes[i];}unexpected();}
s::OperationStorage& Memory::allocate_04b0bc(std::uint64_t size){allocations.push_back(size);return add().storage;}
void Memory::free_04b788(s::OperationStorage& storage){frees.push_back(storage.identity);f.events.push_back("free");if(delete_on_free)for(std::size_t i=0;i<nodes.size();++i)if(nodes[i]&&&nodes[i]->storage==&storage){f.storage_bindings[i].storage=nullptr;nodes[i].reset();return;}}
s::PostedOperation& Memory::project_posted(s::OperationStorage& storage){for(auto& node:nodes)if(node&&&node->storage==&storage){node->receive.reset();node->send.reset();if(!node->posted)node->posted=std::make_unique<s::PostedOperation>(s::PostedOperation{storage,{},0,nullptr});f.bind(*node);return *node->posted;}unexpected();}
LPVOID WINAPI tls_get(DWORD index){auto& f=*active;check(index==f.index,"existing native TLS getter forwards live global index");++f.tls_gets;return f.tls;}
BOOL WINAPI tls_set(DWORD index,LPVOID pointer){auto& f=*active;check(index==f.index,"existing native TLS setter forwards live index");++f.tls_sets;f.tls=pointer;return TRUE;}
BOOL WINAPI post(HANDLE port,DWORD bytes,ULONG_PTR key,LPOVERLAPPED raw){auto& f=*active;check(reinterpret_cast<std::uintptr_t>(port)==f.entry.completion.port&&!bytes&&!key,"existing native publication forwards fullwidth port and zero metadata");++f.posts;if(f.fail_first&&f.posts==1)return FALSE;Packet packet{raw,0,0};for(auto& node:f.memory.nodes)if(node){if(node->receive&&raw==&node->receive->operation.overlapped)packet={raw,0,10040};else if(node->send&&raw==&node->send->operation.overlapped)packet={raw,19,1234};}f.packets.push_back(packet);return TRUE;}
VOID WINAPI set_error(DWORD error){check(!error,"dispatcher resets last error before injected wait");active->error=error;}
DWORD WINAPI get_error(){return active->error;}
BOOL WINAPI get(HANDLE port,LPDWORD bytes,PULONG_PTR key,LPOVERLAPPED* op,DWORD timeout){auto& f=*active;check(reinterpret_cast<std::uintptr_t>(port)==f.entry.completion.port&&timeout==0xffffffffU,"actual worker waits on same port with infinite timeout");check(bytes&&key&&op&&!(*bytes)&&!(*key)&&!(*op),"SDK adapter initializes completion outputs");check(f.cursor<f.packets.size(),"synthetic packet queue bounds native-adapter worker loop");const auto packet=f.packets[f.cursor++];*op=packet.identity;*bytes=packet.bytes;*key=0;f.error=packet.error;++f.gets;return TRUE;}
BOOL WINAPI timer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,LPVOID,BOOL){unexpected();}
void worker_composition(){
 for(bool fallback:{false,true}){Fixture f;f.fail_first=fallback;f.memory.delete_on_free=true;auto& r=f.receive();auto& t=f.send();const auto receive_identity=r.storage.identity,send_identity=t.storage.identity;Functor followup;unsigned receive_calls=0,send_calls=0;
  followup.action=[&](){auto* context=static_cast<s::WorkerTlsContext*>(f.tls);check(context&&context->state&&context->state->cache_00[0]&&context->state->cache_00[1],"posted callback returns reused IO allocation alongside other IO allocation before invocation");check(f.memory.frees.empty(),"outer worker owns cache reclamation until stop");};
  f.effects.receive=[&](auto& handler,auto& error,auto bytes,auto flag){++receive_calls;check(!flag&&error.value==0&&error.category.name_identity==0x2222&&!bytes&&handler.object_a0==0x1234,"actual dispatcher receive callback clears10040 without EOF fallthrough");auto* context=static_cast<s::WorkerTlsContext*>(f.tls);check(context&&context->port_key==&f.entry.completion.port&&context->state,"actual worker publishes matching live TLS frame");const auto cached=context->state->cache_00[0];check(cached&&f.memory.allocations.empty()&&f.memory.frees.empty(),"receive cleanup returns operation storage before handler");s::PostedFunctor* pending=&followup;f.submit(pending);check(!pending&&f.memory.allocations.empty()&&!context->state->cache_00[0],"actual executor allocator reuses IO size-class storage for posted job");};
  f.effects.send=[&](auto& handler,auto& error,auto bytes,auto flag){++send_calls;check(!flag&&error.value==10061&&error.category.name_identity==0x2222&&bytes==19&&handler.object_90==0x9876,"actual send completion maps1234 and preserves payload/bytes");};
  s::enqueue_posted_028ca0(f.entry,r.receive->operation,0,f.api);s::enqueue_posted_028ca0(f.entry,t.send->operation,0,f.api);
  check(f.entry.work_count_30==2&&!receive_calls&&!send_calls,"two actual operation publications increase work before execution");
  check(f.run()==3&&receive_calls==1&&send_calls==1&&followup.invokes==1&&!followup.discards&&!f.routing.unknown,"actual028ca0→0298b0→027c10→IO routing and022e40→027120 posted routing coexist");
  auto freed=f.memory.frees;std::sort(freed.begin(),freed.end());check(freed==std::vector<std::uintptr_t>{receive_identity,send_identity}&&!f.memory.nodes[0]&&!f.memory.nodes[1],"outer worker frees both actual heap operation/storage projections once after cache reuse");
  check(!f.tls&&f.tls_sets==2&&f.entry.work_count_30==0&&f.entry.high_34==1&&!f.gate.depth&&!f.entry.completion.head&&!f.entry.completion.tail&&!f.entry.completion.dispatch_required,"full worker drain restores TLS, clears work and balances fallback queue");
  check(f.original.moves==2&&f.original.destroys==2&&f.empty.destroys==2&&f.errors.categories==1,"each IO environment moved/destroyed once with one mapped system-category call");
  check(f.events.size()>=3&&f.events[f.events.size()-3]=="free"&&f.events[f.events.size()-2]=="free"&&f.events.back()=="finish","both cached allocation lifetimes end before final CRT provider");
 }
}
struct HeapRef:s::IoReference{bool& disposed;bool& deleted;explicit HeapRef(bool& a,bool& b):disposed(a),deleted(b){}void dispose_slot_00()override{disposed=true;}void delete_slot_08()override{deleted=true;delete this;}};
void routing_and_heap_lifetimes(){
 for(bool receive:{false,true})for(bool owner:{false,true}){Fixture f;f.memory.delete_on_free=true;bool disposed=false,deleted=false;auto* control=new HeapRef(disposed,deleted);auto& node=receive?f.receive():f.send();const auto identity=node.storage.identity;s::CancelOperation* operation;
  if(receive){node.receive->handler.control_a8=control;operation=&node.receive->operation;}else{node.send->handler.control_98=control;operation=&node.send->operation;}
  f.original.destroy=[&](){check(disposed&&deleted,"last moved strong reference and actual heap control lifetime end precede local environment destruction");};
  auto observe=[&](auto&,auto& error,auto bytes,auto flag){check(!f.memory.nodes[0]&&f.memory.frees==std::vector<std::uintptr_t>{identity}&&!disposed&&!deleted,"actual operation/storage deleted before handler; moved control remains live");check(!flag&&error.value==-7&&error.category.name_identity==0x5555&&bytes==99,"IO routing forwards owner-independent defined error and byte fields");};
  if(receive)f.effects.receive=observe;else f.effects.send=observe;
  f.routing.complete(owner?&f.entry:nullptr,*operation,{-7,{0x5555}},99);
  check(disposed&&deleted&&!f.memory.nodes[0]&&f.effects.receives==(receive&&owner?1U:0U)&&f.effects.sends==(!receive&&owner?1U:0U),"null owner skips invoke but releases moved ownership; callbacks tolerate actual lifetime ends");
  s::CancelOperation foreign;f.routing.other=[&](auto* own,auto& op,const auto& error,auto bytes){check(own==&f.entry&&&op==&foreign&&error.value==4&&error.category.name_identity==88&&bytes==7,"unknown boundary forwards all fields unchanged despite stale unrelated bindings");};f.routing.complete(&f.entry,foreign,{4,{88}},7);check(f.routing.unknown==1,"unknown-kind provider stays mandatory after deleted object binding");
 }
 Fixture ambiguous;auto& r=ambiguous.receive();auto& t=ambiguous.send();ambiguous.io_bindings[0].send=t.send.get();bool rejected=false;try{ambiguous.routing.complete(&ambiguous.entry,r.receive->operation,{},0);}catch(const s::UnsupportedInput&){rejected=true;}check(rejected&&ambiguous.memory.frees.empty()&&!ambiguous.effects.receives&&!ambiguous.effects.sends,"ambiguous two-kind projection rejects before arbitrary dispatch");
 Fixture null;s::CancelOperation foreign;null.io_bindings[0]={&foreign,nullptr,nullptr};null.routing.other=[&](auto* owner,auto& op,const auto&,auto){check(!owner&&&op==&foreign,"null IO projection forwards required foreign-kind callback");};null.routing.complete(nullptr,foreign,{},0);check(null.routing.unknown==1&&!null.tls_gets,"unbound projection is never silently dropped");
}
}
int main(){try{worker_composition();routing_and_heap_lifetimes();std::cout<<"PASS: "<<checks<<" socket IO completion integration checks; actual dispatcher/worker/IO/posted source composition, injected existing native SDK adapters, mixed size-class cache reuse, mandatory unknown routing and heap lifetime ends\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
