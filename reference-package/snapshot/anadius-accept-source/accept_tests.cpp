#include "accept.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace s=anadius::socket_recovered;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unexpected(){throw std::logic_error("Unexpected bounded accept dependency");}
template<class E=s::UnsupportedInput,class F>void rejects(F action){bool caught=false;try{action();}catch(const E&){caught=true;}check(caught,"declared failure propagates without invented cleanup");}
struct Events {
 std::vector<std::string> items;std::function<void(std::string_view)> hook;
 void add(std::string_view text){items.emplace_back(text);if(hook)hook(text);}
 std::size_t count(std::string_view name)const{return std::count(items.begin(),items.end(),name);}
 std::size_t at(std::string_view name)const{auto it=std::find(items.begin(),items.end(),name);check(it!=items.end(),"required observable event exists");return static_cast<std::size_t>(it-items.begin());}
};
struct Gate final:s::SocketGate {Events& e;explicit Gate(Events& e):e(e){}void enter()override{e.add("gate.enter");}void leave()override{e.add("gate.leave");}};
struct Entry final:s::IocpSchedulerEntry {using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{unexpected();}};
struct Worker final:s::WorkerRunApi {
 Events& e;void* value=nullptr;std::vector<std::uint32_t> indices;std::function<void*()> get;
 explicit Worker(Events& e):e(e){}void* tls_get(std::uint32_t index)override{indices.push_back(index);e.add("tls");return get?get():value;}
 std::int32_t tls_set(std::uint32_t,void*)override{unexpected();}std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{unexpected();}std::uint32_t get_last_error()override{unexpected();}
};
struct Errors final:s::Api {
 Events& e;std::deque<s::Category> cats;s::Category system{0x2222};int code=17;
 explicit Errors(Events& e):e(e){}s::Category system_category_028620()override{e.add("category");if(cats.empty())return system;auto result=cats.front();cats.pop_front();return result;}
 std::int32_t get_last_error()override{e.add("wsa.error");return code;}
 void set_last_error(std::int32_t)override{unexpected();}s::Conversion convert_acp(const char*,std::int32_t)override{unexpected();}s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unexpected();}s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override{unexpected();}std::int32_t select_write(s::Socket,std::int32_t)override{unexpected();}
};
struct Completion final:s::DispatcherApi {
 Events& e;int result=1;std::vector<OVERLAPPED*> posted;
 explicit Completion(Events& e):e(e){}std::int32_t post(std::uintptr_t port,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* ov)override{check(port==0xabcdef&&bytes==0&&key==2,"accept publication uses port/key2/zero bytes");posted.push_back(ov);e.add("post");return result;}
 void set_last_error(std::uint32_t)override{unexpected();}std::uint32_t get_last_error()override{unexpected();}std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t)override{unexpected();}std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unexpected();}
};
struct Sockets final:s::AcceptorApi {
 Events& e;std::uintptr_t attach_result=1;std::uint32_t win_error=5;std::vector<s::Socket> attached,closed;std::vector<std::uintptr_t> ports;int close_result=0;
 explicit Sockets(Events& e):e(e){}std::uintptr_t attach_completion_port(s::Socket file,std::uintptr_t port)override{attached.push_back(file);ports.push_back(port);e.add("attach");return attach_result;}std::uint32_t get_win32_error()override{e.add("win.error");return win_error;}
 std::int32_t close_socket(s::Socket socket)override{closed.push_back(socket);e.add("close");return close_result;}
 s::Socket create_socket(std::int32_t)override{unexpected();}std::int32_t set_option(s::Socket,std::int32_t,std::int32_t,std::int32_t)override{unexpected();}std::int32_t bind_socket(s::Socket,std::span<const std::byte>,std::int32_t)override{unexpected();}std::int32_t listen_socket(s::Socket,std::int32_t)override{unexpected();}std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unexpected();}
};
struct Api final:s::AsyncAcceptApi {
 Events& e;s::AcceptExResult result{0,997};std::int32_t option_result=0,remote_length=28;
 std::array<std::byte,28> remote{};std::vector<std::pair<s::Socket,s::Socket>> calls,updates;OVERLAPPED* observed=nullptr;std::byte* address=nullptr;
 explicit Api(Events& e):e(e){for(unsigned i=0;i<28;++i)remote[i]=static_cast<std::byte>(i+3);}
 s::AcceptExResult accept(s::Socket listener,s::Socket accepted,std::span<std::byte> bytes,OVERLAPPED& ov)override{calls.emplace_back(listener,accepted);observed=&ov;address=bytes.data();check(bytes.size()==288,"AcceptEx gets full original address buffer");e.add("accept");return result;}
 s::AcceptAddresses addresses(std::span<std::byte> bytes)override{check(bytes.size()==288,"addresses gets bounded original range");e.add("addresses");return {remote,remote_length};}
 std::int32_t update_context(s::Socket accepted,s::Socket& listener)override{updates.emplace_back(accepted,listener);listener=0xbad;e.add("update");return option_result;}
};
struct Table final:s::IoHandlerVtable {
 Events& e;std::string name;std::function<void(s::IoHandlerEnvironment&)> destroy,query;std::function<void(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&)> move;
 Table(Events& e,std::string name,std::uintptr_t identity):IoHandlerVtable(identity),e(e),name(std::move(name)){}
 void destroy_slot_00(s::IoHandlerEnvironment& environment)override{e.add(name+".destroy");if(destroy)destroy(environment);}
 void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{e.add(name+".move");if(move)move(to,from);else{to.payload_00=from.payload_00;to.state_18=from.state_18;}}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment& environment)override{e.add(name+".query");if(query)query(environment);return 0xabc;}
};
struct Task final:s::AcceptTask {
 Events& e;std::vector<std::uint8_t> flags;std::function<void()> callback;
 Task(s::OperationStorage& p,Events& e):AcceptTask(p),e(e){}void call_slot_00(std::uint8_t flag)override{flags.push_back(flag);e.add("task.call");if(callback)callback();}
};
struct Memory final:s::PostedMemoryEffects,s::AsyncAcceptEffects {
 Events& e;alignas(16)std::array<std::byte,1025> raw{},task_raw{};
 s::OperationStorage main{0x10000,raw},task_storage{0x20000,task_raw};std::unique_ptr<s::AsyncAcceptOperation> operation;std::unique_ptr<Task> task;
 s::DispatcherOperationBinding packet;std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;std::vector<s::AcceptHandler> invoked;std::vector<s::CancelOperation*> retried;std::vector<std::uint32_t> families;
 std::function<void(s::IoHandlerEnvironment&,std::uintptr_t)> construct;std::function<void()> reset_owner,reset_socket,invoke_hook,retry_hook,free,projection_hook;std::function<void(s::Error&)> create_error;
 s::Socket fresh=77;s::Category success{0x1111},misc{0x3333};bool delete_projection=true;
 explicit Memory(Events& e):e(e){}
 s::OperationStorage& allocate_04b0bc(std::uint64_t count)override{allocations.push_back(count);e.add("allocate");if(count==473){raw.fill(std::byte{0xa5});return main;}if(count==41){task_raw.fill(std::byte{0xb6});return task_storage;}unexpected();}
 void free_04b788(s::OperationStorage& storage)override{frees.push_back(storage.identity);e.add("free");if(free)free();if(&storage==&main&&delete_projection)operation.reset();}
 s::PostedOperation& project_posted(s::OperationStorage&)override{unexpected();}
 s::AsyncAcceptOperation& project(s::OperationStorage& storage)override{check(&storage==&main,"native accept allocation projected once");e.add("project");operation=std::make_unique<s::AsyncAcceptOperation>(storage);packet={&operation->operation,{-8,{0xdead}},0xface,&operation->operation.overlapped};operation->packet=&packet;operation->operation.queued=8;operation->operation.overlapped.Internal=0x99;operation->address_buffer=std::span<std::byte>(raw).subspan(0x68,288);if(projection_hook)projection_hook();return *operation;}
 s::AcceptTask& project_task(s::OperationStorage& storage)override{check(&storage==&task_storage,"owned accept task uses handler allocation");e.add("task.project");task=std::make_unique<Task>(storage,e);return *task;}
 void construct_environment(s::IoHandlerEnvironment& destination,std::uintptr_t allocator)override{e.add("construct");if(construct)construct(destination,allocator);}
 s::Socket create_socket(std::uint32_t family,std::int32_t type,std::int32_t protocol,s::Error& error)override{families.push_back(family);check(type==1&&protocol==6&&error.value==0&&error.category.name_identity==success.name_identity,"029df0 receives live protocol/type1/TCP6 and constant success error");e.add("create");if(create_error)create_error(error);return fresh;}
 void reset_owned_socket(s::Socket& socket)override{check(socket==s::invalid_socket,"accepted holder initially invalid before029590");e.add("reset.socket");socket=s::invalid_socket;if(reset_socket)reset_socket();}
 void reset_pending_owner(s::AcceptPending& pending)override{check(pending.object.state.socket_state==0x10,"pending transfer setsstate0x10 before237f0");e.add("reset.owner");if(reset_owner)reset_owner();}
 s::Category misc_category()override{e.add("misc");return misc;}s::Category success_category()override{return success;}
 void retry(s::AsyncAcceptOperation&,s::CancelOperation& selected)override{retried.push_back(&selected);e.add("retry");if(retry_hook)retry_hook();}
 void invoke(s::AcceptHandler& handler)override{invoked.push_back(handler);e.add("invoke");if(invoke_hook)invoke_hook();}
};
struct Fixture;Fixture* active=nullptr;
void prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&);
void wrong_prepare(s::IoHandlerEnvironment&,std::uintptr_t,s::IoPrepareScratch&){unexpected();}
void borrowed(s::IoHandlerEnvironment&,s::AcceptBorrowed&);
void owned(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&);
struct Fixture {
 Events events;Gate gate{events};CRITICAL_SECTION cs{};Entry entry{gate,cs},other_entry{gate,cs};
 Worker worker{events};Errors errors{events};Completion completion{events};Sockets sockets{events};Api api{events};Memory memory{events};
 Table original{events,"original",0x111},empty{events,"empty",0x777},temporary{events,"temporary",0x888},alternate{events,"alternate",0x999};
 std::unique_ptr<s::IoHandlerExecutor> executor=std::make_unique<s::IoHandlerExecutor>(s::IoHandlerExecutor{&prepare});s::IoHandlerExecutor sentinel_executor{},nested_executor{};
 std::uint32_t tls_index=17;std::array<s::OperationStorageBinding,2> bindings{{{0x10000,&memory.main},{0x20000,&memory.task_storage}}};
 s::IoCompletionEnvironment environment{{tls_index,bindings},{&empty,0x846e8,&sentinel_executor,0x777}};
 s::AcceptSchedule schedule{&owned,&borrowed};std::array<s::AcceptScheduleBinding,1> schedules{{{0x456,&schedule}}};
 s::AsyncAcceptContext context{environment,worker,memory,api,completion,errors,sockets,memory,schedules};
 s::AcceptorState listener,pending_state,other_pending_state;s::SocketObject pending_object{pending_state},other_object{other_pending_state};
 s::AcceptPending pending{pending_object,&entry},other_pending{other_object,&other_entry};s::AsyncAcceptService service{&entry,0x8765};s::AcceptEndpoint output,other_output;std::uintptr_t server=0x123456789ULL;
 s::WorkerThreadState thread{};s::WorkerTlsContext tls{nullptr,&thread,nullptr};
 std::function<void(s::IoHandlerEnvironment&)> factory;
 std::function<void(s::IoHandlerEnvironment&,s::AcceptBorrowed&)> borrow;
 std::function<void(s::IoHandlerEnvironment&,s::OwnedHandlerTask*&)> publish;
 Fixture(){check(!active,"accept fixtures remain isolated");active=this;entry.completion.port=0xabcdef;other_entry.completion.port=0xabcdef;listener.socket=44;listener.socket_state=4;listener.protocol_40=23;pending_state.socket=s::invalid_socket;other_pending_state.socket=s::invalid_socket;output.bytes.fill(std::byte{0xcc});other_output.bytes.fill(std::byte{0xdd});memory.construct=[&](s::IoHandlerEnvironment& destination,std::uintptr_t allocator){check(allocator==0x987,"allocator identity forwarded to023dd0");destination={{11,12},&original,0,0x456,executor.get()};};}
 ~Fixture(){active=nullptr;}
 void start(s::AcceptEndpoint* target=nullptr){s::start_async_accept_021b00(service,listener,pending,target,server,0x987,context);}
 s::AsyncAcceptOperation& op(){check(memory.operation!=nullptr,"operation projection remains live");return *memory.operation;}
 void complete(std::uintptr_t owner,s::Error error){s::complete_async_accept_0271a0(owner,op(),error,context);}
};
void prepare(s::IoHandlerEnvironment& destination,std::uintptr_t query,s::IoPrepareScratch&){auto& f=*active;check(query==0xabc,"cached factory receives preceding query result");f.events.add("prepare");destination={{77,78},&f.temporary,55,0x456,&f.nested_executor};if(f.factory)f.factory(destination);}
void borrowed(s::IoHandlerEnvironment& environment,s::AcceptBorrowed& descriptor){auto& f=*active;f.events.add("borrowed");check(descriptor.invoke_identity==0x180022500ULL,"borrowed accept descriptor uses022500 identity");if(f.borrow)f.borrow(environment,descriptor);}
void owned(s::IoHandlerEnvironment& environment,s::OwnedHandlerTask*& task){auto& f=*active;f.events.add("owned");if(f.publish)f.publish(environment,task);}

