#include "preloader_l/net/transport_io_completion_dispatch.hpp"
#include "preloader_l/net/transport_handler_dispatch_bridge.hpp"
#include "preloader_l/net/transport_completion_dispatcher_native.hpp"
#include "preloader_l/net/transport_worker_run_native.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace s=preloader_l::socket_recovered;
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
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{return 0xabc;}
};
struct Effects:s::RecoveredHandlerDispatchEffects{
 using RecoveredHandlerDispatchEffects::RecoveredHandlerDispatchEffects;
 unsigned receives=0,sends=0;std::function<void(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)> receive;std::function<void(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)> send;
 s::Category misc_category_0285b0()override{unexpected();}
 void receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{++receives;if(receive)receive(h,e,bytes,flag);}
 void send_body_0261d0(s::SendHandler& h,s::Error& e,std::uint64_t bytes,std::uint32_t flag)override{++sends;if(send)send(h,e,bytes,flag);}
};
struct Fixture;Fixture* active=nullptr;
void prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&);void borrow(s::IoHandlerEnvironment&,s::BorrowedIoHandler&);void publish(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&);
LPVOID WINAPI tls_get(DWORD);BOOL WINAPI tls_set(DWORD,LPVOID);BOOL WINAPI post(HANDLE,DWORD,ULONG_PTR,LPOVERLAPPED);VOID WINAPI set_error(DWORD);DWORD WINAPI get_error();BOOL WINAPI get(HANDLE,LPDWORD,PULONG_PTR,LPOVERLAPPED*,DWORD);BOOL WINAPI timer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,LPVOID,BOOL);
struct Node{std::array<std::byte,1025> bytes{};s::OperationStorage storage;std::unique_ptr<s::ReceiveIoOperation> receive;std::unique_ptr<s::SendIoOperation> send;std::unique_ptr<s::PostedOperation> posted;std::unique_ptr<s::ReceiveHandlerTask> receive_task;std::unique_ptr<s::SendHandlerTask> send_task;explicit Node(std::uintptr_t id):storage{id,bytes}{}};
struct Memory:s::HandlerDispatchMemory{
 Fixture& f;std::array<std::unique_ptr<Node>,8> nodes;std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;bool delete_on_free=false;
 explicit Memory(Fixture& fixture):f(fixture){}Node& add();s::OperationStorage& allocate_04b0bc(std::uint64_t)override;void free_04b788(s::OperationStorage&)override;s::PostedOperation& project_posted(s::OperationStorage&)override;s::ReceiveHandlerTask& project_receive_task(s::OperationStorage&,s::HandlerTaskContext&)override;s::SendHandlerTask& project_send_task(s::OperationStorage&,s::HandlerTaskContext&)override;
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
struct Worker:s::RecoveredDispatcherWorkerEffects{
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
 Gate gate;CRITICAL_SECTION cs{};Entry entry{gate,cs};Errors errors;Table original{0x111},empty{0x777};s::IoHandlerExecutor executor{&prepare};std::uint32_t index=17;std::int32_t guard=0;
 std::array<s::OperationStorageBinding,8> storage_bindings{};std::array<s::PostedOperationBinding,8> posted_bindings{};std::array<s::IoCompletionBinding,8> io_bindings{};std::array<s::DispatcherOperationBinding,8> dispatcher_bindings{};
 s::DispatcherState state{entry,nullptr,0,0,dispatcher_bindings};
 s::NativeWorkerRunApi worker_api{{&tls_get,&tls_set,&post,&get_error}};s::NativeDispatcherApi api{{&set_error,&get_error,&get,&post,&timer}};
 Memory memory{*this};s::HandlerSchedule schedule{&publish,nullptr};std::array<s::HandlerScheduleBinding,1> schedules{{{0x456,&schedule}}};Effects effects{environment(),worker_api,memory,schedules};Routing routing{*this};Worker worker{*this};s::PostedExecutor posted_executor{0x1234ab,&entry,1};
 void* tls=nullptr;DWORD error=0;std::vector<Packet> packets;std::size_t cursor=0;unsigned posts=0,gets=0,tls_gets=0,tls_sets=0;bool fail_first=false;std::vector<std::string> events;unsigned published=0,borrowed=0,prepared=0;
 Fixture(){active=this;entry.completion.port=0xabcdef1234ULL;entry.timeout_40=0xffffffffU;}
 s::IoCompletionEnvironment environment(){return {{index,storage_bindings},{&empty,0x846e8,&executor,0x777}};}
 Node& receive(){auto& n=memory.add();n.receive=std::make_unique<s::ReceiveIoOperation>(s::ReceiveIoOperation{n.storage,{},0x10,nullptr,99,{},{{11,12},&original,0,0x456,&executor}});n.receive->handler.object_a0=0x1234;n.bytes[0xe0]=std::byte{56};bind(n);return n;}
 Node& send(){auto& n=memory.add();n.send=std::make_unique<s::SendIoOperation>(s::SendIoOperation{n.storage,{},nullptr,{},{{21,22},&original,0,0x456,&executor}});n.send->handler.object_90=0x9876;n.bytes[0xd0]=std::byte{52};bind(n);return n;}
 void bind(Node& n){for(std::size_t i=0;i<memory.nodes.size();++i)if(memory.nodes[i].get()==&n){auto* op=n.receive?&n.receive->operation:n.send?&n.send->operation:&n.posted->operation;io_bindings[i]={op,n.receive.get(),n.send.get()};posted_bindings[i]={op,n.posted.get()};dispatcher_bindings[i]={op,{},0,&op->overlapped};return;}unexpected();}
 void submit(s::PostedFunctor*& source){s::dispatch_posted_022e40(posted_executor,source,environment().storage,worker_api,api,memory);}
 std::uint64_t run(){s::WorkerRunResult result;const auto count=s::run_worker_0298b0(entry,result,{index,{0x1111}},worker_api,errors,worker);check(result.error&&result.error->value==0&&result.error->category.name_identity==0x1111,"actual worker source returns logical generic success");return count;}
};
Routing::Routing(Fixture& fixture):IoDispatcherEffects(fixture.posted_bindings,fixture.io_bindings,fixture.environment(),fixture.worker_api,fixture.memory,fixture.errors,fixture.effects),f(fixture){}
Worker::Worker(Fixture& fixture):RecoveredDispatcherWorkerEffects(fixture.state,{fixture.guard,{0x1111},{0x2222}},fixture.api,fixture.routing,fixture.worker_api,fixture.errors),f(fixture){}
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
s::ReceiveHandlerTask& Memory::project_receive_task(s::OperationStorage& storage,s::HandlerTaskContext& context){for(auto& node:nodes)if(node&&&node->storage==&storage){node->send_task.reset();node->receive_task=std::make_unique<s::ReceiveHandlerTask>(storage,context);return *node->receive_task;}unexpected();}
s::SendHandlerTask& Memory::project_send_task(s::OperationStorage& storage,s::HandlerTaskContext& context){for(auto& node:nodes)if(node&&&node->storage==&storage){node->receive_task.reset();node->send_task=std::make_unique<s::SendHandlerTask>(storage,context);return *node->send_task;}unexpected();}
void prepare(s::IoHandlerEnvironment& environment,std::uintptr_t query,s::IoPrepareScratch&){auto& f=*active;check(query==0xabc,"nested send factory receives original query result");++f.prepared;environment={{31,32},&f.original,0,0x456,&f.executor};}
void borrow(s::IoHandlerEnvironment&,s::BorrowedIoHandler& descriptor){auto& f=*active;++f.borrowed;descriptor.invoke(f.effects);}
void publish(s::IoHandlerEnvironment&,s::OwnedHandlerTask*& slot){auto& f=*active;++f.published;check(slot!=nullptr,"recovered handler scheduler receives owned closure");auto* task=slot;slot=nullptr;task->call_slot_00(1);}
void worker_composition(){
 for(bool fallback:{false,true})for(bool borrowed:{false,true}){Fixture f;f.fail_first=fallback;f.memory.delete_on_free=true;if(borrowed)f.schedule.borrowed_slot_18=&borrow;
  auto& r=f.receive();auto& t=f.send();r.receive->environment_b0.state_18=1;t.send->environment_a0.state_18=1;const auto r_id=r.storage.identity,t_id=t.storage.identity;Functor posted;std::uintptr_t handler_cache=0;unsigned receive_calls=0,send_calls=0;
  posted.action=[&](){auto* tls=static_cast<s::WorkerTlsContext*>(f.tls);check(tls&&tls->state&&tls->state->cache_00[0],"posted completion returns reused operation allocation to first two slots");};
  f.effects.receive=[&](auto& handler,auto& error,auto bytes,auto flag){++receive_calls;check(!flag&&!error.value&&error.category.name_identity==0x2222&&!bytes&&handler.object_a0==0x1234,"actual complete_receive plus recovered handler scheduler invokes original defined arguments");auto* tls=static_cast<s::WorkerTlsContext*>(f.tls);check(tls&&tls->state&&tls->port_key==&f.entry.completion.port,"recovered handler dispatch executes inside actual worker TLS frame");check(tls->state->cache_00[0]==r_id,"IO cleanup returns operation storage before scheduling handler");if(!borrowed){handler_cache=tls->state->cache_00[4];check(handler_cache&&f.memory.allocations==std::vector<std::uint64_t>{121},"owned receive closure already returns separate pool4 allocation before handler");}else check(!tls->state->cache_00[4]&&f.memory.allocations.empty(),"borrowed receive incurs no owned closure allocation");s::PostedFunctor* pending=&posted;f.submit(pending);check(!pending&&!tls->state->cache_00[0],"actual executor reuses operation pool0 while handler pool4 remains independent");};
  f.effects.send=[&](auto& handler,auto& error,auto bytes,auto flag){++send_calls;check(!flag&&error.value==10061&&error.category.name_identity==0x2222&&bytes==19&&handler.object_90==0x9876,"actual send error mapping flows through recovered nested scheduler and native thunk");auto* tls=static_cast<s::WorkerTlsContext*>(f.tls);if(!borrowed){check(tls->state->cache_00[4]==handler_cache&&f.memory.allocations==std::vector<std::uint64_t>{121},"smaller send closure reuses returned receive handler allocation in pool4");}};
  if(fallback){s::enqueue_posted_028ca0(f.entry,t.send->operation,0,f.api);s::enqueue_posted_028ca0(f.entry,r.receive->operation,0,f.api);}else{s::enqueue_posted_028ca0(f.entry,r.receive->operation,0,f.api);s::enqueue_posted_028ca0(f.entry,t.send->operation,0,f.api);}
  check(f.run()==3&&receive_calls==1&&send_calls==1&&posted.invokes==1&&!posted.discards&&!f.routing.unknown,"actual publication worker dispatcher IO callback recovered scheduler owned callback and posted composition");
  std::vector<std::uintptr_t> expected{r_id,t_id};if(!borrowed)expected.push_back(handler_cache);auto freed=f.memory.frees;std::sort(freed.begin(),freed.end());std::sort(expected.begin(),expected.end());check(freed==expected,"outer worker frees operation and separate handler size-class allocations exactly once");
  check(f.published==(borrowed?0U:2U)&&f.borrowed==(borrowed?2U:0U)&&f.prepared==2,"both IO directions choose borrowed/owned consistently; send enters recovered nested factory once");check(!f.tls&&f.tls_sets==2&&!f.entry.work_count_30&&f.entry.high_34==1&&!f.gate.depth&&!f.entry.completion.head&&!f.entry.completion.tail&&!f.entry.completion.dispatch_required,"full worker drains fallback queue and work then restores original TLS");check(f.original.moves==4&&f.original.destroys==4&&f.empty.destroys==2,"IO caller and nested handler environments move/destroy with source-derived lifetimes");
 }
}
struct HeapRef:s::IoReference{bool& disposed;bool& deleted;explicit HeapRef(bool& a,bool& b):disposed(a),deleted(b){}void dispose_slot_00()override{disposed=true;}void delete_slot_08()override{deleted=true;delete this;}};
void routing_heap_lifetimes(){
 for(bool receive:{false,true}){Fixture f;f.memory.delete_on_free=true;bool disposed=false,deleted=false;auto* ref=new HeapRef(disposed,deleted);auto& node=receive?f.receive():f.send();if(receive)node.receive->environment_b0.state_18=1;else node.send->environment_a0.state_18=1;const auto id=node.storage.identity;auto* op=receive?&node.receive->operation:&node.send->operation;if(receive)node.receive->handler.control_a8=ref;else node.send->handler.control_98=ref;
  auto observe=[&](auto&,auto& error,auto bytes,auto flag){check(!flag&&error.value==-7&&error.category.name_identity==0x5555&&bytes==99,"direct routing preserves arbitrary defined error/bytes into recovered owned handler");check(!f.memory.nodes[0]&&!f.memory.nodes[1]&&f.memory.frees.size()==2&&!disposed&&!deleted,"actual IO object/storage and owned closure/storage deleted before user body; moved control remains live");};if(receive)f.effects.receive=observe;else f.effects.send=observe;
  f.routing.complete(&f.entry,*op,{-7,{0x5555}},99);check(disposed&&deleted&&f.memory.frees.front()==id&&f.published==1,"actual control deletes itself after handler despite both original allocation lifetime ends");
  s::CancelOperation unknown;f.routing.other=[&](auto* owner,auto& operation,const auto& error,auto bytes){check(owner==&f.entry&&&operation==&unknown&&error.value==4&&bytes==7,"required unknown callback remains live after all old projected objects deleted");};f.routing.complete(&f.entry,unknown,{4,{88}},7);check(f.routing.unknown==1,"unknown operation forwarded without dereferencing stale unrelated IO bindings");
 }
}
}
int main(){try{worker_composition();routing_heap_lifetimes();std::cout<<"PASS: "<<checks<<" socket handler dispatch integration checks; actual027c10/0298b0/IO routing/handler scheduler composition, borrowed and owned branches, injected SDK adapters, fallback queue, independent pool0/1 plus4/5 reuse and actual heap lifetime ends\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
