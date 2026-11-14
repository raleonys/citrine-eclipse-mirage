// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_io_completion_dispatch.hpp"
#include "laureondo/net/transport_composed_write_native.hpp"
#include "laureondo/net/transport_queued_write.hpp"
#include "laureondo/net/transport_completion_dispatcher_native.hpp"
#include "laureondo/net/transport_worker_run_native.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
namespace s=laureondo::socket;
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
struct Effects:s::FinishedWriteEffects{
 using FinishedWriteEffects::FinishedWriteEffects;
 s::AsyncSendOperation& project_async_send(s::OperationStorage&)override;


 s::Category misc_category_0285b0()override{return {0x2222};}
 void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unexpected();}
};struct Fixture;Fixture* active=nullptr;std::uintptr_t constructor_type();void construct_environment(s::IoHandlerEnvironment&,std::uintptr_t,s::IoEnvironmentConstructorScratch&);struct Types:s::IoEnvironmentTypeApi{std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)override;};int WSAAPI sdk_send(SOCKET,LPWSABUF,DWORD,LPDWORD,DWORD,LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE);int WSAAPI sdk_error();
void prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&);void borrow(s::IoHandlerEnvironment&,s::BorrowedIoHandler&);void publish(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&);
LPVOID WINAPI tls_get(DWORD);BOOL WINAPI tls_set(DWORD,LPVOID);BOOL WINAPI post(HANDLE,DWORD,ULONG_PTR,LPOVERLAPPED);VOID WINAPI set_error(DWORD);DWORD WINAPI get_error();BOOL WINAPI get(HANDLE,LPDWORD,PULONG_PTR,LPOVERLAPPED*,DWORD);BOOL WINAPI timer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,LPVOID,BOOL);
struct Node{std::array<std::byte,1025> bytes{};s::OperationStorage storage;std::unique_ptr<s::AsyncSendOperation> async;std::unique_ptr<s::ReceiveIoOperation> receive;std::unique_ptr<s::SendIoOperation> send;std::unique_ptr<s::PostedOperation> posted;std::unique_ptr<s::ReceiveHandlerTask> receive_task;std::unique_ptr<s::SendHandlerTask> send_task;explicit Node(std::uintptr_t id):storage{id,bytes}{}};
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
struct Worker:s::DispatcherWorkerEffects{
 Fixture& f;explicit Worker(Fixture&);
 std::uint32_t exchange_stop_34(s::IocpSchedulerEntry&,std::uint32_t)override;
 std::uint32_t exchange_wake_38(s::IocpSchedulerEntry&,std::uint32_t)override;
 void initialize_04a240(std::array<std::uintptr_t,2>&)override;void finish_04a2cc(std::array<std::uintptr_t,2>&)override;
 void free_cache_04b788(std::uintptr_t)override;void delete_function_04b0f8(s::IocpWorkerFunction&,std::uint32_t)override{unexpected();}
 [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unexpected();}
};
struct Packet{OVERLAPPED* identity=nullptr;DWORD bytes=0,error=0;ULONG_PTR key=0;};
struct Functor:s::PostedFunctor{unsigned invokes=0,discards=0;std::function<void()> action;void call_slot_00(std::uint8_t flag)override{if(flag){++invokes;if(action)action();}else ++discards;}};
struct FinishCategory:s::WriteFinishCategory{void message_slot_10(s::WriteFinishString&,std::int32_t)override;};
struct FinishApi:s::QueuedWriteApi{
 Fixture& f;s::WriteFinishStream initial{0x100,0},returned{0x200,-8};explicit FinishApi(Fixture& v,s::QueuedWriteContext& c):QueuedWriteApi(c),f(v){}
 void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
 [[noreturn]]void invalid_parameter_05c974()override{unexpected();}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override;
 s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override;
 std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override;
 void put_00fe90(s::WriteFinishStream&,std::uint8_t)override;
 void flush_00e1e0(s::WriteFinishStream&)override;
};
struct Terminal:s::QueuedWriteTerminal{[[noreturn]]void bad_weak_ptr_026860()override{unexpected();}};
struct OwnerRef:s::IoReference{unsigned disposes=0,deletes=0;void dispose_slot_00()override{++disposes;}void delete_slot_08()override{++deletes;}};
struct Fixture{
 Gate gate;CRITICAL_SECTION cs{};Entry entry{gate,cs};Errors errors;Table original{0x111},empty{0x777};s::IoHandlerExecutor executor{&prepare};std::uint32_t index=17;std::int32_t guard=0;
 std::array<s::OperationStorageBinding,8> storage_bindings{};std::array<s::PostedOperationBinding,8> posted_bindings{};std::array<s::IoCompletionBinding,8> io_bindings{};std::array<s::DispatcherOperationBinding,8> dispatcher_bindings{};
 s::DispatcherState state{entry,nullptr,0,0,dispatcher_bindings};
 s::NativeWorkerRunApi worker_api{{&tls_get,&tls_set,&post,&get_error}};s::NativeDispatcherApi api{{&set_error,&get_error,&get,&post,&timer}};
 Memory memory{*this};s::HandlerSchedule schedule{&publish,nullptr};std::array<s::HandlerScheduleBinding,1> schedules{{{0x456,&schedule}}};s::NativeAsyncSendApi send_api{{&sdk_send,&sdk_error}};s::AsyncSendService service{&entry};s::SendSocketImplementation implementation{77,0,0,nullptr};std::array<s::SendStreamBinding,1> streams{{{0x456,&service,&implementation,0x4be}}};Types types;std::array<s::IoEnvironmentTypeBinding,1> type_queries{{{0x456,&constructor_type}}};std::array<s::IoEnvironmentFactoryBinding,1> factories{{{&executor,&construct_environment}}};s::IoEnvironmentConstructorContext constructor{types,type_queries,factories};s::IoHandlerEnvironment source_environment{{51,52},&original,4,0x456,&executor};std::array<s::IoEnvironmentSourceBinding,1> sources{{{0x4be,&source_environment}}};s::WriteFinishString queue_item,second_item;std::array<s::WriteFinishString*,8> queue_map{};s::WriteFinishOwner queue_owner{queue_map,8,3,2};std::array<s::WriteFinishOwnerBinding,1> finish_owners{{{0x446,&queue_owner}}};FinishCategory finish_category;std::array<s::WriteFinishCategoryBinding,1> finish_categories{{{{0x2222},&finish_category}}};std::array<s::WriteFinishAllocationHeader,0> finish_headers{};OwnerRef owner_control;Terminal terminal;s::QueuedWriteOwner queued_owner{0x1357,&owner_control,&queue_owner,&streams[0]};std::array<s::QueuedWriteOwnerBinding,1> queued_bindings{{{0x446,&queued_owner}}};s::QueuedWriteContext queued_context{queued_bindings,terminal,nullptr};FinishApi finish_api{*this,queued_context};Effects effects{environment(),worker_api,memory,schedules,send_api,api,errors,streams,constructor,sources,finish_api,finish_owners,finish_categories,finish_headers};Routing routing{*this};Worker worker{*this};s::PostedExecutor posted_executor{0x1234ab,&entry,1};
 void* tls=nullptr;DWORD error=0;std::vector<Packet> packets;std::size_t cursor=0;unsigned posts=0,gets=0,tls_gets=0,tls_sets=0;bool fail_first=false;std::vector<std::string> events;unsigned published=0,borrowed=0,prepared=0,send_calls=0,finishes=0;bool early=false,heap_finish=false;std::uint64_t finished_progress=0;std::int32_t finished_error=0;std::vector<std::uint32_t> lengths;std::vector<std::uintptr_t> pointers;std::vector<std::string> sdk_events;std::uint32_t completion_error=0;int sdk_result=SOCKET_ERROR,sdk_last_error=WSA_IO_PENDING;
 Fixture(){active=this;queued_context.write=&effects.write_context();queue_map.fill(&queue_item);queue_map[4]=&second_item;second_item.size_10=7;second_item.storage_00[0]=0x62;queue_item.capacity_18=16;queue_item.size_10=150000;queue_item.storage_00[2]=1;finish_category.identity_08=0x2222;entry.completion.port=0xabcdef1234ULL;entry.timeout_40=0xffffffffU;}
 s::IoCompletionEnvironment environment(){return {{index,storage_bindings},{&empty,0x846e8,&executor,0x777}};}
 Node& receive(){auto& n=memory.add();n.receive=std::make_unique<s::ReceiveIoOperation>(s::ReceiveIoOperation{n.storage,{},0x10,nullptr,99,{},{{11,12},&original,0,0x456,&executor}});n.receive->handler.object_a0=0x1234;n.bytes[0xe0]=std::byte{56};bind(n);return n;}
 Node& send(){auto& n=memory.add();n.send=std::make_unique<s::SendIoOperation>(s::SendIoOperation{n.storage,{},nullptr,{},{{21,22},&original,0,0x456,&executor}});n.send->handler.object_90=0x9876;n.bytes[0xd0]=std::byte{52};bind(n);return n;}
 void bind(Node& n){for(std::size_t i=0;i<memory.nodes.size();++i)if(memory.nodes[i].get()==&n){auto* op=n.async?&n.async->io.operation:n.receive?&n.receive->operation:n.send?&n.send->operation:&n.posted->operation;io_bindings[i]={op,n.receive.get(),n.async?&n.async->io:n.send.get()};posted_bindings[i]={op,n.posted.get()};dispatcher_bindings[i]={op,{},0,&op->overlapped};return;}unexpected();}
 void submit(s::PostedFunctor*& source){s::dispatch_posted_022e40(posted_executor,source,environment().storage,worker_api,api,memory);}
 std::uint64_t run(){s::WorkerRunResult result;const auto count=s::run_worker_0298b0(entry,result,{index,{0x1111}},worker_api,errors,worker);check(result.error&&result.error->value==0&&result.error->category.name_identity==0x1111,"actual worker source returns logical generic success");return count;}
};
void FinishApi::free_sized_04b0f8(std::uintptr_t p,std::uint64_t bytes){f.events.push_back("stringfree");check(p==0x10000&&bytes==17,"actual025a70 frees synthetic queued heap string by exact sized arguments");check(f.queue_item.size_10==150000&&f.queue_owner.count_e8==2,"string free precedes actual queue reset/pop");}
s::WriteFinishStream& FinishApi::begin_log_006880(std::uintptr_t global,std::uintptr_t prefix){f.events.push_back("beginlog");check(global==0x1800a4130ULL&&prefix==0x1800845d8ULL,"actual025a70 forwards exact log global and prefix");return initial;}
void FinishCategory::message_slot_10(s::WriteFinishString& v,std::int32_t code){auto& f=*active;++f.finishes;f.finished_error=code;f.events.push_back("message");v.size_10=4;v.capacity_18=15;v.storage_00[0]='e';}
s::WriteFinishStream& FinishApi::append_007990(s::WriteFinishStream& stream,std::uintptr_t data,std::uint64_t size){f.events.push_back("appendlog");check(&stream==&initial&&data&&size==4,"actual025a70 appends constructed inline category message");return returned;}
std::uint64_t FinishApi::widen_01e510(std::uintptr_t stream,std::uint8_t c){f.events.push_back("widenlog");check(stream==0x1f8&&c==10,"actual025a70 uses returned stream with negative virtual offset");return 0x3ff;}
void FinishApi::put_00fe90(s::WriteFinishStream& stream,std::uint8_t c){f.events.push_back("putlog");check(&stream==&returned&&c==255,"actual025a70 truncates widen to character byte");}
void FinishApi::flush_00e1e0(s::WriteFinishStream& stream){f.events.push_back("flushlog");check(&stream==&returned,"actual025a70 flushes returned stream");}
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
s::PostedOperation& Memory::project_posted(s::OperationStorage& storage){for(auto& node:nodes)if(node&&&node->storage==&storage){node->async.reset();node->receive.reset();node->send.reset();if(!node->posted)node->posted=std::make_unique<s::PostedOperation>(s::PostedOperation{storage,{},0,nullptr});f.bind(*node);return *node->posted;}unexpected();}
LPVOID WINAPI tls_get(DWORD index){auto& f=*active;check(index==f.index,"existing native TLS getter forwards live global index");++f.tls_gets;return f.tls;}
BOOL WINAPI tls_set(DWORD index,LPVOID pointer){auto& f=*active;check(index==f.index,"existing native TLS setter forwards live index");++f.tls_sets;f.tls=pointer;return TRUE;}
BOOL WINAPI post(HANDLE port,DWORD bytes,ULONG_PTR key,LPOVERLAPPED raw){auto& f=*active;check(reinterpret_cast<std::uintptr_t>(port)==f.entry.completion.port&&!bytes&&(key==0||key==2),"existing native publication forwards live port, zero SDK bytes and key0/2");++f.posts;if(f.fail_first&&raw){f.fail_first=false;return FALSE;}f.packets.push_back({raw,0,0,key});return TRUE;}VOID WINAPI set_error(DWORD error){check(!error,"dispatcher resets last error before injected wait");active->error=error;}
DWORD WINAPI get_error(){return active->error;}
BOOL WINAPI get(HANDLE port,LPDWORD bytes,PULONG_PTR key,LPOVERLAPPED* op,DWORD timeout){auto& f=*active;check(reinterpret_cast<std::uintptr_t>(port)==f.entry.completion.port&&(timeout==0xffffffffU||timeout==0),"actual worker waits on same port with infinite timeout");check(bytes&&key&&op&&!(*bytes)&&!(*key)&&!(*op),"SDK adapter initializes completion outputs");if(f.cursor==f.packets.size()){check(timeout==0,"only early handshake poll may exhaust packet queue");f.error=258;return FALSE;}const auto packet=f.packets[f.cursor++];*op=packet.identity;*bytes=packet.bytes;*key=packet.key;f.error=packet.error;++f.gets;return TRUE;}
BOOL WINAPI timer(HANDLE,const LARGE_INTEGER*,LONG,PTIMERAPCROUTINE,LPVOID,BOOL){unexpected();}
s::ReceiveHandlerTask& Memory::project_receive_task(s::OperationStorage& storage,s::HandlerTaskContext& context){for(auto& node:nodes)if(node&&&node->storage==&storage){node->send_task.reset();node->receive_task=std::make_unique<s::ReceiveHandlerTask>(storage,context);return *node->receive_task;}unexpected();}
s::SendHandlerTask& Memory::project_send_task(s::OperationStorage& storage,s::HandlerTaskContext& context){for(auto& node:nodes)if(node&&&node->storage==&storage){node->receive_task.reset();node->send_task=std::make_unique<s::SendHandlerTask>(storage,context);return *node->send_task;}unexpected();}
void prepare(s::IoHandlerEnvironment& environment,std::uintptr_t query,s::IoPrepareScratch&){auto& f=*active;check(query==0xabc,"nested send factory receives original query result");++f.prepared;environment={{31,32},&f.original,0,0x456,&f.executor};}
void borrow(s::IoHandlerEnvironment&,s::BorrowedIoHandler& descriptor){auto& f=*active;++f.borrowed;descriptor.invoke(f.effects);}
void publish(s::IoHandlerEnvironment&,s::OwnedHandlerTask*& slot){auto& f=*active;++f.published;check(slot!=nullptr,"recovered handler scheduler receives owned closure");auto* task=slot;slot=nullptr;task->call_slot_00(1);}