bool zero(const OVERLAPPED& ov){return !ov.Internal&&!ov.InternalHigh&&!ov.Pointer&&!ov.hEvent;}
void initiation(){
 for(std::int32_t result:{0,1,7})for(std::uint32_t error:{0U,64U,1234U,997U}){
  Fixture f;f.api.result={result,error};f.start(&f.output);auto& op=f.op();
  check(f.memory.allocations==std::vector<std::uint64_t>{473}&&f.memory.raw[472]==std::byte{118},"real0269b0/021820 rounds472 to473 and writes capacity118 marker");
  check(op.callback_identity==0x1800271a0ULL&&op.listener==44&&op.pending==&f.pending&&op.protocol==23&&op.no_retry==1&&op.server_this==f.server,"native pre-environment fields captured with exact callback/rawserverptr/bit2 widths");
  check(op.endpoint_output==&f.output&&op.retry_engine_identity==0x8765&&!op.alternate&&!op.retry_context,"native optional output/retry service fields and null alternative initialized");
  check(op.accepted==77&&f.api.calls==std::vector<std::pair<s::Socket,s::Socket>>{{44,77}}&&f.api.observed==&op.operation.overlapped,"SDK receives fresh socket and actual distinct typed OVERLAPPED");
  check(f.api.address==f.memory.raw.data()+0x68&&zero(op.operation.overlapped),"address region matchesraw+68 and realOVERLAPPED startszero");
  for(unsigned i=0;i<sizeof(OVERLAPPED);++i)check(f.memory.raw[i]==std::byte{},"original rawOVERLAPPED range zeroed without using it as typed model");
  check(f.entry.work_count_30==1&&op.operation.queued==1,"accept increments work once and real publication marksqueued");
  const bool started=result!=0||error==997;
  check(f.completion.posted.size()==(started?0U:1U)&&f.events.count("category")==static_cast<unsigned>(!started),"BOOLtrue orFALSE997 uses028c00; immediate failure uses028b50 once");
  check(f.memory.packet.packet_error.value==(started?0:static_cast<std::int32_t>(error))&&f.memory.packet.transferred_14==0,"accept immediate64/1234 errors stayraw and always zero transferred bytes");
  check(f.memory.frees.empty()&&f.memory.invoked.empty(),"start transfers operation ownership without completion or cleanup");
 }
 for(bool pending_open:{false,true}){Fixture f;if(pending_open)f.pending_state.socket=99;else f.listener.socket=s::invalid_socket;f.completion.result=0;f.start();auto& op=f.op();check(f.api.calls.empty()&&f.memory.families.empty()&&op.operation.queued==1,"invalidlistener or already-open pending avoids socketfactory/AcceptEx");check(f.memory.packet.packet_error.value==(pending_open?1:10009)&&f.memory.packet.transferred_14==0,"start invalid/misc checks publish system category exact native status");check(f.entry.completion.head==&op.operation&&f.entry.completion.tail==&op.operation&&f.entry.completion.dispatch_required==1,"failed real post queues operation behind gate and sets dispatch flag");check(f.events.count("gate.enter")==1&&f.events.count("gate.leave")==1,"failed publication acquires/releases queue gate once");}
 for(int error:{0,1234}){Fixture f;f.memory.fresh=s::invalid_socket;f.memory.create_error=[&](s::Error& out){out={error,{0x9999}};};f.start();check(f.api.calls.empty()&&f.memory.packet.packet_error.value==error&&f.memory.packet.packet_error.category.name_identity==0x9999&&f.memory.packet.transferred_14==0,"invalid fresh socket uses028ab0 copy including customcategory/error0 and zero bytes");check(f.events.count("category")==0&&f.completion.posted.size()==1,"copied factory error has no new category lookup");}
 {Fixture f;f.thread.cache_00[0]=f.memory.main.identity;f.memory.raw[0]=std::byte{118};f.worker.value=&f.tls;f.start();check(f.memory.allocations.empty()&&!f.thread.cache_00[0]&&f.memory.raw[472]==std::byte{118},"accept actualallocator reuses currentTLS slot0 capacity118 withoutallocation");}
 {Fixture f;f.memory.construct=[&](auto& env,auto allocator){check(allocator==0x987,"construct allocator forwarded before live reloads");env={{11,12},&f.original,0,0x456,f.executor.get()};check(f.op().listener==44&&f.op().protocol==23&&f.op().server_this==f.server&&f.op().no_retry==1,"pre-environment fields precede callbacks");f.listener.protocol_40=2;f.listener.socket=45;f.listener.socket_state=0;f.server=0x999;};f.memory.reset_socket=[&]{f.listener.socket=46;f.pending_state.socket=0xbeef;f.op().accepted=0xbad;};f.start();check(f.memory.families==std::vector<std::uint32_t>{2}&&f.api.calls==std::vector<std::pair<s::Socket,s::Socket>>{{46,77}},"live protocol/pending tested afterconstruct; listener reloaded aftercreate/reset; fresh accepted stayscached");check(f.op().listener==44&&f.op().protocol==23&&f.op().no_retry==1&&f.op().server_this==0x123456789ULL&&f.op().accepted==77,"initial captured fields surviveconstructmutations andreset-holder mutation overwritten byfresh");}
 {Fixture f;f.memory.construct=[&](auto& env,auto){env={{},&f.original,0,0x456,f.executor.get()};f.pending_state.socket=99;};f.start();check(f.memory.families.empty()&&f.api.calls.empty()&&f.memory.packet.packet_error.value==1,"pending socket reloaded after environmentconstruction");}
 {Fixture f;f.events.hook=[&](std::string_view e){if(e=="construct")throw std::runtime_error("stop");};rejects<std::runtime_error>([&]{f.start();});check(f.memory.allocations.size()==1&&f.entry.work_count_30==0&&f.memory.frees.empty()&&f.api.calls.empty(),"throwing requiredenvironment constructor has no invented tail cleanup/workpublication");}
 for(unsigned kind:{0U,1U,2U}){Fixture f;f.memory.projection_hook=[&]{if(kind==0)f.op().packet=nullptr;else if(kind==1)f.memory.main.bytes=f.memory.main.bytes.first(31);else f.op().address_buffer=f.op().address_buffer.first(287);};rejects([&]{f.start();});check(!f.events.count("construct")&&!f.events.count("create")&&!f.events.count("post")&&f.memory.frees.empty(),"invalidprojectionguardretainsfiniteprefixwithoutfakepublication/EHcleanup");}
}
std::array<std::byte,28> endpoint(const Api& api,bool copy){std::array<std::byte,28> result{};result[0]=std::byte{2};if(copy&&api.remote_length>=0&&api.remote_length<=28)std::copy_n(api.remote.begin(),api.remote_length,result.begin());return result;}
std::array<std::byte,28> pending_endpoint(const s::AcceptorState& state){std::array<std::byte,28> bytes;std::memcpy(bytes.data(),&state.slot_48,8);std::memcpy(bytes.data()+8,&state.slot_50,8);std::memcpy(bytes.data()+16,&state.slot_58,8);std::memcpy(bytes.data()+24,&state.slot_60,4);return bytes;}
void completion(){
 for(int error:{0,64,1234,995,10009,-7})for(bool owner:{false,true}){
  Fixture f;f.start(&f.output);const auto before=f.events.items.size();s::Error original{error,{0x4444}};
  f.memory.invoke_hook=[&]{check(!f.memory.operation&&f.memory.frees==std::vector<std::uintptr_t>{0x10000},"native projection actually deleted before handlercallback");};
  f.complete(owner?9:0,original);
  check(!f.memory.operation&&f.memory.frees==std::vector<std::uintptr_t>{0x10000},"completion releases originalstorage once after moving environment");
  check(original.value==error&&original.category.name_identity==0x4444,"packet error input remainsindependent");
  check(f.events.count("original.move")==1&&f.events.count("empty.destroy")==1&&f.events.count("original.destroy")==1,"sourceenvironment moves then sentinel and local eachdestroy once");
  check(f.memory.invoked.size()==static_cast<std::size_t>(owner)&&f.memory.retried.empty(),"owner0 skipscallback and no_retry prevents995retry");
  check(f.events.at("original.move")<f.events.at("empty.destroy")&&f.events.at("empty.destroy")<f.events.at("free")&&f.events.at("free")<f.events.at("original.destroy"),"sourceenvironment destruction precedes release; local survives untilaftercallback");
  if(owner){
   check(f.memory.invoked.front().server_this==f.server&&f.memory.invoked.front().error.value==(error==64?0x2745:error),"completion delivers rawserver and only64 maps2745");
   check(f.events.at("free")<f.events.at("invoke")&&f.events.at("invoke")<f.events.at("original.destroy"),"free-beforecallback then localenvironmentdestroy order");
   check(f.output.bytes==endpoint(f.api,error==0),"optionalendpoint written for success and error callbacks");
   check(f.events.count("category")==static_cast<unsigned>(error==64?2:1),"owner completion alwayscomparescategory;64 remapcallsitfirst");
  }else{
   check(f.events.count("addresses")==0&&f.events.count("update")==0&&f.events.count("category")==0&&f.events.count("attach")==0,"owner0 skipsalladdress/error-map/category/transferlogic");
   check(std::all_of(f.output.bytes.begin(),f.output.bytes.end(),[](auto b){return b==std::byte{0xcc};}),"owner0 neverwritesoptionalendpoint");
  }
  const bool transferred=owner&&error==0;
  check(f.sockets.closed==(transferred?std::vector<s::Socket>{}:std::vector<s::Socket>{77}),"normal cleanup closes unconsumed live accepted socket only");
  if(transferred){check(f.pending_state.socket==77&&f.pending_state.socket_state==0x10&&f.pending_state.protocol_40==23&&f.pending_state.byte_44==1,"success movesfreshsocket to pending withprotocol/endpoint marker");check(pending_endpoint(f.pending_state)==endpoint(f.api,true),"success writescomplete logical28byteendpoint");}
  check(f.events.items.size()>before,"completion producesobservableeffects");
 }
 for(int length:{-1,0,1,7,16,28,29})for(int option:{0,7})for(int wsa:{0,55}){
  Fixture f;f.api.remote_length=length;f.api.option_result=option;f.errors.code=wsa;f.start(&f.output);f.complete(1,{0,{0x4444}});
  const bool success=option==0||wsa==0;const auto expected=endpoint(f.api,true);
  check(f.output.bytes==expected,"endpoint0/partial28 copies retain defaultfamily2+zero untouchedbytes");
  check(f.api.updates==std::vector<std::pair<s::Socket,s::Socket>>{{77,44}},"update_context runs even remote lengthnegative/29 and usesearlycachedlistener/accepted");
  check(f.memory.invoked.front().error.value==(success?0:55),"successfuloption resets earlier2726; failedoption usesWSAcode includingzero");
  check(f.sockets.attached.size()==static_cast<unsigned>(success)&&f.sockets.closed.size()==static_cast<unsigned>(!success),"optionresultcontrols transfer onlythroughcapturedWSAerror");
  check(f.events.count("wsa.error")==static_cast<unsigned>(option!=0),"WSAGetLastError queried onlyafterfailedcontextoption");
  if(option)check(f.events.at("category")<f.events.at("wsa.error"),"failedcontextoption callscategory beforeWSAerror");
  if(success)check(pending_endpoint(f.pending_state)==expected,"pendingendpoint matchesdefault/partial/copiedlocalendpoint");
 }
 for(std::uintptr_t result:{std::uintptr_t{0},std::uintptr_t{1}})for(std::uint32_t error:{0U,5U,0x80000001U}){
  Fixture f;f.sockets.attach_result=result;f.sockets.win_error=error;f.start(&f.output);f.complete(1,{0,{0x4444}});const bool success=result||!error;
  check(f.sockets.attached==std::vector<s::Socket>{77}&&f.sockets.ports==std::vector<std::uintptr_t>{0xabcdef},"transferuseslivependingcontext port andfullacceptedhandle");
  check(f.memory.invoked.front().error.value==(success?0:std::bit_cast<std::int32_t>(error)),"NULLIOCPcapturesWin32bits;error0stilltransfers");
  check(f.events.count("win.error")==static_cast<unsigned>(!result),"Win32errorreadonlyafterNULLIOCP");
  if(!result){const auto first=f.events.at("win.error");auto category=std::find(f.events.items.begin()+static_cast<std::ptrdiff_t>(first),f.events.items.end(),"category");check(category!=f.events.items.end(),"IOCPGetLastError precedesfollowingcategory");}
  check(f.pending_state.socket==(success?77:s::invalid_socket)&&f.events.count("reset.owner")==static_cast<unsigned>(success),"IOCPfailureonlymovesonzeroerror andownerresetfollowsmove");
 }
 {Fixture f;f.start(&f.output);f.op().accepted=s::invalid_socket;f.complete(1,{0,{0x4444}});check(f.api.updates.empty()&&f.sockets.attached.empty()&&f.memory.invoked.front().error.value==10009&&f.sockets.closed.empty(),"invalidcachedaccepted yields10009 withoutoption/transfer/close");check(f.output.bytes==endpoint(f.api,true),"invalidaccepteds tillwritesdecodedoptionalendpoint");}
 {Fixture f;f.start(&f.output);f.pending_state.socket=123;f.complete(1,{0,{0x4444}});check(f.memory.invoked.front().error.value==1&&f.memory.invoked.front().error.category.name_identity==0x3333&&f.sockets.attached.empty()&&f.sockets.closed==std::vector<s::Socket>{77},"already-openpendingproducesmisc1andclosesunconsumedaccepted");}
 {Fixture f;f.start(&f.output);f.pending_state.socket=123;f.pending.scheduler=nullptr;f.complete(1,{0,{0x4444}});check(f.memory.invoked.front().error.value==1&&f.memory.invoked.front().error.category.name_identity==0x3333&&f.sockets.attached.empty(),"occupiedpendingdoesnotdereferencemissingdestination schedulerandstillproducesmisc1");check(f.sockets.closed==std::vector<s::Socket>{77}&&f.output.bytes==endpoint(f.api,true),"occupiedpending/nullschedulerkeepsnormalendpointwriteandunconsumedacceptedcleanup");}
}
void retry_and_live(){
 for(int error:{64,0x2745})for(bool alternate:{false,true}){
  Fixture f;f.listener.socket_state=0;f.start(&f.output);auto& op=f.op();s::CancelOperation alternate_operation;s::DispatcherOperationBinding alternate_packet{&alternate_operation,{9,{0xdead}},11,&alternate_operation.overlapped};s::AcceptAlternate other{alternate_operation,alternate_packet};
  op.operation.overlapped.Internal=99;op.operation.overlapped.hEvent=reinterpret_cast<HANDLE>(0x55);op.operation.queued=7;f.memory.packet.packet_error={8,{0xdead}};f.memory.packet.transferred_14=9;
  alternate_operation.overlapped.Internal=77;alternate_operation.queued=8;if(alternate)op.alternate=&other;const auto endpoint_before=f.output.bytes;const auto events_before=f.events.items.size();
  f.complete(1,{error,{0x2222}});
  check(f.memory.operation&&f.memory.retried==std::vector<s::CancelOperation*>{alternate?&alternate_operation:&op.operation},"matching2745/no_retry0 callsrequired29660withselectedoperation");
  check(zero(op.operation.overlapped)&&!op.operation.queued&&f.memory.packet.packet_error.value==0&&f.memory.packet.transferred_14==0,"retryresetsactualmainOVERLAPPED queuedandpacketmetadata");
  if(alternate)check(zero(alternate_operation.overlapped)&&!alternate_operation.queued&&alternate_packet.packet_error.value==0&&alternate_packet.transferred_14==0,"retryalsoresetsalternativeOVERLAPPED queuedandpacketmetadata");
  check(f.memory.frees.empty()&&f.memory.invoked.empty()&&f.sockets.closed.empty()&&f.output.bytes==endpoint_before,"retryreturnswithoutcleanup/envmove/output/callback");
  check(std::find(f.events.items.begin()+static_cast<std::ptrdiff_t>(events_before),f.events.items.end(),"original.move")==f.events.items.end(),"retrydoesnotmovehandlerenvironment");
 }
 {Fixture f;f.listener.socket_state=0;f.errors.cats={{0x1111},{0x2222}};f.start();f.complete(1,{64,{0x9999}});check(f.memory.retried.empty()&&f.memory.invoked.front().error.value==0x2745&&f.memory.invoked.front().error.category.name_identity==0x1111,"64mapandlaterunconditionalcategorymayhavedifferentnameidentityandpreventretry");}
 {Fixture f;f.start(&f.output);auto& op=f.op();f.api.remote_length=0;f.events.hook=[&](std::string_view event){if(event=="addresses"){op.accepted=88;op.listener=45;}if(event=="category"){op.accepted=99;op.pending=&f.other_pending;op.endpoint_output=&f.other_output;}};f.memory.reset_owner=[&]{op.protocol=2;op.accepted=100;op.pending=&f.pending;};f.complete(1,{0,{0x4444}});check(f.api.updates==std::vector<std::pair<s::Socket,s::Socket>>{{77,44}},"optionusesaccepted/listenercachedbeforeaddress/categorycallbacks");check(f.sockets.attached==std::vector<s::Socket>{99}&&f.other_pending_state.socket==99&&f.pending_state.socket==s::invalid_socket,"postcategoryreloadsacceptedandpendingdestination;destinationcachedacrossownerreset");check(f.other_pending_state.protocol_40==2&&f.other_pending_state.byte_44==1&&f.sockets.closed.empty(),"protocolreadsliveafterownerreset; consumedacceptedINVALIDoverwritesprovidermutation");check(f.other_output.bytes==endpoint(f.api,true)&&f.output.bytes[0]==std::byte{0xcc},"optionaloutputpointerreadsliveafteralltransfercallbacks");}
 {Fixture f;f.start(&f.output);auto& op=f.op();f.events.hook=[&](std::string_view event){if(event=="category"){op.endpoint_output=&f.other_output;}};f.complete(1,{55,{0x4444}});check(f.other_output.bytes==endpoint(f.api,false)&&f.output.bytes[0]==std::byte{0xcc},"liveoptionaloutputappliesevenfailedcompletion");}
 {Fixture f;f.start();f.empty.destroy=[&](auto&){f.op().accepted=88;f.op().server_this=0x777;};f.original.move=[&](auto& to,auto& from){to.payload_00=from.payload_00;to.state_18=0;f.op().server_this=0x666;};f.complete(0,{64,{0x4444}});check(f.sockets.closed==std::vector<s::Socket>{88}&&f.memory.invoked.empty(),"sourceenvironmentdtorprecedescloseandreloadsacceptedliveevenowner0");}
 {Fixture f;f.start();f.empty.destroy=[&](auto&){check(f.op().accepted==s::invalid_socket,"successfultransferconsumesacceptedbeforeoriginalenvironmentdtor");f.op().accepted=88;};f.complete(1,{0,{0x4444}});check(f.pending_state.socket==77&&f.sockets.closed==std::vector<s::Socket>{88},"cleanupclosesdtorreplacementacceptedwithoutclosingtransferredsocket");}
 {Fixture f;f.start();f.empty.destroy=[&](auto&){f.op().server_this=0x777;};f.original.move=[&](auto& to,auto& from){to.payload_00=from.payload_00;to.state_18=0;f.op().server_this=0x666;};f.complete(1,{55,{0x4444}});check(f.memory.invoked.front().server_this==0x666,"handlerrawservercachedaftermovebeforeoriginalenvironmentdestructor");}
 {Fixture f;f.listener.socket_state=0;f.start();f.complete(1,{0x2745,{0x2222,anadius::server_recovered::ErrorCategory::other}});check(f.memory.retried.size()==1,"retrycomparesopaque+8nameidentityratherthanlogicalenum");}
 {Fixture f;f.listener.socket_state=0;f.start();f.complete(1,{0x2745,{0x9999}});check(f.memory.retried.empty()&&f.memory.invoked.front().error.value==0x2745,"raw2745withmismatchednameidentitydoesnotretry");}
 {Fixture f;f.start();s::Error input{55,{0x4444}};f.original.move=[&](auto& to,auto& from){to.payload_00=from.payload_00;to.state_18=0;input={99,{0x9999}};};s::complete_async_accept_0271a0(1,f.op(),input,f.context);check(f.memory.invoked.front().error.value==55&&f.memory.invoked.front().error.category.name_identity==0x4444,"incomingerrorcopiedbeforemove/providerchangesoriginalpacket");}
}
void scheduling(){
 for(bool guard:{false,true})for(bool borrow:{false,true})for(int returned:{0,1,2}){
  if(borrow&&returned)continue;
  Fixture f;f.start();f.op().environment.state_18=9;if(!borrow)f.schedule.borrowed_slot_18=nullptr;
  f.original.query=[&](auto& environment){check(environment.executor_28==f.executor.get()&&!f.memory.operation,"queryseesoldexecutorafteractualearlyprojectiondelete");f.executor->prepare_slot_40=&wrong_prepare;f.executor.reset();environment.executor_28=nullptr;};
  f.temporary.move=[&](auto& to,auto& from){check(from.operations_10==&f.empty&&from.schedule_20==0x846e8,"nested023cb0resetsinitializedtemporarytablesbeforemovecallback");to.payload_00=from.payload_00;to.state_18=from.state_18;from.executor_28=&f.nested_executor;if(!guard)from.operations_10=&f.alternate;};
  Task replacement(f.memory.task_storage,f.events);
  f.borrow=[&](auto& nested,auto& descriptor){check(nested.operations_10==&f.temporary&&nested.state_18==55&&nested.executor_28==&f.nested_executor,"borrowedschedulergetsactualnestedmovedenvironment");check(descriptor.handler.server_this==0x123456789ULL&&descriptor.handler.error.value==55,"borroweddescriptorcontainscopiedrawserver/error");nested.operations_10=&f.alternate;};
  f.publish=[&](auto& nested,s::OwnedHandlerTask*& slot){check(f.memory.task&&slot==f.memory.task.get()&&f.memory.task->callback_identity==0x180022520ULL,"ownedtaskprojectiongets22520callbackidentityandmutableslot");check(f.memory.task->handler.server_this==0x123456789ULL&&f.memory.task->handler.error.value==55,"ownedtaskcopiesacceptpayload");check(f.memory.allocations==std::vector<std::uint64_t>{473,41}&&f.memory.task_raw[40]==std::byte{10},"real021900allocates41fortaskrequest28andmarkercap10");if(returned==1)slot=nullptr;else if(returned==2)slot=&replacement;nested.operations_10=&f.alternate;};
  f.worker.get=[&]()->void*{if(f.worker.indices.size()==3)f.schedule.owned_slot_10=[](s::IoHandlerEnvironment&,s::OwnedHandlerTask*&){unexpected();};return nullptr;};
  f.complete(1,{55,{0x4444}});
  check(!f.executor&&f.events.count("original.query")==1&&f.events.count("prepare")==1,"cachedfactoryfunctionsurvivesexecutoractualdeleteinquerycallback");
  check(f.events.at("free")<f.events.at("original.query")&&f.events.at("original.query")<f.events.at("prepare"),"factoryquery/factorybothfollowearlyoriginalrelease");
  check(f.events.count("alternate.destroy")==static_cast<unsigned>(guard?1:2)&&f.events.count("original.destroy")==1&&f.events.count("empty.destroy")==1,"temporaryguardconditionalandlivefinalnested/localdestructorsfollowpublication");
  check(f.memory.invoked.empty()&&f.events.count(borrow?"borrowed":"owned")==1,"nonzeroenvironmentstateschedulesexactlyoneacceptdescriptor");
  if(borrow){check(f.worker.indices.size()==2&&f.memory.allocations==std::vector<std::uint64_t>{473}&&!f.memory.task,"borrowedslot18priorityavoidsownedallocation/TLS");}
  else{check(f.worker.indices.size()==3&&f.memory.task,"ownedslot10cachedbeforeTLSlookupandallocatesclosure");check(f.memory.task->flags==(returned==0?std::vector<std::uint8_t>{0}:std::vector<std::uint8_t>{}),"ownedpublisherresidualoriginalslotdiscardedflag0onlyifstillreturned");check(replacement.flags==(returned==2?std::vector<std::uint8_t>{0}:std::vector<std::uint8_t>{}),"returnedreplacementratherthanentrytaskgetsflag0");}
 }
 {Fixture f;f.start();f.memory.invoke_hook=[](){throw std::runtime_error("callback");};rejects<std::runtime_error>([&]{f.complete(1,{55,{0x4444}});});check(!f.memory.operation&&f.memory.frees.size()==1&&!f.events.count("original.destroy"),"throwingrequiredhandlerpropagatesafterearlyfreewithoutinventedlocaldestroy");}
 {Fixture f;f.start();f.op().environment.state_18=1;f.original.query=[](auto&){throw std::runtime_error("query");};rejects<std::runtime_error>([&]{f.complete(1,{55,{0x4444}});});check(!f.memory.operation&&f.memory.frees.size()==1&&!f.events.count("prepare")&&!f.events.count("original.destroy"),"throwingqueryhasboundednativeprefixwithoutfabricatedtail/EHrollback");}
 {Fixture f;f.start();f.memory.delete_projection=false;f.op().environment.operations_10=nullptr;rejects([&]{f.complete(1,{55,{0x4444}});});check(f.memory.frees.empty(),"missingrequiredhandlerenvironmentfailsbeforeinventedrelease");}
}
struct NativeSpy {
 std::vector<std::string> trace;s::Socket listener=0x123456789abcdef0ULL,accepted=0xfedcba9876543210ULL;std::span<std::byte> buffer;OVERLAPPED* ov=nullptr;
 int result=1,error=64,option=0,length=28;std::ptrdiff_t offset=144;bool null_remote=false;
};
NativeSpy* spy=nullptr;
BOOL PASCAL fake_accept(SOCKET listener,SOCKET accepted,PVOID buffer,DWORD receive,DWORD local,DWORD remote,LPDWORD bytes,LPOVERLAPPED ov){auto& f=*spy;f.trace.push_back("accept");check(listener==f.listener&&accepted==f.accepted&&buffer==f.buffer.data()&&ov==f.ov,"SDKAcceptExexactfullwidthsocket/address/realOVERLAPPEDidentities");check(receive==0&&local==0x90&&remote==0x90&&bytes&&*bytes==0,"SDKAcceptExzero receive length/0x90bothaddresslengths/outDWORDinitializedzero");*bytes=0xdeadbeef;return f.result;}
int WSAAPI fake_error(){spy->trace.push_back("error");return spy->error;}
void PASCAL fake_addresses(PVOID buffer,DWORD receive,DWORD local,DWORD remote,sockaddr** local_address,int* local_size,sockaddr** remote_address,int* remote_size){auto& f=*spy;f.trace.push_back("addresses");check(buffer==f.buffer.data()&&!receive&&local==0x90&&remote==0x90,"SDKaddressdecoderusesoriginalbuffer/lengthconstants");check(local_address&&local_size&&remote_address&&remote_size&&!*local_address&&!*local_size&&!*remote_address&&!*remote_size,"SDKaddressoutpointersandlengthsinitializedzero");*local_address=reinterpret_cast<sockaddr*>(f.buffer.data());*local_size=16;*remote_address=f.null_remote?nullptr:reinterpret_cast<sockaddr*>(reinterpret_cast<std::uintptr_t>(f.buffer.data())+f.offset);*remote_size=f.length;}
int WSAAPI fake_option(SOCKET accepted,int level,int option,const char* value,int size){auto& f=*spy;f.trace.push_back("option");check(accepted==f.accepted&&level==0xffff&&option==0x700b&&size==8,"SO_UPDATE_ACCEPT_CONTEXTexactsocket/level/name/eightbytevalue");s::Socket listener=0;check(value!=nullptr,"contextoptionhasinitializedlistenerpointer");std::memcpy(&listener,value,8);check(listener==f.listener,"contextoptioncontainsfullcachedlistener");return f.option;}
void sdk(){
 std::array<std::byte,288> bytes{};OVERLAPPED ov{};
 for(int result:{0,1,7})for(int error:{0,64,997,1234}){NativeSpy f;spy=&f;f.buffer=bytes;f.ov=&ov;f.result=result;f.error=error;s::AsyncAcceptImports imports;imports.accept=&fake_accept;imports.error=&fake_error;s::NativeAsyncAcceptApi api(imports);auto value=api.accept(f.listener,f.accepted,bytes,ov);check(value.result==result&&value.last_error==static_cast<std::uint32_t>(error)&&f.trace==std::vector<std::string>{"accept","error"},"SDKwrapperunconditionallyreadsWSAerrorincludingBOOLtrueandignoresoutbytes");}
 for(int length:{-1,0,1,16,28,29}){NativeSpy f;spy=&f;f.buffer=bytes;f.length=length;f.null_remote=length==0;s::AsyncAcceptImports imports;imports.addresses=&fake_addresses;s::NativeAsyncAcceptApi api(imports);auto address=api.addresses(bytes);check(address.remote_length==length&&address.remote.size()==static_cast<std::size_t>(length<0||length>28?0:length),"SDKaddresswrapperretainsrawnegative/29lengthbutboundspositive<=28span");if(length>0&&length<=28)check(address.remote.data()==bytes.data()+144,"decoderremoteendpointspanborrowsactualbufferwithoutcopy");}
 for(int offset:{-1,280,289}){NativeSpy f;spy=&f;f.buffer=bytes;f.offset=offset;f.length=28;s::AsyncAcceptImports imports;imports.addresses=&fake_addresses;s::NativeAsyncAcceptApi api(imports);rejects([&]{api.addresses(bytes);});}
 for(int status:{-1,0,7}){NativeSpy f;spy=&f;f.option=status;s::AsyncAcceptImports imports;imports.option=&fake_option;s::NativeAsyncAcceptApi api(imports);auto listener=f.listener;check(api.update_context(f.accepted,listener)==status&&f.trace==std::vector<std::string>{"option"},"contextoptionwrapperpreservesrawstatuswithoutprematureerrorlookup");}
 {NativeSpy f;spy=&f;f.buffer=bytes;f.ov=&ov;s::AsyncAcceptImports imports;imports.accept=&fake_accept;imports.error=&fake_error;imports.addresses=&fake_addresses;s::NativeAsyncAcceptApi api(imports);rejects([&]{api.accept(f.listener,f.accepted,std::span(bytes).first(287),ov);});rejects([&]{api.addresses(std::span(bytes).first(287));});check(f.trace.empty(),"SDKinvalidboundedbufferguardsinvoke noforeignfunctions");}
 for(int result:{0,1})for(int error:{64,1234,997}){Fixture fixture;NativeSpy f;spy=&f;f.listener=44;f.accepted=77;f.result=result;f.error=error;f.buffer=std::span(fixture.memory.raw).subspan(0x68,288);fixture.memory.construct=[&](auto& environment,auto allocator){check(allocator==0x987,"SDKcompositionallocatoridentity");environment={{11,12},&fixture.original,0,0x456,fixture.executor.get()};f.ov=&fixture.op().operation.overlapped;};s::AsyncAcceptImports imports;imports.accept=&fake_accept;imports.error=&fake_error;s::NativeAsyncAcceptApi api(imports);s::AsyncAcceptContext context{fixture.environment,fixture.worker,fixture.memory,api,fixture.completion,fixture.errors,fixture.sockets,fixture.memory,fixture.schedules};s::start_async_accept_021b00(fixture.service,fixture.listener,fixture.pending,nullptr,fixture.server,0x987,context);const bool started=result!=0||error==997;check(fixture.memory.packet.transferred_14==0&&fixture.memory.packet.packet_error.value==(started?0:error),"actualNativeAcceptExoutDEADBEEFdoesnotleakintofailuremetadata;64/1234remainraw");check(f.trace==std::vector<std::string>{"accept","error"}&&fixture.completion.posted.size()==static_cast<unsigned>(!started),"freshnative021b00andSDKwrappercomposewithoutactualnetwork");}
 spy=nullptr;
}
}
int main(){try{initiation();completion();retry_and_live();scheduling();sdk();std::cout<<"PASS: "<<checks<<" native accept core checks; retained021b00/0271a0 flow, actual allocator/publication helpers, bounded SDK imports, live reloads and early projection deletion\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
