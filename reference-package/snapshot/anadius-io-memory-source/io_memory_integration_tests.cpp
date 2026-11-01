#include "io_memory.hpp"
#include "../anadius-socket-source/async_receive_test_fixture.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include "../anadius-socket-source/message_dispatch.hpp"
#include "../anadius-handlers-source/handlers.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <algorithm>
#include <exception>
#include <optional>

namespace {
namespace s=anadius::socket_recovered;namespace m=anadius::messages_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n';throw std::runtime_error(text);}}
[[noreturn]]void unused(){throw std::logic_error("Unselected owner integration dependency");}
void weak_release(s::IoReference* value){if(value&&value->weak_0c.fetch_sub(1)==1)value->delete_slot_08();}
void strong_release(s::IoReference* value){if(value&&value->strong_08.fetch_sub(1)==1){value->dispose_slot_00();weak_release(value);}}
struct Cancel:s::Control {
    unsigned disposed=0,deleted=0;Cancel(){strong=1;weak=1;resource=0x9876;}
    void destroy_resource()override{++disposed;}void delete_control()override{++deleted;}
};
struct EmptyControl:s::ControlFactory {s::Control* allocate_04b0bc(std::uint32_t)override{return nullptr;}};
struct Gate:s::SocketGate {bool held=false;void enter()override{if(held)unused();held=true;}void leave()override{if(!held)unused();held=false;}};
struct Life:s::LifecycleEffects {
    s::SocketService& service;std::vector<s::Socket> closed;unsigned cleanups=0;
    std::function<void(s::SocketObject&)> before_close;
    explicit Life(s::SocketService& v):service(v){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService& selected,s::SocketObject& object)override{
        if(&selected!=&service)unused();
        if(before_close)before_close(object);
        if(object.state.socket!=s::invalid_socket){closed.push_back(object.state.socket);object.state.socket=s::invalid_socket;}
        EmptyControl empty;s::replace_owner_0237f0(object.cancellation,0,empty);
    }
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{++cleanups;}
};
// Lifecycle provider is an alias into a root owning the service and gate.
// SessionMemoryDomain retains this whole root, not a dangling borrowed service.
struct ServiceRoot {Gate gate;s::SocketService service{gate};Life life{service};};
// B6's startup-only HeapAlloc spy requires HEAP_ZERO_MEMORY; application
// malloc source legitimately requests flags0. Both routes use the real heap.
void* WINAPI real_heap_allocate(HANDLE heap,DWORD flags,SIZE_T bytes){
    auto& state=*b6_core_test::observed;++state.allocations;
    if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))unused();
    auto* result=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();
    if(result)state.blocks.emplace(result,bytes);::SetLastError(error);return result;
}
struct NativeDomain {
    b6_core_test::Observation memory;b6_core_test::RuntimeState os;std::shared_ptr<anadius::thread_domain::DomainStorage> domain;
    NativeDomain(){
        if(b6_core_test::observed||b6_core_test::runtime_state)unused();
        b6_core_test::observed=&memory;b6_core_test::runtime_state=&os;
        auto apis=b6_core_test::apis();apis.heap.heap_alloc=real_heap_allocate;
        domain=anadius::thread_domain::DomainStorage::create(b6_core_test::seed(),apis);
    }
    ~NativeDomain(){
        if(domain){try{if(!domain->close())std::terminate();}catch(...){std::terminate();}domain.reset();}
        b6_core_test::observed=nullptr;b6_core_test::runtime_state=nullptr;
    }
};
struct Factory {
    NativeDomain native;std::weak_ptr<ServiceRoot> service_lifetime;ServiceRoot* root=nullptr;
    Cancel cancel;std::shared_ptr<s::SessionMemoryDomain> domain;std::unique_ptr<s::OwnedIoMemory> io_memory;
    s::AcceptorState pending_state;s::SocketObject pending{pending_state};s::SessionPair pair;unsigned moves=0;
    bool socket_after_input=false;
    static void move(void* p,s::ExecutorFields& destination,s::ExecutorFields& source,std::uintptr_t table){
        auto& f=*static_cast<Factory*>(p);if(table!=0x180084928ULL||f.root->gate.held)unused();++f.moves;
        destination.context=source.context;destination.zero_74=source.zero_74;destination.self_reference_80=true;
    }
    Factory(){
        if(!native.domain->initialize())unused();auto services=std::make_shared<ServiceRoot>();root=services.get();service_lifetime=services;
        domain=std::make_shared<s::SessionMemoryDomain>(native.domain->make_memory(),std::shared_ptr<s::LifecycleEffects>(services,&services->life));
        io_memory=std::make_unique<s::OwnedIoMemory>(domain,16);
        auto operation=domain->enter();s::initialize_0211b0(pending,{0x1234},root->life);
        pending_state.socket=77;pending_state.socket_state=0x10;pending.cancellation={0x9876,&cancel};
        pair=domain->construct_session(pending,{this,move});
        root->life.before_close=[&](s::SocketObject& object){
            if(object.state.socket==77){auto& owner=*pair_owner;const auto snapshot=domain->snapshot();
                socket_after_input=owner.input.size_10==0&&owner.input.capacity_18==15&&owner.input.storage_00[0]==0&&owner.output.proxy_c8==nullptr&&snapshot.proxies==0&&snapshot.buffer_borrows==0&&snapshot.blocks==0&&snapshot.maps==0&&owner.weak_self.control_08->weak_0c>=2;
            }
        };
        pair_owner=pair.object;
    }
    s::SessionOwner* pair_owner=nullptr; // read only while observer weak keeps control allocated
    void release_external(){auto* control=pair.control;pair={};strong_release(control);}
    void destroy_pending(){s::destroy_024db0(pending,root->life);}
    void close_after_drain(){
        require(!domain->close()&&domain->snapshot().provider_leases==1,"Idle OwnedIoMemory provider lease blocks parent close after all session objects drain");
        require(io_memory->close()&&io_memory->snapshot().phase==s::IoMemoryPhase::Closed&&domain->snapshot().provider_leases==0,"Observed IO completion and terminal cache drain permit IO close to release parent lease");
        io_memory.reset();
        require(domain->close()&&domain->snapshot().phase==s::SessionMemoryPhase::Closed&&native.domain->provider_leases()==0,"Quiescent session close releases its sole ApplicationMemory provider lease");
        domain.reset();require(service_lifetime.expired(),"Session memory domain retained owning lifecycle root through all callbacks and released it at destruction");
        require(native.domain->close()&&native.memory.blocks.empty()&&native.memory.allocations==native.memory.frees&&native.memory.slot_releases==1,"Backing real process heap/PTD/FLS domain closes with every observed heap allocation freed");
    }
};
struct Table:s::IoHandlerVtable {
    unsigned moves=0,destroys=0;Table():IoHandlerVtable(0x180017a30ULL){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;to.payload_00=from.payload_00;to.state_18=from.state_18;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}
};
struct Terminal:s::FrameReadTerminal {[[noreturn]]void bad_weak_ptr_026860()override{unused();}};
struct Category:s::WriteFinishCategory {Category(){identity_08=0x3333;}void message_slot_10(s::WriteFinishString&,std::int32_t)override{unused();}};
// Only SDK/TLS/error/entry components are borrowed from the earlier fixture.
// Concrete OwnedIoMemory owns every receive allocation and projection.
struct ReceiveEffects:s::AsyncReceiveEffects {
    s::OwnedIoMemory& memory;std::optional<s::IoMemoryHandle> pending;
    std::function<void(s::IoHandlerEnvironment&,std::uintptr_t)> construct;
    explicit ReceiveEffects(s::OwnedIoMemory& m):memory(m){}
    s::AsyncReceiveOperation& project_async_receive(s::OperationStorage& storage)override{
        auto& operation=memory.project_async_receive(storage);pending=memory.handle_for(storage);return operation;
    }
    void construct_environment_023dd0(s::IoHandlerEnvironment& environment,std::uintptr_t allocator)override{if(construct)construct(environment,allocator);}
};
struct Io {
    Gate gate;CRITICAL_SECTION critical{};receive_test::Entry entry{gate,critical};receive_test::Worker worker;
    receive_test::Receive lower;receive_test::Completion completion;receive_test::Errors errors;
    s::OwnedIoMemory& memory;s::PinnedAsyncReceiveApi pinned;ReceiveEffects effects;
    s::WorkerThreadState thread{};s::WorkerTlsContext tls{nullptr,&thread,nullptr};
    std::uint32_t index=17;s::IoCompletionEnvironment environment;s::AsyncReceiveService service{&entry};s::ReceiveSocketImplementation implementation{77,0,0,nullptr};
    s::AsyncReceiveContext context;
    explicit Io(s::OwnedIoMemory& m):memory(m),pinned(m,lower),effects(m),environment{{index,m.storage_bindings()},{}},
        context{environment,worker,m,pinned,completion,errors,effects}{entry.completion.port=0xabcdef;}
    s::AsyncReceiveOperation& operation(){if(!effects.pending)unused();return memory.receive_operation(*effects.pending);}
    void drain_cache(){
        // Test scheduler has observed all completions and ends its worker now.
        // Source worker does not clear cache words; explicit controller adapter
        // zeros each terminal slot only after concrete free succeeds.
        for(auto index:{0,1,4,5})if(const auto identity=thread.cache_00[index]){memory.free_cached_identity(identity);thread.cache_00[index]=0;}
    }
};
struct Loop;
struct Completion:s::IoCompletionEffects {
    Loop& loop;explicit Completion(Loop& l):loop(l){}
    s::Category misc_category_0285b0()override{return {0x3333};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override;
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unused();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}
};
struct FrameApi:s::FrameReadApi {
    Loop& loop;explicit FrameApi(Loop& l):loop(l){}std::unique_ptr<s::FrameMessageClosure> closure;unsigned detached=0,freed=0;
    void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override;
    s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override{if(n!=48||closure)unused();closure=std::make_unique<s::FrameMessageClosure>();return closure.get();}
    std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t a,std::uint32_t b,std::uintptr_t entry,s::FrameMessageClosure* p,std::uint32_t flags,std::uint32_t& id)override{
        if(a||b||flags||entry!=0x180021770ULL||p!=closure.get())unused();id=57;return 99;
    }
    std::int32_t detach_04a458(const s::FrameThreadHandle& h)override{if(h.id!=57||h.handle!=99)unused();++detached;return 0;}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
    [[noreturn]]void invalid_parameter_05c974()override{unused();}
    void next_write_02a500(std::uintptr_t)override{unused();}
    s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}
    void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}
};
struct ReadApi:s::RecoveredFrameReadApi {
    using RecoveredFrameReadApi::RecoveredFrameReadApi;
    void construct_length_error_024390(s::ReadLengthError&,std::uintptr_t)override{unused();}
    [[noreturn]]void throw_length_error_023930(s::ReadLengthError&)override{unused();}
};
struct Loop {
    Factory& factory;Io io;Table table;Terminal terminal;Category category;Completion completion{*this};FrameApi api{*this};
    s::SessionOwner& owner=*factory.pair.object;s::IoReference* control=factory.pair.control;std::uintptr_t identity=owner.identity();
    std::array<s::FrameReadOwnerBinding,1> owners{{{identity,&owner.weak_self}}};
    std::array<s::WriteFinishCategoryBinding,1> categories{{{{0x3333},&category}}};
    s::FrameReadContext frame{factory.domain->buffer_context(),completion,api,owners,categories,{},terminal,{0x1111},nullptr};ReadApi read_api{frame};
    std::array<s::ReceiveStringBinding,1> strings{{{identity+0xa8,&owner.input}}};
    std::array<s::ReceiveStreamBinding,1> streams{{{identity+0x10,&io.service,&io.implementation}}};
    s::ComposedReadContext read{factory.domain->buffer_context(),io.context,completion,read_api,strings,streams};unsigned callbacks=0;
    explicit Loop(Factory& storage):factory(storage),io(*storage.io_memory){
        frame.read=&read;io.implementation={owner.socket_state.socket,0x10,identity,control};
        io.environment.sentinels={&table,0,nullptr,table.destroy_identity};
        io.effects.construct=[&](auto& env,auto allocator){if(allocator!=identity+0x78)unused();env.operations_10=&table;env.state_18=0;};
    }
    void start(){s::start_frame_read_0280d0(identity,frame);}
    void complete(std::string payload,bool eof=false){
        if(!io.effects.pending)unused();const auto handle=*io.effects.pending;auto& operation=io.memory.receive_operation(handle);
        if(payload.size()>operation.io.requested_58)unused();
        auto destination=factory.domain->buffer_context().memory.view(operation.buffer_data_50,payload.size());std::copy(payload.begin(),payload.end(),destination.begin());
        io.effects.pending.reset();++callbacks;
        io.memory.complete_receive(handle,1,{0,{0x2222}},payload.size(),io.environment,io.worker,io.errors,completion);
        io.entry.work_count_30.fetch_sub(1);
        if(eof&&io.effects.pending)unused();
    }
    std::string text(const s::ReceiveBufferString& value){
        if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
        auto bytes=factory.domain->buffer_context().memory.view(s::receive_buffer_address(value),value.size_10);return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
    }
    void copy(s::ReceiveBufferString& out,std::string_view value){
        out={};if(value.size()<=15){out.size_10=value.size();std::copy(value.begin(),value.end(),out.storage_00.begin());return;}
        auto& context=factory.domain->buffer_context();s::resize_receive_buffer_010040(out,value.size(),0,context);
        auto bytes=context.memory.view(s::receive_buffer_address(out),value.size());std::copy(value.begin(),value.end(),bytes.begin());
    }
    void free_string(s::ReceiveBufferString& value){
        if(value.capacity_18>15){auto address=s::receive_buffer_address(value);auto bytes=value.capacity_18+1;
            if(bytes>=4096){auto header=factory.domain->buffer_context().memory.view(address-8,8);address=0;for(unsigned n=0;n<8;++n)address|=std::uintptr_t(header[n])<<(8*n);bytes+=39;}
            factory.domain->free_sized_04b0f8(address,bytes);
        }value={};
    }

};
void Completion::receive_body_025c40(s::ReceiveHandler& h,s::Error& e,std::uint64_t n,std::uint32_t f){s::composed_receive_025c40(h,e,n,f,loop.read);}
void FrameApi::construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t address,std::uint64_t count){auto bytes=loop.factory.domain->buffer_context().memory.view(address,count);loop.copy(out,{reinterpret_cast<const char*>(bytes.data()),bytes.size()});}
void FrameApi::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){
    if(closure&&p==reinterpret_cast<std::uintptr_t>(closure.get())){if(n!=48||closure->message_00.size_10)unused();closure.reset();++freed;return;}
    loop.factory.domain->free_sized_04b0f8(p,n);
}
void factory_read_eof(){
    Factory storage;auto* const control=storage.pair.control;
    {
        auto operation=storage.domain->enter();Loop loop(storage);auto& f=storage;
        require(f.moves==2&&f.root->service.head==&loop.owner.socket&&loop.owner.socket.next==&f.pending,"Actual domain factory performs two moves and temporary cleanup before concrete IO binding");
        require(control->strong_08==1&&control->weak_0c==2&&loop.owner.weak_self.control_08==control&&loop.owner.weak_self.input_a8==&loop.owner.input,"Factory owner/control/input publication is consumed without a fixture-prepared owner");
        const auto made=f.domain->snapshot();
        require(made.controls==1&&made.proxies==1&&made.allocations==2&&made.provider_leases==1&&f.native.domain->provider_leases()==1&&!f.service_lifetime.expired(),"Idle concrete IO provider retains parent and owning service/heap/FLS domain");
        require(!f.domain->close()&&f.domain->snapshot().phase==s::SessionMemoryPhase::Open,"Active whole-chain Operation blocks parent close");
        require(loop.owner.socket.cancellation.control==&f.cancel&&reinterpret_cast<void*>(control)!=reinterpret_cast<void*>(&f.cancel),"Socket cancellation control remains distinct from source IO owner control");
        control->weak_0c.fetch_add(1);loop.start();const auto handle=*loop.io.effects.pending;auto& receive=loop.io.operation();
        const auto by_storage=loop.io.memory.handle_for(receive.io.storage);const auto by_overlapped=loop.io.memory.handle_for(&receive.io.operation.overlapped);
        require(receive.io.handler.control_a8==control&&receive.io.weak_48==control&&receive.weak_object_40==loop.identity&&by_storage.slot==handle.slot&&by_storage.generation==handle.generation&&by_overlapped.slot==handle.slot&&by_overlapped.generation==handle.generation,"Concrete receive storage/sidecar OVERLAPPED map to exact generation and factory strong/weak pairs");
        require(reinterpret_cast<std::uintptr_t>(&receive.io.operation.overlapped)!=receive.io.storage.identity&&loop.io.memory.storage_bindings().size()==16&&loop.io.memory.dispatcher_bindings().size()==16,"Fixed concrete binding arrays distinguish typed sidecar OVERLAPPED from raw allocation identity");
        const auto pending=f.domain->snapshot();
        require(control->strong_08==3&&control->weak_0c==4&&pending.buffer_borrows==1&&pending.buffers==2&&f.io_memory->snapshot().pending==1,"Pinned SDK submission holds extra strong owner plus real input-range lease beside receive storage");
        f.io_memory->note_cancel_requested(handle);const auto cancel=f.io_memory->snapshot();
        require(cancel.pending==1&&cancel.cancel_requested==1&&control->strong_08==3&&f.domain->snapshot().buffer_borrows==1,"Cancellation request records intent and retains pending source owner and input lease");
        bool denied=false;try{f.domain->free_sized_04b0f8(s::receive_buffer_address(loop.owner.input),loop.owner.input.capacity_18+1);}catch(const std::logic_error&){denied=true;}
        require(denied&&f.domain->snapshot().phase==s::SessionMemoryPhase::Open&&f.domain->snapshot().buffer_borrows==1,"Exact owned input free is rejected while SDK buffer borrower remains pending");
        f.release_external();require(control->strong_08==2&&f.root->life.closed.empty(),"Pending native handler and explicit SDK pin retain owner after external strong release");
        loop.complete("",true);const auto disposed=f.domain->snapshot();
        require(control->strong_08==0&&control->weak_0c==1&&disposed.allocations==1&&disposed.controls==1&&!disposed.proxies&&!disposed.buffers&&!disposed.buffer_borrows&&!f.io_memory->snapshot().pending&&!f.io_memory->snapshot().allocations,"Observed actual027710 EOF releases IO storage/loan and disposes session before observer weak remains alone");
        require(f.socket_after_input&&f.root->life.closed==std::vector<s::Socket>{77}&&f.cancel.disposed==1&&f.cancel.deleted==1,"Concrete IO completion releases pin before actual session input/proxy/socket disposal");
        require(!loop.io.effects.pending&&!loop.io.entry.work_count_30,"All observed completion work drains without treating cancel request as kernel quiescence");
    }
    require(!storage.domain->close()&&storage.domain->snapshot().phase==s::SessionMemoryPhase::Open,"Outside-scope parent close retains live observer weak and concrete provider");
    {
        auto operation=storage.domain->enter();weak_release(control);storage.destroy_pending();const auto done=storage.domain->snapshot();
        require(!done.allocations&&done.controls_destroyed==1&&done.successful_allocations==done.successful_frees&&!storage.root->service.head,"Final observer weak actual delete drains parent allocation registry after concrete IO completion");
    }
    storage.close_after_drain();
}
struct Events:m::EventQueue {
    bool nonempty_relaxed()const override{return false;}int lock_04a6ac()override{unused();}std::int32_t& lock_count_4c()override{unused();}bool nonempty_locked()const override{unused();}std::string copy_front_locked()const override{unused();}void pop_front_locked()override{unused();}void unlock_04a6b4()override{unused();}
};
struct Random:m::Random {m::Key challenge_bytes_034cf0()override{return {};}m::Key key_from_response_034c30(std::string_view)override{return {};}};
struct XmlFacade:anadius::handlers_recovered::Xml {
    m::XmlPrinter& printer;explicit XmlFacade(m::XmlPrinter& p):printer(p){}
    const char* attribute_002cc0(anadius::handlers_recovered::Request,const char*)override{unused();}
    void open_004080(const char* name,bool)override{printer.open(name);}void attribute_005560(const char* key,const char* value)override{printer.attribute(key,value);}void close_virtual_48(bool)override{printer.close();}
};
struct MessageEffects:m::Effects {
    unsigned commands=0;int atoi_05ee08(std::string_view)override{unused();}
    bool registered_handler(std::uint32_t,const m::XmlNode&,m::XmlPrinter& printer)override{++commands;XmlFacade xml(printer);return anadius::handlers_recovered::get_config_02e5f0(xml);}
    [[noreturn]]void invalid_message_02bd30_04d228()override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{}void log_response(std::string_view)override{}void log_event(std::string_view)override{unused();}void log_bad_xml(std::string_view,std::string_view)override{unused();}void log_decryption_failure(std::string_view)override{unused();}
};
struct MessageApi:s::MessageDispatchApi {
    Loop& loop;anadius::xml_reader_bridge::Backend xml;s::SessionCipherFactory crypto;Random random;Events events;MessageEffects effects;
    std::optional<s::MessageWriteTask> job;unsigned processed=0,cleanups=0;std::uint8_t show=0;
    explicit MessageApi(Loop& l):loop(l){}
    void process_message_033d70(std::uintptr_t handler,s::ReceiveBufferString& response,s::ReceiveBufferString& request)override{
        if(handler!=loop.identity+0xf0||loop.control->strong_08==0)unused();++processed;
        auto input=loop.text(request);auto output=m::process_message_033d70({loop.owner.cipher.encrypted_c4,show,loop.owner.cipher,events},input,xml,crypto,random,effects);
        loop.free_string(request);loop.copy(request,input);loop.copy(response,output);
    }
    // Explicit controlled lower scheduling adapter, not recovered021420/IOCP.
    void dispatch_response_021420(s::IoHandlerEnvironment&,s::MessageWriteTask& task,std::uint64_t a,std::uint64_t b)override{
        if(a||b||job||task.control_30!=loop.control||task.object_28!=loop.identity||task.function_00!=0x180028270ULL)unused();job=task;task.response_08={};task.object_28=0;task.control_30=nullptr;
    }
    void destroy_empty_message_00aa50(s::ReceiveBufferString& value)override{loop.free_string(value);}
    void cleanup_exit_waits_04a47c()override{++cleanups;}
};
void copy_environment(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from){to.payload_00=from.payload_00;to.state_18=from.state_18;}
void seed_quiescent_output(Loop& loop){
    auto& domain=*loop.factory.domain;auto map=domain.allocate_output_map(8);
    for(auto index:{3,6,7})map[index]=domain.allocate_output_block();
    loop.copy(*map[6],std::string(96,'q'));loop.copy(*map[7],"last");
    loop.owner.output.fields={map,8,6,2};
}
void factory_frame_message_response(bool encrypted){
    Factory storage;{auto operation=storage.domain->enter();Loop loop(storage);auto& f=storage;auto* const control=loop.control;control->weak_0c.fetch_add(1);
    s::IoHandlerEnvironment environment{{1,2},&loop.table,0,0x456,nullptr};s::MessageDispatchOwner owner{&loop.owner.weak_self,&environment};
    std::array<s::MessageDispatchOwnerBinding,1> owners{{{loop.identity,&owner}}};std::array<s::MessageEnvironmentCopyBinding,1> copies{{{&loop.table,copy_environment}}};
    MessageApi api{loop};s::MessageDispatchContext message{loop.frame,api,owners,copies,{}};
    // First reset belongs to actual023000. Explicit accepted-start reset is
    // second; no semantic Session constructor introduces another reset.
    loop.io.worker.tls=&loop.io.tls;s::reset_session_cipher_02bc70(loop.owner.cipher);loop.start();
    const std::string request="<LSX><Request id=\"owner\" recipient=\"test\"><GetConfig/></Request></LSX>";
    auto wire=request;s::SessionCipher client;
    if(encrypted){
        client.set_fixed_key_084c98();loop.owner.cipher.set_fixed_key_084c98();loop.owner.cipher.encrypted_c4=1;
        require(client.encrypt_042610(wire),"Independent client encrypts owned request for factory SessionCipher");m::hex_encode_03d190(wire);
    }
    loop.complete(wire+std::string(1,'\0'));
    require(loop.api.closure&&loop.api.closure->owner_20==loop.identity&&loop.api.detached==1&&loop.io.effects.pending&&control->strong_08==3,"Actual025480 publishes raw factory owner closure and rearms actual native receive before message execution");
    f.release_external();require(control->strong_08==2&&f.domain->snapshot().controls_destroyed==0,"Pending read explicitly keeps raw detached job owner alive before033d70 strong acquisition");
    require(s::message_thread_entry_021770(*loop.api.closure,message)==0,"Actual021770 runs actual028d30 against factory owner and frees its frame closure");
    require(!loop.api.closure&&loop.api.freed==1&&api.processed==1&&api.effects.commands==1&&api.cleanups==1&&api.job&&api.job->control_30==control&&control->strong_08==3,"Actual message core/GetConfig response transfers acquired factory strong into controlled response job");
    auto response=loop.text(api.job->response_08);
    require(!response.empty()&&response.back()==0,"Response job preserves real message core NUL terminator");
    if(encrypted){
        response.pop_back();m::hex_decode_03ce30(response);
        require(client.decrypt_042480(response)&&loop.owner.cipher.encrypted_c4==1&&loop.owner.cipher.native().mode_00==2,"Factory SessionCipher decrypts request and encrypts response, independently client-decrypted after native dispatch");response.push_back(0);
    }
    require(response.find("GetConfigResponse")!=std::string::npos&&response.find("id=\"owner\"")!=std::string::npos&&!response.empty()&&response.back()==0,"Real XML parser/MemoryPrinter and recovered GetConfig body produce exact correlated NUL response");
    loop.complete("",true);
    require(control->strong_08==1&&f.domain->snapshot().controls_destroyed==0&&f.root->life.closed.empty(),"After receive EOF only response job owns factory object; socket stays open until response drain");
    loop.free_string(api.job->response_08);auto* residual=api.job->control_30;api.job.reset();
    // Own synthetic retained deque allocations after all external message/read
    // borrowers drain. No new native write publication or thread claim.
    require(f.io_memory->snapshot().pending==0&&f.domain->snapshot().buffer_borrows==0,"Reentrant frame completion and EOF release all observed SDK pins before worker cache drain");
    loop.io.drain_cache();require(f.io_memory->snapshot().allocations==0,"Terminal worker adapter frees concrete receive cache identities only after callbacks drain");
    seed_quiescent_output(loop);strong_release(residual);
    const auto disposed=f.domain->snapshot();
    require(disposed.allocations==1&&disposed.controls==1&&!disposed.proxies&&!disposed.buffers&&!disposed.blocks&&!disposed.maps&&!disposed.pending_frees&&f.socket_after_input,"Actual026880/026570 release domain-backed queued heap/blocks/map/proxy/input before socket disposal");
    require(loop.owner.weak_self.control_08==control&&loop.owner.weak_self.object_00==loop.identity&&control->strong_08==0&&control->weak_0c==1&&!disposed.controls_destroyed,"Recovered disposal preserves selfweak slots and only observer weak retains typed control storage");
    weak_release(control);f.destroy_pending();const auto done=f.domain->snapshot();
    require(!done.allocations&&done.controls_destroyed==1&&done.successful_allocations==done.successful_frees&&!f.root->service.head&&!loop.io.entry.work_count_30&&f.cancel.disposed==1&&f.cancel.deleted==1,"Final actual017420/026290 deletes real domain control after plain/encrypted source-chain drain");
    }
    storage.close_after_drain();
}
}
int main(){try{factory_read_eof();factory_frame_message_response(false);factory_frame_message_response(true);std::cout<<"PASS: "<<checks<<" IO-memory integration checks; concrete OwnedIoMemory/pinned SDK submission, real process heap/FLS, actual native frame/message/disposal, GetConfig/AES and controlled response scheduler\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected IO-memory integration exception\n";return 1;}}