s::AsyncSendOperation& Effects::project_async_send(s::OperationStorage& storage){auto& f=*active;for(auto& node:f.memory.nodes)if(node&&&node->storage==&storage){node->receive.reset();node->send.reset();node->posted.reset();node->async=std::make_unique<s::AsyncSendOperation>(storage);f.bind(*node);for(std::size_t i=0;i<f.memory.nodes.size();++i)if(f.memory.nodes[i].get()==node.get()){node->async->packet=&f.dispatcher_bindings[i];return *node->async;}}unexpected();}
std::uintptr_t constructor_type(){active->events.push_back("constructor_type");return 0x1000;}
std::int32_t Types::compare_type_names_04d2d0(std::uintptr_t first,std::uintptr_t second){check(first==0x1008&&second==0x1800a25c8ULL,"actual023dd0 invokes required comparator with exact opaque identities");active->events.push_back("constructor_compare");return -1;}
void construct_environment(s::IoHandlerEnvironment& environment,std::uintptr_t query,s::IoEnvironmentConstructorScratch&){auto& f=*active;check(query==0xabc,"actual023dd0 cached native factory receives slot18 query value");f.events.push_back("constructor_factory");environment={{21,22},&f.original,1,0x456,&f.executor};}
int WSAAPI sdk_send(SOCKET socket,LPWSABUF buffers,DWORD count,LPDWORD bytes,DWORD flags,LPWSAOVERLAPPED overlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE callback){auto& f=*active;check(socket==77&&buffers&&count==1&&bytes&&!*bytes&&!flags&&overlapped&&!callback,"exact SDK WSASend receives one buffer, zero initialized DWORD and null completion routine");f.sdk_events.push_back("send");++f.send_calls;f.lengths.push_back(buffers[0].len);f.pointers.push_back(reinterpret_cast<std::uintptr_t>(buffers[0].buf));*bytes=buffers[0].len;f.packets.push_back({overlapped,buffers[0].len,f.completion_error,0});if(f.early){s::WorkerThreadState local{};s::WorkerRunResult result;check(f.worker.dispatch_027c10(f.entry,0,local,result)==0&&result.error&&result.error->value==0,"actual dispatcher consumes early packet by queued0-to1 then zero-timeout returns without callback");check(!f.finishes,"early packet cannot invoke final completion before send returns");}return f.sdk_result;}
int WSAAPI sdk_error(){auto& f=*active;check(!f.sdk_events.empty()&&f.sdk_events.back()=="send","WSAGetLastError occurs immediately and unconditionally after SDK send");f.sdk_events.push_back("error");return f.sdk_last_error;}

