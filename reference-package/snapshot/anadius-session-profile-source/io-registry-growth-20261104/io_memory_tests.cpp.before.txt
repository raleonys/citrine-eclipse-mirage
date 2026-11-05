#include "io_memory.hpp"
#include "../anadius-socket-source/async_receive_test_fixture.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <iostream>
#include <thread>

namespace {
namespace s=anadius::socket_recovered;namespace td=anadius::thread_domain;namespace b=b6_core_test;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<"\n";throw std::runtime_error(text);}}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Unknown{};
struct Faults {
    bool fail_alloc=false,throw_alloc=false,throw_after_free=false;
    void* fail_free=nullptr;unsigned free_calls=0;
    std::vector<std::pair<void*,std::size_t>> allocations;
    std::function<void()> allocation_hook;
    std::function<void()> free_hook;
};
Faults* faults=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T bytes){
    if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))throw std::logic_error("unexpected heap allocation contract");
    ++b::observed->allocations;
    if(faults->fail_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();
    if(raw){b::observed->blocks.emplace(raw,bytes);faults->allocations.emplace_back(raw,bytes);}
    if(faults->allocation_hook)faults->allocation_hook();
    if(faults->throw_alloc)throw Unknown{};
    ::SetLastError(error);return raw;
}
BOOL WINAPI release(HANDLE heap,DWORD flags,void* raw){
    ++faults->free_calls;
    if(heap!=::GetProcessHeap()||flags||!b::observed->blocks.contains(raw))throw std::logic_error("unknown/double raw heap free");
    if(faults->fail_free==raw){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    const auto result=::HeapFree(heap,flags,raw);const auto error=::GetLastError();
    if(result){b::observed->blocks.erase(raw);++b::observed->frees;}
    if(faults->free_hook)faults->free_hook();
    if(faults->throw_after_free)throw Unknown{};
    ::SetLastError(error);return result;
}
struct Heap {
    b::Observation observed;b::RuntimeState runtime;Faults state;std::shared_ptr<td::DomainStorage> root;
    Heap(){
        if(b::observed||b::runtime_state||faults)throw std::logic_error("overlapping heap fixture");
        b::observed=&observed;b::runtime_state=&runtime;faults=&state;
        auto api=b::apis();api.heap.heap_alloc=allocate;api.heap.heap_free=release;
        root=td::DomainStorage::create(b::seed(),api);
        if(!root->initialize())throw std::runtime_error("own FLS/heap initialization failed");
    }
    ~Heap(){
        state.fail_alloc=false;state.fail_free=nullptr;state.throw_after_free=false;state.throw_alloc=false;state.allocation_hook={};
        if(!root->close())std::terminate();root.reset();
        if(!observed.blocks.empty())std::terminate();faults=nullptr;b::observed=nullptr;b::runtime_state=nullptr;
    }
};
struct Gate:s::SocketGate {void enter()override{}void leave()override{}};
struct Life:s::LifecycleEffects {
    Gate gate;s::SocketService service{gate};unsigned closes=0;
    std::function<void()> retiring;
    ~Life(){if(retiring)retiring();}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& socket)override{if(socket.state.socket!=s::invalid_socket)++closes;socket.state.socket=s::invalid_socket;}
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}
};
void executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t){to.context=from.context;to.zero_74=from.zero_74;to.self_reference_80=true;}
void weak_release(s::IoReference* value){if(value&&value->weak_0c.fetch_sub(1)==1)value->delete_slot_08();}
void strong_release(s::IoReference* value){if(value&&value->strong_08.fetch_sub(1)==1){value->dispose_slot_00();weak_release(value);}}
struct Pending {
    std::shared_ptr<Life> life;s::AcceptorState state;s::SocketObject socket{state};
    explicit Pending(std::shared_ptr<Life> l):life(std::move(l)){s::initialize_0211b0(socket,{77},*life);state.socket=77;}
    void destroy(){s::destroy_024db0(socket,*life);}
};
[[noreturn]]void unused(){throw std::logic_error("Unselected owned IO dependency");}
struct Table:s::IoHandlerVtable {
    Table():IoHandlerVtable(0x777){}unsigned moves=0,destroys=0;
    void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;to.payload_00=from.payload_00;to.state_18=from.state_18;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}
};
struct Effects:s::IoCompletionEffects {
    unsigned receives=0,sends=0;std::function<void()> receive,send;
    s::Category misc_category_0285b0()override{return {0x3333};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{++receives;if(receive)receive();}
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{++sends;if(send)send();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}
};
struct Send:s::AsyncSendApi {
    unsigned calls=0;s::AsyncSendResult answer{-1,997,0};std::function<void()> hook;
    s::AsyncSendResult send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov)override{
        require(socket==77&&buffer.len==3&&!flags&&!ov.Internal&&!ov.InternalHigh&&!ov.Pointer&&!ov.hEvent,"selected SDK send gets live three-byte buffer and true zeroed OVERLAPPED");
        ++calls;if(hook)hook();return answer;
    }
};
struct Functor:s::PostedFunctor {unsigned calls=0;void call_slot_00(std::uint8_t flag)override{require(flag==1,"posted completion invokes moved functor");++calls;}};
struct Fixture {
    Heap heap;std::shared_ptr<Life> life=std::make_shared<Life>();Pending pending{life};
    std::shared_ptr<s::SessionMemoryDomain> root=std::make_shared<s::SessionMemoryDomain>(heap.root->make_memory(),life);
    s::OwnedIoMemory memory;s::SessionPair pair;std::uint32_t index=17;
    receive_test::Worker worker;receive_test::Errors errors;Effects effects;Table table;
    s::IoHandlerExecutor executor;s::WorkerThreadState thread{};s::WorkerTlsContext tls{nullptr,&thread,nullptr};
    explicit Fixture(std::size_t capacity=8):memory(root,capacity){auto operation=root->enter();pair=root->construct_session(pending.socket,{nullptr,::executor});}
    s::IoCompletionEnvironment environment(){return {{index,memory.storage_bindings()},{&table,0x846e8,&executor,0x777}};}
    s::IoHandlerEnvironment handler_environment(){return {{1,2},&table,0,0x846e8,&executor};}
    s::Buffer buffer(){s::resize_receive_buffer_010040(pair.object->input,3,'i',root->buffer_context());s::Buffer value;value.len=3;value.buf=reinterpret_cast<char*>(s::receive_buffer_address(pair.object->input));return value;}
    void receive_owner(s::AsyncReceiveOperation& op){pair.control->strong_08.fetch_add(1);pair.control->weak_0c.fetch_add(1);op.io.handler.control_a8=pair.control;op.io.handler.object_a0=reinterpret_cast<std::uintptr_t>(pair.object);op.io.weak_48=pair.control;op.io.environment_b0=handler_environment();}
    void send_owner(s::AsyncSendOperation& op){pair.control->strong_08.fetch_add(1);pair.control->weak_0c.fetch_add(1);op.io.handler.control_98=pair.control;op.io.handler.object_90=reinterpret_cast<std::uintptr_t>(pair.object);op.io.weak_40=pair.control;op.io.environment_a0=handler_environment();}
    void drain_terminal_cache(){for(const auto identity:thread.cache_00)if(identity)memory.free_cached_identity(identity);}
    void finish(){
        {auto operation=root->enter();require(memory.snapshot().allocations==0,"all IO allocations drained before session disposal");strong_release(pair.control);pair={};pending.destroy();require(root->snapshot().allocations==0,"actual B32 disposal drains controls/proxy/input after IO");}
        require(!root->close()&&root->snapshot().provider_leases==1,"idle concrete IO provider keeps parent open");
        require(memory.close()&&memory.close()&&root->snapshot().provider_leases==0,"IO close retires provider lease idempotently");
        require(root->close()&&heap.root->provider_leases()==0,"session close drains dedicated real application-memory lease");
    }
};
void admission_and_fixed_views(){
    Fixture f(2);auto bindings=f.memory.storage_bindings();auto packets=f.memory.dispatcher_bindings();
    require(bindings.size()==2&&packets.size()==2,"fixed-capacity bindings supplied before any allocation");
    rejects<std::logic_error>([&]{f.memory.allocate_04b0bc(73);},"foreign call without whole parent Operation rejected");
    {auto operation=f.root->enter();
     rejects<std::logic_error>([&]{f.root->acquire_provider_lease();},"controller provider lease cannot be acquired inside Operation");
     rejects<std::invalid_argument>([&]{f.memory.allocate_04b0bc(0);},"empty marker storage rejected before heap");
     auto& first=s::allocate_cached_021820(0,nullptr,0x48,1,f.environment().storage,f.memory);
     auto& posted=f.memory.project_posted(first);auto h=f.memory.handle_for(first);
     require(first.identity==reinterpret_cast<std::uintptr_t>(first.bytes.data())&&first.bytes.size()==73,"owned raw operation identity is actual same-domain heap allocation");
     require(f.memory.storage_bindings().data()==bindings.data()&&f.memory.dispatcher_bindings().data()==packets.data(),"borrowed copied spans never relocate");
     require(packets[h.slot].operation==&posted.operation&&packets[h.slot].overlapped_identity==&posted.operation.overlapped&&reinterpret_cast<std::uintptr_t>(&posted.operation.overlapped)!=first.identity,"SDK uses separate typed OVERLAPPED, never guessed native raw overlay");
     auto& second=f.memory.allocate_04b0bc(73);auto allocations=f.heap.observed.allocations;
     rejects<std::bad_alloc>([&]{f.memory.allocate_04b0bc(73);},"fixed metadata capacity fails before native allocation");
     require(allocations==f.heap.observed.allocations&&f.memory.snapshot().phase==s::IoMemoryPhase::Open,"capacity failure is effect-free and leaves admission open");
     s::OperationStorage forged{first.identity,first.bytes};rejects<std::invalid_argument>([&]{f.memory.free_04b788(forged);},"forged descriptor cannot free owned raw allocation");
     const auto span=first.bytes;first.bytes=first.bytes.first(72);
     rejects<std::invalid_argument>([&]{f.memory.free_04b788(first);},"altered public span cannot change original raw-allocation ledger");first.bytes=span;
     const auto identity=first.identity;first.identity=second.identity;
     rejects<std::invalid_argument>([&]{f.memory.free_04b788(first);},"altered identity cannot free a different owned allocation");first.identity=identity;
     require(f.heap.observed.allocations==allocations&&f.memory.snapshot().allocations==2,"descriptor validation fails before raw free or IO poison");
     f.memory.free_04b788(second);f.memory.free_04b788(first);
     require(!packets[h.slot].operation&&!packets[h.slot].overlapped_identity&&!bindings[h.slot].storage,"bindings deactivate before actual projection destruction");
     rejects<std::invalid_argument>([&]{f.memory.kind(h);},"freed generation rejects stale explicit controller handle");
     auto& replacement=f.memory.allocate_04b0bc(73);f.memory.project_posted(replacement);auto fresh=f.memory.handle_for(replacement);
     require(fresh.slot==h.slot&&fresh.generation>h.generation,"slot reuse gets a new generation regardless of allocator address reuse");f.memory.free_04b788(replacement);
    }f.finish();
}
void receive_send_and_cross_kind_cache(){
    Fixture f;f.worker.tls=&f.tls;Functor functor;std::uint64_t original_extent=0;std::uintptr_t original_raw=0;
    {auto operation=f.root->enter();auto buffer=f.buffer();
     auto& storage=s::allocate_io_storage_0269b0(0xe0,f.environment().storage,f.worker,f.memory);original_extent=storage.bytes.size();original_raw=storage.identity;
     auto& receive=f.memory.project_async_receive(storage);f.receive_owner(receive);const auto old=f.memory.handle_for(storage);
     receive_test::Receive lower;s::PinnedAsyncReceiveApi sdk(f.memory,lower);auto result=sdk.receive(77,buffer,0,receive.io.operation.overlapped);
     require(result.last_error==997&&lower.calls==1&&f.pair.control->strong_08==3&&f.pair.control->weak_0c==3,"pending receive has source handler/weak plus explicit extra SDK owner strong");
     require(f.root->snapshot().buffer_borrows==1&&f.memory.snapshot().pending==1,"actual SDK buffer range is pinned in owning parent ledger");
     f.memory.note_cancel_requested(old);require(f.memory.snapshot().cancel_requested==1&&f.pair.control->strong_08==3&&f.root->snapshot().buffer_borrows==1,"cancellation request cannot imply terminal completion or early unpin");
     rejects<std::logic_error>([&]{f.memory.free_04b788(storage);},"pending operation allocation cannot be freed");
     rejects<std::logic_error>([&]{f.memory.project_posted(storage);},"pending raw allocation cannot be reprojected as another type");
     require(!f.memory.close()&&!f.root->close(),"pending work keeps both IO and parent controllers open");
     f.effects.receive=[&]{
        require(f.memory.snapshot().active_calls==1&&f.root->snapshot().buffer_borrows==0&&f.pair.control->strong_08==2,"terminal observation releases compatibility pin before native callback while moved native owner remains live");
        auto& cached=s::allocate_cached_021820(0,&f.thread,0x48,1,f.environment().storage,f.memory);
        require(cached.identity==original_raw&&cached.bytes.size()==original_extent,"native receive cache reuses the original225-byte allocation as posted work");
        auto& posted=f.memory.project_posted(cached);posted.functor_38=&functor;auto next=f.memory.handle_for(cached);
        rejects<std::invalid_argument>([&]{f.memory.kind(old);},"nested callback invalidates original generation without old projection reuse");
        f.memory.complete_posted(next,1,f.environment().storage,f.worker);
     };
     f.memory.complete_receive(old,1,{},3,f.environment(),f.worker,f.errors,f.effects);
     require(f.effects.receives==1&&functor.calls==1&&f.pair.control->strong_08==1&&f.pair.control->weak_0c==2,"actual027710/027120 continue from independent locals after nested projection replacement");
     auto& reused=s::allocate_io_storage_0269b0(0xd0,f.environment().storage,f.worker,f.memory);auto& send=f.memory.project_async_send(reused);f.send_owner(send);
     Send lower_send;s::PinnedAsyncSendApi send_sdk(f.memory,lower_send);auto send_result=send_sdk.send(77,buffer,0,send.io.operation.overlapped);auto send_handle=f.memory.handle_for(reused);
     require(send_result.last_error==997&&reused.identity==original_raw&&reused.bytes.size()==225&&f.root->snapshot().buffer_borrows==1,"send borrows same parent buffer while preserving larger original receive extent");
     f.effects.send=[&]{require(f.root->snapshot().buffer_borrows==0&&f.pair.control->strong_08==2,"send terminal packet releases SDK loan before native send callback");};
     f.memory.complete_send(send_handle,1,{},3,f.environment(),f.worker,f.errors,f.effects);
     rejects<std::logic_error>([&]{f.memory.complete_send(send_handle,1,{},3,f.environment(),f.worker,f.errors,f.effects);},"duplicate terminal completion of cached projection rejected");
     require(f.effects.sends==1&&f.pair.control->strong_08==1&&f.pair.control->weak_0c==2&&f.memory.snapshot().pending==0,"native send cleanup drops only original handler/weak references once");
     f.drain_terminal_cache();require(f.memory.snapshot().allocations==0,"terminal worker bridge frees original225-byte extent despite last request209");
    }f.finish();
}
struct FrameApi:s::FrameReadApi,s::FrameReadTerminal {
    s::SessionMemoryDomain& root;explicit FrameApi(s::SessionMemoryDomain& r):root(r){}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{root.free_sized_04b0f8(p,n);}
    [[noreturn]]void invalid_parameter_05c974()override{unused();}void next_write_02a500(std::uintptr_t)override{unused();}
    s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unused();}
    s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unused();}
    std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unused();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unused();}void flush_00e1e0(s::WriteFinishStream&)override{unused();}
    void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override{unused();}
    s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t)override{unused();}
    std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::FrameMessageClosure*,std::uint32_t,std::uint32_t&)override{unused();}
    std::int32_t detach_04a458(const s::FrameThreadHandle&)override{unused();}[[noreturn]]void thread_error_04a648(int)override{unused();}
    [[noreturn]]void bad_weak_ptr_026860()override{unused();}
};
void owned_handler_and_response(){
    Fixture f;f.worker.tls=&f.tls;
    {auto operation=f.root->enter();auto env=f.environment();s::HandlerTaskContext ctx{env,f.worker,f.memory,f.effects,{}};
     auto& storage=s::allocate_handler_cached_021900(&f.thread,0x78,1,env.storage,f.memory);auto raw=storage.identity;
     auto& receive=f.memory.project_receive_task(storage,ctx);receive.handler.control_a8=f.pair.control;f.pair.control->strong_08.fetch_add(1);auto h=f.memory.handle_for(storage);
     f.effects.receive=[&]{require(f.thread.cache_00[4]==raw&&f.pair.control->strong_08==2,"native022670 caches raw before invoking moved independent owner");};
     f.memory.invoke_task(h,1);require(f.effects.receives==1&&f.pair.control->strong_08==1,"receive task native cleanup releases exactly one transferred strong");
     auto& reused=s::allocate_handler_cached_021900(&f.thread,0x70,1,env.storage,f.memory);auto& send=f.memory.project_send_task(reused,ctx);send.handler.control_98=f.pair.control;f.pair.control->strong_08.fetch_add(1);auto next=f.memory.handle_for(reused);
     require(reused.identity==raw&&reused.bytes.size()==121&&next.generation>h.generation,"native slots4/5 reuse receive task storage as smaller send task");f.memory.invoke_task(next,0);
     require(f.effects.sends==0&&f.pair.control->strong_08==1,"discard cleans native send task owner without invoking handler");
     FrameApi api(*f.root);s::FrameReadContext strings{f.root->buffer_context(),f.effects,api,{},{},{},api,{}};
     auto& response_memory=f.memory.response_memory();s::OwnedResponseContext response{env.storage,f.worker,response_memory,strings,nullptr};
     s::MessageWriteTask source;source.control_30=f.pair.control;source.object_28=reinterpret_cast<std::uintptr_t>(f.pair.object);f.pair.control->strong_08.fetch_add(1);
     s::resize_receive_buffer_010040(f.pair.object->input,64,'R',f.root->buffer_context());source.response_08=f.pair.object->input;
     f.pair.object->input.size_10=0;f.pair.object->input.capacity_18=15;f.pair.object->input.storage_00[0]=0;const auto before=f.root->snapshot().buffers;
     s::PostedFunctor* output=nullptr;s::make_owned_response_020cb0(output,source,response);auto* task=static_cast<s::OwnedResponseTask*>(output);auto response_handle=f.memory.handle_for(task->storage);
     require(task->storage.identity==raw&&!source.control_30&&source.response_08.size_10==0,"actual020cb0 transfers independent heap response payload into same cached allocation");
     f.memory.invoke_task(response_handle,0);require(f.pair.control->strong_08==1&&f.root->snapshot().buffers+1==before,"actual022280 frees moved heap response and strong exactly once after cache release");
     f.drain_terminal_cache();require(f.memory.snapshot().allocations==0,"terminal handler cache bridge frees larger original121-byte extent");
    }f.finish();
}
void failed_arm_and_exact_ranges(){
    Fixture f;
    {auto operation=f.root->enter();auto buffer=f.buffer();auto& storage=s::allocate_io_storage_0269b0(0xe0,f.environment().storage,f.worker,f.memory);auto& recv=f.memory.project_async_receive(storage);f.receive_owner(recv);
     receive_test::Receive lower;s::PinnedAsyncReceiveApi sdk(f.memory,lower);s::Buffer bad=buffer;bad.buf=reinterpret_cast<char*>(0x1234);
     rejects<std::invalid_argument>([&]{sdk.receive(77,bad,0,recv.io.operation.overlapped);},"foreign actual SDK buffer rejected before lower call");
     auto count=f.pair.control->strong_08.load();f.pair.control->strong_08=UINT32_MAX;
     rejects<std::logic_error>([&]{sdk.receive(77,buffer,0,recv.io.operation.overlapped);},"saturated extra owner retain cannot wrap");f.pair.control->strong_08=count;
     require(!lower.calls&&!f.root->snapshot().buffer_borrows&&!f.memory.snapshot().pending,"failed submission arming rolls back only metadata pin and never calls SDK");
     OVERLAPPED fake{};auto h=f.memory.handle_for(storage);f.memory.dispatcher_bindings()[h.slot].overlapped_identity=&fake;
     rejects<std::invalid_argument>([&]{sdk.receive(77,buffer,0,fake);},"mutable packet view cannot forge actual typed OVERLAPPED identity");f.memory.dispatcher_bindings()[h.slot].overlapped_identity=&recv.io.operation.overlapped;
     auto& context=f.root->buffer_context();auto allocation=context.api.allocate_04b0bc(64);auto pin=f.root->pin_buffer(allocation+16,48);
     require(f.root->snapshot().buffer_borrows==1,"one coherent owned tail range can be borrowed");
     rejects<std::invalid_argument>([&]{f.root->pin_buffer(allocation+16,49);},"SDK byte range cannot exceed actual owned allocation");
     rejects<std::logic_error>([&]{f.root->free_sized_04b0f8(allocation,64);},"raw buffer lifetime is retained independently of session owner");pin.reset();f.root->free_sized_04b0f8(allocation,64);
     // No SDK occurred: source local terminal completion is trusted only after
     // the source operation initialization/publication has ended.
     f.memory.complete_receive(h,0,{},0,f.environment(),f.worker,f.errors,f.effects);
     require(f.pair.control->strong_08==1&&f.pair.control->weak_0c==2,"local non-SDK terminal cleanup drops original handler and weak without extra pin");
    }f.finish();
}
void cross_thread_metadata_unpin(){
    Fixture f;std::shared_ptr<void> pin,copy;std::uintptr_t raw=0;
    {auto operation=f.root->enter();raw=f.root->buffer_context().api.allocate_04b0bc(64);pin=f.root->pin_buffer(raw,64);copy=pin;
     require(f.root->snapshot().buffer_borrows==1,"shared token copies keep one actual range loan");pin.reset();
     require(f.root->snapshot().buffer_borrows==1,"loan persists until last token owner drops it");}
    auto frees=f.heap.state.free_calls;std::thread([last=std::move(copy)]()mutable{last.reset();}).join();
    require(f.root->snapshot().buffer_borrows==0&&f.heap.state.free_calls==frees,"foreign thread without PTD releases metadata only, never source/native heap");
    {auto operation=f.root->enter();f.root->free_sized_04b0f8(raw,64);}f.finish();
}
[[noreturn]]void retained_child(){
    Fixture f;auto operation=f.root->enter();auto buffer=f.buffer();auto& storage=s::allocate_io_storage_0269b0(0xe0,f.environment().storage,f.worker,f.memory);auto& recv=f.memory.project_async_receive(storage);f.receive_owner(recv);
    receive_test::Receive lower;lower.hook=[](){throw Unknown{};};s::PinnedAsyncReceiveApi sdk(f.memory,lower);
    rejects<Unknown>([&]{sdk.receive(77,buffer,0,recv.io.operation.overlapped);},"throwing lower retains unknown SDK outcome");
    require(f.memory.snapshot().phase==s::IoMemoryPhase::Poisoned&&f.memory.snapshot().pending==1&&f.root->snapshot().buffer_borrows==1&&f.pair.control->strong_08==3,"unknown SDK publication retains raw projection, real buffer pin and extra strong");
    require(!f.memory.close()&&!f.root->close(),"unknown outcome never pretends to drain or close");
    std::cout<<"retained unknown SDK child verified\n"<<std::flush;std::_Exit(0);
}
void child_check(){
    std::array<wchar_t,32768> executable{};if(!::GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size())))unused();
    std::wstring command=L"\""+std::wstring(executable.data())+L"\" --unknown-sdk";STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    if(!::CreateProcessW(executable.data(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process))unused();
    auto waited=::WaitForSingleObject(process.hProcess,30000);DWORD code=99;if(waited!=WAIT_OBJECT_0)::TerminateProcess(process.hProcess,98);else ::GetExitCodeProcess(process.hProcess,&code);
    ::CloseHandle(process.hThread);::CloseHandle(process.hProcess);require(waited==WAIT_OBJECT_0&&code==0,"own child proves conservative unknown SDK retention without executing DLL target");
}
} // namespace
int main(int argc,char**){try{if(argc==2)retained_child();admission_and_fixed_views();receive_send_and_cross_kind_cache();owned_handler_and_response();failed_arm_and_exact_ranges();cross_thread_metadata_unpin();child_check();
    std::cout<<"PASS: "<<checks<<" owned IO memory controls\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected owned IO failure\n";return 1;}}