void writes(){for(bool borrowed:{false,true})for(bool early:{false,true})for(bool fallback:{false,true})for(bool zero_second:{false,true}){Fixture f;f.early=early;f.fail_first=fallback;f.memory.delete_on_free=true;if(borrowed)f.schedule.borrowed_slot_18=&borrow;if(zero_second)f.second_item.size_10=0;
 s::start_queued_write_02a500(0x446,f.queued_context);check(f.owner_control.strong_08==2&&f.entry.work_count_30==1&&!f.finishes,"actual02a500 acquires shared owner and actual021fd0 owns it before callback");const bool loses_metadata=early&&fallback;const auto first_chunks=loses_metadata?1U:3U,chunks=first_chunks+1U;const auto second_pointer=zero_second?0:reinterpret_cast<std::uintptr_t>(f.second_item.storage_00.data());const auto handled=f.run();
 check(handled==chunks&&f.queue_owner.count_e8==0&&f.queue_owner.offset_e0==0&&!f.finished_error,"actual02a500/025a70 advances two distinct queued messages then empty initiation returns");check(f.queue_item.size_10==0&&f.second_item.size_10==0&&f.queue_item.capacity_18==15&&f.second_item.capacity_18==15,"actual finish resets both queued string representations");
 const std::vector<std::uint32_t> expected_lengths=loses_metadata?std::vector<std::uint32_t>{65536,zero_second?0U:7U}:std::vector<std::uint32_t>{65536,65536,18928,zero_second?0U:7U};const std::vector<std::uintptr_t> expected_pointers=loses_metadata?std::vector<std::uintptr_t>{0x10000,second_pointer}:std::vector<std::uintptr_t>{0x10000,0x20000,0x30000,second_pointer};check(f.lengths==expected_lengths&&f.pointers==expected_pointers,"actual queue start uses selected string then exact max64KiB continuation; zero second data null");
 check(f.send_calls==chunks&&f.sdk_events.size()==chunks*2,"each actual send invokes exactly injected send/error SDK pair");for(unsigned i=0;i<chunks;++i)check(f.sdk_events[i*2]=="send"&&f.sdk_events[i*2+1]=="error","SDK last error follows each send immediately and unconditionally");
 check(std::count(f.events.begin(),f.events.end(),"constructor_factory")==chunks&&std::count(f.events.begin(),f.events.end(),"constructor_type")==chunks,"each actual queued and continuation send uses recovered023dd0");check(f.published==(borrowed?0U:chunks)&&f.borrowed==(borrowed?chunks:0U)&&f.prepared==chunks,"both queued messages use real borrowed or owned recovered handler scheduler");
 check(f.memory.allocations==std::vector<std::uint64_t>(borrowed?std::initializer_list<std::uint64_t>{209}:std::initializer_list<std::uint64_t>{209,113}),"IO cache0 and separate handler cache4 reused across slices and next queued message");check(f.owner_control.strong_08==1&&f.owner_control.weak_0c==1&&!f.owner_control.disposes&&!f.owner_control.deletes,"each queue initiation reference transfers through all continuations then releases to original owner");check(f.memory.frees.size()==(borrowed?1U:2U)&&!f.tls&&!f.entry.work_count_30&&f.entry.high_34==1&&!f.gate.depth&&!f.entry.completion.head&&!f.entry.completion.tail,"worker restores TLS and ends final cached operation/closure actual heap lifetimes");check(f.posts>=(early?chunks:0U)&&!f.routing.unknown,"early key2 preserving publication and key0 fallback remain existing native evidence semantics");}}
void completion_error(){for(std::uint32_t code:{1234U,10054U})for(bool borrowed:{false,true}){Fixture f;f.completion_error=code;f.memory.delete_on_free=true;if(borrowed)f.schedule.borrowed_slot_18=&borrow;s::start_queued_write_02a500(0x446,f.queued_context);check(f.run()==1&&f.finishes==1&&f.queue_owner.count_e8==2&&f.finished_error==(code==1234?10061:static_cast<std::int32_t>(code)),"actual queued callback error mapping reaches real finish without pop or next message");check(f.send_calls==1&&f.owner_control.strong_08==1,"failed first message releases acquired owner without next queued submission");check(std::find(f.events.begin(),f.events.end(),"flushlog")!=f.events.end(),"actual finish error emits required category message and flush");}}
void suppression(){Fixture f;f.completion_error=2;f.schedule.borrowed_slot_18=&borrow;s::start_queued_write_02a500(0x446,f.queued_context);check(f.run()==1&&!f.finishes&&f.queue_owner.count_e8==2&&f.owner_control.strong_08==1,"actual completed send code2 matching category suppresses log, releases owner and never pops queue");check(std::find(f.events.begin(),f.events.end(),"beginlog")==f.events.end(),"silent error never enters required log API");}
}
int main(){try{writes();completion_error();suppression();std::cout<<"PASS: "<<checks<<" socket queued write integration checks; actual02a500/021fd0/0261d0/025a70/02a500 cycle for two distinct messages, 64KiB slices, shared owner transfer and release, borrowed/owned scheduling, zero second item, cache reuse, early key2 and fallback key0 evidence semantics, mapped errors and category suppression; injected SDK only, actual synthetic heap lifetime ends\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
